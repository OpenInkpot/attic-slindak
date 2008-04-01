/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "common.h"
#include "debfile.h"
#include "conf.h"
#include "ov.h"

int deb_ver_gt(char *v1, char *v2)
{
	char *argv[] = { "dpkg", "--compare-versions", v1, "gt", v2, NULL };
	int ret;

	ret = spawn(DPKG_BIN_PATH, argv);
	if (ret)
		return GE_ERROR;

	return GE_OK;
}

/* skip whitespaces */
#define skipws(P)              \
	do {                       \
		while (                \
				' ' == *P  ||  \
				'\t' == *P ||  \
				'\n' == *P     \
			  ) P++;           \
	} while (0);

/*
 * control file parser callback for binary controls
 * understands: Package, Version, Architecture, Section, Source
 * This function is called from parse_buf() when a key-value pair
 * is read from a control file.
 */
static int debfile_scanner_cb(struct debfile *df, char *key, char *value)
{
	if (!strcmp(key, "Package")) {
		strncpy(df->debname, value, DF_NAMELEN);
	} else if (!strcmp(key, "Version")) {
		strncpy(df->version, value, DF_VERLEN);
	} else if (!strcmp(key, "Architecture")) {
		/* for arch="all" leave df->arch empty */
		if (strcmp(value, "all"))
			strncpy(df->arch, value, DF_ARCHLEN);
	} else if (!strcmp(key, "Section")) {
		strncpy(df->component, value, DF_COMPLEN);
	} else if (!strcmp(key, "Source")) {
		strncpy(df->source, value, DF_SRCLEN);
	}

	return GE_OK;
}

/*
 * control file parser callback for source controls
 * understands: Package, Version, Architecture, Section, Source, Files
 * This function is called from parse_buf() when a key-value pair
 * is read from a control file.
 */
static int dscfile_scanner_cb(struct dscfile *df, char *key, char *value)
{
	if (!strcmp(key, "Package") && !df->pkgname[0]) {
		strncpy(df->pkgname, value, DF_SRCLEN);
	} else if (!strcmp(key, "Version") && !df->version[0]) {
		strncpy(df->version, value, DF_VERLEN);
	} else if (!strcmp(key, "Architecture") && !df->arch[0]) {
		if (!strcmp(value, "all") || !strcmp(value, "any"))
			df->arch[0] = '\0';
		else
			strncpy(df->arch, value, DF_ARCHLEN);
	} else if (!strcmp(key, "Section") && !df->component[0]) {
		strncpy(df->component, value, DF_COMPLEN);
	} else if (!strcmp(key, "Source") && !df->pkgname[0]) {
		strncpy(df->pkgname, value, DF_SRCLEN);
	} else if (!strcmp(key, "Files")) {
		struct pkgfile PF;
		char *__p = value;

		while (__p && sscanf(__p, "%32s %d %s\n",
					PF.md5sum, &PF.size, PF.name) == 3) {
			struct pkgfile *pf = malloc(sizeof(struct pkgfile));
			GE_ERROR_IFNULL(pf);

			DBG("## => File: %s, size: %d, md5: %s\n", PF.name, PF.size, PF.md5sum);
			strncpy(pf->name, PF.name, FILENAME_MAX);
			strncpy(pf->md5sum, PF.md5sum, 33);
			pf->size = PF.size;

			df->files[df->nfiles++] = pf;

			__p = strchr(__p, '\n');
			if (__p) skipws(__p);
		}
	}

	return GE_OK;
}

/* callback type for control file scanner */
typedef int (*scanner_fn)(void *user, char *key, char *value);

/* parser states */
enum {
	AT_NEW = 0, /* expect a key /^([^:]+):/ */
	AT_KEY      /* expect value string(s) */
};

/* possible string delimiters, per state */
const char *delims[] = {
	/* AT_NEW */":",
	/* AT_KEY */"\n"
};

/*
 * Parse a buffer containing a control file.
 * Calls scanner_cb() for each key-value pair found.
 */
int parse_buf(char *buf, scanner_fn scanner_cb, void *user)
{
	char *start = buf, *end;
	char *token, *key = NULL, *val = NULL;
	int state = AT_NEW;

	do {
		int i, d = -1;
		char *_end;

		skipws(start);

		end = start + strlen(buf);
		for (i = 0; i < strlen(delims[state]); i++) {
			_end = strchr(start, delims[state][i]);
			if (!_end) continue;
			if (_end == start) continue;
			if (_end <= end) {
				end = _end;
				d = i;
			}
		}

		if (d == -1)
			break;

		if (end > start) {
			char *__p = end + 1;

			token = strndup(start, end - start);
			if (!token) {
				SHOUT("SHIT!\n");
				return GE_ERROR;
			}

			switch (state) {
				case AT_NEW:
					state = AT_KEY;

					if (key) {
						if (scanner_cb(user, key, val) != GE_OK)
							return GE_ERROR;

						free(key);
						free(val);
						key = val = NULL;
					}

					key = token;
					DBG("found key: \"%s\"\n", token);
					break;

				case AT_KEY:
					/* this is unsafe, since skipws() can go outside
					 * the input buffer */
					skipws(__p); /* __p now points to first non-whitespace */

					if (__p - end <= 1)
						state = AT_NEW;

					if (val) {
						/* 2 == newline + nul terminator */
						val = realloc(val, strlen(val) + strlen(token) + 2);
						sprintf(val, "%s\n%s", val, token);
					} else
						val = token;

					DBG("found value: \"%s\"\n", token);
					break;

				default:
					SHOUT("Internal slindak error at %s:%d\n",
							__FILE__, __LINE__);
			}
		}

		start = end + 1;
	} while (*end);

	if (scanner_cb(user, key, val) != GE_OK)
		return GE_ERROR;

	return GE_OK;
}

int debfile_read(const char *path, struct debfile *df)
{
	FILE *p;
	char cmd[PATH_MAX];
	char tok[256];
	struct stat sbuf;
	int s;

	s = stat(path, &sbuf);
	if (s) {
		SHOUT("Can't stat %s\n", path);
		return GE_ERROR;
	}

	memset(df, 0, sizeof(struct debfile));

	/* make sure the file is within our repo, just in case */
	if (G.op_mode == OM_POOL) {
		s = strlen(G.repo_dir);
		if (strncmp(path, G.repo_dir, s)) {
			SHOUT("slindak internal error at %s:%d\n"
					"package path=\"%s\" pool path=\"%s\"",
					__FILE__, __LINE__, path, G.repo_dir);
			abort();
		}

		/* cut away slashes */
		for (; path[s] == '/'; s++);
		/* get in-pool path to the package */
		strncpy(df->pool_file, &path[s], PATH_MAX);
	}

	df->deb_size = sbuf.st_size;

	s = md5sum(path, df->deb_md5);
	if (s != GE_OK) {
		SHOUT("Can't fetch md5 for %s\n", path);
		return GE_ERROR;
	}

	read_pipe(&df->deb_control, "ar p %s control.tar.gz|tar zxO ./control", path);
	GE_ERROR_IFNULL(df->deb_control);

	parse_buf(df->deb_control, debfile_scanner_cb, df);

	if (df->source[0] == '\0')
		strncpy(df->source, df->debname, DF_SRCLEN);

	/* treat cross packages specially */
	if (df->arch[0] == '\0' && strlen(df->debname) > 6) {
		char *p = df->debname + strlen(df->debname) - 6;
		int n = 0;

		/* FIXME: there might be something-wacky-cross_XX.YY_all.deb */
		if (!strcmp(p, "-cross")) {
			while (*--p != '-' && p > df->debname);

			while (*++p != '-')
				df->crossarch[n++] = *p;
		}
	}

	return GE_OK;
}

void debfile_free(struct debfile *debf)
{
	if (debf->suite)
		free(debf->suite);

	if (debf->deb_control)
		free(debf->deb_control);
}

int dscfile_read(const char *path, struct dscfile *df)
{
	char *buf;

	memset(df, 0, sizeof(struct dscfile));

	read_file(&buf, path);
	GE_ERROR_IFNULL(buf);

	parse_buf(buf, dscfile_scanner_cb, df);
	free(buf);

	if (df->component[0] == '\0')
		strcpy(df->component, "host-tools");

	return GE_OK;
}

void display_deb_info(struct debfile *debf)
{
	char *suite;
	int s;

	s = ov_find_suite(debf->source, debf->version, debf->arch, &suite);
	if (s != GE_OK)
		return;

	SAY(
			"Binary package information:\n"
			"\tName: %s\n"
			"\tVersion: %s\n"
			"\tSource: %s\n"
			"\tArchitecture: %s\n"
			"\tSection (component): %s\n"
			"\tSuite: %s\n",
			debf->debname, debf->version, debf->source,
			debf->arch, debf->component, suite
	   );

	free(suite);
}

void display_dsc_info(struct dscfile *dscf)
{
	char *suite;
	int s;

	s = ov_find_suite(dscf->pkgname, dscf->version, "", &suite);
	if (s != GE_OK)
		return;

	SAY(
			"Binary package information:\n"
			"\tName: %s\n"
			"\tVersion: %s\n"
			"\tSection (component): %s\n"
			"\tSuite: %s\n",
			dscf->pkgname, dscf->version,
			dscf->component, suite
	   );

	free(suite);
}

