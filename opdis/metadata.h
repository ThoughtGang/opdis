/*!
 *  \file metadata.h
 *  \brief Metadata for instructions and objects.
 *  \details This defines the metadata for instructions and operands in the
 *           opdis data model.
 *  \author thoughtgang.org
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
	opdis_op_cat_register	/*!< CPU register */
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
