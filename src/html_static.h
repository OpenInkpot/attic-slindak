/*
 * vi: sw=4 ts=4 noexpandtab
 */
#ifndef __HTML_STATIC__
#define __HTML_STATIC__

#define html_header_fmt "<html><head>"  \
	"<title>SlindWeb -- %s</title>" \
	"<link rel=\"stylesheet\" type=\"text/css\" href=\"/gitweb.css\"/>" \
	"</head><body>" \
	"<table><tr>" \
	"<td><a class=\"title\" href=\"?act=list\">All records</td>" \
	"<td><a class=\"title\" href=\"?act=new\">Add record</td>" \
	"</tr></table>"

#define html_footer_fmt "<p>%s</p></body></html>"

#define SV(x, s) (strlen(x) ? x : s)
#define Q(x) (SV(x, "&nbsp;"))

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

#define O(fmt, args ...) (output_printf(fmt, ## args))
int output_printf(char *fmt, ...);
void flush_output();
void init_output();
char *html_table(char *class, char *content);
char *html_tr(char *class, char *content);
char *html_td(char *class, char *content);
char *html_anchor(char *href, char *content);
char *html_input(char type, char *name, char *value);

#endif

