typedef unsigned char uchar;

enum Command
    {
    ADD_TO_CACHE,
    GET_FROM_CACHE
    };

typedef struct _Cache
    {
    uchar *aucKey;
    int nKey;
    uchar *aucData;
    int nData;
    time_t tExpiresAt;
    } Cache;

void WriteThing(int nFD,uchar *aucThing,int nLength);
uchar *ReadThing(int nFD,int *pnLength);

Cache *LocalCacheFind(uchar *aucKey,int nLength);
Cache *LocalCacheAdd(uchar *aucKey,int nLength,time_t tExpiresAt);

void InitGlobalCache(const char *szPort);
void GlobalCacheAdd(uchar *aucKey,int nKey,uchar *aucData,
		int nData,time_t tExpiresAt);
uchar *GlobalCacheGet(uchar *aucKey,int nKey,int *pnData,time_t *ptExpiresAt);
