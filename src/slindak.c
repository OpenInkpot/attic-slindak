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
#include "query.h"
#include "package.h"
#include "pool.h"

static const char *cli_file = NULL;
static const char *inj_file = NULL;
static const char *cli_query = NULL;
static const char *cli_qfmt = NULL;
static const char *cli_list = NULL;
static int cli_bin_inc = 0;
static int cli_bin_excl = 0;

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
	{ "binary",   'b', POPT_ARG_NONE,   NULL,      'b',
	  "list only those packages that have at least one "
	  "binary package built from them" },
	{ "nobinary", 'B', POPT_ARG_NONE,   NULL,      'B',
	  "list only those packages that have no "
	  "binary packages built from them" },
	{ "repodir",  'r', POPT_ARG_STRING, &G.repo_dir, 0,
	  "repository base directory" },
	{ "suite",    's', POPT_ARG_STRING, &G.users_suite, 0,
	  "specify a name of a suite" },
	{ "arch",     'a', POPT_ARG_STRING, &G.users_arch, 0,
	  "specify target architecture name" },
	{ "cleanup",  'C', POPT_ARG_NONE,   &G.cleanup, 0,
	  "remove binary packages that do not match source packages "
	  "known to overrides.db" },
	{ "cache",    'c', POPT_ARG_NONE,   &G.cached, 0,
	  "use cached information on binary packages" },
	{ "force",    'F', POPT_ARG_NONE,   &G.force, 0,
	  "forced complete rebuild of all the indices, "
	  "despices apt-ftparchive's caches." },
	{ "verbose",  'v', 0, NULL, 'v', "turn on debugging output"   },
	{ "version",  'V', 0, NULL, 'V', "show our version number"    },
	{ "help",     'h', 0, NULL, 'h', "print help message"         },
	POPT_TABLEEND
};

/* from messages.c */
void help(void);
void version(void);

static void prologue(void)
{
	OUT[LOG] = fopen(G.logfile, "w");
	if (!OUT[LOG]) {
		SHOUT("Can't open logfile\n");
		exit(EXIT_FAILURE);
	}

	if (bl_take(G.repo_dir) != GE_OK) abort();
}

static void epilogue(void)
{
	bl_release();

	fclose(OUT[LOG]);
}

int main(int argc, const char **argv)
{
	int s;
	int o;
	poptContext optcon;

	memset(&G, 0, sizeof(struct global_config));

	optcon = poptGetContext(NULL, argc, argv, opts_table, 0);
	poptSetOtherOptionHelp(optcon, "[<option>]");

	while ((o = poptGetNextOpt(optcon)) >= 0) {
		switch (o) {
			case 'b':
				cli_bin_inc++;
				break;

			case 'B':
				cli_bin_excl++;
				break;

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

	libslindak_lock();
	init_slind();
	push_cleaner(done_slind);

	L_init();
	push_cleaner(L_done);

	s = config_init();
	if (s != GE_OK) {
		SHOUT("Error initializing configuration.\n");
		libslindak_unlock();
		exit(EXIT_FAILURE);
	}

	push_cleaner(config_done);
	libslindak_unlock();

	L_load_aptconf();

	libslindak_lock();
	prologue();
	push_cleaner(epilogue);

	s = db_init(G.odb_path);
	if (s != GE_OK) {
		SHOUT("Can't open database\n");
		libslindak_unlock();
		exit(EXIT_FAILURE);
	}

	push_cleaner(db_done);
	libslindak_unlock();

	if (cli_query) {
		G.op_mode = OM_QUERY;

		s = query_pkginfo(cli_query, G.users_suite, G.users_arch, cli_qfmt);

		exit(s == GE_OK ? EXIT_SUCCESS : EXIT_FAILURE);
	}

	if (cli_list) {
		G.op_mode = OM_QUERY;

		s = (cli_bin_inc || cli_bin_excl)
			? query_deblist(cli_bin_inc, G.users_suite, G.users_arch,
					cli_qfmt)
			: query_pkglist(cli_list, G.users_suite, G.users_arch, cli_qfmt);

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
	if (!G.cached)
		bc_clear();

	lists_cleanup();
	s = scan_pool();
	L_call_aptconf();

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

	return 0;
}

