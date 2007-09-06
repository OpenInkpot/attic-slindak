/*
 * vi: sw=4 ts=4 noexpandtab
 */

#ifndef __UTIL_H__
#define __UTIL_H__

char *parent_dir(char *path);

typedef void (*traverse_fn_t)(char *path, void *data);

int traverse(char *path, traverse_fn_t callback, void *data);

#endif

