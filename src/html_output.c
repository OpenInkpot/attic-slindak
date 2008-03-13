/*
 * vi: sw=4 ts=4 noexpandtab
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "html_static.h"

#define BUFSZ (256*1024)
static char *output;
static char *outp;

int html_output_puts(char *str)
{
	size_t len = strlen(str);

	if (outp - output >= BUFSZ)
		return 0;

	memcpy(outp, str, len);
	outp += len;

	return len;
}

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

void flush_html_output(void)
{
	puts(output);
}

void init_html_output(void)
{
	outp = output = malloc(BUFSZ);
}

void done_html_output(void)
{
	free(output);
}

