/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include "common.h"

FILE *OUT[OUTFILES_MAX];
int verbosity;

void output_init()
{
	/* initialize output */
	verbosity = VERB_NORMAL;
	OUT[STD] = stdout;
	OUT[ERR] = stderr;
	OUT[LOG] = stdout; /* XXX */
}

