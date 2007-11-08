/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <libcgi/cgi.h>
#include "slindak.h"
#include "output.h"
#include "lua-helpers.h"
#include "html_static.h"
#include "ov.h"

static int ov_fetch_html(void *user, int cols, char **values, char **keys)
{
	char *html;
	int i;

	html = L_call("HtmlListTableRow", 5, Q(values[0]), Q(values[1]),
			Q(values[2]), values[3], Q(values[4]));
	DBG("%s", html);
	html_output_printf(html);

	free(html);

	return GE_OK;
}

void action_list()
{
	char *html;
	char *order = cgi_param("order");
	char *pkg = cgi_param("pkg");
	char *where;
	int s;

	if (!order)
		order = "pkgname";

	if (pkg)
		asprintf(&where, " WHERE pkgname='%s' ORDER BY %s ASC", pkg, order);
	else
		asprintf(&where, " ORDER BY %s ASC", order);

	html = L_call("HtmlListTableHeader", 0);
	html_output_printf(html);
	free(html);

	s = ov_search_all(where, NULL, ov_fetch_html);
	/*if (s != GE_OK)
		cgi_redirect("?act=bail");*/

	html = L_call("HtmlListTableFooter", 0);
	html_output_printf(html);

	free(where);

	free(html);
}

void action_edit()
{
	char *html, *comp;
	char *pkg = cgi_param("pkg");
	char *ver = cgi_param("ver");
	char *suite = cgi_param("suite");
	char *arch = cgi_param("arch");
	int s;

	if (!arch)
		arch = "";

	s = ov_find_component(pkg, ver, arch, suite, &comp);
	if (s != GE_OK)
		return;

	DBG("%s=%s, suite=%s, comp=%s\n", pkg, ver, suite, comp);
	html = L_call("HtmlEditPage", 5, pkg, ver, suite, comp, arch);
	html_output_printf(html);

	free(html);
	free(suite);
}

void action_new()
{
	char *html;
	int s;

	html = L_call("HtmlNewPage", 0);
	html_output_printf(html);

	free(html);
}

void action_commit()
{
	char *pkgname = cgi_param("pkg");
	char *version = cgi_param("ver");
	char *arch    = cgi_param("arch");
	char *suite   = cgi_param("suite");
	char *set_version   = cgi_param("set_version");
	char *set_suite     = cgi_param("set_suite");
	char *set_component = cgi_param("set_component");
	char *set_arch      = cgi_param("set_arch");
	int ret;

	if (!pkgname) {
		html_output_printf("<h1>Invalid parameters</h1>");
		return;
	}

	if (set_arch && !strcmp(set_arch, "ALL"))
		set_arch[0] = '\0';

	if (version) {
		DBG("UPDATE\n");
		/*req = sqlite3_mprintf("UPDATE overrides SET"
				" version='%s', suite='%s', component='%s', arch='%s'"
				" WHERE pkgname='%s' AND version='%s'",
				set_version, set_suite, set_component, set_arch,
				pkgname, version);*/
		ret = ov_update_all(pkgname, arch, suite, version,
				set_version, set_suite, set_component, set_arch);
	} else {
		DBG("INSERT\n");
		/*req = sqlite3_mprintf("INSERT INTO overrides VALUES"
				"('%s', '%s', '%s', '%s', '%s')", pkgname,
				set_version, set_suite, set_component, set_arch);*/
		ret = ov_insert(pkgname, set_version, set_arch, set_suite,
				set_component);
	}

	if (ret != GE_OK) {
		html_output_printf("<h1>An error occured while updating the table</h1>");
	} else
		action_list();
}

void action_del()
{
	char *pkgname = cgi_param("pkg");
	char *version = cgi_param("ver");
	char *arch    = cgi_param("arch");
	char *suite   = cgi_param("suite");
	int s;

	s = ov_delete(pkgname, version, suite, arch);
	if (s != GE_OK) {
		html_output_printf("<h1>An error occured while deleting %s</h1>",
				pkgname);
	} else
		action_list();
}

