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
#include "configuration.h"
#include "debfile.h"

static lua_State *L;

#if HACKER
#define LUA_APT_CONF "lua/apt-conf.lua"
#define LUA_MAIN_CONF "lua/slindak-config.lua"
#endif

#ifndef LUA_APT_CONF
#define LUA_APT_CONF "/usr/share/slindak/apt-conf.lua"
#endif

#ifndef LUA_MAIN_CONF
#define LUA_MAIN_CONF "/etc/slindak-config.lua"
#endif

#ifndef LUA_TABLE_SUITES
#define LUA_TABLE_SUITES "Suites"
#endif

#ifndef LUA_TABLE_CONFIG
#define LUA_TABLE_CONFIG "Config"
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
	ret = (char *)lua_tostring(L, -1);
	lua_pop(L, 1);

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
		L_pop_table(1);
	}

	/* call the f'n */
	/*lua_getfield(L, LUA_GLOBALSINDEX, "dump_table");*/
	lua_getfield(L, LUA_GLOBALSINDEX, "generate_apt_conf");
	/*lua_getfield(L, LUA_GLOBALSINDEX, "settings");*/
	lua_pcall(L, 0, 1, 0);
	aptf = fopen(G.apt_config, "w");
	fprintf(aptf, "%s\n", lua_tostring(L, -1));
	fclose(aptf);
	lua_pop(L, 1);

	L_pop_table(1);
}

char *L_get_string(char *name, int table)
{
	char *ret;

	lua_getfield(L, table, name);
	ret = (char *)lua_tostring(L, -1);
	lua_pop(L, 1);

	if (!ret)
		return NULL;

	return strdup(ret);
}

char *L_get_confstr(char *field, char *table)
{
	char *ret;

	lua_getfield(L, LUA_GLOBALSINDEX, table);
	ret = L_get_string(field, -1);
	lua_pop(L, 1);

	if (!ret)
		return NULL;

	return strdup(ret);
}

int L_validate_env()
{
	int tbl, ttbl;

	lua_getfield(L, LUA_GLOBALSINDEX, LUA_TABLE_CONFIG);
	tbl = lua_gettop(L);

	if (G.repo_dir)
		L_push_pair_str(tbl, "repo_dir", G.repo_dir);

	return GE_OK;
}

int L_dofile(const char *path)
{
	int s;

	s = luaL_dofile(L, path);
	if (s) {
		SHOUT("Can't load %s: %s\n", path,
				lua_tostring(L, -1));
		lua_pop(L, 1);
		return GE_ERROR;
	}

	return GE_OK;
}

int L_get_array(int index, char **array, int narr)
{
	int i = 0;

	lua_pushnil(L);
	while (lua_next(L, index) && i < narr) {
		lua_pushvalue(L, -1);
		DBG(" * %s\n",
				lua_tostring(L, -1));

		array[i++] = strdup(lua_tostring(L, -1));
		lua_pop(L, 2);
	}

	return i;
}

int extl_suite_add(lua_State *L)
{
	char *suite;
	char *archlist[MAX_ARCHES];
	char *complist[MAX_COMPS];
	int argc = lua_gettop(L);
	int s, tbl;

	if (argc != 1) {
		SHOUT("%s requires 1 argument\n", __FUNCTION__);
		return 0;
	}

	suite = (char *)lua_tostring(L, -1);
	lua_pop(L, 1);

	s = get_suite_by_name(suite);
	if (s != GE_ERROR)
		suite_remove(s);
	else
		s = -1;

	memset(archlist, 0, sizeof(archlist));
	memset(complist, 0, sizeof(complist));

	DBG("Hashing suite %s\n", suite);
	lua_getfield(L, LUA_GLOBALSINDEX, LUA_TABLE_SUITES);
	lua_getfield(L, -1, suite);
	lua_getfield(L, -1, "arches");

	L_get_array(-2, archlist, MAX_ARCHES);

	lua_pop(L, 1);
	lua_getfield(L, -1, "components");

	L_get_array(-2, complist, MAX_COMPS);

	lua_pop(L, 4);
	suite_add(suite, archlist, complist, s);

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

	L_validate_env();
}

void L_done()
{
	lua_close(L);
}

