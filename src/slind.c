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

int pkg_append(char *path, char *suite, char *arch, char *comp, int src)
{
	FILE *f;
	char *fn;

	fn = L_call(src
			? "RenderSrcListFileName"
			: "RenderListFileName", 3, suite, arch, comp);
	GE_ERROR_IFNULL(fn);

	f = fopen(fn, "a");
	GE_ERROR_IFNULL(f);
	fprintf(f, "%s\n", path);
	fclose(f);

	return GE_OK;
}

int init_slind()
{
}

void done_slind()
{
	suite_remove_all();
}

int get_suite_by_name(char *suite)
{
	int i;

	for (i = 0; i < nsuites; i++)
		if (SUITES[i] && !strcmp(suite, SUITES[i]->name))
			return i;

	return GE_ERROR;
}

