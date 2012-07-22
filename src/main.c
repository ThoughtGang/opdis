/* main.c
 * Startup routine for opdis command-line utility
 * Copyright (c) 2010 ThoughtGang
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <argp.h>		/* glibc command line option parser */
#include <bfd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <opdis/opdis.h>

#include "asm_format.h"
#include "job_list.h"
#include "map.h"
#include "target_list.h"

/* ---------------------------------------------------------------------- */
/* ARGUMENTS AND DOC */

const char * argp_program_version = "opdis 1.0";
const char * argp_program_bug_address = "<dev@thoughtgang.org>";
static const char usage_str[] = "[FILE]...";
static const char help_str[] = 
/* brief description: */
"Opdis command-line disassembler" 
"\v" /* delim */ 
/* detailed documentation:  */
"Disassembler based on libopcodes.\n"
"  memspec = [target]:offset|@vma[+size]\n"
"  bfdname = [target:]name\n"
"  mapspec = [target]:offset@vma[+size]\n"
"  target  = ID (#) of target; use --dry-run to see IDs\n" 
"  fmtspec = asm|dump|delim|xml|fmt_str\n"
;


/* ---------------------------------------------------------------------- */
/* RUNTIME OPTIONS */

static struct argp_option options[] = {
	{ 0, 0, 0, OPTION_DOC, "Basic Options:" },
	{ "cflow", 'c', "memspec", 0,
	  "Control flow disassemble starting at address" },
	{ "linear", 'l', "[memspec]", 0,
	  "Linear disassembly starting at address" },
	{ "architecture", 'a', "name", 0, 
	  "Machine architecture to disassemble for" },
	{ "syntax", 's', "name", 0, 
	  "Assembly language syntax : att|intel" },
	{ "format", 'f', "fmtspec", 0, 
	  "Output format" },
	{ "output", 'o', "filename", 0, 
	  "File to output to"},
	{ "quiet", 'q', 0, 0, 
	  "Suppress status messages"},
	{ "debug", 'd', 0, 0, 
	  "Print libopdis debug messages"},
	{ 0, 0, 0, OPTION_DOC, "BFD Options:" },
	{ "bfd-entry", 'E', 0, 0,
	  "Perform control flow disassembly on BFD entry point"},
	{ "bfd-symbol", 'N', "bfdname", 0,
	  "Perform control flow disassembly on symbol"},
	{ "bfd-section", 'S', "bfdname", 0,
	  "Perform linear disassembly on section"},
	{ "bfd", 'B', "[target]", OPTION_ARG_OPTIONAL, 
	  "Use BFD library to load and manage target"},
	{ 0, 0, 0, OPTION_DOC, "Advanced Options:" },
	{ "map", 'm', "mapspec", 0, 
	  "Map offset to memory address." },
	{ "bytes", 'b', "string", 0, 
	  "List of input bytes in hex or octal" },
	{ "disassembler-options", 'O', "string", 0,
	  "Apply specific options to disassembler"},
	{ "list-architectures", 1, 0, 0, 
	  "Print available machine architectures"},
	{ "list-disassembler-options", 2, 0, 0, 
	  "Print available disassembler options"},
	{ "list-syntaxes", 3, 0, 0, 
	  "Print available syntax options"},
	{ "list-formats", 4, 0, 0, 
	  "Print available format options"},
	{ "list-bfd-symbols", 5, 0, 0, 
	  "Print symbols found in BFD target"},
	{ "dry-run", 6, 0, 0, 
	  "Print out disasm jobs and exit"},
	{0}
};

struct BFD_TARGET {
	unsigned int id;
	struct BFD_TARGET * next;
};

struct opdis_options {
	job_list_t	jobs;
	mem_map_t	map;
	tgt_list_t	targets;
	opdis_t		opdis;

	unsigned int		arch;
	const char *		arch_str;
	enum opdis_x86_syntax_t syntax;
	const char *		syntax_str;
	enum asm_format_t	fmt;
	const char * 		fmt_str;
	const char *		output;

	int			bfd_all_targets;
	struct BFD_TARGET * 	bfd_targets;
	const char *		disasm_opts;

	int 		list_arch;
	int 		list_disasm_opt;
	int		list_syntax;
	int		list_format;
	int		list_symbols;
	int		dry_run;
	int		quiet;
	int 		debug;

	FILE *			output_file;
	opdis_insn_tree_t	insn_tree;
};

static void set_defaults( struct opdis_options * opts ) {
	opts->jobs = job_list_alloc();
	opts->map = mem_map_alloc();
	opts->targets = tgt_list_alloc();
	opts->opdis = opdis_init();
	opts->insn_tree = opdis_insn_tree_init( 1 );
	opts->output_file = stdout;

	// TODO get first available arch
	// TODO: use 64-bit detection?
	opts->arch_str = "i386";
	opts->arch = bfd_mach_i386_i386;

	opts->syntax_str = "att";
	opts->syntax = opdis_x86_syntax_att;

	opts->fmt_str = "dump";
	opts->fmt = asmfmt_dump;


	opts->disasm_opts = "";
}

/* ---------------------------------------------------------------------- */
/* ARGUMENT HANDLING */

static int set_arch( struct opdis_options * opts, const char * arg ) {
	const bfd_arch_info_type * arch = bfd_scan_arch( arg );
	if (! arch ) {
		fprintf( stderr, "Unsupported architecture: '%s'\n", arg );
		return 0;
	}

	opts->arch = arch->mach;
	opts->arch_str = arg;

	return 1;
}

static int set_syntax( struct opdis_options * opts, const char * arg ) {
	if (! strcmp( "att", arg ) ) {
		opts->syntax = opdis_x86_syntax_att;
	} else if (! strcmp( "intel", arg ) ) {
		opts->syntax = opdis_x86_syntax_intel;
	} else {
		fprintf( stderr, "Unreognized syntax : '%s'\n", arg );
		return 0;
	}

	opts->syntax_str = arg;
	return 1;
}

static int set_format( struct opdis_options * opts, const char * arg ) {
	if (! strcmp( "asm", arg) ) {
		opts->fmt = asmfmt_asm;
	} else if ( ! strcmp( "dump", arg ) ) {
		opts->fmt = asmfmt_dump;
	} else if ( ! strcmp( "delim", arg ) ) {
		opts->fmt = asmfmt_delim;
	} else if ( ! strcmp( "xml", arg ) ) {
		opts->fmt = asmfmt_xml;
	} else if ( strchr( arg, '%' ) ) {
		opts->fmt = asmfmt_custom;
	} else {
		fprintf( stderr, "Unreognized format : '%s'\n", arg );
		return 0;
	}

	opts->fmt_str = arg;
	return 1;
}

static void parse_memspec( const char * memspec, unsigned int * target,
			  opdis_off_t * offset, opdis_off_t * size,
			  opdis_vma_t * vma ) {
	const char * o_start, *s_start, *v_start;

	*target = 1;	/* default to first target */
	*offset = 0;	/* default to offset 0 */
	*vma = OPDIS_INVALID_ADDR;
	*size = 0;	/* default to entire target */

	o_start = strchr( memspec, ':' );
	v_start = strchr( memspec, '@' );
	s_start = strchr( memspec, '+' );

	if ( o_start != memspec && v_start != memspec && s_start != memspec ) {
		*target = strtoul( memspec, NULL, 0);
	}

	if ( o_start ) {
		*offset = strtoul( o_start + 1, NULL, 0);
	}
	if ( v_start ) {
		*vma = strtoul( v_start + 1, NULL, 0);
	}
	if ( s_start ) {
		*size = strtoul( s_start + 1, NULL, 0);
	}
}

static void parse_bfdname( const char * bfdname, unsigned int * target,
		           const char ** name ) {
	const char * n_start = strchr( bfdname, ':' );
	char * err;

	if ( n_start ) {
		*target = strtoul( bfdname, &err, 0);
		if ( err && err != n_start ) {
			fprintf( stderr, "Invalid target ID: %s\n", bfdname );
			*target = 0;
			return;
		}
		*name = n_start + 1;
	} else {
		*target = 1;
		*name = bfdname;
	}
}

static int add_job( job_list_t jobs, enum job_type_t type, const char * arg ) {
	unsigned int target;
	opdis_off_t offset, size;
	opdis_vma_t vma;

	parse_memspec( arg, &target, &offset, &size, &vma );

	return job_list_add( jobs, type, arg, target, offset, vma, size );
}

static int add_bfd_target( struct opdis_options * opts, unsigned int id ) {
	struct BFD_TARGET * tgt, *ptr;

	for ( ptr = opts->bfd_targets; ptr; ptr = ptr->next ) {
		if ( ptr->id == id ) {
			return 1;
		}
	}

	tgt = (struct BFD_TARGET *) calloc( 1, sizeof(struct BFD_TARGET) );
	if (! tgt ) {
		fprintf( stderr, "Unable to allocate memory for BFD target\n" );
		return 0;
	}

	tgt->id = id;
	if (! opts->bfd_targets ) {
		opts->bfd_targets = tgt;
		return 1;
	}

	for ( ptr = opts->bfd_targets; ptr->next; ptr = ptr->next )
		;
	ptr->next = tgt;

	return 1;
}

static int add_bfd_job( struct opdis_options * opts, enum job_type_t type, 
			const char * arg ) {
	unsigned int target = 1;
	const char * name = NULL;

	if ( arg ) {
		parse_bfdname( arg, &target, &name );
	}

	if (! target || (arg && (! name || ! strlen(name))) ) {
		return 0;
	}

	if (! opts->bfd_all_targets ) {
		if(! add_bfd_target( opts, target ) ) {
			return 0;
		}
	}

	return job_list_add_bfd( opts->jobs, type, arg, target, name );
}

static int add_map( mem_map_t map, const char * arg ) {
	unsigned int target;
	opdis_off_t offset, size;
	opdis_vma_t vma;

	parse_memspec( arg, &target, &offset, &size, &vma );
	if ( offset == OPDIS_INVALID_ADDR || vma == OPDIS_INVALID_ADDR ) {
		fprintf( stderr, 
			"Invalid map memspec '%s' : VMA and offset required\n",
			 arg );
		return 0;
	}

	return mem_map_add( map, target, offset, size, vma );
}

static int set_bfd_target( struct opdis_options * opts, const char * arg ) {
	unsigned int id;
	char * err;
	struct BFD_TARGET * tgt, *ptr;

	if (! arg ) {
		opts->bfd_all_targets = 1;
		return 1;
	}

	id = strtoul( arg, &err, 0 );
	if ( err && *err ) {
		fprintf( stderr, "Not a valid target ID: %s\n", arg );
		return 0;
	}

	return add_bfd_target( opts, id );
}

static int set_output_file( struct opdis_options * opts, const char * arg ) {
	FILE * f;
	if (! strcmp( "-", arg ) ) {
		opts->output = NULL;
		opts->output_file = stdout;
		return 1;
	}

	f = fopen( arg, "w" );
	if (! f ) {
		fprintf( stderr, "Unable to open '%s' for writing: %s\n", arg,
			 strerror(errno) );
		return 0;
	}

	opts->output = arg;
	opts->output_file = f;

	return 1;
}

static error_t parse_arg( int key, char * arg, struct argp_state *state ) {
	struct opdis_options * opts = state->input;

	switch ( key ) {
		case 'c':
			if (! add_job( opts->jobs, job_cflow, arg ) ) {
				argp_error( state, "Invalid argument for -c" );
			}
			break;
		case 'l':
			if (! add_job( opts->jobs, job_linear, arg ) ) {
				argp_error( state, "Invalid argument for -l" );
			}
			break;
		case 'a':
			if (! set_arch( opts, arg ) ) {
				argp_error( state, "Invalid argument for -s" );
			}
			break;
		case 's':
			if (! set_syntax( opts, arg ) ) {
				argp_error( state, "Invalid argument for -s" );
			}
			break;
		case 'f':
			if (! set_format( opts, arg ) ) {
				argp_error( state, "Invalid argument for -f" );
			}
			break;
		case 'o':
			if (! set_output_file( opts, arg ) ) {
				argp_error( state, "Invalid argument for -o" );
			}
			break;
		case 'b':
			if (! tgt_list_add( opts->targets, tgt_bytes, arg ) ) {
				argp_error( state, "Invalid argument for -b" );
			}
			break;
		case 'm':
			if (! add_map( opts->map, arg ) ) {
				argp_error( state, "Invalid argument for -m" );
			}
			break;
		case 'O': 
			opts->disasm_opts = arg;
			break;
		case 'B':
			if (! set_bfd_target( opts, arg ) ) {
				argp_error( state, "Invalid argument for -B" );
			}
			break;
		case 'E':
			add_bfd_job( opts, job_bfd_entry, NULL );
			break;
		case 'N':
			if (! add_bfd_job( opts, job_bfd_symbol, arg ) ) {
				argp_error( state, "Invalid argument for -N" );
			}
			break;
		case 'S':
			if (! add_bfd_job( opts, job_bfd_section, arg ) ) {
				argp_error( state, "Invalid argument for -N" );
			}
			break;

		case 'q': opts->quiet = 1; break;
		case 'd': opts->debug++; break;
		case 1: opts->list_arch = 1; break;
		case 2: opts->list_disasm_opt = 1; break;
		case 3: opts->list_syntax = 1; break;
		case 4: opts->list_format = 1; break;
		case 5: opts->list_symbols = 1; break;
		case 6: opts->dry_run = 1; break;

		case ARGP_KEY_ARG:
			tgt_list_add( opts->targets, tgt_file, arg );
			break;

		default:
			return ARGP_ERR_UNKNOWN;

	}
	return 0;
}

static struct argp argp_cfg = {
	options,	/* array of argp_option structs defining options */
	parse_arg,	/* function to handle each argument */
	usage_str,	/* usage string for args */
	help_str,	/* doc */
	NULL,		/* children */
	NULL,		/* help filter */
	NULL		/* translation domain */
};

/* ---------------------------------------------------------------------- */
/* CALLBACKS */

void opdis_display_cb ( const opdis_insn_t * insn, void * arg ) {
	opdis_insn_t * i;
	opdis_insn_tree_t tree = (opdis_insn_tree_t) arg;
	if (! tree ) {
		return;
	}

	i = opdis_insn_dupe( insn );
	opdis_insn_tree_add( tree, i );
}

opdis_vma_t opdis_resolver_cb( const opdis_insn_t * i, void * arg ) {
	mem_map_t map = (mem_map_t) arg;
	opdis_vma_t vma = opdis_default_resolver( i, arg );
	if ( vma != OPDIS_INVALID_ADDR ) {
		return vma;
	}

	if (! map ) {
		return OPDIS_INVALID_ADDR;
	}

	// TODO : check map for target operand if absolute or indirect
	return OPDIS_INVALID_ADDR;
}

static int print_insn( opdis_insn_t * i, void * arg ) {
	struct opdis_options * opts = (struct opdis_options *) arg;

	if (! opts ) {
		return 0;
	}

	// TODO : have display track jump/call targets in a tree,
	//        then emit a comment label line before the tree if
	//        the format is .asm
	asm_fprintf_insn( opts->output_file, opts->fmt, opts->fmt_str, i );

	return 1;
}

static void output_disassembly( struct opdis_options * opts ) {
	asm_fprintf_header( opts->output_file, opts->fmt );

	// TODO: print targets and maps
	
	opdis_insn_tree_foreach( opts->insn_tree, print_insn, opts );
	asm_fprintf_footer( opts->output_file, opts->fmt );
}

/* ---------------------------------------------------------------------- */

static void bfd_load(  tgt_list_item_t * target, unsigned int id, void * arg ) {
	tgt_list_make_bfd( target );
}

static void load_bfd_targets( struct opdis_options * opts ) {
	int load_individually = 1;
	struct BFD_TARGET * tgt;
	if ( opts->bfd_all_targets ) {
		tgt_list_foreach( opts->targets, bfd_load, NULL );
		load_individually = 0;
	}

	for ( tgt = opts->bfd_targets; tgt; tgt = opts->bfd_targets ) {
		if ( load_individually ) {
			tgt_list_item_t * target = tgt_list_find( opts->targets,
								  tgt->id );
			bfd_load( target, tgt->id, NULL );
		}

		opts->bfd_targets = tgt->next;
		free( tgt );
	}
}

static void map_buffer_args( struct opdis_options * opts ) {
	tgt_list_item_t * tgt;
	unsigned int id = 1;
	opdis_vma_t vma = 0;

	/* user has manually mapped memory: defer to them */
	if ( opdis_tree_count( opts->map ) ) {
		return;
	}

	for ( tgt = opts->targets->head; tgt; id++, tgt = tgt->next ) {
		/* map all buffers sequentially into memory */
		if ( tgt->type == tgt_bytes ) {
			mem_map_add( opts->map, id, 0, tgt->data->len, vma );
			vma += tgt->data->len;
		}
	}
}

static void configure_opdis( struct opdis_options * opts ) {
	opdis_t o = opts->opdis;
	const bfd_arch_info_type * arch_info = bfd_scan_arch( opts->arch_str );

	opdis_set_arch( o, arch_info->arch, opts->arch, NULL );

	if ( opts->disasm_opts && opts->disasm_opts[0] ) {
		opdis_set_disassembler_options( o, opts->disasm_opts );
	}

	opdis_set_x86_syntax( o, opts->syntax );

	opdis_set_display( o, opdis_display_cb, opts->insn_tree );
	opdis_set_resolver( o, opdis_resolver_cb, opts->map );

	o->debug = opts->debug;
}

static void set_job_opts( struct opdis_options * o, job_opts_t j ) {
	j->targets = o->targets;
	j->map = o->map;
	j->opdis = o->opdis;
	j->quiet = o->quiet;
}

static void print_target_syms (tgt_list_item_t * t, unsigned int id, void * a) {
	if ( t->symtab ) {
		printf( "Target %d:\n", id );
		sym_tab_print( t->symtab, stdout );
	}
}

static void dry_run( struct opdis_options * opts ) {
	printf( "Architecture: %s\n", opts->arch_str );
	printf( "Disassembler options: %s\n", opts->disasm_opts );
	printf( "Syntax: %s\n", opts->syntax_str );
	printf( "Format: %s\n", opts->fmt_str );
	printf( "Output: %s\n\n", opts->output ? opts->output : "STDOUT" );

	if ( opts->targets->num_items ) {
		printf( "Targets:\n" );
		tgt_list_print( opts->targets, stdout );
	}

	if ( opts->map->num ) {
		printf( "Memory Map:\n" );
		mem_map_print( opts->map, stdout );
	}

	if ( opts->jobs->num_items ) {
		printf( "Jobs:\n" );
		job_list_print( opts->jobs, stdout );
	}
}

/* ---------------------------------------------------------------------- */

static void list_arch() {
	const char ** arch_list = bfd_arch_list();
	const char ** a;
	const char * first = NULL;

	for ( a = arch_list; *a; a++ ) {
		if (! first ) {
			first = *a;
		}
		printf( "\t%s\n", *a );
	}

	printf("Default architecture is '%s'\n", first );
	free(arch_list);
}

static void list_disasm_opts() {
	/* NOTE: this is not architecture-specific; libopcodes prints all
	 * available disassembler options to STDOUT */
	disassembler_usage( stdout );
}

static void list_syntax() {
	printf( "\tatt\n" );
	printf( "\tintel\n" );
}

static void list_format() {
	printf( "\tasm\t: Assembly language listing (just insn)\n" );
	printf( "\tdump\t: Disassembled listing (address, bytes, insn)\n" );
	printf( "\tdelim\t: Pipe-delimited instruction info\n" );
	printf( "\txml\t: XML representation\n" );
	printf( "\t(format string)\n" );
}

static void list_bfd_symbols( struct opdis_options * opts ) {

	if ( opts->bfd_all_targets || opts->bfd_targets ) {
		printf( "BFD Symbols:\n" );
		tgt_list_foreach( opts->targets, print_target_syms, NULL );
	}
}

/* ---------------------------------------------------------------------- */
/* MAIN */
int main( int argc, char ** argv ) {
	struct opdis_options opts = {0};
	struct job_options_t job_opts;
	int list_only = 0;

	set_defaults( &opts );

	argp_parse( &argp_cfg, argc, argv, 0, 0, &opts );

	if ( opts.list_arch ) {
		list_arch();
		list_only = 1;
	}

	if ( opts.list_disasm_opt ) {
		list_disasm_opts();
		list_only = 1;
	}

	if ( opts.list_syntax ) {
		list_syntax();
		list_only = 1;
	}

	if ( opts.list_format ) {
		list_format();
		list_only = 1;
	}

	if ( list_only ) {
		return 0;
	}

	if (! opts.jobs->num_items ) {
		/* if no jobs were requested, do a linear disasm of all */
		int i;
		for ( i = 0; i < opts.targets->num_items; i++ ) {
			job_list_add( opts.jobs, job_linear, "(default)", 
				      i + 1, 0, OPDIS_INVALID_ADDR, 0 );
		}
	}

	load_bfd_targets( & opts );

	if ( opts.list_symbols ) {
		list_bfd_symbols( & opts );
		return 0;
	}

	if ( opts.dry_run ) {
		dry_run( & opts );
		return 0;
	}

	if (! opts.targets->num_items ) {
		fprintf( stderr, "No targets specified! Use -? for help.\n" );
		return 1;
	}

	map_buffer_args( & opts );

	configure_opdis( & opts );
	set_job_opts( &opts, &job_opts );
	job_list_perform_all( opts.jobs, &job_opts );

	output_disassembly( & opts );

	return 0;
}

