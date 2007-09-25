/*
 * vi: sw=4 ts=4 noexpandtab
 */

#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

typedef enum {
	OM_NONE = 0,
	OM_POOL,
	OM_INJECT,
	OM_INFO
} op_mode_t;

struct global_config {
	/* paths */
	char *repo_dir;
	char *pool_dir;
	char *odb_path;
	char *apt_config;       /* path to apt-ftparchive.conf */
	char *logfile;

	/* suites */
	char *devel_suite;      /* current 'unstable' */
	char *attic_suite;      /* dead n4r storage */
	char *users_suite;      /* -s option */

	/* general */
	op_mode_t op_mode;      /* operation mode, that is */
	int cleanup;            /* remove leftovers */
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

