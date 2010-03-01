/* asm_format.h
 * Copyright (c) 2010 ThoughtGang
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef ASM_FORMAT_H
#define ASM_FORMAT_H

#include <stdio.h>

#include <opdis/model.h>

enum asm_format_t {
	asmfmt_custom,
	asmfmt_asm,
	asmfmt_dump,
	asmfmt_delim,
	asmfmt_xml
};

int asm_fprintf_header( FILE * f, enum asm_format_t fmt );

int asm_fprintf_footer( FILE * f, enum asm_format_t fmt );

int asm_fprintf_insn( FILE * f, enum asm_format_t fmt, const char * fmt_str, 
		      opdis_insn_t * insn );
#endif
