/*
 * vi: sw=4 ts=4 noexpandtab
 */
#ifndef __DEBFILE_H__
#define __DEBFILE_H__

#define DF_NAMELEN   256
#define DF_VERLEN    64
#define DF_ARCHLEN   32
#define DF_COMPLEN   64
#define DF_SRCLEN    DF_NAMELEN

struct debfile {
	char debname[DF_NAMELEN];
	char version[DF_VERLEN];
	char arch[DF_ARCHLEN];
	char component[DF_COMPLEN];
	char source[DF_SRCLEN];
};

struct dscfile {
	char pkgname[DF_SRCLEN];
	char version[DF_VERLEN];
	char arch[DF_ARCHLEN];
	char component[DF_COMPLEN];
};


#endif

