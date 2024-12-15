/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	CDgroups.h - Open/close group file
	

	$Id: CDgroups.c,v 1.8 1997/09/30 22:12:31 pbj Exp $
____________________________________________________________________________*/

#include "pgpcdlgx.h"
#include <process.h>

// external globals
extern HINSTANCE g_hInst;

//	_____________________________________
//
//	open group file and return groupfile object

PGPError PGPcdExport
PGPcomdlgOpenGroupFile (PGPcdGROUPFILE** ppGroup)
{
	CHAR szPath[MAX_PATH];
	CHAR szFile[64];
	PFLFileSpecRef	filespec;
	PGPError err;
	PFLContextRef pflcontext;
	PGPGroupSetRef groupset;

	*ppGroup = 0;
	groupset = NULL;
	pflcontext = NULL;

	PGPcomdlgGetPGPPath (szPath, sizeof(szPath));
	LoadString (g_hInst, IDS_GROUPFILENAME, szFile, sizeof(szFile));
	strcat (szPath, szFile);

	err = PFLNewContext (&pflcontext);
	if(IsntPGPError (err))
	{
		err = PFLNewFileSpecFromFullPath (pflcontext, szPath, &filespec);
		if(IsntPGPError (err))
		{
			err = PGPNewGroupSetFromFile (filespec, &groupset);
			if (IsPGPError (err)) {
				err = PGPNewGroupSet (pflcontext, &groupset);
			}
			
			*ppGroup = (PGPcdGROUPFILE *)malloc (sizeof(PGPcdGROUPFILE));
			if (*ppGroup) {
				(*ppGroup)->pflcontext = pflcontext;
				(*ppGroup)->pflfilespec = filespec;
				(*ppGroup)->groupset = groupset;
			}
			else {
				err = kPGPError_OutOfMemory;
				PGPFreeGroupSet (groupset);
				PFLFreeFileSpec (filespec);
				PFLFreeContext (pflcontext);
			}
		}
	}

	return err;
}


//	_____________________________________
//
//	write group data to file

PGPError PGPcdExport
PGPcomdlgSaveGroupFile (PGPcdGROUPFILE* pGroup)
{
	if (!pGroup) return kPGPError_BadParams;

	return (PGPSaveGroupSetToFile (pGroup->groupset, pGroup->pflfilespec));
}


//	_____________________________________
//
//	close down group file

PGPError PGPcdExport
PGPcomdlgCloseGroupFile (PGPcdGROUPFILE* pGroup)
{
	if (!pGroup) return kPGPError_BadParams;

	PGPFreeGroupSet (pGroup->groupset);
	PFLFreeFileSpec (pGroup->pflfilespec);
	PFLFreeContext (pGroup->pflcontext);

	free (pGroup);

	return kPGPError_NoErr;
}

