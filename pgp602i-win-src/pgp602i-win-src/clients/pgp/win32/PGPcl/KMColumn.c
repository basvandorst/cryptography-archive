/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMColumn.c - manage column selection
	

	$Id: KMColumn.c,v 1.6 1998/08/11 14:43:22 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"

// pgp header files
#include "pgpClientPrefs.h"

// typedefs
typedef struct {
	WORD wColumnField[NUMBERFIELDS];
	WORD wFieldWidth[NUMBERFIELDS];
	LONG lSortField;
} COLUMNPREFSSTRUCT, *PCOLUMNPREFSSTRUCT;

// constant definitions
#define DEFAULTCOLWIDTHNAME		240
#define DEFAULTCOLWIDTHVALID	50
#define DEFAULTCOLWIDTHTRUST	50
#define DEFAULTCOLWIDTHSIZE		70
#define DEFAULTCOLWIDTHDESC		200
#define MINWIDTH				10
#define MAXWIDTH				1000

#define KMI_NAME				0
#define KMI_VALIDITY			1
#define KMI_SIZE				2
#define KMI_DESCRIPTION			3
#define KMI_KEYID				4
#define KMI_TRUST				5
#define KMI_CREATION			6
#define KMI_EXPIRATION			7
#define KMI_ADK					8
#define NUMBERFIELDS			9

// external global variables
extern HINSTANCE g_hInst;

//	___________________________________________
//
//	Get widths of control columns from client prefs file
VOID 
KMGetColumnPreferences (PKEYMAN pKM)
{
	PGPError			err;
	PGPPrefRef			prefref;
	PGPSize				size;
	PCOLUMNPREFSSTRUCT	pcps;
	INT					i;
	WORD				wField, wWidth;

#if PGP_BUSINESS_SECURITY
	pKM->wColumnField[0]				= KMI_NAME;
	pKM->wColumnField[1]				= KMI_VALIDITY;
	pKM->wColumnField[2]				= KMI_SIZE;
	pKM->wColumnField[3]				= KMI_DESCRIPTION;
	pKM->wColumnField[4]				= 0;
	pKM->wColumnField[5]				= 0;
	pKM->wColumnField[6]				= 0;
	pKM->wColumnField[7]				= 0;
	pKM->wColumnField[8]				= 0;

	pKM->wFieldWidth[KMI_NAME]			= DEFAULTCOLWIDTHNAME;
	pKM->wFieldWidth[KMI_VALIDITY]		= DEFAULTCOLWIDTHVALID;
	pKM->wFieldWidth[KMI_SIZE]			= DEFAULTCOLWIDTHSIZE;
	pKM->wFieldWidth[KMI_DESCRIPTION]	= DEFAULTCOLWIDTHDESC;
	pKM->wFieldWidth[KMI_KEYID]			= 0;
	pKM->wFieldWidth[KMI_TRUST]			= 0;
	pKM->wFieldWidth[KMI_CREATION]		= 0;
	pKM->wFieldWidth[KMI_EXPIRATION]	= 0;
	pKM->wFieldWidth[KMI_ADK]			= 0;
#else
	pKM->wColumnField[0]				= KMI_NAME;
	pKM->wColumnField[1]				= KMI_VALIDITY;
	pKM->wColumnField[2]				= KMI_TRUST;
	pKM->wColumnField[3]				= KMI_SIZE;
	pKM->wColumnField[4]				= KMI_DESCRIPTION;
	pKM->wColumnField[5]				= 0;
	pKM->wColumnField[6]				= 0;
	pKM->wColumnField[7]				= 0;
	pKM->wColumnField[8]				= 0;

	pKM->wFieldWidth[KMI_NAME]			= DEFAULTCOLWIDTHNAME;
	pKM->wFieldWidth[KMI_VALIDITY]		= DEFAULTCOLWIDTHVALID;
	pKM->wFieldWidth[KMI_SIZE]			= DEFAULTCOLWIDTHSIZE;
	pKM->wFieldWidth[KMI_DESCRIPTION]	= DEFAULTCOLWIDTHDESC;
	pKM->wFieldWidth[KMI_KEYID]			= 0;
	pKM->wFieldWidth[KMI_TRUST]			= DEFAULTCOLWIDTHTRUST;
	pKM->wFieldWidth[KMI_CREATION]		= 0;
	pKM->wFieldWidth[KMI_EXPIRATION]	= 0;
	pKM->wFieldWidth[KMI_ADK]			= 0;
#endif

	pKM->lKeyListSortField				= kPGPUserIDOrdering;

	err = PGPclOpenClientPrefs (PGPGetContextMemoryMgr (pKM->Context), 
										&prefref);
	if (IsntPGPError (err)) {
		err = PGPGetPrefData (prefref, kPGPPrefPGPkeysWinColumnData,
							  &size, &pcps);

		if (IsntPGPError (err)) {
			if (size == sizeof(COLUMNPREFSSTRUCT)) {
				for (i=0; i<NUMBERFIELDS; i++) {
					wField = pcps->wColumnField[i];
					if ((wField >= 0) && (wField < NUMBERFIELDS))
						pKM->wColumnField[i] = wField;
					wWidth = pcps->wFieldWidth[i];
					if ((wWidth >= MINWIDTH) && (wWidth <= MAXWIDTH))
						pKM->wFieldWidth[i] = wWidth;
				}
				pKM->lKeyListSortField = pcps->lSortField;
			}
			PGPDisposePrefData (prefref, pcps);
		}
		PGPclCloseClientPrefs (prefref, FALSE);
	}
}

//	___________________________________________
// 
//	Put column information into client prefs file

VOID 
KMSetColumnPreferences (PKEYMAN pKM) 
{
	PGPError			err;
	PGPPrefRef			prefref;
	COLUMNPREFSSTRUCT	cps;
	INT					i, iField;


	err = PGPclOpenClientPrefs (PGPGetContextMemoryMgr (pKM->Context), 
										&prefref);
	if (IsntPGPError (err)) {
		for (i=0; i<NUMBERFIELDS; i++) {
			cps.wColumnField[i] = pKM->wColumnField[i];

			iField = pKM->wColumnField[i];
			if ((i == 0) || (iField != 0)) pKM->wFieldWidth[iField] = 
					LOWORD (TreeList_GetColumnWidth (pKM->hWndTree, i));
			cps.wFieldWidth[i] = pKM->wFieldWidth[i];
		}

		cps.lSortField = pKM->lKeyListSortField;

		PGPSetPrefData (prefref, kPGPPrefPGPkeysWinColumnData,
							  sizeof(cps), &cps);

		PGPclCloseClientPrefs (prefref, TRUE);
	}
}


//	_________________________
//
//	Retrieve flags indicating which columns are displayed

VOID 
KMGetSelectedColumns (PKEYMAN pKM, ULONG* pulColumnFlags) 
{
	INT		iCol, iField;

	if (!pulColumnFlags) return;

	*pulColumnFlags = 0;

	for (iCol=1; iCol<NUMBERFIELDS; iCol++) {
		iField = pKM->wColumnField[iCol];
		switch (iField) {
		case KMI_VALIDITY :
			*pulColumnFlags |= KM_VALIDITY;
			break;
		case KMI_SIZE :
			*pulColumnFlags |= KM_SIZE;
			break;
		case KMI_DESCRIPTION :
			*pulColumnFlags |= KM_DESCRIPTION;
			break;
		case KMI_KEYID :
			*pulColumnFlags |= KM_KEYID;
			break;
		case KMI_TRUST :
			*pulColumnFlags |= KM_TRUST;
			break;
		case KMI_CREATION :
			*pulColumnFlags |= KM_CREATION;
			break;
		case KMI_EXPIRATION :
			*pulColumnFlags |= KM_EXPIRATION;
			break;
		case KMI_ADK :
			*pulColumnFlags |= KM_ADK;
			break;
		}
	}
	return;
}


//	_________________________
//
//	Remove columns to display

static VOID 
sRemoveColumn (PKEYMAN pKM, INT iField) 
{
	INT i, j;

	for (i=1; i<NUMBERFIELDS; i++) {
		if (pKM->wColumnField[i] == iField) {
			for (j=i+1; j<NUMBERFIELDS; j++) {
				pKM->wColumnField[j-1] = pKM->wColumnField[j];
			}
			pKM->wColumnField[NUMBERFIELDS-1] = 0;
			return;
		}
	}
}

			
//	_________________________
//
//	Add columns to display

static VOID 
sAddColumn (PKEYMAN pKM, INT iField) 
{
	INT i, iNumCol;
	
	iNumCol = 1;
	for (i=0; i<NUMBERFIELDS; i++) {
		if (pKM->wColumnField[i]) iNumCol++;
	}

	for (i=0; i<iNumCol; i++) {
		if (pKM->wColumnField[i] == iField) return;
	}

	pKM->wColumnField[iNumCol] = iField;
	if ((pKM->wFieldWidth[iField] > 1000) ||
		(pKM->wFieldWidth[iField] <= 0)) 
		pKM->wFieldWidth[iField] = 60;
	
}


//	_________________________
//
//	Select columns to display

VOID 
KMSelectColumns (PKEYMAN pKM, ULONG ulColumnFlags) 
{
	if (ulColumnFlags & KM_VALIDITY) sAddColumn (pKM, KMI_VALIDITY);
	else sRemoveColumn (pKM, KMI_VALIDITY);

	if (ulColumnFlags & KM_SIZE) sAddColumn (pKM, KMI_SIZE);
	else sRemoveColumn (pKM, KMI_SIZE);

	if (ulColumnFlags & KM_DESCRIPTION) sAddColumn (pKM, KMI_DESCRIPTION);
	else sRemoveColumn (pKM, KMI_DESCRIPTION);

	if (ulColumnFlags & KM_KEYID) sAddColumn (pKM, KMI_KEYID);
	else sRemoveColumn (pKM, KMI_KEYID);

	if (ulColumnFlags & KM_TRUST) sAddColumn (pKM, KMI_TRUST);
	else sRemoveColumn (pKM, KMI_TRUST);

	if (ulColumnFlags & KM_CREATION) sAddColumn (pKM, KMI_CREATION);
	else sRemoveColumn (pKM, KMI_CREATION);

	if (ulColumnFlags & KM_EXPIRATION) sAddColumn (pKM, KMI_EXPIRATION);
	else sRemoveColumn (pKM, KMI_EXPIRATION);

	if (ulColumnFlags & KM_ADK) sAddColumn (pKM, KMI_ADK);
	else sRemoveColumn (pKM, KMI_ADK);

}
