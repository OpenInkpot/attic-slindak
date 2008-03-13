/*
 * vi: sw=4 ts=4 noexpandtab
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "common.h"
#include "lua-helpers.h"
#include "configuration.h"
#include "debfile.h"

lua_State *L;

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

int L_init(void)
{
	L = luaL_newstate();
	if (!L)
		return GE_ERROR;

	luaL_openlibs(L);

	L_push_pair_str(LUA_GLOBALSINDEX, "BUILD_DATE", BUILD_DATE);

	return GE_OK;
}

void L_done(void)
{
	lua_close(L);
}

