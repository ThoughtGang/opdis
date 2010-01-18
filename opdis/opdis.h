/*!
 * \file opdis.h
 * \brief Disassembler front-end for libopcodes
 * \author thoughtgang.org
 */

#ifndef OPDIS_H
#define OPDIS_H

#include <dis-asm.h>		/* libopcodes (provided by binutils-dev) */

#include <opdis/insn>
#include <opdis/insn_buf.h>

#ifdef WIN32
        #define LIBCALL _stdcall
#else
        #define LIBCALL
#endif

typedef bfd_byte opdis_byte_t;
typedef size_t opdis_off_t;
typedef bfd_vma opdis_addr_t;

typedef struct {
	opdis_byte_t * 		data;
	opdis_off_t 		len;
} opdis_buffer_t;
typedef opdis_buffer_t * opdis_buf_t;

/* ---------------------------------------------------------------------- */

enum opdis_x86_syntax_t syntax { x86_syntax_intel, x86_syntax_att };
enum opdis_error_t { opdis_error_unknown, 
		     opdis_error_bounds, 
		     opdis_error_invalid_insn,
		     opdis_error_max_items };

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

opdis_t opdis_init( void );

void opdis_term( opdis_t );

void opdis_init_from_bfd( opdist_t, bfd * );

/*!
 * \fn opdis_set_defaults
 * \brief Initializes an opdis object to default, sane values.
 * \param o opdis object to re-initialize
 * \relates opdis_init
 * \sa opdis_set_arch
 * \note The default architecture is i386, and the defaul syntax is Intel.
 */
void opdis_set_defaults( opdis_t o );

// convenience funtion to set AT&T vs INTEL syntax
// this performs a setarch to x86 and uses the appropriate intel print_insn
void opdis_set_x86_syntax( opdis_t, opdis_x86_syntax_t syntax );

// set architecture and disassembler to use
// if disassembler_ftype is NULL, the default for arch will be chosen
// default is x86 at&t
// also sets decoder
void opdis_set_arch( opdis_t, enum bfd_architecture, disassembler_ftype );

void opdis_set_disassembler_options( opdis_t, const char * options );

// instructions are emitted to handler; handler makes decision to continue
void opdis_set_handler( opdis_t, OPDIS_HANDLER, void * arg );

// instructions are submitted to the decoder as an array of strings;
// the decoder fills the instruction object
void opdis_set_decoder( opdis_t, OPDIS_DECODER );

// instruction is emitted to resolver; resolver returns jump/call target
void opdis_set_resolver( opdis_t, OPDIS_RESOLVER );

void opdis_set_error_reporter( opdis_t, OPDIS_ERROR );

// size of single insn at address
size_t opdis_disasm_insn_size( opdis_t, opdis_buf_t buf, opdis_off_t offset );

// disasm single insn at address
int opdis_disasm_insn( opdis_t, opdis_buf_t buf, opdis_off_t offset,
		       opdis_insn_t * insn );

// disasm serial range of addresses
int opdis_disasm_linear( opdis_t, opdis_buf_t buf, opdis_off_t offset, 
		         opdis_off_t length );
// disasm following cflow
int opdis_disasm_cflow( opdis_t, opdis_buf_t buf, opdis_off_t offset );

void opdis_error( opdis_t, opdis_error_t, const char * );

// todo invariant?
//opdis_( opdis_t * );
#ifdef __cplusplus
}
#endif

#endif
