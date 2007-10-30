/*
 * vi: sw=4 ts=4 noexpandtab
 */

#ifndef __LUA_HELPERS_H__
#define __LUA_HELPERS_H__

#define L_pop_table(x) do { /*lua_pop(L, 2 * x);*/ } while (0)

int L_push_int_str(int table, int key, char *val);
int L_push_pair_str(int table, char *key, char *val);
int L_push_str_vstr(int table, char *key, char *fmt, ...);
int L_push_table(char *table, int parent);
char *L_call(char *fn, int argc, ...);
char *L_get_string(char *name, int table);
char *L_get_confstr(char *field, char *table);
int L_dofile(const char *path);
int L_get_array(int index, char **array, int narr);
int L_init();
void L_done();

#endif /* __UTIL_H__ */

