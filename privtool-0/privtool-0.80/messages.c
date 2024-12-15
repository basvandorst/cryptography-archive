
/*
 *	@(#)messages.c	1.12 6/29/94
 *
 *	(c) Copyright 1993-1994 by Mark Grant. All right reserved.
 *	The author assumes no liability for damages resulting from the 
 *	use of this software, even if the damage results from defects in
 *	this software. No warranty is expressed or implied.
 *
 *	This software is being distributed under the GNU Public Licence,
 *	see the file COPYING for more details.
 *
 *			- Mark Grant (mark@unicorn.com) 29/6/94
 *
 */

#include <stdio.h>
#include <malloc.h>

#include "def.h"
#include "buffers.h"
#include "message.h"

void	free_string(s)

char	*s;

{
	if (s) {
		bzero (s, strlen(s));
		free (s);
	}
}

MESSAGE	*new_message()

{
	MESSAGE	*m;

	m = (MESSAGE *)malloc(sizeof(MESSAGE));

	m->message = 0;
	m->header = 0;
	m->signature = 0;
	m->decrypted = 0;
	m->sender = 0;
	m->email = 0;
	m->subject = 0;
	m->date = 0;
	m->next = 0;
	m->prev = 0;
	m->lines = 0;
	m->size = 0;
	m->number = 0;
	m->list_pos = 0;
	m->description = 0;
	m->message_id = 0;
	m->header_date = 0;

	m->flags = 0;

	return m;
}

free_message(b)

MESSAGE	*b;

{
	if (b->message)
		free_buffer (b->message);
	if (b->header)
		free_buffer (b->header);
	if (b->signature)
		free_buffer (b->signature);
	if (b->decrypted)
		free_buffer (b->decrypted);

	if (b->sender)
		free_string (b->sender);
	if (b->email)
		free_string (b->email);
	if (b->date)
		free_string (b->date);
	if (b->subject)
		free_string (b->subject);
	if (b->description)
		free_string (b->description);
	if (b->message_id)
		free_string (b->message_id);
	if (b->header_date)
		free_string (b->header_date);

	free(b);
}

/* Add a message to the specified message list */

void	add_to_message_list_start(l,m)

MESSAGE_LIST	*l;
MESSAGE		*m;

{
	if (l->start) {
		m->next = l->start;
		l->start->prev = m;
		l->start = m;
		m->prev = NULL;
	}
	else {
		m->next = NULL;
		m->prev = NULL;

		l->start = l->end = m;
	}

	l->number++;
}

void	add_to_message_list_end(l,m)

MESSAGE_LIST	*l;
MESSAGE		*m;

{
	if (l->end) {
		m->prev = l->end;
		l->end->next = m;
		l->end = m;
		m->next = NULL;
	}
	else {
		m->next = NULL;
		m->prev = NULL;

		l->start = l->end = m;
	}

	l->number++;
}
