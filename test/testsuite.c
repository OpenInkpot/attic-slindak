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
#include "slindak.h"

void do_pkg_tests();

int main()
{
	OUT[LOG] = fopen("/dev/null", "w");

	printf("Slindak test suite started.\n");
	do_pkg_tests();
	printf("Slindak test suite finished.\n");

	return 0;
}

