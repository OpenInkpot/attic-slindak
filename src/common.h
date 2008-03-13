/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include "output.h"

#ifndef __COMMON_H__
#define __COMMON_H__

#include "config.h"

#ifndef HOST_OS
#define HOST_OS "linux"
#endif
#ifndef BUILD_DATE
#define BUILD_DATE "N/A"
#endif

#include "slindak.h"

int lists_cleanup();
int pkg_append(const char *path, char *suite, char *arch, char *comp,
		int src);

/* initialize/deinitialize suites
 * these functions should be a part of a bigger construction,
 * thus might be gone from here anytime soon */
int init_slind(void);
void done_slind(void);

#endif

