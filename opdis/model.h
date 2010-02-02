/*!
 * \file model.h
 * \brief Data model for libopdis.
 * \details This defines the data model for libopdis.
 * \author thoughtgang.org
 */

#ifndef OPDIS_MODEL_H
#define OPDIS_MODEL_H

#ifdef WIN32
        #define LIBCALL _stdcall
#else
        #define LIBCALL
#endif


#define OPDIS_MAX_INSN_SZ 64		/* max bytes in insn */

/* ---------------------------------------------------------------------- */
#if 0
typedef struct {
	/* ASCII representation of instruction. This is the raw libopcodes
	 * output. */
	char ascii[OPDIS_MAX_INSN_STR];

	/* offset, load_address, and bytes in instruction. Note that address
	 * is set to offset by default; the OPDIS_HANDLER can choose to
	 * supply a real load address. */
	// TODO: use resolver?
	opdis_off_t offset;
	opdis_off_t address;
	opdis_off_t size;
	opdis_byte_t bytes[OPDIS_MAX_INSN_SIZE];

	/* instruction  */
	char mnemonic[OPDIS_MAX_ITEM_SZ];
	// prefixes
	// instruction type/ is cflow
	
	// operands
	opdis_off_t num_operands;
	//opdis_op_t operands[OPDIS_MAX_OP];
} opdis_insn_buf_t;
#endif

/* ---------------------------------------------------------------------- */
/* OPERAND */
/*!
 * \struct opdis_op_t 
 * \ingroup model
 * \brief Operand object
 * \details Stuff
 * \sa opdis_insn_t
 */

typedef struct {
	const char * ascii;		/*!< String representation of operand */
} opdis_op_t;

/* ---------------------------------------------------------------------- */
/* INSTRUCTION */
/*!
 * \struct opdis_insn_t 
 * \ingroup model
 * \brief Instruction object
 * \details Stuff
 * \sa opdis_op_t
 */
typedef struct {
	const char * ascii;		/*!< String representation of insn */

	opdis_off_t offset;		/*!< Offset of instruction in buffer */
	opdis_off_t address;		/*!< VMA of instruction */

	opdis_off_t size;		/*!< Size (# bytes) of insn */
	opdis_byte_t * bytes;		/*!< Pointer to insn bytes in buffer */

	/* instruction  */
	opdis_off_t num_prefixes;	/*!< Number of prefixes in insn */
	const char * prefixes;		/*!< Array of prefix strings */

	const char * mnemonic;		/*!< ASCII mnemonic for insn */
	int category;			/*!< Type of insn */
	union {
		// TODO: fn instruction type/ is cflow
		int cflow_flags;	/*!< Control flow insn flags */
	} flags;			/*!< Instruction-specific flags */

	
	// operands
	opdis_off_t num_operands;	/*!< Number of operands in insn */
	opdis_op_t * operands;		/*!< Array of operand objects */
} opdis_insn_t;

/* ---------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif


/*!
 * \fn
 * \ingroup model
 * \brief
 * \param
 * \return
 * \sa
 */

// insn alloc
// insn init

#ifdef __cplusplus
}
#endif

#endif
