/* disasm_linear.c
 * Simple program to test linear disassembly of a file
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <opdis/opdis.h>

static void display_insn( const opdis_insn_t * insn, void * arg ) {
	const char * filename = (const char *) arg;
	printf( "%08X [%s:%X]\t%s\n", insn->vma, filename, insn->offset,
		insn->ascii );
}

static int disassemble_file( const char * name, opdis_off_t offset,
			     opdis_off_t length ) {
	int rv;
	FILE * f;
	opdis_t o;
	opdis_buf_t buf;

	f = fopen( name, "r" );
	if (! f ) {
		printf( "Unable to open file %s: %s\n", name, strerror(errno) );
		return -1;
	}

	buf = opdis_buf_read( f, 0, 0 );

	fclose( f );

	o = opdis_init();
	opdis_set_display( o, display_insn, (void *) name );

	rv = opdis_disasm_linear( o, buf, (opdis_vma_t) offset, length );

	opdis_term( o );
	
	return (rv > 0) ? 0 : -2;
}

int main( int argc, char ** argv ) {
	opdis_off_t offset = 0;
	opdis_off_t length = 0;

	if ( argc < 2 ) {
		printf( "Usage: %s file [offset [len]]\n", argv[0] );
		return 1;
	} else if ( argc >= 3 ) {
		offset = (opdis_off_t) strtoul( argv[2], NULL, 0 );
		if ( argc >= 4 ) {
			length = (opdis_off_t) strtoul( argv[3], NULL, 0 );
		}
	}
	return disassemble_file( argv[1], offset, length );
}
