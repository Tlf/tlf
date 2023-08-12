# Process this file with autoconf to produce a configure script.
AC_INIT([Tlf],
	[1.5~git],
	[tlf-devel@nongnu.org],
	[tlf],
	[https://github.com/Tlf/tlf])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/tlf.h])
AC_CONFIG_MACRO_DIR([macros])
AC_CONFIG_HEADERS([config.h])

dnl Clean compilation output makes compiler warnings more visible
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for programs.
# (autoconf 2.70 obsoletes AC_PROG_CC_C99, it's now done by AC_PROG_CC)
m4_version_prereq([2.70],[AC_PROG_CC],[AC_PROG_CC_C99])
if test x"$ac_cv_prog_cc_c99" = x"no"; then
	AC_MSG_FAILURE([*** C99 support is mandatory])
fi
AC_PROG_INSTALL

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h \
	sys/ioctl.h sys/socket.h sys/time.h syslog.h termios.h])

# Checks for typedefs, structures, and compiler characteristics.
dnl needed? AC_HEADER_STDBOOL
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
dnl obsolescent AC_PROG_GCC_TRADITIONAL
dnl obsolescent AC_FUNC_STRFTIME
AC_CHECK_FUNCS([bzero floor ftruncate gethostbyname memset mkfifo putenv \
	select socket sqrt strcasecmp strchr strcspn strdup strpbrk strspn \
	strstr])

# Checks for libraries.
AC_CHECK_LIB([m], [atan], [
	AC_DEFINE([HAVE_LIBM], [1],
		[Define to 1 if you have the `m' library (-lm).])
	AC_SUBST([LIBM_LIB], [-lm])], [
	AC_MSG_ERROR([m library not found...])])

AX_PTHREAD


##----------------------------------##
## Third party library tests follow ##
##----------------------------------##

dnl Make certain pkg-config is installed
PKG_PROG_PKG_CONFIG([0.16])

AS_IF([test "x$PKG_CONFIG" = x],
	[AC_MSG_ERROR([pkg-config is required to configure $PACKAGE_NAME])])

dnl From /usr/share/aclocal/glib-2.0.m4 on Debian 8.0.
dnl Provides compilation testing of the found glib library for suitability.
AM_PATH_GLIB_2_0(2.40.0,,[AC_MSG_ERROR([GLib 2.40 or later not found])])

dnl check for cmocka test framework and warn if not installed
have_cmocka="yes"
PKG_CHECK_MODULES([CMOCKA], [cmocka >= 1.0.1],,[have_cmocka="no"])
AC_CHECK_HEADER([setjmp.h])
AC_CHECK_HEADER([cmocka.h],, [have_cmocka="no"],
[#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
])

if test "${have_cmocka}" = "no"; then
    AC_MSG_WARN([you will need cmocka for running unit tests])
fi


# ncurses and panel required, if not found exit with error message.

dnl macros/ax_with_curses.m4
AX_WITH_CURSES

AS_IF([test "x$ax_cv_curses" != xyes || test "x$ax_cv_curses_color" != xyes],
	[AC_MSG_ERROR([a Curses library with color support is required])])

dnl macros/ax_with_curses_extra.m4
AX_WITH_CURSES_PANEL

AS_IF([test "x$ax_cv_panel" != xyes],
	[AC_MSG_ERROR([the Curses Panel library is required])])


dnl Check the Hamradio control libraries (hamlib)

hamlib_modules="hamlib >= 1.2.8"
PKG_CHECK_MODULES([HAMLIB], [$hamlib_modules], [],
        [AC_MSG_ERROR([Hamradio control libraries 1.2.8 or later not found...])])

tlf_saved_LIBS=$LIBS
LIBS="$LIBS $HAMLIB_LIBS"

AC_CHECK_LIB([hamlib], [rig_open], [
        AC_DEFINE([HAVE_LIBHAMLIB], [1],
                [Define to 1 if you have the `hamlib' library (-lhamlib).])], [
        AC_MSG_ERROR([Hamradio control libraries not found!])])

AC_DEFINE_UNQUOTED([HAMLIB_VERSION],
    [`$PKG_CONFIG --modversion hamlib | awk '{print int($1*100)}'`],
        [Hamlib version (times 100)])

LIBS=$tlf_saved_LIBS

tlf_saved_CFLAGS=$CFLAGS
tlf_saved_CPPFLAGS=$CPPFLAGS
CFLAGS="$CFLAGS $HAMLIB_CFLAGS"
CPPFLAGS="$CPPFLAGS `$PKG_CONFIG --cflags-only-I hamlib 2>&1`"

AC_CHECK_HEADERS([hamlib/rig.h], [], [
        AC_MSG_ERROR([Hamlib headers not found...])])

CPPFLAGS=$tlf_saved_CPPFLAGS
CFLAGS=$tlf_saved_CFLAGS


dnl Check if we want to use xmlrpc to read carrier from Fldigi
AC_MSG_CHECKING([whether to build Fldigi XML RPC support])
AC_ARG_ENABLE([fldigi-xmlrpc],
	[AS_HELP_STRING([--enable-fldigi-xmlrpc],
		[Add support for XMLRPC to read Fldigi's carrier (usually in RTTY/DIGIMODE)])],
	[wantfldigixmlrpc=true],
	[wantfldigixmlrpc=false])

AS_IF([test "x$wantfldigixmlrpc" = xtrue], [
	dnl Look for xmlrpc libraries
	AC_MSG_RESULT([yes])

	AC_CHECK_LIB([xmlrpc], [xmlrpc_read_int], [
		AC_DEFINE([HAVE_LIBXMLRPC], [1],
			[Define to 1 if you have the `xmlrpc' library (-lxmlrpc).])
		AC_SUBST([LIBXMLRPC_LIB], [-lxmlrpc])], [
		AC_MSG_ERROR([xmlrpc library not found...])])

	AC_CHECK_LIB([xmlrpc_client], [xmlrpc_client_cleanup], [
		AC_DEFINE([HAVE_LIBXMLRPC_CLIENT], [1],
			[Define to 1 if you have the `xmlrpc_client' library (-lxmlrpc_client).])
		AC_SUBST([LIBXMLRPC_CLIENT_LIB], [-lxmlrpc_client])], [
		AC_MSG_ERROR([xmlrpc-client library not found...])])

	AC_CHECK_LIB([xmlrpc_util], [xmlrpc_env_init], [
		AC_DEFINE([HAVE_LIBXMLRPC_UTIL], [1],
			[Define to 1 if you have the `xmlrpc_util' library (-lxmlrpc_util).])
		AC_SUBST([LIBXMLRPC_UTIL_LIB], [-lxmlrpc_util])], [
		AC_MSG_ERROR([xmlrpc-util library not found...])])

	AC_CHECK_HEADERS([xmlrpc-c/base.h], [], [
		AC_MSG_ERROR([xmlrpc-c/base.h header not found...])])

	AC_CHECK_HEADERS([xmlrpc-c/client.h], [], [
		AC_MSG_ERROR([xmlrpc-c/client.h header not found...])])], [

	dnl else Fldigi XML RPC not wanted
	AC_MSG_RESULT([no])])

dnl Check if Python3 and pexpect are available
AM_PATH_PYTHON([3.8])

AC_MSG_CHECKING([for pexpect])
AS_IF([$PYTHON -c "import pexpect" 2>/dev/null],
        [AC_MSG_RESULT([yes])],
        [AC_MSG_ERROR([Python module pexpect is not available])]
)

dnl Check if we want to use 'plugin' feature with Python3
AC_MSG_CHECKING([whether to build Python plugin support])
AC_ARG_ENABLE([python-plugin],
	[AS_HELP_STRING([--enable-python-plugin],
		[Add Python support to use custom rules])],
	[wantpythonplugin=true],
	[wantpythonplugin=false])

AS_IF([test "x$wantpythonplugin" = xtrue], [
	dnl Look for Python
	AC_MSG_RESULT([yes])

	AM_PATH_PYTHON([3.9])

	python_main_ver=`$PYTHON -c "import sys; print(sys.version.split('.')[[0]])"`
	python_sub_ver=`$PYTHON -c "import sys; print(sys.version.split('.')[[1]])"`

	AC_ARG_VAR([PYTHON_INCLUDE], [Include flags for python, bypassing python-config])
	AC_ARG_VAR([PYTHON_CONFIG], [Path to python-config])
	AS_IF([test -z "$PYTHON_INCLUDE"], [
	    AS_IF([test -z "$PYTHON_CONFIG"], [
		AC_PATH_PROGS([PYTHON_CONFIG],
		    [python$PYTHON_VERSION-config python-config],
		    [no],
		    [`dirname $PYTHON`])
		AS_IF([test "$PYTHON_CONFIG" = no], [AC_MSG_ERROR([cannot find python-config for $PYTHON.])])
	    ])
	    AC_MSG_CHECKING([python include flags])
	    pyinc=`$PYTHON_CONFIG --includes`
	    AC_SUBST([PYTHON_INCLUDE], [$pyinc])
	    AC_MSG_RESULT([$pyinc])
	    AC_MSG_CHECKING([python libs])
	    AS_IF([test $python_sub_ver -gt 7], [pylibs=`$PYTHON_CONFIG --libs --embed`], [pylibs=`$PYTHON_CONFIG --libs`])
	    AC_SUBST([PYTHON_LIBS], [$pylibs])
	    AC_MSG_RESULT([$pylibs])
            AC_DEFINE([HAVE_PYTHON], [1],
			[Define to 1 if you have the Python3 libraries.])
	])
	], [
	AC_MSG_RESULT([no])])

dnl Set warnings for compilation
dnl CFLAGS is reserved for user, so set the AM_* counterpart.
# macros/ax_cflags_warn_all.m4
AX_CFLAGS_WARN_ALL([AM_CFLAGS])

AC_SUBST([AM_CFLAGS])

m4_syscmd([ cd test; perl generate_defs.pl ])


AC_CONFIG_FILES([Makefile
	doc/Makefile
	rules/Makefile
	scripts/Makefile
	share/Makefile
	src/Makefile
	test/Makefile
	tlf.1])

AC_OUTPUT

echo \
"----------------------------------------------------------------------

 ${PACKAGE_NAME} Version ${PACKAGE_VERSION} configuration:

 Prefix         ${prefix}
 Preprocessor   ${CPP} ${CPPFLAGS}
 C Compiler     ${CC} ${CFLAGS}

 Package features:

 With XML RPC		$wantfldigixmlrpc
 With Python plugin	$wantpythonplugin

-----------------------------------------------------------------------"
