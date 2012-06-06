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

#include "db.h"

#include <sqlite3.h>
#include <string.h>

#include "session.h"

/************************************************************************* definitions */

void
change_database(const char *path)
{
	close_database();
	strncpy(session.db_path, path, session.max_path);
	open_database();
}

void
close_database(void)
{
	if (0 != session.db)
		sqlite3_close(session.db);

	session.db = 0;
}

void
open_database(void)
{
	if (SQLITE_OK != sqlite3_open_v2(session.db_path, &session.db,
	    SQLITE_OPEN_READWRITE, 0)) {
		ERROR((stderr, "Can't open database: %s\n%s\n",
		       session.db_path, sqlite3_errmsg(session.db)));
		quit(-1);
	}

	INFO((stderr, "Database Changed: %s\n", session.db_path));
}

