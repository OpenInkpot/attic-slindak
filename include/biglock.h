/*
 * vi: sw=4 ts=4 noexpandtab
 */
#ifndef __BIGLOCK_H__
#define __BIGLOCK_H__

int bl_take(const char *lockdir);

int bl_release();

#endif /* __BIGLOCK_H__ */

