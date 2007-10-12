/*
 * vi: sw=4 ts=4 noexpandtab
 */

/*
 * Mass operation on all recognizable objects in a pool
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "common.h"
#include "configuration.h"
#include "debfile.h"
#include "util.h"

char *mk_pool_path(char *comp, char *pkgname, char *suite)
{
	char *newpath = NULL;
	int s;

	if (!strncmp(pkgname, "lib", 3))
		s = asprintf(&newpath, "%s/%s/lib%c/%s/%s", G.pool_dir, comp,
				pkgname[3], pkgname, suite);
	else
		s = asprintf(&newpath, "%s/%s/%c/%s/%s", G.pool_dir, comp,
				pkgname[0], pkgname, suite);

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

int scan_pool()
{
	struct file_entry *entry;

	traverse(G.repo_dir, check_file, NULL);

	if (!dscs_list.next)
		return GE_ERROR;

	entry = dscs_list.next;
	SAY("Processing %d source packages.\n", dscs_list.next->n);
	while (entry) {
		process_dsc(entry->pathname);
		entry = entry->next;
	}

	if (!debs_list.next)
		return GE_OK;

	entry = debs_list.next;
	SAY("Processing %d binary packages.\n", debs_list.next->n);
	while (entry) {
		process_deb(entry->pathname);
		entry = entry->next;
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

