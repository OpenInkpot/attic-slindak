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
#include "lua.h"
#include "configuration.h"
#include "db.h"
#include "debfile.h"

struct suite *SUITES[MAX_SUITES];
int nsuites = 0;

int suite_add(char *name, char **arches, char **comps, int idx)
{
	if (idx == -1)
		idx = nsuites;

	if (idx >= MAX_SUITES - 1)
		return GE_ERROR;

	SUITES[idx] = malloc(sizeof(struct suite));
	if (!SUITES[idx]) {
		do {
			free(SUITES[--idx]->name);
			free(SUITES[idx]);
		} while (idx);

		return GE_ERROR;
	}

	SUITES[idx]->name = strdup(name);
	memcpy(SUITES[idx]->archlist, arches, MAX_ARCHES * sizeof(char *));
	memcpy(SUITES[idx]->complist, comps, MAX_COMPS * sizeof(char *));

	if (idx == nsuites)
		nsuites++;

	return GE_OK;
}

void suite_remove(int idx)
{
	int i;

	if (idx > MAX_SUITES || !SUITES[idx])
		return;

	for (i = 0; SUITES[idx]->archlist[i]; i++)
		free(SUITES[idx]->archlist[i]), SUITES[idx]->archlist[i] = NULL;
	for (i = 0; SUITES[idx]->complist[i]; i++)
		free(SUITES[idx]->complist[i]), SUITES[idx]->complist[i] = NULL;

	free(SUITES[idx]->name);
	free(SUITES[idx]);
}

void suite_remove_all()
{
	while (nsuites--)
		suite_remove(nsuites);
}

int lists_cleanup()
{
	int sn, an, cn, s;
	char *fn;

	SAY("Cleaning up.\n");

	/* clean all the lists */
	for (sn = 0; sn < nsuites; sn++) {
		if (G.force) {
			asprintf(&fn, "/tmp/%s.db", SUITES[sn]->name);
			unlink(fn);
			free(fn);
		}
			
		for (cn = 0; SUITES[sn]->complist[cn]; cn++) {
			char *dir;

			fn = L_call("RenderSrcListFileName",
					2, SUITES[sn]->name, SUITES[sn]->complist[cn]);

			/* make sure the directory exists */
			s = asprintf(&dir, "%s/dists/%s/%s/source",
					G.repo_dir, SUITES[sn]->name, SUITES[sn]->complist[cn]);

			if (s == -1 || !fn) {
				SHOUT("Can't allocate memory!\n");
				free(fn);
				return GE_ERROR;
			}

			mkdir_p(dir, 0755);
			free(dir);

			unlink(fn);
			creat(fn, 0644);
			free(fn);

			for (an = 0; SUITES[sn]->archlist[an]; an++) {
				fn = L_call("RenderListFileName",
						3, SUITES[sn]->name,
						SUITES[sn]->archlist[an],
						SUITES[sn]->complist[cn]);

				s = asprintf(&dir, "%s/dists/%s/%s/binary-%s",
						G.repo_dir, SUITES[sn]->name, SUITES[sn]->complist[cn],
						SUITES[sn]->archlist[an]);

				if (s == -1 || !fn) {
					SHOUT("Can't allocate memory!\n");
					free(fn);
					return GE_ERROR;
				}

				mkdir_p(dir, 0755);
				free(dir);

				unlink(fn);
				creat(fn, 0644);
				free(fn);
			}
		}
	}

	return GE_OK;
}

static int file_append(char *fn, char *text)
{
	FILE *f;
	f = fopen(fn, "a");

	GE_ERROR_IFNULL(f);
	fprintf(f, "%s\n", text);
	fclose(f);

	return GE_OK;
}

int pkg_append(char *path, char *suite, char *arch, char *comp, int src)
{
	char *fn;
	int sn, an;

	if (src) {
		fn = L_call("RenderSrcListFileName", 2, suite, comp);
		GE_ERROR_IFNULL(fn);

		file_append(fn, path);
		free(fn);
	} else {
		if (!strcmp(arch, "all")) {
			sn = get_suite_by_name(suite);
			for (an = 0; SUITES[sn]->archlist[an]; an++) {
				fn = L_call("RenderListFileName", 3, suite,
						SUITES[sn]->archlist[an], comp);
				GE_ERROR_IFNULL(fn);

				file_append(fn, path);
				free(fn);
			}
		} else {
			fn = L_call("RenderListFileName", 3, suite, arch, comp);
			GE_ERROR_IFNULL(fn);

			file_append(fn, path);
			free(fn);
		}
	}

	return GE_OK;
}

int init_slind()
{
	memset(SUITES, 0, sizeof(SUITES));

	return GE_OK;
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

