/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include "common.h"
#include "lua-helpers.h"
#include "configuration.h"
#include "db.h"
#include "debfile.h"

struct suite *SUITES[MAX_SUITES];
int nsuites = 0;

int suite_add(char *name, char **arches, char **comps, int idx)
{
	if (idx == -1)
		idx = nsuites;

	if (idx >= MAX_SUITES - 1)
		return GE_ERROR;

	SUITES[idx] = malloc(sizeof(struct suite));
	if (!SUITES[idx]) {
		do {
			free(SUITES[--idx]->name);
			free(SUITES[idx]);
		} while (idx);

		return GE_ERROR;
	}

	SUITES[idx]->name = strdup(name);
	memcpy(SUITES[idx]->archlist, arches, MAX_ARCHES * sizeof(char *));
	memcpy(SUITES[idx]->complist, comps, MAX_COMPS * sizeof(char *));

	if (idx == nsuites)
		nsuites++;

	return GE_OK;
}

void suite_remove(int idx)
{
	int i;

	if (idx > MAX_SUITES || !SUITES[idx])
		return;

	for (i = 0; SUITES[idx]->archlist[i]; i++)
		free(SUITES[idx]->archlist[i]), SUITES[idx]->archlist[i] = NULL;
	for (i = 0; SUITES[idx]->complist[i]; i++)
		free(SUITES[idx]->complist[i]), SUITES[idx]->complist[i] = NULL;

	free(SUITES[idx]->name);
	free(SUITES[idx]);
}

void suite_remove_all()
{
	while (nsuites--)
		suite_remove(nsuites);
}

int init_slind()
{
	memset(SUITES, 0, sizeof(SUITES));

	return GE_OK;
}

void done_slind()
{
	suite_remove_all();
}

int get_suite_by_name(char *suite)
{
	int i;

	for (i = 0; i < nsuites; i++)
		if (SUITES[i] && !strcmp(suite, SUITES[i]->name))
			return i;

	return GE_ERROR;
}

/* interface function for lua */
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

