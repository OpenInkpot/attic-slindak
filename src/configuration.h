/*
 * vi: sw=4 ts=4 noexpandtab
 */

#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

struct global_config {
	/* paths */
	char *repo_dir;
	char *pool_dir;
	char *odb_path;
	char *apt_config;       /* path to apt-ftparchive.conf */

	/* suites */
	char *devel_suite;      /* current 'unstable' */
	char *attic_suite;      /* dead n4r storage */
};

extern struct global_config G;

#define CONFIG_SET(name, table) \
	do { \
		if (!G.name) \
			G.name = L_get_confstr(# name, table); \
	} while (0);

int config_init();
void config_done();

#endif

