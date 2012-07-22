/*!
 * \file opdis.c
 * \brief Disassembler front-end for libopcodes
 * \author TG Community Developers <community@thoughtgang.org>
 * \note Copyright (c) 2010 ThoughtGang.
 * Released under the GNU Lesser Public License (LGPL), version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <opdis/opdis.h>


/* ---------------------------------------------------------------------- */
/* MNEMONICS */

static void set_isa( opdis_insn_t * out, const char * item ) {

	/* check for obvious subsets */
	if ( item[0] == 'f' ) {
		out->isa = opdis_insn_subset_fpu;
		return;
	}

	if ( strstr( item, "pd" ) || strstr( item, "ps" ) ||
	     strstr( item, "ss" ) || strstr( item, "sd" ) ) {
		out->isa = opdis_insn_subset_simd;
		return;
	}

	if ( item[0] == 'p' && strncmp( "pause", item, 5 ) &&
	     strncmp( "pop", item, 3 ) && strncmp( "push", item, 4 ) &&
	     strncmp( "prefetch", item, 8 ) ) {
		out->isa = opdis_insn_subset_simd;
		return;
	}

	out->isa = opdis_insn_subset_gen;
}

static void decode_intel_mnemonic( opdis_insn_t * out, const char * item ) {
	int i, num;

	set_isa( out, item );

	/* detect NOP */
	if (! strcmp( "nop", item ) || ! strcmp( "fnop", item ) ) {
		out->category = opdis_insn_cat_nop;
		return;
	}

	/* detect JMP */
	if (! strncmp( "jmp", item, 3 ) || ! strncmp( "ljmp", item, 4) ) {
		out->category = opdis_insn_cat_cflow;
		out->flags.cflow = opdis_cflow_flag_jmp;
		return;
	}

	/* detect RET */
	if (! strncmp( "ret", item, 3 ) || ! strncmp( "lret", item, 4) ||
	    ! strncmp( "iret", item, 4 ) || ! strcmp( "sysexit", item ) ||
	    ! strcmp( "sysret", item ) ) {
		out->category = opdis_insn_cat_cflow;
		out->flags.cflow = opdis_cflow_flag_ret;
		return;
	}

	/* detect branch (call/jcc) */
	if (! strncmp( "call", item, 4 ) || ! strncmp( "lcall", item, 5) ||
	    ! strcmp( "syscall", item ) || ! strcmp("sysenter", item ) ) {
		out->category = opdis_insn_cat_cflow;
		out->flags.cflow = opdis_cflow_flag_call;
		return;
	}
	if ( item[0] == 'j' || ! strncmp( "loop", item, 4) ) {
		/* all mnemonics starting with J are either JMP or Jcc */
		out->category = opdis_insn_cat_cflow;
		out->flags.cflow = opdis_cflow_flag_jmpcc;
		return;
	}

	/* stack instructions */
	if (! strncmp( "pop", item, 3 ) && strcmp( "popcnt", item ) ) {
		out->category = opdis_insn_cat_stack;
		out->flags.stack = opdis_stack_flag_pop;
		return;
	}
	if (! strncmp( "push", item, 4 ) ) {
		out->category = opdis_insn_cat_stack;
		out->flags.stack = opdis_stack_flag_push;
		return;
	}
	if (! strncmp( "enter", item, 5 ) ) {
		out->category = opdis_insn_cat_stack;
		out->flags.stack = opdis_stack_flag_frame;
		return;
	}
	if (! strncmp( "leave", item, 5 ) ) {
		out->category = opdis_insn_cat_stack;
		out->flags.stack = opdis_stack_flag_unframe;
		return;
	}

	/* load/store instructions */
	if ( strstr( item, "mov" ) || strstr( item, "xch" ) ||
	    ! strncmp( "lod", item, 3 ) || ! strncmp( "sto", item, 3 ) || 
	    ! strncmp( "fild", item, 4 ) || ! strncmp( "fist", item, 4 ) ||
	    ! strncmp( "fld", item, 3 ) || ! strncmp( "fst", item, 3 ) ||
	    ! strncmp( "ld", item, 2 ) || ! strncmp( "la", item, 2 ) ||
	    ! strncmp( "ll", item, 2 ) || ! strncmp( "lf", item, 2 ) ||
	    ! strncmp( "lg", item, 2 ) || ! strncmp( "lm", item, 2 ) ||
	    ! strncmp( "mask", item, 4 ) || ! strncmp( "rd", item, 2 ) ||
	    ! strncmp( "sahf", item, 4 ) || ! strncmp( "sg", item, 2 ) ||
	    ! strncmp( "si", item, 2 ) || ! strncmp( "sl", item, 2 ) ||
	    ! strncmp( "sm", item, 2 ) || ! strncmp( "stm", item, 3 ) ||
	    ! strncmp( "str", item, 3 ) || ! strncmp( "swap", item, 4 ) ||
	    ! strncmp( "wrm", item, 3 ) || ! strncmp( "xget", item, 4 ) ||
	    ! strncmp( "xset", item, 4 ) || ! strncmp( "xsave", item, 5 ) ||
	    ! strncmp( "xrstor", item, 6 ) ) {
		out->category = opdis_insn_cat_lost;
		return;
	}

	/* bitwise instructions */
	if (! strncmp( "and", item, 3 ) || ! strncmp( "pand", item, 4 ) ) {
		out->category = opdis_insn_cat_bit;
		out->flags.bit = opdis_bit_flag_and;
		return;
	}
	if (! strncmp( "or", item, 2 ) || ! strncmp( "por", item, 3 ) ) {
		out->category = opdis_insn_cat_bit;
		out->flags.bit = opdis_bit_flag_or;
		return;
	}
	if (! strncmp( "xor", item, 3 ) || ! strncmp( "pxor", item, 4 ) ) {
		out->category = opdis_insn_cat_bit;
		out->flags.bit = opdis_bit_flag_xor;
		return;
	}
	if (! strncmp( "neg", item, 3 ) || ! strncmp( "not", item, 3 ) ) {
		out->category = opdis_insn_cat_bit;
		out->flags.bit = opdis_bit_flag_not;
		return;
	}
	if (! strncmp( "sal", item, 3 ) ) {
		out->category = opdis_insn_cat_bit;
		out->flags.bit = opdis_bit_flag_asl;
		return;
	}
	if (! strncmp( "sar", item, 3 ) || ! strncmp( "psra", item, 4 ) ) {
		out->category = opdis_insn_cat_bit;
		out->flags.bit = opdis_bit_flag_asr;
		return;
	}
	if (! strncmp( "shl", item, 3 ) || ! strncmp( "psll", item, 4 ) ) {
		out->category = opdis_insn_cat_bit;
		out->flags.bit = opdis_bit_flag_lsl;
		return;
	}
	if (! strncmp( "shr", item, 3 ) || ! strncmp( "psrl", item, 4 ) ) {
		out->category = opdis_insn_cat_bit;
		out->flags.bit = opdis_bit_flag_lsr;
		return;
	}
	if (! strncmp( "rcl", item, 3 ) ) {
		out->category = opdis_insn_cat_bit;
		out->flags.bit = opdis_bit_flag_rcl;
		return;
	}
	if (! strncmp( "rcr", item, 3 ) ) {
		out->category = opdis_insn_cat_bit;
		out->flags.bit = opdis_bit_flag_rcr;
		return;
	}
	if (! strncmp( "rol", item, 3 ) ) {
		out->category = opdis_insn_cat_bit;
		out->flags.bit = opdis_bit_flag_rol;
		return;
	}
	if (! strncmp( "ror", item, 3 ) ) {
		out->category = opdis_insn_cat_bit;
		out->flags.bit = opdis_bit_flag_ror;
		return;
	}

	/* trap */
	if (! strncmp( "int", item, 3 ) || ! strcmp( "cli", item ) ||
	    ! strcmp( "sti", item ) || ! strcmp( "ud2", item ) ) {
		out->category = opdis_insn_cat_trap;
		return;
	}

	/* test */
	if ( strstr( item, "cmp" ) || strstr( item, "test" ) ||
	     strstr( item, "com" ) || strstr( item, "min" ) ||
	     strstr( item, "max" ) || ! strncmp( "mps", item, 3 ) ||
	    ! strncmp( "bt", item , 2 ) || ! strncmp( "ftst", item, 4 ) ) {
		out->category = opdis_insn_cat_test;
		return;
	}

	/* math */
	if ( strstr( item, "add" ) || strstr( item, "sub" ) ||
	     strstr( item, "div" ) || strstr( item, "mul" ) ||
	     strstr( item, "cos" ) || strstr( item, "sin" ) ||
	     strstr( item, "sqrt" ) || strstr( item, "abs" ) ||
	     strstr( item, "avg" ) || ! strncmp( "rou", item, 3 ) ||
	    ! strncmp( "inc", item, 3 ) || ! strncmp( "dec", item, 3 ) ||
	    ! strncmp( "adc", item, 3 ) || ! strncmp( "fp", item, 2 ) ||
	    ! strncmp( "fy", item, 2 ) || ! strncmp( "f2", item, 2 ) ||
	    ! strncmp( "dp", item, 2 ) || ! strncmp( "rcp", item, 2 ) ||
	    ! strncmp( "lea", item, 3 ) || ! strncmp( "fscale", item, 6 ) ||
	    ! strncmp( "psad", item, 4 ) ) {
		out->category = opdis_insn_cat_math;
		return;
	}

	/* system instructions */
	if (! strncmp( "inv", item, 3 ) || ! strncmp( "halt", item, 4 ) ||
	    ! strncmp( "clts", item, 4 ) || ! strncmp( "ltr", item, 3 ) ||
	    ! strncmp( "rsm", item, 3 ) || ! strncmp( "wbinvd", item, 6 ) ) {
		out->category = opdis_insn_cat_priv;
		return;
	}

	/* i/o instructions */
	if (! strncmp( "in", item, 2 ) ) {
		out->category = opdis_insn_cat_io;
		out->flags.io = opdis_io_flag_in;
		return;
	}
	if (! strncmp( "out", item, 3 ) ) {
		out->category = opdis_insn_cat_io;
		out->flags.io = opdis_io_flag_out;
		return;
	}
}

typedef void (*MNEMONIC_DECODE_FN) ( opdis_insn_t *, const char * );
static void decode_mnemonic( opdis_insn_t * insn, MNEMONIC_DECODE_FN decode_fn, 
			    const char * item ) {
	int i;
	char buf[64];
	for ( i=0; item[i] && ! isspace(item[i]); i++ ) {
		buf[i] = item[i];
	}
	buf[i] = '\0';

	opdis_insn_set_mnemonic( insn, buf );

	decode_fn( insn, item );
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

static int is_prefix_byte( opdis_byte_t b ) {
	switch (b) {
		case 0xF0: case 0xF2: case 0xF3:
		case 0x26: case 0x2E: case 0x36: case 0x3E:
		case 0x64: case 0x65: case 0x66: case 0x67:
			return 1; break;
		default:
			return 0;
	}
}

/* ---------------------------------------------------------------------- */
/* CPU REGISTERS */

static char intel_reg_id[] = {
	1, 2, 3, 4, 1, 2, 3, 4, 	// al, cl, dl, bl, ah, ch, dh, bh
	1, 2, 3, 4, 5, 6, 7, 8, 	// ax, cx, dx, bx, sp, bp, si, di
	1, 2, 3, 4, 5, 6, 7, 8, 	// eax,ecx,edx,ebx,esp,ebp,esi,edi
	1, 2, 3, 4, 5, 6, 7, 8, 	// rax,rcx,rdx,rbx,rsp,rbp,rsi,rdi
	9, 10, 11, 12, 13, 14, 15, 16, 	// r8 - r15
	9, 10, 11, 12, 13, 14, 15, 16, 	// r8l - r15l
	9, 10, 11, 12, 13, 14, 15, 16, 	// r8w - r15w
	9, 10, 11, 12, 13, 14, 15, 16, 	// r8d - r15d
	17, 18, 19, 20, 21, 22, 23, 24, // mm0 - mm7
	25, 26, 27, 28, 29, 30, 31, 32,	// xmm0 - xmm7
	17, 18, 19, 20, 21, 22, 23, 24, // st(0) - st(7)
	33, 34, 35, 36, 37, 38, 39, 40, // cr0 - cr7
	41, 42, 43, 44, 45, 46, 47, 48, // dr0 - dr7
	49, 50, 51, 52, 53, 54, 	// cs, ds, ss, es, fs, gs 
	55, 55, 56, 56, 		// eip, rip, eflags, rflags
	5, 6, 7, 8, 			// spl, bpl, sil, dil
	57, 58, 59, 60, 61 		// gdtr, ldtr, idtr, tr, mxcsr
};

static enum opdis_reg_flag_t lookup_register_type( unsigned int id ) {
	enum opdis_reg_flag_t type = opdis_reg_flag_unknown;

	if ( id == 5 ) {
		type = opdis_reg_flag_gen | opdis_reg_flag_stack;
	} else if ( id == 6 ) {
		type = opdis_reg_flag_gen | opdis_reg_flag_frame;
	} else if ( id <= 16 ) {
		type = opdis_reg_flag_gen;
	} else if ( id >= 17 && id <= 24 ) {
		type = opdis_reg_flag_fpu | opdis_reg_flag_simd;
	} else if (( id >= 25 && id <= 32 ) || id == 61 ) {
		type = opdis_reg_flag_simd;
	} else if ( id >= 33 && id <= 40 ) {
		type = opdis_reg_flag_task;
	} else if ( id >= 41 && id <= 48 ) {
		type = opdis_reg_flag_debug;
	} else if ( id >= 49 && id <= 54 ) {
		type = opdis_reg_flag_gen | opdis_reg_flag_seg;
	} else if ( id == 55 ) {
		type = opdis_reg_flag_pc;
	} else if ( id == 56 ) {
		type = opdis_reg_flag_flags;
	} else if ( id >= 57 && id <= 60 ) {
		type = opdis_reg_flag_mem;
	}

	return type;
}

static char intel_reg_size[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 	// al, cl, dl, bl, ah, ch, dh, bh
	2, 2, 2, 2, 2, 2, 2, 2, 	// ax, cx, dx, bx, sp, bp, si, di
	4, 4, 4, 4, 4, 4, 4, 4, 	// eax,ecx,edx,ebx,esp,ebp,esi,edi
	8, 8, 8, 8, 8, 8, 8, 8, 	// rax,rcx,rdx,rbx,rsp,rbp,rsi,rdi
	8, 8, 8, 8, 8, 8, 8, 8, 	// r8 - r15
	1, 1, 1, 1, 1, 1, 1, 1, 	// r8l - r15l
	2, 2, 2, 2, 2, 2, 2, 2, 	// r8w - r15w
	4, 4, 4, 4, 4, 4, 4, 4, 	// r8d - r15d
	8, 8, 8, 8, 8, 8, 8, 8, 	// mm0 - mm7
	16, 16, 16, 16, 16, 16, 16, 16,	// xmm0 - xmm7
	10, 10, 10, 10, 10, 10, 10, 10,	// st(0) - st(7)
	4, 4, 4, 4, 4, 4, 4, 4, 	// cr0 - cr7
	4, 4, 4, 4, 4, 4, 4, 4, 	// dr0 - dr7
	2, 2, 2, 2, 2, 2, 		// cs, ds, ss, es, fs, gs 
	4, 8, 4, 8, 			// eip, rip, eflags, rflags
	1, 1, 1, 1, 			// spl, bpl, sil, dil
	6, 6, 6, 6, 4 			// gdtr, ldtr, idtr, tr, mxcsr
};

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
	"st(0)", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)",
	"cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", "cr7",
	"dr0", "dr1", "dr2", "dr3", "dr4", "dr5", "dr6", "dr7",
	"cs", "ds", "ss", "es", "fs", "gs", 
	"eip", "rip", "eflags", "rflags",
	"spl", "bpl", "sil", "dil", 
	"gdtr", "ldtr", "idtr", "tr", "mxcsr"
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

static void fill_register_by_id(opdis_reg_t * reg, int id) {
	if ( id > -1 ) {
		reg->id = intel_reg_id[id];
		reg->flags = lookup_register_type(reg->id);
		reg->size = intel_reg_size[id];
		strncpy( reg->ascii, intel_registers[id], 
			 OPDIS_REG_NAME_SZ - 1 );
	} else {
		reg->flags = opdis_reg_flag_unknown;
		reg->id = reg->size = 0;
	}
}

static void fill_register( opdis_reg_t * reg, const char * name ) {
	int id = intel_register_lookup(name);
	return fill_register_by_id(reg, id);
}

static int register_for_token( const char * tok ) {
	char buf[OPDIS_REG_NAME_SZ];
	int i;
	if (! tok ) {
		return -1;
	}

	for ( i=0; tok[i] && isalnum(tok[i]); i++ ) {
		buf[i] = tok[i];
	}
	buf[i] = '\0';

	return intel_register_lookup( buf );
}

/* ---------------------------------------------------------------------- */
/* OPERANDS */

static void fill_immediate( uint64_t * imm, const char * item ) {
	if ( *item == '-' ) {
		/* sign extend result */
		*imm = (uint64_t) strtoll( item, NULL, 0 );
	} else {
		*imm = strtoull( item, NULL, 0 );
	}
}

typedef void (*OPERAND_DECODE_FN) ( opdis_op_t *, const char * );
static int decode_operand( opdis_op_t * op, OPERAND_DECODE_FN decode_fn, 
			   const char * item ) {
	if (! op ) {
		return 0;
	}

	op->category = opdis_op_cat_unknown;
	op->flags = opdis_op_flag_none;
	opdis_op_set_ascii( op, item );
	decode_fn( op, item );

	return 1;
}

/* ---------------------------------------------------------------------- */
/* SHARED DECODING */

struct INSN_BUF_PARSE {
	int pfx, mnem, first_op, last_op, cmt, cmt_char;
};

typedef int (*IS_OPERAND_FN) ( const char * );
static void parse_insn_buf( const opdis_insn_buf_t in, IS_OPERAND_FN is_operand,
			    struct INSN_BUF_PARSE * parse ) {

	int i;
	parse->pfx = parse->mnem = parse->first_op = parse->last_op = 
		     parse->cmt = parse->cmt_char = -1;

	for ( i=0; i < in->item_count; i++ ) {
		if ( is_operand(in->items[i]) && parse->cmt_char == -1 ) {
			/* all operands-looking tokens before # are operands */
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
	} else if (parse->mnem == 0 && intel_prefix_lookup(in->items[0]) > -1) {
		/* verify that this is not a prefix */
		parse->pfx = parse->mnem;
		parse->mnem = 0;
	}
}

static void add_prefixes( const opdis_insn_buf_t in, opdis_insn_t * out,
			  struct INSN_BUF_PARSE * parse ) {
	int i, max_i, rv;

	/* fill prefixes */
	max_i = (parse->mnem > -1) ? parse->mnem : in->item_count;
	for ( i = parse->pfx; i > -1 && i < max_i; i++ ) {
		opdis_insn_add_prefix( out, in->items[i] );
	}
}

static void add_comments( const opdis_insn_buf_t in, opdis_insn_t * out,
			  struct INSN_BUF_PARSE * parse ) {
	int i;

	for ( i = parse->cmt; i > -1 && i < in->item_count; i++ ) {
		char * c = in->items[i];
		while ( *c && isspace(*c) )
			c++;
		opdis_insn_add_comment( out, c );
	}

	if ( parse->pfx > -1 && parse->mnem == -1 ) {
		// TODO: set insn status to invalid?
		opdis_insn_add_comment( out, "Warning: prefix w/o insn" );
	} else if ( parse->mnem > -1 && in->items[parse->mnem][0] == '.' ) {
		// TODO: set insn status to invalid?
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
	// TODO: handle mem operand size: b w l q ?
	return decode_intel_mnemonic( out, item );
}



static void fill_att_expression( opdis_addr_expr_t *expr, const char * item,
			     const char * first_paren ) {
	/* format : section:disp(base,index.scale) */
	int seg = -1, base = -1, index = -1, scale = 1, tok_count = 0;
	enum opdis_addr_expr_elem_t flags = 0;
	const char * base_tok = NULL, * index_tok = NULL, * scale_tok = NULL;
	const char * ptr, * tok, * end_paren = strchr( item, ')' );
	end_paren = (end_paren == NULL) ? item + strlen(item) -1 : end_paren;

	if ( first_paren != item ) {
		/* displacement always precedes SIB */
		const char * col = strchr( item, ':' );
		flags |= opdis_addr_expr_disp;
		if ( col != NULL ) {
			/* segment register followed by optional displacement */
			fill_register_by_id( &expr->displacement.a.segment, 
					     register_for_token(&item[1]) );
			if ( col + 1 < first_paren ) {
				fill_immediate( &expr->displacement.a.offset, 
						col + 1 );
			}
			flags |= opdis_addr_expr_disp_abs;
		} else {
			/* displacement */
			fill_immediate( &expr->displacement.u, item );
			if ( item[0] == '-' ) {
				flags |= opdis_addr_expr_disp_s;
			} else {
				flags |= opdis_addr_expr_disp_u;
			}
		}
	}

	for ( ptr = tok = first_paren + 1; ptr < end_paren; ptr++ ) {
		if ( *ptr == ',' ) {
			if (! tok_count ) {
				base_tok = tok;
			} else {
				index_tok = tok;
			}
			tok_count++;
			tok = ptr + 1;
		}
	}

	if (! base_tok ) {
		base_tok = tok;
	} else if (! index_tok && *tok != '1' ) {	/* handle disp(,1) */
		index_tok = tok;
	} else {
		scale_tok = tok;
	}

	if ( base_tok ) {
		base = register_for_token( &base_tok[1] );
	}
	if ( index_tok ) {
		index = register_for_token( &index_tok[1] );
	}
	if ( scale_tok ) {
		scale = strtol( scale_tok, NULL, 0 );
	}

	if ( base != -1 ) {
		fill_register_by_id( &expr->base, base );
		flags |= opdis_addr_expr_base;
	}
	if ( index != -1 ) {
		fill_register_by_id( &expr->index, index );
		flags |= opdis_addr_expr_index;
	}
	expr->scale = scale;
	expr->shift = opdis_addr_expr_asl;
	expr->elements = flags;
}

static void decode_att_operand( opdis_op_t * out, const char * item ) {
	const char * start;
	out->flags = 0;

	switch ( item[0] ) {
		case '$':			/* immediate value */
			out->category = opdis_op_cat_immediate;
			fill_immediate( &out->value.immediate.u, &item[1] );
			out->flags |= opdis_op_flag_r;
			if ( item[1] == '-' ) {
				out->flags |= opdis_op_flag_signed;
			}
			return;
		case '%':			/* CPU register */
			out->category = opdis_op_cat_register;
			fill_register( &out->value.reg, &item[1] );
			return;
		case '*':			/* indirect jump/call addr */
			out->flags |= opdis_op_flag_indirect;
			/* * is followed by either a reg or expr; recurse */
			decode_att_operand( out, &item[1] );
			return;
		default:
			/* All other values are assumed to be addresses */
			out->flags |= opdis_op_flag_address;
			start = strchr( item, '(' );
			if ( start ) {
				out->category = opdis_op_cat_expr;
				fill_att_expression(&out->value.expr, item, 
						    start);
			} else {
				start = strchr( item, ',' );
				if ( start ) {
					out->category = opdis_op_cat_absolute;
					fill_register_by_id( 
						&out->value.abs.segment, 
						register_for_token(item) );
					fill_immediate( &out->value.abs.offset, 
							start );
				} else {
					out->category = opdis_op_cat_immediate;
					fill_immediate( &out->value.immediate.u,
							item );
				}
			}
	}
}

int opdis_x86_att_decoder( const opdis_insn_buf_t in, opdis_insn_t * out,
		           const opdis_byte_t * buf, opdis_off_t offset,
			   opdis_vma_t vma, opdis_off_t length, void * arg ) {

	int i, max_i, rv;
	struct INSN_BUF_PARSE parse = { 0 };

	rv = opdis_default_decoder( in, out, buf, offset, vma, length, NULL );

	parse_insn_buf( in, is_att_operand, & parse );

	add_prefixes( in, out, &parse );

	/* fill instruction info */
	if ( parse.mnem > -1 ) {
		char *c, mnem[32];
		int i;

		/* check for branch hint */
		for ( i = 0, c = in->items[parse.mnem]; 
		      i < 32 && *c && *c != ','; i++, c++ ) {
			mnem[i] = *c;
		}
		mnem[i] = '\0';
		if ( *c == ',' ) {
			char buf[16];

			for ( i = 0, c++; i < 16 && *c && ! isspace(*c); 
			      i++, c++ ) {
				buf[i] = *c;
			}
			buf[i] = '\0';
			opdis_insn_add_prefix( out, buf );
		}

		decode_mnemonic( out, decode_att_mnemonic, mnem );
	}

	/* fill operands */
	for ( i = parse.first_op; i > -1 && i <= parse.last_op; i++ ) {
		if ( in->items[i][0] != ',' ) {
			decode_operand( opdis_insn_next_avail_op(out),
					decode_att_operand, in->items[i] );
		}
	}

	add_comments( in, out, & parse );

	/* set operand pointers */
	if ( out->category == opdis_insn_cat_cflow ) {
		if ( out->num_operands > 0 &&
		     (out->flags.cflow >= opdis_cflow_flag_call &&
		      out->flags.cflow <= opdis_cflow_flag_jmpcc ) ) {
			out->target = out->operands[0];
			out->target->flags |= opdis_op_flag_r | opdis_op_flag_x;
		}
	} else if ( out->num_operands > 0 ) {
		// TODO : insns such as bound, invlpga, 2-imm-insn,
		//        and non-commutative FPU insns are dest,src
		out->src = out->operands[0];
		out->src->flags |= opdis_op_flag_r;
		if ( out->num_operands > 1 ) {
			out->dest = out->operands[1];
			// TODO: find exceptions to this rule and apply them
			out->dest->flags |= opdis_op_flag_w;
		}
	}

	// NOTE: it might be better to set the *_flags status(es) only
	//       when insn and operands have been successfully parsed
	out->status |= (opdis_decode_basic | opdis_decode_mnem | 
			opdis_decode_ops | opdis_decode_mnem_flags |
			opdis_decode_op_flags);

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

	if ( strstr( item, "PTR" ) != NULL ) {
		return 1;
	}

	return 0;
}

static void fill_intel_expression( opdis_addr_expr_t *expr, const char * item,
				   const char * first_paren ) {
	/* format: segment:[base + index * scale + disp] */
	int seg = -1, base = -1, index = -1, scale = 1, tok_count = 0;
	enum opdis_addr_expr_elem_t flags = 0;
	const char * base_tok = NULL, * index_tok = NULL, * scale_tok = NULL,
	      	   * disp_tok = NULL;
	const char * ptr, * tok, * end_paren = strchr( item, ']' );
	end_paren = (end_paren == NULL) ? item + strlen(item) -1 : end_paren;

	if ( first_paren != item ) {
		/* segment always precedes SIP */
		const char * col = strchr( item, ':' );
		if ( col != NULL ) {
			fill_register_by_id( &expr->displacement.a.segment,
					register_for_token( base_tok ) );
			flags |= opdis_addr_expr_disp_abs;
		}
	}

	for ( ptr = tok = first_paren + 1; ptr < end_paren; ptr++ ) {
		if ( *ptr == '+' ) {
			if (! tok_count ) {
				base_tok = tok;
			} else {
				scale_tok = tok;
			}
			tok_count++;
			tok = ptr + 1;
		} else if ( *ptr == '*' ) {
			index_tok = tok;
			tok_count++;
			tok = ptr + 1;
		}
	}

	/* only a single token means either base register or disp */
	if (! base_tok ) {
		if ( isalpha(tok[0]) ) {
			base_tok = tok;
		} else {
			disp_tok = tok;
		}
	} else if ( *(tok-1) == '*' ) {
		/* remaining token is scale if previous byte was '*' */
		scale_tok = tok;
	} else {
		/* remaining token is displacement */
		disp_tok = tok;
	}

	base = register_for_token( base_tok );
	index = register_for_token( index_tok );
	if ( scale_tok ) {
		scale = strtol( scale_tok, NULL, 0 );
	}

	if ( disp_tok ) {
		flags |= opdis_addr_expr_disp_abs;
		fill_immediate( &expr->displacement.u, item );
		if ( item[0] == '-' ) {
			flags |= opdis_addr_expr_disp_s;
		} else {
			flags |= opdis_addr_expr_disp_u;
		}
	}

	if ( base != -1 ) {
		fill_register_by_id( &expr->base, base );
		flags |= opdis_addr_expr_base;
	}
	if ( index != -1 ) {
		fill_register_by_id( &expr->index, index );
		flags |= opdis_addr_expr_index;
	}

	expr->scale = scale;
	expr->shift = opdis_addr_expr_asl;
	expr->elements = flags;
}

static void decode_intel_operand( opdis_op_t * op, const char * item ) {
	const char * start;
	int reg = intel_register_lookup( item );

	op->flags = 0;
	if ( reg != -1 ) {
		op->category = opdis_op_cat_register;
		fill_register_by_id( &op->value.reg, reg );
		return;
	}

	/* default category will be immediate */
	op->category = opdis_op_cat_immediate;

	// NOTE: the byte/word/dword before 'ptr' should be used for op size */
	start = strstr( item, "PTR" );
	if ( start != NULL ) {
		op->flags |= opdis_op_flag_indirect | opdis_op_flag_address;
	}

	start = strchr( item, '[' );
	if ( start != NULL ) {
		op->category = opdis_op_cat_expr;
		fill_intel_expression( &op->value.expr, item, start);
		return;
	}

	start = strchr( item, ':' );
	if ( start != NULL ) {
		op->category = opdis_op_cat_absolute;
		fill_register_by_id( &op->value.abs.segment, 
				     register_for_token(item) );
		fill_immediate( &op->value.immediate.u, start + 1 );
		return;
	}

	fill_immediate( &op->value.immediate.u, item );
	if ( item[0] == '-' ) {
		op->flags |= opdis_op_flag_signed;
	}
}

int opdis_x86_intel_decoder( const opdis_insn_buf_t in, opdis_insn_t * out,
		             const opdis_byte_t * buf, opdis_off_t offset,
			     opdis_vma_t vma, opdis_off_t length, void * arg ) {

	int i, max_i, rv;
	struct INSN_BUF_PARSE parse = { 0 };

	rv = opdis_default_decoder( in, out, buf, offset, vma, length, NULL );

	parse_insn_buf( in, is_intel_operand, & parse );

	add_prefixes( in, out, & parse );

	/* fill instruction info */
	if ( parse.mnem > -1 ) {
		decode_mnemonic( out, decode_intel_mnemonic, 
				 in->items[parse.mnem] );
	}

	/* fill operands */
	for ( i = parse.first_op; i > -1 && i <= parse.last_op; i++ ) {
 		if ( in->items[i][0] != ',' ) {
			decode_operand( opdis_insn_next_avail_op(out),
					decode_intel_operand, in->items[i] );
		}
	}

	add_comments( in, out, & parse );

	/* set operand pointers */
	if ( out->category == opdis_insn_cat_cflow ) {
		if ( out->num_operands > 0 &&
		     (out->flags.cflow >= opdis_cflow_flag_call &&
		      out->flags.cflow <= opdis_cflow_flag_jmpcc ) ) {
			out->target = out->operands[0];
			out->target->flags |= opdis_op_flag_r | opdis_op_flag_x;
		}
	} else if ( out->num_operands > 0 ) {
		out->dest = out->operands[0];
		// TODO: find exceptions to this rule and apply them 
		out->dest->flags |= opdis_op_flag_w;
		if ( out->num_operands > 1 ) {
			out->src = out->operands[1];
			out->src->flags |= opdis_op_flag_r;
		}
	}
	
	out->status |= (opdis_decode_basic | opdis_decode_mnem | 
			opdis_decode_ops | opdis_decode_mnem_flags |
			opdis_decode_op_flags);

	return rv;
}
