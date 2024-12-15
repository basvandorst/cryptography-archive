/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*4514 is "unreferenced inline func"*/
#pragma warning (disable : 4214 4201 4115 4514)
#include <windows.h>
#pragma warning (default : 4214 4201 4115)
#include <assert.h>

#include "define.h"
#include "..\include\config.h"
#include "..\include\pgpkeydb.h"
#include "..\include\pgpcomdlg.h"

UINT EnclyptorAddKey(HWND hwnd, void* pInput, DWORD dwInSize)
 {
	UINT ReturnValue = STATUS_FAILURE;
	PGPKeySet *RingSet;
	PGPKeySet *NewKeySet;
	PGPError error;

	if(hwnd)
		; /*Avoid W4 Warning*/

	assert(pInput);

     pgpLibInit();

     RingSet = pgpOpenDefaultKeyRings(TRUE, NULL);

	if(RingSet)
	{
		NewKeySet = pgpImportKeyBuffer((BYTE*)pInput, dwInSize);

		if(NewKeySet)
		{
#if 1	//>>> pre-existing code
			error = pgpAddKeys(RingSet, NewKeySet);
#else	//>>> PGPcomdlgQueryAddKeys changes ...
			error = PGPcomdlgQueryAddKeys(hwnd, RingSet, NewKeySet);
#endif	//<<< PGPcomdlgQueryAddKeys changes ...

			if(error == PGPERR_OK)
			{
				error = pgpCommitKeyRingChanges(RingSet);
				
				if(error == PGPERR_OK)
				{
					ReturnValue = STATUS_SUCCESS;
					PGPcomdlgNotifyKeyringChanges(GetCurrentProcessId());
 	 	 	 	 }
 	 	 	 }

 	 	 	 pgpFreeKeySet(NewKeySet);
 	 	 }

 	 	 pgpFreeKeySet(RingSet);
 	 }

 	 pgpLibCleanup();

 	 return ReturnValue;
 }
