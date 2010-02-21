/* asm_format.h
 * Copyright (c) 2010 ThoughtGang
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef ASM_FORMAT_H
#define ASM_FORMAT_H

enum asm_format_t {
	asmfmt_custom,
	asmfmt_asm,
	asmfmt_dump,
	asmfmt_delim,
	asmfmt_xml
};

/* returns true if fmt is asm, dump, delim, or xml, or if fmt contains a % */ 
int is_supported_format( const char * fmt );

#endif
