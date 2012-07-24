/*
 * cflow_callback.c
 * Simple control-flow disassembler example.
 * Copied from tests/howto_callbacks.c just to have something in examples dir.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <opdis/opdis.h>

static int print_insn( opdis_insn_t * insn, void * arg ) {
	const char * filename = (const char *) arg;
	printf( "%08X [%s:%X]\t%s\n", insn->vma, filename, insn->offset,
		insn->ascii );
	return 1;
}

/* display callback */
static void my_display( const opdis_insn_t * insn, void * arg ) {
	opdis_insn_tree_t tree = (opdis_insn_tree_t) arg;
	opdis_insn_t * i = opdis_insn_dupe( insn );

	opdis_insn_tree_add( tree, i );

	printf( "%d bytes at offset %X\n", insn->size, insn->offset );
}

struct HANDLER_ARG { char halt_mnem[32]; opdis_t opdis; };

/* handler callback */
static int my_handler( const opdis_insn_t * insn, void * arg ) {
	struct HANDLER_ARG * harg = (struct HANDLER_ARG *) arg;

	/* halt disassembly if specified mnemonic is encountered */
	if ( (insn->status & opdis_decode_mnem) && 
	     ! strcmp(harg->halt_mnem, insn->mnemonic) ) {
		harg->opdis->display( insn, harg->opdis->display_arg );
		return 0;
	}

	/* invoke default handler to check for invalid and visited addresses */
	return opdis_default_handler( insn, harg->opdis );
}

/* resolver callback */
static opdis_vma_t my_resolver( const opdis_insn_t * insn, void * arg ) {
	/* return the offset component of segment:offset operands */
	if ( (insn->status & opdis_decode_op_flags) &&
	     insn->target && insn->target->category == opdis_op_cat_absolute ) {
		return (opdis_vma_t) insn->target->value.abs.offset;
	}

	/* invoke the default resolver to handle immediate values */
	return opdis_default_resolver( insn, NULL );
}

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

static void handle_target( opdis_insn_t * out, const char * item ) {
	opdis_op_t * op = out->operands[0];
	op->category = opdis_op_cat_unknown;
	op->flags = opdis_op_flag_x;
	opdis_op_set_ascii( op, item );
	out->target = out->operands[0];
}

static int decode_mnemonic( char ** items, int idx, opdis_insn_t * out ) {
	int i, num;
	const char *item = items[idx];

	/* detect JMP */
	num = (int) sizeof(jmp_insns) / sizeof(char *);
	for ( i = 0; i < num; i++ ) {
		if (! strcmp(jmp_insns[i], item) ) {
			out->category = opdis_insn_cat_cflow;
			out->flags.cflow = opdis_cflow_flag_jmp;
			handle_target( out, items[idx+1] );
			return 1;
		}
	}

	/* detect RET */
	num = (int) sizeof(ret_insns) / sizeof(char *);
	for ( i = 0; i < num; i++ ) {
		if (! strcmp(ret_insns[i], item) ) {
			out->category = opdis_insn_cat_cflow;
			out->flags.cflow = opdis_cflow_flag_ret;
			return 1;
		}
	}

	/* detect branch (call/jcc) */
	num = (int) sizeof(call_insns) / sizeof(char *);
	for ( i = 0; i < num; i++ ) {
		if (! strcmp(call_insns[i], item) ) {
			out->category = opdis_insn_cat_cflow;
			out->flags.cflow = opdis_cflow_flag_call;
			handle_target( out, items[idx+1] );
			return 1;
		}
	}
	num = (int) sizeof(jcc_insns) / sizeof(char *);
	for ( i = 0; i < num; i++ ) {
		if (! strcmp(jcc_insns[i], item) ) {
			out->category = opdis_insn_cat_cflow;
			out->flags.cflow = opdis_cflow_flag_jmpcc;
			handle_target( out, items[idx+1] );
			return 1;
		}
	}

	return 0;
}

/* decoder callback */
static int my_decoder( const opdis_insn_buf_t in, opdis_insn_t * out,
		       const opdis_byte_t * buf, opdis_off_t offset,
		       opdis_vma_t vma, opdis_off_t length, void * arg ) {
	int i, rv;

	/* the default decoder fills ascii, vma, offset, size, and bytes.
	 * it sets status to opdis_decode_basic. */
	rv = opdis_default_decoder( in, out, buf, offset, vma, length, NULL );
	if (! rv ) {
		return rv;
	}

	for ( i=0; i < in->item_count; i++ ) {
		if ( decode_mnemonic( in->items, i, out ) ) {
			out->status |= (opdis_decode_mnem | opdis_decode_ops | 
					opdis_decode_mnem_flags);
			break;
		}
	}

	return rv;
}

static int disassemble_file( const char * name, opdis_off_t offset ) {
	int rv;
	FILE * f;
	opdis_t o;
	opdis_buf_t buf;
	opdis_insn_tree_t tree;
	struct HANDLER_ARG handler_arg;

	f = fopen( name, "r" );
	if (! f ) {
		printf( "Unable to open file %s: %s\n", name, strerror(errno) );
		return -1;
	}

	buf = opdis_buf_read( f, 0, 0 );

	fclose( f );

	o = opdis_init();
	o->visited_addr = opdis_vma_tree_init();

	tree = opdis_insn_tree_init( 1 );
	strncpy( handler_arg.halt_mnem, "ret", 32 );
	handler_arg.opdis = o;
	opdis_set_handler( o, my_handler, (void *) &handler_arg );
	opdis_set_display( o, my_display, (void *) tree );
	opdis_set_resolver( o, my_resolver, NULL );
	opdis_set_decoder( o, my_decoder, NULL );

	rv = opdis_disasm_cflow( o, buf, (opdis_vma_t) offset );
	opdis_insn_tree_foreach( tree, print_insn, (void *) name );

	opdis_insn_tree_free( tree );
	opdis_term( o );
	
	return (rv > 0) ? 0 : -2;
}

int main( int argc, char ** argv ) {
	opdis_off_t offset = 0;

	if ( argc < 2 ) {
		printf( "Usage: %s file [offset]\n", argv[0] );
		return 1;
	} else if ( argc >= 3 ) {
		offset = (opdis_off_t) strtoul( argv[2], NULL, 0 );
	}
	return disassemble_file( argv[1], offset );
}
