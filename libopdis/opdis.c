/*!
 * \file opdis.c
 * \brief Disassembler front-end for libopcodes
 * \author thoughtgang.org
 */

#include "opdis.h"

/* ---------------------------------------------------------------------- */
/* Default callbacks */

static int default_handler( const opdis_insn_t * insn, void * arg ) {
	// add to internal list
	// if already present, return false
	// return true
}

static opdis_addr_t default_resolver( const opdis_insn_t * insn ) {
	// resolve address if relative
}

static void default_opdis_error( opdis_error_t error, const char * msg ) {
	char * str;

	switch (error) {
		case opdis_error_bounds:
			str = "memory bounds exceeded"; break;
		case opdis_error_invalid_insn:
			str = "invalid instruction"; break;
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

static int default_decoder( const opdis_insn_raw_t * in, opdis_insn_t * out,
		            const opdis_byte_t * start, opdis_off_t length ) {
	// fill out
	// handler must put bytes into buffer as well?
	return 0;
}

/* ---------------------------------------------------------------------- */
/* fprintf handler */

static int build_insn_fprintf( FILE *stream, const char *format, ... ) {
	char buf[MAX_ITEM_SIZE];
	int rv;
	/* hack to get around libopcodes' fprintf-only output */
	opdis_t o = (opdis_t) stream;

	va_list args;
	va_start (args, format);
	rv = vsnprintf( buf, MAX_ITEM_SIZE - 1, format, args );
	va_end (args);

	/* TODO: handle buf */
	// append to tmp
	// append to insn.ascii

	return rv;
}

/* this is used only to get the size of an insn: it discards all insn details */
static int null_fprintf( FILE *, const char *, ... ) {
	return 0;
}



/* ---------------------------------------------------------------------- */
opdis_t opdis_init( void ) {
	opdis_t o = (opdis_t) calloc( sizeof(opdis_t), 1 );
	
	if ( o ) {
		init_disassemble_info ( &o->info, o, build_insn_fprintf );
		opdis_set_defaults( o );
	}

	return o;
}

void opdis_term( opdis_t o ) {
	if ( o ) {
		free( o );
	}
}

void opdis_init_from_bfd( bfd * ) {
}

/* ---------------------------------------------------------------------- */
/* Configuration */
// NOTE: void disassembler_usage (FILE *);

void opdis_set_defaults( opdis_t o ) {
	opdis_set_handler( o, default_handler );
	opdis_set_resolver( o, default_resolver );
	opdis_set_error_reporter( o, default_error );

	opdis_set_x86_syntax( o, x86_syntax_intel );
}

void opdis_set_x86_syntax( opdis_t o, opdis_x86_syntax_t syntax ) {
	disassembler_ftype fn = print_insn_i386_intel;
	OPDIS_DECODER d_fn = default_decoder; // intel

	if (! o ) {
		return;
	}

	if ( syntax == x86_syntax_att ) {
		fn = print_insn_i386_att;
		d_fn = default_decoder; // att
	}

	opdis_set_arch( o, bfd_arch_i386, fn );
	opdis_set_decoder( d_fn );
}

void opdis_set_arch( opdis_t o, enum bfd_architecture arch, 
		     disassembler_ftype fn ) {
	if (! o ) {
		return;
	}

	o->disassembler = fn;
	o->info.arch = arch;
	disassemble_init_for_target( &o->info );

	opdis_set_decoder( default_decoder );
}

void opdis_set_handler( opdis_t o, OPDIS_HANDLER fn, void * arg ) {
	if (! o ) {
		return;
	}
	
	o->handler = fn;
	o->handler_arg = arg;
}

void opdis_set_decoder( opdis_t o, OPDIS_DECODER fn ) {
	if ( o ) o->decoder = fn;
}

void opdis_set_resolver( opdis_t o, OPDIS_RESOLVER fn ) {
	if ( o ) o->resolver = fn;
}

void opdis_set_error_reporter( opdis_t o, OPDIS_ERROR fn ) {
	if ( o ) o->error_reporter = fn;
}

/* ---------------------------------------------------------------------- */
/* Disassemble instruction */

static int buffer_check( opdis_buf_t * buf, opdis_off_t offset ) {
	if ( buf->data + offset >= buf->len ) {
		char buf[64];
		snprintf( buf, 63, "Offset %d exceeds buffer length %d\n",
			 offset, buf->len );

		opdis_error( o, opdis_error_bounds, buf );
		return 0;
	}

	return 1;
}

// size of single insn at address
size_t opdis_disasm_insn_size( opdis_t o, opdis_buf_t buf, opdis_off_t offset ){
	size_t size;
	OPDIS_DECODER fn = o->fprintf_func;
	o->fprintf_func = null_fprintf;

	if (! o || ! buf ||! buffer_check( buf, offset ) ) {
		return 0;
	}

	size = o->disassembler( &buf->data[offset], &o->info );
	
	o->fprintf_func = fn;
	return size;
}

// disasm single insn at address
int opdis_disasm_insn( opdis_t o, opdis_buf_t buf, opdis_off_t offset,
		       opdis_insn_t * insn ) {
	size_t size;

	if (! o || ! buf ||! buffer_check( buf, offset ) ) {
		return 0;
	}

	size = o->disassembler( &buf->data[offset], &o->info );

	if (! size ) {
		char buf[64];
		snprintf( buf, 63, "Invalid insn at offset %d\n", offset );
		opdis_error( o, opdis_error_bounds, buf );
		return 0;
	}

	// o.decoder( o.raw, insn, &buf->data[offset], size );

	return size;
}

/* ---------------------------------------------------------------------- */
/* Disassembler algorithms */

int opdis_disasm_linear( opdis_t o, opdis_buf_t buf, opdis_off_t offset, 
		         opdis_off_t length ) {
	//cont = o.handler( insn, o.handler_arg );
	return 0;
}

int opdis_disasm_cflow( opdis_t o, opdis_buf_t buf, opdis_off_t offset ) {
	//cont = o.handler( insn, o.handler_arg );
	//addr = o.resolver(insn)
	return 0;
}

/* ---------------------------------------------------------------------- */
void opdis_error( opdis_t o, opdis_error_t error, const char * msg ) {
	if ( o ) o.error_reporter(error, msg );
}
