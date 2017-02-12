# Define package version here.
#
# CW_VERSION_MAJOR               : Increment when the whole library was rewritten; or some VERY MAJOR change was made.
# CW_VERSION_MINOR               : Increment when major changes have occured, like removing interfaces.
# CW_VERSION_REVISION            : Increment every public release; or set to 0 when CW_VERSION_MINOR was incremented.

define(CW_VERSION_MAJOR, 0)
define(CW_VERSION_MINOR, 0)
define(CW_VERSION_REVISION, 0)

# Package name and bug reporting.
define(CW_PACKAGE_NAME, [speech])
define(CW_BUGREPORT, [Aleric.Inglewood@gmail.com])

# CW_COMPILE_FLAGS may contain warning flags, but also default debug flags (-g*)
# and optimization flags (-O*) that which will be stripped when not required.
define(CW_COMPILE_FLAGS, [-fmax-errors=2 -W -Wall -Woverloaded-virtual -Wundef -Wpointer-arith -Wwrite-strings -Winline])
# CW_THREADS can be [no] (single-threaded), [yes] (multi-threaded) or [both] (single and multi-threaded applications).
define(CW_THREADS, [yes])

# Include cwm4 header.
m4_include([cwm4/configure_ac_top.m4])

# We use doxygen for documentation.
CW_DOXYGEN

# Detect boost.
AX_BOOST_BASE([1.58.0])
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM

# Libraries.
AX_PKG_CHECK_MODULES([LIBXML], [libxml++-2.6])
AX_PKG_CHECK_MODULES([LIBJACK], [jack])
AX_PKG_CHECK_MODULES([LIBGTKMM], [gtkmm-3.0])
AX_PKG_CHECK_MODULES([LIBFFTWF], [fftw3f])

# Used in sys.h to force recompilation when the compiler version changes.
CW_PROG_CXX_FINGER_PRINTS
CC_FINGER_PRINT="$cw_prog_cc_finger_print"
AC_SUBST([CC_FINGER_PRINT])
CXX_FINGER_PRINT="$cw_prog_cxx_finger_print"
AC_SUBST([CXX_FINGER_PRINT])
CXXCPP_FINGER_PRINT="$cw_prog_cxxcpp_finger_print"
AC_SUBST([CXXCPP_FINGER_PRINT])

# Generate sys.h from sys.h.in
CW_CONFIG_FILE([.], [sys.h])

# Output the other configuration files.
AC_CONFIG_FILES(
        [src/Makefile]
)

# Include cwm4 footer.
m4_include([cwm4/configure_ac_bottom.m4])
