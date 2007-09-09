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
#include "db.h"
#include "debfile.h"
#include "util.h"

char *repo_dir;
char pool_dir[PATH_MAX];
char odb_path[PATH_MAX];

static struct poptOption opts_table[] = {
	{ "verbose",  'v', 0, 0, 'v', "turn on debugging output"   },
	{ "version",  'V', 0, 0, 'V', "show our version number"    },
	{ "help",     'h', 0, 0, 'h', "print help message"         },
	POPT_TABLEEND
};

void check_file(char *path)
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
			}
		}
	}
}

int main(int argc, char **argv)
{
	int s, sn, an, cn;
	char *c, *fn, o;
	poptContext optcon;
	
	output_init();
	root_squash();

	optcon = poptGetContext(NULL, argc, argv, opts_table, 0);
	poptSetOtherOptionHelp(optcon, "[<option>] <path-to-repository>");

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

	repo_dir = poptGetArg(optcon);
	if (!repo_dir) {
		SHOUT("A repository path is required\n");
		exit(EXIT_FAILURE);
	}

	snprintf(pool_dir, PATH_MAX, "%s/pool", repo_dir);
	snprintf(odb_path, PATH_MAX, "%s/indices/overrides.db", repo_dir);

	init_slind();
	L_init();

	s = db_init();
	if (s != GE_OK) {
		SHOUT("Can't open database\n");
		exit(EXIT_FAILURE);
	}

	/* clean all the lists */
	for (sn = 0; sn < nsuites; sn++)
		for (cn = 0; SUITES[sn]->complist[cn]; cn++) {
			fn = L_call("RenderSrcListFileName",
					2, SUITES[sn]->name, SUITES[sn]->complist[cn]);
			unlink(fn);
			free(fn);

			for (an = 0; SUITES[sn]->archlist[an]; an++) {
				fn = L_call("RenderListFileName",
						3, SUITES[sn]->name,
						SUITES[sn]->archlist[an],
						SUITES[sn]->complist[cn]);
				unlink(fn);
				free(fn);
			}
		}
				
	traverse(repo_dir, check_file, NULL);

	db_done();
	done_slind();
	L_done();
	poptFreeContext(optcon);

	system("apt-ftparchive generate /tmp/apt-ftparchive.conf");

	return 0;
}

