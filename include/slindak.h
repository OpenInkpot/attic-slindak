/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include "output.h"

#ifndef __SLINDAK_H__
#define __SLINDAK_H__

#include <sys/types.h>

#define MAX_SUITES 8
#define MAX_ARCHES 32
#define MAX_COMPS  32

struct suite {
	char *name;
	char *archlist[MAX_ARCHES];
	char *complist[MAX_COMPS];
};

extern struct suite *SUITES[MAX_SUITES];
extern int nsuites;

#define GE_OK    (0)
#define GE_ERROR (-1)
#define GE_EMPTY (-2)

#define GE_ERROR_IFNULL(x) do { if (!(x)) return GE_ERROR; } while (0)

/* util.h */
char *parent_dir(char *path, int tailcut);

typedef void (*traverse_fn_t)(char *path, void *data);

int traverse(char *path, traverse_fn_t callback, void *data);

int spawn(char *cmd, char **argv);

int rm_rf(char *dir);

int mkdir_p(char *dst, mode_t mode);

int copy(char *src, char *dst);

int md5sum(char *file, char *buf);

int dpkg_deb(char *path);

int dpkg_source(char *dir, char *where);

size_t vread_pipe(char **out, const char *openstr);

size_t read_pipe(char **out, const char *fmt, ...);

void root_squash();

/* library */
typedef void (*exit_fn_t)(void);

int push_cleaner(exit_fn_t exit_fn);
int pop_cleaner(exit_fn_t exit_fn);

void libslindak_lock(void);
void libslindak_unlock(void);

#endif /* __SLINDAK_H__ */

