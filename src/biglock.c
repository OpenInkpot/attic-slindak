/*
 * vi: sw=4 ts=4 noexpandtab
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>
#include "common.h"

static int fd = -1;

/*
 * create and lock a lockfile in a given directory
 */
int bl_take(const char *lockdir)
{
	struct flock fl = {
		.l_type   = F_WRLCK,
		.l_whence = SEEK_SET,
		.l_start  = 0,
		.l_len    = 0,
	};
	char *fn;
	int s;

	s = asprintf(&fn, "%s/lock", lockdir);
	if (s == -1) {
		SHOUT("Out of memory\n");
		return GE_ERROR;
	}

	fd = open(fn, O_RDWR|O_CREAT|O_TRUNC, 0660);
	if (fd == -1) {
		SHOUT("Can't open lockfile.\n");
		s = GE_ERROR;
		goto out_cold;
	}

acquire:
	/* sleep on this lock if already taken */
	s = fcntl(fd, F_SETLKW, &fl);
	if (s == -1) {
		if (errno == EACCES || errno == EAGAIN) {
			/* this shouldn't normally happen with F_SETLKW */
			DBG("Hmm-hmm.\n");
			goto acquire;
		} else if (errno == EINTR) /* some signal arrived */
			goto acquire;

		SHOUT("Can't acquire %s lock: %d\n", fn, errno);
		s = GE_ERROR;
		goto out_cold;
	}

	s = GE_OK;

out_cold:
	free(fn);

	return s;
}

/*
 * release previously taken lock
 */
int bl_release(void)
{
	struct flock fl = {
		.l_type   = F_UNLCK,
		.l_whence = SEEK_SET,
		.l_start  = 0,
		.l_len    = 0,
	};
	int s;

	/* no lockfile opened */
	if (fd == -1)
		return GE_ERROR;

	s = fcntl(fd, F_SETLK, &fl);
	if (s == -1) {
		SHOUT("Can't unlock: %d\n", errno);
		return GE_ERROR;
	}

	fd = -1;

	return GE_OK;
}

