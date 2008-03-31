/*
 * vi: sw=4 ts=4 noexpandtab
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <unistd.h>
#include <stdarg.h>
#include "common.h"
#include "util.h"

/*
 * Given a path, returns a parent directory,
 * that is allocated (needs free'ing).
 */
char *parent_dir(const char *path, int tailcut)
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
int spawn(const char *cmd, char *const argv[])
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
		close(1);
		close(2);

		dup2(fileno(OUT[LOG]), 1);
		dup2(fileno(OUT[LOG]), 2);

		ret = execve(cmd, argv, environ);
		if (ret) {
			DBG("exec %s failed\n", cmd);
			exit(EXIT_FAILURE);
		}
	}

	return ret;
}

/*
 * remove a directory (with all contents)
 */
int rm_rf(char *dir)
{
	char *argv[] = { "rm", "-rf", dir, NULL };
	int ret;

	ret = spawn(RM_BIN_PATH, argv);
	if (ret)
		return GE_ERROR;

	return GE_OK;
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
int copy(const char *src, const char *dst)
{
	char *const argv[] = { "cp", (char *)src, (char *)dst, NULL };
	int ret;

	ret = spawn(CP_BIN_PATH, argv);
	if (ret)
		return GE_ERROR;

	return GE_OK;
}

/*
 * calculate md5 hash of a 'file'
 */
int md5sum(const char *file, char *buf)
{
	FILE *p;
	char cmd[PATH_MAX];

	/* TODO: access(file) */
	snprintf(cmd, PATH_MAX, "%s %s", MD5SUM_BIN_PATH, file);
	p = popen(cmd, "r");
	if (!p)
		return GE_ERROR;

	fscanf(p, "%32s", buf);
	pclose(p);

	return 0;
}

/*
 * call 'dpkg-deb' to build a binary package
 */
int dpkg_deb(char *path)
{
	char *argv[] = { "dpkg-deb", "-b", path, NULL };
	int ret;

	ret = spawn(DPKGDEB_BIN_PATH, argv);

	return ret;
}

/*
 * call 'dpkg-source' to build a source package
 */
int dpkg_source(char *dir, char *where)
{
	char *argv[] = { "dpkg-source", "-b", dir, NULL };
	char *pwd = get_current_dir_name();
	int ret;

	chdir(where);
	ret = spawn(DPKGSRC_BIN_PATH, argv);
	chdir(pwd);

	return ret;
}

size_t __vread(int is_pipe, char **out, const char *openstr)
{
	FILE *p;
	char *buf = NULL;
	size_t len = 0;
	int r = 1;

	p = is_pipe ? popen(openstr, "r") : fopen(openstr, "r");
	if (!p)
		return GE_ERROR;

	while (!feof(p) && r) {
		buf = realloc(buf, len + BUFSIZ);
		if (!buf)
			return GE_ERROR;

		len +=
		r = read(fileno(p), buf + len, BUFSIZ);
	}

	if (is_pipe)
		pclose(p);
	else
		fclose(p);

	buf[len] = '\0';
	*out = buf;

	return len;
}

size_t vread_pipe(char **out, const char *openstr)
{
	return __vread(1, out, openstr);
}

size_t read_pipe(char **out, const char *fmt, ...)
{
	va_list args;
	char *opstr;
	int s;

	va_start(args, fmt);
	s = vasprintf(&opstr, fmt, args);
	va_end(args);

	if (s == -1)
		return GE_ERROR;

	return vread_pipe(out, opstr);
}

size_t vread_file(char **out, const char *openstr)
{
	return __vread(0, out, openstr);
}

size_t read_file(char **out, const char *fmt, ...)
{
	va_list args;
	char *opstr;
	int s;

	va_start(args, fmt);
	s = vasprintf(&opstr, fmt, args);
	va_end(args);

	if (s == -1)
		return GE_ERROR;

	return vread_file(out, opstr);
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

