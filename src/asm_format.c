/* asm_format.c
 * Copyright (c) 2010 ThoughtGang
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <string.h>

#include "asm_format.h"

int asm_fprintf_header( FILE * f, enum asm_format_t fmt ) {
	int rv = 0;
	switch (fmt) {
		case asmfmt_asm:
		case asmfmt_dump:
		case asmfmt_delim:
		case asmfmt_xml:
		case asmfmt_custom:
			break;

	}
	return rv;
}

int asm_fprintf_footer( FILE * f, enum asm_format_t fmt ) {
	int rv = 0;
	switch (fmt) {
		case asmfmt_asm:
		case asmfmt_dump:
		case asmfmt_delim:
		case asmfmt_xml:
		case asmfmt_custom:
			break;
	}
	return rv;
}

/* ---------------------------------------------------------------------- */
static int dump_insn( FILE * f, opdis_insn_t * insn ) {
	int i, prev_op = 0, rv = 0;

	if ( insn->vma == 0 ) {
		rv += fprintf( f, "0x0:" );
	} else {
		rv += fprintf( f, "%p:", (void *) insn->vma );
	}

	for ( i = 0; i < insn->size && i < 8; i++ ) {
		rv += fprintf( f, " %02X", insn->bytes[i] );
	}

	if ( insn->status == opdis_decode_invalid ) {
		rv += fprintf( f, "(invalid instruction)\n" );
		return;
	}

	/* enforce space for 6 bytes */
	for ( i=rv; i < 36; i++ ) {
		fprintf( f, " " );
	}

	if ( insn->num_prefixes ) {
		rv += fprintf( f, "%s ", insn->prefixes ); 
	}
	rv += fprintf( f, "%s\t", insn->mnemonic );

	for ( i=0; i < insn->num_operands; i++ ) {
		if ( prev_op ) {
			fprintf( f, ", " );
		}

		fprintf( f, "%s", insn->operands[i]->ascii );
		prev_op = 1;
	}

	if ( insn->comment[0] ) {
		rv += fprintf( f, "\t# %s", insn->comment );
	}
	fprintf( f, "\n" );

	/* print additional instruction bytes */
	if ( insn->size > 8 ) {
		char buf[32];
		int sz = sprintf( buf, "%p:", (void *) insn->vma );
		for ( i = 0; i < sz; i++ ) {
			rv += fprintf( f, " " );
		}
		for ( i = 8; i < insn->size; i++ ) {
			rv += fprintf( f, " %02X", insn->bytes[i] );
		}
		rv += fprintf( f, "\n" );
	}

	return rv;
}

static int delim_insn( FILE * f, opdis_insn_t * insn ) {
}

static int xml_operand( FILE * f, opdis_op_t * op ) {
	return 0;
}

static int xml_insn( FILE * f, opdis_insn_t * insn ) {
	// TODO
	return 0;
}

static int custom_insn( FILE * f, const char * fmt_str, opdis_insn_t * insn ) {
	// TODO
	return 0;
}

int asm_fprintf_insn( FILE * f, enum asm_format_t fmt, const char * fmt_str,
		      opdis_insn_t * insn ) {
	int rv = 0;
	switch (fmt) {
		case asmfmt_asm:
			rv += fprintf( f, "%s", insn->ascii );
			if (! strchr( insn->ascii, '#' ) ) {
				rv += fprintf( f, "\t#" );
			}
			rv += fprintf( f, " [%p]\n", (void *) insn->vma );
			break;
		case asmfmt_dump:
			rv = dump_insn( f, insn ); break;
		case asmfmt_delim:
			rv = delim_insn( f, insn ); break;
		case asmfmt_xml:
			rv = xml_insn( f, insn ); break;
		case asmfmt_custom:
			rv = custom_insn( f, fmt_str, insn ); break;

	}
	return rv;
}
