/*
 * vi: sw=4 ts=4 noexpandtab
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include "slindak.h"

#define mkpdir(p) do {                          \
		char *__p = parent_dir(p, 0);           \
		if (__p) mkdir_p(__p, 0755), free(__p); \
		else return GE_ERROR;                   \
	} while (0);

#define xmalloc(__s) ({                    \
                void *__ret = malloc(__s); \
                if (!__ret)                \
                        return GE_ERROR;   \
                __ret;                     \
        })

#define xfree(__p) do {                      \
                void **__P = (void *)&(__p); \
                if (*__P)                    \
                        free(*__P);          \
                *__P = NULL;                 \
        } while (0)

#endif

