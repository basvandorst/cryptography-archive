/*
 * Bytefifo.c - an infinitely-expandable FIFO structure for bytes.
 *
 * $Id: pgpByteFIFO.c,v 1.20.4.1 1998/09/14 21:48:15 hal Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>	/* For BUFSIZ */
#include <string.h>

#include "pgpDebug.h"
#include "pgpByteFIFO.h"
#include "pgpMem.h"
#include "pgpUsuals.h"
#include "pgpContext.h"

#if (BUFSIZ < 16384) && (PGP_MACINTOSH || PGP_WIN32)
#define kPGPFIFOPageSize	16384
#else
#define kPGPFIFOPageSize	BUFSIZ
#endif

typedef struct ByteFifoPage {
	struct ByteFifoPage *next;
	PGPByte buf[kPGPFIFOPageSize-sizeof(struct ByteFifoPage *)];
	DEBUG_STRUCT_CONSTRUCTOR( ByteFifoPage )
} ByteFifoPage;

struct PGPFifoContext {
	PGPMemoryMgrRef	memoryMgr;
	ByteFifoPage *head, *tail;
	PGPByte *putp;
	PGPByte const *getp;
	PGPSize pages;
	PGPBoolean secure;
	DEBUG_STRUCT_CONSTRUCTOR( PGPFifoContext )
};

static void
byteFifoInit(
	PGPMemoryMgrRef		memoryMgr,
	PGPFifoContext *	fifo,
	PGPBoolean			secure)
{
	fifo->memoryMgr	= memoryMgr;
	fifo->head = 0;
	fifo->tail = 0;
	fifo->putp = 0;
	fifo->getp = 0;
	fifo->pages = 0;
	fifo->secure = secure;
}

void
byteFifoFlush(PGPFifoContext *fifo)
{
	ByteFifoPage *page;

	while ((page = fifo->head) != 0) {
		fifo->head = page->next;
		pgpClearMemory( page,  sizeof(page));
		PGPFreeData( page );
	}
	fifo->tail = 0;
	fifo->putp = 0;
	fifo->getp = 0;
	fifo->pages = 0;
}

PGPSize
byteFifoSize(PGPFifoContext const *fifo)
{
	PGPSize	space;
	if (!fifo->pages)
		return 0;
	space = (fifo->putp - fifo->tail->buf) +
		(fifo->head->buf + sizeof(fifo->head->buf) - fifo->getp);
	space += (fifo->pages-1) * sizeof(fifo->head->buf);
	return space - sizeof(fifo->head->buf);
}

PGPFifoContext *
byteFifoCreate( PGPContextRef context  )
{
	PGPFifoContext *fifo;
	PGPMemoryMgrRef	memoryMgr	= PGPGetContextMemoryMgr( context );

	fifo = (PGPFifoContext *)
		PGPNewData( memoryMgr, sizeof(*fifo), 0);
	if (fifo)
	{
		byteFifoInit( memoryMgr, fifo, FALSE);
	}
	return fifo;
}

PGPFifoContext *
byteFifoCreateSecure( PGPContextRef context  )
{
	PGPFifoContext *fifo;
	PGPMemoryMgrRef	memoryMgr	= PGPGetContextMemoryMgr( context );

	fifo = (PGPFifoContext *)
		PGPNewData( memoryMgr, sizeof(*fifo), 0);
	if (fifo)
	{
		byteFifoInit( memoryMgr, fifo, TRUE);
	}
	return fifo;
}

void
byteFifoDestroy(PGPFifoContext *fifo)
{
	byteFifoFlush(fifo);
	PGPFreeData( fifo );
}

PGPByte *
byteFifoGetSpace(PGPFifoContext *fifo, PGPSize *len)
{
	PGPSize avail;
	ByteFifoPage *page;

	if (fifo->head) {
		pgpAssert(fifo->putp);
		avail = fifo->tail->buf + sizeof(fifo->tail->buf) - fifo->putp;
		if (avail) {
			*len = avail;
			return fifo->putp;
		}
	}
	if (fifo->secure) {
		page = (ByteFifoPage *)
			PGPNewSecureData( fifo->memoryMgr, sizeof(ByteFifoPage), 0);
	} else {
		page = (ByteFifoPage *)
			PGPNewData( fifo->memoryMgr, sizeof(ByteFifoPage), 0);
	}
		
	if (!page) {	/* Out of memory */
		*len = 0;
		return 0;
	}
	fifo->pages++;
	if (fifo->head) {
		pgpAssert(fifo->pages > 1);
		fifo->tail->next = page;
	} else {
		pgpAssert(fifo->pages == 1);
		fifo->head = page;
		fifo->getp = page->buf;
	}
	fifo->tail = page;
	page->next = 0;
	*len = sizeof(fifo->head->buf);
	return fifo->putp = page->buf;
}

void
byteFifoSkipSpace(PGPFifoContext *fifo, PGPSize len)
{
	if (!len)
		return;
	
	pgpAssert(fifo->head);
	pgpAssert(fifo->putp);
	pgpAssert(len <= (PGPSize)(fifo->tail->buf + sizeof(fifo->tail->buf)
	                         - fifo->putp));

	fifo->putp += len;
}

PGPByte const *
byteFifoPeek(PGPFifoContext *fifo, PGPSize *len)
{
	if (!fifo->head) {
		*len = 0;
		return 0;	/* No bytes available */
	}
	pgpAssert(fifo->getp);
	/* Head and tail on same page? */
	if (fifo->tail == fifo->head)
		*len = fifo->putp - fifo->getp;
	else
		*len = fifo->head->buf + sizeof(fifo->head->buf) - fifo->getp;
	return *len ? fifo->getp : 0;
}

void
byteFifoSeek(PGPFifoContext *fifo, PGPSize len)
{
	ByteFifoPage *page;

	if (!len)
		return;

	pgpAssert(fifo->head);
	pgpAssert(len <= (PGPSize)(fifo->head->buf + sizeof(fifo->head->buf)
	                         - fifo->getp));

	if (fifo->head == fifo->tail) {
		pgpAssert(fifo->pages == 1);
		pgpAssert(fifo->head->next == 0);
		pgpAssert(len <= (PGPSize)(fifo->putp - fifo->getp));
		if (len < (PGPSize)(fifo->putp - fifo->getp)) {
			fifo->getp += len;
		} else {
			pgpClearMemory(fifo->head, sizeof(*fifo->head));
			PGPFreeData( fifo->head);
			byteFifoInit( fifo->memoryMgr, fifo, fifo->secure);
		}
		return;
	}

	pgpAssert(fifo->pages > 1);

	if (len < (PGPSize)(fifo->head->buf + sizeof(fifo->head->buf)
	                     - fifo->getp))
	{
		fifo->getp += len;
	} else {
		page = fifo->head;
		fifo->head = fifo->head->next;
		pgpAssert(fifo->head);
		pgpClearMemory( page,  sizeof(*page));
		PGPFreeData( page);	
		fifo->pages--;
		pgpAssert(fifo->pages);
		fifo->getp = fifo->head->buf;
	}
}

PGPSize
byteFifoWrite(PGPFifoContext *fifo, PGPByte const *buf, PGPSize len)
{
	PGPSize avail;
	PGPByte *ptr;
	PGPSize len0 = len;

	while (len) {
		ptr = byteFifoGetSpace(fifo, &avail);
		if (!ptr)
			return len0 - len;
		if (avail > len)
			avail = len;
		memcpy(ptr, buf, avail);
		byteFifoSkipSpace(fifo, avail);
		buf += avail;
		len -= avail;
	}
	return len0;
}

PGPSize
byteFifoRead(PGPFifoContext *fifo, PGPByte *buf, PGPSize len)
{
	PGPSize avail;
	PGPByte const *ptr;
	PGPSize len0 = len;

	while (len) {
		ptr = byteFifoPeek (fifo, &avail);
		if (!ptr)
			return len0 - len;
		if (avail > len)
			avail = len;
		memcpy (buf, ptr, avail);
		buf += avail;
		len -= avail;
		byteFifoSeek (fifo, avail);
	}
	return len0;
}

#if UNITTEST
/* Test driver */
#include <stdio.h>

static void
byteFifoDump(PGPFifoContext *fifo)
{
	PGPByte const *ptr;
	PGPSize avail;

	while ((ptr = byteFifoPeek(fifo, &avail)) != 0) {
		fwrite(ptr, 1, avail, stdout);
		byteFifoSeek(fifo, avail);
	}
}

int
main(int argc, char **argv)
{
	PGPFifoContext fifo;
	PGPByte const c = ' ';
	int i, j;

	byteFifoInit(&fifo);

	for (i = 1; i < argc; i++) {
		for (j = 1; j <= i; j++) {
			byteFifoWrite(&fifo, argv[j], strlen(argv[j]));
			if (i != j)
				byteFifoWrite(&fifo, &c, 1);
		}
		printf("argv[1..%d] = (%lu) \"", i, byteFifoSize(&fifo));
		byteFifoDump(&fifo);
		puts("\"");
	}
	return 0;
}

#endif
