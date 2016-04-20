/*
 * Copyright (c) 2015, Guillermo Amaral <gamaral@kdab.com>
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

#include "stack.h"

#include <stdlib.h>

/************************************************************************ declarations */

struct t_STACK
{
	struct t_STACK_ITEM *head;
};

struct t_STACK_ITEM
{
	struct t_STACK_ITEM *next;
	int value;
};

/************************************************************************* definitions */

STACK
stack_create(void)
{
	STACK s = malloc(sizeof(struct t_STACK));
	s->head = NULL;
	return(s);
}

void
stack_destroy(STACK s)
{
	struct t_STACK_ITEM *item = s->head;

	while (item) {
		struct t_STACK_ITEM *prev = item;
		item = prev->next;
		free(prev);
	}

	free(s);
}

void
stack_push(STACK s, int value)
{
	struct t_STACK_ITEM *prev = s->head;

	s->head = malloc(sizeof(struct t_STACK_ITEM));
	s->head->next = prev;
	s->head->value = value;
}

int
stack_pop(STACK s)
{
	struct t_STACK_ITEM *item = s->head;
	int result;

	if (NULL == item)
		return(-1);

	s->head = item->next;
	result = item->value;
	free(item);

	return(result);
}

BOOL
stack_empty(STACK s)
{
	return(s->head == NULL ? TRUE : FALSE);
}

