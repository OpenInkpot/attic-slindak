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

