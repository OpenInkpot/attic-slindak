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

const char *cli_file = NULL;
const char *inj_file = NULL;

static struct poptOption opts_table[] = {
	{ "info",     'I', POPT_ARG_STRING, &cli_file, 0,
	  "obtain information on a package (may misbehave)" },
	{ "inject",   'i', POPT_ARG_STRING, &inj_file, 0,
	  "inject a package into a repository" },
	{ "repodir",  'r', POPT_ARG_STRING, &G.repo_dir, 0,
	  "repository base directory" },
	{ "suite",    's', POPT_ARG_STRING, &G.users_suite, 0,
	  "specify a name of a suite" },
	{ "cleanup",  'C', POPT_ARG_NONE,   &G.cleanup, 0,
	  "remove binary packages that do not match source packages "
	  "known to overrides.db" },
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

	s = config_init();
	if (s != GE_OK) {
		SHOUT("Error initializing configuration.\n");
		exit(EXIT_FAILURE);
	}

	OUT[LOG] = fopen(G.logfile, "w");
	if (!OUT[LOG]) {
		SHOUT("Can't open logfile\n");
		exit(EXIT_FAILURE);
	}

	s = db_init();
	if (s != GE_OK) {
		SHOUT("Can't open database\n");
		exit(EXIT_FAILURE);
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
	lists_cleanup();
	scan_pool();
	L_call_aptconf();

	db_done();
	done_slind();
	L_done();

	config_done();

	poptFreeContext(optcon);

	SAY2("Running apt-ftparchive to generate indices... ");
	s = apt_ftparchive();
	if (s != GE_OK) {
		SHOUT("\nError running apt-ftparchive\n");
		exit(EXIT_FAILURE);
	}
	SAY("Done.\n");

	fclose(OUT[LOG]);

	return 0;
}

