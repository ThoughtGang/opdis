/*!
 * \file metadata.h
 * \brief Metadata for instructions and objects.
 * \details This defines the metadata for instructions and operands in the
 *          opdis data model.
 * \author TG Community Developers <community@thoughtgang.org>
 * \note Copyright (c) 2010 ThoughtGang.
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef OPDIS_METADATA_H
#define OPDIS_METADATA_H

/* ---------------------------------------------------------------------- */
/* OPERANDS */

/*!
 * \enum opdis_op_cat_t
 * \ingroup model
 * \brief The \e category of an operand.
 * \details The type or \e category of an instruction operand. This is used
 *          to distinguish between operands that are registers, memory
 *          locations, immediate values, or relative offsets.
 */
enum opdis_op_cat_t {
	opdis_op_cat_unknown,	/*!< Unknown operand type */
	opdis_op_cat_register,	/*!< CPU register */
	opdis_op_cat_immr,	/*!< Immediate value */
	opdis_op_cat_addr,	/*!< Address */
	opdis_op_cat_faraddr,	/*!< Far address (seg:offset) */
	opdis_op_cat_rel,	/*!< Relative (to PC) offset */
	opdis_op_cat_exp	/*!< Address expression */
};

/*!
 * \enum opdis_op_flag_t
 * \ingroup model
 * \brief Flags applied to an operand.
 * \details Operand flags are used to encode additional information about the
 *          operand.
 */
enum opdis_op_flag_t {
	opdis_op_flag_r = 1,		/*!< Operand is read by insn */
	opdis_op_flag_w = 2,		/*!< Operand is written by insn */
	opdis_op_flag_x = 4,		/*!< Operand is executed by insn */
	opdis_op_flag_signed = 8,	/*!< Immediate data is signed */
	opdis_op_flag_address = 16,	/*!< Operand value is an address */
	opdis_op_flag_indirect = 32	/*!< Operand value points to address */
};

/*!
 * \enum opdis_reg_cat_t
 * \ingroup model
 * \brief The category of a CPU register.
 * \details The type or \e category of a CPU register.
 * \note A CPU register may have multiple purposes, so these can be ORed.
 */
enum opdis_reg_cat_t {
	opdis_reg_cat_unknown=0,  /*!< Unknown register type */
	opdis_reg_cat_gen=1,	  /*!< General-purpose register, e.g. EAX */
	opdis_reg_cat_fpu=2,	  /*!< FPU register, e.g. ST(0) */
	opdis_reg_cat_gpu=4,	  /*!< GPU register */
	opdis_reg_cat_sse=8,	  /*!< SSE register, e.g. XMM0 */
	opdis_reg_cat_task=16,	  /*!< Task management register, e.g. CR0 */
	opdis_reg_cat_mem=32,	  /*!< Memory management register, e.g. LDTR */
	opdis_reg_cat_debug=64,	  /*!< Debug register, e.g. DR0 */
	opdis_reg_cat_pc=128,	  /*!< Program Counter register, e.g. EIP */
	opdis_reg_cat_flags=256,  /*!< Flags register, e.g. EFLAGS */
	opdis_reg_cat_stack=512,  /*!< Stack pointer register, e.g. ESP */
	opdis_reg_cat_frame=1024, /*!< Frame pointer register, e.g. EBP */
	opdis_reg_cat_seg=2048    /*!< Segment register, e.g. CS */
};

/* ---------------------------------------------------------------------- */
/* INSTRUCTIONS */

/*!
 * \enum opdis_insn_cat_t
 * \ingroup model
 * \brief The \e category of an operand.
 * \details The type or \e category of an instructions. This is used to 
 *          distinguish between instructions at a high level : control
 *          flow instructions (jmp, call, ret), stack instructions (push,
 *          pop), floating point instructions, etc.
 */
enum opdis_insn_cat_t {
	opdis_insn_cat_unknown,	/*!< Unknown instruction type */
	opdis_insn_cat_cflow,	/*!< Control flow instruction */
	opdis_insn_cat_stack,	/*!< Stack manipulation instruction */
	opdis_insn_cat_fpu,	/*!< Floating-Point instruction */
	opdis_insn_cat_gpu,	/*!< GPU instruction */
	opdis_insn_cat_simd	/*!< SIMD extension instruction */
};

/*!
 * \enum opdis_cflow_flag_t
 * \ingroup model
 * \brief Details of the control flow instruction.
 * \details These flags encode specific information about the control flow
 *          instructions: is it a call, does it branch, is it a return,
 *          etc.
 */
enum opdis_cflow_flag_t {
	opdis_cflow_flag_none	/*!< No flags */
};

#endif
