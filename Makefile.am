include $(srcdir)/cwm4/root_makefile_top.am

SUBDIRS = @CW_SUBDIRS@ src

EXTRA_DIST += LICENSE INSTALL README NEWS
DISTCLEANFILES = stamp-h.in

CXXFLAGS = @CXXFLAGS@ @LIBCWD_FLAGS@
LIBS = @LIBCWD_LIBS@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = 

include $(srcdir)/cwm4/root_makefile_bottom.am

sys.h: timestamp-sys.h

distclean-local:
	rm -f sys.h
