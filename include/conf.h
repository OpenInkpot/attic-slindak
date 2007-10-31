/*
 * vi: sw=4 ts=4 noexpandtab
 */

#ifndef __SLINDAK_CONF_H__
#define __SLINDAK_CONF_H__

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
	int force;              /* remove apt-ftparchive caches */
};

extern struct global_config G;

#endif /* __SLINDAK_CONF_H__ */

