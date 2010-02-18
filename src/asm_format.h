/* asm_format.h
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
