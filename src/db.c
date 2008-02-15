/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <unistd.h>
#include <errno.h>
#include "common.h"
#include "ov.h"

sqlite3 *db;

static int sql_authoriser(void *a, int b, const char *c, const char *d,
		const char *e, const char *f)
{
	return SQLITE_OK;
}

int db_init(char *db_path)
{
	int s, call_create = 0;
	
	s = access(db_path, R_OK | W_OK);
	if (s == -1) {
		switch (errno) {
			case ENOENT:
				call_create++;
				break;

			default:
				return GE_ERROR;
		}
	}

	s = sqlite3_open(db_path, &db);
	if (s != SQLITE_OK)
		return GE_ERROR;

	sqlite3_set_authorizer(db, sql_authoriser, NULL);

	if (call_create)
		ov_create_table();

	return GE_OK;
}

void db_done()
{
	sqlite3_close(db);
}

