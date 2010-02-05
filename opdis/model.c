/*!
 * \file model.c
 * \brief Data model for libopdis.
 * \author thoughtgang.org
 */

#include <opdis/model.h>

opdis_insn_t * LIBCALL opdis_insn_alloc( size_t num_operands );

opdis_insn_t * LIBCALL opdis_insn_alloc_fixed( size_t ascii_sz, 
				size_t mnemonic_sz, size_t num_operands,
				size_t op_ascii_sz );

opdis_insn_t * LIBCALL opdis_insn_dupe( const opdis_insn_t * i );

void LIBCALL opdis_insn_free( opdis_insn_t * i );

void LIBCALL opdis_insn_set_ascii( opdis_insn_t * i, const char * ascii );

void LIBCALL opdis_insn_set_mnemonic( opdis_insn_t * i, const char * mnemonic );

void LIBCALL opdis_insn_add_operand( opdis_insn_t * i, opdis_op_t * op );

opdis_op_t * LIBCALL opdis_op_alloc( void );

opdis_op_t * LIBCALL opdis_op_alloc_fixed( size_t ascii_sz );

opdis_op_t * LIBCALL opdis_op_alloc(  opdis_op_t * op );

void LIBCALL opdis_op_free( opdis_op_t * op );

void LIBCALL opdis_set_ascii( opdis_op_t * op, const char * ascii );
