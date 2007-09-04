/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include "common.h"
#include "db.h"

/* selections are ordered by ... */
#define ORDER_BY  (ov_columns[abs(order)])
#define ORDER_DIR (order >= 0 ? "ASC" : "DESC")

static int row_count = 0;
static int ov_select_cb(void *user, int cols, char **values, char **keys)
{
	int i;

	printf("# select: %s=%s\n", keys[0], values[0]);
	return GE_OK;
}

struct fetch {
	int size;
	char ***data;
};

static int ov_fetch_cb(void *user, int cols, char **values, char **keys)
{
	char **out = (char **)user;
	int i;

	printf("# fetch: ");
	for (i = OV_FIRSTCOL; i < OV_NCOLS; i++) {
		printf("%s=\"%s\" ", keys[i], values[i]);
		if (out)
			out[i] = values[i];
	}
	printf("\n");

	return GE_OK;
}

int ov_search(char *where, int order, char **data)
{
	char *req;
	char *err;

	data[0] = NULL;
	req = sqlite3_mprintf(
			"SELECT " OV_COLS " FROM overrides "
			"WHERE %s ORDER BY %s %s LIMIT 1",
			where, ORDER_BY, ORDER_DIR);

	sqlite3_exec(db, req, ov_fetch_cb, data, &err);
	sqlite3_free(req);

	return (!data[0] ? GE_EMPTY : GE_OK);
}

/*
 * find a source package
 */
int ov_find_component(char *pkgname, char *version, char *arch, char *suite,
		char **component)
{
	char *req;
	char *err;
	char *data[OV_NCOLS];
	int s;

	GE_ERROR_IFNULL(pkgname);
	GE_ERROR_IFNULL(version);
	GE_ERROR_IFNULL(arch);
	GE_ERROR_IFNULL(suite);

	asprintf(&req,
			"pkgname='%s' "
			"AND version='%s' "
			"AND suite='%s' "
			"AND (arch='%s' OR arch='')", /* ??? */
			pkgname, version, suite, arch
		);

	GE_ERROR_IFNULL(req);

	s = ov_search(req, OV_ARCH, data);
	free(req);

	if (s != GE_OK) {
		component = NULL;
		return s;
	}

	*component = strdup(data[OV_COMPONENT]);

	return GE_OK;
}

int ov_find_suite(char *pkgname, char *version, char *arch, char *component,
		char **suite)
{
	char *req;
	char *err;
	char *data[OV_NCOLS];

	GE_ERROR_IFNULL(pkgname);
	GE_ERROR_IFNULL(version);
	GE_ERROR_IFNULL(arch);
	GE_ERROR_IFNULL(component);

	asprintf(&req,
			"pkgname='%s' "
			"AND version='%s' "
			"AND suite='%s' "
			"AND (arch='%s' OR arch='')", /* ??? */
			pkgname, version, suite, arch
		);

	GE_ERROR_IFNULL(req);

	ov_search(req, OV_ARCH, data);
	free(req);

	*suite = strdup(data[OV_SUITE]);

	return GE_OK;
}

