#include <stdio.h>
#include <string.h>
/*#include <sys/ddi.h>*/

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a) < (b) ? (b) : (a))
#endif

#include "pgpPubTypes.h"
#include "pgpUtilities.h"
#include "pgpErrors.h"
#include "pgpKeys.h"
#include "pgpRandomPool.h"
#include "pgpOptionList.h"
#include "pgpUserInterface.h"

/* prototypes */
void ReportError(PGPError, const char *);
void CheckPGPSPD(PGPContextRef);
void CheckPGPCRDD(PGPContextRef);

void
ReportError(PGPError err, const char *function)
{
	char errorStr[1024];

	PGPGetErrorString(err, 1024, errorStr);
	printf("%s error: %s\n", function, errorStr);
	memset(errorStr, 0, 1024);
}

/*
 * Test the PGPSigningPassphraseDialog() function call.
 *
 */
void
CheckPGPSPD(PGPContextRef cRef)
{
	PGPKeySetRef            keySet, singleKeySet;
	PGPKeyRef		key;
        PGPKeySetRef            keys = NULL;  
	PGPFilterRef            filter = NULL; 
        PGPKeyListRef           klist = NULL;
        PGPKeyIterRef           kiter = NULL; 
	char * pszPassphrase;
	PGPError err;
	
	err = PGPOpenDefaultKeyRings(cRef, kPGPKeyRingOpenFlags_Mutable, &keySet);
	if (IsPGPError(err))
		ReportError(err, "PGPOpenDefaultKeyRings()");
	
	err = PGPNewUserIDStringFilter(cRef, "test",
				       kPGPMatchSubString, &filter );
        if ( IsPGPError( err ) )
                ReportError(err, "PGPNewUserIDStringFilter()");

        err     = PGPFilterKeySet(keySet, filter, &keys);
        if ( IsPGPError( err ) )
                ReportError(err, "PGPFilterKeySet()");
        PGPFreeFilter( filter );
        filter  = NULL;

        PGPOrderKeySet(keys, kPGPAnyOrdering, &klist);
        PGPNewKeyIter(klist, &kiter);

        err = PGPKeyIterNext(kiter, &key);
        if ( IsPGPError( err ) )
                ReportError(err, "PGPKeyIterNext()");

        PGPFreeKeyIter( kiter );
        kiter = NULL;
        PGPFreeKeyList( klist );
        klist = NULL;
        PGPFreeKeySet( keys );
        keySet = NULL;
#if 0
	err = PGPGetDefaultPrivateKey(keySet, &key);
	if (IsPGPError(err))
		ReportError(err, "PGPGetDefaultPrivateKey()");
#endif
	
	err = PGPNewSingletonKeySet(key, &singleKeySet);
	if (IsPGPError(err))
		ReportError(err, "PGPNewSingletonKeySet()");
	
	err = PGPSigningPassphraseDialog(cRef, singleKeySet, &key,
								 PGPOUIOutputPassphrase(cRef, &pszPassphrase),
								 PGPOUIFindMatchingKey(cRef, 1),
								 PGPOLastOption(cRef));
	if (IsPGPError(err))
		ReportError(err, "PGPSigningPassphraseDialog()");
}

void
CheckPGPCRDD(PGPContextRef contextRef)
{
	PGPError pgpRC;
	PGPUInt32 minPoolSize, currentPoolSize;
	
	if (!PGPGlobalRandomPoolHasMinimumEntropy()) {
		currentPoolSize = PGPGlobalRandomPoolGetEntropy();
		minPoolSize  = PGPGlobalRandomPoolGetMinimumEntropy();
		
		pgpRC = PGPCollectRandomDataDialog(contextRef, 
								   max(600, 2 * minPoolSize - currentPoolSize),
								   PGPOLastOption(contextRef));
		if (IsPGPError(pgpRC)) 
			ReportError(pgpRC, "PGPCollectRandomDataDialog()");
		
		if (!PGPGlobalRandomPoolHasMinimumEntropy()) {
			printf("Still don't have enough entropy. This is bad.\n");
		} else {
			printf("Everything is hunky-dory\n");
		}
	} else
		printf("Don't need any entropy right now\n");
}	

		
int main()
{
	PGPContextRef contextRef;
	PGPError pgpRC;

	pgpRC = PGPsdkInit();
	if (IsPGPError(pgpRC)) 
		ReportError(pgpRC, "PGPsdkInit()");
	
	pgpRC = PGPNewContext(kPGPsdkAPIVersion, &contextRef);
	if (IsPGPError(pgpRC)) 
		ReportError(pgpRC, "PGPNewContext()");

	CheckPGPCRDD(contextRef); 
	
	CheckPGPSPD(contextRef);
	
	return 0;
}
	
 
