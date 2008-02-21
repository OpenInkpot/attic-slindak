/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include "common.h"
#include "db.h"
#include "util.h"

/* selections are ordered by ... */
#define ORDER_BY  (bc_columns[abs(order)])
#define ORDER_DIR (order >= 0 ? "ASC" : "DESC")

static int row_count = 0;
static int bc_select_cb(void *user, int cols, char **values, char **keys)
{
	int i;

	DBG("select: %s=%s\n", keys[0], values[0]);
	return GE_OK;
}

int bc_search_all(char *where, void *user, bc_callback_fn callback)
{
	char *req;
	char *err;
	int s;

	req = sqlite3_mprintf("SELECT " BC_COLS " FROM binary_cache %s", where);

	DBG("sql req: \"%s\"\n", req);
	s = sqlite3_exec(db, req, callback, user, &err);
	if (s != SQLITE_OK) {
		SHOUT("Error (%d) occured while selecting: %s,\n"
				"query was: \"%s\"\n", s, err ? err : "", req);

		s = GE_ERROR;
	} else
		s = GE_OK;

	sqlite3_free(req);

	return s;
}

int bcov_search_all(char *suite, char *arch, int existing, void *user,
		bc_callback_fn callback)
{
	char *req;
	char *err;
	int s;

	req = sqlite3_mprintf(
			"SELECT " OV_COLS " FROM overrides O "
			" WHERE O.suite='%q' "
			"   AND (O.arch='%q' OR O.arch='') "
			"   AND %s EXISTS ( "
			"SELECT 1 FROM binary_cache B "
			" WHERE O.pkgname=B.pkgname "
			"   AND O.suite=B.suite "
			"   AND O.version=B.version "
			"   AND (B.deb_arch='%q' OR B.deb_arch='')) "
			" ORDER BY pkgname ASC",
			suite, arch, existing ? "" : "NOT", arch
			);

	DBG("sql req: \"%s\"\n", req);
	s = sqlite3_exec(db, req, callback, user, &err);
	if (s != SQLITE_OK) {
		SHOUT("Error (%d) occured while selecting: %s,\n"
				"query was: \"%s\"\n", s, err ? err : "", req);

		s = GE_ERROR;
	} else
		s = GE_OK;

	sqlite3_free(req);

	return s;
}

int bc_clear()
{
	char *req;
	char *err;
	int s;

	req = sqlite3_mprintf("DELETE FROM binary_cache");

	DBG("sql req: \"%s\"\n", req);
	s = sqlite3_exec(db, req, NULL, NULL, &err);
	if (s != SQLITE_OK) {
		SHOUT("Error (%d) occured while deleting: %s,\n"
				"query was: \"%s\"\n", s, err ? err : "", req);

		s = GE_ERROR;
	} else
		s = GE_OK;

	sqlite3_free(req);

	return s;
}

int bc_insert_debf(struct debfile *debf)
{
	char *req;
	char *err;
	int s;

	req = sqlite3_mprintf(
			"INSERT INTO binary_cache (" BC_COLS ") "
			"VALUES ('%q', '%q', '%q', '%q', '%q', "
			"'%q', '%q', '%d', '%q', '%q')",
			debf->source, debf->version, debf->suite,
			debf->pool_file, debf->debname, debf->component,
			debf->arch, debf->deb_size, debf->deb_md5, debf->deb_control);
	DBG("sql req: \"%s\"\n", req);

	s = sqlite3_exec(db, req, NULL, NULL, &err);

	if (s != SQLITE_OK) {
		SHOUT("Error (%d) occured while updating the table: %s,\n"
				"query was: \"%s\"\n", s, err ? err : "", req);

		s = GE_ERROR;
	} else
		s = GE_OK;

	sqlite3_free(req);

	return s;
}

int bc_create_table()
{
	char *req;
	char *err;
	int s;

	s = bc_search_all("", NULL, NULL);
	if (s == GE_OK) {
		DBG("binary_cache table seems to exist in overrides.db\n");
		return GE_OK;
	}

	DBG("Creating binary_cache table in overrides.db\n");
	req = sqlite3_mprintf("CREATE TABLE binary_cache ("
			"pkgname varchar NOT NULL,"
			"version varchar NOT NULL,"
			"suite  char(24) NOT NULL,"
			"pool_file varchar NOT NULL,"
			"deb_name varchar NOT NULL,"
			"deb_section varchar NOT NULL,"
			"deb_arch char(32) NOT NULL,"
			"deb_size int NOT NULL,"
			"deb_md5 char(32) NOT NULL,"
			"deb_control text NOT NULL,"
			"UNIQUE(pkgname, suite, deb_arch, deb_name, deb_section));");

	DBG("sql req: \"%s\"\n", req);
	s = sqlite3_exec(db, req, NULL, NULL, &err);
	if (s != SQLITE_OK) {
		SHOUT("Error (%d) occured while selecting: %s,\n"
				"query was: \"%s\"\n", s, err ? err : "", req);

		s = GE_ERROR;
	} else
		s = GE_OK;

	sqlite3_free(req);

	return s;
}

