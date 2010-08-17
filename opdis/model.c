/*!
 * \file model.c
 * \brief Data model implementation for libopdis.
 * \author TG Community Developers <community@thoughtgang.org>
 * \note Copyright (c) 2010 ThoughtGang.
 * Released under the GNU Lesser Public License (LGPL), version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <stdlib.h>
#include <string.h>

#include <opdis/model.h>

opdis_insn_t * LIBCALL opdis_insn_alloc( opdis_off_t num_operands ) {
	opdis_insn_t * i = (opdis_insn_t *) calloc( 1, sizeof(opdis_insn_t) );
	if (! i ) {
		return NULL;
	}

	if (! num_operands ) {
		return i;
	}

	/* operands is an array of opdis_t pointers */
	i->operands = (opdis_op_t **) calloc( num_operands, 
					     sizeof(opdis_op_t *) );
	if (! i->operands ) {
		free(i);
		return NULL;
	}

	i->alloc_operands = num_operands;

	return i;
}

#define PREFIX_SIZE(mnem_size) (4 * mnem_size)

opdis_insn_t * LIBCALL opdis_insn_alloc_fixed( size_t ascii_sz, 
				size_t mnemonic_sz, size_t num_operands,
				size_t op_ascii_sz ) {
	int i;

	opdis_insn_t * insn = opdis_insn_alloc( num_operands );
	if (! insn ) {
		return NULL;
	}

	insn->bytes = calloc( 1, 128 );	/* plenty */
	insn->ascii = calloc( 1, ascii_sz );
	insn->prefixes = calloc( 1, PREFIX_SIZE(mnemonic_sz) );
	insn->mnemonic = calloc( 1, mnemonic_sz );
	insn->comment = calloc( 1, ascii_sz );

	if (! insn->ascii || ! insn->prefixes || ! insn->mnemonic ||
	    ! insn->prefixes || ! insn->comment || ! insn->bytes ) {
		opdis_insn_free( insn );
		return NULL;
	}

	for ( i = 0; i < num_operands; i++ ) {
		opdis_op_t * op = opdis_op_alloc_fixed( op_ascii_sz );
		if ( op ) {
			insn->operands[i] = op;
			insn->alloc_operands++;
		} else {
			opdis_insn_free( insn );
			return NULL;
		}
	}

	insn->fixed_size = 1;
	insn->ascii_sz = ascii_sz;
	insn->mnemonic_sz = mnemonic_sz;

	return insn;
}

static unsigned int idx_for_op( const opdis_insn_t * insn, 
				const opdis_op_t * op ) {
	int i;
	for ( i = 0; i < insn->num_operands; i++ ) {
		if ( insn->operands[i] == op ) {
			return i;
		}
	}

	return 0;
}

opdis_insn_t * LIBCALL opdis_insn_dupe( const opdis_insn_t * insn ) {
	int i;
	opdis_op_t ** new_operands = NULL;

	opdis_insn_t * new_insn = opdis_insn_alloc( insn->num_operands );
	if (! new_insn ) {
		return NULL;
	}

	new_operands = new_insn->operands;
	memcpy( new_insn, insn, sizeof(opdis_insn_t) );

	new_insn->bytes = NULL;
	new_insn->ascii = NULL;
	new_insn->mnemonic = NULL;
	new_insn->prefixes = NULL;
	new_insn->operands = new_operands;
	new_insn->fixed_size = new_insn->ascii_sz = new_insn->mnemonic_sz = 0;

	new_insn->bytes = calloc( 1, insn->size );
	if (! new_insn->bytes ) {
		opdis_insn_free(new_insn);
		return NULL;
	}
	memcpy( new_insn->bytes, insn->bytes, insn->size );

	if ( insn->ascii ) {
		new_insn->ascii = strdup(insn->ascii);
		if (! new_insn->ascii ) {
			opdis_insn_free(new_insn);
			return NULL;
		}
	}

	if ( insn->prefixes ) {
		new_insn->prefixes = strdup(insn->prefixes);
		if (! new_insn->prefixes ) {
			opdis_insn_free(new_insn);
			return NULL;
		}
		new_insn->num_prefixes = insn->num_prefixes;
	}

	if ( insn->mnemonic ) {
		new_insn->mnemonic = strdup(insn->mnemonic);
		if (! new_insn->mnemonic ) {
			opdis_insn_free(new_insn);
			return NULL;
		}
	}

	if ( insn->comment ) {
		new_insn->comment = strdup(insn->comment);
		if (! new_insn->comment ) {
			opdis_insn_free(new_insn);
			return NULL;
		}
	}

	new_insn->num_operands = 0;
	for ( i = 0; i < insn->num_operands && i < new_insn->alloc_operands;
	      i++ ) {
		opdis_op_t * op = opdis_op_dupe( insn->operands[i] );
		if ( op ) {
			new_insn->operands[i] = op;
			new_insn->num_operands++;
		} else {
			opdis_insn_free(new_insn);
			return NULL;
		}
	}

	if ( insn->target ) {
		new_insn->target = new_insn->operands[idx_for_op(insn, 
						      insn->target)    ];
	}
	if ( insn->dest ) {
		new_insn->dest = new_insn->operands[idx_for_op(insn, 
						    insn->dest)    ];
	}
	if ( insn->src ) {
		new_insn->src = new_insn->operands[idx_for_op(insn, 
						   insn->src)      ];
	}

	return new_insn;
}

void LIBCALL opdis_insn_clear( opdis_insn_t * insn ) {
	int i;
	if ( insn ) {
		insn->status = opdis_decode_invalid;
		insn->category = opdis_insn_cat_unknown;
		insn->flags.cflow = 0;
		if (insn->ascii) insn->ascii[0] = '\0';
		insn->num_prefixes = 0;
		if (insn->prefixes) insn->prefixes[0] = '\0';
		if (insn->mnemonic) insn->mnemonic[0] = '\0';
		if (insn->comment) insn->comment[0] = '\0';
		for ( i = 0; i < insn->num_operands; i++ ) {
			opdis_op_clear( insn->operands[i] );
		}
		insn->num_operands = 0;
		insn->target = insn->dest = insn->src = NULL;
	}
}

void LIBCALL opdis_insn_free( opdis_insn_t * insn ) {
	int i; 
	if (! insn ) {
		return;
	}

	if ( insn->bytes ) {
		free( (void *) insn->bytes);
	}

	if ( insn->ascii ) {
		free( (void *) insn->ascii);
	}

	if ( insn->mnemonic ) {
		free( (void *) insn->mnemonic);
	}

	for ( i = 0; i < insn->num_operands; i++ ) {
		opdis_op_t * op = insn->operands[i];
		if ( op ) {
			opdis_op_free( op );
		}
	}

	free(insn);
}

void LIBCALL opdis_insn_set_ascii( opdis_insn_t * i, const char * ascii ) {
	if (! i || ! ascii ) {
		return;
	}

	if ( i->fixed_size ) {
		strncpy( i->ascii, ascii, i->ascii_sz - 1 );
		return;
	}

	if ( i->ascii ) {
		free((void *) i->ascii);
	}

	i->ascii = strdup(ascii);
}

void LIBCALL opdis_insn_set_mnemonic( opdis_insn_t * i, const char * mnemonic ){
	if (! i || ! mnemonic ) {
		return;
	}

	if ( i->fixed_size ) {
		strncpy( i->mnemonic, mnemonic, i->mnemonic_sz - 1 );
		return;
	}

	if ( i->mnemonic ) {
		free((void *) i->mnemonic);
	}

	i->mnemonic = strdup(mnemonic);
}

void LIBCALL opdis_insn_add_prefix( opdis_insn_t * i, const char * prefix ){
	if (! i || ! prefix ) {
		return;
	}

	if ( i->fixed_size ) {
		unsigned int size = PREFIX_SIZE(i->mnemonic_sz) - 
				    strlen(i->prefixes);
		if ( i->prefixes[0] ) {
			strncat( i->prefixes, " ", size - 1 );
		}
		strncat( i->prefixes, prefix, size - 2 );
		i->num_prefixes++;
		return;
	}

	if ( i->prefixes ) {
		void * ptr = realloc( i->prefixes, strlen(i->prefixes) + 
			       strlen(prefix) + 2 );
		if (! ptr ) {
			return;
		}

		i->prefixes = ptr;
		strcat( i->prefixes, " " );
	} else {
		i->prefixes = calloc( 1, strlen(prefix) + 1 );
		if (! i->prefixes ) {
			return;
		}
	}

	strcat( i->prefixes, prefix );
	i->num_prefixes++;
}

void LIBCALL opdis_insn_add_comment( opdis_insn_t * i, const char * cmt ){
	if (! i || ! cmt ) {
		return;
	}

	if ( i->fixed_size ) {
		unsigned int size = i->ascii_sz - strlen(i->comment);
		strncat( i->comment, " ", size - 1 );
		strncat( i->comment, cmt, size - 2 );
		return;
	}

	if ( i->comment ) {
		void * ptr = realloc( i->comment, 
				      strlen(i->comment) + strlen(cmt) + 2 );
		if (! ptr ) {
			return;
		}

		i->comment = ptr;
		strcat( i->comment, ";" );
	} else {
		i->comment = calloc( 1, strlen(cmt) + 1 );
		if (! i->comment ) {
			return;
		}
	}

	strcat( i->comment, cmt );
}

int LIBCALL opdis_insn_add_operand( opdis_insn_t * i, opdis_op_t * op ) {
	void * p;

	if (! i || ! op ) {
		return 0;
	}

	if ( i->num_operands < i->alloc_operands ) {
		i->operands[i->num_operands] = op;
		i->num_operands++;
		return 1;
	}

	i->alloc_operands++;
	p = realloc( i->operands, sizeof(opdis_op_t *) * i->alloc_operands ); 
	if (! p ) {
		i->alloc_operands--;
		return 0;
	}

	i->operands = (opdis_op_t **) p;
	i->operands[i->num_operands] = op;
	i->num_operands++;

	return 1;
}

opdis_op_t * LIBCALL opdis_insn_next_avail_op( opdis_insn_t * i ) {
	opdis_op_t * op;

	if (! i ) {
		return;
	}

	if ( i->num_operands == i->alloc_operands ) {
		return NULL;
	}

	op = i->operands[i->num_operands];
	i->num_operands++;

	return op;
}


int LIBCALL opdis_insn_is_branch( opdis_insn_t * i ) {
	/* All CALL and JMP instructions have a branch target */
	if ( i && i->category == opdis_insn_cat_cflow ) {
		if ( i->flags.cflow == opdis_cflow_flag_call ||
		     i->flags.cflow == opdis_cflow_flag_callcc ||
		     i->flags.cflow == opdis_cflow_flag_jmp ||
		     i->flags.cflow == opdis_cflow_flag_jmpcc ) {
			return 1;
		}
	}

	return 0;
}

int LIBCALL opdis_insn_fallthrough( opdis_insn_t * i ) {
	/* No fall-through for RET and JMP instructions */
	if ( i && i->category == opdis_insn_cat_cflow ) {
		if ( i->flags.cflow == opdis_cflow_flag_ret ||
		     i->flags.cflow == opdis_cflow_flag_jmp ) {
			return 0;
		}
	}
	return 1;
}

opdis_op_t * LIBCALL opdis_op_alloc( void ) {
	return (opdis_op_t *) calloc( 1, sizeof(opdis_op_t) );
}

opdis_op_t * LIBCALL opdis_op_alloc_fixed( size_t ascii_sz ) {
	opdis_op_t * op = opdis_op_alloc();
	if ( op ) {
		op->fixed_size = 1;
		op->ascii_sz = ascii_sz;
		op->ascii = (char *) calloc( 1, ascii_sz );
		if (! op->ascii ) {
			free(op);
			op = NULL;
		}
	}

	return op;
}

opdis_op_t * LIBCALL opdis_op_dupe(  opdis_op_t * op ) {
	opdis_op_t * new_op = opdis_op_alloc();
	if (! new_op ) {
		return NULL;
	}

	memcpy( new_op, op, sizeof(opdis_op_t) );
	new_op->fixed_size = new_op->ascii_sz = 0;
	new_op->ascii = NULL;

	if ( op->ascii ) {
		new_op->ascii = strdup(op->ascii);
		if (! new_op->ascii ) {
			free(new_op);
			new_op = NULL;
		}
	}

	return new_op;
}

void LIBCALL opdis_op_clear( opdis_op_t * op ) {
	if ( op ) {
		if (op->ascii) op->ascii[0] = '\0';
		op->category = opdis_op_cat_unknown;
		op->flags = opdis_op_flag_none;
		memset( &op->value, 0, sizeof(op->value) );
	}
}

void LIBCALL opdis_op_free( opdis_op_t * op ) {
	if (! op ) {
		return;
	}

	if ( op->ascii ) {
		free((void *) op->ascii);
	}
}

void LIBCALL opdis_op_set_ascii( opdis_op_t * op, const char * ascii ) {
	if (! op || ! ascii ) {
		return;
	}

	if ( op->fixed_size ) {
		strncpy( op->ascii, ascii, op->ascii_sz - 1 );
		return;
	}

	if ( op->ascii ) {
		free((void *) op->ascii);
	}

	op->ascii = strdup(ascii);
}

int LIBCALL opdis_insn_isa_str( const opdis_insn_t * insn, char * buf, 
				int buf_len ) {
	int max_size;
	if (! insn || ! buf || ! buf_len ) {
		return 0;
	}

	max_size = buf_len - strlen(buf) - 1;

	switch ( insn->isa ) {
		case opdis_insn_subset_gen:
			strncat( buf, "general purpose", max_size ); break;
		case opdis_insn_subset_fpu:
			strncat( buf, "fpu", max_size ); break;
		case opdis_insn_subset_gpu:
			strncat( buf, "gpu", max_size ); break;
		case opdis_insn_subset_simd:
			strncat( buf, "simd", max_size ); break;
		case opdis_insn_subset_vm:
			strncat( buf, "virtualization", max_size ); break;
	}

	return strlen(buf);
}

int LIBCALL opdis_insn_cat_str( const opdis_insn_t * insn, char * buf, 
				int buf_len ) {
	int max_size;
	if (! insn || ! buf || ! buf_len ) {
		return 0;
	}

	max_size = buf_len - strlen(buf) - 1;

	switch ( insn->category ) {
		case opdis_insn_cat_unknown:
			strncat( buf, "unknown", max_size ); break;
		case opdis_insn_cat_cflow:
			strncat( buf, "control flow", max_size ); break;
		case opdis_insn_cat_stack:
			strncat( buf, "stack", max_size ); break;
		case opdis_insn_cat_lost:
			strncat( buf, "load/store", max_size ); break;
		case opdis_insn_cat_test:
			strncat( buf, "compare", max_size ); break;
		case opdis_insn_cat_math:
			strncat( buf, "arithmetic", max_size ); break;
		case opdis_insn_cat_bit:
			strncat( buf, "bitwise", max_size ); break;
		case opdis_insn_cat_io:
			strncat( buf, "i/o", max_size ); break;
		case opdis_insn_cat_trap:
			strncat( buf, "trap", max_size ); break;
		case opdis_insn_cat_priv:
			strncat( buf, "privileged", max_size ); break;
		case opdis_insn_cat_nop:
			strncat( buf, "nop", max_size ); break;
	}

	return strlen(buf);
}

#define delim_strcat( buf, max_size, str, delim, use_delim ) 	\
	if ( use_delim ) {					\
		strncat( buf, delim, max_size );		\
		max_size -= strlen(delim);			\
	}							\
	strncat( buf, str, max_size );				\
	max_size -= strlen(str);				\
	use_delim = 1;

int LIBCALL opdis_insn_flags_str( const opdis_insn_t * insn, char * buf, 
				  int buf_len, const char * delim ) {
	int max_size, use_delim = 0;
	if (! insn || ! buf || ! buf_len ) {
		return 0;
	}

	max_size = buf_len - strlen(buf) - 1;

	switch ( insn->category ) {
		case opdis_insn_cat_cflow:
			if ( (insn->flags.cflow & opdis_cflow_flag_call) != 0 ){
				delim_strcat( buf, max_size, "call", 
					      delim, use_delim );
			}
			if ( (insn->flags.cflow & opdis_cflow_flag_callcc)!=0){
				delim_strcat( buf, max_size, "conditional call",
					      delim, use_delim );
			}
			if ( (insn->flags.cflow & opdis_cflow_flag_jmp) != 0 ){
				delim_strcat( buf, max_size, "jump", 
					      delim, use_delim );
			}
			if ( (insn->flags.cflow & opdis_cflow_flag_jmpcc)!= 0){
				delim_strcat( buf, max_size, "conditional jump",
					      delim, use_delim );
			}
			if ( (insn->flags.cflow & opdis_cflow_flag_ret) != 0 ){
				delim_strcat( buf, max_size, "return", 
					      delim, use_delim );
			}
			break;

		case opdis_insn_cat_stack:
			if ( (insn->flags.stack & opdis_stack_flag_push) != 0 ){
				delim_strcat( buf, max_size, "push", 
					      delim, use_delim );
			}
			if ( (insn->flags.stack & opdis_stack_flag_pop) != 0 ) {
				delim_strcat( buf, max_size, "pop", 
					      delim, use_delim );
			}
			if ( (insn->flags.stack & opdis_stack_flag_frame) != 0){
				delim_strcat( buf, max_size, "enter frame", 
					      delim, use_delim );
			}
			if ( (insn->flags.stack & opdis_stack_flag_unframe)!=0){
				delim_strcat( buf, max_size, "exit frame", 
					      delim, use_delim );
			}
			break;

		case opdis_insn_cat_bit:
			if ( (insn->flags.bit & opdis_bit_flag_and) != 0 ) {
				delim_strcat( buf, max_size, "bitwise and", 
					      delim, use_delim );
			}
			if ( (insn->flags.bit & opdis_bit_flag_or) != 0 ) {
				delim_strcat( buf, max_size, "bitwise or", 
					      delim, use_delim );
			}
			if ( (insn->flags.bit & opdis_bit_flag_xor) != 0 ) {
				delim_strcat( buf, max_size, "bitwise xor", 
					      delim, use_delim );
			}
			if ( (insn->flags.bit & opdis_bit_flag_not) != 0 ) {
				delim_strcat( buf, max_size, "bitwise not", 
					      delim, use_delim );
			}
			if ( (insn->flags.bit & opdis_bit_flag_lsl) != 0 ) {
				delim_strcat( buf, max_size, 
					      "logical shift left", 
					      delim, use_delim );
			}
			if ( (insn->flags.bit & opdis_bit_flag_lsr) != 0 ) {
				delim_strcat( buf, max_size, 
					      "logical shift right", 
					      delim, use_delim );
			}
			if ( (insn->flags.bit & opdis_bit_flag_asl) != 0 ) {
				delim_strcat( buf, max_size, 
					      "arithmetic shift left", 
					      delim, use_delim );
			}
			if ( (insn->flags.bit & opdis_bit_flag_asr) != 0 ) {
				delim_strcat( buf, max_size, 
					      "arithmetic shift right", 
					      delim, use_delim );
			}
			if ( (insn->flags.bit & opdis_bit_flag_rol) != 0 ) {
				delim_strcat( buf, max_size, "rotate left", 
					      delim, use_delim );
			}
			if ( (insn->flags.bit & opdis_bit_flag_ror) != 0 ) {
				delim_strcat( buf, max_size, "rotate right", 
					      delim, use_delim );
			}
			if ( (insn->flags.bit & opdis_bit_flag_rcl) != 0 ) {
				delim_strcat( buf, max_size, 
					      "rotate carry left", 
					      delim, use_delim );
			}
			if ( (insn->flags.bit & opdis_bit_flag_rcr) != 0 ) {
				delim_strcat( buf, max_size, 
					      "rotate carry right", 
					      delim, use_delim );
			}
			break;

		case opdis_insn_cat_io:
			if ( (insn->flags.io & opdis_io_flag_in) != 0 ) {
				delim_strcat( buf, max_size, "input from port", 
					      delim, use_delim );
			}
			if ( (insn->flags.io & opdis_io_flag_out) != 0 ) {
				delim_strcat( buf, max_size, "output to port", 
					      delim, use_delim );
			}
			break;

		default:
			break;
	}

	return strlen(buf);
}

int LIBCALL opdis_op_cat_str( const opdis_op_t * op, char * buf, int buf_len ) {
	int max_size;
	if (! op || ! buf || ! buf_len ) {
		return 0;
	}

	max_size = buf_len - strlen(buf) - 1;

	switch ( op->category ) {
		case opdis_op_cat_unknown:
			strncat( buf, "unknown", max_size ); break;
		case opdis_op_cat_register:
			strncat( buf, "register", max_size ); break;
		case opdis_op_cat_immediate:
			strncat( buf, "immediate", max_size ); break;
		case opdis_op_cat_absolute:
			strncat( buf, "absolute address", max_size ); break;
		case opdis_op_cat_expr:
			strncat( buf, "address expression", max_size ); break;
	}

	return strlen(buf);
}

int LIBCALL opdis_op_flags_str( const opdis_op_t * op, char * buf, int buf_len,
				const char * delim ) {
	int max_size, use_delim = 0;
	if (! op || ! buf || ! buf_len || ! delim ) {
		return 0;
	}

	max_size = buf_len - strlen(buf) - 1;

	if ( (op->flags & opdis_op_flag_r) != 0 ) {
		delim_strcat( buf, max_size, "read", delim, use_delim );
	}
	if ( (op->flags & opdis_op_flag_w) != 0 ) {
		delim_strcat( buf, max_size, "write", delim, use_delim );
	}
	if ( (op->flags & opdis_op_flag_x) != 0 ) {
		delim_strcat( buf, max_size, "exec", delim, use_delim );
	}
	if ( (op->flags & opdis_op_flag_signed) != 0 ) {
		delim_strcat( buf, max_size, "signed", delim, use_delim );
	}
	if ( (op->flags & opdis_op_flag_address) != 0 ) {
		delim_strcat( buf, max_size, "address", delim, use_delim );
	}
	if ( (op->flags & opdis_op_flag_indirect) != 0 ) {
		delim_strcat( buf, max_size, "indirect", delim, use_delim );
	}

	return strlen(buf);
}

int LIBCALL opdis_reg_flags_str( const opdis_reg_t * reg, char * buf, 
				 int buf_len, const char * delim ) {
	int max_size, use_delim = 0;
	if (! reg || ! buf || ! buf_len || ! delim ) {
		return 0;
	}

	max_size = buf_len - strlen(buf) - 1;
	if ( (reg->flags & opdis_reg_flag_gen) != 0 ) {
		delim_strcat( buf, max_size, "general", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_fpu) != 0 ) {
		delim_strcat( buf, max_size, "fpu", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_gpu) != 0 ) {
		delim_strcat( buf, max_size, "gpu", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_simd) != 0 ) {
		delim_strcat( buf, max_size, "simd", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_task) != 0 ) {
		delim_strcat( buf, max_size, "task mgt", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_mem) != 0 ) {
		delim_strcat( buf, max_size, "memory mgt", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_debug) != 0 ) {
		delim_strcat( buf, max_size, "debug", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_pc) != 0 ) {
		delim_strcat( buf, max_size, "pc", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_flags) != 0 ) {
		delim_strcat( buf, max_size, "flags", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_stack) != 0 ) {
		delim_strcat( buf, max_size, "stack ptr", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_frame) != 0 ) {
		delim_strcat( buf, max_size, "frame ptr", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_seg) != 0 ) {
		delim_strcat( buf, max_size, "segment", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_zero) != 0 ) {
		delim_strcat( buf, max_size, "zero", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_argsin) != 0 ) {
		delim_strcat( buf, max_size, "in args", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_argsout) != 0 ) {
		delim_strcat( buf, max_size, "out args", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_locals) != 0 ) {
		delim_strcat( buf, max_size, "locals", delim, use_delim );
	}
	if ( (reg->flags & opdis_reg_flag_return) != 0 ) {
		delim_strcat( buf, max_size, "return addr", delim, use_delim );
	}

	return strlen(buf);
}

int LIBCALL opdis_addr_expr_shift_str( const opdis_addr_expr_t * exp, 
					char * buf, int buf_len ) {
	int max_size, use_delim = 0;
	if (! exp || ! buf || ! buf_len ) {
		return 0;
	}

	max_size = buf_len - strlen(buf) - 1;
	switch ( exp->shift ) {
		case opdis_addr_expr_lsl:
			strncat( buf, "lsl", max_size ); break;
        	case opdis_addr_expr_lsr:
			strncat( buf, "lsr", max_size ); break;
        	case opdis_addr_expr_asl:
			strncat( buf, "asl", max_size ); break;
        	case opdis_addr_expr_ror:
			strncat( buf, "ror", max_size ); break;
		case opdis_addr_expr_rrx:
			strncat( buf, "rrx", max_size ); break;
		default:
			strncat( buf, "unknown", max_size ); break;
	}

	return strlen(buf);
}
