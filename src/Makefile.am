bin_PROGRAMS = slindak
lib_LTLIBRARIES = libslindak.la

libslindak_la_SOURCES = \
	db.c           \
	overrides.c    \
	debfile.c      \
	slind.c        \
	util.c         \
	output.c
libslindak_la_CFLAGS = -DBUILD_DATE="\"$(BUILD_DATE)\"" -DHACKER=$(HACKER) $(LUA_CFLAGS)
libslindak_la_LDFLAGS = -Wl,-soname=$(SLINDAK_SONAME)

slindak_SOURCES = \
	slindak.c      \
	lists.c        \
	lua.c          \
	messages.c     \
	package.c      \
	pool.c         \
	configuration.c
slindak_CFLAGS = -DBUILD_DATE="\"$(BUILD_DATE)\"" -DHACKER=$(HACKER) $(LUA_CFLAGS)
slindak_LDADD = $(SQL_LIBS) $(LUA_LIBS) $(POPT_LIBS) -lslindak

