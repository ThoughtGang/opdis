/*!
 * \file opdis.h
 * \brief Public API for libopdis
 * \details This defines the API for the libopdis library.
 * \author TG Community Developers <community@thoughtgang.org>
 * \note Copyright (c) 2010 ThoughtGang.
 * Released under the GNU Lesser Public License (LGPL), version 2.1.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef OPDIS_H
#define OPDIS_H

#include <dis-asm.h>		/* libopcodes (provided by binutils-dev) */

#include <opdis/types.h>
#include <opdis/insn_buf.h>
#include <opdis/model.h>
#include <opdis/tree.h>

#ifdef WIN32
        #define LIBCALL _stdcall
#else
        #define LIBCALL
#endif


#ifdef __cplusplus
extern "C"
{
#endif

/* ---------------------------------------------------------------------- */

/*!
 * \typedef int (*OPDIS_HANDLER) ( const opdis_insn_t * i, void * arg )
 * \ingroup configuration
 * \brief Callback used to determine if disassembly should continue.
 * \param i The most recently disassembled instruction.
 * \param arg Argument provided when the callback is set
 * \return 0 if disassembly should halt, nonzero (1) otherwise.
 * \details This function is invoked after the display callback. The default 
 * behavior is to halt disassembly only if an invalid instruction is
 * encountered. The caller can override this function in order to 
 * specify more detailed halting conditions, e.g. to halt once a
 * sequence of instructions has been encountered.
 */
typedef int (*OPDIS_HANDLER) ( const opdis_insn_t * i, void * arg );

/*!
 * \fn int opdis_default_handler( const opdis_insn_t *, void * )
 * \ingroup configuration
 * \brief The built-in opdis handler callback.
 * The default handler returns true unless the instruction is invalid or
 * if the address has already been visited.
 * \note The default handler callback takes an opdis_t as the \e arg
 *       parameter. If this parameter is NULL, or if the \e visited_addr
 *       field of the opdis_t is NULL, the handler will not check
 *       check if an address has already been visited.
 */
int opdis_default_handler( const opdis_insn_t * i, void * arg );

/*!
 * \typedef void (*OPDIS_DISPLAY) ( const opdis_insn_t * i, void * arg )
 * \ingroup configuration
 * \brief Callback used to display or store a disassembled instruction
 * \param i The most recently disassembled instruction.
 * \param arg Argument provided when the callback is set
 * \details This function is invoked every time an instruction is disassembled;
 * it emits the instruction to the caller of the disassembler. The default
 * display function writes to STDOUT.
 * \note The caller must copy the instruction if they are going to store it;
 *       the contents of i will be overwritten when the next instruction
 *       is disassembled.
 * \note The default display routine writes instructions to STDOUT without
 *       storing or sorting them. This will lead to unexpected behavior
 *       (i.e. out-of-order instruction listings) when using 
 *       opdis_disasm_cflow(). 
 */
typedef void (*OPDIS_DISPLAY) ( const opdis_insn_t * i, void * arg );

/*!
 * \fn void opdis_default_display ( const opdis_insn_t *, void * )
 * \ingroup configuration
 * \brief  The built-in opdis display callback
 * This callback writes the instruction \e ascii field to STDOUT.
 * \note The default display callback takes a NULL \e arg parameter.
 */
void opdis_default_display ( const opdis_insn_t * i, void * arg );

/*!
 * \typedef int (*OPDIS_DECODER) ( const opdis_insn_buf_t, opdis_insn_t *,
			           opdis_byte_t *, opdis_off_t, opdis_vma_t, 
				   opdis_off_t )
 * \ingroup configuration
 * \brief Callback used to fill an opdis_insn_t from an opdis_insn_buf_t
 * \param in The opdis_insn_buf_t containing the libopcodes output.
 * \param out Pointer to the opdis_insn_t to fill.
 * \param buf Buffer containing the instruction
 * \param offset Offset of start of instruction in \e buf.
 * \param vma Address (vma) of start of instruction.
 * \param length Size of instruction in bytes.
 * \param arg Optional argument to pass to callback
 * \return 0 on failure, nonzero on success. 
 * \details This function is invoked after libopcodes has finished 
 *          disassembling the instruction. The strings emitted from libopcodes
 *          are in the opdis_insn_buf_t; the decoder must use these to build
 *          a valid opdis_insn_t. The default decoder for unsupported
 *   	    architectures will only fill the ascii field of the opdis_insn_t.
 *   	    More sophisticated decoders, such as the decoder for the x86
 *   	    platform, will fill the rest of the object.
 * \sa \ref sec_supported_arch
 * \note The caller will only need to provide a decoder callback if they
 *       are disassembling an architecture not supported by Opdis.
 * \note The default decoder, opdis_default_decoder, should be used by
 *       all decoder callbacks to set the basic instruction info
 *       (ascii, offset, vma, bytes, size).
 */
typedef int (*OPDIS_DECODER) ( const opdis_insn_buf_t in, 
			       opdis_insn_t * out,
			       const opdis_byte_t * buf, 
			       opdis_off_t offset,
			       opdis_vma_t vma,
			       opdis_off_t length,
			       void * arg );

/*!
 * \fn int opdis_default_decoder( const opdis_insn_buf_t, opdis_insn_t *,
			          const opdis_byte_t *, opdis_off_t, 
				  opdis_vma_t, opdis_off_t, void * )
 * \ingroup configuration
 * \brief The built-in opdis instruction decoder
 * This callback fills the \e ascii, \e offset, \e vma, \e bytes,
 * and \e size fields of the output instruction object. It is recommended 
 * that all other decoders invoke this callback directly to fill these fields.
 * \note The default decoder callback takes a NULL \e arg parameter.
 */
int opdis_default_decoder( const opdis_insn_buf_t in, opdis_insn_t * out,
			   const opdis_byte_t * buf, opdis_off_t,
			   opdis_vma_t vma, opdis_off_t length,
			   void * arg );

/*!
 * \typedef opdis_vma_t (*OPDIS_RESOLVER) ( const opdis_insn_t * i, 
 * 					     void * arg )
 * \ingroup configuration
 * \brief Callback used to convert a branch target to a buffer offset.
 * \param i The instruction to resolve.
 * \param arg Argument provided when the callback is set.
 * \return A valid offset in the buffer, or OPDIS_INVALID_ADDR.
 * \details This function is invoked when a branch instruction (call or jmp)
 *          is encountered; it determines the buffer offset of the branch
 *          target. This is used to convert relative addresses, virtual
 *          memory addresses, and registers into buffer offsets. If the
 *          branch target cannot be converted to a memory address (e.g.
 *          register contents are not being tracked, the load address of
 *          the buffer is not known, or the address lies outside the buffer)
 *          then this function must return OPDIS_INVALID_ADDR. The default
 *          resolver only handled relative addresses.
 */
typedef opdis_vma_t (*OPDIS_RESOLVER) ( const opdis_insn_t * i, void * arg );

/*!
 * \fn opdis_vma_t opdis_default_resolver( const opdis_insn_t *, void * )
 * \ingroup configuration
 * \brief The built-in opdis resolver callback.
 * This callback returns the immediate value of the taregt operand for the
 * instruction (if set) or OPDIS_INVALID_ADDR.
 * \note The default resolver callback takes a NULL \e arg parameter.
 */
opdis_vma_t opdis_default_resolver( const opdis_insn_t * i, void * arg );

/*!
 * \enum opdis_error_t
 * \ingroup disassembly
 * \brief Error codes passed to OPDIS_ERROR
 */
enum opdis_error_t { opdis_error_unknown, 
		     opdis_error_bounds,	/*!< Buffer bounds exceeded */ 
		     opdis_error_invalid_insn,	/*!< Invalid instruction */
		     opdis_error_decode_insn,	/*!< Decoding error */
		     opdis_error_bfd,		/*!< BFD error */
		     opdis_error_max_items	/*!< Instruction > insn_buf */ 
		   };

/*!
 * \typedef void (*OPDIS_ERROR) ( enum opdis_error_t error, const char * msg,
			          void * arg )
 * \ingroup configuration
 * \brief Callback used to handle error messages.
 * \param error Type of error.
 * \param msg Detailed message describing error.
 * \param arg Argument provided when the callback is set.
 * \details This function is invoked whenever an error is encountered by the
 *          disassembler. The default error handler writes to STDERR.
 */
typedef void (*OPDIS_ERROR) ( enum opdis_error_t error, const char * msg,
			      void * arg );

/*!
 * \fn void opdis_default_error_reporter( enum opdis_error_t , const char *,
			      		  void * )
 * \ingroup configuration
 * \brief The built-in error reporter.
 * \note The default error reporter takes a NULL \e arg parameter.
 */
void opdis_default_error_reporter( enum opdis_error_t error, const char * msg,
			      void * arg );

/* ---------------------------------------------------------------------- */

/*!
 * \struct opdis_info_t
 * \ingroup configuration
 * \brief An opdis disassembler
 * \note The \e visited_addr tree is NULL by default. Both 
 *       opdis_disasm_cflow and opdis_disasm_bfd_cflow create this tree
 *       if \e visited_addr is NULL. This means that linear disassembly
 *       will not check if an address exists before invoking the display
 *       callback.
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
	 *  \brief callback to build an opdis_insn_t from libopcodes strings.
	 */
	OPDIS_DECODER decoder;
	void * decoder_arg;

	/*! \var buf
	 *  \brief buffer for storing libopcodes strings as they are emitted.
	 */
	opdis_insn_buf_t buf;


	/*! \var visited_addr
	 *  \brief Index of all VMAs that have been disassembled and displayed.
	 *  \details A tree of all instructions that have been disassembled.
	 *   If this is non-NULL, the default handler will check if the VMA
	 *   for the current instruction is in the tree. If not, the instruction
	 *   is added to the tree and the handler returns 1 (i.e. instruction
	 *   will be displayed). Otherwise, the handler returns 0 (do not
	 *   display the instruction, and stop disassembly).
	 *   \note Use of the visited addresses tree will ensure that no
	 *         duplicate instructions are emitted, but will noticably 
	 *         slow down disassembly.
	 */
	opdis_vma_tree_t visited_addr;

	/*! \var debug
	 *  \brief Print debug info to STDERR
	 */
	int debug;
} opdis_info_t;

/*!
 * \typedef opdis_info_t * opdis_t
 * \ingroup configuration
 * \brief Disassembler handle (pointer to opdis_info_t).
 */

typedef opdis_info_t * opdis_t;

/* ---------------------------------------------------------------------- */

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
 * \ingroup bfd
 * \brief Initialize an opdis disassembler based on a BFD object
 * \details Allocates an opdis_t using opdis_init(), then configures libopcodes 
 * based on information (e.g. the architecture) in the BFD.
 * \param target A BFD object to operate on
 * \sa opdis_init opdis_config_from_bfd
 * \return An opdis disassembler object
 * \note This requires that the BFD be configured correctly for the target
 *       architecture. See the documentation for libbfd in the GNU binutils
 *       distribution.
 */
opdis_t LIBCALL opdis_init_from_bfd( bfd * target );

/*!
 * \fn opdis_config_from_bfd( opdis_t, bfd * )
 * \ingroup bfd
 * \brief Use a BFD object to configure an existing opdis disassembler
 * \details Configures libopcodes based on information (e.g. the architecture) 
 *  in the BFD.
 * \param o opdis disassembler to configure.
 * \param target A BFD object to operate on
 * \sa opdis_init opdis_init_from_bfd
 * \return An opdis disassembler object
 * \note This requires that the BFD be configured correctly for the target
 *       architecture. See the documentation for libbfd in the GNU binutils
 *       distribution.
 */
void LIBCALL opdis_config_from_bfd( opdis_t o, bfd * target );


/*!
 * \fn opdis_dupe()
 * \ingroup disassembly
 * \brief Duplicate an opdis disassembler
 * \details Allocates an opdis_t and fills it based on the provided opdis_t. 
 * This is used when running multiple threads in a single target, as one 
 * opdis_t must be used per-thread..
 * \sa opdis_init opdis_term
 * \return An opdis disassembler object
 */

opdis_t LIBCALL opdis_dupe( opdis_t );

/*!
 * \fn opdis_term( opdis_t )
 * \ingroup disassembly
 * \brief Cleanup an opdis disassembler
 * \details Frees an opdis_t and all its associated resources
 * \param o The opdis disassembler to free
 * \sa opdis_init
 */

void LIBCALL opdis_term( opdis_t o );


/*!
 * \fn opdis_set_defaults( opdis_t )
 * \ingroup configuration
 * \brief Initializes an opdis object to default, sane values.
 * \param o opdis disassembler to configure.
 * \sa opdis_init opdis_set_arch opdis_set_x86_syntax opdis_set_error
 * \sa opdis_set_display opdis_set_handler opdis_set_resolver opdis_set_decoder
 * \sa opdis_set_disassembler_options
 * \note The default architecture is i386, and the default syntax is Intel.
 *       All callbacks will be set to the default internal routines.
 */
void LIBCALL opdis_set_defaults( opdis_t o );

typedef void (*OPCODES_INIT) (struct disassemble_info *, void *, fprintf_ftype);

/*!
 * \fn opdis_override_opcodes_init( opdis_t o, OPCODES_INIT fn );
 * \ingroup configuration
 * \brief Invoke a custom libopcodes init_disassemble_info function
 * This invokes the provided init_disassemble_info function in the config
 * member of the opdis_t. This is used to apply a dynamically-loaded libopcodes
 * to an opdis_t created by opdis_init.
 * \param o opdis disassembler to configure.
 * \param fn pointer to a libopcodes init_disassemble_info function
 * \sa opdis_init opdis_set_arch opdis_set_disassembler_options
 * \note This will reset o.config.disassembler, o.config.arch, o.config.mach, 
 * o.config.application_data, and o.config.memory_error_func.
 */
void LIBCALL opdis_override_opcodes_init( opdis_t o, OPCODES_INIT fn );

/*!
 * \enum opdis_x86_syntax_t
 * \ingroup configuration
 * \brief Syntax options for x86 disassembly.
 * \note This wraps the libopcodes syntax options, which only include
 *       Intel and AT&T (i.e. no Nasm support).
 */
enum opdis_x86_syntax_t { 
	opdis_x86_syntax_intel, 	/*!< Intel syntax (dest, src) */
	opdis_x86_syntax_att 		/*!< AT&T syntax (src, dest) */
};

/*!
 * \fn opdis_set_x86_syntax( opdis_t, enum opdis_x86_syntax_t )
 * \ingroup configuration
 * \brief Configure the disassembler to use Intel or AT&T syntax.
 * \details Sets the libopcodes print instruction to either
 *          print_insn_i386_intel or print_insn_i386_att.
 * \param o opdis disassembler to configure.
 * \param syntax The syntax option to use.
 * \note This only applies to x86 disassemblers.
 */
void LIBCALL opdis_set_x86_syntax( opdis_t o, enum opdis_x86_syntax_t syntax );

/*!
 * \fn opdis_set_arch( opdis_t, enum bfd_architecture, unsigned long mach,
 * 		       disassembler_ftype )
 * \ingroup configuration
 * \brief Set the architecture and disassembler routine for libopcodes
 * \details This will set architecture of the libopcodes target to a BFD 
 *          architecture, and specify the libopcodes print_insn routine
 *          that will be used for disassembly.
 * \param o opdis disassembler to configure.
 * \param arch A valid BFD architecture from /usr/include/bfd.h.
 * \param mach A valid bfd_mach definition from /usr/include/bfd.h.
 * \param fn A valid libopcodes print_insn routine from /usr/include/dis-asm.h .
 * \note If the disassembler_ftype parameter is NULL, the default disassembler
 *       for the architecture will be selected. On x86 targets, this is
 *       print_insn_i386_att.
 * \note This sets the decoder to the appropriate built-in decoder for the
 *       architecture. To override the decoder, call opdis_set_decoder after
 *       calling this routine.
 * \note The \e arch parameter is only used when calling
 *       \e disassemble_init_for_target to initialize the libopcodes
 *       \e disassemble_info structure. The \e mach parameter is used by the
 *       disassembler to determine how to decode the instructions. For x86
 *       and x86-64 platforms, \e arch is always \e bfd_arch_i386, and
 *       \e mach is one of the following:
 *       	- bfd_mach_i386_i386
 *       	- bfd_mach_i386_i8086
 *       	- bfd_mach_i386_i386_intel_syntax
 *       	- bfd_mach_x86_64
 *       	- bfd_mach_x86_64_intel_syntax
 *       It is not necessary to specify the Intel syntax in \e mach;
 *       this can be done with \e opdis_set_x86_syntax.
 */
void LIBCALL opdis_set_arch( opdis_t o, enum bfd_architecture arch, 
			     unsigned long mach, disassembler_ftype fn );

/*!
 * \fn opdis_set_disassembler_options( opdis_t, const char * )
 * \ingroup configuration
 * \brief Set libopcodes disassembler options
 * \details Sets the libopcodes disassembler_options config field. Opdis passes
 *          this string blindly to libopcodes. To view the valid options for
 *          a disassembler, invoke the appropriate print_*_disassembler_options
 *          routine listed in /usr/include/dis-asm.h .
 * \param o opdis disassembler to configure.
 * \param options The options string for the disassembler.
 * \note The libopcodes disassembler options override the settings applied
 *       by opdis_set_syntax.
 */
void LIBCALL opdis_set_disassembler_options( opdis_t o, const char * options );

/*!
 * \fn opdis_set_display( opdis_t, OPDIS_DISPLAY, void * )
 * \ingroup configuration
 * \brief Set the callback used to display or store disassembled instructions.
 * \details This sets the function invoked to manage disassembled instructions.
 *          The callback is invoked every time an instruction is disassembled.
 *          It is expected to display or store the instruction, which will be
 *          discarded when the next address is disassembled. The default
 *          handler writes to STDOUT.
 * \param o opdis disassembler to configure.
 * \param fn The callback function.
 * \param arg An optional argument to pass to the callback function.
 * \note If the callback stores the instruction, it must make a copy using
 *       opdis_insn_dupe().
 */
void LIBCALL opdis_set_display( opdis_t o, OPDIS_DISPLAY fn, void * arg );

/*!
 * \fn opdis_set_handler( opdis_t, OPDIS_HANDLER, void * )
 * \ingroup configuration
 * \brief Set the callback used to determine whether to continue disassembly.
 * \details This sets the function invoked to determine whether to continue
 *          linear or control-flow disassembly after each instruction has
 *          been disassembled and displayed. The default handler will halt
 *          disassembly if an invalid instruction has been encountered, or
 *          (for control-flow) if the end of a control-flow branch (i.e. a
 *          return statement) has been reached.
 * \param o opdis disassembler to configure.
 * \param fn The callback function.
 * \param arg An optional argument to pass to the callback function.
 */
void LIBCALL opdis_set_handler( opdis_t o, OPDIS_HANDLER fn, void * arg );

/*!
 * \fn opdis_set_decoder( opdis_t, OPDIS_DECODER, void * )
 * \ingroup configuration
 * \brief Set the callback used to build an opdis_insn_t from libopcodes data.
 * \details This sets the function used to fill an opdis_insn_t based on the
 *          array of strings (stored in an opdis_insn_buf_t) emitted by
 *          libopcodes. If the default handler does not support the ISA
 *          being disassembled (i.e. an architecture other than x86 is
 *          being used), then this must be overridden in order to get
 *          more than the ASCII version of the instruction in the
 *          opdis_insn_t.
 * \param o opdis disassembler to configure.
 * \param fn The callback function.
 * \param arg An optional argument to pass to the callback function.
 */
void LIBCALL opdis_set_decoder( opdis_t o, OPDIS_DECODER fn, void * arg );

/*!
 * \fn opdis_set_resolver( opdis_t, OPDIS_RESOLVER, void * )
 * \ingroup configuration
 * \brief Set the callback used to obtain the buffer offset of a branch target.
 * \details This sets the function used to convert the target of a branch
 *          to an offset into the buffer. The control flow disassembler will 
 *          require a resolver in order to disassemble all non-relative branch 
 *          targets.
 * \param o opdis disassembler to configure.
 * \param fn The callback function.
 * \param arg An optional argument to pass to the callback function.
 * \note The resolver receives an instruction and is expected to return
 *       a valid offset or -1 if no address can be determined. The
 *       resolver can be a subsystem that manages the stack and registers
 *       in order to provide more complete branch resolution.
 */
void LIBCALL opdis_set_resolver( opdis_t o, OPDIS_RESOLVER fn, void * arg );

/*!
 * \fn opdis_set_error_reporter( opdis_t, OPDIS_ERROR, void * )
 * \ingroup configuration
 * \brief Set the callback used to report errors.
 * \details This sets the function used to report errors encountered during
 *          disassembly. The default error reporter simply writes to STDERR.
 * \param o opdis disassembler to configure.
 * \param fn The callback function.
 * \param arg An optional argument to pass to the callback function.
 */
void LIBCALL opdis_set_error_reporter( opdis_t o, OPDIS_ERROR fn, void * arg );

/*!
 * \fn opdis_disasm_insn_size( opdis_t, opdis_buf_t, opdis_vma_t )
 * \ingroup disassembly
 * \brief Return the size of the instruction at an offset in the buffer.
 * \param o opdis disassembler
 * \param buf The buffer to disassemble
 * \param vma The address (VMA) in the buffer to disassemble.
 * \note If the vma of \e buf is 0, then \e vma is the offset into the buffer.
 */
unsigned int LIBCALL opdis_disasm_insn_size( opdis_t o, opdis_buf_t buf, 
					     opdis_vma_t vma );

// TODO: opdis_disam_invariant
//       * wraps decoder
//       * after decode, find addr arguments in insns bytes (search backwards
//         from end) and wildcard them out of bytes to create signature
/*!
 * \fn opdis_disasm_insn( opdis_t, opdis_buf_t, opdis_vma_t, opdis_insn_t * )
 * \ingroup disassembly
 * \brief Disassemble a single instruction in the buffer
 * \param o opdis disassembler
 * \param buf The buffer to disassemble
 * \param vma The address (VMA) in the buffer to disassemble.
 * \param insn The op_insn_t to fill with the disassembled instruction
 * \note If the vma of \e buf is 0, then \e vma is the offset into the buffer.
 */
unsigned int LIBCALL opdis_disasm_insn( opdis_t o, opdis_buf_t buf, 
					opdis_vma_t vma, 
					opdis_insn_t * insn );

/*!
 * \fn opdis_disasm_linear( opdis_t, opdis_buf_t, opdis_vma_t, opdis_off_t )
 * \ingroup disassembly
 * \brief Disassemble a sequence of instructions in order.
 * \param o opdis disassembler
 * \param buf The buffer to disassemble
 * \param vma The address (VMA) in the buffer to start disassembly at.
 * \param length The number of bytes to disassemble.
 * \note If the vma of \e buf is 0, then \e vma is the offset into the buffer.
 * \note If \e length is zero, then all bytes from \e offset to the end of
 *       the buffer will be disassembled.
 */
int LIBCALL opdis_disasm_linear( opdis_t o, opdis_buf_t buf, opdis_vma_t vma,
				 opdis_off_t length );
/*!
 * \fn opdis_disasm_cflow( opdis_t, opdis_buf_t, opdis_vma_t )
 * \ingroup disassembly
 * \brief Disassemble a buffer following flow of control.
 * \param o opdis disassembler
 * \param buf The buffer to disassemble
 * \param vma The address (VMA) of the entry point in the buffer
 * \note If the vma of \e buf is 0, then \e vma is the offset into the buffer.
 */
int LIBCALL opdis_disasm_cflow( opdis_t o, opdis_buf_t buf, 
				opdis_vma_t vma );
/*!
 * \fn opdis_disasm_insn( opdis_t, bfd *, opdis_vma_t, opdis_insn_t * )
 * \ingroup bfd
 * \brief Disassemble a single instruction in a BFD
 * \param o opdis disassembler
 * \param abfd The BFD to disassemble
 * \param vma The address (VMA) in the BFD to start disassembly at.
 * \param insn The op_insn_t to fill with the disassembled instruction
 * \note If the vma of \e buf is 0, then \e vma is the offset into the buffer.
 */
unsigned int LIBCALL opdis_disasm_bfd_insn( opdis_t o, bfd * abfd, 
					    opdis_vma_t vma, 
					    opdis_insn_t * insn );
/*!
 * \fn opdis_disasm_bfd_linear( opdis_t, bfd *, opdis_vma_t, opdis_off_t )
 * \ingroup bfd
 * \brief Disassemble a sequence of instructions in a BFD.
 * \param o opdis disassembler
 * \param abfd The BFD to disassemble
 * \param vma The address (VMA) in the BFD to start disassembly at.
 * \param length The number of bytes to disassemble.
 * \note If \e length is zero, then all bytes from \e offset to the end of
 *       the BFD will be disassembled.
 */
int LIBCALL opdis_disasm_bfd_linear( opdis_t o, bfd * abfd, opdis_vma_t vma,
				     opdis_off_t length );
/*!
 * \fn opdis_disasm_bfd_cflow( opdis_t, bfd *, opdis_vma_t )
 * \ingroup bfd
 * \brief Disassemble a contents of a BFD following flow of control.
 * \param o opdis disassembler
 * \param abfd The BFD to disassemble
 * \param vma The address (VMA) of the entry point in the BFD
 */
int LIBCALL opdis_disasm_bfd_cflow( opdis_t o, bfd * abfd, opdis_vma_t vma );

/*!
 * \fn opdis_disasm_bfd_section( opdis_t, asection * )
 * \ingroup bfd
 * \brief Disassemble a the contents of a BFD section using linear disassembly.
 * \param o opdis disassembler
 * \param sec The section to disassemble
 */
int LIBCALL opdis_disasm_bfd_section( opdis_t o, asection * sec );

/*!
 * \fn opdis_disasm_bfd_symbol( opdis_t, asymbol * )
 * \ingroup bfd
 * \brief Disassemble a BFD following flow of control from a symbol.
 * \param o opdis disassembler
 * \param sym The BFD symbol to start disassembly at.
 */
int LIBCALL opdis_disasm_bfd_symbol( opdis_t o, asymbol * sym );

/*!
 * \fn opdis_disasm_bfd_entry( opdis_t, bfd * )
 * \ingroup bfd
 * \brief Disassemble a BFD following flow of control from entry point (_start).
 * \param o opdis disassembler
 * \param abfd The BFD to disassemble
 **/
int LIBCALL opdis_disasm_bfd_entry( opdis_t o, bfd * abfd );

/*!
 * \fn opdis_error( opdis_t, enum opdis_error_t, const char * )
 * \ingroup disassembly
 * \brief Send an error message to the error reporter/
 * \details This is used internally and by callbacks to report errors.
 * \param o opdis disassembler
 * \param error The error code
 * \param msg The detailed error message
 */
void LIBCALL opdis_error( opdis_t o, enum opdis_error_t error, 
			  const char * msg );

/*!
 * \fn opdis_debug( opdis_t, int, const char *, ... )
 * \ingroup internal
 * \brief Print debug message to STDERR
 */
void LIBCALL opdis_debug( opdis_t o, int min_level, const char * format, ... );

#ifdef __cplusplus
}
#endif

#endif
