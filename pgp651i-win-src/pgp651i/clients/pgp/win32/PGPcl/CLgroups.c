/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLgroups.c - Open/close group file
	

	$Id: CLgroups.c,v 1.5 1999/03/01 17:41:30 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpclx.h"

// external globals
extern HINSTANCE g_hInst;

//	_____________________________________
//
//	open group file and return groupfile object

PGPError PGPclExport
PGPclOpenGroupFile (
		PGPContextRef		context,
		PGPclGROUPFILE**	ppGroup)
{
	CHAR			szPath[MAX_PATH];
	CHAR			szFile[64];
	PGPFileSpecRef	filespec;
	PGPError		err;
	PGPGroupSetRef	groupset;

	*ppGroup = 0;
	groupset = NULL;

	PGPclGetClientPrefsPath (szPath, sizeof(szPath));
	LoadString (g_hInst, IDS_GROUPFILENAME, szFile, sizeof(szFile));
	strcat (szPath, szFile);

	err = PGPNewFileSpecFromFullPath (context, szPath, &filespec);
	if(IsntPGPError (err))
	{
		err = PGPNewGroupSetFromFile (context, filespec, &groupset);
		if (IsPGPError (err)) {
			err = PGPNewGroupSet (context, &groupset);
		}
		
		*ppGroup = (PGPclGROUPFILE *)malloc (sizeof(PGPclGROUPFILE));
		if (*ppGroup) {
			(*ppGroup)->filespec = filespec;
			(*ppGroup)->groupset = groupset;
		}
		else {
			err = kPGPError_OutOfMemory;
			PGPFreeGroupSet (groupset);
			PGPFreeFileSpec (filespec);
		}
	}

	return err;
}


//	_____________________________________
//
//	write group data to file

PGPError PGPclExport
PGPclSaveGroupFile (PGPclGROUPFILE* pGroup)
{
	if (!pGroup) return kPGPError_BadParams;

	return (PGPSaveGroupSetToFile (pGroup->groupset, pGroup->filespec));
}


//	_____________________________________
//
//	close down group file

PGPError PGPclExport
PGPclCloseGroupFile (PGPclGROUPFILE* pGroup)
{
	if (!pGroup) return kPGPError_BadParams;

	if(pGroup->groupset)
		PGPFreeGroupSet (pGroup->groupset);

	if(pGroup->filespec)
		PGPFreeFileSpec (pGroup->filespec);

	free (pGroup);

	return kPGPError_NoErr;
}

