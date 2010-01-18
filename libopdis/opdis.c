/*!
 * \file opdis.c
 * \brief Disassembler front-end for libopcodes
 * \author thoughtgang.org
 */

#include <dis-asm.h>

#include "opdis.h"

opdis_t opdis_init( void );

int opdis_term( opdis_t );

void opdis_set_syntax( opdis_t );

// instruction is emitted to resolver; resolver returns jump/call target
void opdis_set_resolver( opdis_t );

// instructions are emitted to handler; handler makes decision to continue
void opdis_set_handler( opdis_t );

// size of single insn at address
size_t opdis_disasm_insn_size( opdis_t, opdis_buf_t buf, opdis_off_t offset );

// disasm single insn at address
int opdis_disasm_insn( opdis_t, opdis_buf_t buf, opdis_off_t offset ) {
	void init_disassemble_info (struct disassemble_info *info, void *stream,
			            fprintf_ftype fprintf_func);
	// after setting info->arch:
	void disassemble_init_for_target (struct disassemble_info * info);
	// void disassembler_usage (FILE *);
	save info stream
	set info stream to 'this'
	invoke print_insn or whatnot
	note handler should sprintf what it gets into a buf
	handler must put bytes into buffer as well?
	clean up insn (decode) and call handler
	restore info stream
}

// disasm serial range of addresses
int opdis_disasm_linear( opdis_t, opdis_buf_t buf, opdis_off_t offset, 
		         opdis_off_t length );
// disasm following cflow
int opdis_disasm_cflow( opdis_t, opdis_buf_t buf, opdis_off_t offset );

