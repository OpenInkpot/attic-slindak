/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

#include "common.h"
#include "db.h"
#include "ov.h"

#ifndef QF_DEFAULT
#define QF_DEFAULT "%v"
#endif

#define QUERY_BUFSIZE 4096

static void query_printf(const char *fmt, const char *pkgname,
		char *version, char *suite, char *arch, char *component)
{
	char buf[QUERY_BUFSIZE];
	char *s, *d = buf;
	int mod = 0;

	for (s = fmt; *s && d - buf < QUERY_BUFSIZE; s++) {
		if (*s == '%') { mod++; continue; }
		else if (!mod) { *d++ = *s; continue; }

		mod = 0;
		switch (*s) {
			case 'p':
				d += snprintf(d, QUERY_BUFSIZE - (d - buf), "%s", pkgname);
				break;

			case 'v':
				d += snprintf(d, QUERY_BUFSIZE - (d - buf), "%s", version);
				break;

			case 's':
				d += snprintf(d, QUERY_BUFSIZE - (d - buf), "%s", suite);
				break;

			case 'c':
				d += snprintf(d, QUERY_BUFSIZE - (d - buf), "%s", component);
				break;

			case 'a':
				d += snprintf(d, QUERY_BUFSIZE - (d - buf), "%s", arch);
				break;

			default:
				DBG("Unknown format modifier: %%%c\n", *s);
		}
	}

	SAY(buf);
}

static int query_fetch_cb(void *user, int cols, char **values, char **keys)
{
	const char *fmt = (const char *)user;

	query_printf(fmt, values[0], values[1], values[2], values[3], values[4]);
	SAY("\n");

	return GE_OK;
}

int query_pkglist(const char *component, char *suite,
		char *arch, const char *fmt)
{
	char *where;
	int s;

	GE_ERROR_IFNULL(suite);

	if (!fmt) fmt = QF_DEFAULT;
	if (!arch) arch = "all";

	s = asprintf(&where,
			"WHERE suite='%s'"
			"  AND component='%s'"
			"  AND (arch='%s' OR arch='')",
			suite, component, arch);
	if (s == -1)
		return GE_ERROR;

	s = ov_search_all(where, fmt, query_fetch_cb);
	free(where);

	return s;
}

int query_deblist(int exists, char *suite,
		char *arch, const char *fmt)
{
	char *where;
	int s;

	GE_ERROR_IFNULL(suite);

	if (!fmt) fmt = QF_DEFAULT;
	if (!arch) arch = "all";

	s = bcov_search_all(suite, arch, exists, fmt, query_fetch_cb);

	return s;
}

int query_pkginfo(const char *pkgname, char *suite,
		char *arch, const char *fmt)
{
	char *ver;
	int s;

	GE_ERROR_IFNULL(suite);

	if (!fmt) fmt = QF_DEFAULT;
	if (!arch) arch = "all";

	s = ov_find_version(pkgname, arch, suite, &ver);
	if (s != GE_OK) {
		SAY("NOTFOUND\n");
		return GE_ERROR;
	}

	query_printf(fmt, pkgname, ver, suite, arch, "COMPONENT");
	SAY("\nOK\n");
	free(ver);

	return GE_OK;
}

