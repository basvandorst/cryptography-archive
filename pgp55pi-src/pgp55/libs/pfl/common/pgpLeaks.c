/*____________________________________________________________________________
	pgpLeaks.c
	
	Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Leak tracking module for memory allocation routines

	$Id: pgpLeaks.c,v 1.19 1997/09/18 01:34:11 lloyd Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "pgpMem.h"
#include "pgpLeaks.h"
#include "pgpTypes.h"
#if PGP_MACINTOSH
#include "MacEvents.h"
#endif

#if USE_PGP_LEAKS	/* [ */

#ifdef _WIN32
#define inline
#endif

typedef struct LeaksSession
{
	struct LeaksSession *	prevSession;
	char const *			name;
	PGPUInt32				allocationNumber;
	LeakItem				dummyItem;
} LeaksSession;

static DECLARE_LEAKDEALLOCTYPE(kSessionDummyDeallocType,
								"Session dummy dealloc");
static DECLARE_LEAKALLOCTYPE(kSessionDummyAllocType, "Session dummy alloc",
								kSessionDummyDeallocType);

/*
 * In order to avoid allocating memory until we have the first session
 * initialized, we use this static variable for the first session.
 */
static LeaksSession		sInitialSession = 
{
	NULL,										/* prevSession	*/
	"init",										/* name			*/
	0,
	{											/* dummyItem	*/
	kLeakItemMagic,								/*   magic		*/
	&LeakStructName(kSessionDummyAllocType),	/*   allocType	*/
	0,											/*   size		*/
	FALSE,										/*   ignore		*/
	&sInitialSession.dummyItem,					/*   prev		*/
	&sInitialSession.dummyItem,					/*   next		*/
	__FILE__,									/*   fileName	*/
	__LINE__,									/*   lineNumber	*/
	&sInitialSession							/*   memory		*/
	}
};

static LeaksSession *	sCurrentSession = &sInitialSession;
static int				sNumUnallocatedSessions = 0;
static int				sSuspendCount = 0;


	void
pgpLeaksSuspend(void)
{
	++sSuspendCount;
}

	void
pgpLeaksResume(void)
{
	pgpAssertMsg(sSuspendCount >= 1,
		"pgpLeaksResume: not currently suspended");
	--sSuspendCount;
}

	static inline PGPBoolean
pgpLeaksSuspended(void)
{
	return sSuspendCount != 0;
}

	static void
pgpLeaksIgnoreIfSuspended(
	LeakItem *		leak)
{
	if (pgpLeaksSuspended())
		pgpLeaksIgnoreItem(leak);
}

	static void
pgpLeaksLinkItem(
	LeakItem *		existingItem,
	LeakItem *		newItem)
{
	pgpa((
		pgpaLeakItemValid(existingItem),
		pgpaMsg("Invalid existing item")));
	newItem->prev			= existingItem;
	newItem->next			= existingItem->next;
	newItem->prev->next		= newItem;
	newItem->next->prev		= newItem;
	pgpa(pgpaLeakItemValid(newItem));
}

	static void
pgpLeaksUnlinkItem(
	LeakItem *		item)
{
	pgpa((
		pgpaLeakItemValid(item),
		pgpaLeakItemValid(item->prev),
		pgpaLeakItemValid(item->next),
		pgpaMsg("Invalid item or item links")));
	item->prev->next = item->next;
	item->next->prev = item->prev;
}

#define pgpaLeaksSessionValid(session)						\
		pgpaCall(pgpaInternalLeaksSessionValid, (pgpaCallPrefix, session))

#if PGP_DEBUG	/* [ */

	static PGPBoolean
pgpaInternalLeaksSessionValid(
	pgpaCallPrefixDef,
	const LeaksSession *	session)
{
	/*
	 * pgpLeaksNotifyItemMoved assumes this routine
	 * won't check the prev and next links
	 */
	pgpaAddrValid(session, LeaksSession);
	pgpaLeakItemValid(&session->dummyItem);
	pgpaAssert(session->dummyItem.allocType == kSessionDummyAllocType);
	pgpaMsg("pgpaLeaksSessionValid failed");

	return pgpaFailed;
}

#endif	/* ] PGP_DEBUG */

	static void
pgpLeaksInitLinkItem(
	LeakItem *			item,
	LeakAllocType *		allocType,
	void const *		memory,
	PGPUInt32				size,
	char const *		fileName,
	long				lineNumber)
{
	pgpa((
		pgpaAddrValid(item, LeakItem),
		pgpaLeakAllocTypeValid(allocType),
		pgpaAddrValid(memory, VoidAlign),
		pgpaStrValid(fileName),
		pgpaMsg("Invalid parameters")));
	
	item->magic			= kLeakItemMagic;
	item->allocType		= allocType;
	item->size			= size;
	item->ignore		= FALSE;
	item->prev			= item;
	item->next			= item;
	item->fileName		= fileName;
	item->lineNumber	= lineNumber;
	item->memory		= memory;
	/* monotonically increasing number, to identify a leak */

	item->allocationNumber	= sCurrentSession->allocationNumber++;
}

	void
pgpLeaksBeginSession(
	char const *		sessionName)
{
	LeaksSession *		newSession = NULL;
	
	pgpa((
		pgpaStrValid(sessionName),
		pgpaMsg("Invalid parameters")));
	
	/*
	 * If we can't allocate a new session, we just increment the
	 * sNumUnallocatedSessions counter, so we keep using the parent session
	 * instead.  In addition, if we're already in a session which couldn't
	 * be allocated (sNumUnallocatedSessions > 0), we won't even try to
	 * allocate this one, because it would be more complicated to keep track
	 * of than it's worth.
	 */
	if (sNumUnallocatedSessions <= 0)
		newSession = (LeaksSession *)pgpAlloc(sizeof(LeaksSession));
	
	if (newSession == NULL)
	{
		sNumUnallocatedSessions++;
		pgpDebugMsg("Unable to allocate new leaks session");
	}
	else
	{
		pgpClearMemory( newSession, sizeof( *newSession ) );
		
		newSession->prevSession			= sCurrentSession;
		newSession->name				= sessionName;
		newSession->allocationNumber	= 0;
		pgpLeaksInitLinkItem(&newSession->dummyItem, kSessionDummyAllocType,
							(void *)newSession, 0, __FILE__, __LINE__);
		sCurrentSession = newSession;
	}
}

	static PGPUInt32
pgpLeaksCountLeaks(
	const LeaksSession *		session)
{
	PGPUInt32				numLeaks = 0;
	LeakItem const *	item;
	
	pgpa((
		pgpaLeaksSessionValid(session),
		pgpaMsg("Invalid LeaksSession")));
	
	for (item = session->dummyItem.next; item != &session->dummyItem;
			item = item->next)
	{
		pgpa(pgpaLeakItemValid(item));
		if (!item->ignore)
			++numLeaks;
	}
	return numLeaks;
}

	static void
pgpLeaksShowSessionLeaks(
	const LeaksSession *		session)
{
	PGPUInt32				numLeaks;
	LeakItem const *	item;
		
	pgpa((
		pgpaLeaksSessionValid(session),
		pgpaMsg("Invalid LeaksSession")));
	
	numLeaks = pgpLeaksCountLeaks(session);
	if (numLeaks != 0)
	{
		pgpDebugFmtMsg((pgpaFmtPrefix,
						"Ending session %s with %lu leaks:",
						session->name, numLeaks));
	
		for (item = session->dummyItem.next; item != &session->dummyItem;
				item = item->next)
		{
			PGPBoolean	shouldShowLeak;
			
			pgpa(pgpaLeakItemValid(item));
			
			shouldShowLeak	= ! item->ignore;
		#if PGP_MACINTOSH
			shouldShowLeak	= shouldShowLeak &&
					(( GetAllModifiers() & alphaLock ) == 0 );
		#endif
			if ( shouldShowLeak )
			{
				pgpDebugFmtMsg((pgpaFmtPrefix,
								"LEAK ID=%ld at %p of %lu bytes from %s "
								"line %ld (%s), session %s ",
								(long)item->allocationNumber,
								item->memory, item->size,
								item->fileName, item->lineNumber,
								item->allocType->name, session->name ));
			}
		}
	}
}

	void
pgpLeaksEndSession(void)
{
	if (sNumUnallocatedSessions > 0)
		sNumUnallocatedSessions--;
	else
	{
		LeaksSession *	oldSession;
		
		pgpa(pgpaLeaksSessionValid(sCurrentSession));
		oldSession = sCurrentSession;
		sCurrentSession = oldSession->prevSession;
		pgpLeaksShowSessionLeaks(oldSession);
		if (oldSession != &sInitialSession)
			pgpFree(oldSession);
	}
}

	void
pgpLeaksRememberItem(
	LeakItem *			item,
	LeakAllocType *		type,
	void const *		memory,
	PGPUInt32				size,
	char const *		fileName,
	long				lineNumber)
{
	pgpa(pgpaLeaksSessionValid(sCurrentSession));
	
	pgpa((
		pgpaAddrValid(item, LeakItem),
		pgpaLeakAllocTypeValid(type),
		pgpaAddrValid(memory, VoidAlign),
		pgpaStrValid(fileName),
		pgpaMsg("Invalid parameters")));
	pgpa(pgpaLeaksSessionValid(sCurrentSession));
	pgpLeaksInitLinkItem(item, type, memory, size, fileName, lineNumber);
	pgpLeaksLinkItem(&sCurrentSession->dummyItem, item);
	
	pgpLeaksIgnoreIfSuspended(item);
}

	void
pgpLeaksIgnoreItem(
	LeakItem *		item)
{
	pgpa((
		pgpaLeakItemValid(item),
		pgpaMsg("Invalid parameters")));
	
	item->ignore = TRUE;
}

	void
pgpLeaksIgnoreMemory(
	const void *	memory)
{
	pgpAssertAddrValid(memory, VoidAlign);
	pgpLeaksIgnoreItem(pgpLeaksFindItem(memory));
}


void
pgpLeaksForgetItem(
	LeakItem *			item,
	LeakDeallocType *	deallocType)
{
	pgpa((
		pgpaLeakItemValid(item),
		pgpaLeakDeallocTypeValid(deallocType),
		pgpaMsg("Invalid parameters")));
	pgpa((
		pgpaAssert(item->allocType->deallocType == deallocType),
		pgpaFmtMsg((pgpaFmtPrefix,
			"Block deallocated incorrectly with %s instead of %s\n"
			"  from %s in %s line %ld",
			deallocType->name, item->allocType->deallocType->name,
			item->allocType->name, item->fileName, item->lineNumber))));
	pgpLeaksUnlinkItem(item);
}

	LeakItem *
pgpLeaksFindItem(
	void const *	memory)
{
	LeaksSession const *	session;
	LeakItem  *				item;

	pgpa((
		pgpaAddrValid(memory, VoidAlign),
		pgpaMsg("Invalid parameters")));
	for (session = sCurrentSession; session != NULL;
			session = session->prevSession)
	{
		pgpa(pgpaLeaksSessionValid(session));
		for (item = session->dummyItem.next; item != &session->dummyItem;
				item = item->next)
		{
			pgpa(pgpaLeakItemValid(item));
			if (item->memory == memory)
				return item;
		}
	}
	return NULL;
}

	void
pgpLeaksNotifyItemMoved(
	LeakItem const *	oldItem,
	LeakItem *			newItem)
{
	/* Assumes that the assertion won't check the links */
	pgpa((
		pgpaLeakItemValid(newItem),
		pgpaMsg("Invalid parameters")));
	
	if (newItem->prev == oldItem)
	{
		/* This is the only item in the list */
		newItem->prev = newItem->next = newItem;
	}
	else
	{
		newItem->prev->next = newItem;
		newItem->next->prev = newItem;
	}
}

#endif /* ] USE_PGP_LEAKS */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
