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
	    free(pCache[n].aucKey);
	    free(pCache[n].aucData);
	    }
    }

Cache *LocalCacheFind(uchar *aucKey,int nLength)
    {
    int n;
    static int nCalls;

    if(++nCalls == 1)
	{
	nCalls=0;
	ExpireCache();
	}

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
