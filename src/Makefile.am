bin_PROGRAMS = slindak slindweb
lib_LTLIBRARIES = libslindak.la

libslindak_la_SOURCES = \
	configuration.c\
	db.c           \
	overrides.c    \
	debfile.c      \
	slind.c        \
	util.c         \
	lua.c          \
	output.c
libslindak_la_CFLAGS = -DBUILD_DATE="\"$(BUILD_DATE)\"" -DHACKER=$(HACKER) $(LUA_CFLAGS) -I../include
libslindak_la_LDFLAGS = -Wl,-soname=$(SLINDAK_SONAME)

slindak_SOURCES = \
	slindak.c      \
	aptconf.c      \
	lists.c        \
	messages.c     \
	package.c      \
	pool.c
slindak_CFLAGS = -DBUILD_DATE="\"$(BUILD_DATE)\"" -DHACKER=$(HACKER) $(LUA_CFLAGS) -I../include
slindak_LDADD = $(SQL_LIBS) $(LUA_LIBS) $(POPT_LIBS) -lslindak

slindweb_SOURCES = \
	slindweb.c
slindweb_CFLAGS = -DBUILD_DATE="\"$(BUILD_DATE)\"" -DHACKER=$(HACKER) $(LUA_CFLAGS) -I../include
slindweb_LDADD = $(SQL_LIBS) $(LUA_LIBS) $(POPT_LIBS) $(CGI_LIBS) -lslindak

