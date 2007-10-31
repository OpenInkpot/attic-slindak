/*
 * vi: sw=4 ts=4 noexpandtab
 */

#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include "conf.h"

#if HACKER
#define LUA_APT_CONF "lua/apt-conf.lua"
#define LUA_MAIN_CONF "lua/slindak-config.lua"
#endif

#ifndef LUA_APT_CONF
#define LUA_APT_CONF "/usr/share/slindak/apt-conf.lua"
#endif

#ifndef LUA_MAIN_CONF
#define LUA_MAIN_CONF "/etc/slindak-config.lua"
#endif

#ifndef LUA_TABLE_SUITES
#define LUA_TABLE_SUITES "Suites"
#endif

#ifndef LUA_TABLE_CONFIG
#define LUA_TABLE_CONFIG "Config"
#endif

#define CONFIG_SET(name, table) \
	do { \
		if (!G.name) \
			G.name = L_get_confstr(# name, table); \
	} while (0);

int config_init();
void config_done();

#endif

