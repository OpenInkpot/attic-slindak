/*
 * vi: sw=4 ts=4 noexpandtab
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common.h"
#include "lua.h"
#include "configuration.h"

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

static int file_append(const char *fn, const char *text)
{
	FILE *f;
	f = fopen(fn, "a");

	GE_ERROR_IFNULL(f);
	fprintf(f, "%s\n", text);
	fclose(f);

	return GE_OK;
}

int pkg_append(const char *path, char *suite, char *arch, char *comp,
		int src)
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

