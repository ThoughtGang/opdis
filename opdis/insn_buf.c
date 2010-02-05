/*!
 * \file insn_buf.c
 * \brief Buffer for building Opdis instructions from libopcodes output.
 * \author thoughtgang.org
 */

#include <opdis/insn_buf.h>

opdis_insn_buf_t LIBCALL opdis_insn_buf_alloc( size_t max_items, 
					       size_t max_item_size,
					       size_t max_insn_str ) {
}

void LIBCALL opdis_insn_buf_term( opdis_insn_buf_t ) {
}

int LIBCALL opdis_insn_buf_append( opdis_insn_buf_t, const char * item ) {
}

opdis_insn_buf_t LIBCALL opdis_insn_buf_alloc( size_t max_items, 
					       size_t max_item_size,
					       size_t max_insn_str );

void LIBCALL opdis_insn_buf_free( opdis_insn_buf_t buf );

int LIBCALL opdis_insn_buf_append( opdis_insn_buf_t buf, const char * item );
