/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include "common.h"
#include "db.h"
#include "debfile.h"

char repo_dir[PATH_MAX];
char pool_dir[PATH_MAX];
char odb_path[PATH_MAX];

void init_output()
{
}

void check_file(char *path)
{
	char *p = path;
	char *s1, *s2 = NULL;
	char suite[64];
	struct debfile debf;
	struct dscfile dscf;
	char *c;
	int s;

	/* XXX: consider a simple '.deb' check sufficient? */
	p += strlen(path) - 4;
	if (!strcmp(p, ".deb")) {
		debfile_read(path, &debf);

		s1 = strrchr(path, '/');
		if (s1) {
			*s1 = '\0';
			s2 = strrchr(path, '/');
			*s1 = '/';
			s2++;

			if (s2) {
				strncpy(suite, s2, s1 - s2);
				suite[s1-s2] = '\0';
				printf("suite: %s\n", suite);
			}
		}
		
		/* XXX: skip packages that are not placed in a suite-named
		 * directory */
		if (!s2)
			return;

		/* validate the suite name */
		if (get_suite_by_name(suite) == GE_ERROR)
			return;

		s = ov_find_component(debf.source, debf.version, debf.arch,
				suite, &c);
		if (s == GE_OK) {
			printf("%s=%s: dists/clydesdale/%s/binary-%s/Packages\n",
					debf.debname, debf.version,
					debf.component, debf.arch);
			pkg_append(path, suite, debf.arch, debf.component, 0);
		/*printf(" * %s (%s): %s %s %s/%s\n", debf.debname, debf.source,
				debf.version, debf.arch, debf.component, c);*/
			free(c);
		}
	} else if (!strcmp(p, ".dsc")) {
		dscfile_read(path, &dscf);

		s = ov_find_component(dscf.pkgname, dscf.version, dscf.arch,
				"clydesdale", &c);
		if (s == GE_OK) {
			printf("%s=%s: dists/clydesdale/%s/source/Sources\n",
					dscf.pkgname, dscf.version,
					dscf.component);
			pkg_append(path, "clydesdale", dscf.arch, dscf.component, 1);
		/*printf(" = %s: %s %s %s/%s\n", dscf.pkgname,
				dscf.version, dscf.arch, dscf.component, c);*/
			free(c);
		}
	}
}

int traverse(char *path)
{
	DIR *dir;
	struct dirent *de;
	struct stat st;
	char *newpath;
	int s;

	dir = opendir(path);
	GE_ERROR_IFNULL(dir);

	do {
		de = readdir(dir);
		if (!de)
			break;

		if ( 
			de->d_name[0] == '.' &&
			((de->d_name[1] == '.' && de->d_name[2] == '\0') ||
			(de->d_name[1] == '\0'))
		   )
			continue;

		asprintf(&newpath, "%s/%s", path, de->d_name);

		s = stat(newpath, &st);
		if (s) {
			free(newpath);
			continue;
		}

		if (S_ISDIR(st.st_mode))
			traverse(newpath);
		else
			check_file(newpath);

		free(newpath);
	} while (de);
	closedir(dir);

	return GE_OK;
}

int main(int argc, char **argv)
{
	int s;
	char *c;
	
	if (argc != 2) {
		fprintf(stderr, "Gimme a repo, ye wee cunt!\n");
		exit(EXIT_FAILURE);
	}

	strcpy(repo_dir, argv[1]);
	snprintf(pool_dir, PATH_MAX, "%s/pool", repo_dir);
	snprintf(odb_path, PATH_MAX, "%s/indices/overrides.db", repo_dir);

	init_output();
	init_slind();
	L_init();

	s = db_init();
	if (s != GE_OK) {
		fprintf(stderr, "Can't open database\n");
		exit(EXIT_FAILURE);
	}

	traverse(repo_dir);

	db_done();
	done_slind();
	L_done();

	return 0;
}

