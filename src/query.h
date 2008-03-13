/*
 * vi: sw=4 ts=4 noexpandtab
 */
#ifndef __QUERY_H__
#define __QUERY_H__

int query_pkglist(const char *component, char *suite,
		char *arch, const char *fmt);
int query_deblist(int exists, char *suite,
		char *arch, const char *fmt);
int query_pkginfo(const char *pkgname, char *suite,
		char *arch, const char *fmt);

#endif /* __QUERY_H__ */

