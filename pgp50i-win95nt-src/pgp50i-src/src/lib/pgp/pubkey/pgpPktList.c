/*
* pgpPktList.c
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpPktList.c,v 1.1.2.1 1997/06/07 09:51:28 mhw Exp $
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>	 /* For memcpy */
#include <stddef.h>	 /* For offsetof */

#include "pgpPktList.h"
#include "pgpMem.h"

struct PktList *
pgpPktListNew(int type, byte const *buf, size_t len)
{
		struct PktList *pkl;

		pkl = (struct PktList *)pgpMemAlloc(len + offsetof(struct PktList,
									buf));
		if (pkl) {
				pkl->next = (struct PktList *)0;
				pkl->type = type;
				pkl->len = len;
				memcpy(pkl->buf, buf, len);
		}
		return pkl;
}

void
pgpPktListFreeOne(struct PktList *pkl)
{
		memset(pkl, 0, pkl->len + offsetof(struct PktList, buf));
		pgpMemFree(pkl);
}

void
pgpPktListFreeList(struct PktList *list)
{
		struct PktList *cur;

		while (list) {
			/* If you don't understand this, yer a wimp. Belch. Grunt. */
			list = (cur = list)->next;
			memset(cur, 0, cur->len + offsetof(struct PktList, buf));
			pgpMemFree(cur);
		}
}
