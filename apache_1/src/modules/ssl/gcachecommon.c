/* ====================================================================
 * Copyright (c) 1995, 1996, 1997, 1998 Ben Laurie.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by Ben Laurie
 *    for use in the Apache-SSL HTTP server project."
 *
 * 4. The name "Apache-SSL Server" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Ben Laurie
 *    for use in the Apache-SSL HTTP server project."
 *
 * THIS SOFTWARE IS PROVIDED BY BEN LAURIE ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL BEN LAURIE OR
 * HIS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of patches to the Apache HTTP server interfacing it
 * to SSLeay.
 * For more information on Apache-SSL see http://www.apache-ssl.org.
 *
 * For more information on Apache see http://www.apache.org.
 *
 * For more information on SSLeay see http://www.psy.uq.oz.au/~ftp/Crypto/.
 *
 * Ben Laurie <ben@algroup.co.uk>.
 *
 */

#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <stdio.h>
#include "gcache.h"

#define INIT_CACHE_SIZE	1000

void WriteThing(int nFD,uchar *aucThing,int nLength)
    {
    static unsigned short usLength;

    usLength=(unsigned short)nLength;
    write(nFD,&usLength,sizeof usLength);
    write(nFD,aucThing,usLength);
    }

static int saferead(int nFD,void *pBuf,int nLength)
    {
    int n;
    char *acBuf=pBuf;

    for(n=0 ; n < nLength ; )
	{
	int r;

	r=read(nFD,acBuf+n,nLength-n);
	if(r < 0)
	    {
	    perror("read in saferead");
	    return r;
	    }
	if(r == 0)
	    {
	    fprintf(stderr,"EOF in saferead after %d bytes\n",n);
	    return n;
	    }
	n+=r;
	}

    return n;
    }

uchar *ReadThing(int nFD,int *pnLength)
    {
    static unsigned short usLength;
    uchar *aucThing;
    int nRead;

    nRead=saferead(nFD,&usLength,sizeof usLength);
    assert(nRead == sizeof usLength);
    if(!usLength)
	{
	*pnLength=0;
	return NULL;
	}
    aucThing=malloc(usLength);
    assert(aucThing);
    nRead=saferead(nFD,aucThing,usLength);
    assert(nRead == usLength);

    *pnLength=usLength;
    return aucThing;
    }

static Cache *pCache;
static int nCache;
static int nCacheMax;

static void ExpireCache()
    {
    int n,m;
    time_t tNow=time(NULL);

    for(n=m=0 ; n < nCache ; ++m)
	if(pCache[m].tExpiresAt >= tNow)
	    {
	    pCache[n]=pCache[m];
	    ++n;
	    }
	else
	    {
	    --nCache;
	    free(pCache[m].aucKey);
	    free(pCache[m].aucData);
	    }
    }

Cache *LocalCacheFind(uchar *aucKey,int nLength)
    {
    int n;

    ExpireCache();

    for(n=0 ; n < nCache ; ++n)
	if(pCache[n].nKey == nLength
	   && !memcmp(pCache[n].aucKey,aucKey,nLength))
	    return &pCache[n];
    return NULL;
    }

Cache *LocalCacheAdd(uchar *aucKey,int nKey,time_t tExpiresAt)
    {
    int n;

    ExpireCache();
    n=nCache++;
    if(n >= nCacheMax)
	{
	if(nCacheMax)
	    {
	    nCacheMax*=2;
	    pCache=realloc(pCache,nCacheMax*sizeof *pCache);
	    }
	else
	    {
	    nCacheMax=INIT_CACHE_SIZE;
	    pCache=malloc(nCacheMax*sizeof *pCache);
	    }
	assert(pCache);
	}
	    
    pCache[n].aucKey=aucKey;
    pCache[n].nKey=nKey;
    pCache[n].tExpiresAt=tExpiresAt;

    return &pCache[n];
    }
