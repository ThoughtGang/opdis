/*
 * disasm_cflow.c
 * Simple control-flow disassembler
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <opdis/opdis.h>

static int print_insn( opdis_insn_t * insn, void * arg ) {
	const char * filename = (const char *) arg;
	printf( "%08X [%s:%X]\t%s\n", (unsigned int) insn->vma, filename, 
		(unsigned int) insn->offset, insn->ascii );
	return 1;
}

static void store_insn( const opdis_insn_t * insn, void * arg ) {
	opdis_insn_tree_t tree = (opdis_insn_tree_t) arg;
	opdis_insn_t * i = opdis_insn_dupe( insn );

	opdis_insn_tree_add( tree, i );
}

static int disassemble_file( const char * name, opdis_off_t offset ) {
	int rv;
	FILE * f;
	opdis_t o;
	opdis_buf_t buf;
	opdis_insn_tree_t tree;

	f = fopen( name, "r" );
	if (! f ) {
		printf( "Unable to open file %s: %s\n", name, strerror(errno) );
		return -1;
	}

	buf = opdis_buf_read( f, 0, 0 );

	fclose( f );

	o = opdis_init( NULL );

	tree = opdis_insn_tree_init( 1 );
	opdis_set_display( o, store_insn, tree );

	rv = opdis_disasm_cflow( o, buf, (opdis_vma_t) offset );
	opdis_insn_tree_foreach( tree, print_insn, (void *) name );

	opdis_insn_tree_free( tree );
	opdis_term( o );
	
	return (rv > 0) ? 0 : -2;
}

int main( int argc, char ** argv ) {
	opdis_off_t offset = 0;

	if ( argc < 2 ) {
		printf( "Usage: %s file [offset]\n", argv[0] );
		return 1;
	} else if ( argc >= 3 ) {
		offset = (opdis_off_t) strtoul( argv[2], NULL, 0 );
	}
	return disassemble_file( argv[1], offset );
}
