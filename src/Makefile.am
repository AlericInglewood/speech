AM_CPPFLAGS = -iquote $(top_srcdir)

bin_PROGRAMS = speech

speech_SOURCES = \
        Persist.cpp \
        Configuration.cpp \
        CrossfadeProcessor.cpp \
        JackFIFOBuffer.cpp \
        RecordingDeviceState.cpp \
        FFTJackClient.cpp \
        JackChunkAllocator.cpp \
        JackClient.cpp \
        JackInput.cpp \
        JackOutput.cpp \
        JackPorts.cpp \
        JackProcessor.cpp \
        JackRecorder.cpp \
        JackServerInput.cpp \
        JackServerOutput.cpp \
        JackSilenceOutput.cpp \
        JackSwitch.cpp \
        FFTJackProcessor.cpp \
        UIWindow.cpp \
        speech.cpp

speech_CXXFLAGS = @LIBCWD_FLAGS@ @LIBXML_CFLAGS@ @LIBJACK_CFLAGS@ @LIBGTKMM_CFLAGS@ @LIBFFTWF_CFLAGS@
speech_LDADD = threadsafe/.libs/libthreadsafe.la xml/libxml_r.la utils/libutils_r.la $(top_builddir)/cwds/libcwds_r.la \
               @LIBCWD_LIBS@ @LIBXML_LIBS@ @LIBJACK_LIBS@ @LIBGTKMM_LIBS@ @LIBFFTWF_LIBS@

# --------------- Maintainer's Section

SUBDIRS = utils xml threadsafe

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
	rm -rf $(srcdir)/autom4te.cache
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
