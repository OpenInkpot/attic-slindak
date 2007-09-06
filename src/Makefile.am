bin_PROGRAMS = slindak

slindak_SOURCES = main.c db.c overrides.c debfile.c slind.c lua.c util.c output.c
slindak_CFLAGS = -DBUILD_DATE="\"$(BUILD_DATE)\"" -DHACKER=$(HACKER) $(LUA_CFLAGS)
slindak_LDADD = $(SQL_LIBS) $(LUA_LIBS)

