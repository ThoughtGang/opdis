/*!
 * \file opdis.h
 * \brief Disassembler front-end for libopcodes
 * \author thoughtgang.org
 */

#ifndef OPDIS_H
#define OPDIS_H

#include <dis-asm.h>		/* libopcodes (provided by binutils-dev) */

#include <opdis/types.h>
#include <opdis/insn.h>
#include <opdis/insn_buf.h>

#ifdef WIN32
        #define LIBCALL _stdcall
#else
        #define LIBCALL
#endif


/* ---------------------------------------------------------------------- */

// instructions are emitted to handler; handler makes decision to continue
// handler must copy instruction (const *) as it is overwritten on next
// iteration
// default is stdout
typedef int (*OPDIS_HANDLER) ( const opdis_insn_t *, void * arg );

// instructions are submitted to the decoder as an array of strings;
// the decoder fills the instruction object
// default is the internal x86 decoder or a generic string-only decoder
typedef int (*OPDIS_DECODER) ( const opdis_insn_buf_t * in, 
			       opdis_insn_t * out,
			       const opdis_byte_t * start, 
			       opdis_off_t length );

typedef opdis_addr_t (*OPDIS_RESOLVER) ( const opdis_buf_t * );

enum opdis_error_t { opdis_error_unknown, 
		     opdis_error_bounds,	/* Bounds of input exceeded */ 
		     opdis_error_invalid_insn,	/* Invalid instruction */
		     opdis_error_max_items	/* Instruction > insn_buf */ 
		   };
// default writes to stderr
typedef void (*OPDIS_ERROR) ( opdis_error_t error, const char * msg );

/* ---------------------------------------------------------------------- */
typedef struct {
	disassemble_info config;
	//int syntax;
	disassembler_ftype disassembler;
	OPDIS_DECODER decoder;
	OPDIS_HANDER handler;
	OPDIS_RESOLVER resolver;
	OPDIS_ERROR error_reporter;

	// tmp data filled by fprintf
	// note: allocated during init, freed during term
	opdis_insn_buf_t * insn;

} opdis_info_t;
typedef opdis_info_t * optdis_t;

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

opdis_t LIBCALL opdis_init( void );

void LIBCALL opdis_term( opdis_t );

void LIBCALL opdis_init_from_bfd( opdist_t, bfd * );

/*!
 * \fn opdis_set_defaults
 * \brief Initializes an opdis object to default, sane values.
 * \param o opdis object to re-initialize
 * \relates opdis_init
 * \sa opdis_set_arch
 * \note The default architecture is i386, and the defaul syntax is Intel.
 */
void LIBCALL opdis_set_defaults( opdis_t o );

enum opdis_x86_syntax_t syntax { x86_syntax_intel, x86_syntax_att };
// convenience funtion to set AT&T vs INTEL syntax
// this performs a setarch to x86 and uses the appropriate intel print_insn
void LIBCALL opdis_set_x86_syntax( opdis_t o, opdis_x86_syntax_t syntax );

// set architecture and disassembler to use
// if disassembler_ftype is NULL, the default for arch will be chosen
// default is x86 at&t
// also sets decoder
void LIBCALL opdis_set_arch( opdis_t o, enum bfd_architecture arch, 
			     disassembler_ftype fn );

void LIBCALL opdis_set_disassembler_options( opdis_t o, const char * options );

// instructions are emitted to handler; handler makes decision to continue
void LIBCALL opdis_set_handler( opdis_t o, OPDIS_HANDLER fn, void * arg );

// instructions are submitted to the decoder as an array of strings;
// the decoder fills the instruction object
void LIBCALL opdis_set_decoder( opdis_t o, OPDIS_DECODER fn );

// instruction is emitted to resolver; resolver returns jump/call target
void LIBCALL opdis_set_resolver( opdis_t o, OPDIS_RESOLVER fn );

void LIBCALL opdis_set_error_reporter( opdis_t o, OPDIS_ERROR fn );

// size of single insn at address
size_t LIBCALL opdis_disasm_insn_size( opdis_t o, opdis_buf_t buf, 
				       opdis_off_t offset );

// disasm single insn at address
int LIBCALL opdis_disasm_insn( opdis_t o, opdis_buf_t buf, opdis_off_t offset,
			       opdis_insn_t * insn );

// disasm serial range of addresses
int LIBCALL opdis_disasm_linear( opdis_t o, opdis_buf_t buf, opdis_off_t offset,
				 opdis_off_t length );
// disasm following cflow
int LIBCALL opdis_disasm_cflow( opdis_t o, opdis_buf_t buf, 
				opdis_off_t offset );

void LIBCALL opdis_error( opdis_t o, opdis_error_t error, const char * msg );

// todo invariant?
//opdis_( opdis_t * );
#ifdef __cplusplus
}
#endif

#endif
