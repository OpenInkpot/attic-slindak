/*
 * vi: sw=4 ts=4 noexpandtab
 */
#ifndef __SUITE_H__
#define __SUITE_H__

int suite_add(char *name, char **arches, char **comps, int idx);
void suite_remove(int idx);
void suite_remove_all(void);
int get_suite_by_name(char *suite);

#endif /* __SUITE_H__ */

