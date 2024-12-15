/*
* pgpByteFIFO.c - an infinitely-expandable FIFO structure for bytes.
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpByteFIFO.c,v 1.3.2.1 1997/06/07 09:49:59 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>	/* For BUFSIZ */

#include "pgpDebug.h"
#include "pgpByteFIFO.h"
#include "pgpMem.h"
#include "pgpUsuals.h"

#if (BUFSIZ < 16384) && (MACINTOSH || WIN32)
#define kPGPFIFOPageSize	16384
#else
#define kPGPFIFOPageSize	BUFSIZ
#endif

struct ByteFifoPage {
 struct ByteFifoPage *next;
 byte buf[kPGPFIFOPageSize-sizeof(struct ByteFifoPage *)];
};

struct PgpFifoContext {
		struct ByteFifoPage *head, *tail;
		byte *putp;
		byte const *getp;
		unsigned pages;
	};

static void
byteFifoInit(struct PgpFifoContext *fifo)
	{
		fifo->head = 0;
		fifo->tail = 0;
		fifo->putp = 0;
		fifo->getp = 0;
		fifo->pages = 0;
	}

	void
byteFifoFlush(struct PgpFifoContext *fifo)
	{
		struct ByteFifoPage *page;

		while ((page = fifo->head) != 0) {
		 fifo->head = page->next;
		 memset(page, 0, sizeof(page));
		 pgpMemFree(page);
		}
		fifo->tail = 0;
		fifo->putp = 0;
 fifo->getp = 0;
 fifo->pages = 0;
}

unsigned long
byteFifoSize(struct PgpFifoContext const *fifo)
{
unsigned long space;
if (!fifo->pages)
 return 0;
space = (fifo->putp - fifo->tail->buf) +
 (fifo->head->buf + sizeof(fifo->head->buf) - fifo->getp);
space += (fifo->pages-1) * sizeof(fifo->head->buf);
return space - sizeof(fifo->head->buf);
}

struct PgpFifoContext *
byteFifoCreate(void)
{
		struct PgpFifoContext *fifo;

		fifo = (struct PgpFifoContext *)pgpMemAlloc(sizeof(*fifo));
		if (fifo)
		 byteFifoInit(fifo);
		return fifo;
	}

void
byteFifoDestroy(struct PgpFifoContext *fifo)
	{
		byteFifoFlush(fifo);
		pgpMemFree(fifo);
	}

byte *
byteFifoGetSpace(struct PgpFifoContext *fifo, unsigned *len)
	{
		unsigned avail;
		struct ByteFifoPage *page;

		if (fifo->head) {
			pgpAssert(fifo->putp);
			avail = fifo->tail->buf + sizeof(fifo->tail->buf) - fifo->putp;
			if (avail) {
			 *len = avail;
			 return fifo->putp;
			}
		}
		page = (struct ByteFifoPage *)pgpMemAlloc(sizeof(struct ByteFifoPage));
		if (!page) { /* Out of memory */
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
byteFifoSkipSpace(struct PgpFifoContext *fifo, unsigned len)
	{
		if (!len)
		 return;

		pgpAssert(fifo->head);
		pgpAssert(fifo->putp);
		pgpAssert(len <= (unsigned)(fifo->tail->buf + sizeof(fifo->tail->buf)
		- fifo->putp));

		fifo->putp += len;
	}

byte const *
byteFifoPeek(struct PgpFifoContext *fifo, unsigned *len)
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
byteFifoSeek(struct PgpFifoContext *fifo, unsigned len)
	{
		struct ByteFifoPage *page;

if (!len)
 return;

		pgpAssert(fifo->head);
		pgpAssert(len <= (unsigned)(fifo->head->buf + sizeof(fifo->head->buf)
		- fifo->getp));

		if (fifo->head == fifo->tail) {
			pgpAssert(fifo->pages == 1);
			pgpAssert(fifo->head->next == 0);
			pgpAssert(len <= (unsigned)(fifo->putp - fifo->getp));
			if (len < (unsigned)(fifo->putp - fifo->getp)) {
			 fifo->getp += len;
			} else {
				memset(fifo->head, 0, sizeof(*fifo->head));
				pgpMemFree(fifo->head);
				byteFifoInit(fifo);
			}
			return;
		}

		pgpAssert(fifo->pages > 1);

		if (len < (unsigned)(fifo->head->buf + sizeof(fifo->head->buf)
		- fifo->getp))
		{
		 fifo->getp += len;
		} else {
			page = fifo->head;
			fifo->head = fifo->head->next;
			pgpAssert(fifo->head);
			memset(page, 0, sizeof(*page));
			pgpMemFree(page);
			fifo->pages--;
			pgpAssert(fifo->pages);
			fifo->getp = fifo->head->buf;
		}
	}

size_t
byteFifoWrite(struct PgpFifoContext *fifo, byte const *buf, size_t len)
	{
		unsigned avail;
		byte *ptr;
		size_t len0 = len;

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

size_t
byteFifoRead(struct PgpFifoContext *fifo, byte *buf, size_t len)
	{
		unsigned avail;
		byte const *ptr;
		size_t len0 = len;

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
byteFifoDump(struct PgpFifoContext *fifo)
	{
		byte const *ptr;
		unsigned avail;

while ((ptr = byteFifoPeek(fifo, &avail)) != 0) {
		fwrite(ptr, 1, avail, stdout);
		byteFifoSeek(fifo, avail);
	}
}

int
main(int argc, char **argv)
	{
		struct PgpFifoContext fifo;
		byte const c = ' ';
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
