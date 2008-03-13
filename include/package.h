/*
 * vi: sw=4 ts=4 noexpandtab
 */
#ifndef __SLINDAK_PACKAGE_H__
#define __SLINDAK_PACKAGE_H__

int inject_deb(struct debfile *debf, char *suite, const char *path);
int process_deb(const char *path);
int process_dsc(char *path);
int validate_deb(char *path);
int validate_dsc(char *path);

#endif /* __SLINDAK_PACKAGE_H__ */

