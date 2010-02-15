/*!
 * \file main.c
 * \brief Startup routine for opdis command-line utility
 * \author thoughtgang.org
 */

#include <argp.h>		/* glibc command line option parser */
#include <opdis/opdis.h>

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
"  jobspec = cflow|linear[memspec]\n"
"  bfdspec = symbols|sections|all-sections|entry|symbol:name|section:name\n"
"  mapspec = [target]:offset@rva[+size]\n"
"  fmtspec = asm|dump|delim|xml|custom:fmt_str\n"
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
	{ "format", 'F', "fmtspec", 0, 
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
	{ "dry-run", 3, 0, 0, 
	  "Print out disasm jobs and exit"},
	{0}
};

/* ---------------------------------------------------------------------- */
/* ARGUMENT PARSING */

struct opdis_options {
	int	loud;
};

static error_t parse_arg( int key, char * arg, struct argp_state *state ) {
	struct opdis_options * opts = state->input;

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

int main( int argc, char ** argv ) {
	struct opdis_options opts = {0}; // TODO: defaults

	argp_parse( &argp_cfg, argc, argv, 0, 0, &opts );
	// huge todo
	return 0;
}

