/* asm_format.c
 */

#include <string.h>

#include "asm_format.h"

int is_supported_format( const char * fmt ) {
	return (! strcmp( "asm", fmt) ||
		! strcmp( "dump", fmt ) ||
		! strcmp( "delim", fmt ) ||
		! strcmp( "xml", fmt ) ||
		strchr( fmt, '%' ) );
}
