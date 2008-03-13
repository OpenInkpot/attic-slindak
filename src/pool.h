/*
 * vi: sw=4 ts=4 noexpandtab
 */

#ifndef __POOL_H__
#define __POOL_H__

char *mk_pool_path(char *comp, char *pkgname, char *suite);
int scan_pool(void);
int apt_ftparchive(void);

#endif

