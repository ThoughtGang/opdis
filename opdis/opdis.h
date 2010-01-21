/*!
 * \file opdis.h
 * \brief Disassembler front-end for libopcodes.
 * \details Opdis is a wrapper for libopcodes which provides more support 
 * for disassembly than that needed by objdump.
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

/*!
 * \typedef int (*OPDIS_HANDLER) ( const opdis_insn_t * i, void * arg )
 * \ingroup configuration
 * \brief Callback used to determine if disassembly should continue.
 * \param i The most recently disassembled instruction.
 * \param arg Argument provided when the callback is set
 * \return 0 if disassembly should halt, nonzero (1) otherwise.
 * This function is invoked after the display callback. The default 
 * behavior is to halt disassembly only if an invalid instruction is
 * encountered. The caller can override this function in order to 
 * specify more detailed halting conditions, e.g. to halt once a
 * sequience of instructions has been encountered.
 */
typedef int (*OPDIS_HANDLER) ( const opdis_insn_t * i, void * arg );

/*!
 * \typedef void (*OPDIS_DISPLAY) ( const opdis_insn_t * i, void * arg );
 * \ingroup configuration
 * \brief Callback used to display or store a disassembled instruction
 * \param i The most recently disassembled instruction.
 * \param arg Argument provided when the callback is set
 * This function is invoked every time an instruction is disassembled; it
 * emits the instruction to the caller of the disassembler. The default
 * display function writes to STDOUT.
 * \note The caller must copy the instruction if they are going to store it;
 *       the contents of i will be overwritten when the next instruction
 *       is disassembled.
 */
typedef void (*OPDIS_DISPLAY) ( const opdis_insn_t * i, void * arg );

// instructions are submitted to the decoder as an array of strings;
// the decoder fills the instruction object
// default is the internal x86 decoder or a generic string-only decoder
typedef int (*OPDIS_DECODER) ( const opdis_insn_buf_t * in, 
			       opdis_insn_t * out,
			       const opdis_byte_t * start, 
			       opdis_off_t length );

typedef opdis_addr_t (*OPDIS_RESOLVER) ( const opdis_buf_t *, void * arg );

enum opdis_error_t { opdis_error_unknown, 
		     opdis_error_bounds,	/* Bounds of input exceeded */ 
		     opdis_error_invalid_insn,	/* Invalid instruction */
		     opdis_error_max_items	/* Instruction > insn_buf */ 
		   };
// default writes to stderr
typedef void (*OPDIS_ERROR) ( opdis_error_t error, const char * msg,
			      void * arg );

/* ---------------------------------------------------------------------- */

/*!
 * \struct opdis_t
 * \ingroup configuration
 * \brief An opdis disassembler
 */
typedef struct {
	/*! \var config
	 *  \brief libopcodes configuration structure
	 *  \note This is defined in dis-asm.h in the binutils distribution.
	 */
	disassemble_info config;

	/*! \var disassembler
	 *  \brief libopcodes print_insn routine
	 *  \note This is defined in dis-asm.h in the binutils distribution.
	 */
	disassembler_ftype disassembler;

	/*! \var error_reporter
	 *  \brief callback for reporting errors encountered during disassembly
	 */
	OPDIS_ERROR error_reporter;
	void * error_reporter_arg;

	/*! \var display
	 *  \brief callback to display or store a disassembled instruction
	 */
	OPDIS_DISPLAY display;
	void * display_arg;

	/*! \var handler
	 *  \brief callback which determines whether to continue disassembly
	 */
	OPDIS_HANDLER handler;
	void * handler_arg;

	/*! \var resolver
	 *  \brief callback for converting a virtual address to a buffer offset
	 */
	OPDIS_RESOLVER resolver;
	void * resolver_arg;

	/*! \var decoder
	 *  \brief callback which builds an opdis_insn_t from libopcodes strings
	 */
	OPDIS_DECODER decoder;

	/*! \var buf
	 *  \brief buffer for storing libopcodes strings as they are emitted 
	 */
	opdis_insn_buf_t * buf;

} * opdis_t;

/* ---------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * \fn opdis_init()
 * \ingroup disassembly
 * \brief Initialize an opdis disassembler
 * \details Allocates an opdis_t and configures it using opdis_set_defaults().
 * \sa opdis_init_from_bfd opdis_term
 * \return An opdis disassembler object
 */

opdis_t LIBCALL opdis_init( void );


/*!
 * \fn opdis_init_from_bfd( bfd * )
 * \ingroup disassembly
 * \brief Initialize an opdis disassembler based on a BFD object
 * \details Allocates an opdis_t using opdis_init(), then configures libopcodes 
 * based on information (e.g. the architecture) in the BFD.
 * \param target A BFD object to operate on
 * \sa opdis_init_from_bfd
 * \return An opdis disassembler object
 * \note This requires that the BFD be configured correctly for the target
 *       architecture. See the documentation for libbfd in the GNU binutils
 *       distribution.
 */
opdis_t LIBCALL opdis_init_from_bfd( bfd * target );

/*!
 * \fn opdis_term( opdis_t )
 * \ingroup disassembly
 * \brief Cleanup an opdis disassembler
 * \details Frees an opdis_t and all its associated resources
 * \param o The opdis disassembler to free
 * \sa opdis_init
 */

void LIBCALL opdis_term( opdis_t );


/*!
 * \fn opdis_set_defaults( opdis_t )
 * \ingroup configuration
 * \brief Initializes an opdis object to default, sane values.
 * \param o opdis object to re-initialize
 * \sa opdis_init opdis_set_arch opdis_set_x86_syntax opdis_set_error
 * \sa opdis_set_display opdis_set_handler opdis_set_resolver opdis_set_decoder
 * \sa opdis_set_disassembler_options
 * \note The default architecture is i386, and the default syntax is Intel.
 */
void LIBCALL opdis_set_defaults( opdis_t o );

/*!
 * \enum opdis_x86_syntax_t
 * \ingroup configuration
 * \brief Syntax options for x86 disassembly.
 * \note This wraps the libopcodes syntax options, which only include
 *       Intel and AT&T (i.e., no Nasm support).
 */
enum opdis_x86_syntax_t { 
	/* Intel syntax (dest, src) */
	x86_syntax_intel, 
	/* AT&T syntax (src, dest) */
	x86_syntax_att 
};

/*!
 * \fn opdis_set_x86_syntax( opdis_t, opdis_x86_syntax_t )
 * \ingroup configuration
 * \brief Configure the disassembler to use Intel or AT&T syntax.
 * \details Invokes opdist_set_arch() to force the architecture to i386,
 *          and sets the libopcodes print instruction to either
 *          print_insn_i386_intel or print_insn_i386_att.
 * \param o opdis object to configure.
 * \param syntax The syntax option to use.
 * \note This only applied to x86 disassemblers.
 */
void LIBCALL opdis_set_x86_syntax( opdis_t o, opdis_x86_syntax_t syntax );

// set architecture and disassembler to use
// if disassembler_ftype is NULL, the default for arch will be chosen
// default is x86 at&t
// also sets decoder
/*!
 * \fn opdis_set_arch( opdis_t, enum bfd_architecture, disassembler_ftype )
 * \ingroup configuration
 */
void LIBCALL opdis_set_arch( opdis_t o, enum bfd_architecture arch, 
			     disassembler_ftype fn );

/*!
 * \fn opdis_set_disassembler_options( opdis_t, const char * )
 * \ingroup configuration
 */
void LIBCALL opdis_set_disassembler_options( opdis_t o, const char * options );

// instructions are emitted to display
/*!
 * \fn opdis_set_display( opdis_t, OPDIS_DISPLAY, void * )
 * \ingroup configuration
 */
void LIBCALL opdis_set_display( opdis_t o, OPDIS_DISPLAY fn, void * arg );

// instructions are emitted to handler; handler makes decision to continue
/*!
 * \fn opdis_set_handler( opdis_t, OPDIS_HANDLER, void * )
 * \ingroup configuration
 */
void LIBCALL opdis_set_handler( opdis_t o, OPDIS_HANDLER fn, void * arg );

// instructions are submitted to the decoder as an array of strings;
// the decoder fills the instruction object
/*!
 * \fn opdis_set_decoder( opdis_t, OPDIS_DECODER )
 * \ingroup configuration
 */
void LIBCALL opdis_set_decoder( opdis_t o, OPDIS_DECODER fn );

// instruction is emitted to resolver; resolver returns jump/call target
/*!
 * \fn opdis_set_resolver( opdis_t, OPDIS_RESOLVER, void * )
 * \ingroup configuration
 */
void LIBCALL opdis_set_resolver( opdis_t o, OPDIS_RESOLVER fn, void * arg );

/*!
 * \fn opdis_set_error_reporter( opdis_t, OPDIS_ERROR, void * )
 * \ingroup configuration
 */
void LIBCALL opdis_set_error_reporter( opdis_t o, OPDIS_ERROR fn, void * arg );

// size of single insn at address
/*!
 * \fn opdis_disasm_insn_size( opdis_t, opdis_buf_t, opdis_off_t )
 * \ingroup disassembly
 */
size_t LIBCALL opdis_disasm_insn_size( opdis_t o, opdis_buf_t buf, 
				       opdis_off_t offset );

// disasm single insn at address
/*!
 * \fn opdis_disasm_insn(opdis_t, opdis_buf_t, opdis_off_t, opdis_insn_t * )
 * \ingroup disassembly
 */
int LIBCALL opdis_disasm_insn( opdis_t o, opdis_buf_t buf, opdis_off_t offset,
			       opdis_insn_t * insn );

// disasm serial range of addresses
/*!
 * \fn opdis_disasm_linear( opdis_t, opdis_buf_t, opdis_off_t, opdis_off_t )
 * \ingroup disassembly
 */
int LIBCALL opdis_disasm_linear( opdis_t o, opdis_buf_t buf, opdis_off_t offset,
				 opdis_off_t length );
// disasm following cflow
/*!
 * \fn opdis_disasm_cflow( opdis_t, opdis_buf_t, opdis_off_t )
 * \ingroup disassembly
 */
int LIBCALL opdis_disasm_cflow( opdis_t o, opdis_buf_t buf, 
				opdis_off_t offset );

/*!
 * \fn opdis_error( opdis_t, opdis_error_t, const char * )
 */
void LIBCALL opdis_error( opdis_t o, opdis_error_t error, const char * msg );

// todo invariant?
//opdis_( opdis_t * );
#ifdef __cplusplus
}
#endif

#endif
