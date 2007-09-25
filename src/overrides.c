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

struct fetch {
	int size;
	char ***data;
};

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

int ov_insert(char *pkgname, char *version, char *arch,
		char *suite, char *component)
{
	char *req;
	char *err;
	int s;

	req = sqlite3_mprintf(
			"INSERT INTO overrides (" OV_COLS ") "
			"VALUES ('%s', '%s', '%s', '%s', '%s')",
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
			"UPDATE overrides SET version='%s' "
			"WHERE pkgname='%s' "
			"AND suite='%s'%s",
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
			"UPDATE overrides SET suite='%s' "
			"WHERE pkgname='%s' "
			"AND suite='%s' "
			"AND version='%s'%s",
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

