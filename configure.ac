# Process this file with autoconf to produce a configure script.
AC_INIT([Tlf],
	[1.2.1-waedc],
	[rein@couperus.com es tb@forth-ev.de],
	[tlf],
	[https://github.com/Tlf/tlf])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

dnl Clean compilation output makes compiler warnings more visible
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
PKG_CHECK_MODULES(GLIB, glib-2.0)
AC_CHECK_LIB([m], [atan])
AC_CHECK_LIB([pthread], [pthread_create])
ACX_PTHREAD
AC_CHECK_LIB([ncurses], [initscr],,AC_MSG_ERROR([needs ncurses library]))
AC_CHECK_LIB([panel], [update_panels],,
		AC_MSG_ERROR([needs ncurses panel library]))

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h memory.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h syslog.h termios.h unistd.h])

dnl Check if we want to link the Hamradio control libraries (hamlib)
AC_ARG_ENABLE([hamlib],
	[AS_HELP_STRING([--enable-hamlib],
			[Add support for Ham Radio Control Libraries])],
[case "${enableval}" in
	yes) wanthamlib=true ;;
	no)  wanthamlib=false ;;
#	*)   AC_MSG_ERROR([[bad value ${enableval} for --enable-hamlib]]) ;;
	esac],
[wanthamlib=false])

if test "x${wanthamlib}" != "xfalse"; then
    dnl Look for hamlib
    hamlib_modules="hamlib >= 1.2.8"
    PKG_CHECK_MODULES(HAMLIB, [$hamlib_modules], [
                           CFLAGS="$CFLAGS $HAMLIB_CFLAGS";
                           LIBS="$LIBS $HAMLIB_LIBS";
                           ], [
                               AC_MSG_ERROR([Hamradio control libraries 1.2.8 or later not found...])
                           ])

	AC_CHECK_LIB(hamlib,main,,AC_MSG_ERROR([Hamradio control libraries not found...]))
	AC_CHECK_HEADERS(hamlib/rig.h,,AC_MSG_ERROR([Hamlib headers not found...]))
	AC_DEFINE([WANT_HAMLIB], 1, [Want Hamlib])
	echo "enabling Hamlib support"
else
	AC_DEFINE([WANT_HAMLIB], 0, [No Hamlib])
	echo "disabling Hamlib support"
fi

dnl Check if we want to use xmlrpc to read carrier from Fldigi
AC_ARG_ENABLE([fldigi-xmlrpc],
	[AS_HELP_STRING([--enable-fldigi-xmlrpc],
			[Add support for XMLRPC to read Fldigi's carrier (usually in RTTY/DIGIMODE)])],
[case "${enableval}" in
	yes) wantfldigixmlrpc=true ;;
	no)  wantfldigixmlrpc=false ;;
	esac],
[wantfldigixmlrpc=false])

if test "x${wantfldigixmlrpc}" != "xfalse"; then
    dnl Look for xmlrpc libraries

	AC_CHECK_LIB(xmlrpc,xmlrpc_read_int,,AC_MSG_ERROR([Xmlrpc libraries not found...]))
	AC_CHECK_LIB(xmlrpc_client,xmlrpc_client_cleanup,,AC_MSG_ERROR([Xmlrpc-client libraries not found...]))
	AC_CHECK_LIB(xmlrpc_util,xmlrpc_env_init,,AC_MSG_ERROR([Xmlrpc-util libraries not found...]))
	AC_CHECK_HEADERS(xmlrpc-c/base.h,,AC_MSG_ERROR([Xmlrpc base.h header not found...]))
	AC_CHECK_HEADERS(xmlrpc-c/client.h,,AC_MSG_ERROR([Xmlrpc client.h header not found...]))
	echo "enabling XMLRPC for Fldigi"
else
	echo "disabling XMLRPC for Fldigi"
fi

if test "${ac_cv_c_compiler_gnu}" = "yes"; then
	CFLAGS="${CFLAGS} -Wall"
fi



# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([bzero floor ftruncate gethostbyname memset mkfifo putenv select socket sqrt strcasecmp strchr strcspn strdup strpbrk strspn strstr])

# Set PACKAGE_DATA_DIR in config.h.
if test "x${prefix}" = "xNONE"; then
AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR,"${ac_default_prefix}/share/${PACKAGE}",
		   [package_data_dir is in default])
else
AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${prefix}/share/${PACKAGE}",
		   [package_data_dir is in prefix])
fi


AC_CONFIG_FILES([Makefile
	doc/Makefile
	rules/Makefile
	scripts/Makefile
	share/Makefile
	src/Makefile
	tlf.1])

AC_OUTPUT
