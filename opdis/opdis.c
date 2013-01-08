/*!
 * \file opdis.c
 * \brief Disassembler front-end for libopcodes
 * \author TG Community Developers <community@thoughtgang.org>
 * \note Copyright (c) 2010 ThoughtGang.
 * Released under the GNU Lesser Public License (LGPL), version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <opdis/opdis.h>
#include <opdis/x86_decoder.h>

void opdis_debug( opdis_t o, int min_level, const char * format, ... ) {
	if (  o->debug >= min_level ) {
		va_list args;
		va_start (args, format);

		fprintf( stderr, "[DEBUG] " );
		vfprintf( stderr, format, args );
		fprintf( stderr, "\n" );

		va_end (args);
	}
}

/* ---------------------------------------------------------------------- */
/* Default callbacks */

int opdis_default_handler( const opdis_insn_t * insn, void * arg ) {
	opdis_t o = (opdis_t) arg;

	if ( insn->status == opdis_decode_invalid ) {
		/* invalid instruction */
		return 0;
	}

	if (! o || ! o->visited_addr ) {
		/* visited addresses not being tracked */
		return 1;
	}

	/* returns 0 if address already exists in tree */
	return opdis_vma_tree_add( o->visited_addr, insn->vma );
}

void opdis_default_display( const opdis_insn_t * i, void * arg ) {
	printf( "%s\n", i->ascii );
}

opdis_vma_t opdis_default_resolver( const opdis_insn_t * insn, void * arg ) {
	/* NOTE : More sophisticated resolvers can handle register contents
	 *        and address expressions, or can validate that the target
	 *        is a valid VMA. */
	if ( insn->status & opdis_decode_ops && insn->target ) {
		opdis_op_t * dest = insn->target;
		if ( dest->category == opdis_op_cat_immediate ) {
			return dest->value.immediate.vma;
		}
	}

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
			   opdis_vma_t vma, opdis_off_t length, void * arg ) {
	opdis_insn_set_ascii( out, in->string );


	if (! out->bytes ) {
		out->bytes = calloc(1, length);
		if (! out->bytes ) {
			return 0;
		}
	}
	memcpy( out->bytes, &buf[offset], length );

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

static void report_memory_error( int status, bfd_vma vma, 
				 struct disassemble_info * info ) {
	char msg[48];
	opdis_t o = (opdis_t) info->application_data;
	if (! o ) {
		return;
	}

	snprintf( msg, 47, "VMA %p (status %d)\n", (void *) vma, status );

	opdis_error( o, opdis_error_bounds, msg );
}

/* ---------------------------------------------------------------------- */
/* OPDIS MGT */

opdis_t LIBCALL opdis_init( void ) {
	opdis_t o = (opdis_t) calloc( sizeof(opdis_info_t), 1 );
	
	if ( o ) {
		o->buf = opdis_insn_buf_alloc( 0, 0, 0 );
		init_disassemble_info ( &o->config, o, build_insn_fprintf );
		o->config.application_data = (void *) o;
		o->config.memory_error_func = report_memory_error;
		opdis_set_defaults( o );
	}

	return o;
}

opdis_t LIBCALL opdis_dupe( opdis_t src ) {
	if (! src ) {
		return opdis_init();
	}

	opdis_t o = (opdis_t) calloc( sizeof(opdis_info_t), 1 );
	
	if ( o ) {
		o->buf = opdis_insn_buf_alloc( 0, 0, 0 );

		memcpy( &o->config, &src->config, sizeof(disassemble_info) );
		o->config.application_data = (void *) o;

		o->disassembler = src->disassembler;
		o->error_reporter = src->error_reporter;
		o->error_reporter_arg = src->error_reporter_arg;
		o->display = src->display;
		o->display_arg = src->display_arg;
		o->handler = src->handler;
		o->handler_arg = src->handler_arg;
		o->resolver = src->resolver;
		o->resolver_arg = src->resolver_arg;
		o->decoder = src->decoder;
		o->decoder_arg = src->decoder_arg;
		o->debug = src->debug;

		/* NOTE: this is not threadsafe, but we don't really care;
		 *       it just means an insn might be disassembled twice */
		o->visited_addr = src->visited_addr;
	}

	return o;
}

void LIBCALL opdis_term( opdis_t o ) {
	if ( o ) {
		opdis_insn_buf_free(o->buf);
		free( o );
	}
}

void LIBCALL opdis_config_from_bfd( opdis_t o, bfd * abfd ) {
	if ( o ) {
		o->config.flavour = bfd_get_flavour(abfd);
		o->config.endian = abfd->xvec->byteorder;

		opdis_set_arch( o, bfd_get_arch(abfd), bfd_get_mach(abfd),
				disassembler(abfd) );
	}
}

opdis_t LIBCALL opdis_init_from_bfd( bfd * abfd ) {
	opdis_t o = opdis_init();
	
	opdis_config_from_bfd( o, abfd );

	return o;
}

/* ---------------------------------------------------------------------- */
/* Configuration */

void LIBCALL opdis_set_defaults( opdis_t o ) {
	opdis_set_handler( o, opdis_default_handler, o );
	opdis_set_display( o, opdis_default_display, NULL );
	opdis_set_resolver( o, opdis_default_resolver, NULL );
	opdis_set_error_reporter( o, opdis_default_error_reporter, NULL );
	opdis_set_arch( o, bfd_arch_i386, bfd_mach_i386_i386, NULL );
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
	OPDIS_DECODER d_fn = opdis_x86_intel_decoder;

	if (! o ) {
		return;
	}

	if ( syntax == opdis_x86_syntax_att ) {
		fn = print_insn_i386_att;
		d_fn = opdis_x86_att_decoder;
	}

	/* force x86 architecture */
	o->config.arch = bfd_arch_i386;

	o->disassembler = fn;

	opdis_set_decoder( o, d_fn, o );
}

void LIBCALL opdis_set_arch( opdis_t o, enum bfd_architecture arch, 
			     unsigned long mach, disassembler_ftype fn ) {
	OPDIS_DECODER d_fn = opdis_default_decoder;

	if (! o ) {
		return;
	}

	o->disassembler = fn;
	o->config.arch = arch;
	o->config.mach = mach;
	disassemble_init_for_target( &o->config );

	/* set appropriate decoder for architecture */
	if ( arch == bfd_arch_i386 ) {
		/* default to ATT syntax and decoder */
		opdis_set_x86_syntax( o, opdis_x86_syntax_att );
	} else {
		// TODO: else switch on arch, setting d_fn 
		opdis_set_decoder( o, d_fn, o );
	}
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

/* Internal wrapper for libopcodes disassembler used by the three main
 * disasm functions: disasm_insn, disasm_linear, disasm_cflow. */
// NOTE: This requires that opdis_set_buffer() have been called
static unsigned int disasm_single_insn( opdis_t o, opdis_vma_t vma, 
					opdis_insn_t * insn ) {
	unsigned int size;

	o->config.insn_info_valid = 0;
	o->buf->item_count = 0;
	o->buf->string[0] = '\0';
	opdis_insn_clear( insn );

	o->config.stream = o;
	size = o->disassembler( (bfd_vma) vma, &o->config );
	if (! size ) {
		char msg[32];
		snprintf( msg, 31, "VMA %p: %02X\n", (void *) vma, 
			  o->config.buffer[(vma - o->config.buffer_vma)] );
		opdis_error( o, opdis_error_invalid_insn, msg );
		return 0;
	}

	opdis_debug( o, 3, "Disassembled %d bytes at %p", size, (void *) vma );

	opdis_debug( o, 4, "%p : %s", (void *) vma, o->buf->string );


	/* fill insn_buf with libopcodes meta-info */
	o->buf->insn_info_valid = o->config.insn_info_valid;
	o->buf->branch_delay_insns = o->config.branch_delay_insns;
	o->buf->data_size = o->config.data_size;
	o->buf->insn_type = o->config.insn_type;
	o->buf->target = o->config.target;
	o->buf->target2 = o->config.target2;

	if (! o->decoder( o->buf, insn, o->config.buffer, 
			  vma - o->config.buffer_vma, vma, size,
			  o->decoder_arg ) ) {
		char msg[64];
		snprintf( msg, 63, "VMA %p: '%s'\n", (void *) vma,
			  o->buf->string );
		opdis_error( o, opdis_error_decode_insn, msg );
		// Note: this is a warning, not an error
	}

	/* clear insn buffer now that decoding has taken place */
	opdis_insn_buf_clear( o->buf );

	return size;
}

// size of single insn at address
unsigned int LIBCALL opdis_disasm_insn_size( opdis_t o, opdis_buf_t buf, 
					     opdis_vma_t vma ){
	size_t size;
	fprintf_ftype fn = o->config.fprintf_func;
	o->config.fprintf_func = null_fprintf;

	if (! o || ! buf  ) {
		return 0;
	}

	o->config.stream = o;
	size = o->disassembler( vma, &o->config );
	
	o->config.fprintf_func = fn;
	return size;
}

static void set_opdis_buffer( opdis_t o, opdis_buf_t buf ) {
	opdis_debug( o, 2, "Buffer VMA %p size %d\n", (void *) buf->vma,
		     buf->len );
	o->config.buffer_vma = buf->vma;
	o->config.buffer = (bfd_byte *) buf->data;
	o->config.buffer_length = buf->len;
}

unsigned int LIBCALL opdis_disasm_insn( opdis_t o, opdis_buf_t buf, 
					opdis_vma_t vma, 
					opdis_insn_t * insn ) {
	size_t size;

	if (! o || ! buf  ) {
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
	// TODO: verify these values across architectures
	//       or use per-arch values
	return opdis_insn_alloc_fixed( 128, 32, 16, 32 );
}

static int disasm_linear( opdis_t o, opdis_vma_t vma, opdis_off_t length ) {
	opdis_insn_t * insn;
	int cont = 1;
	unsigned int count = 0;
	opdis_off_t pos = vma;
	length = (length == 0) ? o->config.buffer_length : length;
	opdis_off_t max_pos = o->config.buffer_vma + length;

	insn = alloc_fixed_insn();
	if (! insn ) {
		fprintf( stderr, "Unable to alloc insn\n" );
		return 0;
	}

	opdis_debug( o, 1, "Start linear from %p max %p", (void *) vma,
		     (void *) max_pos );

	while ( cont && pos < max_pos ) {
		unsigned int size = disasm_single_insn( o, pos, insn );
		pos += size;
		if ( pos - vma > length ) {
			opdis_debug( o, 1, "Instruction at %p exceeds buffer", 
				    (void *) vma );
			break;
		}
		count++;
		o->display( insn, o->display_arg );
		cont = o->handler( insn, o->handler_arg );
	}

	opdis_debug( o, 1, "End linear %p (count %d)", (void *) vma, count );

	return count;
}

int LIBCALL opdis_disasm_linear( opdis_t o, opdis_buf_t buf, opdis_vma_t vma,
				 opdis_off_t length ) {
	set_opdis_buffer( o, buf );

	return disasm_linear( o, vma, length );
}

static int disasm_cflow(opdis_t o, opdis_vma_tree_t tree, opdis_vma_t vma) {
	int cont = 1;
	unsigned int count = 0;
	opdis_off_t pos = vma;
	opdis_off_t max_pos = o->config.buffer_vma + o->config.buffer_length;
	opdis_insn_t * insn = alloc_fixed_insn();

	if (! insn ) {
		fprintf( stderr, "Unable to alloc insn\n" );
		return 0;
	}

	if ( pos < o->config.buffer_vma ) {
		return 0;
	}

	opdis_debug( o, 1, "Start cflow from %p max %p", (void *) vma,
		     (void *) max_pos );

	while ( cont && pos < max_pos ) {
		unsigned int size = disasm_single_insn( o, pos, insn );
		pos += size;
		count++;

		/* NOTE : handler determines if an address has already been
		 *        visited, and if not it adds the insn to the addr
		 *        list. this means that the first insn of a branch could
		 *        be disassembled, but will not be added. a bit
		 *        inefficient, but not too troubling. */
		cont = o->handler( insn, o->handler_arg );

		if ( cont ) {
			o->display( insn, o->display_arg );
		} else {
			opdis_debug( o, 2, "VMA %p invalid or already visited",
			             (void *) pos );
		}

		if (! opdis_insn_fallthrough( insn ) ) {
			opdis_debug( o, 2, "CFLOW BRANCH END: %s",
				     insn->ascii );
			cont = 0;
		}

		if (! opdis_insn_is_branch( insn ) ) {
			/* no need for further processing */
			continue;
		}

		opdis_vma_t vma = o->resolver( insn, o->resolver_arg );
		/* recurse on branch target */
		if ( vma == OPDIS_INVALID_ADDR ) {
			opdis_debug( o, 2, "Cannot Resolve: %s", insn->ascii );
		} else if ( vma < o->config.buffer_vma || vma >= max_pos ) {
			opdis_debug( o, 2, "Branch target %p not in buffer %p", 
				(void *) vma, (void *) o->config.buffer_vma );
		} else if ( opdis_vma_tree_add( tree, vma ) ) {
			opdis_debug( o, 2, "CFLOW BRANCH START: %p", 
				     (void *) vma );
			count +=  disasm_cflow( o, tree, vma );
		} else {
			opdis_debug( o, 3, "VMA %p already visited\n",
				     (void *) vma );
		}
	}

	opdis_debug( o, 1, "End cflow %p (count %d)", (void *) vma, count );

	return count;
}

int LIBCALL opdis_disasm_cflow( opdis_t o, opdis_buf_t buf, opdis_vma_t vma ) {
	int count;
	opdis_vma_tree_t tree = opdis_vma_tree_init();

	set_opdis_buffer( o, buf );

	count = disasm_cflow( o, tree, vma );

	opdis_vma_tree_free( tree );

	return count;
}

/* ---------------------------------------------------------------------- */
/* BFD interface */

struct BFD_VMA_SECTION {
	bfd_vma vma;
	asection * sec;
};

static int load_section( opdis_t o, asection * s ) {
	int size;
	unsigned char *buf;
	opdis_vma_t vma;

	size = bfd_section_size( s->owner, s );
	vma = bfd_section_vma( s->owner, s );
	buf = calloc( size, 1 );
	if (! buf || ! bfd_get_section_contents( s->owner, s, buf, 0, size ) ) {
		char msg[32];
		snprintf( msg, 31, "Unable to get section %s\n", s->name );
		opdis_error( o, opdis_error_bfd, msg );
		return 0;
	}

	opdis_debug( o, 2, "Loaded section of %d bytes [%p-%p]\n", size,
		     (void *) vma, (void *) (vma + size - 1) );
	o->config.section = s;
	o->config.buffer = buf;
	o->config.buffer_length = size;
	o->config.buffer_vma = vma;

	return 1;
}

static void vma_in_section( bfd * abfd, asection *s, PTR data ){
	struct BFD_VMA_SECTION * req = (struct BFD_VMA_SECTION *) data;
	if ( req && req->vma >= s->vma && req->vma < 
	     (s->vma + bfd_section_size(abfd, s)) ) {
		req->sec = s;
	}
}

static int load_section_for_vma( opdis_t o, bfd * abfd, bfd_vma vma ){
	int size;
	unsigned char * buf;
	struct BFD_VMA_SECTION req = { vma, NULL };

	bfd_map_over_sections( abfd, vma_in_section, & req );
	if (! req.sec ) {
		char msg[32];
		snprintf( msg, 31, "No section for VMA %p\n", (void *) vma );
		opdis_error( o, opdis_error_bfd, msg );
		return 0;
	}

	if (! load_section( o, req.sec ) ) {
		return 0;
	}

	return 1;
}

unsigned int LIBCALL opdis_disasm_bfd_insn( opdis_t o, bfd * abfd, 
					    opdis_vma_t vma, 
					    opdis_insn_t * insn ) {
	size_t size;

	if (! o || ! abfd ) {
		return 0;
	}

	if (! load_section_for_vma(o, abfd, vma) ) {
		return 0;
	}

	size = disasm_single_insn( o, vma, insn );
	o->display( insn, o->display_arg );

	free( o->config.buffer );
	o->config.buffer = NULL;

	return size;
}

int LIBCALL opdis_disasm_bfd_linear( opdis_t o, bfd * abfd, opdis_vma_t vma,
				     opdis_off_t length ) {
	int count;
	if (! o || ! abfd ) {
		return 0;
	}

	if (! load_section_for_vma(o, abfd, vma) ) {
		return 0;
	}

	count = disasm_linear( o, vma, length );

	free( o->config.buffer );
	o->config.buffer = NULL;

	return count;
}

int LIBCALL opdis_disasm_bfd_cflow( opdis_t o, bfd * abfd, opdis_vma_t vma ) {
	int count;
	opdis_vma_tree_t tree;

	if (! o || ! abfd ) {
		return 0;
	}

	if (! load_section_for_vma(o, abfd, vma) ) {
		return 0;
	}

	tree = opdis_vma_tree_init();

	count = disasm_cflow( o, tree, vma );

	opdis_vma_tree_free( tree );

	free( o->config.buffer );
	o->config.buffer = NULL;

	return count;
}


int LIBCALL opdis_disasm_bfd_section( opdis_t o, asection * sec ) {
	int count = 0;
	if (! o || ! sec ) {
		return 0;
	}

	if ( load_section( o, sec ) ) {
		count = disasm_linear( o, bfd_section_vma(sec->owner, sec), 0 );
		free( o->config.buffer );
		o->config.buffer = NULL;
	}
	return count;
}


int LIBCALL opdis_disasm_bfd_symbol( opdis_t o, asymbol * sym ) {
	int count;
	opdis_vma_tree_t tree;
	asection * sec = sym->section;
	if (! o || ! sym || ! sec ) {
		return 0;
	}

	if ( load_section( o, sec ) ) {
		symbol_info info;
		bfd_symbol_info( sym, &info );

		tree = opdis_vma_tree_init();
		count = disasm_cflow( o, tree, info.value );
		opdis_vma_tree_free( tree );

		free( o->config.buffer );
		o->config.buffer = NULL;
	}
	return count;
}


int LIBCALL opdis_disasm_bfd_entry( opdis_t o, bfd * abfd ) {
	if (! o || ! abfd ) {
		return 0;
	}
	return opdis_disasm_bfd_cflow( o, abfd, bfd_get_start_address(abfd) );
}


/* ---------------------------------------------------------------------- */
void LIBCALL opdis_error( opdis_t o, enum opdis_error_t error, 
			  const char * msg ) {
	if ( o ) o->error_reporter(error, msg, o->error_reporter_arg );
}

