/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sqlite3.h>
#include <libcgi/cgi.h>
#include "common.h"
#include "configuration.h"
#include "lua.h"
#include "db.h"
#include "debfile.h"
#include "util.h"
#include "html_static.h"
#include "lua-helpers.h"
#include "cgi-actions.h"

int main(int argc, const char **argv)
{
	int s;
	char o;
	char *act, *debug, *title;

	OUT[STD] = OUT[LOG] = stderr;

	memset(&G, 0, sizeof(struct global_config));

	init_slind();
	L_init();
	lua_register(L, "CgiEscapeSpecialChars", extl_cgi_escape_special_chars);

	s = config_init();
	if (s != GE_OK) {
		SHOUT("Error initializing configuration.\n");
		exit(EXIT_FAILURE);
	}

	s = db_init(G.odb_path);
	if (s != GE_OK) {
		SHOUT("Can't open database\n");
		exit(EXIT_FAILURE);
	}

	L_dofile(LUA_HTML);

	init_html_output();

	cgi_init();
	cgi_process_form();

	debug = cgi_param("debug");
	if (debug && !strcmp(debug, "yes"))
		verbosity = VERB_DEBUG;

	act = cgi_param("act");
	title = cgiaction_do(act ? act : cgi_actions[0].name);

	cgi_init_headers();
	puts(L_call("HtmlHeader", 1, title));
	flush_html_output();
	puts(L_call("HtmlFooter", 0));

	done_html_output();

	db_done();
	done_slind();
	L_done();

	config_done();

	return 0;
}

