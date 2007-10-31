/*
 * vi: sw=4 ts=4 noexpandtab
 */
#ifndef __DB_H__
#define __DB_H__

#include <sqlite3.h>

/* overrides.db sqlite3 handle, db.c */
extern sqlite3 *db;

int db_init();
void db_done();

#include "ov.h"

#endif

