/*
 * disasm_cflow.c
 * 
 */

#include <stdio.h>

#include <opdis/opdis.h>

int main( int argc, char ** argv ) {
	FILE * f;

	if ( argc < 3 ) {
		printf( "Usage: %s file offset", argv[0] );
		return 1;
	}

	f = fopen( argv[1], "r" );
	// read
	fclose( f );
	// TODO : implement!
	
	return 0;
}
