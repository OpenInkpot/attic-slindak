bin_PROGRAMS = slindak

slindak_SOURCES = main.c db.c overrides.c debfile.c slind.c lua.c
slindak_CFLAGS = -DBUILD_DATE="\"$(BUILD_DATE)\"" $(LUA_CFLAGS)
slindak_LDADD = $(SQL_LIBS) $(LUA_LIBS)

