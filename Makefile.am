INCLUDES = -I$(top_srcdir)
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = dist/opdis

lib_LTLIBRARIES = dist/libopdis.la

check_PROGRAMS = test/tree_test test/disasm_cflow test/disasm_linear

TESTS = $(check_PROGRAMS)

EXTRA_DIST = doc/api doc/examples

dist_opdis_SOURCES = src/main.c
dist_opdis_LDADD = dist/libopdis.la -lbfd -lopcodes 

nobase_include_HEADERS = opdis/insn_buf.h opdis/metadata.h opdis/model.h \
		         opdis/opdis.h opdis/tree.h opdis/types.h

dist_libopdis_la_SOURCES = opdis/insn_buf.c opdis/model.c opdis/opdis.c \
		      opdis/tree.c opdis/types.c

test_tree_test_SOURCES = test/tree_test.c
test_disasm_cflow_SOURCES = test/disasm_cflow.c
test_disasm_linear_SOURCES = test/disasm_linear.c
docs:
	doxygen config/Doxyfile


