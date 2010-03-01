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
 * \note It is the responsibility of the architecture-specific decoder to
 *       enure that the correct category is set. A value of 'unknown' does
 *       not ensure that one of the other values is not suitable, but may
 *       indicate that the decoder does not fully support the operand.
 */
enum opdis_op_cat_t {
	opdis_op_cat_unknown,	/*!< Unknown operand type */
	opdis_op_cat_register,	/*!< CPU register */
	opdis_op_cat_immediate,	/*!< Immediate value */
	opdis_op_cat_absolute,	/*!< Absolute address (seg:offset) */
	opdis_op_cat_expr	/*!< Address expression */
};

/*!
 * \enum opdis_op_flag_t
 * \ingroup model
 * \brief Flags applied to an operand.
 * \details Operand flags are used to encode additional information about the
 *          operand.
 * \note It is the responsibility of the architecture-specific decoder to
 *       enure that the correct flags set. A missing flag does not 
 *       necessarily mean that the flag does not apply, but may indicate
 *       that the decoder does not fully support the operand.
 */
enum opdis_op_flag_t {
	opdis_op_flag_none = 0,		/*!< No flags */
	opdis_op_flag_r = 1,		/*!< Operand is read by insn */
	opdis_op_flag_w = 2,		/*!< Operand is written by insn */
	opdis_op_flag_x = 4,		/*!< Operand is executed by insn */
	opdis_op_flag_signed = 8,	/*!< Immediate data is signed */
	opdis_op_flag_address = 16,	/*!< Operand value is an address */
	opdis_op_flag_indirect = 32	/*!< Operand value points to address */
};

/*!
 * \enum opdis_reg_flags_t
 * \ingroup model
 * \brief The flags for a CPU register.
 * \details The type or \e flags of a CPU register.
 * \note A CPU register may have multiple purposes, so these can be ORed.
 * \note It is the responsibility of the architecture-specific decoder to
 *       enure that the correct flags set. A missing flag does not 
 *       necessarily mean that the flag does not apply, but may indicate
 *       that the decoder does not fully support the register.
 */
enum opdis_reg_flag_t {
	opdis_reg_flag_unknown=0,     /*!< Unknown register type */
	opdis_reg_flag_gen=1,	      /*!< General-purpose register, e.g. EAX */
	opdis_reg_flag_fpu=2,	      /*!< FPU register, e.g. ST(0) */
	opdis_reg_flag_gpu=4,	      /*!< GPU register */
	opdis_reg_flag_simd=8,	      /*!< SIMD register, e.g. XMM0 */
	opdis_reg_flag_task=16,	      /*!< Task management register, e.g. CR0 */
	opdis_reg_flag_mem=32,	      /*!< Memory mgt register, e.g. LDTR */
	opdis_reg_flag_debug=64,      /*!< Debug register, e.g. DR0 */
	opdis_reg_flag_pc=128,	      /*!< Program Counter register, e.g. EIP */
	opdis_reg_flag_flags=256,     /*!< Flags register, e.g. EFLAGS */
	opdis_reg_flag_stack=512,     /*!< Stack pointer register, e.g. ESP */
	opdis_reg_flag_frame=1024,    /*!< Frame pointer register, e.g. EBP */
	opdis_reg_flag_seg=2048,      /*!< Segment register, e.g. CS */
	opdis_reg_flag_zero=4096,     /*!< Zero register (RISC) */
	opdis_reg_flag_argsin=8192,   /*!< Incoming arguments (RISC) */
	opdis_reg_flag_argsout=16384, /*!< Outgoing arguments (RISC) */
	opdis_reg_flag_locals=32768,  /*!< Local variables (RISC) */
	opdis_reg_flag_return=65536   /*!< Return address */
};

/* ---------------------------------------------------------------------- */
/* INSTRUCTIONS */

/*!
 * \enum opdis_insn_subset_t
 * \ingroup model
 * \brief The \e subset of an ISA that the instruction belongs to.
 * \note It is the responsibility of the architecture-specific decoder to
 *       enure that the correct ISA is set. A value of 'general' does
 *       not ensure that one of the other values is not suitable, but may
 *       indicate that the decoder does not fully support the ISA.
 */
enum opdis_insn_subset_t {
	opdis_insn_subset_gen,	/*!< General-purpose instruction */
	opdis_insn_subset_fpu,	/*!< Floating-Point instruction */
	opdis_insn_subset_gpu,	/*!< GPU instruction */
	opdis_insn_subset_simd,	/*!< SIMD extension instruction */
	opdis_insn_subset_vm	/*!< Virtual Machine extension */
};

/*!
 * \enum opdis_insn_cat_t
 * \ingroup model
 * \brief The \e category of an instruction.
 * \details The type or \e category of an instruction. This is used to 
 *          distinguish between instructions at a high level : control
 *          flow instructions (jmp, call, ret), stack instructions (push,
 *          pop), floating point instructions, etc.
 * \note It is the responsibility of the architecture-specific decoder to
 *       enure that the correct category is set. A value of 'unknown' does
 *       not ensure that one of the other values is not suitable, but may
 *       indicate that the decoder does not fully support the instruction.
 */
enum opdis_insn_cat_t {
	opdis_insn_cat_unknown,	/*!< Unknown instruction type */
	opdis_insn_cat_cflow,	/*!< Control flow instruction */
	opdis_insn_cat_stack,	/*!< Stack manipulation instruction */
	opdis_insn_cat_lost,	/*!< Load/store instruction */
	opdis_insn_cat_test,	/*!< Test/compare instruction */
	opdis_insn_cat_math,	/*!< Arithmetic instruction */
	opdis_insn_cat_bit,	/*!< Bitwise (shift/and/or/etc) instruction */
	opdis_insn_cat_flag,	/*!< Flag register test/set instruction */
	opdis_insn_cat_io,	/*!< I/O port instruction */
	opdis_insn_cat_trap,	/*!< Interrupt/trap instruction */
	opdis_insn_cat_priv,	/*!< Privileged (ring0) instruction */
	opdis_insn_cat_nop	/*!< No-operation instruction */

};

/*!
 * \enum opdis_cflow_flag_t
 * \ingroup model
 * \brief Details of the control flow instruction.
 * \details These flags encode specific information about the control flow
 *          instructions: is it a call, does it branch, is it a return,
 *          etc.
 * \note It is the responsibility of the architecture-specific decoder to
 *       enure that the correct flags set. A missing flag does not 
 *       necessarily mean that the flag does not apply, but may indicate
 *       that the decoder does not fully support the instruction.
 */
enum opdis_cflow_flag_t {
	opdis_cflow_flag_none=0,	/*!< No flags */
	opdis_cflow_flag_call=1,	/*!< Call */
	opdis_cflow_flag_callcc=2,	/*!< Conditional call */
	opdis_cflow_flag_jmp=4,		/*!< Jump */
	opdis_cflow_flag_jmpcc=8,	/*!< Conditional jump */
	opdis_cflow_flag_ret=16		/*!< Return from call */
};

/*!
 * \enum opdis_stack_flag_t
 * \ingroup model
 * \brief Details of the instruction.
 * \details These flags encode specific information about the 
 *          instructions: is it a push or a pop, does it enter or
 *          leave a stack frame, etc.
 * \note It is the responsibility of the architecture-specific decoder to
 *       enure that the correct flags set. A missing flag does not 
 *       necessarily mean that the flag does not apply, but may indicate
 *       that the decoder does not fully support the instruction.
 */
enum opdis_stack_flag_t {
	opdis_stack_flag_none=0,	/*!< No flags */
	opdis_stack_flag_push=1,	/*!< Push to stack */
	opdis_stack_flag_pop=2,		/*!< Pop from stack */
	opdis_stack_flag_frame=4,	/*!< Enter stack frame */
	opdis_stack_flag_unframe=8	/*!< Exit stack frame */
};

/*!
 * \enum opdis_bit_flag_t
 * \ingroup model
 * \brief Details of the instruction.
 * \details These flags encode specific information about the 
 *          instructions: is it a shift or rotate (and is the carry flag
 *          significant), is it an AND/OR/XOR/NOT operation, etc.
 * \note It is the responsibility of the architecture-specific decoder to
 *       enure that the correct flags set. A missing flag does not 
 *       necessarily mean that the flag does not apply, but may indicate
 *       that the decoder does not fully support the instruction.
 */
enum opdis_bit_flag_t {
	opdis_bit_flag_none=0,	/*!< No flags */
	opdis_bit_flag_and=1,	/*!< bitwise AND */
	opdis_bit_flag_or=2,	/*!< bitwise OR */
	opdis_bit_flag_xor=4,	/*!< bitwise XOR */
	opdis_bit_flag_not=8,	/*!< bitwise NOT */
	opdis_bit_flag_lsl=16,	/*!< Logical shift left */
	opdis_bit_flag_lsr=32,	/*!< Logical shift right */
	opdis_bit_flag_asl=64,	/*!< Arithmetic shift left */
	opdis_bit_flag_asr=128,	/*!< Arithmetic shift right */
	opdis_bit_flag_rol=256,	/*!< Rotate left */
	opdis_bit_flag_ror=512,	/*!< Rotate right */
	opdis_bit_flag_rcl=1024,/*!< Rotate left with carry */
	opdis_bit_flag_rcr=2048	/*!< Rotate right with carry */
};

/*!
 * \enum opdis_io_flag_t
 * \ingroup model
 * \brief Details of the instruction.
 * \details These flags encode specific information about the 
 *          instructions: does it read input from a port, does it write
 *          output to a port, etc.
 * \note It is the responsibility of the architecture-specific decoder to
 *       enure that the correct flags set. A missing flag does not 
 *       necessarily mean that the flag does not apply, but may indicate
 *       that the decoder does not fully support the instruction.
 */
enum opdis_io_flag_t {
	opdis_io_flag_none=0,	/*!< No flags */
	opdis_io_flag_in=1,	/*!< Input from port */
	opdis_io_flag_out=2	/*!< Output from port */
};

#endif
