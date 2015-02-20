dnl Run autogen.sh to produce a configure script.

dnl CW_VERSION_MAJOR               : Increment when the whole application was rewritten; or some VERY MAJOR change was made.
dnl CW_VERSION_MINOR               : Increment when major changes have occured.
dnl CW_VERSION_REVISION            : Increment every public release; or set to 0 when CW_VERSION_MINOR was incremented.

define(CW_VERSION_MAJOR, 0)
define(CW_VERSION_MINOR, 0)
define(CW_VERSION_REVISION, 0)

define(CW_PACKAGE_NAME, [speech])
define(CW_BUGREPORT, [Aleric.Inglewood@gmail.com])

dnl Package name and version
AC_INIT(CW_PACKAGE_NAME, CW_VERSION_MAJOR.CW_VERSION_MINOR.CW_VERSION_REVISION, CW_BUGREPORT)

dnl Put resulting configuration defines in this header file.
AC_CONFIG_HEADERS([config.h])

dnl Put revision of configure.ac in configure script.
AC_REVISION($Revision: 5 $)dnl

dnl Ensure that the directory specified with --srcdir was correct
dnl by demanding that it contains `speech.cpp'.
AC_CONFIG_SRCDIR([src/speech.cpp])

dnl Automake options.
AM_INIT_AUTOMAKE

dnl Include maintainer mode targets
AM_MAINTAINER_MODE

dnl Because we use cwautomacros.
CW_AUTOMACROS([20140831])

dnl We use doxygen for documentation.
CW_DOXYGEN

dnl Demand at least version 2.57 of autoconf.
AC_PREREQ(2.57)

dnl Check for compiler and preprocessor
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CXXCPP

dnl Add --enable-debug (DEBUG, DOXYGEN_DEBUG), --enable-libcwd (CWDEBUG, DOXYGEN_CWDEBUG),
dnl --enable-optimise and --enable-profile options. Update USE_LIBCWD, CWD_LIBS and CXXFLAGS accordingly.
CW_OPG_FLAGS([-W -Wall -Woverloaded-virtual -Wundef -Wpointer-arith -Wwrite-strings -Winline])

dnl This application uses threads.
LIBCWD_FLAGS="$CWD_R_FLAGS"
LIBCWD_LIBS="$CWD_R_LIBS"
AC_SUBST(LIBCWD_FLAGS)
AC_SUBST(LIBCWD_LIBS)

dnl This source code is C++11.
CXXFLAGS="$CXXFLAGS -pthread -std=c++11"

dnl Each Makefile.am should use DEFS = @DEFS@. Set DEFS here.
DEFS="-DHAVE_CONFIG_H"
AC_SUBST(DEFS)

dnl These CFLAGS are used in the .pc file.
PCCFLAGS=
AC_SUBST(PCCFLAGS)

dnl Checks for other programs.
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl Detect boost.
CW_BOOST([$cw_enable_static], [no], [filesystem system])

# Library flags.
#CW_LIB_LIBGTKMM

dnl Libraries.
PKG_CHECK_MODULES([LIBJACK], [jack])
PKG_CHECK_MODULES([LIBXML], [libxml++-2.6])
dnl PKG_CHECK_MODULES([LIBGTKMM], [gtkmm-3.0])
LIBGTKMM_CFLAGS="-pthread -I/usr/src/debian/gtkmm3.0-3.14.0/gtk -I/usr/include/atkmm-1.6 -I/usr/src/debian/gtk+3.0-3.14.5 -I/usr/src/debian/gtkmm3.0-3.14.0/gdk -I/usr/src/debian/glibmm2.4-2.42.0/gio -I/usr/include/pangomm-1.4 -I/usr/lib/x86_64-linux-gnu/pangomm-1.4/include -I/usr/src/debian/glibmm2.4-2.42.0/glib -I/usr/include/at-spi2-atk/2.0 -I/usr/include/at-spi-2.0 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/pango-1.0 -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/sigc++-2.0 -I/usr/lib/x86_64-linux-gnu/sigc++-2.0/include -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/libpng12 -I/usr/src/debian/glib2.0-2.43.4 -I/usr/src/debian/glib2.0-2.43.4/glib -I/usr/include/cairomm-1.0 -I/usr/lib/cairomm-1.0/include -I/usr/src/debian/gtk+3.0-3.14.5/debian/build/shared"
LIBGTKMM_LIBS="-L/usr/src/debian/gtk+3.0-3.14.5/debian/build/shared/gtk/.libs -L/usr/src/debian/gtk+3.0-3.14.5/debian/build/shared/gdk/.libs -L/usr/src/debian/glib2.0-2.43.4/debian/build/deb/gio/.libs -L/usr/src/debian/glib2.0-2.43.4/debian/build/deb/gobject/.libs -L/usr/src/debian/glib2.0-2.43.4/debian/build/deb/glib/.libs -L/usr/src/debian/gtkmm3.0-3.14.0/gtk/gtkmm/.libs -L/usr/src/debian/gtkmm3.0-3.14.0/gdk/gdkmm/.libs -L/usr/src/debian/glibmm2.4-2.42.0/gio/giomm/.libs -L/usr/src/debian/glibmm2.4-2.42.0/glib/glibmm/.libs -lgtkmm-3.0 -latkmm-1.6 -lgdkmm-3.0 -lgiomm-2.4 -lpangomm-1.4 -lglibmm-2.4 -lgtk-3 -lgdk-3 -lpangocairo-1.0 -lpango-1.0 -latk-1.0 -lcairo-gobject -lgio-2.0 -lcairomm-1.0 -lcairo -lsigc-2.0 -lgdk_pixbuf-2.0 -lgobject-2.0 -lglib-2.0"
AC_SUBST(LIBGTKMM_CFLAGS)
AC_SUBST(LIBGTKMM_LIBS)

dnl Used in sys.h to force recompilation when the compiler version changes.
CW_PROG_CXX_FINGER_PRINTS
CC_FINGER_PRINT="$cw_prog_cc_finger_print"
AC_SUBST([CC_FINGER_PRINT])
CXX_FINGER_PRINT="$cw_prog_cxx_finger_print"
AC_SUBST([CXX_FINGER_PRINT])
CXXCPP_FINGER_PRINT="$cw_prog_cxxcpp_finger_print"
AC_SUBST([CXXCPP_FINGER_PRINT])

dnl Generate src/sys.h from src/sys.h.in
CW_CONFIG_FILE([src], [sys.h])

dnl Output the other configuration files.
AC_CONFIG_FILES(
        [Makefile]
        [src/Makefile]
        [src/cwd/Makefile]
        [src/utils/Makefile]
        [src/xml/Makefile]
        [doc/Makefile]
        [doc/doxygen.config]
        [doc/html.header]
        [doc/html.footer])

AC_OUTPUT
