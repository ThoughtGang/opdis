/*!
 * \file main.c
 * \brief Startup routine for opdis command-line utility
 * \author thoughtgang.org
 */

#include <argp.h>		/* glibc command line option parser */
#include <bfd.h>
#include <stdio.h>
#include <stdlib.h>

#include <opdis/opdis.h>

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
"  mapspec = [target]:offset@rva[+size]\n"
"  fmtspec = asm|dump|delim|xml|fmt_str\n"
"Targets...(bytes). at least one target must be specified\n"
"Actions... If no actions specified, linear disasm is performed on each tgt.\n"
"Map...\n"
"BFD...\n"
;

// const bfd_arch_info_type *bfd_scan_arch (const char *string)
// const char **bfd_arch_list (void)
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
	{ 0, 0, 0, OPTION_DOC, "Advanced Options:" },
	{ "map", 'm', "mapspec", 0, 
	  "Map offset to memory address." },
	{ "bytes", 'b', "string", 0, 
	  "List of input bytes in hex or octal" },
	{ "disassembler-options", 'O', "string", 0,
	  "Apply specific options to disassembler"},
	{ "bfd", 'B', "[target]", OPTION_ARG_OPTIONAL, 
	  "Use BFD library to load and manage target"},
	{ "bfd-symbol", 'N', "name", 0,
	  "Perform control flow disassembly on symbol"},
	{ "bfd-section", 'S', "name", 0,
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

/* ---------------------------------------------------------------------- */
/* ARGUMENT PARSING */

struct opdis_options {
	void *		jobs;
	void *		maps;
	void *		targets;

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
};

static set_defaults( struct opdis_options * opts ) {
	/* use 64-bit detection? */
	opts->mach = bfd_mach_i386_i386;
	opts->syntax = opdis_x86_syntax_att;
	opts->format = asmfmt_dump;
}

static error_t parse_arg( int key, char * arg, struct argp_state *state ) {
	struct opdis_options * opts = state->input;

	switch ( key ) {
		case 'c':
			// add job
		case 'l':
			// add job
		case 'a':
			// set arch
		case 's':
			// set syntax
		case 'f':
			// set format
		case 'o':
			// set output
		case 'b':
			// add target
		case 'm':
			// map buffer
		case 'O':
			// set disasm options
		case 'B':
			// set BFD target (or 0)
		case 'N':
			// add job
		case 'S':
			// add job
			break;
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
	for ( a = arch_list; *a; a++ ) {
		printf( "\t%s\n", *a );
	}

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

	// load all targets in arguments

	if ( opts.dry_run ) {
		dry_run( & opts );
		return 0;
	}

	// foreach job...

	return 0;
}

