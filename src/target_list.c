/* target_list.c
 * Copyright (c) 2010 ThoughtGang
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 2.1.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "target_list.h"

/* ---------------------------------------------------------------------- */

/* allocate a target list */
tgt_list_t tgt_list_alloc( void ) {
	return (tgt_list_t) calloc( 1, sizeof(struct TARGET_LIST_HEAD) );
}

/* free an allocated target list */
void tgt_list_free( tgt_list_t targets ) {
	tgt_list_item_t* item, * next;

	if (! targets ) {
		return;
	}

	for ( item = targets->head; item; item = next ) {
		next = item->next;
		if ( item->data ) {
			opdis_buf_free( item->data );
		}

		if ( item->tgt_bfd ) {
			// bfd_close ?
		}

		free( item );
	}

	free( targets );
}

static opdis_buf_t load_bytes( const char * bytes ) {
	char *str, * p, * tok, *err;
	opdis_buf_t buf;
	int i, count, base = 16;

	if ( bytes[0] == '\\' ) {
		switch ( bytes[1] ) {
			case 'o': case 'O': base = 8; break;
			case 'x': case 'X': base = 16; break;
			case 'd': case 'D': base = 10; break;
			case 'b': case 'B': base = 2; break;
			default:
				base = (int) ((unsigned char) bytes[1]);
		}
	
		bytes = &bytes[2];
	}

	/* get number of bytes in string */
	for ( i = 0, count = 1; ; i++ ) {
		if ( bytes[i] == ' ' ) {
			count ++;
		} else if ( bytes[i] == '\0' ) {
			break;
		}
	}

	buf = opdis_buf_alloc( count, 0 );
	if (! buf ) {
		fprintf( stderr, "Unable to allocate buffer\n" );
		return NULL;
	}

	str = strdup( bytes );
	for ( p = str, i = 0; i < buf->len; p = NULL, err = NULL, i++ ) {
		tok = strtok( p, " " );
		if (! tok ) {
			break;
		}

		buf->data[i] = (opdis_byte_t) strtoul( tok, &err, base );
		if ( err && *err != '\0' ) {
			fprintf( stderr, "Invalid number %s for base %d\n", 
				 tok, base );
			break;
		}
	}
	free(str);

	if ( err ) {
		opdis_buf_free( buf );
		return NULL;
	}

	return buf;
}

static opdis_buf_t load_file( const char * path ) {
	FILE * f;
	opdis_buf_t buf;

	f = fopen( path, "r" );
	if (! f ) {
		fprintf( stderr, "Unable to open %s: %s\n", path, 
			 strerror(errno) );
		return NULL;
	}

	buf = opdis_buf_read( f, 0, 0 );
	if (! buf ) {
		fprintf( stderr, "Unable to read %s into buffer\n", path );
	}

	fclose( f );

	return buf;
}

/* add a target to a list */
unsigned int tgt_list_add( tgt_list_t targets, enum target_type_t type, 
			   const char * ascii ) {
	tgt_list_item_t * item, * prev;

	if (! targets || ! ascii ) {
		return 0;
	}

	item = (tgt_list_item_t *) calloc( 1, sizeof(tgt_list_item_t) );
	if (! item ) {
		return 0;
	}

	item->type = type;
	item->ascii = ascii;

	if ( type == tgt_bytes ) {
		item->data = load_bytes( ascii );
	} else if ( type == tgt_file ) {
		item->data = load_file( ascii );
	} else {
		fprintf( stderr, "Unrecognized target type %d\n", type );
	}

	if (! item->data ) {
		/* error message will have already been printed */
		free( item );
		return 0;
	}

	/* append item to list */
	if (! targets->head ) {
		targets->head = item;
	} else {
		for ( prev = targets->head; prev->next; prev = prev->next ) 
			;
		prev->next = item;
	}

	targets->num_items++;

	return targets->num_items;
}

static void load_bfd_symbols( sym_tab_t symtab, asymbol ** syms, 
			      unsigned int num_syms ) {
	unsigned int i;
	symbol_info info;

	for ( i = 0; i < num_syms; i++ ) {
		bfd_symbol_info( syms[i], &info );
		// TODO: check symbol type
		sym_tab_add( symtab, info.name, info.value );
	}
}

static void load_symbols( bfd * abfd, sym_tab_t symtab ) {
	size_t size;
	unsigned int num;
	asymbol ** syms;

	if (! bfd_get_file_flags(abfd) & HAS_SYMS ) {
		return;
	}

	size = bfd_get_dynamic_symtab_upper_bound( abfd );
	if ( size > 0 ) {
		syms = (asymbol **) malloc(size);
		if ( syms ) {
			num = bfd_canonicalize_dynamic_symtab( abfd, syms );
			load_bfd_symbols( symtab, syms, num );
			free(syms);
		}
	}

	size = bfd_get_symtab_upper_bound( abfd );
	if ( size > 0 ) {
		syms = (asymbol **) malloc(size);
		if ( syms ) {
			num = bfd_canonicalize_symtab( abfd, syms );
			load_bfd_symbols( symtab, syms, num );
			free(syms);
		}
	}
}

int tgt_list_make_bfd( tgt_list_item_t * tgt ) {
	if (! tgt ) {
		return 0;
	}

	tgt->tgt_bfd = bfd_openr( tgt->ascii, NULL );
	if (! tgt->tgt_bfd  ) {
		fprintf( stderr, "Unable to create BFD for %s\n", tgt->ascii );
		fprintf( stderr, "Will continue using non-BFD target\n" );
		return 0;
	}

	if ( bfd_get_flavour( tgt->tgt_bfd ) == bfd_target_unknown_flavour ) {
		fprintf( stderr, "WARNING: unknown BFD flavor\n" );
	}

	tgt->symtab = sym_tab_alloc();

	if ( bfd_check_format( tgt->tgt_bfd, bfd_object ) ) {
		/* only object file will have symbols */
		load_symbols( tgt->tgt_bfd, tgt->symtab );
	}

	// TODO: delete data

	return 1;
}

/* return the ID of the specified target */
/* note: ID is implicit : it is offset of item in list + 1 */
unsigned int tgt_list_id( tgt_list_t targets, const char * ascii ) {
	tgt_list_item_t* item;
	unsigned int id = 1;

	if (! targets || ! ascii ) {
		return 0;
	}

	for ( item = targets->head; item; item = item->next, id++ ) {
		if (! strcmp( ascii, item->ascii ) ) {
			return id;
		}
	}
	return 0;
}

tgt_list_item_t * tgt_list_find( tgt_list_t targets, unsigned int id ) {
	tgt_list_item_t* item;
	unsigned int curr_id = 1;

	if (! targets ) {
		return NULL;
	}

	for ( item = targets->head; item; item = item->next, curr_id++ ) {
		if ( id == curr_id ) {
			return item;
		}
	}

	return NULL;
}

/* return the data for the specified target ID */
opdis_buf_t tgt_list_data( tgt_list_t targets, unsigned int id ) {
	tgt_list_item_t * item = tgt_list_find( targets, id );
	if ( item ) {
		return item->data;
	}

	return NULL;
}

/* return the name for the specified target ID */
const char * tgt_list_ascii( tgt_list_t targets, unsigned int id ) {
	tgt_list_item_t * item = tgt_list_find( targets, id );
	if ( item ) {
		return item->ascii;
	}

	return NULL;
}

bfd * tgt_list_bfd( tgt_list_t targets, unsigned int id ) {
	tgt_list_item_t* item;
	unsigned int curr_id = 1;

	if (! targets ) {
		return NULL;
	}

	for ( item = targets->head; item; item = item->next, curr_id++ ) {
		if ( id == curr_id ) {
			return item->tgt_bfd;
		}
	}

	return NULL;
}

/* invoke callback for every item in list */
void tgt_list_foreach( tgt_list_t targets, TGT_LIST_FOREACH_FN fn, void * arg ){
	tgt_list_item_t* item;
	unsigned int id = 1;

	if (! targets || ! fn ) {
		return;
	}

	for ( item = targets->head; item; item = item->next, id++ ) {
		fn( item, id, arg );
	}
}

static void print_item( tgt_list_item_t * item, unsigned int id, void * arg ) {
	int i, max;
	const char *bfd_str = "", *etc = "";
	FILE * f = (FILE *) arg;
	if (! f ) {
		return;
	}

	fprintf( f, "\t%d\t", id );
	if ( item->tgt_bfd ) {
		bfd_str = " [BFD]";
	}

	switch (item->type) {
		case tgt_bytes:
			fprintf( f, "Byte String of %d bytes: ", 
				 (unsigned int) item->data->len );

			max = (item->data->len > 8) ? 8 : item->data->len;
			etc = (item->data->len > 8) ? "..." : etc;
			for ( i = 0; i < max; i++ ) {
				fprintf( f, "%02X ", item->data->data[i] );
			}
			fprintf( f, "%s%s\n", etc, bfd_str );
			break;
		case tgt_file:
			fprintf( f, "File '%s'%s\n", item->ascii, bfd_str );
			break;
		default:
			fprintf( f, "Unknown target type for '%s'\n", 
				 item->ascii );
	}
}	

/* print target list to f */
void tgt_list_print( tgt_list_t targets, FILE * f ) {
	tgt_list_foreach( targets, print_item, f );
}
