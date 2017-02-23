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

#include "task.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "session.h"
#include "stack.h"

/************************************************************************ declarations */

static int task_recurse_name_callback(void *, int, char **, char **);
static int task_tasks_callback(void *, int, char **, char **);

/*************************************************************************** constants */

static const size_t ctask_size = sizeof(TASK);
static const size_t ctask_bookmark_size = sizeof(TASK_BOOKMARK);
static const size_t ctask_recent_size = sizeof(TASK_RECENT);

/******************************************************************************* flags */

static BOOL modtask;
static BOOL modbookmark;
static BOOL modrecent;

/************************************************************************* definitions */

BOOL
task_active(void)
{
	return(0 == task.start_time? FALSE : TRUE);
}

void
task_bookmark_clear(void)
{
	memset(&bookmark, 0, ctask_bookmark_size);
	modbookmark = TRUE;
}

void
task_bookmark_load(int fd)
{
	lseek(fd, (off_t) 0, SEEK_SET);
	if ((int)ctask_bookmark_size != read(fd, &bookmark, ctask_bookmark_size)) {
		INFO((stderr, "Unable to read bookmark tasks. Initializing.\n"));
		task_bookmark_clear();
	}
	modbookmark = FALSE;
}

void
task_bookmark_print(void)
{
	int i;

	for (i = 0; i < MAX_TASK_BOOKMARK_LEN; ++i) {
		if (0 == bookmark.tasks[i].task_id) {
			printf("Index %02d: EMPTY\n", i);
		} else {
			printf("Index %02d: %s (%s)\n",
			       i,
			       bookmark.tasks[i].task_name,
			       bookmark.tasks[i].comment);
		}
	}
	printf("\n");
}

void
task_bookmark_save(int fd)
{
	if (FALSE == modbookmark)
		return;

	lseek(fd, (off_t) 0, SEEK_SET);
	if ((int)ctask_bookmark_size != write(fd, &bookmark, ctask_bookmark_size)) {
		ERROR((stderr, "Unable to save bookmark tasks. ABORT.\n"));
		quit(-1);
	}
	modbookmark = FALSE;
}

void
task_bookmark_select(int idx)
{
	time_t starttime;

	if (0 > idx || MAX_TASK_BOOKMARK_LEN <= idx)
		return;

	starttime = task.start_time;
	memcpy(&task, &bookmark.tasks[idx], ctask_size);
	task.start_time = starttime;
	modtask = TRUE;
}

void
task_bookmark_store(int idx)
{
	if (idx < 0 || idx >= MAX_TASK_BOOKMARK_LEN) {
		ERROR((stderr, "Bookmark index out of bounds. ABORT.\n"));
		quit(-1);
	}

	memcpy(&bookmark.tasks[idx], &task, ctask_size);
	bookmark.tasks[idx].start_time = 0;
	modbookmark = TRUE;
}

void
task_clear(BOOL full)
{
	if (TRUE == full) {
		task.task_id = 0;
		memset(task.task_name, 0, sizeof(task.task_name));
		memset(task.comment, 0, sizeof(task.comment));
	}
	task.start_time = 0;
	modtask = TRUE;
}

void
task_comment(const char *comment)
{
	strncpy(task.comment, comment, MAX_TASK_COMMENT_LEN);
	modtask = TRUE;
}

void
task_recurse_name(int id, char *task_name)
{
	char querystr[128];
	char *errstr;
	int   errcode;

	sprintf(querystr, "SELECT `parent`, `task_id`, `trackable`, `name` FROM `Tasks` "
	                  "WHERE `task_id` = \"%d\" LIMIT 1",
	                  id);

	errcode = sqlite3_exec(session.db, querystr, task_recurse_name_callback, task_name, &errstr);
	if (SQLITE_OK != errcode &&
	    SQLITE_ABORT != errcode) {
		ERROR((stderr, "SQL error: %s\n", errstr));
		sqlite3_free(errstr);
		quit(-1);
	}
}

void
task_load(int fd)
{
	lseek(fd, (off_t) 0, SEEK_SET);
	if ((int)ctask_size != read(fd, &task, ctask_size)) {
		INFO((stderr, "Unable to load current task state. Initializing.\n"));
		task_clear(TRUE);
	}
	modtask = FALSE;
}

void
task_print(void)
{
	if (TRUE == task_active()) {
		double delta_t = difftime(time(0), task.start_time);
		double hours, minutes, seconds;

		hours = floor(delta_t / SECONDS_PER_HOUR);
		delta_t -= (hours * SECONDS_PER_HOUR);
		minutes = floor(delta_t / SECONDS_PER_MINUTE);
		seconds = delta_t - (minutes * SECONDS_PER_MINUTE);

		printf("Task: %s\nComment: %s\nStarted: %sRunning Time: %02d:%02d:%02d\n\n",
		       task.task_name, task.comment, ctime(&task.start_time),
		       (int) hours, (int) minutes, (int) seconds);
	} else {
		printf("** No task currently running **\n\n");
		printf("Task: %s\nComment: %s\n\n",
		       task.task_name, task.comment);
	}
}

void
task_recent_clear(void)
{
	memset(&recent, 0, ctask_recent_size);
	modrecent = TRUE;
}

void
task_recent_load(int fd)
{
	lseek(fd, (off_t) 0, SEEK_SET);
	if ((int)ctask_recent_size != read(fd, &recent, ctask_recent_size)) {
		INFO((stderr, "Unable to read recent tasks. Initializing.\n"));
		task_recent_clear();
	}
	modrecent = FALSE;
}

void
task_recent_print(void)
{
	int i;

	for (i = 0; i < MAX_TASK_RECENT_LEN; ++i) {
		if (0 == recent.tasks[i].task_id) {
			printf("Index %02d: EMPTY\n", i);
		} else {
			printf("Index %02d: [%04d] %s (%s)\n",
			       i,
			       recent.tasks[i].task_id,
			       recent.tasks[i].task_name,
			       recent.tasks[i].comment);
		}
	}
	printf("\n");
}

void
task_recent_save(int fd)
{
	if (FALSE == modrecent)
		return;

	lseek(fd, (off_t) 0, SEEK_SET);
	if ((int)ctask_recent_size != write(fd, &recent, ctask_recent_size)) {
		ERROR((stderr, "Unable to save recent tasks. ABORT.\n"));
		quit(-1);
	}
	modrecent = FALSE;
}

void
task_recent_select(int idx)
{
	time_t starttime;

	if (0 > idx || MAX_TASK_RECENT_LEN <= idx)
		return;

	starttime = task.start_time;
	memcpy(&task, &recent.tasks[idx], ctask_size);
	task.start_time = starttime;
	modtask = TRUE;
}

void
task_recent_store(void)
{
	int i;
	for (i = MAX_TASK_RECENT_LEN - 1; i > 0; --i)
		memcpy(&recent.tasks[i], &recent.tasks[i - 1], ctask_size);
	memcpy(&recent.tasks[0], &task, ctask_size);
	recent.tasks[0].start_time = 0;
	modrecent = TRUE;
}

void
task_reset(void)
{
	task.start_time = time(0);
	modtask = TRUE;
}

void
task_save(int fd)
{
	if (FALSE == modtask)
	  return;

	lseek(fd, (off_t) 0, SEEK_SET);
	if ((int)ctask_size != write(fd, &task, ctask_size)) {
		ERROR((stderr, "Unable to save current task state. ABORT.\n"));
		quit(-1);
	}
	modtask = FALSE;
}

void
task_select(int id)
{
	task.task_id = id;
	memset(task.task_name, 0, sizeof(task.task_name));
	task_recurse_name(id, task.task_name);
	modtask = TRUE;
}

void
task_store(void)
{
	void *null = 0;
	char querystr[512];
	char *errstr;
	time_t now;
	struct tm start_time;
	struct tm end_time;
	struct tm *tm_time;

	if (0 == task.start_time)
		return;

	now = time(0);
	tm_time = localtime(&task.start_time);
	memcpy(&start_time, tm_time, sizeof(struct tm));
	tm_time = localtime(&now);
	memcpy(&end_time,   tm_time, sizeof(struct tm));

	sprintf(querystr, "INSERT INTO `Events` "
	                  " (`installation_id`, `report_id`, `task`, `comment`, `start`, `end`) "
	                  " VALUES (1, 0, %d, \"%s\", \"%4d-%02d-%02dT%02d:%02d:%02d\", \"%4d-%02d-%02dT%02d:%02d:%02d\")",
	        task.task_id, task.comment,
	        1900 + start_time.tm_year, 1 + start_time.tm_mon, start_time.tm_mday, start_time.tm_hour, start_time.tm_min, start_time.tm_sec,
	        1900 + end_time.tm_year, 1 + end_time.tm_mon, end_time.tm_mday, end_time.tm_hour, end_time.tm_min, end_time.tm_sec);

	if (SQLITE_OK != sqlite3_exec(session.db, querystr, 0, null, &errstr) ) {
		ERROR((stderr, "SQL error: %s\n", errstr));
		sqlite3_free(errstr);
		quit(-1);
	}

	sprintf(querystr, "UPDATE `Events` SET `event_id` = last_insert_rowid() WHERE `id` = last_insert_rowid()");

	if (SQLITE_OK != sqlite3_exec(session.db, querystr, 0, null, &errstr) ) {
		ERROR((stderr, "SQL error: %s\n", errstr));
		sqlite3_free(errstr);
		quit(-1);
	}

	task_recent_store();
}

void
task_tasks(const char *keyword)
{
	STACK leafs;
	char querystr[512];
	char task_name[MAX_TASK_NAME_LEN + 1];
	char *errstr;
	int task_id;

	sprintf(querystr, "SELECT `task_id`, `name` FROM `Tasks` "
	                  "WHERE (`name` LIKE \"%%%s%%\"  OR `task_id` = \"%s\") "
	                    "AND (`validfrom`  <= CURRENT_DATE OR `validfrom`  ISNULL) "
	                    "AND (`validuntil` >= CURRENT_DATE OR `validuntil` ISNULL)",
	                   keyword, keyword);

	leafs = stack_create();

	if (SQLITE_OK != sqlite3_exec(session.db, querystr, task_tasks_callback, leafs, &errstr) ) {
		ERROR((stderr, "SQL error: %s\n", errstr));
		sqlite3_free(errstr);
		quit(-1);
	}

	while (stack_empty(leafs) == FALSE) {
		memset(task_name, 0, sizeof(task_name));
		task_id = stack_pop(leafs);
		task_recurse_name(task_id, task_name);

		printf("%s\n", task_name);
	}

	stack_destroy(leafs);
}

BOOL
task_trackable(int id)
{
	BOOL trackable;
	int ret;
	sqlite3_stmt *stmt;

	const char querystr[] =
	    "SELECT `trackable` FROM `Tasks` "
	    "WHERE (`task_id` = ?) "
	       "AND (`validfrom`  <= CURRENT_DATE OR `validfrom`  ISNULL) "
	       "AND (`validuntil` >= CURRENT_DATE OR `validuntil` ISNULL) "
	       "LIMIT 1";

	ret = sqlite3_prepare_v2(session.db, querystr, sizeof(querystr), &stmt, NULL);
	if (SQLITE_OK != ret) {
		ERROR((stderr, "SQL error: '%s' %s\n", querystr, sqlite3_errmsg(session.db)));
		quit(-1);
	}
	assert(stmt);

	ret = sqlite3_bind_int(stmt, 1, id);
	if (SQLITE_OK != ret) {
		ERROR((stderr, "SQL error: '%s' %s\n", querystr, sqlite3_errmsg(session.db)));
		quit(-1);
	}

	trackable = FALSE;
	do {
		ret = sqlite3_step(stmt);
		switch (ret) {
		case SQLITE_ROW:
			trackable = sqlite3_column_int(stmt, 0) == 1;
			/* fall-through */
		case SQLITE_DONE:
			break;

		default:
			ERROR((stderr, "SQL error: %s\n", sqlite3_errmsg(session.db)));
			quit(-1);
			break;
		}
	} while (SQLITE_DONE != ret);

	sqlite3_finalize(stmt);

	return trackable;
}

void
task_find_leafs(STACK stack, int parent)
{
	int ret;
	sqlite3_stmt *stmt;

	const char querystr[] =
	    "SELECT `task_id`, `trackable` FROM `Tasks` "
	    "WHERE (`parent` = ?) "
	       "AND (`validfrom`  <= CURRENT_DATE OR `validfrom`  ISNULL) "
	       "AND (`validuntil` >= CURRENT_DATE OR `validuntil` ISNULL)";

	ret = sqlite3_prepare_v2(session.db, querystr, sizeof(querystr), &stmt, NULL);
	if (SQLITE_OK != ret) {
		ERROR((stderr, "SQL error: %s\n", sqlite3_errmsg(session.db)));
		quit(-1);
	}
	assert(stmt);

	ret = sqlite3_bind_int(stmt, 1, parent);
	if (SQLITE_OK != ret) {
		ERROR((stderr, "SQL error: '%s' %s\n", querystr, sqlite3_errmsg(session.db)));
		quit(-1);
	}

	do {
		ret = sqlite3_step(stmt);
		switch (ret) {
		case SQLITE_ROW:
			task_find_leafs(stack, sqlite3_column_int(stmt, 0));
			continue;

		case SQLITE_DONE:
			break;

		default:
			ERROR((stderr, "SQL error: %s\n", sqlite3_errmsg(session.db)));
			quit(-1);
			break;
		}
	} while (SQLITE_DONE != ret);

	sqlite3_finalize(stmt);

	if (task_trackable(parent) && !stack_contains(stack, parent)) {
		stack_push(stack, parent);
	}
}

int
task_recurse_name_callback(void *task_name, int columns, char **data, char **headers)
{
	char task_str[MAX_TASK_NAME_LEN];
	int parent_id;
	int task_id;
	BOOL trackable;

	UNUSED(columns);
	UNUSED(headers);

	parent_id = atoi(data[0]);
	task_id = atoi(data[1]);
	trackable = (atoi(data[2]) == 1);

	if (*(char *)task_name != '\0') {
	    strncat((char *)task_name, "\n   ", MAX_TASK_NAME_LEN);
	}

	if (trackable)
		snprintf(task_str, MAX_TASK_NAME_LEN, "[%04d] %s", task_id, data[3]);
	else
		snprintf(task_str, MAX_TASK_NAME_LEN, "{%04d} %s", task_id, data[3]);
	strncat((char *)task_name, task_str, MAX_TASK_NAME_LEN);

	if (parent_id != 0)
		task_recurse_name(parent_id, task_name);

	return (1);
}

int
task_tasks_callback(void *stack, int columns, char **data, char **headers)
{
	UNUSED(columns);
	UNUSED(headers);

	task_find_leafs(stack, atoi(data[0]));

	return (0);
}

