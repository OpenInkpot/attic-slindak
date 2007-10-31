/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "configuration.h"
#include "lua-helpers.h"

extern lua_State *L;

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

int L_load_aptconf()
{
	int s;

	s = L_dofile(LUA_APT_CONF);
	if (s)
		return GE_ERROR;

	return GE_OK;
}

