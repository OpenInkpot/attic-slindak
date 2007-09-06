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

/*
 * Given a path, returns a parent directory,
 * that is allocated (needs free'ing).
 */
char *parent_dir(char *path)
{
	char *s1, *s2 = NULL;
	char *res = NULL;

	s1 = strrchr(path, '/');
	if (s1) {
		*s1 = '\0';

		s2 = strrchr(path, '/');
		if (s2++)
			res = strdup(s2);

		*s1 = '/';
	}

	return res;
}
		
