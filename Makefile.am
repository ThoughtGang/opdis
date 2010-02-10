INCLUDES = -I$(top_srcdir)
ACLOCAL_AMFLAGS = -I m4
bin_PROGRAMS = opdis
lib_LTLIBRARIES = libopdis.la
check_PROGRAMS = tree_test disasm_cflow disasm_linear
TESTS = $(check_PROGRAMS)
opdis_SOURCES = src/main.c
opdis_LDADD = libopdis.la
libopdis_la_SOURCES = opdis/insn_buf.c opdis/insn_buf.h opdis/metadata.h \
opdis/model.c opdis/model.h opdis/opdis.c opdis/opdis.h opdis/tree.c \
opdis/tree.h opdis/types.c opdis/types.h
tree_test_SOURCES = test/tree_test.c
disasm_cflow_SOURCES = test/disasm_cflow/disasm_cflow.c
disasm_linear_SOURCES = test/disasm_linear/disasm_linear.c
docs:
	doxygen config/Doxyfile


