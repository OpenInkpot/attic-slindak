/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <popt.h>
#include "common.h"
#include "lua.h"
#include "db.h"
#include "debfile.h"
#include "util.h"

struct global_config G;

static struct poptOption opts_table[] = {
	{ "repodir",  'r', POPT_ARG_STRING, &G.repo_dir, 0,
	  "repository base directory" },
	{ "verbose",  'v', 0, 0, 'v', "turn on debugging output"   },
	{ "version",  'V', 0, 0, 'V', "show our version number"    },
	{ "help",     'h', 0, 0, 'h', "print help message"         },
	POPT_TABLEEND
};

#define CONFIG_SET(name, table) \
	do { \
		if (!G.name) \
			G.name = L_get_confstr(# name, table); \
	} while (0);

int config_init()
{
	int s;

	CONFIG_SET(repo_dir, "Config");
	CONFIG_SET(devel_suite, "Config");
	CONFIG_SET(attic_suite, "Config");

	s = asprintf(&G.pool_dir, "%s/pool", G.repo_dir);
	if (s == -1)
		return GE_ERROR;

	s = asprintf(&G.odb_path, "%s/indices/overrides.db", G.repo_dir);
	if (s == -1)
		return GE_ERROR;

	G.devel_suite = L_get_confstr("devel_suite", "Config");
	G.attic_suite = L_get_confstr("attic_suite", "Config");

	return GE_OK;
}

void config_done()
{
	free(G.repo_dir);
	free(G.pool_dir);
	free(G.odb_path);
	free(G.devel_suite);
	free(G.attic_suite);
}

void check_file(char *path, void *data)
{
	char *p = path;
	char *suite;
	struct debfile debf;
	struct dscfile dscf;
	char *c;
	int s;

	/* XXX: consider a simple '.deb' check sufficient? */
	p += strlen(path) - 4;
	if (!strcmp(p, ".deb")) {
		s = debfile_read(path, &debf);
		if (s != GE_OK)
			return;

		/* XXX: skip packages that are not placed in a suite-named
		 * directory */
		suite = parent_dir(path);
		if (!suite)
			return;

		/* validate the suite name */
		if (get_suite_by_name(suite) == GE_ERROR) {
			free(suite);
			return;
		}

		s = ov_find_component(debf.source, debf.version, debf.arch,
				suite, &c);
		if (s == GE_OK) {
			DBG("%s=%s: dists/clydesdale/%s/binary-%s/Packages\n",
					debf.debname, debf.version,
					debf.component, debf.arch);
			pkg_append(path, suite, debf.arch, debf.component, 0);
			free(c);
		}

		free(suite);
	} else if (!strcmp(p, ".dsc")) {
		int sn;

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
}

int main(int argc, const char **argv)
{
	int s;
	char *c, o;
	poptContext optcon;
	
	output_init();
	root_squash();

	memset(&G, 0, sizeof(struct global_config));

	optcon = poptGetContext(NULL, argc, argv, opts_table, 0);
	poptSetOtherOptionHelp(optcon, "[<option>]");

	while ((o = poptGetNextOpt(optcon)) >= 0) {
		switch (o) {
			case 'h':
				help();
				poptPrintHelp(optcon, OUT[STD], 0);
				exit(EXIT_SUCCESS);
				break;

			case 'V':
				version();
				exit(EXIT_SUCCESS);
				break;

			case 'v':
				verbosity = VERB_DEBUG;
				break;

			default:
				SHOUT("Invalid option on the command line");
				exit(EXIT_FAILURE);
		}
	}

	if (o < -1) {
		poptPrintUsage(optcon, OUT[ERR], 0);
		poptFreeContext(optcon);
		exit(EXIT_FAILURE);
	}

	init_slind();
	L_init();

	config_init();

	s = db_init();
	if (s != GE_OK) {
		SHOUT("Can't open database\n");
		exit(EXIT_FAILURE);
	}

	traverse(G.repo_dir, check_file, NULL);

	db_done();
	done_slind();
	L_done();

	config_done();

	poptFreeContext(optcon);

	system("apt-ftparchive generate /tmp/apt-ftparchive.conf");

	return 0;
}

