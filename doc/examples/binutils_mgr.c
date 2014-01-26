/* binutils_mgr
 * Example of using libopdis with libbinutils-mgr
   (c) Copyright 2014 mkfs (https://github.com/mkfs)
   NOT FOR REDISTRIBUTION

   Compile with:
     gcc doc/examples/binutils-mgr.c -Ldist/.libs -I. -ldl -lbinutils_mgr \
         -lopdis 
   Usage:
     binutils_mgr [DIR]... BINFILE
   ...where DIR is a binutils distribution
 */

#include <stdio.h>
#include <stdlib.h>

#include <binutils_mgr/reg.h>
#include <opdis/opdis.h>

static void arch_disasm( bu_arch_t * arch, opdis_buf_t target ) {
	struct PRINT_INSN_FUNC * descr;

	opdis_t o = opdis_init();
	if (! o ) {
		fprintf(stderr, "Could not alloc opdis_info_t!\n");
		return;
	}

	opdis_override_opcodes_init( o, arch->init_disasm_info );

	/* foreach print_insn function in this libopcodes library... */
        for (descr = arch->print_insn; descr; descr = descr->next ) {
		printf("; ### ARCH: %s ###\n", descr->name );

		/* configure opdis to use this print_insn function */
		/* NOTE: default BFD architecture and machine are used here
		 *       but the application can override this. */
		opdis_set_arch( o, bfd_arch_unknown, 0, descr->fn );

		/* optional: override default instruction decoder: */
		//opdis_set_decoder( o, opdis_default_decoder, o );
		
		/* disassemble all addresses in buffer: */
		opdis_disasm_linear( o, target, 0, 0 );
        }

	opdis_term(o);
}

int main(int argc, char ** argv) {
	unsigned int i;
	opdis_buf_t target;
	FILE * f;

	if (argc < 2 ) {
		fprintf(stderr, "Usage: %s [DIR]... FILE\n", argv[0]);
		return -1;
	}

	/* load system binutils into binutils-mgr as default */
	if (! bu_arch_register_system_binutils( NULL ) ) {
                fprintf(stderr, "Unable to register system binutils\n");
        }

	/* read user binutils-mgr configuration */
	bu_read_config();

	/* load any additional bintils distributions from command line */
	for (i = 1; i < (argc - 1); i++) {
		const char *dir = argv[i];
		bu_arch_register( dir, NULL );
	}

	f = fopen(argv[i], "rb");
	if (! f ) {
		fprintf(stderr, "Could not open %s\n", argv[i]);
		return 1;
	}

	target =  opdis_buf_read( f, 0, 0 );

	if (! target ) {
		fprintf(stderr, "Unable to allocate opdis_buf_t\n");
		fclose(f);
		return 2;
	}

	fclose(f);

	bu_arch_foreach( (BU_ARCH_CB) arch_disasm, target );

	opdis_buf_free(target);

	return 0;
}
