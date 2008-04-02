/*
 * vi: sw=4 ts=4 noexpandtab
 */

/*
 * Mass operation on all recognizable objects in a pool
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "common.h"
#include "configuration.h"
#include "debfile.h"
#include "package.h"
#include "util.h"
#include "bc.h"

char *mk_pool_path(char *comp, char *pkgname, char *suite, int abs)
{
	char *newpath = NULL;
	int s;

	if (!strncmp(pkgname, "lib", 3))
		s = asprintf(&newpath, "%s/%s/lib%c/%s/%s", abs ? G.pool_dir : "pool",
				comp, pkgname[3], pkgname, suite);
	else
		s = asprintf(&newpath, "%s/%s/%c/%s/%s", abs ? G.pool_dir : "pool",
				comp, pkgname[0], pkgname, suite);

	if (s == -1)
		return NULL;

	return newpath;
}

/*
 * Files we might care about are stored in a singly-linked
 * list.
 */
struct file_entry {
	struct file_entry *next;
	int n;
	char pathname[0];
};

static struct file_entry debs_list = { .next = NULL, .n = 0 };
static struct file_entry dscs_list = { .next = NULL, .n = 0 };

static int new_file(char *name, struct file_entry *parent)
{
	struct file_entry *new;
	size_t len;

	len = strlen(name);
	if (len > PATH_MAX)
		return GE_ERROR;

	new = xmalloc(len + 1 + sizeof(struct file_entry));

	strcpy(new->pathname, name);

	new->n = parent->next ? parent->next->n + 1 : 1;
	new->next = parent->next;
	parent->next = new;

	return GE_OK;
}

static void check_file(char *path, void *data)
{
	char *suite;
	int s;

	if (FILE_IS_DEB(path)) {
		/* XXX: skip packages that are not placed in a suite-named
		 * directory */
		suite = parent_dir(path, 1);
		if (!suite)
			return;

		/* validate the suite name */
		if (get_suite_by_name(suite) == GE_ERROR) {
			free(suite);
			return;
		}

		s = new_file(path, &debs_list);
		if (s != GE_OK)
			return;

		free(suite);
	} else if (FILE_IS_DSC(path)) {
		s = new_file(path, &dscs_list);
		if (s != GE_OK)
			return;
	}
}

static int process_cache_cb(void *user, int cols, char **values, char **keys)
{
	DBG("binary pkg %s\n", values[BC_POOL_FILE]);
	pkg_append(values[BC_POOL_FILE], values[BC_SUITE], values[BC_DEB_ARCH],
			values[BC_DEB_SECTION], 0);

	return GE_OK;
}

int scan_pool(void)
{
	struct file_entry *entry;
	int s;

	traverse(G.repo_dir, check_file, NULL);

	if (!dscs_list.next)
		return GE_ERROR;

	entry = dscs_list.next;
	SAY("Processing %d source packages.\n", dscs_list.next->n);
	while (entry) {
		if (G.cleanup)
			validate_dsc(entry->pathname);
		else
			process_dsc(entry->pathname);
		entry = entry->next;
	}

	if (!debs_list.next)
		return GE_OK;

	if (G.cached) {
		SAY("Processing cached binary packages.\n");

		s = bc_search_all("", NULL, process_cache_cb);
		if (s != GE_OK)
			return GE_ERROR;
	} else {
		entry = debs_list.next;
		SAY("Processing %d binary packages.\n", debs_list.next->n);
		while (entry) {
			if (G.cleanup)
				validate_deb(entry->pathname);
			else
				process_deb(entry->pathname);
			entry = entry->next;
		}
	}

	return GE_OK;
}

int apt_ftparchive()
{
	char *argv[] = { "apt-ftparchive", "generate", G.apt_config, NULL };
	int ret;

	ret = spawn(APTFA_BIN_PATH, argv);
	if (ret)
		return GE_ERROR;

	return GE_OK;
}

