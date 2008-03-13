/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include "common.h"

void version(void)
{
	SAY(
		"%s %s for %s compiled on %s\n",
		PACKAGE_NAME, VERSION, HOST_OS, BUILD_DATE
	   );
}

void help(void)
{
	SAY(
		"%s is a collection of tools targeted at maintenance of slind\n"
		"(http://www.slind.org/) package repository and (potentially)\n"
		"any other distribution's package repository.\n\n", PACKAGE_NAME
	   );
}

