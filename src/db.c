/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include "common.h"
#include "configuration.h"

sqlite3 *db;

static int sql_authoriser(void *a, int b, const char *c, const char *d,
		const char *e, const char *f)
{
	return SQLITE_OK;
}

int db_init()
{
	int s;
	
	s = sqlite3_open(G.odb_path, &db);
	if (s != SQLITE_OK)
		return GE_ERROR;

	sqlite3_set_authorizer(db, sql_authoriser, NULL);

	return GE_OK;
}

void db_done()
{
	sqlite3_close(db);
}

