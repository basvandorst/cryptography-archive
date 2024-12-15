#include <stdio.h>
#include <windows.h>
#include "PGPKeys.h"
#include "pgpKeyServer.h"
#include "pgpKeyServerTypes.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpEncode.h"
#include "pgpUtilities.h"
#include "pgpphrsx.h"


PGPError EncryptSignEvent(PGPContextRef context, 
						  PGPEvent *event, 
						  PGPUserValue userValue);

int main( int argc, char **argv )
{
	PGPContextRef	context   = NULL;
	PGPKeySetRef	keys      = NULL;
	PGPKeySetRef	keysError = NULL;
	PGPKeyServerRef	server    = NULL;
	PGPFileSpecRef	fileSpec;
	PGPError		err;
	int				rc;
	int				i;
	char			*hostName = NULL;
	char			*addFile  = NULL;
	char			*deactivate = NULL;
	PGPBoolean		bQuery = FALSE;
	PGPBoolean		bMonitor = FALSE;
	PGPFilterRef 	outFilter;

	if (argc < 2)
	{
		printf("ldaptest [server]\n");
		return(-1);
	}
	
	PGPKeyServerInit();

	if (IsPGPError(err=PGPNewContext(kPGPsdkAPIVersion, &context)))
		goto done;

	for ( i = 1; i < argc; i++ )
	{
		if (!stricmp(argv[i], "-h") && i < argc)
		{
			hostName = argv[i+1];
		}
		else if (!stricmp(argv[i], "-a") && i < argc)
		{
			addFile = argv[i+1];
		}
		else if (!stricmp(argv[i], "-q"))
		{
			bQuery = TRUE;
		}
		else if (!stricmp(argv[i], "-x") && i < argc)
		{
			deactivate = argv[i+1];
		}
		else if (!stricmp(argv[i], "-m"))
		{
			bMonitor = TRUE;
		}

	}

	if (!hostName)
	{
		printf("Missing hostname\n");
		goto done;
	}

	err = PGPNewKeyServerFromURL( context, 
						hostName, 
						kPGPKSAccess_Administrator,
						kPGPKSKeySpaceNormal,
						&server );

	if (IsPGPError(err))
	{
		goto done;
	}

	if (addFile)
	{
		err = PGPNewFileSpecFromFullPath( context, addFile, 
					&fileSpec);

		if (err != kPGPError_NoErr) {
			printf("Error creating FileSpec %s\n", addFile);
			exit(-1);
		}

		err = PGPOpenKeyRing (context,
						kPGPKeyRingOpenFlags_Mutable,
						fileSpec,
						&keys);

		if (err != kPGPError_NoErr) {
			printf("Error opening %s\n", addFile);
			exit(-1);
		}
		err = PGPUploadToKeyServer( server, 
									keys, 
									NULL, NULL,
									&keysError);
		if (IsPGPError(err))
		{
			char		errorString[255];

			if (keysError)
			{
				PGPUInt32	numKeys;
				
				(void)PGPCountKeys( keysError, &numKeys );
				printf( "%d count\n", numKeys); 
			}
			PGPGetErrorString( err, 255, errorString );
			printf("upload keys failed: 0x%x %s\n", err, errorString);
			PGPFreeKeySet( keys );
			PGPFreeKeySet( keysError );
			PGPFreeFileSpec( fileSpec );
			goto done;
		}
		PGPFreeFileSpec( fileSpec );
		PGPFreeKeySet( keys );
	}
	if (bQuery)
	{
		PGPKeyIDRef keyId;

		PGPNewKeyIDFromString( context, "0xC6EB854A6EADF0CA", &keyId);
		PGPNewSubKeyIDFilter( context, keyId, &outFilter);

		rc = PGPQueryKeyServer( server, outFilter, NULL, NULL, &keys );
		if (IsntPGPError(rc))
		{
			PGPFreeKeySet(keys);	
		}
		PGPFreeFilter( outFilter );
		PGPFreeKeyID( keyId );
	}
	if (deactivate)
	{
		err = PGPNewFileSpecFromFullPath( context, deactivate, 
					&fileSpec);

		if (err != kPGPError_NoErr) {
			printf("Error creating FileSpec %s\n", addFile);
			exit(-1);
		}

		err = PGPOpenKeyRing (context,
						kPGPKeyRingOpenFlags_Mutable,
						fileSpec,
						&keys);

		if (err != kPGPError_NoErr) {
			printf("Error opening %s\n", deactivate);
			exit(-1);
		}
		err = PGPDeleteFromKeyServer( server, 
								  keys, 
								  EncryptSignEvent, NULL,
								  &keysError);

		if (err != kPGPError_NoErr)
		{
			if (keysError)
			{
				PGPUInt32	numKeys;
				
				(void)PGPCountKeys( keysError, &numKeys );
				printf( "%d count\n", numKeys); 
			}
			printf("deactivate keys failed: 0x%x\n", err);
			exit(-1);
		}

	}
	if (bMonitor == TRUE)
	{
		PGPKeyServerMonitorRef monitor;

		err = PGPLDAPNewServerMonitor( server, NULL, NULL, &monitor );

		if (IsntPGPError(err))
		{
			PGPKeyServerMonitorRef	currentMonitor	= NULL;
			PGPKeyServerMonitorRef	nextMonitor		= NULL;
			char					**value			= NULL;
			char					**currentValue	= NULL;

			currentMonitor = monitor;

			while ( currentMonitor != NULL )
			{
				nextMonitor = currentMonitor->next;
				value = currentMonitor->monitorValues;
				currentValue = value;

				while ( *currentValue != NULL )
				{
					printf("%s = %s\n",
						currentMonitor->monitorTag, *currentValue);
					currentValue++;
				}
				currentMonitor = nextMonitor;
			}

			err = PGPLDAPFreeServerMonitor( server, monitor );
		}
	}

done:

	if ( server != NULL )
	{
		PGPFreeKeyServer(server);
	}
	PGPKeyServerCleanup();
	if ( context != NULL )
	{
		PGPFreeContext(context);
	}
	return(0);
}

PGPError EncryptSignEvent(PGPContextRef context, 
						  PGPEvent *event, 
						  PGPUserValue userValue)
{
	HWND			hwnd			= NULL;
	PGPOptionListRef	tempOptions		= NULL;
	static char *	szPassPhrase	= NULL;
	static BOOL		bAlreadyAsked	= FALSE;
	char *			szPrompt		= NULL;
	char *			szPassphrase	= NULL;
	PGPKeySetRef	pubKeySet		= NULL;
	PGPKeySetRef	signKeySet		= NULL;
	PGPKeyRef		signKey			= NULL;
	PGPError		err				= kPGPError_NoErr;
	int				nPassphraseLen	= 0;
	BOOL			bGotPassphrase	= FALSE;

	pgpAssert(PGPRefIsValid(context));
	pgpAssert(event != NULL);

	hwnd = (HWND) userValue;

	switch (event->type)
	{
	case kPGPEvent_KeyServerSignEvent:

		err = PGPOpenDefaultKeyRings(context, FALSE, &pubKeySet);

		err = PGPGetDefaultPrivateKey(pubKeySet, &signKey);
		if (err == kPGPError_ItemNotFound)
		{
			PGPKeyListRef	pubKeyList = NULL;
			PGPKeyIterRef	pubKeyIter = NULL;
			
			PGPOrderKeySet(pubKeySet, kPGPTrustOrdering, &pubKeyList);
			PGPNewKeyIter(pubKeyList, &pubKeyIter);
			PGPKeyIterNext(pubKeyIter, &signKey);
			PGPFreeKeyIter(pubKeyIter);
			PGPFreeKeyList(pubKeyList);
		}
		else if (IsPGPError(err))
		{
			goto done;
		}

		szPrompt = "Please enter your passphrase:";

		err = PGPGetSignCachedPhrase(context, hwnd, 
				szPrompt, TRUE, &szPassphrase, 
				pubKeySet, &signKey, NULL, NULL, 0);
		
		if (IsPGPError(err))
		{
			goto done;
		}
		
		bGotPassphrase = TRUE;
		nPassphraseLen = strlen(szPassphrase);
		
		err = PGPAddJobOptions(event->job,
				PGPOSignWithKey(context, 
					signKey, 
					PGPOPassphraseBuffer(context,
						szPassphrase, 
						nPassphraseLen),
					PGPOLastOption(context)),
				PGPOClearSign(context, TRUE),
				PGPOLastOption(context));
		
		if (IsPGPError(err))
		{
			goto done;
		}
		break;

	default:
		break;

	}

done:

	return err;
}


