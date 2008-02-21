/*
 * vi: sw=4 ts=4 noexpandtab
 */
#ifndef __DEBFILE_H__
#define __DEBFILE_H__

#include <limits.h> /* PATH_MAX */

#define FILE_IS_DEB(path) ({    \
		char *p = path;         \
		p += strlen(path) - 4;  \
		strcmp(p, ".deb") == 0; \
	})

#define FILE_IS_DSC(path) ({    \
		char *p = path;         \
		p += strlen(path) - 4;  \
		strcmp(p, ".dsc") == 0; \
	})

#define FILE_IS_ORIG(path) ({                     \
		char *p = path;                           \
		int __r = 0;                              \
		if (strlen(path) > 12) {                  \
			p += strlen(path) - 12;               \
			__r = strcmp(p, ".orig.tar.gz") == 0; \
		}                                         \
		__r;                                      \
	})

#define DF_NAMELEN   256
#define DF_VERLEN    64
#define DF_ARCHLEN   256
#define DF_COMPLEN   64
#define DF_MD5LEN    33
#define DF_SRCLEN    DF_NAMELEN

struct pkgfile {
	char name[FILENAME_MAX];
	char md5sum[33];
	size_t size;
};

struct debfile {
	char debname[DF_NAMELEN];
	char version[DF_VERLEN];
	char arch[DF_ARCHLEN];
	char crossarch[DF_ARCHLEN];
	char component[DF_COMPLEN];
	char source[DF_SRCLEN];
	char pool_file[PATH_MAX];
	char *suite;
	off_t deb_size;
	char deb_md5[DF_MD5LEN];
	char *deb_control;
};

struct dscfile {
	char pkgname[DF_SRCLEN];
	char version[DF_VERLEN];
	char arch[DF_ARCHLEN];
	char component[DF_COMPLEN];
	struct pkgfile *files[3];
	int nfiles;
};

int deb_ver_gt(char *v1, char *v2);

int debfile_read(char *path, struct debfile *df);
void debfile_free(struct debfile *debf);
int dscfile_read(char *path, struct dscfile *df);

#endif

