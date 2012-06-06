/*
 * Copyright (c) 2010, Guillermo Amaral <gamaral@kdab.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "common.h"

#include <sys/stat.h>

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "db.h"
#include "session.h"
#include "task.h"

/****************************************************************** external variables */

SESSION       session;
TASK          task;
TASK_BOOKMARK bookmark;
TASK_RECENT   recent;

/********************************************************************* local variables */

static int    exit_code;

/************************************************************************ declarations */

static void finalize(void);
static void initialize(void);
static void print_help(const char *);
static void process_arguments(int, char **);

/************************************************************************* definitions */

int
main(int argc, char ** argv)
{
	initialize();

	process_arguments(argc, argv);

	finalize();

	return (exit_code);
}

/******************************************************************* local definitions */

void
finalize(void)
{
	close_database();

	/* Storage current task */
	task_save(session.taskfd);
	close(session.taskfd);

	task_bookmark_save(session.bookmarkfd);
	close(session.bookmarkfd);

	task_recent_save(session.recentfd);
	close(session.recentfd);

	free(session.db_path);
	free(session.home_path);
}

void
initialize(void)
{
	int fd;

	session.max_path     = (size_t) pathconf(getenv("HOME"), _PC_PATH_MAX);
	session.db_path      = malloc(session.max_path);
	session.home_path    = malloc(session.max_path);
	session.db           = 0;

	/* Set exit code to normal */
	exit_code            = 0;

	/* Set default Charm path */
	strcpy(session.home_path, getenv("HOME"));
	strcat(session.home_path, "/"CHARM_DIRECTORY);

	/* Change CWD to home */
	if (0 != chdir(session.home_path)) {
		ERROR((stderr, "Charm directory doesn't seem to exist: %s\n", session.home_path));
		quit(-1);
	}

	/* Check for debug db */
	if (0 < (fd = open(CHARM_DB, O_RDONLY))) {
		close(fd);
		change_database(CHARM_DB);
	} else if (DEBUG) {
		change_database(CHARM_DB_RELEASE);
	} else {
		change_database(CHARM_DB_DEBUG);
	}

	/* Load current task */
	if (-1 == (session.taskfd = open(TASK_PATH, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR))) {
		ERROR((stderr, "Unable to access task file.\n"));
		quit(-1);
	}
	task_load(session.taskfd);

	/* Load bookmark tasks */
	if (-1 == (session.bookmarkfd = open(BOOKMARK_TASKS_PATH, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR))) {
		ERROR((stderr, "Unable to access bookmarked tasks file.\n"));
		quit(-1);
	}
	task_bookmark_load(session.bookmarkfd);

	/* Load recent tasks */
	if (-1 == (session.recentfd = open(RECENT_TASKS_PATH, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR))) {
		ERROR((stderr, "Unable to access recent tasks file.\n"));
		quit(-1);
	}
	task_recent_load(session.recentfd);
}

void
print_help(const char * cmd)
{
	printf("CliCharm - CLI Charm (duh!)\n"
	       "Usage: %s [COMMAND] ... [OPTION] ... [QUERY] ...\n"
	       "\n", cmd);

	printf("  Commands: help                          This thing your reading right now.\n"
	       "            bookmark       [INDEX]        Bookmark current task.\n"
	       "            bookmarks                     Print out bookmarked tasks.\n"
	       "            discard                       Discard current task.\n"
	       "            recent                        Print out recent tasks.\n");
	printf("            start                         Start task timer.\n"
	       "            status                        Print out current task.\n"
	       "            stop                          Stop task timer and save.\n"
	       "            wipe                          Discard and wipe task clean.\n"
	       "\n");

	printf("   Queries: tasks [KEYWORD]               Print out tasks matching keyword.\n"
	       "\n");

	printf("   Options: -h, --help                    This thing your reading right now.\n"
	       "            -C, --charm-db [PATH]         Override default charm db path.\n"
	       "            -b, --bookmark [INDEX]        Clone bookmarked task.\n"
	       "            -c, --comment  [COMMENT]      Change comment for current task.\n"
	       "            -i, --task-id  [ID]           Change id for current task.\n"
	       "            -r, --recent   [INDEX]        Clone recent task.\n"
	       "\n");

	printf("   Example:\n"
	       "            %s discard -i 100 -c \"tardis\" start status\n"
	       "\n", cmd);
}

void
process_arguments(int argc, char **argv)
{
	register int i;

	if (argc <= 1) {
		task_print();
		exit_code = (TRUE == task_active() ? 1 : 0);
	}

	/* Command */
	for (i = 1; i < argc; ++i)
	{
		const char is_command = ('-' != argv[i][0]);

		if (is_command) {
			if (0 == strcasecmp("help", argv[i])) {
				print_help(argv[0]);
				quit(0);
			} else if (0 == strcasecmp("bookmark", argv[i])) {
				if ( ++i >= argc ) {
					ERROR((stderr, "No bookmark index was specified.\nAbort.\n"));
					quit(-1);
				}
				task_bookmark_store(atoi(argv[i]));
				INFO((stderr, "Tasks bookmarked.\n"));
			} else if (0 == strcasecmp("bookmarks", argv[i])) {
				task_bookmark_print();
			} else if (0 == strcasecmp("discard", argv[i])) {
				task_clear(FALSE);
				INFO((stderr, "Task discarted.\n"));
			} else if (0 == strcasecmp("start", argv[i])) {
				if (FALSE == task_active()) {
					task_reset();
					INFO((stderr, "Task started.\n"));
				} else {
					INFO((stderr, "Task already started.\nIgnored.\n"));
				}
			} else if (0 == strcasecmp("recent", argv[i])) {
				task_recent_print();
			} else if (0 == strcasecmp("status", argv[i])) {
				task_print();
				exit_code = (TRUE == task_active() ? 1 : 0);
			} else if (0 == strcasecmp("stop", argv[i])) {
				task_store();
				task_clear(FALSE);
				INFO((stderr, "Task stopped and stored.\n"));
			} else if (0 == strcasecmp("tasks", argv[i])) {
				if ( ++i >= argc ) {
					ERROR((stderr, "No keyword was specified.\nAbort.\n"));
					quit(-1);
				}
				task_tasks(argv[i]);
				INFO((stderr, "Tasks displayed.\n"));
			} else if (0 == strcasecmp("wipe", argv[i])) {
				task_clear(TRUE);
				INFO((stderr, "Task wipped.\n"));
			} else {
				ERROR((stderr, "Invalid Command: %s\nAbort.\n", argv[i]));
				quit(-1);
			}
		} else {
			if ((0 == strcmp("--help", argv[i])) || 
			    (0 == strcmp("-h",     argv[i]))) {
				print_help(argv[0]);
				quit(0);
			} else if ((0 == strcmp("--bookmark", argv[i])) || 
			           (0 == strcmp("-b",       argv[i]))) {
				if ( ++i >= argc ) {
					ERROR((stderr, "No bookmarked task index was specified.\nAbort.\n"));
					quit(-1);
				}
				task_bookmark_select(atoi(argv[i]));
			} else if ((0 == strcmp("--charm-db", argv[i])) || 
			           (0 == strcmp("-C",         argv[i]))) {
				if ( ++i >= argc ) {
					ERROR((stderr, "No database path was specified.\nAbort.\n"));
					quit(-1);
				}
				change_database(argv[i]);
			} else if ((0 == strcmp("--task-id", argv[i])) || 
			           (0 == strcmp("-i",        argv[i]))) {
				if ( ++i >= argc ) {
					ERROR((stderr, "No task id was specified.\nAbort.\n"));
					quit(-1);
				}
				task_select(atoi(argv[i]));
			} else if ((0 == strcmp("--comment", argv[i])) || 
			           (0 == strcmp("-c",        argv[i]))) {
				if ( ++i >= argc ) {
					ERROR((stderr, "No comment was specified.\nAbort.\n"));
					quit(-1);
				}
				task_comment(argv[i]);
			} else if ((0 == strcmp("--recent", argv[i])) || 
			           (0 == strcmp("-r",       argv[i]))) {
				if ( ++i >= argc ) {
					ERROR((stderr, "No recent task index was specified.\nAbort.\n"));
					quit(-1);
				}
				task_recent_select(atoi(argv[i]));
			} else {
				ERROR((stderr, "Invalid Option: %s\nAbort.\n", argv[i]));
				quit(-1);
			}
		}
	}
}

void
quit(int code)
{
	finalize();

	if ( 0 != code )
		ERROR((stderr, "Force Quit\n"));

	_exit(code);
}

