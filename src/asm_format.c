/* asm_format.c
 * Copyright (c) 2010 ThoughtGang
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <string.h>

#include "asm_format.h"

#define FPRINTF_ADDR( rv, f, vma ) 					\
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

static int delim_reg( FILE * f, opdis_reg_t * reg ) {
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
			rv += fprintf( f, "{%s;%d;%d%s}", 
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
	}

	rv += fprintf( f, "\n" );

	return rv;
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
