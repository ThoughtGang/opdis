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
/* UTILITY ROUTINES */

static inline void rtrim( char * str ) {
	int i;
	for ( i = strlen(str) - 1; i >= 0 && isspace(str[i]); i++ )
		str[i] = '\0';
}
/* ---------------------------------------------------------------------- */
/* MNEMONICS */

static const char * jcc_insns[] = {
	"ja", "jae", "jb", "jbe", "jc", "jcxz", "jecxz", 
	"jrcxz", "je", "jg", "jge", "jl", "jle", "jna", "jnae", "jnb", "jnbe", 
	"jnc", "jne", "jng", "jnge", "jnl", "jnle", "jno", "jnp", "jns", "jnz",
	"jo", "jp", "jpe", "js", "jz"
};

static const char * call_insns[] = { "lcall", "call", "callq" };

static const char * jmp_insns[] = { "jmp", "ljmp", "jmpq" };

static const char * ret_insns[] = {
	"ret", "lret", "retq", "retf", "iret", "iretd", "iretq"
};

static void decode_intel_mnemonic( opdis_insn_t * out, const char * item ) {
	int i, num;

	if ( item[0] == 'f' ) {
		// fpu insn
		return;
	}

	/* detect JMP */
	num = (int) sizeof(jmp_insns) / sizeof(char *);
	for ( i = 0; i < num; i++ ) {
		if (! strcmp(jmp_insns[i], item) ) {
			// set jmp insn info
			return;
		}
	}

	/* detect RET */
	num = (int) sizeof(ret_insns) / sizeof(char *);
	for ( i = 0; i < num; i++ ) {
		if (! strcmp(ret_insns[i], item) ) {
			// set ret insn info
			return;
		}
	}

	/* detect branch (call/jcc) */
	num = (int) sizeof(call_insns) / sizeof(char *);
	for ( i = 0; i < num; i++ ) {
		if (! strcmp(call_insns[i], item) ) {
			// set branch insn info
			return;
		}
	}
	num = (int) sizeof(jcc_insns) / sizeof(char *);
	for ( i = 0; i < num; i++ ) {
		if (! strcmp(jcc_insns[i], item) ) {
			// set branch insn info
			return;
		}
	}
}

typedef void (*MNEMONIC_DECODE_FN) ( opdis_insn_t *, const char * );
static int decode_mnemonic( opdis_insn_t * insn, MNEMONIC_DECODE_FN decode_fn, 
			    const char * item ) {
	opdis_insn_set_mnemonic( insn, item );

	rtrim( insn->mnemonic );
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

static void make_relative_operand( opdis_op_t * op ) {
	// TODO
}

static void fix_rel_operands( opdis_insn_t * insn ) {
	const opdis_byte_t * start, * max;

	if ( insn->category != opdis_insn_cat_cflow ) {
		return ;
	}

	for ( start = insn->bytes, max = insn->bytes + insn->size; 
	      start < max && is_prefix_byte(*start); start++ ) 
		;

	if ( start >= max ) {
		return;
	}

	if ( insn->flags.cflow == opdis_cflow_flag_jmpcc ) {
		/* All JCC instructions are relative. The bytes are
		 * 0xE3, 0x70-7F, and 0x0F 0x80-8F. */
		make_relative_operand( insn->operands[0] );
	} else if ( insn->flags.cflow == opdis_cflow_flag_call ) {
		if ( *start == 0xE8 ) {
			make_relative_operand( insn->operands[0] );
		}
	} else if ( insn->flags.cflow == opdis_cflow_flag_jmp ) {
		if ( *start == 0xE9 || * start == 0xEB ) {
			make_relative_operand( insn->operands[0] );
		}
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

static enum opdis_reg_cat_t lookup_register_type( unsigned int id ) {
	enum opdis_reg_cat_t type = opdis_reg_cat_unknown;

	if ( id == 5 ) {
		type = opdis_reg_cat_gen | opdis_reg_cat_stack;
	} else if ( id == 6 ) {
		type = opdis_reg_cat_gen | opdis_reg_cat_frame;
	} else if ( id <= 16 ) {
		type = opdis_reg_cat_gen;
	} else if ( id >= 17 && id <= 24 ) {
		type = opdis_reg_cat_fpu | opdis_reg_cat_sse;
	} else if ( id >= 25 && id <= 32 || id == 61 ) {
		type = opdis_reg_cat_sse;
	} else if ( id >= 33 && id <= 40 ) {
		type = opdis_reg_cat_task;
	} else if ( id >= 41 && id <= 48 ) {
		type = opdis_reg_cat_debug;
	} else if ( id >= 49 && id <= 54 ) {
		type = opdis_reg_cat_gen | opdis_reg_cat_seg;
	} else if ( id == 55 ) {
		type = opdis_reg_cat_pc;
	} else if ( id == 56 ) {
		type = opdis_reg_cat_flags;
	} else if ( id >= 57 && id <= 60 ) {
		type = opdis_reg_cat_mem;
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

static void fill_register_by_id(opdis_reg_t * reg, int id, const char *name) {
	if ( id > -1 ) {
		reg->category = lookup_register_type(id);
		reg->id = intel_reg_id[id];
		reg->size = intel_reg_size[id];
	} else {
		reg->category = opdis_reg_cat_unknown;
		reg->id = reg->size = 0;
	}
	strncpy( reg->ascii, name, OPDIS_REG_NAME_SZ - 1 );
}

static void fill_register( opdis_reg_t * reg, const char * name ) {
	int id = intel_register_lookup(name);
	return fill_register_by_id(reg, id, name);
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

static void fill_absolute( opdis_abs_addr_t * abs, const char * item,
			   const char * colon ) {
	int i;
	char buf[OPDIS_REG_NAME_SZ];
	int seg = intel_register_lookup( item );

	for ( i = 0; item + i < colon && item[i] != ':' ; i++ ) {
		buf[i] = item[i];
	}
	buf[i] = '\0';

	fill_register( &abs->segment, buf );
	fill_immediate( &abs->offset, colon + 1 );
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
}

/* ---------------------------------------------------------------------- */
/* SHARED DECODING */

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
	for ( i = parse->pfx; i > -1 && i < max_i; i++ ) {
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
		// TODO: set flags ?
		opdis_insn_add_comment( out, "Warning: prefix w/o insn" );
	} else if ( parse->mnem > -1 && in->items[parse->mnem][0] == '.' ) {
		// TODO: set flags ?
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

static int att_register_for_token( const char * tok ) {
	char buf[OPDIS_REG_NAME_SZ];
	int i;
	if (! tok ) {
		return -1;
	}

	tok++; 		/* move past '%' */

	for ( i=0; tok[i] && tok[i] != ',' && tok[i] != ')' && tok[i] != ':'; 
	      i++ ) {
		buf[i] = tok[i];
	}
	buf[i] = '\0';

	return intel_register_lookup( buf );
}


static void fill_att_expression( opdis_addr_expr_t *expr, const char * item,
			     const char * first_paren ) {
	int seg = -1, base = -1, index = -1, scale = 1, tok_count = 0;
	long long int displacement;
	const char * ptr, * tok, * end_paren = strchr( item, ')' );
	const char * base_tok = NULL, * index_tok = NULL, * scale_tok = NULL;
	end_paren = (end_paren == NULL) ? item + strlen(item) -1 : end_paren;

	if ( first_paren != item ) {
		const char * col = strchr( item, ':' );
		if ( col != NULL ) {
			fill_absolute( &expr->displacement.a, &item[1], col );
		} else {
			fill_immediate( &expr->displacement.u, item );
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

	base = att_register_for_token( base_tok );
	index = att_register_for_token( index_tok );
	if ( scale_tok ) {
		scale = strtol( scale_tok, NULL, 0 );
	}

	// TODO
	// fill addr expr scale-index-base
}

static void decode_att_operand( opdis_op_t * out, const char * item ) {
	const char * start;
	switch ( item[0] ) {
		case '$':			/* immediate value */
			out->category = opdis_op_cat_immediate;
			fill_immediate( &out->value.immediate.u, &item[1] );
			return;
		case '%':			/* CPU register */
			out->category = opdis_op_cat_register;
			fill_register( &out->value.reg, &item[1] );
			return;
		case '*':			/* absolute jump/call addr */
			out->flags |= opdis_op_cat_register;
			decode_att_operand( out, &item[1] );
			return;
		default:
			start = strchr( item, '(' );
			if ( start ) {
				out->flags |= opdis_op_cat_expr;
				fill_att_expression(&out->value.expr, item, 
						    start);
			} else {
				start = strchr( item, ',' );
				if ( start ) {
					out->flags |= opdis_op_cat_absolute;
					fill_absolute( &out->value.abs,
						       item, start );
				} else {
					/* assume all other values are
					 * addresses, not immediates */
					out->flags |= opdis_op_cat_address;
					fill_immediate( &out->value.immediate.u,
							item );
				}
			}
	}
}

int opdis_x86_att_decoder( const opdis_insn_buf_t in, opdis_insn_t * out,
		           const opdis_byte_t * buf, opdis_off_t offset,
			   opdis_vma_t vma, opdis_off_t length ) {

	int i, max_i, rv;
	struct INSN_BUF_PARSE parse;

	rv = opdis_default_decoder( in, out, buf, offset, vma, length, NULL );

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

		decode_mnemonic( out, decode_att_mnemonic, mnem );
	}

	/* fill operands */
	for ( i = parse.first_op; i > -1 && i < parse.last_op; i++ ) {
		if ( in->items[i][0] != ',' ) {
			decode_operand( opdis_insn_next_avail_op(out),
					decode_att_operand, in->items[i] );
		}
	}

	fix_rel_operands( out );

	add_comments( in, out, & parse );

	/* set operand pointers */
	if ( out->category == opdis_insn_cat_cflow ) {
		if ( out->num_operands > 0 &&
		     (out->flags.cflow >= opdis_cflow_flag_call &&
		      out->flags.cflow <= opdis_cflow_flag_jmpcc ) ) {
			out->target = out->operands[0];
		}
	} else if ( out->num_operands > 0 ) {
		// TODO : exceptions such as bound, invlpga, 2-imm-insn,
		//        and non-commutative FPU insns
		out->src = out->operands[0];
		if ( out->num_operands > 1 ) {
			out->dest = out->operands[1];
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

	// replace with substr PTR
	if (! strncmp( "BYTE PTR", item, 8 ) ||
	    ! strncmp( "WORD PTR", item, 8 ) ||
	    ! strncmp( "DWORD PTR", item, 9 ) ||
	    ! strncmp( "QWORD PTR", item, 9 ) ) {
		return 1;
	}

	return 0;
}

static void fill_intel_expression( opdis_addr_expr_t *expr, const char * item,
				   const char * first_paren ) {
	// 	find ]
	// 	tokenize by +-*
	// 	scale = int, base is first, index is * scale
}

static void decode_intel_operand( opdis_op_t * out, const char * item ) {
	const char * start;
	int reg = intel_register_lookup( item );
	if ( reg != -1 ) {
		fill_register_by_id( &out->value.reg, reg, item );
		return;
	}

	start = strchr( item, '[' );
	if ( start != NULL ) {
		fill_intel_expression( &out->value.expr, item, start);
		return;
	}

	start = strstr( item, "PTR" );
	if ( start != NULL ) {
		// do something;
		return;
	}

	start = strchr( item, ':' );
	if ( start != NULL ) {
		// do seg:offset
		return;
	}

	out->value.immediate.s = strtoll( item, NULL, 0 );
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
		decode_mnemonic( out, decode_intel_mnemonic, 
				 in->items[parse.mnem] );
	}

	/* fill operands */
	for ( i = parse.first_op; i > -1 && i < parse.last_op; i++ ) {
		if ( in->items[i][0] != ',' ) {
			decode_operand( opdis_insn_next_avail_op(out),
					decode_intel_operand, in->items[i] );
		}
	}

	fix_rel_operands( out );

	add_comments( in, out, & parse );

	/* set operand pointers */
	if ( out->category == opdis_insn_cat_cflow ) {
		if ( out->num_operands > 0 &&
		     (out->flags.cflow >= opdis_cflow_flag_call &&
		      out->flags.cflow <= opdis_cflow_flag_jmpcc ) ) {
			out->target = out->operands[0];
		}
	} else if ( out->num_operands > 0 ) {
		out->dest = out->operands[0];
		if ( out->num_operands > 1 ) {
			out->src = out->operands[1];
		}
	}
	
	out->status |= (opdis_decode_basic | opdis_decode_mnem | 
			opdis_decode_ops | opdis_decode_mnem_flags |
			opdis_decode_op_flags);

	return rv;
}
