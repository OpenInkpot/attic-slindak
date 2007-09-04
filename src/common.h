/*
 * vi: sw=4 ts=4 noexpandtab
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#define NSUITES 3
static char *suites[NSUITES] = { "clydesdale", "percheron", "attic" };

#define GE_OK    (0)
#define GE_ERROR (-1)
#define GE_EMPTY (-2)

#define GE_ERROR_IFNULL(x) do { if (!(x)) return GE_ERROR; } while (0)

#include <limits.h>
extern char repo_dir[PATH_MAX];
extern char pool_dir[PATH_MAX];
extern char odb_path[PATH_MAX];

struct list_file {
	char name[PATH_MAX];
	FILE *file;
};

#define MAX_SUITES 8
extern struct list_file *lists[MAX_SUITES * 2];
extern int nlists;

#endif

