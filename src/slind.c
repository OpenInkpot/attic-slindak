/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include "common.h"
#include "db.h"
#include "debfile.h"

struct suite {
	char *name;
	char *archlist;
	char *complist;
};

struct suite *SUITES[MAX_SUITES];
int nsuites = 0;

int suite_add(char *name, char *arches, char *comps)
{
	if (nsuites >= MAX_SUITES - 1)
		return GE_ERROR;

	SUITES[nsuites] = malloc(sizeof(struct suite));
	if (!SUITES[nsuites])
		return GE_ERROR;

	printf("### %s: %s [%s]\n", name, arches, comps);
	SUITES[nsuites]->name = strdup(name);
	SUITES[nsuites]->archlist = strdup(arches);
	SUITES[nsuites]->complist = strdup(comps);
	nsuites++;

	return GE_OK;
}

void suite_remove_all()
{
	while (nsuites--) {
		free(SUITES[nsuites]->name);
		free(SUITES[nsuites]->archlist);
		free(SUITES[nsuites]->complist);
		free(SUITES[nsuites]);
	}
}

struct list_file *lists[MAX_SUITES * 2];
int nlists = 0;

static int listfile_open()
{
	if (!lists[nlists])
		return GE_ERROR;

	lists[nlists]->file = fopen(lists[nlists]->name, "w");
	if (!lists[nlists]->file) {
		fprintf(stderr, "Failed to open %s\n", lists[nlists]->name);
		exit(EXIT_FAILURE); /* XXX */
	}

	return GE_OK;
}

int init_slind()
{
	int i;

	for (i = 0; i < NSUITES; i++) {
		lists[nlists] = malloc(sizeof(struct list_file));
		snprintf(lists[nlists]->name, PATH_MAX, "%s/indices/%s.list",
				repo_dir, suites[i]);

		listfile_open();

		lists[++nlists] = malloc(sizeof(struct list_file));

		snprintf(lists[nlists]->name, PATH_MAX, "%s/indices/%s.src.list",
				repo_dir, suites[i]);

		listfile_open();

		nlists++;
	}
}

void done_slind()
{
	int i;

	suite_remove_all();
	for (i = 0; i < nlists; i++) {
		fclose(lists[i]->file);
		free(lists[i]);
	}
}

int get_suite_by_name(char *suite)
{
	int i;

	for (i = 0; i < NSUITES; i++)
		if (suites[i] && !strcmp(suite, suites[i]))
			return i << 1;

	return GE_ERROR;
}

void list_append(int sn, char *debfile)
{
	fprintf(lists[sn]->file, "%s\n", debfile);
	fflush(lists[sn]->file);
}

