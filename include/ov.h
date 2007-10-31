/*
 * vi: sw=4 ts=4 noexpandtab
 */
#ifndef __SLINDAK_OV_H__
#define __SLINDAK_OV_H__

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

#endif /* __SLINDAK_OV_H__ */

