/*
 * vi: sw=4 ts=4 noexpandtab
 */
#ifndef __HTML_STATIC__
#define __HTML_STATIC__

#define SV(x, s) (strlen(x) ? (x) : s)
#define Q(x) (SV(x ? x : "", "&nbsp;"))

static inline char *sasprintf(char *fmt, ...)
{
	va_list ap;
	char *__str;
	int __r;

	va_start(ap, fmt);
	__r = vasprintf(&__str, fmt, ap);
	va_end(ap);
	return (__r == -1 ? NULL : __str);
}

#define O(fmt, args ...) (html_output_printf(fmt, ## args))
int html_output_puts(char *str);
int html_output_printf(char *fmt, ...);
void flush_html_output(void);
void init_html_output(void);
void done_html_output(void);

#endif

