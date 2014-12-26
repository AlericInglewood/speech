## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = foreign

SUBDIRS = cwd utils xml .

EXTRA_DIST = ${BUILT_SOURCES}

bin_PROGRAMS = speech
BUILT_SOURCES =
DEFS = @DEFS@
DEFAULT_INCLUDES=-iquote $(top_builddir) -iquote $(top_builddir)/src -iquote $(srcdir) -iquote $(srcdir)/cwd
CXXFLAGS =

speech_SOURCES = \
	speech.cpp

speech_CXXFLAGS = @CXXFLAGS@ @CWD_R_FLAGS@ -pthread @LIBJACK_CFLAGS@
speech_LDADD = @LIBS@ @CWD_R_LIBS@ @LIBJACK_LIBS@

# --------------- Maintainer's Section

sys.h: timestamp-sys.h

#dist-hook:

all-local:
	@if test -e $(top_srcdir)/../.libcwdrc-src -a ! -e .libcwdrc; then \
	  echo "ln -s $(top_srcdir)/../.libcwdrc-src .libcwdrc"; \
	  ln -s $(top_srcdir)/../.libcwdrc-src .libcwdrc; \
	fi
	@if test -e $(top_srcdir)/../.gdbinit-src -a ! -e .gdbinit; then \
	  echo "ln -s $(top_srcdir)/../.gdbinit-src .gdbinit"; \
	  ln -s $(top_srcdir)/../.gdbinit-src .gdbinit; \
	fi

distclean-local:
	rm -rf $(srcdir)/autom4te.cache sys.h
	@MAINTAINER_MODE_TRUE@rm -f .all_sources *.s *.ii

clean-local:

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in tags troep* *.cgd

if MAINTAINER_MODE
# Usage: You have to set the CTAGS_ROOT_SRCDIRS environment variable yourself,
# for example: export CTAGS_ROOT_SRCDIRS="/usr/src/gtk/glib-current /usr/src/gtk/gtk+-current"
# Then run 'vim -u /some/path/.vimrc' where the .vimrc is the normal .vimrc, but with the
# additional lines: set tags=/full/path/to/top_builddir/tags
#                   nmap <C-N> :tnext<CR>
ctags:
	rm -f .all_sources
	touch .all_sources
	for d in $(CTAGS_ROOT_SRCDIRS) $(top_srcdir); do \
	  find $$d -follow -type f -name '*.h' -o -name '*.hpp' -o -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' -o -name '*.C' -o -name '*.inl' >> .all_sources; \
	done
	ctags `cat .all_sources`
endif
