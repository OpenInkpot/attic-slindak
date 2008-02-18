noinst_PROGRAMS = testsuite

testsuite_CFLAGS = -I../include -I../src -O0 -ggdb
testsuite_LDADD = -lslindak -L../src $(LUA_LIBS) $(SQL_LIBS)
testsuite_SOURCES = \
	testlib.c   \
	testsuite.c

