/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <sys/types.h>
#include <lua.h>
#include <lualib.h>
#include "common.h"
#include "lua-helpers.h"
#include "configuration.h"
#include "util.h"

struct global_config G;

int extl_suite_add(lua_State *L);

static int config_load()
{
	int tbl, s;

	/* install lua c call(s) */
	lua_register(L, "SuiteAdd", extl_suite_add);

	s = L_dofile(LUA_MAIN_CONF);
	if (s)
		return GE_ERROR;

	/* override Config["repo_dir"] if necessary */
	if (G.repo_dir) {
		lua_getfield(L, LUA_GLOBALSINDEX, LUA_TABLE_CONFIG);
		tbl = lua_gettop(L);

		L_push_pair_str(tbl, "repo_dir", G.repo_dir);
	}

	return GE_OK;
}

int config_init()
{
	int s;
	char *d;

	s = config_load();
	if (s != GE_OK) {
		SHOUT("Can't load configuration file %s\n", LUA_MAIN_CONF);
		return GE_ERROR;
	}

	CONFIG_SET(repo_dir, "Config");
	CONFIG_SET(devel_suite, "Config");
	CONFIG_SET(attic_suite, "Config");

	s = asprintf(&G.pool_dir, "%s/pool", G.repo_dir);
	if (s == -1)
		return GE_ERROR;

	s = asprintf(&G.odb_path, "%s/indices/overrides.db", G.repo_dir);
	if (s == -1)
		return GE_ERROR;
	mkpdir(G.odb_path);

	s = asprintf(&G.apt_config, "%s/scripts/apt-ftparchive.conf", G.repo_dir);
	if (s == -1)
		return GE_ERROR;
	mkpdir(G.apt_config);

	s = asprintf(&G.logfile, "%s/scripts/slindak.log", G.repo_dir);
	if (s == -1)
		return GE_ERROR;

	G.devel_suite = L_get_confstr("devel_suite", "Config");
	G.attic_suite = L_get_confstr("attic_suite", "Config");

	return GE_OK;
}

void config_done()
{
	free(G.repo_dir);
	free(G.pool_dir);
	free(G.odb_path);
	free(G.devel_suite);
	free(G.attic_suite);
}

