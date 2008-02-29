/*
 * vi: sw=4 ts=4 noexpandtab
 */
#ifndef __CGI_ACTIONS_H__
#define __CGI_ACTIONS_H__

struct cgi_action {
	void (*act_fn)(void);
	const char *name;
	const char *title;
	int auth_req;
};

extern struct cgi_action cgi_actions[];

extern int client_authd;

#define DECLARE_ACT(__name, __title, __auth) \
	{                                        \
		.name     = # __name,                \
		.act_fn   = action_ ## __name,       \
		.title    = __title,                 \
		.auth_req = __auth                   \
	}

#define FINAL_ACT \
	{ .name = NULL, .act_fn = NULL, .title = NULL, .auth_req = 0 }

static int valid_string(char *str, int max, int min)
{
	if (!str) return 0;
	if (strlen(str) > max || strlen(str) < min) return 0;
	return 1;
}

static int grep_string(char *str, char **arr, int sz)
{
	int i;

	if (sz) {
		for (i = 0; i < sz; i++)
			if (arr[i] && !strcmp(str, arr[i]))
				return i;
	} else {
		for (i = 0; arr[i]; i++)
			if (!strcmp(str, arr[i]))
				return i;
	}

	return -1;
}

int extl_cgi_escape_special_chars(lua_State *L);
const char *cgiaction_do(const char *act_name);

#endif /* __CGI_ACTIONS_H__ */

