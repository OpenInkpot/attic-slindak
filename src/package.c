/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

#include "common.h"
#include "configuration.h"
#include "lua.h"
#include "db.h"
#include "debfile.h"
#include "util.h"

int inject_deb(struct debfile *debf, char *suite, char *path)
{
	int s;
	char *newpath;
	char *c;

	s = ov_find_component(debf->source, debf->version, debf->arch,
			suite, &c);
	if (s != GE_OK) {
		SHOUT("Source package %s not known to overrides.db\n", debf->source);
		return GE_EMPTY;
	}

	newpath = mk_pool_path(c, debf->source, suite);
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
	char *suite, *c;

	s = debfile_read(path, &debf);
	if (s != GE_OK)
		return;

	if (G.op_mode == OM_POOL) {
		/* package's parent directory must reflect suite */
		suite = parent_dir(path, 1);
		if (!suite) {
			SHOUT("Package in a wrong directory: %s\n", path);
			/*unlink(path);*/
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
	s = ov_find_component(debf.source, debf.version, debf.arch,
			suite, &c);
	if (s == GE_OK) {
		DBG("adding %s=%s: [%s] [%s]\n",
				debf.debname, debf.version,
				debf.component, debf.arch);
		pkg_append(path, suite, debf.arch, debf.component, 0);
		free(c);
		free(suite);

		return GE_OK;
	}

	free(suite);

	/* it doesn't, thus should be removed in OM_POOL */
	SHOUT("Package %s doesn't match overrides.db.\n", path);
	/*unlink(path);*/

	return GE_ERROR;
}

int process_dsc(char *path)
{
	int s, sn;
	char *c;
	struct dscfile dscf;

	dscfile_read(path, &dscf);

	for (sn = 0; SUITES[sn]; sn++) {
		s = ov_find_component(dscf.pkgname, dscf.version, dscf.arch,
				SUITES[sn]->name, &c);
		if (s == GE_OK) {
			pkg_append(path, SUITES[sn]->name,
					dscf.arch, dscf.component, 1);

			free(c);
		} else { /* XXX: only devsuite */
			char *ver;

			if (strcmp(SUITES[sn]->name, G.devel_suite))
				continue;

			s = ov_find_version(dscf.pkgname, dscf.arch,
					SUITES[sn]->name, &ver);
			if (s != GE_OK) {
				SAY("Adding package %s (%s, %s, %s)\n",
						dscf.pkgname, dscf.version, dscf.arch,
						SUITES[sn]->name);
				ov_insert(dscf.pkgname, dscf.version, dscf.arch,
						SUITES[sn]->name, dscf.component);
			} else {
				s = deb_ver_gt(dscf.version, ver);
				if (s == GE_OK) {
					SAY("Found newer version of %s (%s >> %s)\n",
							dscf.pkgname, dscf.version, ver);
					ov_update_suite(dscf.pkgname, ver, "",
							SUITES[sn]->name, G.attic_suite);
					ov_insert(dscf.pkgname, dscf.version, dscf.arch,
							SUITES[sn]->name, dscf.component);
				}

				free(ver);
			}
		}
	}
}

