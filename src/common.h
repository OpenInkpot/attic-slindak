/*
 * vi: sw=4 ts=4 noexpandtab
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#define MAX_SUITES 8

struct suite {
	char *name;
	char *archlist;
	char *complist;
};

extern struct suite *SUITES[MAX_SUITES];
extern int nsuites;

#define GE_OK    (0)
#define GE_ERROR (-1)
#define GE_EMPTY (-2)

#define GE_ERROR_IFNULL(x) do { if (!(x)) return GE_ERROR; } while (0)

#include <limits.h>
extern char repo_dir[PATH_MAX];
extern char pool_dir[PATH_MAX];
extern char odb_path[PATH_MAX];

#endif

