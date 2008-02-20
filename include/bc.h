/*
 * vi: sw=4 ts=4 noexpandtab
 */
#ifndef __SLINDAK_BC_H__
#define __SLINDAK_BC_H__

typedef int (*bc_callback_fn)(void *, int, char **, char **);

#define BC_PKGNAME     0
#define BC_VERSION     1
#define BC_SUITE       2
#define BC_POOL_FILE   3
#define BC_DEB_NAME    4
#define BC_DEB_SECTION 5
#define BC_DEB_ARCH    6
#define BC_DEB_SIZE    7
#define BC_DEB_MD5     8
#define BC_DEB_CONTROL 9
#define BC_NCOLS       10
#define BC_FIRSTCOL  (BC_PKGNAME)

static char *bc_columns[BC_NCOLS] = {
	"pkgname",
	"version",
	"suite",
	"pool_file",
	"deb_name",
	"deb_section",
	"deb_arch",
	"deb_size",
	"deb_md5",
	"deb_control"
};

#include "debfile.h" /* XXX: debfile.h is not visible and exported */

#define BC_COLS "pkgname, version, suite, pool_file, deb_name, " \
				"deb_section, deb_arch, deb_size, deb_md5, deb_control"

int bc_create_table();
int bc_insert_debf(struct debfile *debf);

#endif /* __SLINDAK_BC_H__ */

