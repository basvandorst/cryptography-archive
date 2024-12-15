/*
 * $Id: pgpPktList.h,v 1.7 1997/06/24 07:16:40 lloyd Exp $
 */

#ifndef Included_pgpPktList_h
#define Included_pgpPktList_h

#include <stddef.h>	/* For size_t */

#include "pgpPubTypes.h"	/* For PGPByte */

PGP_BEGIN_C_DECLARATIONS

/* 
 * This is a trivial interface.  You may just access the members
 * directly.  buf may change from an included buffer to a pointer
 * at some point, but that should not require any changes to the
 * source code.
 */

typedef struct PktList
{
	PGPContextRef	context;
	
	struct PktList *next;
	int type;
	size_t len;
	PGPByte buf[1];	/* Extensible */
} PktList ;

PktList *pgpPktListNew( PGPContextRef context,
	int type, PGPByte const *buf, size_t len);
void pgpPktListFreeOne(PktList *list);
void pgpPktListFreeList(PktList *list);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpPktList_h */
