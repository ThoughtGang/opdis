/*!
 * \file model.c
 * \brief Data model implementation for libopdis.
 * \author TG Community Developers <community@thoughtgang.org>
 * \note Copyright (c) 2010 ThoughtGang.
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <stdlib.h>
#include <string.h>

#include <opdis/model.h>

opdis_insn_t * LIBCALL opdis_insn_alloc( size_t num_operands ) {
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

opdis_insn_t * LIBCALL opdis_insn_alloc_fixed( size_t ascii_sz, 
				size_t mnemonic_sz, size_t num_operands,
				size_t op_ascii_sz ) {
	int i;

	opdis_insn_t * insn = opdis_insn_alloc( num_operands );
	if (! insn ) {
		return NULL;
	}

	insn->ascii = calloc( 1, ascii_sz );
	insn->mnemonic = calloc( 1, mnemonic_sz );

	if (! insn->ascii || ! insn->mnemonic ) {
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

	return insn;
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
	new_insn->ascii = NULL;
	new_insn->mnemonic = NULL;
	new_insn->operands = new_operands;

	if ( insn->ascii ) {
		new_insn->ascii = strdup(insn->ascii);
		if (! new_insn->ascii ) {
			opdis_insn_free(new_insn);
			return NULL;
		}
	}

	if ( insn->mnemonic ) {
		new_insn->mnemonic = strdup(insn->mnemonic);
		if (! new_insn->mnemonic ) {
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

	return new_insn;
}

void LIBCALL opdis_insn_clear( opdis_insn_t * i ) {
	if ( i ) {
		i->status = opdis_decode_invalid;
		i->ascii[0] = '\0';
		i->num_prefixes = 0;
		i->prefixes[0] = '\0';
		i->mnemonic[0] = '\0';
		i->num_operands = 0;
		i->target = i->dest = i->src = NULL;
	}
}

void LIBCALL opdis_insn_free( opdis_insn_t * insn ) {
	int i; 
	if (! insn ) {
		return;
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

	if ( i->ascii ) {
		free((void *) i->ascii);
	}

	i->ascii = strdup(ascii);
}

void LIBCALL opdis_insn_set_mnemonic( opdis_insn_t * i, const char * mnemonic ){
	if (! i || ! mnemonic ) {
		return;
	}

	if ( i->mnemonic ) {
		free((void *) i->mnemonic);
	}

	i->mnemonic = strdup(mnemonic);
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

int LIBCALL opdis_insn_is_branch( opdis_insn_t * insn ) {
	// TODO : check flags for jcc, call
	return 0;
}

int LIBCALL opdis_insn_fallthrough( opdis_insn_t * insn ) {
	// TODO : check flags for ret. jmp
	return 1;
}

opdis_op_t * LIBCALL opdis_op_alloc( void ) {
	return (opdis_op_t *) calloc( 1, sizeof(opdis_op_t) );
}

opdis_op_t * LIBCALL opdis_op_alloc_fixed( size_t ascii_sz ) {
	opdis_op_t * op = opdis_op_alloc();
	if ( op ) {
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

	if ( op->ascii ) {
		free((void *) op->ascii);
	}

	op->ascii = strdup(ascii);
}
