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
"  jobspec = cflow|linear[[:target]:offset:rva:size]\n"
"  bfdspec = symbols|sections|all-sections|entry|symbol:name|section:name\n"
"  mapspec = [target:]offset:size:rva\n"
"  fmtspec = asm|dump|delim|xml|custom:fmt_str\n"
"Targets...(bytes). at least one target must be specified\n"
"Actions... If no actions specified, linear disasm is performed on each tgt.\n"
"Map...\n"
"BFD...\n"
;

static struct argp_option options[] = {
	{ 0, 0, 0, OPTION_DOC, "Basic Options:" },
	{ "action", 'a', "jobspec", 0, 
	  "Action to perform" },
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
	{ "bfd", 'B', "bfdspec", OPTION_ARG_OPTIONAL, 
	  "Use BFD library to load and manage target"},
	{ "dry-run", 1, 0, 0, 
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

