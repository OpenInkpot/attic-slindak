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

#endif

