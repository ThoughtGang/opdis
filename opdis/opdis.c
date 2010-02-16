/*!
 * \file opdis.c
 * \brief Disassembler front-end for libopcodes
 * \author thoughtgang.org
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <opdis/opdis.h>

/* ---------------------------------------------------------------------- */
/* Default callbacks */

int opdis_default_handler( const opdis_insn_t * insn, void * arg ) {
	// add to internal list
	// if already present, return false
	// return true unless invalid insn
	return 1;
}

void opdis_default_display( const opdis_insn_t * i, void * arg ) {
	printf( "%s\n", i->ascii );
}

opdis_vma_t opdis_default_resolver( const opdis_insn_t * insn, void * arg ) {
	// resolve address if relative
	return OPDIS_INVALID_ADDR;
}

void opdis_default_error_reporter( enum opdis_error_t error, const char * msg,
				   void * arg ) {
	char * str;

	switch (error) {
		case opdis_error_bounds:
			str = "memory bounds exceeded"; break;
		case opdis_error_invalid_insn:
			str = "invalid instruction"; break;
		case opdis_error_decode_insn:
			str = "unable to decode instruction"; break;
		case opdis_error_max_items:
			str = "max insn items exceeded"; break;
		case opdis_error_unknown:
		default:
			str = "unknown error"; break;
	}

	fprintf( stderr, "Error (%s): %s\n", str, msg );
}

/* ---------------------------------------------------------------------- */
/* Built-in decoders */

int opdis_default_decoder( const opdis_insn_buf_t in, opdis_insn_t * out,
		           const opdis_byte_t * buf, opdis_off_t offset,
			   opdis_vma_t vma, opdis_off_t length ) {
	opdis_insn_set_ascii( out, in->string );

	out->bytes = &buf[offset ];
	out->size = length;
	out->offset = offset;
	out->vma = vma;

	out->status |= opdis_decode_basic;
	return 1;
}

/* ---------------------------------------------------------------------- */
/* libopcodes callbacks */

static int build_insn_fprintf( void * stream, const char * format, ... ) {
	char str[OPDIS_MAX_ITEM_SIZE];
	int rv;
	/* hack to get around libopcodes' fprintf-only output */
	opdis_t o = (opdis_t) stream;

	va_list args;
	va_start (args, format);
	rv = vsnprintf( str, OPDIS_MAX_ITEM_SIZE - 1, format, args );
	va_end (args);

	opdis_insn_buf_append( o->buf, str );

	return rv;
}

/* this is used only to get the size of an insn: it discards all insn details */
static int null_fprintf( void * f, const char * str, ... ) {
	return 0;
}

/* ---------------------------------------------------------------------- */
/* OPDIS MGT */

opdis_t LIBCALL opdis_init( void ) {
	opdis_t o = (opdis_t) calloc( sizeof(opdis_info_t), 1 );
	
	if ( o ) {
		o->buf = opdis_insn_buf_alloc( 0, 0, 0 );
		init_disassemble_info ( &o->config, o, build_insn_fprintf );
		o->config.application_data = (void *) o;
		// TODO : read memory error fn
		opdis_set_defaults( o );
	}

	return o;
}

void LIBCALL opdis_term( opdis_t o ) {
	if ( o ) {
		free( o );
	}
}

opdis_t LIBCALL opdis_init_from_bfd( bfd * abfd ) {
	return NULL;
}

/* ---------------------------------------------------------------------- */
/* Configuration */
// NOTE: void disassembler_usage (FILE *);

void LIBCALL opdis_set_defaults( opdis_t o ) {
	opdis_set_handler( o, opdis_default_handler, NULL );
	opdis_set_display( o, opdis_default_display, NULL );
	opdis_set_resolver( o, opdis_default_resolver, NULL );
	opdis_set_error_reporter( o, opdis_default_error_reporter, NULL );
	/* note: this sets the decoder */
	opdis_set_x86_syntax( o, opdis_x86_syntax_intel );
}

void LIBCALL opdis_set_disassembler_options( opdis_t o, const char * options ) {
	if (! o ) {
		return;
	}

	if ( o->config.disassembler_options ) {
		free( o->config.disassembler_options );
	}

	o->config.disassembler_options = strdup( options );
}

void LIBCALL opdis_set_x86_syntax( opdis_t o, enum opdis_x86_syntax_t syntax ) {
	disassembler_ftype fn = print_insn_i386_intel;
	// TODO: x86 intel decoder
	OPDIS_DECODER d_fn = opdis_default_decoder; // intel

	if (! o ) {
		return;
	}

	if ( syntax == opdis_x86_syntax_att ) {
		fn = print_insn_i386_att;
		// TODO: x86 att decoder
		d_fn = opdis_default_decoder; // att
	}

	if ( o->config.arch != bfd_arch_i386 ) {
		opdis_set_arch( o, bfd_arch_i386, bfd_mach_i386_i386, fn );
	}

	opdis_set_decoder( o, d_fn, NULL );
}

void LIBCALL opdis_set_arch( opdis_t o, enum bfd_architecture arch, 
			     unsigned long mach, disassembler_ftype fn ) {
	if (! o ) {
		return;
	}

	o->disassembler = fn;
	o->config.arch = arch;
	o->config.mach = mach;
	disassemble_init_for_target( &o->config );

	// TODO: set appropriate decoder!
	opdis_set_decoder( o, opdis_default_decoder, NULL );
}

void LIBCALL opdis_set_display( opdis_t o, OPDIS_DISPLAY fn, void * arg ) {
	if ( o && fn ) {
		o->display = fn;
		o->display_arg = arg;
	}
}

void LIBCALL opdis_set_handler( opdis_t o, OPDIS_HANDLER fn, void * arg ) {
	if ( o && fn ) {
		o->handler = fn;
		o->handler_arg = arg;
	}
}

void LIBCALL opdis_set_decoder( opdis_t o, OPDIS_DECODER fn, void * arg ) {
	if ( o && fn ) {
		o->decoder = fn;
		o->decoder_arg = arg;
	}
}

void LIBCALL opdis_set_resolver( opdis_t o, OPDIS_RESOLVER fn, void * arg ) {
	if ( o && fn ) {
		o->resolver = fn;
		o->resolver_arg = arg;
	}
}

void LIBCALL opdis_set_error_reporter( opdis_t o, OPDIS_ERROR fn, void * arg ) {
	if ( o && fn ) {
		o->error_reporter = fn;
		o->error_reporter_arg = arg;
	}
}

/* ---------------------------------------------------------------------- */
/* Disassemble instruction */

static int buffer_check( opdis_buf_t buf, opdis_vma_t vma ) {
	if ( vma >= buf->vma + buf->len ) {
		char msg[64];
		snprintf( msg, 63, "Offset %d exceeds buffer length %d\n",
			 (int) (vma - buf->vma), (int) buf->len );

		//opdis_error( o, opdis_error_bounds, buf );
		return 0;
	}

	return 1;
}

/* Internal wrapper for libopcodes disassembler used by the three main
 * disasm functions: disasm_insn, disasm_linear, disasm_cflow. */
// NOTE: This requires that opdis_set_buffer() have been called
static unsigned int disasm_single_insn( opdis_t o, opdis_vma_t vma, 
					opdis_insn_t * insn ) {
	unsigned int size;

	o->config.insn_info_valid = 0;
	o->buf->item_count = 0;
	o->buf->string[0] = '\0';

	size = o->disassembler( (bfd_vma) vma, &o->config );
	if (! size ) {
		char msg[32];
		snprintf( msg, 31, "VMA %p: %02X\n", (void *) vma, 
			  o->config.buffer[(vma - o->config.buffer_vma)] );
		opdis_error( o, opdis_error_invalid_insn, msg );
		return 0;
	}

	o->buf->insn_info_valid = o->config.insn_info_valid;
	o->buf->branch_delay_insns = o->config.branch_delay_insns;
	o->buf->data_size = o->config.data_size;
	o->buf->insn_type = o->config.insn_type;
	o->buf->target = o->config.target;
	o->buf->target2 = o->config.target2;

	if (! o->decoder( o->buf, insn, o->config.buffer, 
			  o->config.buffer_vma - vma, vma, size ) ) {
		char msg[64];
		snprintf( msg, 63, "VMA %p: '%s'\n", (void *) vma,
			  o->buf->string );
		opdis_error( o, opdis_error_decode_insn, msg );
		// Note: this is a warning, not an error
	}

	return size;
}

// size of single insn at address
unsigned int LIBCALL opdis_disasm_insn_size( opdis_t o, opdis_buf_t buf, 
					     opdis_vma_t vma ){
	size_t size;
	fprintf_ftype fn = o->config.fprintf_func;
	o->config.fprintf_func = null_fprintf;

	if (! o || ! buf ||! buffer_check( buf, vma ) ) {
		return 0;
	}

	o->config.stream = o;
	size = o->disassembler( vma, &o->config );
	
	o->config.fprintf_func = fn;
	return size;
}

static void set_opdis_buffer( opdis_t o, opdis_buf_t buf ) {
	o->config.buffer_vma = buf->vma;
	o->config.buffer = (bfd_byte *) buf->data;
	o->config.buffer_length = buf->len;
}

// disasm single insn at address
unsigned int LIBCALL opdis_disasm_insn( opdis_t o, opdis_buf_t buf, 
					opdis_vma_t vma, 
					opdis_insn_t * insn ) {
	size_t size;

	if (! o || ! buf ||! buffer_check( buf, vma ) ) {
		return 0;
	}

	set_opdis_buffer( o, buf );
	size = disasm_single_insn( o, vma, insn );
	o->display( insn, o->display_arg );

	return size;
}

/* ---------------------------------------------------------------------- */
/* Disassembler algorithms */

static inline opdis_insn_t * alloc_fixed_insn() {
	// TODO: verify these
	return opdis_insn_alloc_fixed( 128, 32, 16, 32 );
}

int LIBCALL opdis_disasm_linear( opdis_t o, opdis_buf_t buf, opdis_vma_t vma,
				 opdis_off_t length ) {
	opdis_insn_t * insn;
	int cont = 1;
	unsigned int count = 0;
	opdis_off_t idx = 0;
	opdis_off_t pos = vma;
	opdis_off_t max_pos = buf->vma + buf->len;
	length = (length == 0) ? buf->len : length;
	
	insn = alloc_fixed_insn();
	if (! insn ) {
		fprintf( stderr, "Unable to alloc insn\n" );
		return 0;
	}

	set_opdis_buffer( o, buf );
	while ( cont && idx < length && pos < max_pos ) {
		// TODO: clear insn
		unsigned int size = disasm_single_insn( o, pos, insn );
		pos += size;
		idx += size;
		count++;
		o->display( insn, o->display_arg );
		cont = o->handler( insn, o->handler_arg );
	}

	return count;
}

static int disasm_cflow(opdis_t o, opdis_insn_t * insn, opdis_vma_t vma) {
	int cont = 1;
	unsigned int count = 0;
	opdis_off_t pos = vma;
	opdis_off_t max_pos = o->config.buffer_vma + o->config.buffer_length;

	while ( cont && pos < max_pos ) {
		// TODO: clear insn
		unsigned int size = disasm_single_insn( o, pos, insn );
		pos += size;
		count++;

		o->display( insn, o->display_arg );

		// NOTE : handler determines if an address has already been
		//        visited, and if not it adds the insn to the addr
		//        list. this means that the first insn of a branch could
		//        be disassemled, but will not be added. a bit
		//        inefficient, but not too troubling.
		cont = o->handler( insn, o->handler_arg );

		if (! opdis_insn_fallthrough( insn ) ) {
			cont = 0;
		}

		if ( opdis_insn_is_branch( insn ) ) {
			opdis_vma_t vma = o->resolver( insn, o->resolver_arg );
			/* recurse on branch target */
			if ( vma != OPDIS_INVALID_ADDR ) {
				count +=  disasm_cflow( o, insn, vma );
			}
		}
	}

	return count;
}

int LIBCALL opdis_disasm_cflow( opdis_t o, opdis_buf_t buf, opdis_vma_t vma ) {
	opdis_insn_t * insn = alloc_fixed_insn();
	if (! insn ) {
		fprintf( stderr, "Unable to alloc insn\n" );
		return 0;
	}

	set_opdis_buffer( o, buf );

	return disasm_cflow( o, insn, vma );
}

/* ---------------------------------------------------------------------- */
void LIBCALL opdis_error( opdis_t o, enum opdis_error_t error, 
			  const char * msg ) {
	if ( o ) o->error_reporter(error, msg, o->error_reporter_arg );
}


