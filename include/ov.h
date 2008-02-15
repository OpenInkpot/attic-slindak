/*
 * vi: sw=4 ts=4 noexpandtab
 */
#ifndef __SLINDAK_OV_H__
#define __SLINDAK_OV_H__

typedef int (*ov_callback_fn)(void *, int, char **, char **);

#define OV_PKGNAME   0
#define OV_VERSION   1
#define OV_SUITE     2
#define OV_ARCH      3
#define OV_COMPONENT 4
#define OV_NCOLS     5
#define OV_FIRSTCOL  (OV_PKGNAME)

static char *ov_columns[OV_NCOLS] = {
	"pkgname",
	"version",
	"suite",
	"arch",
	"component"
};

#define OV_COLS "pkgname, version, suite, arch, component"

int ov_version_count(char *pkgname, char *suite, int *count);
int ov_find_same_uver(char *pkgname, char *uver);
int ov_search_all(char *where, void *user, ov_callback_fn callback);
int ov_update_all(char *pkgname, char *arch, char *suite, char *version,
		char *set_version, char *set_suite, char *set_component,
		char *set_arch);
int ov_insert(char *pkgname, char *version, char *arch,
		char *suite, char *component);
int ov_delete(char *pkgname, char *version, char *suite, char *arch);
int ov_create_table();

#endif /* __SLINDAK_OV_H__ */

