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

#ifndef TASK_H
#define TASK_H 1

#include <time.h>

#include "common.h"

/****************************************************************** compiler constants */

#define MAX_TASK_COMMENT_LEN 255
#define MAX_TASK_NAME_LEN 127
#define MAX_TASK_RECENT_LEN RECENT_TASKS_MAX
#define MAX_TASK_BOOKMARK_LEN BOOKMARK_TASKS_MAX

#define SECONDS_PER_HOUR 3600.f
#define SECONDS_PER_MINUTE 60.f

/************************************************************************ declarations */

struct t_TASK {
	int    task_id;
	char   task_name[MAX_TASK_NAME_LEN + 1];
	time_t start_time;
	char   comment[MAX_TASK_COMMENT_LEN + 1];
};
typedef struct t_TASK TASK;

struct t_TASK_BOOKMARK {
	TASK tasks[MAX_TASK_BOOKMARK_LEN];
};
typedef struct t_TASK_BOOKMARK TASK_BOOKMARK;

struct t_TASK_RECENT {
	TASK tasks[MAX_TASK_RECENT_LEN];
};
typedef struct t_TASK_RECENT TASK_RECENT;

/****************************************************************** exported variables */

extern TASK task;
extern TASK_BOOKMARK bookmark;
extern TASK_RECENT recent;

/************************************************************************ declarations */

BOOL task_active(void);
void task_bookmark_clear(void);
void task_bookmark_load(int fd);
void task_bookmark_print(void);
void task_bookmark_save(int fd);
void task_bookmark_select(int index);
void task_bookmark_store(int index);
void task_clear(BOOL);
void task_comment(const char *);
void task_load(int fd);
void task_print(void);
void task_recent_clear(void);
void task_recent_load(int fd);
void task_recent_print(void);
void task_recent_save(int fd);
void task_recent_select(int index);
void task_recent_store(void);
void task_reset(void);
void task_save(int fd);
void task_select(int id);
void task_store(void);
void task_tasks(const char *);

#endif
