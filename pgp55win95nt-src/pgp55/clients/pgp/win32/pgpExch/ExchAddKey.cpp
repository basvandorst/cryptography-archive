/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: ExchAddKey.cpp,v 1.13 1997/10/02 16:41:55 dgal Exp $
____________________________________________________________________________*/
#include <windows.h>

#include "stdinc.h"
#include "pgpExch.h"
#include "resource.h"
#include "UIutils.h"

#include "pgpConfig.h"
#include "pgpEncode.h"
#include "pgpMem.h"
#include "pgpUtilities.h"
#include "pgpKeys.h"
#include "pgpSDKPrefs.h"
#include "PGPcmdlg.h"
#include "PGPphras.h"
#include "PGPkm.h"


PGPError AddKeyBuffer(HWND hwnd, PGPContextRef context, void* pInput, 
					  DWORD dwInSize, BOOL bAutoAddKeys)
{
	PGPError			err;
	PGPKeySetRef		importKeySet;

	UpdateWindow(hwnd);

	err = PGPsdkLoadDefaultPrefs(context);
	if (IsPGPError(err))
		return err;

	err= PGPImportKeySet(context, &importKeySet,
			PGPOInputBuffer( context, (PGPByte *) pInput, dwInSize ),
			PGPOLastOption( context ) );

	if (IsPGPError(err))
	{
		return err;
	}

	if (importKeySet) 
	{
		PGPUInt32	numKeys;
		
		(void)PGPCountKeys(importKeySet, &numKeys );
		if ( numKeys > 0) 
		{
			err = PGPkmQueryAddKeys(context, hwnd, importKeySet, NULL);
		}
		else
			UIDisplayStringID(hwnd, IDS_E_NOKEYS);

		PGPFreeKeySet(importKeySet);
	}
	else
		UIDisplayStringID(hwnd, IDS_E_NOKEYS);
	
	return err;
}


PGPError AddKeyFile(HWND hwnd, PGPContextRef context, char *szInFile, 
					BOOL bAutoAddKeys)
{
	PGPError			err;
	PGPKeySetRef		importKeySet;
	PGPFileSpecRef		inputFile;

	UpdateWindow(hwnd);

	PGPNewFileSpecFromFullPath(context, szInFile, &inputFile);

	err = PGPsdkLoadDefaultPrefs(context);
	if (IsPGPError(err))
		return err;

	err= PGPImportKeySet(context, &importKeySet,
			PGPOInputFile( context, inputFile ),
			PGPOLastOption( context ) );

	if (IsPGPError(err))
	{
		return err;
	}

	if (importKeySet) 
	{
		PGPUInt32	numKeys;
		
		(void)PGPCountKeys(importKeySet, &numKeys );
		if (numKeys > 0) 
		{
			err = PGPkmQueryAddKeys(context, hwnd, importKeySet, NULL);
		}
		else
			UIDisplayStringID(hwnd, IDS_E_NOKEYS);

		PGPFreeKeySet(importKeySet);
	}
	else
		UIDisplayStringID(hwnd, IDS_E_NOKEYS);
	
	return err;
}




/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
