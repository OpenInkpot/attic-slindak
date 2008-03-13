/*
 * vi: sw=4 ts=4 noexpandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include "common.h"
#include "util.h"

#define MAX_EXITFN 32

static exit_fn_t exit_calls[MAX_EXITFN];
static int last_exit_call = 0;

/*
 * add a function to the cleanup function stack
 */
int push_cleaner(exit_fn_t exit_fn)
{
	if (last_exit_call >= MAX_EXITFN)
		return GE_ERROR;

	exit_calls[last_exit_call++] = exit_fn;
	return last_exit_call;
}

/*
 * remove a cleanup function specified by address
 */
int pop_cleaner(exit_fn_t exit_fn)
{
	int i;

	for (i = 0; i < last_exit_call; i++)
		if (exit_calls[i] == exit_fn) {
			memcpy(&exit_calls[i], &exit_calls[i + 1],
					sizeof(exit_fn_t) * (last_exit_call - i));

			return GE_OK;
		}

	return GE_ERROR;
}

static struct sigaction __sigint_act_saved;
static int __sigint_lock = 0; /* lock to prevent sigint */

/*
 * acquire sigint lock
 * code that follows is protected for sigint untill libslindak_unlock
 */
void libslindak_lock(void)
{
	__sigint_lock++;
}

/*
 * release sigint lock
 */
void libslindak_unlock(void)
{
	if (__sigint_lock)
		__sigint_lock--;
}

/*
 * perform necessary cleanup operation before exiting
 */
void __attribute__((destructor)) libslindak_exit(void)
{
	while (last_exit_call--) {
		(*exit_calls[last_exit_call])();
		exit_calls[last_exit_call] = NULL;
	}

	last_exit_call = 0;
	sigaction(SIGINT, &__sigint_act_saved, NULL);
}

/*
 * sigint handler: do nothing on locked code paths
 */
static void sigint_handler(int sig)
{
	if (__sigint_lock)
		return;

	SAY("Control-C pressed.\n");
	signal(sig, NULL);
	exit(EXIT_FAILURE);
}

void __attribute__((constructor)) libslindak_init(void)
{
	struct sigaction sa;

	output_init();
	root_squash();

	memset(&exit_calls, 0, sizeof(exit_calls));
	atexit(libslindak_exit);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sigint_handler;
	sigfillset(&sa.sa_mask);
	sigaction(SIGINT, &sa, &__sigint_act_saved);
}

