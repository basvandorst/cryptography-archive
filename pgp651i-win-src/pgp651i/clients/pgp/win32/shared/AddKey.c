/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: AddKey.c,v 1.2 1999/03/10 02:28:57 heller Exp $
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
#include "PGPcl.h"

// Project Headers
#include "AddKey.h"

PGPError AddKey(HWND hwnd, PGPContextRef context, 
				PGPtlsContextRef tlsContext, PGPOptionListRef options);


PGPError AddKeyBuffer(HWND hwnd, PGPContextRef context, 
					  PGPtlsContextRef tlsContext, void* pInput, 
					  DWORD dwInSize)
{
	PGPError			err;
	PGPOptionListRef	options;

	err = PGPBuildOptionList(context, &options,
			PGPOInputBuffer( context, (PGPByte *) pInput, dwInSize ),
			PGPOLastOption( context ) );

	if (IsPGPError(err))
		return err;

	err = AddKey(hwnd, context, tlsContext, options);

	PGPFreeOptionList(options);
	return err;
}


PGPError AddKeyFile(HWND hwnd, PGPContextRef context, 
					PGPtlsContextRef tlsContext, char *szInFile)
{
	PGPError			err;
	PGPFileSpecRef		inputFile;
	PGPOptionListRef	options;

	err = PGPNewFileSpecFromFullPath(context, szInFile, &inputFile);
	if (IsPGPError(err))
		return err;

	err = PGPBuildOptionList(context, &options,
			PGPOInputFile( context, inputFile ),
			PGPOLastOption( context ) );

	if (IsPGPError(err))
	{
		PGPFreeFileSpec(inputFile);
		return err;
	}

	err = AddKey(hwnd, context, tlsContext, options);

	PGPFreeFileSpec(inputFile);
	PGPFreeOptionList(options);
	return err;
}


PGPError AddKey(HWND hwnd, PGPContextRef context, 
				PGPtlsContextRef tlsContext, PGPOptionListRef options)
{
	PGPError			err;
	PGPKeySetRef		importKeySet;

	UpdateWindow(hwnd);

	err = PGPsdkLoadDefaultPrefs(context);
	if (IsPGPError(err))
		return err;

	err = PGPImportKeySet(context, &importKeySet,
			options,
			PGPOLastOption( context ) );

	if (IsPGPError(err))
		return err;

	if (importKeySet) 
	{
		PGPUInt32	numKeys;
		
		(void)PGPCountKeys(importKeySet, &numKeys );
		if ( numKeys > 0) 
		{
			err = PGPclQueryAddKeys(context, tlsContext, hwnd, importKeySet, 
					NULL);
		}

		PGPFreeKeySet(importKeySet);
	}
	
	return err;
}




/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
