/* main.c
 * Startup routine for opdis command-line utility
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

const char * argp_program_version = "opdis 0.1";
const char * argp_program_bug_address = "<dev@thoughtgang.org>";
static const char usage_str[] = "[FILE]...";
static const char help_str[] = 
/* brief description: */
"Opdis command-line disassembler" 
"\v" /* delim */ 
/* detailed documentation:  */
"Disassembler that uses libopcodes in a less sucky way than objdump.\n"
"  memspec = [target]:offset|@rva[+size]\n"
"  bfdname = [target:]name\n"
"  mapspec = [target]:offset@rva[+size]\n"
"  target  = ID (#) of target; use --dry-run to see IDs\n" 
"  fmtspec = asm|dump|delim|xml|fmt_str\n"
"Targets...(bytes). at least one target must be specified\n"
"Actions... If no actions specified, linear disasm is performed on each tgt.\n"
"Map...\n"
"BFD...\n"
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
	{ 0, 0, 0, OPTION_DOC, "Advanced Options:" },
	{ "map", 'm', "mapspec", 0, 
	  "Map offset to memory address." },
	{ "bytes", 'b', "string", 0, 
	  "List of input bytes in hex or octal" },
	{ "disassembler-options", 'O', "string", 0,
	  "Apply specific options to disassembler"},
	{ "bfd", 'B', "[target]", OPTION_ARG_OPTIONAL, 
	  "Use BFD library to load and manage target"},
	{ "bfd-symbol", 'N', "bfdname", 0,
	  "Perform control flow disassembly on symbol"},
	{ "bfd-section", 'S', "bfdname", 0,
	  "Perform linear disassembly on section"},
	{ "list-architectures", 1, 0, 0, 
	  "Print available machine architectures"},
	{ "list-disassembler-options", 2, 0, 0, 
	  "Print available disassembler options"},
	{ "list-syntaxes", 3, 0, 0, 
	  "Print available syntax options"},
	{ "list-formats", 4, 0, 0, 
	  "Print available format options"},
	{ "dry-run", 5, 0, 0, 
	  "Print out disasm jobs and exit"},
	{0}
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
	const char * 		asm_fmt;
	const char *		output;

	// TODO : list of bfd targets or bfd all
	int		bfd_target;
	const char *	disasm_opts;

	int 		list_arch;
	int 		list_disasm_opt;
	int		list_syntax;
	int		list_format;
	int		dry_run;
	int		quiet;
};

static void set_defaults( struct opdis_options * opts ) {
	opts->jobs = job_list_alloc();
	opts->map = mem_map_alloc();
	opts->targets = tgt_list_alloc();
	opts->opdis = opdis_init();

	/* TODO: use 64-bit detection? */
	opts->arch_str = "i386";
	opts->arch = bfd_mach_i386_i386;

	opts->syntax_str = "att";
	opts->syntax = opdis_x86_syntax_att;

	opts->asm_fmt = "dump";

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

static void parse_memspec( const char * memspec, unsigned int * target,
			  opdis_off_t * offset, opdis_off_t * size,
			  opdis_vma_t * vma ) {
	const char * o_start, *s_start, *v_start;

	*target = 1;	/* default to first target */
	*offset = OPDIS_INVALID_ADDR;
	*vma = OPDIS_INVALID_ADDR;
	*size = 0;

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

	if ( n_start ) {
		*target = strtoul( bfdname, NULL, 0);
		*name = n_start + 1;
	} else {
		*name = bfdname;
	}
}

static int add_job( job_list_t jobs, enum job_type_t type, const char * arg ) {
	unsigned int target;
	opdis_off_t offset, size;
	opdis_vma_t vma;

	parse_memspec( arg, &target, &offset, &size, &vma );
	if ( offset == OPDIS_INVALID_ADDR && vma == OPDIS_INVALID_ADDR ) {
		fprintf( stderr, "Invalid memspec '%s' : no VMA or offset\n",
			 arg );
		return 0;
	}

	return job_list_add( jobs, type, arg, target, offset, vma, size );
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
			if (! is_supported_format(arg) ) {
				argp_error( state, "Invalid argument for -f" );
			}
			opts->asm_fmt = arg;
			break;
		case 'o':
			opts->output = arg;
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
		case 'B': 		// set BFD target (or 0)
			// set_bfd_target
			break;
		case 'N': 		// add job
			//parse_bfdname( arg, &target, &name );
			// add_job( opts, symbol, arg )
			// note: if not bfd, set bfd to 1
			break;
		case 'S': 		// add job
			//parse_bfdname( arg, &target, &name );
			// add_job( opts, section, arg )
			// note: if not bfd, set bfd to 1
			break;

		case 'q': opts->quiet = 1; break;
		case 1: opts->list_arch = 1; break;
		case 2: opts->list_disasm_opt = 1; break;
		case 3: opts->list_syntax = 1; break;
		case 4: opts->list_format = 1; break;
		case 5: opts->dry_run = 1; break;

		case ARGP_KEY_ARG:
			tgt_list_add( opts->targets, tgt_file, arg );
			break;

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

static void load_bfd_targets( struct opdis_options * opts ) {
}

static void configure_opdis( struct opdis_options * opts ) {
}

static void dry_run( struct opdis_options * opts ) {
	printf( "Architecture: %s\n", opts->arch_str );
	printf( "Disassembler options: %s\n", opts->disasm_opts );
	printf( "Syntax: %s\n", opts->syntax_str );
	printf( "Format: %s\n", opts->asm_fmt );
	printf( "Output: %s\n", opts->output ? opts->output : "STDOUT" );


	printf( "Targets:\n" );
	tgt_list_print( opts->targets, stdout );

	printf( "Memory Map:\n" );
	mem_map_print( opts->map, stdout );

	printf( "Jobs:\n" );
	job_list_print( opts->jobs, stdout );
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

/* ---------------------------------------------------------------------- */
/* MAIN */
int main( int argc, char ** argv ) {
	struct opdis_options opts = {0}; // TODO: defaults

	set_defaults( &opts );

	argp_parse( &argp_cfg, argc, argv, 0, 0, &opts );

	if ( opts.list_arch ) {
		list_arch();
		return 0;
	}

	if ( opts.list_disasm_opt ) {
		list_disasm_opts();
		return 0;
	}

	if ( opts.list_syntax ) {
		list_syntax();
		return 0;
	}

	if ( opts.list_format ) {
		list_format();
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

	if ( opts.dry_run ) {
		dry_run( & opts );
		return 0;
	}

	if (! opts.targets->num_items ) {
		fprintf( stderr, "No targets specified! Use -? for help.\n" );
		return 1;
	}

	load_bfd_targets( & opts );

	configure_opdis( & opts );

	job_list_perform_all( opts.jobs, opts.targets, opts.map, opts.opdis,
			      opts.quiet );

	return 0;
}

