/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#include <windows.h>

#include <assert.h>



#include "..\include\config.h"

#include "..\include\pgpkeydb.h"

#include "..\include\pgpcomdlg.h"



#include "define.h"

#include "ShellAddKey.h"



UINT ShellAddKey(HWND hwnd, char* szFile, BOOL DisplayError)

{

	UINT ReturnValue = STATUS_FAILURE;

	PGPKeySet *RingSet;

	PGPKeySet *NewKeySet;

	PGPFileRef* fileRef;

	PGPError error = PGPERR_OK;



	assert(szFile);



    pgpLibInit();


    RingSet = pgpOpenDefaultKeyRings(TRUE, NULL);




	if(RingSet)

	{

		fileRef = pgpNewFileRefFromFullPath(szFile);



		if(fileRef)

		{

			NewKeySet = pgpImportKeyFile(fileRef);



			if(NewKeySet)

			{

#if 1	//>>> pre-existing code
				error = pgpAddKeys(RingSet, NewKeySet);
#else	//>>> PGPcomdlgQueryAddKeys changes ...
				error = PGPcomdlgQueryAddKeys(hwnd, RingSet, NewKeySet);
				ReturnValue = STATUS_CANCEL;
#endif	//<<< PGPcomdlgQueryAddKeys changes ...


				if(error == PGPERR_OK)

				{

					error = pgpCommitKeyRingChanges(RingSet);

					

					if(error == PGPERR_OK)

					{

						MessageBox(hwnd, "Keys Added Successfully", "PGP",
									MB_OK | MB_ICONINFORMATION);

						PGPcomdlgNotifyKeyringChanges((long)hwnd);

						ReturnValue = STATUS_SUCCESS;

					}

				}



				pgpFreeKeySet(NewKeySet);

			}



			pgpFreeFileRef(fileRef);

         }
		
		pgpFreeKeySet(RingSet);

	}



	pgpLibCleanup();



	if(error != PGPERR_OK && DisplayError)

	{

		PGPcomdlgErrorMessage(error);

	}



	return ReturnValue;

}
