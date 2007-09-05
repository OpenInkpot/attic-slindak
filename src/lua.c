/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "common.h"
#include "debfile.h"

static lua_State *L;

#ifndef LUA_APT_CONF
#define LUA_APT_CONF "lua/apt-conf.lua"
#endif

#ifndef LUA_MAIN_CONF
#define LUA_MAIN_CONF "lua/config.lua"
#endif

int L_push_int_str(int table, int key, char *val)
{
	lua_pushinteger(L, key);
	lua_pushlstring(L, val, strlen(val));
	lua_settable(L, table);

	return 0;
}

int L_push_pair_str(int table, char *key, char *val)
{
	lua_pushlstring(L, key, strlen(key));
	lua_pushlstring(L, val, strlen(val));
	lua_settable(L, table);

	return 0;
}

int L_push_str_vstr(int table, char *key, char *fmt, ...)
{
	va_list ap;
	char *buf;
	int len;
	
	va_start(ap, fmt);
	len = vasprintf(&buf, fmt, ap);
	va_end(ap);

	if (len == -1)
		return GE_ERROR;

	lua_pushlstring(L, key, strlen(key));
	lua_pushlstring(L, buf, len);
	lua_settable(L, table);

	free(buf);

	return GE_OK;
}

/*
 * pushes table name literal and table itself (+2)
 */
int L_push_table(char *table, int parent)
{
	int tbl;

	lua_pushlstring(L, table, strlen(table));
	lua_newtable(L);
	lua_settable(L, parent);

	lua_pushlstring(L, table, strlen(table));
	tbl = lua_gettop(L);
	lua_gettable(L, parent);

	return tbl;
}

#define L_pop_table(x) do { /*lua_pop(L, 2 * x);*/ } while (0)

char *L_call(char *fn, int argc, ...)
{
	va_list ap;
	int n = argc;
	char *ret;

	lua_getfield(L, LUA_GLOBALSINDEX, fn);
	va_start(ap, argc);
	while (n--)
		lua_pushstring(L, va_arg(ap, char *));
	va_end(ap);

	lua_pcall(L, argc, 1, 0);
	ret = lua_tostring(L, -1);

	if (!ret)
		return NULL;

	return strdup(ret);
}

int L_call_aptconf()
{
	int tbl, ttbl;
	int i;
	FILE *aptf;

	/* settings = {} */
	tbl = L_push_table("settings", LUA_GLOBALSINDEX);

	L_push_pair_str(tbl, "POOLTOP", repo_dir);

	/* settings["SUITES"] = {} */
	ttbl = L_push_table("SUITES", tbl);
	
	for (i = 0; i < nsuites; i++)
		L_push_int_str(ttbl, i + 1, SUITES[i]->name);

	L_pop_table(1);

	/* settings["SUITE_SETTINGS"] = {} */
	ttbl = L_push_table("SUITE_SETTINGS", tbl);
	
	for (i = 0; i < nsuites; i++) {
		int ltbl;

		ltbl = L_push_table(SUITES[i]->name, ttbl);
		L_push_str_vstr(ltbl, "bin_list_path",
				"%s_$(SECTION)_$(ARCH).list", SUITES[i]->name);
		L_push_str_vstr(ltbl, "src_list_path",
				"%s_$(SECTION).src.list", SUITES[i]->name);
		L_push_pair_str(ltbl, "components", SUITES[i]->complist);
		L_push_pair_str(ltbl, "arches",     SUITES[i]->archlist);
		L_pop_table(1);
	}

	/* call the f'n */
	/*lua_getfield(L, LUA_GLOBALSINDEX, "dump_table");*/
	lua_getfield(L, LUA_GLOBALSINDEX, "generate_apt_conf");
	/*lua_getfield(L, LUA_GLOBALSINDEX, "settings");*/
	lua_pcall(L, 0, 1, 0);
	aptf = fopen("/tmp/apt-ftparchive.conf", "w");
	fprintf(aptf, "%s\n", lua_tostring(L, -1));
	fclose(aptf);
	lua_pop(L, 1);

	L_pop_table(1);
}

char *L_get_string(char *name, int table)
{
	char *ret;

	lua_getfield(L, table, name);
	ret = lua_tostring(L, -1);
	lua_pop(L, 1);

	if (!ret)
		return NULL;

	return strdup(ret);
}

int L_validate_env()
{

}

int L_dofile(const char *path)
{
	int s;

	s = luaL_dofile(L, path);
	if (s) {
		fprintf(stderr, "Can't load %s: %s\n", path,
				lua_tostring(L, -1));
		lua_pop(L, 1);
		return GE_ERROR;
	}

	return GE_OK;
}

int extl_suite_add(lua_State *L)
{
	char *suite;
	int argc = lua_gettop(L);
	int s;

	if (argc != 3) {
		fprintf(stderr, "%s requires 3 args\n", __FUNCTION__);
		return 0;
	}

	suite = lua_tostring(L, -3);
	s = suite_add(
			lua_tostring(L, -3), /* suite name */
			lua_tostring(L, -2), /* arch list  */
			lua_tostring(L, -1)  /* components */
			);
	if (s != GE_OK) {
		fprintf(stderr, "Failed to add suite %s\n",
				lua_tostring(L, -3));
		return 0;
	}

	printf("Successfully added suite %s\n", suite);
	return 0;
}

int L_init()
{
	int s;

	L = luaL_newstate();
	if (!L)
		return GE_ERROR;

	luaL_openlibs(L);

	lua_register(L, "SuiteAdd", extl_suite_add);

	s = L_dofile(LUA_MAIN_CONF);
	if (s)
		return GE_ERROR;

	s = L_dofile(LUA_APT_CONF);
	if (s)
		return GE_ERROR;

	L_call_aptconf();
}

void L_done()
{
	lua_close(L);
}

