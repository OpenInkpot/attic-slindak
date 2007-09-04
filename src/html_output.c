/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "html_static.h"

#define BUFSZ 32768
static char *output;
static char *outp;

int output_printf(char *fmt, ...)
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

void html_header(char *title)
{
	printf(html_header_fmt, title);
}

void html_footer()
{
	printf(html_footer_fmt, BUILD_DATE);
}

void flush_output()
{
	puts(output);
}

void init_output()
{
	outp = output = malloc(BUFSZ);
}

char *html_table(char *class, char *content)
{
	return (class
		? sasprintf("<table class=\"%s\">%s</table>", class, content)
		: sasprintf("<table>%s</table>", content));
}

char *html_tr(char *class, char *content)
{
	return (class
		? sasprintf("<tr class=\"%s\">%s</tr>", class, content)
		: sasprintf("<tr>%s</tr>", content));
}

char *html_td(char *class, char *content)
{
	return (class
		? sasprintf("<td class=\"%s\">%s</td>", class, content)
		: sasprintf("<td>%s</td>", content));
}

char *html_anchor(char *href, char *content)
{
	return sasprintf("<a href=\"%s\">%s</a>", href, content);
}

char *html_input(char type, char *name, char *value)
{
	return sasprintf("<input type=\"%s\" name=\"%s\" value=\"%s\"/>",
			type, name, value);
}

char *html_select(char *name, char **values, char **titles, char *def, int n)
{
	char *arr[32];
	int i;

	for (i = 0; i < n; i++)
		arr[i] = sasprintf("<option value=\"%s\">%s</option>",
				values[i], titles[i]);
	sasprintf("<select name=\"%s\">");
}

