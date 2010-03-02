/* asm_format.c
 * Copyright (c) 2010 ThoughtGang
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <string.h>

#include "asm_format.h"

#define FPRINTF_ADDR( rv, f, vma ) 				\
	if ( vma == 0 ) {					\
		rv += fprintf( f, "0x0" );			\
	} else {						\
		rv += fprintf( f, "%p", (void *) vma );		\
	}

int asm_fprintf_header( FILE * f, enum asm_format_t fmt ) {
	int rv = 0;
	switch (fmt) {
		/* only delim and XML require headers */
		case asmfmt_delim:
			rv += fprintf( f, "offset|vma|bytes|ascii|prefixes|" );
			rv += fprintf( f, "mnemonic|isa|category|flags|");
			rv += fprintf( f, "op|...\n" );
			break;
		case asmfmt_xml:
			rv += fprintf( f, "<?xml version=\"1.0\"?>\n" );
			rv += fprintf( f, "<!DOCTYPE disassembly [\n" );
			rv += fprintf( f, "<!ELEMENT disassembly " );
			rv += fprintf( f, "(instruction*)>\n" );
			rv += fprintf( f, "<!ELEMENT instruction (offset," );
			rv += fprintf( f, "vma,bytes,ascii?,mnemonic?,");
			rv += fprintf( f, "prefix?,isa?,category?,flags?," );
			rv += fprintf( f, "operands?,invalid?)>\n" );
			rv += fprintf( f, "<!ELEMENT offset (#PCDATA)>\n" );
			rv += fprintf( f, "<!ELEMENT vma (#PCDATA)>\n" );
			rv += fprintf( f, "<!ELEMENT bytes (byte+)>\n" );
			rv += fprintf( f, "<!ELEMENT byte (#PCDATA)>\n" );
			rv += fprintf( f, "<!ELEMENT ascii (#PCDATA)>\n" );
			rv += fprintf( f, "<!ELEMENT mnemonic (#PCDATA)>\n" );
			rv += fprintf( f, "<!ELEMENT prefix (#PCDATA)>\n" );
			rv += fprintf( f, "<!ELEMENT isa (#PCDATA)>\n" );
			rv += fprintf( f, "<!ELEMENT category (#PCDATA)>\n" );
			rv += fprintf( f, "<!ELEMENT flags (flag+)>\n" );
			rv += fprintf( f, "<!ELEMENT flag (#PCDATA)>\n" );
			rv += fprintf( f, "<!ELEMENT operands (operand*)>\n" );
			rv += fprintf( f, "<!ELEMENT operand (ascii," );
			rv += fprintf( f, "category,flags,value)>\n" );
			rv += fprintf( f, "<!ATTLIST operand type " );
			rv += fprintf( f, "(target|src|dest) \"\">\n" );
			rv += fprintf( f, "<!ELEMENT value (register?," );
			rv += fprintf( f, "immediate?,absolute?," );
			rv += fprintf( f, "expression?)>\n" );
			rv += fprintf( f, "<!ELEMENT register (ascii,id," );
			rv += fprintf( f, "size,flags)>\n" );
			rv += fprintf( f, "<!ELEMENT immediate (#PCDATA)>\n" );
			rv += fprintf( f, "<!ELEMENT absolute (segment," );
			rv += fprintf( f, "immediate)>\n" );
			rv += fprintf( f, "<!ELEMENT segment (register)>\n" );
			rv += fprintf( f, "<!ELEMENT expression (base?," );
			rv += fprintf( f, "index?,scale,shift?," );
			rv += fprintf( f, "displacement?)>\n" );
			rv += fprintf( f, "<!ELEMENT base (register)>\n" );
			rv += fprintf( f, "<!ELEMENT index (register)>\n" );
			rv += fprintf( f, "<!ELEMENT scale (#PCDATA)>\n" );
			rv += fprintf( f, "<!ELEMENT shift (#PCDATA)>\n" );
			rv += fprintf( f, "<!ELEMENT displacement " );
			rv += fprintf( f, "(absolute?,immediate?)>\n" );
			rv += fprintf( f, "]>\n" );

			rv += fprintf( f, "<disassembly>\n" );
			break;
		case asmfmt_asm:
		case asmfmt_dump:
		case asmfmt_custom:
			break;

	}
	return rv;
}

int asm_fprintf_footer( FILE * f, enum asm_format_t fmt ) {
	int rv = 0;
	switch (fmt) {
		/* only XML requires a footer */
		case asmfmt_xml:
			rv += fprintf( f, "</disassembly>\n" );
			break;
		case asmfmt_asm:
		case asmfmt_dump:
		case asmfmt_delim:
		case asmfmt_custom:
			break;
	}
	return rv;
}

/* ---------------------------------------------------------------------- */
static int dump_insn( FILE * f, opdis_insn_t * insn ) {
	int i, prev_op = 0, rv = 0;

	FPRINTF_ADDR( rv, f, insn->vma );
	fprintf( f, ":" );

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

static int delim_operand( FILE * f, opdis_op_t * op ) {
	int rv = 0;
	char buf[64];

	/* ascii:cat:flags: */
	rv += fprintf( f, "%s:", op->ascii );
	buf[0] = 0;
	opdis_op_cat_str( op, buf, 64 );
	rv += fprintf( f, "%s:", buf );
	buf[0] = 0;
	opdis_op_flags_str( op, buf, 64, "," );
	rv += fprintf( f, "%s:", buf );

	/* value */
	/* NOTE: value is either a number or an object contained in {} */
	switch (op->category) {
		case opdis_op_cat_register:
			/* {ascii;id;size;flags} */
			buf[0] = '\0';
			opdis_reg_flags_str( &op->value.reg, buf, 64, "," );
			rv += fprintf( f, "{%s;%d;%d;%s}", 
					op->value.reg.ascii,
					op->value.reg.id,
					op->value.reg.size, buf );
			break;
		case opdis_op_cat_absolute:
			/* {segment;offset} */
			rv += fprintf( f, "{%s;%llX}", 
					op->value.abs.segment.ascii,
				 	(long long unsigned int)
					op->value.abs.offset );
			break;
		case opdis_op_cat_expr:
			/* {base;index;scale;op;seg;disp} */
			rv += fprintf( f, "{%s;%s;%d;", 
				 op->value.expr.base.ascii,
				 op->value.expr.index.ascii,
				 op->value.expr.scale );

			buf[0] = '\0';
			opdis_addr_expr_shift_str( &op->value.expr, buf, 64 );
			rv += fprintf( f, "%s;%s;", buf,
					(op->value.expr.elements &
					 opdis_addr_expr_disp_abs) ?
			     op->value.expr.displacement.a.segment.ascii : "" );


			if ( op->value.expr.elements & 
			     opdis_addr_expr_disp_abs )  {
				rv += fprintf( f, "%llX", 
					(long long unsigned int)
					op->value.expr.displacement.a.offset);
			} else if ( op->value.expr.elements &
				    opdis_addr_expr_disp_s ) {
				rv += fprintf( f, "%lld", (long long int)
					op->value.expr.displacement.s);
			} else {
				rv += fprintf( f, "%llX", 
					(long long unsigned int)
					op->value.expr.displacement.u);
			}
			rv += fprintf( f, "}" );
			
			break;
		case opdis_op_cat_immediate:
		case opdis_op_cat_unknown:
			if ( op->flags & opdis_op_flag_signed ) {
				rv += fprintf( f, "%lld", 
						(long long int)
						op->value.immediate.s );
			} else {
				rv += fprintf( f, "%#llX", 
						(long long unsigned int)
						op->value.immediate.u );
			}
			break;
	}

	return rv;
}

#define DELIM( rv, f )	rv += fprintf( f, "|" );
static int delim_insn( FILE * f, opdis_insn_t * insn ) {
	int i, tok_req, rv = 0;
	char buf[64];

	/* offset, address */
	FPRINTF_ADDR( rv, f, insn->offset );
	DELIM( rv, f );
	FPRINTF_ADDR( rv, f, insn->vma );
	DELIM( rv, f );

	/* bytes */
	for ( i = 0, tok_req = 0; i < insn->size; i++ ) {
		if ( tok_req ) {
			rv += fprintf( f, " " );
		}
		rv += fprintf( f, "%02X", insn->bytes[i] );
		tok_req = 1;
	}

	/* ascii, prefix, mnemonic */
	rv += fprintf( f, "|%s|%s|%s|", insn->ascii, insn->prefixes, 
			insn->mnemonic );

	/* isa, cat, flags */
	buf[0] = 0;
	opdis_insn_isa_str( insn, buf, 64 );
	rv += fprintf( f, "%s|", buf );
	buf[0] = 0;
	opdis_insn_cat_str( insn, buf, 64 );
	rv += fprintf( f, "%s|", buf );
	buf[0] = 0;
	opdis_insn_flags_str( insn, buf, 64, "," );
	rv += fprintf( f, "%s|", buf );
	
	/* comment */
	rv += fprintf( f, "%s", insn->comment );

	/* operands */
	for ( i=0; i < insn->num_operands; i++ ) {
		DELIM( rv, f );
		rv += delim_operand( f, insn->operands[i] );
		if ( insn->operands[i] == insn->target ) {
			rv += fprintf( f, ":TARGET" );
		}
		if ( insn->operands[i] == insn->src ) {
			rv += fprintf( f, ":SRC" );
		}
		if ( insn->operands[i] == insn->dest ) {
			rv += fprintf( f, ":DEST" );
		}
	}

	rv += fprintf( f, "\n" );

	return rv;
}

static int xml_flags( FILE * f, char * buf, const char *indent ) {
	int rv = 0;
	char *c, *flag;
	rv += fprintf( f, "%s<flags>\n", indent );
	for ( c = buf, flag = buf; *c; c++ ) {
		if ( *c == ',' ) {
			*c = '\0';
			rv += fprintf( f, "%s  <flag>%s</flag>\n",
					indent, flag );
			flag = c + 1;
		}
	}

	if ( c != buf ) {
		/* handle last flag */
		rv += fprintf( f, "%s  <flag>%s</flag>\n", indent, flag );
	}

	rv += fprintf( f, "%s</flags>\n", indent );
	return rv;
}

static int xml_immediate_s( FILE * f, int64_t val, const char * indent ) {
	return fprintf( f, "%s<immediate>%lld</immediate>\n", indent,
			(long long int) val );
}

static int xml_immediate( FILE * f, uint64_t val, const char * indent ) {
	return fprintf( f, "%s<immediate>%#llX</immediate>\n", indent,
			(unsigned long long int) val );
}

static int xml_register( FILE * f, opdis_reg_t * reg, const char * indent ) {
	int rv = 0;
	char buf[96];
	char indent_buf[24];

	rv += fprintf( f, "%s<register>\n", indent );
	rv += fprintf( f, "%s  <ascii>%s</ascii>\n", indent, reg->ascii );
	rv += fprintf( f, "%s  <id>%d</id>\n", indent, reg->id );
	rv += fprintf( f, "%s  <size>%d</size>\n", indent, reg->size );
	buf[0] = 0;
	opdis_reg_flags_str( reg, buf, 96, "," );
	sprintf( indent_buf, "%s  ", indent );
	rv += xml_flags( f, buf, indent_buf ); 
	rv += fprintf( f, "%s</register>\n", indent );

	return rv;
}

static int xml_abs_addr(FILE * f, opdis_abs_addr_t * abs, const char * indent) {
	int rv = 0;
	char indent_buf[24];

	sprintf( indent_buf, "%s    ", indent );
	rv += fprintf( f, "%s<absolute>\n", indent );
	rv += fprintf( f, "%s  <segment>\n", indent );
	rv += xml_register( f, &abs->segment, indent_buf );
	rv += fprintf( f, "%s  </segment>\n", indent );
	rv += xml_immediate( f, abs->offset, indent_buf );
	rv += fprintf( f, "%s</absolute>\n", indent );
	return rv;
}

static int xml_addr_expr( FILE * f, opdis_addr_expr_t * expr, 
			  const char * indent ) {
	int rv = 0;
	char buf[8], indent_buf[24];

	rv += fprintf( f, "%s<expression>\n", indent );

	/* base */
	if ( (expr->elements & opdis_addr_expr_base) != 0 ) {
		rv += fprintf( f, "%s  <base>\n", indent );
		sprintf( indent_buf, "%s    ", indent );
		rv += xml_register( f, &expr->base, indent_buf );
		rv += fprintf( f, "%s  </base>\n", indent );
	}

	/* index */
	if ( (expr->elements & opdis_addr_expr_index) != 0 ) {
		rv += fprintf( f, "%s  <index>\n", indent );
		sprintf( indent_buf, "%s    ", indent );
		rv += xml_register( f, &expr->index, indent_buf );
		rv += fprintf( f, "%s  </index>\n", indent );
	}

	/* scale */
	rv += fprintf( f, "%s  <scale>%d</scale>\n", indent, expr->scale );
	buf[0] = '\0';
	opdis_addr_expr_shift_str( expr, buf, 8 );
	rv += fprintf( f, "%s  <shift>%s</shift>\n", indent, buf );

	/* displacement */
	if ( (expr->elements & opdis_addr_expr_disp) != 0 ) {
		rv += fprintf( f, "%s  <displacement>\n", indent );
		if ( (expr->elements & opdis_addr_expr_disp_abs) != 0 ) {
			rv += xml_abs_addr( f, &expr->displacement.a, 
					    indent_buf );
		} else if ( (expr->elements & opdis_addr_expr_disp_s) != 0 ) {
			rv += xml_immediate_s( f, expr->displacement.s, 
					       indent_buf );
		} else {
			rv += xml_immediate( f, expr->displacement.u, 
					     indent_buf );
		}
		rv += fprintf( f, "%s  </displacement>\n", indent );
	}

	rv += fprintf( f, "%s</expression>\n", indent );
	return rv;
}

static int xml_operand( FILE * f, opdis_op_t * op ) {
	int rv = 0;
	char buf[64];

	/* ascii:cat:flags: */
	rv += fprintf( f, "    <ascii>%s</ascii>\n", op->ascii );
	buf[0] = 0;
	opdis_op_cat_str( op, buf, 64 );
	rv += fprintf( f, "    <category>%s</category>\n", buf );
	buf[0] = 0;
	opdis_op_flags_str( op, buf, 64, "," );
	rv += xml_flags( f, buf, "    " ); 

	/* value */
	rv += fprintf( f, "    <value>\n" );

	switch (op->category) {
		case opdis_op_cat_register:
			rv += xml_register( f, &op->value.reg, "      " );
			break;
		case opdis_op_cat_absolute:
			rv += xml_abs_addr( f, &op->value.abs, "      " );
			break;
		case opdis_op_cat_expr:
			rv += xml_addr_expr( f, &op->value.expr, "      " );
			break;
		case opdis_op_cat_immediate:
		case opdis_op_cat_unknown:
			if ( (op->flags & opdis_op_flag_signed) != 0 ) {
				rv += xml_immediate_s( f, op->value.immediate.s,
							"      ");
			} else {
				rv += xml_immediate( f, op->value.immediate.u, 
						     "      ");
			}
			break;
	}

	rv += fprintf( f, "    </value>\n" );

	return rv;
}

static int xml_insn( FILE * f, opdis_insn_t * insn ) {
	int i, rv = 0;
	char buf[64];

	rv += fprintf( f, "<instruction>\n" );

	rv += fprintf( f, "  <offset>" );
	FPRINTF_ADDR( rv, f, insn->offset );
	rv += fprintf( f, "</offset>\n  <vma>" );
	FPRINTF_ADDR( rv, f, insn->vma );
	rv += fprintf( f, "</vma>\n  <bytes>\n" );
	for ( i = 0; i < insn->size; i++ ) {
		rv += fprintf( f, "    <byte>%02X</byte>\n", 
				insn->bytes[i] );
	}
	rv += fprintf( f, "  </bytes>\n" );

	if ( insn->status == opdis_decode_invalid ) {
		rv += fprintf( f, "  <invalid />\n</instruction>\n" );
		return rv;
	}

	/* ascii, prefix, mnemonic */
	rv += fprintf( f, "  <ascii>%s</ascii>\n", insn->ascii );
	if ( insn->num_prefixes ) {
		rv += fprintf( f, "  <prefix>%s</prefix>\n", insn->prefixes );
	}
	rv += fprintf( f, "  <mnemonic>%s</mnemonic>\n", insn->mnemonic );

	/* isa, cat, flags */
	buf[0] = 0;
	opdis_insn_isa_str( insn, buf, 64 );
	rv += fprintf( f, "  <isa>%s</isa>\n", buf );
	buf[0] = 0;
	opdis_insn_cat_str( insn, buf, 64 );
	rv += fprintf( f, "  <category>%s</category>\n", buf );

	buf[0] = 0;
	opdis_insn_flags_str( insn, buf, 64, "," );
	rv += xml_flags( f, buf, "  " ); 
	
	/* operands */
	rv += fprintf( f, "  <operands>\n" );
	for ( i=0; i < insn->num_operands; i++ ) {
		rv += fprintf( f, "    <operand" );
		if ( insn->operands[i] == insn->target ) {
			rv += fprintf( f, " name=\"target\"" );
		} else if ( insn->operands[i] == insn->src ) {
			rv += fprintf( f, " name=\"src\"" );
		} else if ( insn->operands[i] == insn->dest ) {
			rv += fprintf( f, " name=\"dest\"" );
		}
		rv += fprintf( f, ">\n" );

		rv += xml_operand( f, insn->operands[i] );
		rv += fprintf( f, "    </operand>\n" );
	}
	rv += fprintf( f, "  </operands>\n" );

	/* comment */
	if ( insn->comment[0] ) {
		rv += fprintf( f, "  <comment>\n%s\n</comment>\n", 
				insn->comment );
	}

	rv += fprintf( f, "</instruction>\n" );

	return rv;
}

static int handle_insn( FILE * f, const opdis_insn_t * insn, 
			const char * c ) {
	int rv = 0;
	char buf[64];
	buf[0] = 0;

	switch (*c) {
		case 'I':
			opdis_insn_isa_str( insn, buf, 64 );
			fprintf( f, "%s", buf );
			rv++;
			break;
		case 'C':
			opdis_insn_cat_str( insn, buf, 64 );
			fprintf( f, "%s", buf );
			rv++;
			break;
		case 'F':
			opdis_insn_flags_str( insn, buf, 64, "|" );
			fprintf( f, "%s", buf );
			rv++;
			break;
		case 'A':
			rv++;
			/* fall-through */
		default:
			fprintf( f, "%s", insn->ascii );
	}

	return rv;
}

static int handle_addr( FILE * f, const opdis_insn_t * insn, 
			const char * c ) {
	int rv = 0, junk;
	opdis_vma_t val;

	if ( *c == 'v' ) {
		val = insn->vma;
		c++;
		rv++;
	} else if ( *c == 'o' ) {
		val = insn->offset;
		c++;
		rv++;
	} else {
		val = insn->vma;
	}

	switch (*c) {
		case 'D':
			fprintf( f, "%lld", (long long int) val );
			rv++;
			break;
		case 'O':
			fprintf( f, "%llo", (long long int) val );
			rv++;
			break;
		case 'X':
			rv++;
			/* fall-through */
		default:
			FPRINTF_ADDR( junk, f, val );
	}

	return rv;
}

static int handle_bytes( FILE * f, const opdis_insn_t * insn, 
			 const char * c ) {
	int i, rv = 0, preceding = 0;
	char * fmt_str = "%02X";
	char buf[64];

	switch (*c) {
		case 'C':
			fmt_str = "%c";
			rv++;
			break;
		case 'D':
			fmt_str = "%2d";
			rv++;
			break;
		case 'O':
			fmt_str = "%02o";
			rv++;
			break;
		case 'X':
			rv++;
			/* fall-through */
		default:
			break;

	}

	for ( i = 0; i < insn->size; i++ ) {
		if ( preceding ) {
			fprintf( f, " " );
		}
		fprintf( f, fmt_str, insn->bytes[i] ); 
		preceding = 1;
	}

	return rv;
}

#define OP_CAT( f, op, buf )				\
	buf[0] = '\0';					\
	opdis_op_cat_str( op, buf, 64 );		\
	fprintf( f, "%s", buf );

#define OP_FLAGS( f, op, buf )				\
	buf[0] = '\0';					\
	opdis_op_flags_str( op, buf, 64, "|" );		\
	fprintf( f, "%s", buf );

static int handle_op( FILE * f, const opdis_insn_t * insn, const char * c ) {
	int rv = 0, all_operands = 0;
	opdis_op_t * op = NULL;
	char buf[64];

	if ( *c == 'a' ) {
		all_operands = 1;
		c++;
		rv++;
	} else if ( *c == 't' ) {
		op = insn->target;
		c++;
		rv++;
	} else if ( *c == 'd' ) {
		op = insn->dest;
		c++;
		rv++;
	} else if ( *c == 's' ) {
		op = insn->src;
		c++;
		rv++;
	} else if ( isdigit(*c) ) {
		int i = *c - '0';
		c++;
		rv++;
		if ( i < insn->num_operands ) {
			op = insn->operands[i];
		}
	} else {
		all_operands = 1;
	}

	if (! op && (! all_operands || ! insn->num_operands ) ) {
		if ( *c == 'C' || *c == 'F' || *c == 'A' ) {
			/* consume next */
			rv++;
		}
		return rv;
	}

	switch (*c) {
		case 'C':
			if ( all_operands ) {
				int i;
				for ( i = 0; i < insn->num_operands; i++ ) {
					if ( i > 0 ) {
						fprintf( f, ", " );
					}
					OP_CAT(f, insn->operands[i], buf);
				}
			} else {
				OP_CAT(f, op, buf);
			}
			rv++;
			break;
		case 'F':
			if ( all_operands ) {
				int i;
				for ( i = 0; i < insn->num_operands; i++ ) {
					if ( i > 0 ) {
						fprintf( f, ", " );
					}
					OP_FLAGS(f, insn->operands[i], buf);
				}
			} else {
				OP_FLAGS(f, op, buf);
			}
			rv++;
			break;
		case 'A':
			rv++;
			/* fall-through */
		default:
			if ( all_operands ) {
				int i;
				for ( i = 0; i < insn->num_operands; i++ ) {
					if ( i > 0 ) {
						fprintf( f, ", " );
					}
					fprintf( f, "%s", 
						insn->operands[i]->ascii );
				}
			} else {
				fprintf( f, "%s", op->ascii );
			}
	}

	return rv;
}

static int op_is_present( const opdis_insn_t * insn, char c ) {
	if (! insn->num_operands ) {
		return 0;
	}

	if ( c == 't' && ! insn->target ) {
		return 0;
	}

	if ( c == 'd' && ! insn->dest ) {
		return 0;
	}

	if ( c == 's' && ! insn->src ) {
		return 0;
	}

	if ( isdigit(c) ) {
		if ( (c - '0') >= insn->num_operands ) {
			return 0;
		}
	}

	/* default is all operands */
	return 1;
}

#define COND_DELIM(f, d)			\
	if ( d != '\0' ) {			\
		fprintf( f, "%c", d );		\
		d = '\0';			\
	}


static int custom_insn( FILE * f, const char * fmt_str, opdis_insn_t * insn ) {
	const char *c;
	char cond_delim = '\0';
	int rv = 0, inc;

	for ( c = fmt_str; *c; c++ ) {
		/* very lame and slow implementation */
		if ( *c != '%' ) {
			char out = *c;
			if ( *c == '\\' ) {
				c++;
				switch ( *c ) {
					case 'n': out = '\n'; break;
					case 't': out = '\t'; break;
					case '\\': out = '\\'; break;
					case '\'': out = '\''; break;
					case '\"': out = '\"'; break;
					case 'r': out = '\r'; break;
					case 'b': out = '\b'; break;
					case 'v': out = '\v'; break;
					case 'a': out = '\a'; break;
					case '?': out = '\?'; break;
					default: out = *c;
				}
			}
			rv += fprintf( f, "%c", out );
			cond_delim = '\0'; /* clear cond-delim */
			continue;
		}
		c++;
		if ( *c == '%' ) {
			rv += fprintf( f, "%%" );
			cond_delim = '\0'; /* clear cond-delim */
			continue;
		}

		switch (*c) {
			case 'i':	/* instruction */
				COND_DELIM( f, cond_delim );
				inc = handle_insn( f, insn, &c[1] );
				c += inc;
				rv += inc;
				break;
			case 'a':	/* address */
				COND_DELIM( f, cond_delim );
				inc = handle_addr( f, insn, &c[1] );
				c += inc;
				rv += inc;
				break;
			case 'b':	/* bytes */
				COND_DELIM( f, cond_delim );
				inc = handle_bytes( f, insn, &c[1] );
				c += inc;
				rv += inc;
				break;
			case 'p':	/* prefix */
				if ( insn->num_prefixes ) {
					COND_DELIM( f, cond_delim );
					rv += fprintf(f, "%s", insn->prefixes);
				} else {
					cond_delim = '\0';
				}
				break;
			case 'm':	/* mnemonic */
				if ( insn->mnemonic[0] ) {
					COND_DELIM( f, cond_delim );
					rv += fprintf(f, "%s", insn->mnemonic);
				} else {
					cond_delim = '\0';
				}
				break;
			case 'c':	/* comment */
				if ( insn->comment[0] ) {
					COND_DELIM( f, cond_delim );
					rv += fprintf( f, "%s", insn->comment );
				} else {
					cond_delim = '\0';
				}
				break;
			case 'o':	/* operand */
				if ( op_is_present( insn, c[1] ) ) {
					COND_DELIM( f, cond_delim );
				} else {
					cond_delim = '\0';
				}
				inc = handle_op( f, insn, &c[1] );
				c += inc;
				rv += inc;

				break;
			case '?':	/* conditional delim */
				c++;
				cond_delim = *c;
				break;
			case 't':	/* conditional tab */
				cond_delim = '\t';
				break;
			case 's':	/* conditional space */
				cond_delim = ' ';
				break;
			case 'n':	/* conditional newline */
				cond_delim = '\n';
				break;
			default:
				continue;
		}
	}

	return rv;
/*
 * Objects: i (insn), o (operand), r (register)
 * Numeric Format : BDXO binary decimal hex octal char
 * Component Format: ACF Ascii, Cat, Flags. Default is ascii.
 * %i[cfmt] - instruction
 * %c - comment
 * %b[numfmt|C] - bytes : takes BDXOC
 * %m - mnemonic
 * %p - prefix(es)
 * %o[atds#][numfmt]
 * %oa[cfmt] - all operands
 * %o#[cfmt] - operand '#'
 * %ot[cfmt] - operand target
 * %od[cfmt] - operand dest
 * %os[cfmt] - operand src
 * %a[v|o][format] - address (vma or offset) BDXO
 * %?[char] - conditional delimiter char (only if next % is true)
 * %t - conditional tab
 * %s - conditional space
 * %n - conditional newline
 * %%
 */
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
			rv += fprintf( f, " [" );
			FPRINTF_ADDR( rv, f, insn->vma );
			rv += fprintf( f, "]\n" );
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
