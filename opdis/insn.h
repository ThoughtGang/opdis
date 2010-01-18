/*!
 * \file insn.h
 * \brief Instruction and operand support for libopdis.
 * \author thoughtgang.org
 */

#ifndef OPDIS_INSN_H
#define OPDIS_INSN_H

#ifdef WIN32
        #define LIBCALL _stdcall
#else
        #define LIBCALL
#endif


// this amounts to < 5 k for runtime. overkill, but safe(?).
#define OPDIS_MAX_ITEMS 64		/* max # items (mnem, prefix, op) */
#define OPDIS_MAX_ITEM_SIZE 64		/* max size of a single insn item */
#define OPDIS_MAX_INSN_STR 128		/* max length of ASCII insn string */
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
typedef struct {
	const char * ascii;

	opdis_off_t offset;
	opdis_off_t address;

	opdis_off_t size;
	opdis_byte_t * bytes;

	/* instruction  */
	const char * mnemonic;
	// prefixes
	// instruction type/ is cflow
	
	// operands
	opdis_off_t num_operands;
	//opdis_op_t * operands;
} opdis_insn_t;

/* ---------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif


/*!
 * \fn
 * \brief
 * \param
 * \relates
 * \sa
 */

// insn alloc
// insn init

#ifdef __cplusplus
}
#endif

#endif
