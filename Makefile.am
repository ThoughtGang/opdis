INCLUDES = -I$(top_srcdir)
ACLOCAL_AMFLAGS = -I m4
bin_PROGRAMS = opdis
lib_LTLIBRARIES = libopdis.la
check_PROGRAMS = tree_test disasm_cflow disasm_linear
TESTS = $(check_PROGRAMS)
EXTRA_DIST = doc/api doc/examples
opdis_SOURCES = src/main.c
opdis_LDADD = libopdis.la
nobase_include_HEADERS = opdis/insn_buf.h opdis/metadata.h opdis/model.h \
		         opdis/opdis.h opdis/tree.h opdis/types.h
libopdis_la_SOURCES = opdis/insn_buf.c opdis/model.c opdis/opdis.c \
		      opdis/tree.c opdis/types.c
tree_test_SOURCES = test/tree_test.c
disasm_cflow_SOURCES = test/disasm_cflow/disasm_cflow.c
disasm_linear_SOURCES = test/disasm_linear/disasm_linear.c
docs:
	doxygen config/Doxyfile


