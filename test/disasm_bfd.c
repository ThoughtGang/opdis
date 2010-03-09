/* disasm_bfd.c
 * Simple program to test disassembly of BFD symbol
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <opdis/opdis.h>

static int print_insn( opdis_insn_t * insn, void * arg ) {
	const char * filename = (const char *) arg;
	printf( "%08X [%s:%X]\t%s\n", insn->vma, filename, insn->offset,
		insn->ascii );
	return 1;
}

static void store_insn( const opdis_insn_t * insn, void * arg ) {
	opdis_insn_tree_t tree = (opdis_insn_tree_t) arg;
	opdis_insn_t * i = opdis_insn_dupe( insn );

	opdis_insn_tree_add( tree, i );
}

static asymbol * find_in_symtab( asymbol ** syms, unsigned int num_syms,
				 const char * name ) {
	unsigned int i;
	symbol_info info;
	asymbol * sym = NULL;

	for ( i = 0; i < num_syms; i++ ) {
		bfd_symbol_info( syms[i], &info );
		if ( strcmp( info.name, name ) ) {
			continue;
		}
		sym = malloc( sizeof(asymbol) );
		if ( sym ) {
			memcpy( sym, syms[i], sizeof(asymbol) );
			break;
		}
	}

	return sym;
}

static asymbol * find_bfd_symbol( bfd * abfd, const char * name ) {
	size_t size;
	unsigned int num;
	asymbol ** syms, *s = NULL;

	if (! bfd_get_file_flags(abfd) & HAS_SYMS ) {
		return;
	}

	/* check dynamic symtab first */
	size = bfd_get_dynamic_symtab_upper_bound( abfd );
	if ( size > 0 ) {
		syms = (asymbol **) malloc(size);
		if ( syms ) {
			num = bfd_canonicalize_dynamic_symtab( abfd, syms );
			s = find_in_symtab( syms, num, name );
			free(syms);
		}
	}
	if ( s ) {
		return s;
	}

	/* check regular symtab */
	size = bfd_get_symtab_upper_bound( abfd );
	if ( size > 0 ) {
		syms = (asymbol **) malloc(size);
		if ( syms ) {
			num = bfd_canonicalize_symtab( abfd, syms );
			s = find_in_symtab( syms, num, name );
			free(syms);
		}
	}

	return s;
}

static int disassemble_file( const char * name, const char * symbol ) {
	int rv;
	opdis_t o;
	bfd * abfd;
	asymbol * sym;
	opdis_insn_tree_t tree;

	abfd = bfd_openr( name, NULL );
	if (! abfd ) {
		printf( "Unable to open file %s: %s\n", name, strerror(errno) );
		return -1;
	}
	if ( bfd_get_flavour( abfd ) == bfd_target_unknown_flavour ) {
		printf( "BFD does not recognize file format for %s\n", name );
		bfd_close( abfd );
		return -2;
	}

	sym = find_bfd_symbol( abfd, symbol );
	if (! sym ) {
		printf( "%s does not contain symbol '%s'\n", name, symbol );
		return -3;
	}

	o = opdis_init_from_bfd( abfd );

	tree = opdis_insn_tree_init( 1 );
	opdis_set_display( o, store_insn, tree );

	rv = opdis_disasm_bfd_symbol( o, sym );
	opdis_insn_tree_foreach( tree, print_insn, (void *) name );

	opdis_insn_tree_free( tree );
	opdis_term( o );

	free( sym );
	bfd_close( abfd );

	return (rv > 0) ? 0 : -2;
}

int main( int argc, char ** argv ) {
	if ( argc < 3 ) {
		printf( "Usage: %s file name\n", argv[0] );
		return 1;
	}

	return disassemble_file( argv[1], argv[2] );
}
