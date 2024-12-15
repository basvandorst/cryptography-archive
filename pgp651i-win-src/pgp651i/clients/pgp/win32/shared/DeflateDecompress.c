/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.

	$Id: DeflateDecompress.c,v 1.1 1999/02/19 17:09:50 wjb Exp $
____________________________________________________________________________*/
#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include "zlib.h"
#include "DeflateWrapper.h"

#define OUTSIZE		512

static void *sAllocMem(void *dummy, unsigned int nItems, unsigned int nSize);
static void sFreeMem(void *dummy, void *address);


void Deflate_Decompress(void *pUserValue)
{
	z_stream	myStream;
	int nIn;
	char szIn[2];
	char szOut[OUTSIZE];
	unsigned int nIndex;
	int nErr = Z_OK;

	myStream.zalloc = sAllocMem;
	myStream.zfree = sFreeMem;
	myStream.opaque = NULL;

	inflateInit(&myStream);

	myStream.next_in = szIn;
	myStream.next_out = szOut;
	myStream.avail_out = OUTSIZE;

	nIn = getc_buffer(pUserValue);
	szIn[0] = nIn;

	while (nErr == Z_OK)
	{
		if (nIn != EOF)
		{
			myStream.avail_in = 1;
			nErr = inflate(&myStream, Z_NO_FLUSH);

			if ((nErr == Z_OK) && (!myStream.avail_in))
			{
				nIn = getc_buffer(pUserValue);
				szIn[0] = nIn;
				myStream.next_in = szIn;
			}
		}
		else
			nErr = inflate(&myStream, Z_FINISH);

		if ((nErr == Z_OK) && (!myStream.avail_out))
		{
			nIndex = 0;
			while (nIndex < OUTSIZE)
				putc_buffer(szOut[nIndex++], pUserValue);
		
			myStream.next_out = szOut;
			myStream.avail_out = OUTSIZE;
		}
	}

	if (nErr == Z_STREAM_END)
	{
		nIndex = 0;
		while (nIndex < (OUTSIZE - myStream.avail_out))
			putc_buffer(szOut[nIndex++], pUserValue);
	}

	return;
}


static void *sAllocMem(void *dummy, unsigned int nItems, unsigned int nSize)
{
	return calloc(nItems, nSize);
}


static void sFreeMem(void *dummy, void *address)
{
	free(address);
	return;
}


void pgpCopyMemory(void const *src, void *dest, size_t size)
{
	memcpy(dest, src, size);
	return;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
