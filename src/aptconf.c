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
#include "lua-helpers.h"

extern lua_State *L;

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

int L_validate_env()
{
	int tbl, ttbl;

	lua_getfield(L, LUA_GLOBALSINDEX, LUA_TABLE_CONFIG);
	tbl = lua_gettop(L);

	if (G.repo_dir)
		L_push_pair_str(tbl, "repo_dir", G.repo_dir);

	return GE_OK;
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

void add_luacalls()
{
	int s;

	lua_register(L, "SuiteAdd", extl_suite_add);

	s = L_dofile(LUA_MAIN_CONF);
	if (s)
		return GE_ERROR;

	s = L_dofile(LUA_APT_CONF);
	if (s)
		return GE_ERROR;

	L_validate_env();
}

