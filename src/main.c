/* main.c
 * Startup routine for opdis command-line utility
 */

#include <argp.h>		/* glibc command line option parser */
#include <bfd.h>
#include <stdio.h>
#include <stdlib.h>

#include <opdis/opdis.h>

#include "job_list.h"
#include "map.h"
#include "target_list.h"

enum asm_format_t {
	asmfmt_custom,
	asmfmt_asm,
	asmfmt_dump,
	asmfmt_delim,
	asmfmt_xml
};

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

	unsigned int		mach;
	enum opdis_x86_syntax_t syntax;
	enum asm_format_t	format;
	const char * 		fmt_str;
	const char *		output;

	int		bfd_target;
	const char *	disasm_opts;

	int 		list_arch;
	int 		list_disasm_opt;
	int		list_syntax;
	int		list_format;
	int		dry_run;
	int		quiet;
};

static set_defaults( struct opdis_options * opts ) {
	opts->jobs = job_list_alloc();
	opts->map = mem_map_alloc();
	opts->targets = tgt_list_alloc();

	/* use 64-bit detection? */
	opts->mach = bfd_mach_i386_i386;
	opts->syntax = opdis_x86_syntax_att;
	opts->format = asmfmt_dump;
}

/* ---------------------------------------------------------------------- */
/* ARGUMENT HANDLING */

static int parse_memspec( const char * memspec, unsigned int * target,
			  opdis_off_t * offset, opdis_off_t * size,
			  opdis_vma_t * vma ) {
	// memspec = [target]:offset|@rva[+size]\n"
	// mapspec = [target]:offset@rva[+size]\n"
	// NOTE: caller enforces requirements of mapspec
	return 0;
}

static int parse_bfdname( const char * bfdname, unsigned int * target,
		          char * name ) {
	//  bfdname = [target:]name\n"
	return 0;
}

// TODO : asm format module for printing stuff

static error_t parse_arg( int key, char * arg, struct argp_state *state ) {
	struct opdis_options * opts = state->input;

	switch ( key ) {
		case 'c': 		// add job
			// add_job( opts, cflow, arg )
			break;
		case 'l': 		// add job
			// add_job( opts, linear, arg )
			break;
		case 'a': 		// set arch
			// set_arch( opts, arg )
			break;
		case 's': 		// set syntax
		// const bfd_arch_info_type *bfd_scan_arch (const char *string)
			// set_syntax( opts, arg )
			break;
		case 'f': 		// set format
			// set_format( opts, arg )
			break;
		case 'o': 		// set output
			// set output( opts, arg )
			break;
		case 'b': 		// add target
			// add_target( opts, arg )
			break;
		case 'm': 		// map buffer
			// add_map( opts, arg )
			break;
		case 'O': 		// set disasm options
			// set_disasm_options( opts, arg )
			break;
		case 'B': 		// set BFD target (or 0)
			// set_bfd_target
			break;
		case 'N': 		// add job
			// add_job( opts, symbol, arg )
			// note: if not bfd, set bfd to 1
			break;
		case 'S': 		// add job
			// add_job( opts, section, arg )
			// note: if not bfd, set bfd to 1
			break;

		case 'q': opts->quiet = 1; break;
		case 1: opts->list_arch = 1; break;
		case 2: opts->list_disasm_opt = 1; break;
		case 3: opts->list_syntax = 1; break;
		case 4: opts->list_format = 1; break;
		case 5: opts->dry_run = 1; break;
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

static void dry_run( struct opdis_options * opts ) {
	// print all targets, maps, jobs, syntax, etc
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

	if (! opts.targets->num_items ) {
	// 	help
		return 1;
	}

	// load all targets in arguments
	// create buffers for all maps?

	if ( opts.dry_run ) {
		dry_run( & opts );
		return 0;
	}

	// foreach job...

	return 0;
}

