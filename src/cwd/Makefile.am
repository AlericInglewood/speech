EXTRA_DIST = *.h

noinst_LTLIBRARIES = libdebug.la

# See src/Makefile.am
DEFS = @DEFS@
DEFAULT_INCLUDES=-iquote $(top_builddir) -iquote $(top_builddir)/src -iquote $(srcdir) -iquote $(srcdir)/..

# See src/Makefile.am.
libdebug_la_CXXFLAGS = @CXXFLAGS@ @CWD_FLAGS@
libdebug_la_SOURCES = \
	debug.cpp \
	debug_ostream_operators.cpp

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

if MAINTAINER_MODE
distclean-local:
	rm -f *.s *.ii

clean-local:
endif
