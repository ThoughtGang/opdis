/*!
 * \file x86_decoder.h
 * \brief Decoder for disassembled x86 instructions
 * \author TG Community Developers <community@thoughtgang.org>
 * \note Copyright (c) 2010 ThoughtGang.
 * Released under the GNU Lesser Public License (LGPL), version 2.1.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef OPDIS_X86_DECODER_H
#define OPDIS_X86_DECODER_H

#include <opdis/opdis.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * \fn int opdis_x86_intel_decoder( const opdis_insn_buf_t, opdis_insn_t *,
				    const opdis_byte_t *, opdis_off_t, 
				    opdis_vma_t, opdis_off_t, void * )
 * \ingroup x86
 * \brief The built-in opdis x86 instruction decoder for Intel syntax.
 * \note The \e arg parameter is expected to be an opdis_t.
 */
int opdis_x86_intel_decoder( const opdis_insn_buf_t in, opdis_insn_t * out,
			     const opdis_byte_t * buf, opdis_off_t,
			     opdis_vma_t vma, opdis_off_t length, void * arg );

/*!
 * \fn int opdis_x86_att_decoder( const opdis_insn_buf_t, opdis_insn_t *,
				  const opdis_byte_t *, opdis_off_t, 
				  opdis_vma_t, opdis_off_t, void * )
 * \ingroup x86
 * \brief The built-in opdis x86 instruction decoder for AT&T syntax.
 * \note The \e arg parameter is expected to be an opdis_t.
 */
int opdis_x86_att_decoder( const opdis_insn_buf_t in, opdis_insn_t * out,
			   const opdis_byte_t * buf, opdis_off_t,
			   opdis_vma_t vma, opdis_off_t length, void * arg );

#ifdef __cplusplus
}
#endif

#endif
