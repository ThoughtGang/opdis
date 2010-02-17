/* target_list.c
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
		free( item );
	}

	free( targets );
}

static opdis_buf_t load_bytes( const char * bytes ) {
	// TODO
	return NULL;
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

/* return the ID of the specified target */
/* note: ID is implicit : it is offset of item in list + 1 */
unsigned int tgt_list_id( tgt_list_t targets, const char * ascii ) {
	tgt_list_item_t* item;
	unsigned int id = 1;

	if (! targets || ! ascii ) {
		return;
	}

	for ( item = targets->head; item; item = item->next, id++ ) {
		if (! strcmp( ascii, item->ascii ) ) {
			return id;
		}
	}
	return 0;
}

/* return the data for the specified target ID */
opdis_buf_t tgt_list_data( tgt_list_t targets, unsigned int id ) {
	tgt_list_item_t* item;
	unsigned int curr_id = 1;

	if (! targets ) {
		return;
	}

	for ( item = targets->head; item; item = item->next, curr_id++ ) {
		if ( id == curr_id ) {
			return item->data;
		}
	}

	return NULL;
}

/* return the name for the specified target ID */
const char * tgt_list_ascii( tgt_list_t targets, unsigned int id ) {
	tgt_list_item_t* item;
	unsigned int curr_id = 1;

	if (! targets ) {
		return;
	}

	for ( item = targets->head; item; item = item->next, curr_id++ ) {
		if ( id == curr_id ) {
			return item->ascii;
		}
	}

	return NULL;
}

bfd * tgt_list_bfd( tgt_list_t targets, unsigned int id ) {
	tgt_list_item_t* item;
	unsigned int curr_id = 1;

	if (! targets ) {
		return;
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
	FILE * f = (FILE *) arg;
	if (! f ) {
		return;
	}

	//TODO
}	

/* print target list to f */
void tgt_list_print( tgt_list_t targets, FILE * f ) {
	tgt_list_foreach( targets, print_item, f );
}
