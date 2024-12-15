/*
* pgpPktList.h
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpPktList.h,v 1.2.2.1 1997/06/07 09:51:28 mhw Exp $
*/

#ifndef PGPPKTLIST_H
#define PGPPKTLIST_H

#include <stddef.h>	 /* For size_t */

#include "pgpUsuals.h"	/* For byte */

#ifdef __cplusplus
extern "C" {
#endif

/*
* This is a trivial interface. You may just access the members
* directly. buf may change from an included buffer to a pointer
* at some point, but that should not require any changes to the
* source code.
*/

struct PktList {
		struct PktList *next;
		int type;
		size_t len;
		byte buf[1];	/* Extensible */
};

struct PktList *pgpPktListNew(int type, byte const *buf, size_t len);
void pgpPktListFreeOne(struct PktList *list);
void pgpPktListFreeList(struct PktList *list);

#ifdef __cplusplus
}
#endif

#endif PGPPKTLIST_H
