/* disasm_linear.c
 * Simple program to test linear disassembly of a file
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <opdis/opdis.h>

int main( int argc, char ** argv ) {
	FILE * f;
	long size;
	opdis_t o;
	opdis_buf_t buf;
	opdis_off_t offset = 0;

	if ( argc < 2 ) {
		printf( "Usage: %s file [offset]", argv[0] );
		return 1;
	} else if ( argc >= 3 ) {
		offset = (opdis_off_t) strtoul( argv[2], NULL, 0 );
	}

	f = fopen( argv[1], "r" );
	if (! f ) {
		printf( "Unable to open file %s: %s\n", argv[1], 
			strerror(errno) );
		return -1;
	}

	buf = opdis_buf_read( f, 0 );

	fclose( f );

	o = opdis_init();
	opdis_disasm_linear( o, buf, offset, 0 );
	opdis_term( o );
	
	return 0;
}
