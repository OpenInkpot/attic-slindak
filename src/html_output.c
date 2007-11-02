/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "html_static.h"

#define BUFSZ (256*1024)
static char *output;
static char *outp;

int html_output_printf(char *fmt, ...)
{
	va_list ap;
	size_t len;

	if (outp - output >= BUFSZ)
		return 0;

	va_start(ap, fmt);
	len = vsnprintf(outp, BUFSZ - (outp - output), fmt, ap);
	va_end(ap);
	outp += len;

	return len;
}

void flush_html_output()
{
	puts(output);
}

void init_html_output()
{
	outp = output = malloc(BUFSZ);
}

void done_html_output()
{
	free(output);
}

