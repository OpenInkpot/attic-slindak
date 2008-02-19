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
#include "configuration.h"
#include "lua.h"
#include "db.h"
#include "debfile.h"
#include "util.h"
#include "biglock.h"

const char *cli_file = NULL;
const char *inj_file = NULL;
const char *cli_query = NULL;
const char *cli_qfmt = NULL;
const char *cli_list = NULL;

static struct poptOption opts_table[] = {
	{ "info",     'I', POPT_ARG_STRING, &cli_file, 0,
	  "obtain information on a package (may misbehave)" },
	{ "inject",   'i', POPT_ARG_STRING, &inj_file, 0,
	  "inject a package into a repository" },
	{ "query",    'q', POPT_ARG_STRING, &cli_query, 0,
	  "query package information from database" },
	{ "queryfmt", 'Q', POPT_ARG_STRING, &cli_qfmt, 0,
	  "output format string for query results" },
	{ "list",     'l', POPT_ARG_STRING, &cli_list, 0,
	  "list packages with given component known to database" },
	{ "repodir",  'r', POPT_ARG_STRING, &G.repo_dir, 0,
	  "repository base directory" },
	{ "suite",    's', POPT_ARG_STRING, &G.users_suite, 0,
	  "specify a name of a suite" },
	{ "arch",     'a', POPT_ARG_STRING, &G.users_arch, 0,
	  "specify target architecture name" },
	{ "cleanup",  'C', POPT_ARG_NONE,   &G.cleanup, 0,
	  "remove binary packages that do not match source packages "
	  "known to overrides.db" },
	{ "force",    'F', POPT_ARG_NONE,   &G.force, 0,
	  "forced complete rebuild of all the indices, "
	  "despices apt-ftparchive's caches." },
	{ "verbose",  'v', 0, 0, 'v', "turn on debugging output"   },
	{ "version",  'V', 0, 0, 'V', "show our version number"    },
	{ "help",     'h', 0, 0, 'h', "print help message"         },
	POPT_TABLEEND
};

int main(int argc, const char **argv)
{
	int s;
	char o;
	poptContext optcon;
	
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

	s = config_init();
	if (s != GE_OK) {
		SHOUT("Error initializing configuration.\n");
		exit(EXIT_FAILURE);
	}

	L_load_aptconf();

	OUT[LOG] = fopen(G.logfile, "w");
	if (!OUT[LOG]) {
		SHOUT("Can't open logfile\n");
		exit(EXIT_FAILURE);
	}

	s = db_init(G.odb_path);
	if (s != GE_OK) {
		SHOUT("Can't open database\n");
		exit(EXIT_FAILURE);
	}

	if (cli_query) {
		G.op_mode = OM_QUERY;

		s = query_pkginfo(cli_query, G.users_suite, G.users_arch, cli_qfmt);

		exit(s == GE_OK ? EXIT_SUCCESS : EXIT_FAILURE);
	}

	if (cli_list) {
		G.op_mode = OM_QUERY;

		s = query_pkglist(cli_list, G.users_suite, G.users_arch, cli_qfmt);

		exit(s == GE_OK ? EXIT_SUCCESS : EXIT_FAILURE);
	}

	if (cli_file) {
		struct debfile debf;
		struct dscfile dscf;

		G.op_mode = OM_INFO;
		if (FILE_IS_DEB(cli_file)) {
			s = debfile_read(cli_file, &debf);
			if (s != GE_OK)
				exit(EXIT_FAILURE);

			display_deb_info(&debf);
		} else if (FILE_IS_DSC(cli_file)) {
			s = dscfile_read(cli_file, &dscf);
			if (s != GE_OK)
				exit(EXIT_FAILURE);

			display_dsc_info(&dscf);
		} else {
			SHOUT("File is neither .deb nor .dsc. I'm puzzled.\n");
			exit(EXIT_FAILURE);
		}

		exit(EXIT_SUCCESS);
	}

	if (inj_file) {
		G.op_mode = OM_INJECT;

		if (FILE_IS_DEB(inj_file)) {
			struct debfile debf;

			SAY("Injecting binary package %s\n", inj_file);

			process_deb(inj_file);
		} else if (FILE_IS_DSC(inj_file)) {
			SAY("Injecting source package %s\n", inj_file);
			SHOUT("Not implemented yet.\n");
		} else {
			SHOUT("File is neither .deb nor .dsc. I'm puzzled.\n");
			exit(EXIT_FAILURE);
		}

		exit(EXIT_SUCCESS);
	}

	G.op_mode = OM_POOL;

	if (bl_take(G.repo_dir) != GE_OK) abort();

	lists_cleanup();
	s = scan_pool();
	L_call_aptconf();

	db_done();
	done_slind();
	L_done();

	config_done();

	poptFreeContext(optcon);

	if (s != GE_OK)
		return 0;

	if (!G.cleanup) {
		SAY2("Running apt-ftparchive to generate indices... ");
		s = apt_ftparchive();
		if (s != GE_OK) {
			SHOUT("\nError running apt-ftparchive\n");
			exit(EXIT_FAILURE);
		}
		SAY("Done.\n");
	}

	bl_release();

	fclose(OUT[LOG]);

	return 0;
}

