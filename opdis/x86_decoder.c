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

static const char * intel_prefixes[] = {
	"lock", "addr16", "addr32", "rep", "repe", "repz", "repne", "repnz",
	"cs", "ss", "ds", "es", "fs", "gs", "pt", "pn"
};

static int intel_prefix_lookup( const char * item ) {
	int i;
	int num = (int) sizeof(intel_prefixes) / sizeof(char *);
	for ( i = 0; i < num; i++ ) {
		if (! strcmp(intel_prefixes[i], item) ) {
			return i;
		}
	}

	return -1;
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
	} else if ( intel_prefix_lookup(in->items[parse->mnem]) > -1 ) {
		/* verify that this is not a prefix */
		parse->pfx = parse->mnem;
		parse->mnem = 0;
	}

	//print_parsed_insn( in, parse );
}

static void add_prefixes( const opdis_insn_buf_t in, opdis_insn_t * out,
			  struct INSN_BUF_PARSE * parse ) {
	int i, max_i, rv;

	/* fill prefixes */
	max_i = (parse->mnem > -1) ? parse->mnem : in->item_count;
	for ( i = parse->pfx; i < max_i; i++ ) {
		opdis_insn_add_prefix( out, in->items[i] );
	}
}

static void add_comments( const opdis_insn_buf_t in, opdis_insn_t * out,
			  struct INSN_BUF_PARSE * parse ) {
	int i;

	for ( i = parse->cmt; i > -1 && i < in->item_count; i++ ) {
		opdis_insn_add_comment( out, in->items[i] );
	}

	if ( parse->pfx > -1 && parse->mnem == -1 ) {
		// TODO: set flags
		opdis_insn_add_comment( out, "Warning: prefix w/o insn" );
	} else if ( parse->mnem > -1 && in->items[parse->mnem][0] == '.' ) {
		// TODO: set flags
		opdis_insn_add_comment( out, "Warning: directive (data)" );
	}
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

static void decode_att_mnemonic( opdis_insn_t * out, const char * item ) {
}

static void decode_att_operand( opdis_insn_t * out, const char * item ) {
}

int opdis_x86_att_decoder( const opdis_insn_buf_t in, opdis_insn_t * out,
		           const opdis_byte_t * buf, opdis_off_t offset,
			   opdis_vma_t vma, opdis_off_t length ) {

	int i, max_i, rv;
	struct INSN_BUF_PARSE parse;

	rv = opdis_default_decoder( in, out, buf, offset, vma, length, NULL );
	//printf( "INSN %s (ATT)\n", in->string );

	parse_insn_buf( in, is_att_operand, & parse );

	add_prefixes( in, out, & parse );

	/* fill instruction info */
	if ( parse.mnem > -1 ) {
		const char * mnem = in->items[parse.mnem];
		/* check for branch hint */
		const char * comma = strchr(mnem, ',');
		if ( comma ) {
			char buf[16] = {0};
			strncpy( buf, mnem, 
				 (comma - mnem > 15) ? 15 : comma - mnem );
			opdis_insn_add_prefix( out, buf );
			mnem = comma + 1;
		}

		decode_att_mnemonic( out, mnem );
	}

	/* fill operands */
	for ( i = parse.first_op; i > -1 && i < parse.last_op; i++ ) {
		if ( in->items[i][0] != ',' ) {
			decode_att_operand( out, in->items[i] );
		}
	}

	add_comments( in, out, & parse );

	/* set operand pointers */

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

static void decode_intel_mnemonic( opdis_insn_t * out, const char * item ) {
}

static void decode_intel_operand( opdis_insn_t * out, const char * item ) {
}

int opdis_x86_intel_decoder( const opdis_insn_buf_t in, opdis_insn_t * out,
		             const opdis_byte_t * buf, opdis_off_t offset,
			     opdis_vma_t vma, opdis_off_t length ) {

	struct INSN_BUF_PARSE parse;
	int i, max_i, rv;
	rv = opdis_default_decoder( in, out, buf, offset, vma, length, NULL );

	parse_insn_buf( in, is_intel_operand, & parse );

	add_prefixes( in, out, & parse );

	/* fill instruction info */
	if ( parse.mnem > -1 ) {
		const char * mnem = in->items[parse.mnem];
		/* check for branch hint */
		const char * comma = strchr(mnem, ',');
		if ( comma ) {
			char buf[16] = {0};
			strncpy( buf, mnem, 
				 (comma - mnem > 15) ? 15 : comma - mnem );
			opdis_insn_add_prefix( out, buf );
			mnem = comma + 1;
		}

		decode_intel_mnemonic( out, mnem );
	}

	/* fill operands */
	for ( i = parse.first_op; i > -1 && i < parse.last_op; i++ ) {
		if ( in->items[i][0] != ',' ) {
			decode_intel_operand( out, in->items[i] );
		}
	}

	add_comments( in, out, & parse );

	/* set operand pointers */
	
	// out->status |= opdis_decode_basic;

	return rv;
}
