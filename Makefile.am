INCLUDES = -I$(top_srcdir)
ACLOCAL_AMFLAGS = -I m4

# ----------------------------------------------------------------------
#  TARGETS

# Binary programs to be built and installed by 'make install'
bin_PROGRAMS = dist/opdis

# Libtool libraries to be built and installed by 'make install'
lib_LTLIBRARIES = dist/libopdis.la

# Standard .so/.a libraries to be built and installed by 'make install'
# lib_LIBRARIES = dist/libopdis.a

# Test programs to be built by 'make check'
check_PROGRAMS = test/tree_test test/disasm_cflow test/disasm_linear

# Test programs to be run by 'make check'
TESTS = test/tree_test

# Headers to be installed by 'make install'
nobase_include_HEADERS = opdis/insn_buf.h opdis/metadata.h opdis/model.h \
		         opdis/opdis.h opdis/tree.h opdis/types.h

# Additional files to distribute with the source
EXTRA_DIST = config doc/doxy_input doc/examples doc/html doc/man

# ----------------------------------------------------------------------
# OPDIS TARGET

dist_opdis_SOURCES = src/main.c
dist_opdis_LDADD = dist/libopdis.la -lbfd -lopcodes 


# ----------------------------------------------------------------------
# LIBOPDIS TARGET

dist_libopdis_la_SOURCES = opdis/insn_buf.c opdis/model.c opdis/opdis.c \
		      opdis/tree.c opdis/types.c

# ----------------------------------------------------------------------
# TEST PROGRAMS

test_tree_test_SOURCES = test/tree_test.c
test_tree_test_LDADD = dist/libopdis.la -lbfd -lopcodes 
test_disasm_cflow_SOURCES = test/disasm_cflow.c
test_disasm_cflow_LDADD = dist/libopdis.la -lbfd -lopcodes 
test_disasm_linear_SOURCES = test/disasm_linear.c
test_disasm_linear_LDADD = dist/libopdis.la -lbfd -lopcodes 

# ----------------------------------------------------------------------
# DOXYGEN TARGET
# 	This provides a 'make doxygen' target to build the documentation.

doxygen:
	doxygen config/Doxyfile
