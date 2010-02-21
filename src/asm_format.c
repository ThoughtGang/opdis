/* asm_format.c
 * Copyright (c) 2010 ThoughtGang
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
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
