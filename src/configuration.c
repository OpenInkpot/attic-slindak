/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <sys/types.h>
#include "common.h"
#include "lua.h"
#include "configuration.h"
#include "util.h"

struct global_config G;

int config_init()
{
	int s;
	char *d;

	CONFIG_SET(repo_dir, "Config");
	CONFIG_SET(devel_suite, "Config");
	CONFIG_SET(attic_suite, "Config");

	s = asprintf(&G.pool_dir, "%s/pool", G.repo_dir);
	if (s == -1)
		return GE_ERROR;

	s = asprintf(&G.odb_path, "%s/indices/overrides.db", G.repo_dir);
	if (s == -1)
		return GE_ERROR;
	mkpdir(G.odb_path);

	s = asprintf(&G.apt_config, "%s/scripts/apt-ftparchive.conf", G.repo_dir);
	if (s == -1)
		return GE_ERROR;
	mkpdir(G.apt_config);

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

