AC_INIT([slindak],[0.1])
AM_INIT_AUTOMAKE()

AC_PROG_CC
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AC_HEADER_STDC

PKG_CHECK_MODULES(LUA, lua5.1)
AC_SUBST(LUA_LIBS)
AC_SUBST(LUA_CFLAGS)

AC_CHECK_LIB(cgi, htmlentities, [CGI_LIBS="-lcgi"], [echo "$0: libcgi is missing" >&2; exit 2])
AC_SUBST(CGI_LIBS)

AC_CHECK_LIB(sqlite3, sqlite3_open, [SQL_LIBS="-lsqlite3"], [echo "$0: libsqlite3 is missing" >&2; exit 2])
AC_SUBST(SQL_LIBS)

AC_PATH_PROG(DPKG_BIN_PATH, dpkg)
AC_PATH_PROG(GIT_BIN_PATH, git)
AC_PATH_PROG(TAR_BIN_PATH, tar)
AC_PATH_PROG(CP_BIN_PATH, cp)
AC_PATH_PROG(MV_BIN_PATH, mv)
AC_PATH_PROG(RM_BIN_PATH, rm)
AC_PATH_PROG(MKDIR_BIN_PATH, mkdir)
AC_PATH_PROG(MD5SUM_BIN_PATH, md5sum)
AC_DEFINE_UNQUOTED([DPKG_BIN_PATH],   ["$DPKG_BIN_PATH"], ["Path to dpkg executable"])
AC_DEFINE_UNQUOTED([GIT_BIN_PATH],    ["$GIT_BIN_PATH"], ["Path to git executable"])
AC_DEFINE_UNQUOTED([TAR_BIN_PATH],    ["$TAR_BIN_PATH"], ["Path to tar executable"])
AC_DEFINE_UNQUOTED([CP_BIN_PATH],     ["$CP_BIN_PATH"],  ["Path to cp executable"])
AC_DEFINE_UNQUOTED([MV_BIN_PATH],     ["$MV_BIN_PATH"], ["Path to mv executable"])
AC_DEFINE_UNQUOTED([RM_BIN_PATH],     ["$RM_BIN_PATH"], ["Path to rm executable"])
AC_DEFINE_UNQUOTED([MKDIR_BIN_PATH],  ["$MKDIR_BIN_PATH"], ["Path to mkdir executable"])
AC_DEFINE_UNQUOTED([MD5SUM_BIN_PATH], ["$MD5SUM_BIN_PATH"], ["Path to md5sum executable"])
AC_ARG_WITH(hacker,
		AS_HELP_STRING([--with-hacker],
			[enables hacker mode]),
		[HACKER=1],
		[HACKER=0])
AC_SUBST(HACKER)

BUILD_DATE="`822-date`"
AC_SUBST(BUILD_DATE)

AC_SUBST(AM_CFLAGS, "\
	-g -Wall -std=c89 \
	-I\$(top_srcdir) \
	-I\$(top_srcdir)/include \
	-DSYSCONFDIR=\\\"\$(sysconfdir)\\\" \
	-DLOCALSTATEDIR=\\\"\$(localstatedir)\\\"")

AC_CONFIG_HEADERS([config.h])
AC_OUTPUT([
	Makefile
	src/Makefile
])
