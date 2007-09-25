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

#define MAX_SUITES 8
#define MAX_ARCHES 32
#define MAX_COMPS  32

struct suite {
	char *name;
	char *archlist[MAX_ARCHES];
	char *complist[MAX_COMPS];
};

extern struct suite *SUITES[MAX_SUITES];
extern int nsuites;

#define GE_OK    (0)
#define GE_ERROR (-1)
#define GE_EMPTY (-2)

#define GE_ERROR_IFNULL(x) do { if (!(x)) return GE_ERROR; } while (0)

int lists_cleanup();

#endif

