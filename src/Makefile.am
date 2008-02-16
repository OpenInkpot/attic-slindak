bin_PROGRAMS = slindak
lib_LTLIBRARIES = libslindak.la
cgidir = $(prefix)/lib/cgi-bin
cgi_PROGRAMS = slindweb

libslindak_la_SOURCES = \
	configuration.c\
	biglock.c      \
	db.c           \
	overrides.c    \
	debfile.c      \
	slind.c        \
	util.c         \
	lua.c          \
	output.c       \
	library.c
libslindak_la_CFLAGS = -DBUILD_DATE="\"$(BUILD_DATE)\"" -DHACKER=$(HACKER) $(LUA_CFLAGS) -I../include
libslindak_la_LDFLAGS = -Wl,-soname=$(SLINDAK_SONAME)

slindak_SOURCES = \
	slindak.c      \
	aptconf.c      \
	lists.c        \
	query.c        \
	messages.c     \
	package.c      \
	pool.c
slindak_CFLAGS = -DBUILD_DATE="\"$(BUILD_DATE)\"" -DHACKER=$(HACKER) $(LUA_CFLAGS) -I../include
slindak_LDADD = $(SQL_LIBS) $(LUA_LIBS) $(POPT_LIBS) -lslindak

slindweb_SOURCES = \
	cgi-actions.c \
	html_output.c \
	slindweb.c
slindweb_CFLAGS = -DBUILD_DATE="\"$(BUILD_DATE)\"" -DHACKER=$(HACKER) $(LUA_CFLAGS) -I../include
slindweb_LDADD = $(SQL_LIBS) $(LUA_LIBS) $(POPT_LIBS) $(CGI_LIBS) -lslindak

