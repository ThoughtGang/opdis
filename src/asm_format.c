/* asm_format.c
 * Copyright (c) 2010 ThoughtGang
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <string.h>

#include "asm_format.h"

int asm_fprintf( FILE * f, enum asm_format_t fmt, const char * fmt_str,
		 opdis_insn_t * insn ) {
	int rv = 0;
	switch (fmt) {
		case asmfmt_dump:
		case asmfmt_delim:
		case asmfmt_xml:
		case asmfmt_asm:
			rv = fprintf( f, "%s", insn->ascii );
			if (! strchr( insn->ascii, '#' ) ) {
				fprintf( f, "\t#" );
			}
			fprintf( f, " [%p]\n", (void *) insn->vma );
			break;
		case asmfmt_custom:
			break;

	}
	return rv;
}
