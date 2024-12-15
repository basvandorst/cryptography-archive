/*
 * $Id: pgpPktList.c,v 1.10 1997/07/26 19:37:51 lloyd Exp $
 */

#include "pgpConfig.h"

#include <string.h>	/* For memcpy */
#include <stddef.h>	/* For offsetof */

#include "pgpPktList.h"
#include "pgpContext.h"
#include "pgpMem.h"

PktList *
pgpPktListNew(
	PGPContextRef	context,
	int type, PGPByte const *buf, size_t len)
{
	PktList *pkl;

	pkl = (PktList *)pgpContextMemAlloc( context,
		len + offsetof(PktList, buf), kPGPMemoryFlags_Clear);
	if (pkl) {
		pkl->context	= context;
		pkl->next = (PktList *)0;
		pkl->type = type;
		pkl->len = len;
		memcpy(pkl->buf, buf, len);
	}
	return pkl;
}

void
pgpPktListFreeOne(PktList *pkl)
{
	PGPContextRef	cdkContext;

	pgpAssertAddrValid( pkl, PktList );
	cdkContext	= pkl->context;
	
	pgpClearMemory( pkl,  pkl->len + offsetof(PktList, buf));
	pgpContextMemFree( cdkContext, pkl);
}

void
pgpPktListFreeList(PktList *list)
{
	PktList				*cur;
	PGPContextRef		cdkContext;

	if( IsNull(list) )
		return;
	cdkContext = list->context;
	while (list) {
		/* If you don't understand this, yer a wimp.  Belch.  Grunt. */
		list = (cur = list)->next;
		pgpClearMemory( cur,  cur->len + offsetof(PktList, buf));
		pgpContextMemFree( cdkContext, cur);
	}
}
