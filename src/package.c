/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <limits.h>

#include "common.h"
#include "configuration.h"
#include "lua.h"
#include "db.h"
#include "debfile.h"
#include "util.h"

/*
 * Match binary package against database and inject it into a proper
 * location.
 */
int inject_deb(struct debfile *debf, char *suite, char *path)
{
	int s;
	char *newpath;
	char *c;

	s = ov_find_component(debf->source, debf->version, debf->arch,
			suite, &c);
	if (s != GE_OK) {
		SHOUT("While processing %s: source package %s not known to "
				"overrides.db\n", path, debf->source);
		return GE_EMPTY;
	}

	newpath = mk_pool_path(debf->component, debf->source, suite);
	if (!newpath)
		return GE_ERROR;

	DBG("pool path %s", newpath);
	mkdir_p(newpath, 0755);
	copy(path, newpath);

	pkg_append(path, suite, debf->arch, c, 0);
	free(c);
	free(newpath);

	return GE_OK;
}

int process_deb(char *path)
{
	int s;
	struct debfile debf;
	char *c;

	s = debfile_read(path, &debf);
	if (s != GE_OK)
		return GE_ERROR;

	if (G.op_mode == OM_POOL) {
		/* package's parent directory must reflect suite */
		debf.suite = parent_dir(path, 1);
		if (!debf.suite) {
			SHOUT("Package in a wrong directory: %s (use -C)\n", path);
			return GE_ERROR;
		}

		goto tryadd;
	} else if (G.op_mode == OM_INJECT) {
		/* we request that the suite name be specified by user */
		if (!G.users_suite) {
			SHOUT("Please specify a suite.\n");
			return GE_ERROR;
		}

		s = inject_deb(&debf, G.users_suite, path);
		if (s != GE_OK)
			return GE_ERROR;

		return GE_OK;
	} else {
		/* no other operation mode should call this function */
		SHOUT("slindak internal error at %s:%s\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}

	/* we shouldn't ever get here */
	abort();

tryadd:
	/* check if suite matches overrides.db */
	s = ov_find_component(debf.source, debf.version,
			debf.crossarch[0] ? debf.crossarch : debf.arch,
			debf.suite, &c);
	if (s == GE_OK) {
		DBG("adding %s=%s: [%s] [%s]\n",
				debf.debname, debf.version,
				debf.component, debf.arch);
		if (!debf.arch[0]) {
			int an, sn;

			sn = get_suite_by_name(debf.suite);
			for (an = 0; SUITES[sn]->archlist[an]; an++)
				pkg_append(path, debf.suite, SUITES[sn]->archlist[an],
						debf.component, 0);
		} else
			pkg_append(path, debf.suite, debf.arch, debf.component, 0);
		bc_insert_debf(&debf);
		free(c);
		free(debf.suite);

		return GE_OK;
	}

	/* it doesn't, thus should be removed in OM_POOL */
	SHOUT("Package %s doesn't match overrides.db (use -C)\n", path);

	return GE_ERROR;
}

/*
 * Process a source package
 * ------------------------
 * 1. Check for matching records in overrides.db for each suite, add
 *    the package to corresponding package list(s).
 * 2. Check for other versions of this package in devsuite, compare
 *    versions, if our version is newer, move older one to attic and
 *    replace it with our newer version in devsuite. Add to corresponding
 *    package lists.
 */
int process_dsc(char *path)
{
	int s, sn;
	char *c;
	struct dscfile dscf;

	s = dscfile_read(path, &dscf);
	if (s != GE_OK)
		return GE_ERROR;

	for (sn = 0; SUITES[sn]; sn++) {
		char *__arch, *st = NULL;

		/*
		 * dscf.arch contains a list of architectures for which
		 * a package is intended to build, therefore we split it
		 * into separate architecture names and add them to
		 * overrides.db separately, is needed
		 * XXX: this code needs lots of love
		 */
		__arch = strtok_r(dscf.arch, " ", &st);
		do {
			if (!__arch) __arch = "";

			s = ov_find_component(dscf.pkgname, dscf.version, __arch,
					SUITES[sn]->name, &c);
			if (s == GE_OK) {
				pkg_append(path, SUITES[sn]->name,
						__arch, dscf.component, 1);

				free(c);
			} else { /* XXX: only devsuite */
				char *ver;
				int n;

				if (strcmp(SUITES[sn]->name, G.devel_suite))
					break;

				s = ov_find_version(dscf.pkgname, __arch,
						SUITES[sn]->name, &ver);
				if (s != GE_OK) {
					SAY("Adding package %s (%s, %s, %s)\n",
							dscf.pkgname, dscf.version, __arch,
							SUITES[sn]->name);

					ov_insert(dscf.pkgname, dscf.version, __arch,
							SUITES[sn]->name, dscf.component);

					pkg_append(path, SUITES[sn]->name,
							__arch, dscf.component, 1);
				} else {
					s = ov_version_count(dscf.pkgname,
							SUITES[sn]->name, &n);
					if (s == GE_OK) {
						if (n > 1) {
							/* Most likely this means that there is a new source version
							 * of the package which is overriden for some architectures,
							 * and someone forgot to update overrides.db. The safest way
							 * here is to bail and stop processing this dsc.
							 */
							SHOUT("Package %s=%s should be added to overrides.db"
									" manually\n", dscf.pkgname, dscf.version);
							return s;
						}
					}

					s = deb_ver_gt(dscf.version, ver);
					if (s == GE_OK) {
						SAY("Found newer version of %s (%s >> %s)\n",
								dscf.pkgname, dscf.version, ver);
						ov_update_suite(dscf.pkgname, ver, "",
								SUITES[sn]->name, G.attic_suite);
						ov_insert(dscf.pkgname, dscf.version, __arch,
								SUITES[sn]->name, dscf.component);

						pkg_append(path, SUITES[sn]->name,
								__arch, dscf.component, 1);
					}

					free(ver);
				}
			}

			__arch = __arch[0] ? strtok_r(NULL, " ", &st) : NULL;
		} while (__arch);
	}
}

int validate_deb(char *path)
{
	int s;
	struct debfile debf;
	char *suite, *c;

	s = debfile_read(path, &debf);
	if (s != GE_OK)
		return GE_ERROR;

	if (G.op_mode != OM_POOL || !G.cleanup) {
		/* no other operation mode should call this function */
		SHOUT("slindak internal error at %s:%s\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}

	/* package's parent directory must reflect suite */
	debf.suite = parent_dir(path, 1);
	if (!debf.suite) {
		SAY("Package in a wrong directory: %s, removing\n", path);
		unlink(path);
		free(debf.suite);

		return GE_ERROR;
	}

	s = ov_find_component(debf.source, debf.version,
			debf.crossarch[0] ? debf.crossarch : debf.arch,
			debf.suite, &c);
	if (s != GE_OK) {
		SAY("Found zero mentions of %s=%s in overrides, removing.\n",
				debf.source, debf.version);

		unlink(path);
	}

	return GE_OK;
}

int validate_dsc(char *path)
{
	int i, s, users = 0;
	char *suite, *__arch, *st = NULL;
	struct dscfile dscf;

	s = dscfile_read(path, &dscf);
	if (s != GE_OK)
		return GE_ERROR;

	if (G.op_mode != OM_POOL || !G.cleanup) {
		/* no other operation mode should call this function */
		SHOUT("slindak internal error at %s:%s\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}

	__arch = strtok_r(dscf.arch, " ", &st);
	do {
		if (!__arch) __arch = "";

		/* check if there's a record for this package in overrides.db */
		s = ov_find_suite(dscf.pkgname, dscf.version, __arch, &suite);
		users += (s == GE_OK);

		__arch = __arch[0] ? strtok_r(NULL, " ", &st) : NULL;
	} while (__arch);

	if (!users) {
		char *dir = parent_dir(path, 0);

		SAY("Found zero mentions of %s=%s in overrides, removing.\n",
				dscf.pkgname, dscf.version);
		unlink(path);

		for (i = 0; i < dscf.nfiles; i++) {
			char fpath[PATH_MAX];

			snprintf(fpath, PATH_MAX, "%s/%s", dir, dscf.files[i]->name);

			/*
			 * for .orig.tar.gz files, check if other versions might be
			 * using them before removing
			 */
			if (FILE_IS_ORIG(fpath)) {
				char uver[DF_VERLEN], *p;

				p = strchr(dscf.version, '-');
				if (!p) {
					SHOUT("Package %s is native, but has an orig tarball\n",
							dscf.pkgname);

					continue;
				}

				strncpy(uver, dscf.version, p - dscf.version);
				uver[p - dscf.version] = '\0';
				DBG("UPSTREAM VERSION: %s\n", uver);

				s = ov_find_same_uver(dscf.pkgname, uver);
				if (s != GE_OK) {
					DBG("REMOVING %s\n", fpath);
					unlink(fpath);
				} else
					DBG("NOT REMOVING %s\n", fpath);
			} else {
				unlink(fpath);
				DBG("REMOVING %s\n", fpath);
			}
		}

		free(dir);
	}

	return GE_OK;
}

