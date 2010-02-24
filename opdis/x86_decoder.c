/*!
 * \file opdis.c
 * \brief Disassembler front-end for libopcodes
 * \author TG Community Developers <community@thoughtgang.org>
 * \note Copyright (c) 2010 ThoughtGang.
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <opdis/opdis.h>


/* ---------------------------------------------------------------------- */
/* SHARED DECODING */

static const char * intel_registers[] = {
	"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh",
	"ax", "cx", "dx", "bx", "sp", "bp", "si", "di",
	"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
	"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
	"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
	"r8l", "r9l", "r10l", "r11l", "r12l", "r13l", "r14l", "r15l",
	"r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w",
	"r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d",
	"mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7",
	"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
	"st0", "st1", "st2", "st3", "st4", "st5", "st6", "st7",
	"cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", "cr7",
	"dr0", "dr1", "dr2", "dr3", "dr4", "dr5", "dr6", "dr7",
	"cs", "ds", "ss", "es", "fs", "gs", 
	"eip", "rip", "eflags", "rflags",
	"spl", "bpl", "sil", "dil", 
	"gdtr", "ldtr", "idtr"
};

static int intel_register_lookup( const char * item ) {
	int i;
	int num_regs = (int) sizeof(intel_registers) / sizeof(char *);
	for ( i = 0; i < num_regs; i++ ) {
		if (! strcmp(intel_registers[i], item) ) {
			return i;
		}
	}

	return -1;
}

/* ---------------------------------------------------------------------- */
/* AT&T DECODING */

static int is_att_operand( const char * item ) {
	int rv = 0;
	switch ( item[0] ) {
		case '0': case '%': case '$': case '*': case '-': case '(':
			rv = 1; break;
		default:
			break;
	}
	return rv;
}

typedef int (*IS_OPERAND_FN) ( const char * );
struct INSN_BUF_PARSE {
	int pfx, mnem, first_op, last_op, cmt, cmt_char;
};

static void print_parsed_insn_buf( const opdis_insn_buf_t in,
			    struct INSN_BUF_PARSE * parse ) {
	int i;

	if ( parse->pfx > -1 ) {
		printf("Prefix:" );
		for ( i = 0; i <= parse->pfx; i++ ) {
			printf(" '%s'", in->items[i]);
		}
		printf("'|" );
	}
	printf("Mnemonic: '%s'", in->items[parse->mnem] );
	for ( i = parse->first_op; i > -1 && i <= parse->last_op; i++ ) {
		if ( in->items[i][0] != ',' ) {
			printf("|Operand: '%s'", in->items[i]);
		}
	}
	if ( parse->cmt > -1 ) {
		printf("|Comment:" );
		for ( i = parse->cmt; i <= in->item_count; i++ ) {
			printf(" '%s'", in->items[i]);
		}
	}
	printf("\n");
}

static void parse_insn_buf( const opdis_insn_buf_t in, IS_OPERAND_FN is_operand,
			    struct INSN_BUF_PARSE * parse ) {

	int i;
	parse->pfx = parse->mnem = parse->first_op = parse->last_op = 
		     parse->cmt = parse->cmt_char = -1;

	for ( i=0; i < in->item_count; i++ ) {
		if ( is_operand(in->items[i]) && parse->cmt_char == -1 ) {
			/* all operands-looking tokesn before # are operands */
			if ( parse->first_op == -1 ) {
				parse->first_op = i;
				parse->mnem = i - 1;
			}
			parse->last_op = i;
		}

		if ( strchr(in->items[i], '#') != NULL ) {
			parse->cmt_char = i;
			if ( i + 1 < in->item_count ) {
				parse->cmt = i + 1;
			}
		}

	}

	if ( parse->mnem == -1 ) {
		if ( parse->first_op != -1 ) {
			parse->mnem = parse->first_op - 1;
		} else if ( parse->cmt_char != -1 ) {
			parse->mnem = parse->cmt_char - 1;
		} else {
			parse->mnem = in->item_count - 1;
		}
	}

	if ( parse->mnem > 0 ) {
		parse->pfx = 0;
	}

	//print_parsed_insn( in, parse );
}

int opdis_x86_att_decoder( const opdis_insn_buf_t in, opdis_insn_t * out,
		           const opdis_byte_t * buf, opdis_off_t offset,
			   opdis_vma_t vma, opdis_off_t length ) {

	int i;
	int rv;
	struct INSN_BUF_PARSE parse;


	rv = opdis_default_decoder( in, out, buf, offset, vma, length, NULL );
	printf( "INSN %s (ATT)\n", in->string );

	parse_insn_buf( in, is_att_operand, & parse );

	printf("\n");

	// NOTE: prefix w/ no mnemonic is probably invalid!
	// out->status |= opdis_decode_basic;

	return rv;
}

/* ---------------------------------------------------------------------- */
/* INTEL DECODING */

static int is_intel_operand( const char * item ) {
	if ( intel_register_lookup(item) > -1 ) {
		return 1;
	}

	switch ( item[0] ) {
		case '[': case '+': case '-':
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			return 1;
	}

	if (! strncmp( "BYTE PTR", item, 8 ) ||
	    ! strncmp( "WORD PTR", item, 8 ) ||
	    ! strncmp( "DWORD PTR", item, 9 ) ||
	    ! strncmp( "QWORD PTR", item, 9 ) ) {
		return 1;
	}

	return 0;
}

int opdis_x86_intel_decoder( const opdis_insn_buf_t in, opdis_insn_t * out,
		             const opdis_byte_t * buf, opdis_off_t offset,
			     opdis_vma_t vma, opdis_off_t length ) {

	struct INSN_BUF_PARSE parse;
	int rv;
	rv = opdis_default_decoder( in, out, buf, offset, vma, length, NULL );
	printf( "INSN %s (INTEL)\n", in->string );
	printf( "insn_info_valid? %d\n", in->insn_info_valid );
	parse_insn_buf( in, is_intel_operand, & parse );
	printf("\n");
	
	// must do strcmp
	// lock repne repnx rep repe repz cs ss ds es fs gs 
	// addr32 addr16
	// out->status |= opdis_decode_basic;

	return rv;
}
