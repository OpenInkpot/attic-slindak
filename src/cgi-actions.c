/*
 * vi: sw=4 ts=4 noexpandtab
 */
#define _GNU_SOURCE
#define __IN_ACTION__
#include <stdlib.h>
#include <stdio.h>
#include <libcgi/cgi.h>
#include <string.h>
#include "slindak.h"
#include "configuration.h"
#include "output.h"
#include "lua-helpers.h"
#include "html_static.h"
#include "db.h"
#include "debfile.h"
#include "cgi-actions.h"

/* a quick shut for cgi_redirect */
#define CGI_redirect(__u) do { cgi_redirect(__u); return; } while (0);

/* interface function for lua */
int extl_cgi_escape_special_chars(lua_State *L)
{
	char *str, *newstr, *d, *s;
	int argc = lua_gettop(L);

	if (argc != 1) {
		SHOUT("%s requires 1 argument\n", __FUNCTION__);
		return 0;
	}

	str = (char *)lua_tostring(L, -1);
	lua_pop(L, 1);

	newstr = cgi_escape_special_chars(str);
	str = malloc(strlen(newstr) * 2);
	if (!str) {
		SHOUT("%s couldn't allocate memory\n", __FUNCTION__);
		return 0;
	}

	for (d = str, s = newstr; *s; *d++ = *s++)
		if (*s == '%') *d++ = '%';
	*d = '\0';

	lua_pushlstring(L, str, strlen(str));
	//SHOUT("str=%s newstr=%s\n", str, newstr);
	free(newstr);
	free(str);

	return 1;
}

static int ov_fetch_html(void *user, int cols, char **values, char **keys)
{
	char *html;

	html = L_call("HtmlListTableRow", 5, Q(values[0]), Q(values[1]),
			Q(values[2]), values[3], Q(values[4]));
	DBG("%s", html);
	html_output_puts(html);

	free(html);

	return GE_OK;
}

#define CHECK_PKG(__pkg)                \
	valid_string(__pkg, DF_NAMELEN, 1)
#define CHECK_VER(__ver)                \
	valid_string(__ver, DF_VERLEN, 1)
#define CHECK_SUITE(__s)                \
	valid_string(__s, 16, 1)
#define CHECK_ARCH(__a,__sn)            \
	(grep_string(__a, SUITES[__sn]->archlist, 0) != -1)
#define CHECK_COMP(__c,__sn)            \
	(grep_string(__c, SUITES[__sn]->complist, 0) != -1)
#define CHECK_OVCOL(__o)               \
	(grep_string(__o, ov_columns, OV_NCOLS) != -1)

void action_list(void)
{
	char *html;
	char *order = cgi_param("order");
	char *pkg = cgi_param("pkg");
	char *where;
	int s;

	if (!order || !CHECK_OVCOL(order))
		order = "pkgname";

	if (pkg && CHECK_PKG(pkg))
		asprintf(&where, " WHERE pkgname='%s' ORDER BY %s ASC", pkg, order);
	else
		asprintf(&where, " ORDER BY %s ASC", order);

	html = L_call("HtmlListTableHeader", 0);
	html_output_puts(html);
	free(html);

	s = ov_search_all(where, NULL, ov_fetch_html);
	/*if (s != GE_OK)
		CGI_redirect("?act=bail");*/

	html = L_call("HtmlListTableFooter", 0);
	html_output_puts(html);

	free(where);

	free(html);
}

void action_edit(void)
{
	char *html, *comp;
	char *pkg = cgi_param("pkg");
	char *ver = cgi_param("ver");
	char *suite = cgi_param("suite");
	char *arch = cgi_param("arch");
	int s, sn;

	/* parameter validation */
	if (!CHECK_PKG(pkg)) CGI_redirect("?act=bail&e=inval");
	if (!CHECK_SUITE(suite)) suite = G.devel_suite;
	sn = get_suite_by_name(suite);
	if (sn == GE_ERROR) {
		suite = G.devel_suite;
		sn = get_suite_by_name(suite);
	}
	if (!arch || !CHECK_ARCH(arch, sn)) arch = "";

	s = ov_find_component(pkg, ver, arch, suite, &comp);
	if (s != GE_OK)
		return;

	DBG("%s=%s, suite=%s, comp=%s\n", pkg, ver, suite, comp);
	html = L_call("HtmlEditPage", 5, pkg, ver, suite, comp, arch);
	html_output_puts(html);

	free(html);
	free(suite);
}

void action_new(void)
{
	char *html;

	html = L_call("HtmlNewPage", 0);
	html_output_puts(html);

	free(html);
}

void action_commit(void)
{
	char *pkgname = cgi_param("pkg");
	char *version = cgi_param("ver");
	char *arch    = cgi_param("arch");
	char *suite   = cgi_param("suite");
	char *set_version = cgi_param("set_version");
	char *set_suite   = cgi_param("set_suite");
	char *set_comp    = cgi_param("set_component");
	char *set_arch    = cgi_param("set_arch");
	int ret, sn;

	/* parameter validation */
	if (!CHECK_PKG(pkgname)) CGI_redirect("?act=bail&e=inval");

	if (set_arch && !strcmp(set_arch, "ALL"))
		set_arch[0] = '\0';

	if (!CHECK_SUITE(set_suite)) CGI_redirect("?act=bail&e=inval");
	sn = get_suite_by_name(set_suite);
	if (sn == GE_ERROR) CGI_redirect("?act=bail&e=inval");
	if (!set_arch || !CHECK_ARCH(set_arch, sn)) set_arch = "";
	if (!set_comp || !CHECK_COMP(set_comp, sn))
		CGI_redirect("?act=bail&e=inval");

	if (version) {
		if (!CHECK_SUITE(suite)) CGI_redirect("?act=bail&e=inval");
		sn = get_suite_by_name(suite);
		if (sn == GE_ERROR) CGI_redirect("?act=bail&e=inval");
		if (!arch || !CHECK_ARCH(arch, sn)) arch = "";

		DBG("UPDATE\n");
		ret = ov_update_all(pkgname, arch, suite, version,
				set_version, set_suite, set_comp, set_arch);
	} else {
		DBG("INSERT\n");
		ret = ov_insert(pkgname, set_version, set_arch, set_suite,
				set_comp);
	}

	if (ret != GE_OK) {
		html_output_puts("<h1>An error occured while updating the table</h1>");
	} else
		CGI_redirect("?act=list");
}

void action_del(void)
{
	char *pkgname = cgi_param("pkg");
	char *version = cgi_param("ver");
	char *arch    = cgi_param("arch");
	char *suite   = cgi_param("suite");
	int s, sn;

	/* parameter validation */
	if (!CHECK_PKG(pkgname)) CGI_redirect("?act=bail&e=inval");
	if (!CHECK_SUITE(suite)) suite = G.devel_suite;
	sn = get_suite_by_name(suite);
	if (sn == GE_ERROR) {
		suite = G.devel_suite;
		sn = get_suite_by_name(suite);
	}
	if (!arch || !CHECK_ARCH(arch, sn)) arch = "";

	s = ov_delete(pkgname, version, suite, arch);
	if (s != GE_OK) {
		html_output_printf("<h1>An error occured while deleting %s</h1>",
				pkgname);
	} else
		action_list();
}

static int bc_fetch_html(void *user, int cols, char **values, char **keys)
{
	char *html;
	char *arch = (char *)user;

	html = L_call("HtmlDebListTablePkg", 4, Q(values[0]), Q(values[1]),
			Q(values[2]), Q(arch));
	/*DBG("%s", html);
	html_output_puts(html);*/

	free(html);

	return GE_OK;
}

void action_deblist(void)
{
	char *html;
	char *suite = cgi_param("suite");
	int s, sn, an;

	if (!CHECK_SUITE(suite))
		suite = G.devel_suite;

	sn = get_suite_by_name(suite);
	if (sn == GE_ERROR) {
		suite = G.devel_suite;
		sn = get_suite_by_name(suite);
	}

	html = L_call("HtmlDebListTableHeader", 1, suite);
	html_output_puts(html);
	free(html);

	for (an = 0; SUITES[sn]->archlist[an]; an++)
		s = bcov_search_all(suite, SUITES[sn]->archlist[an], 1,
				SUITES[sn]->archlist[an], bc_fetch_html);
	html = L_call("HtmlDebListTableBody", 1, suite);

	html_output_puts(html);
	free(html);

	html = L_call("HtmlDebListTableFooter", 1, suite);
	html_output_puts(html);

	free(html);
}

static int bc_fetchdeb_html(void *user, int cols, char **values, char **keys)
{
	char *html;

	html = L_call("HtmlDebSrcListItem", 6, Q(values[4]), Q(values[1]),
			Q(values[3]), Q(values[5]), Q(values[7]), Q(values[8]));
	/*DBG("%s", html);*/
	html_output_puts(html);

	free(html);

	return GE_OK;
}

void action_debsrcview(void)
{
	char *html;
	char *pkg = cgi_param("pkg");
	char *arch = cgi_param("arch");
	char *suite = cgi_param("suite");
	char *ver, *where;
	int s, sn;

	if (!CHECK_PKG(pkg)) CGI_redirect("?act=bail&e=inval");
	if (!CHECK_SUITE(suite)) CGI_redirect("?act=bail&e=inval");
	sn = get_suite_by_name(suite);
	if (sn == GE_ERROR) CGI_redirect("?act=bail&e=inval");
	if (!arch || (!CHECK_ARCH(arch, sn) && strcmp(arch, "DEFAULT"))) arch = "";

	s = ov_find_version(pkg, arch, suite, &ver);
	if (s != GE_OK)
		CGI_redirect("?act=bail");

	html = L_call("HtmlDebSrcHeader", 4, pkg, ver, suite, arch);
	html_output_puts(html);
	free(html);

	if (!strcmp(arch, "DEFAULT"))
		where = sqlite3_mprintf(
				"WHERE pkgname='%q' "
				"  AND version='%q' "
				"  AND suite='%q' ",
				pkg, ver, suite
				);
	else
		where = sqlite3_mprintf(
				"WHERE pkgname='%q' "
				"  AND version='%q' "
				"  AND suite='%q' "
				"  AND (deb_arch='%q' OR deb_arch='')",
				pkg, ver, suite, arch
				);
	if (!where)
		CGI_redirect("?act=bail");

	s = bc_search_all(where, NULL, bc_fetchdeb_html);
	if (s != GE_OK)
		CGI_redirect("?act=bail");

	html = L_call("HtmlDebSrcFooter", 4, pkg, ver, suite, arch);
	html_output_puts(html);
	free(html);

	free(ver);
}

static int bc_debcontrol_html(void *user, int cols, char **values, char **keys)
{
	char *html;

	html = L_call("HtmlDebControlPage", 1, Q(values[9]));
	html_output_puts(html);
	free(html);

	return GE_OK;
}

void action_debcontrol(void)
{
	char *where;
	char *hash = cgi_param("h");
	int s;

	where = sqlite3_mprintf("WHERE deb_md5='%q' ", hash);
	if (!where)
		CGI_redirect("?act=bail");

	s = bc_search_all(where, NULL, bc_debcontrol_html);
	if (s != GE_OK)
		CGI_redirect("?act=bail");
}

void action_srcview(void)
{
	char *html;
	char *pkg = cgi_param("pkg");
	char *suite = cgi_param("suite");
	char *ver, *defver = "";
	int s, sn, an, count;

	if (!CHECK_PKG(pkg)) CGI_redirect("?act=bail&e=inval");
	if (!CHECK_SUITE(suite)) CGI_redirect("?act=bail&e=inval");
	sn = get_suite_by_name(suite);
	if (sn == GE_ERROR) CGI_redirect("?act=bail&e=inval");

	html = L_call("HtmlSrcHeader", 2, pkg, suite);
	html_output_puts(html);
	free(html);

	s = ov_find_version(pkg, "all", suite, &defver);
	if (s == GE_OK) {
		bc_debs_count(pkg, defver, suite, "", &count);

		html = L_call("HtmlSrcItem", 5, pkg, suite, "DEFAULT", defver, "%%d");
		html_output_printf(html, count);
		free(html);
	}

	for (an = 0; SUITES[sn]->archlist[an]; an++) {
		s = ov_find_version(pkg, SUITES[sn]->archlist[an], suite, &ver);
		if (s == GE_OK && strcmp(defver, ver)) {
			bc_debs_count(pkg, ver, suite, SUITES[sn]->archlist[an], &count);

			html = L_call("HtmlSrcItem", 5, pkg, suite,
					SUITES[sn]->archlist[an], ver, "%%d");
			html_output_printf(html, count);
			free(html);
		}
	}

	html = L_call("HtmlSrcFooter", 0);
	html_output_puts(html);
	free(html);

	return;
}

void action_bail(void)
{
	char *error = cgi_param("e");
	char *html;

	if (!error) error = "oops";

	html = L_call("HtmlBailPage", 1, error);
	html_output_puts(html);
	free(html);

	return;
}

static int ov_searchres_html(void *user, int cols, char **values, char **keys)
{
	char *html;

	html = L_call("HtmlSearchResItem", 5, Q(values[0]), Q(values[1]),
			Q(values[2]), values[3], Q(values[4]));
	DBG("%s", html);
	html_output_puts(html);

	free(html);

	return GE_OK;
}

void action_searchres(void)
{
	char *html, *where;
	char *pkg = cgi_param("pkg");

	html = L_call("HtmlSearchResHeader", 1, pkg);
	html_output_puts(html);
	free(html);

	where = sqlite3_mprintf("WHERE pkgname LIKE '%%%q%%'", pkg);
	if (!where)
		CGI_redirect("?act=bail&e=intern");

	ov_search_all(where, NULL, ov_searchres_html);

	html = L_call("HtmlSearchResFooter", 0);
	html_output_puts(html);
	free(html);
}

struct cgi_action cgi_actions[] = {
	DECLARE_ACT(deblist,    "Binary Packages", 0),
	DECLARE_ACT(list,       "Source Packages", 0),
	DECLARE_ACT(edit,       "Edit Package Details", 1),
	DECLARE_ACT(commit,     NULL, 1),
	DECLARE_ACT(del,        "Delete Package", 1),
	DECLARE_ACT(new,        "Add Package", 1),
	DECLARE_ACT(debsrcview, "View Binary Package Info", 0),
	DECLARE_ACT(debcontrol, "View Binary Package Control Info", 0),
	DECLARE_ACT(srcview,    "View Source Package Info", 0),
	DECLARE_ACT(searchres,  "Search results", 0),
	DECLARE_ACT(bail,       "Error", 0),
	FINAL_ACT
};

int client_authd = 0;

const char *cgiaction_do(const char *act_name)
{
	int i;

	for (i = 0; cgi_actions[i].name; i++)
		if (!strcmp(act_name, cgi_actions[i].name)) {
			if (cgi_actions[i].auth_req && !client_authd) {
				cgi_redirect("?act=bail&e=auth");
				return NULL;
			}

			(*cgi_actions[i].act_fn)();
			return cgi_actions[i].title;
		}

	(*cgi_actions[0].act_fn)();
	return cgi_actions[0].title;
}

