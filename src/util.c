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
#include "util.h"

/*
 * Given a path, returns a parent directory,
 * that is allocated (needs free'ing).
 */
char *parent_dir(char *path, int tailcut)
{
	char *s1, *s2 = NULL;
	char *res = NULL;

	s1 = strrchr(path, '/');
	if (s1) {
		*s1 = '\0';

		if (tailcut) {
			s2 = strrchr(path, '/');
			if (s2++)
				res = strdup(s2);
		} else
			res = strdup(path);

		*s1 = '/';
	}

	return res;
}

int traverse(char *path, traverse_fn_t callback, void *data)
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
			traverse(newpath, callback, data);
		else
			callback(newpath, data);

		free(newpath);
	} while (de);
	closedir(dir);

	return GE_OK;
}

extern char **environ;
extern char *program_invocation_name;

/*
 * execute a thing (fork, exec, wait)
 * (from grasp, src/system.c)
 */
int spawn(char *cmd, char **argv)
{
	pid_t pid;
	int i = 0;
	int ret;

	if (verbosity >= VERB_DEBUG) {
		DBG("going to execute:");
		while (argv[i])
			output(ERR, VERB_DEBUG, " %s", argv[i++]);
		output(ERR, VERB_DEBUG, "\n");
	}

	pid = fork();
	if (pid)
		waitpid(-1, &ret, 0);
	else {
		ret = execve(cmd, argv, environ);
		if (ret) {
			DBG("exec %s failed\n", cmd);
			exit(EXIT_FAILURE);
		}
	}

	return ret;
}

/*
 * create a directory (with all the missing parent directories)
 */
int mkdir_p(char *dst, mode_t mode)
{
	char *argv[] = { "mkdir", "-p", dst, NULL };
	int ret;

	ret = spawn(MKDIR_BIN_PATH, argv);
	if (ret)
		return GE_ERROR;

	ret = chmod(dst, mode);
	if (ret)
		return GE_ERROR;

	return GE_OK;
}

/*
 * copy something from 'src' to 'dst'
 */
int copy(char *src, char *dst)
{
	char *argv[] = { "cp", src, dst, NULL };
	int ret;

	ret = spawn(CP_BIN_PATH, argv);
	if (ret)
		return GE_ERROR;

	return GE_OK;
}

void root_squash()
{
	uid_t uid = geteuid();

	if (uid == 0) {
		SHOUT("Under no circumstances is %s going to work "
				"with effective uid of root.\n", program_invocation_name);
		exit(EXIT_FAILURE);
	}
}

