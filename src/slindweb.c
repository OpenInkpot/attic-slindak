/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sqlite3.h>
#include "common.h"
#include "configuration.h"
#include "lua.h"
#include "db.h"
#include "debfile.h"
#include "util.h"

int main(int argc, const char **argv)
{
	int s;
	char o;
	
	output_init();
	OUT[STD] = OUT[LOG] = stderr;
	root_squash();

	memset(&G, 0, sizeof(struct global_config));

	init_slind();
	L_init();

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

	G.op_mode = OM_POOL;

	db_done();
	done_slind();
	L_done();

	config_done();

	return 0;
}

