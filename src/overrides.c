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
#define ORDER_BY  (ov_columns[abs(order)])
#define ORDER_DIR (order >= 0 ? "ASC" : "DESC")

static int row_count = 0;
static int ov_select_cb(void *user, int cols, char **values, char **keys)
{
	int i;

	DBG("select: %s=%s\n", keys[0], values[0]);
	return GE_OK;
}

/*
 * return " AND (arch='SOME_ARCH' OR arch='')" that is
 * allocated and requires xfree()'ing
 */
static inline char *ov_arch_clause(char *arch)
{
	char *clause;
	int s = 0;

	if (*arch == '\0')
		clause = arch;
	else
		s = asprintf(&clause, " AND (arch='%s' OR arch='')", arch);

	if (s == -1)
		return NULL;

	return clause;
}

static int ov_fetch_cb(void *user, int cols, char **values, char **keys)
{
	char **out = (char **)user;
	int i;

	DBG("fetch: ");
	for (i = OV_FIRSTCOL; i < OV_NCOLS; i++) {
		_DBG("%s=\"%s\" ", keys[i], values[i]);
		if (out)
			out[i] = values[i];
	}
	_DBG("\n");

	return GE_OK;
}

static int ov_fetch_count_cb(void *user, int cols, char **values, char **keys)
{
	char **out = (char **)user;

	DBG("fetch_count: ");
	_DBG("%s=\"%s\" ", keys[0], values[0]);
	if (out)
		out[0] = values[0];
	_DBG("\n");

	return GE_OK;
}

int ov_insert(char *pkgname, char *version, char *arch,
		char *suite, char *component)
{
	char *req;
	char *err;
	int s;

	req = sqlite3_mprintf(
			"INSERT INTO overrides (" OV_COLS ") "
			"VALUES ('%q', '%q', '%q', '%q', '%q')",
			pkgname, version, suite, arch, component);

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

int ov_update_all(char *pkgname, char *arch, char *suite, char *version,
		char *set_version, char *set_suite, char *set_component,
		char *set_arch)
{
	char *req;
	char *err;
	int s;

	GE_ERROR_IFNULL(pkgname);
	GE_ERROR_IFNULL(suite);
	GE_ERROR_IFNULL(version);

	if (!arch)
		arch = "";

	req = sqlite3_mprintf(
			"UPDATE overrides SET version='%q', suite='%q', "
			"component='%q', arch='%q' "
			"WHERE pkgname='%q' "
			"AND suite='%q' AND version='%q' AND arch='%q'",
			set_version, set_suite, set_component, set_arch,
			pkgname, suite, version, arch);
	DBG("sql req: %s\n", req);

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

int ov_update_version(char *pkgname, char *arch, char *suite, char *version)
{
	char *req;
	char *err;
	char *arch_clause = arch;
	int s;

	GE_ERROR_IFNULL(pkgname);
	GE_ERROR_IFNULL(arch);
	GE_ERROR_IFNULL(suite);
	GE_ERROR_IFNULL(version);

	if (*arch) {
		s = asprintf(&arch_clause, " AND arch='%s'", arch);
		if (s == -1)
			return GE_ERROR;
	}

	req = sqlite3_mprintf(
			"UPDATE overrides SET version='%q' "
			"WHERE pkgname='%q' "
			"AND suite='%q'%s",
			version, pkgname, suite, arch_clause);

	if (arch_clause != arch)
		xfree(arch_clause);

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

int ov_update_suite(char *pkgname, char *version, char *arch,
		char *from_suite, char *to_suite)
{
	char *req;
	char *err;
	char *arch_clause = arch;
	int s;

	GE_ERROR_IFNULL(pkgname);
	GE_ERROR_IFNULL(arch);
	GE_ERROR_IFNULL(from_suite);
	GE_ERROR_IFNULL(to_suite);
	GE_ERROR_IFNULL(version);

	if (*arch) {
		s = asprintf(&arch_clause, " AND arch='%s'", arch);
		if (s == -1)
			return GE_ERROR;
	}

	req = sqlite3_mprintf(
			"UPDATE overrides SET suite='%q' "
			"WHERE pkgname='%q' "
			"AND suite='%q' "
			"AND version='%q'%s",
			to_suite, pkgname, from_suite, version, arch_clause);

	if (arch_clause != arch)
		xfree(arch_clause);

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

int ov_search_count(char *where, char *count_what, int *count)
{
	char *req;
	char *err;
	char *data[OV_NCOLS];
	int s;

	data[0] = NULL;
	req = sqlite3_mprintf(
			"SELECT COUNT(%s) FROM overrides "
			"WHERE %s", count_what, where);

	DBG("sql req: \"%s\"\n", req);
	s = sqlite3_exec(db, req, ov_fetch_count_cb, data, &err);
	if (s != SQLITE_OK) {
		SHOUT("Error (%d) occured while selecting: %s,\n"
				"query was: \"%s\"\n", s, err ? err : "", req);

		s = GE_ERROR;
	} else {
		s = (data[0] ? GE_OK : GE_EMPTY);
		*count = atoi(data[0]);
	}

	sqlite3_free(req);

	return s;
}

int ov_search_all(char *where, void *user, ov_callback_fn callback)
{
	char *req;
	char *err;
	int s;

	req = sqlite3_mprintf("SELECT " OV_COLS " FROM overrides %s", where);

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

int ov_search(char *where, int order, char **data)
{
	char *req;
	char *err;
	int s;

	data[0] = NULL;
	req = sqlite3_mprintf(
			"SELECT " OV_COLS " FROM overrides "
			"WHERE %s ORDER BY %s %s LIMIT 1",
			where, ORDER_BY, ORDER_DIR);

	DBG("sql req: \"%s\"\n", req);
	s = sqlite3_exec(db, req, ov_fetch_cb, data, &err);
	if (s != SQLITE_OK) {
		SHOUT("Error (%d) occured while selecting: %s,\n"
				"query was: \"%s\"\n", s, err ? err : "", req);

		s = GE_ERROR;
	} else
		s = (data[0] ? GE_OK : GE_EMPTY);

	sqlite3_free(req);

	return s;
}

/*
 * find a source package
 */
int ov_find_component(char *pkgname, char *version, char *arch, char *suite,
		char **component)
{
	char *req;
	char *err;
	char *arch_clause;
	char *data[OV_NCOLS];
	int s;

	GE_ERROR_IFNULL(pkgname);
	GE_ERROR_IFNULL(version);
	GE_ERROR_IFNULL(arch);
	GE_ERROR_IFNULL(suite);
 
	arch_clause = ov_arch_clause(arch);
	GE_ERROR_IFNULL(arch_clause);

	asprintf(&req,
			"pkgname='%s' "
			"AND version='%s' "
			"AND suite='%s'"
			"%s",
			pkgname, version, suite, arch_clause
		);

	if (arch_clause != arch)
		xfree(arch_clause);

	GE_ERROR_IFNULL(req);

	s = ov_search(req, -OV_ARCH, data);
	xfree(req);

	if (s != GE_OK) {
		component = NULL;
		return s;
	}

	*component = strdup(data[OV_COMPONENT]);

	return GE_OK;
}

int ov_find_suite(char *pkgname, char *version, char *arch,
		char **suite)
{
	int s;
	char *req;
	char *err;
	char *arch_clause;
	char *data[OV_NCOLS];

	GE_ERROR_IFNULL(pkgname);
	GE_ERROR_IFNULL(version);
	GE_ERROR_IFNULL(arch);

	arch_clause = ov_arch_clause(arch);
	GE_ERROR_IFNULL(arch_clause);

	asprintf(&req,
			"pkgname='%s' "
			"AND version='%s' "
			"%s",
			pkgname, version, arch_clause
		);

	if (arch_clause != arch)
		xfree(arch_clause);

	GE_ERROR_IFNULL(req);

	*suite = NULL;
	s = ov_search(req, OV_ARCH, data);
	xfree(req);

	if (s == GE_EMPTY)
		return GE_EMPTY;

	*suite = strdup(data[OV_SUITE]);

	return GE_OK;
}

int ov_version_count(char *pkgname, char *suite, int *count)
{
	char *req;
	char *err;
	char *data[OV_NCOLS];
	int s;
	int n;

	GE_ERROR_IFNULL(pkgname);
	GE_ERROR_IFNULL(suite);

	asprintf(&req,
			"pkgname='%s' "
			"AND suite='%s'",
			pkgname, suite
		);

	GE_ERROR_IFNULL(req);

	s = ov_search_count(req, ov_columns[OV_VERSION], &n);
	xfree(req);

	if (s != GE_OK)
		return s;

	*count = n;

	return GE_OK;
}

int ov_find_version(char *pkgname, char *arch, char *suite, char **version)
{
	char *req;
	char *err;
	char *data[OV_NCOLS];
	char *arch_clause;
	int s;

	GE_ERROR_IFNULL(pkgname);
	GE_ERROR_IFNULL(arch);
	GE_ERROR_IFNULL(suite);

	arch_clause = ov_arch_clause(arch);
	GE_ERROR_IFNULL(arch_clause);

	asprintf(&req,
			"pkgname='%s' "
			"AND suite='%s'"
			"%s",
			pkgname, suite, arch_clause
		);

	if (arch_clause != arch)
		xfree(arch_clause);

	GE_ERROR_IFNULL(req);

	s = ov_search(req, -OV_ARCH, data);
	xfree(req);

	if (s != GE_OK)
		return s;

	*version = strdup(data[OV_VERSION]);

	return GE_OK;
}

int ov_find_same_uver(char *pkgname, char *uver)
{
	char *req;
	char *err;
	int s, n;

	GE_ERROR_IFNULL(pkgname);
	GE_ERROR_IFNULL(uver);

	asprintf(&req, "pkgname='%s' AND version LIKE '%s-%%'", pkgname, uver);
	GE_ERROR_IFNULL(req);

	s = ov_search_count(req, ov_columns[OV_VERSION], &n);
	xfree(req);

	if (s != GE_OK)
		return s;

	return (n ? GE_OK : GE_EMPTY);
}

int ov_delete(char *pkgname, char *version, char *suite, char *arch)
{
	char *req;
	char *err;
	int s;

	GE_ERROR_IFNULL(pkgname);
	GE_ERROR_IFNULL(version);
	GE_ERROR_IFNULL(suite);

	if (!arch)
		arch = "";

	req = sqlite3_mprintf(
			"DELETE  FROM overrides "
			"WHERE pkgname='%q' AND version='%q' AND suite='%q' "
			"AND arch='%q'",
			pkgname, version, suite, arch);

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

int ov_create_table()
{
	char *req;
	char *err;
	int s;

	req = sqlite3_mprintf("CREATE TABLE overrides ("
			"pkgname varchar NOT NULL,"
			"version varchar NOT NULL,"
			"suite char(24) NOT NULL,"
			"arch char(32) NOT NULL,"
			"component varchar NOT NULL,"
			"UNIQUE(pkgname, version, suite, arch));");

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

