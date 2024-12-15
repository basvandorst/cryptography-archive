/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc. 
	All rights reserved.
	
	

	$Id: AddKey.c,v 1.1.2.1 1997/11/14 06:19:41 elrod Exp $
____________________________________________________________________________*/
 
// System Headers 
#include <windows.h>
#include <windowsx.h>
#include <assert.h>

// PGPsdk Headers
#include "pgpConfig.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUtilities.h" 
#include "pgpSDKPrefs.h"

// Shared Headers
#include "PGPcmdlg.h"
#include "PGPkm.h"

// Project Headers
#include "AddKey.h"

// Global Variables
extern PGPContextRef	g_pgpContext;


PGPError AddKey(char* szBuffer, DWORD dwInSize, long flags)
{
	PGPKeySetRef	DefaultRingSet	= kPGPInvalidRef;
	PGPKeySetRef	NewKeySet		= kPGPInvalidRef;
	PGPFileSpecRef	FileRef			= kPGPInvalidRef;
	PGPError		error			= kPGPError_NoErr;

	assert(szBuffer);

	
	if( flags & KEY_IN_FILE )
	{
		error = PGPNewFileSpecFromFullPath(	g_pgpContext,
											szBuffer,
											&FileRef);

		if( IsntPGPError(error) )
		{
			error = PGPImportKeySet(
				g_pgpContext,
				&NewKeySet,
				PGPOInputFile( g_pgpContext, FileRef),
				PGPOLastOption(g_pgpContext) );
			PGPFreeFileSpec(FileRef);
		}
	} 
	else if( flags & KEY_IN_BUFFER )
	{
		error = PGPImportKeySet(
			g_pgpContext,
			&NewKeySet,
			PGPOInputBuffer( g_pgpContext, szBuffer, dwInSize),
			PGPOLastOption(g_pgpContext) );
	}
	else
	{
		error = kPGPError_BadParams;
	}

	//MessageBox(NULL, szBuffer, "szBuffer", MB_OK);

	if( NewKeySet && IsntPGPError(error) )
	{
		PGPUInt32	numKeys;
		
		error = PGPCountKeys( NewKeySet, &numKeys);
		
		// make sure there are keys in this Set
		if( IsntPGPError(error) && numKeys > 0 ) 
		{
			error = PGPkmQueryAddKeys (	g_pgpContext, 
										NULL,
										NewKeySet, 
										NULL);
			if( IsntPGPError(error) )
			{
				MessageBox(	NULL, 
							"PGP successfully added the key(s) to your keyring.", 
							"PGP Eudora - Extract Key", 
							MB_OK);
			}
		}
		else
		{
			MessageBox(	NULL, 
						"No keys were found.", 
						"PGP Eudora - Extract Key", 
						MB_OK);
		}

		PGPFreeKeySet(NewKeySet);
	}

	return error;
}
