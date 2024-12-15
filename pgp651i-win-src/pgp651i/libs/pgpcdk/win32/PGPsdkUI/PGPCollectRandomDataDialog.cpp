/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: PGPCollectRandomDataDialog.cpp,v 1.10 1999/03/10 02:49:07 heller Exp $
____________________________________________________________________________*/

#include "PGPui32.h"
#include "pgpDialogs.h"
#include "PGPCL.h"
#include "pgpSDKUILibPriv.h"

// PGP SDK header files
#include "pgpRandomPool.h"

#define TIMERID			4321L
#define DELAYTIME		2000L
#define UPDATETIMER		4322L
#define UPDATETIME		50L

typedef struct _RANDSTRUCT {
	LONG lOrigNeeded;
	LONG lStillNeeded;
	LONG lTotalNeeded; 
	BOOL bCollectionEnabled;
} RANDSTRUCT, *PRANDSTRUCT;

PGPBoolean g_bEntropy	= FALSE;

// external globals
extern HHOOK		hhookKeyboard;
extern HHOOK		hhookMouse;

//	___________________________________________
//
//  Get Key Randomness Dialog Message procedure

BOOL CALLBACK 
KeyRandDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	PRANDSTRUCT	prs;
	LONG		l;
	CHAR		sz[256];

	switch (uMsg) 
	{

	case WM_INITDIALOG:
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		prs = (PRANDSTRUCT)lParam;

		prs->lStillNeeded = 
			prs->lTotalNeeded - PGPGlobalRandomPoolGetEntropy ();

		if (prs->lStillNeeded <= 0) 
			EndDialog (hDlg, kPGPError_NoErr);
		else 
		{
			prs->lOrigNeeded = prs->lStillNeeded;
			SendDlgItemMessage (hDlg, IDC_BITSPROGBAR, PBM_SETRANGE, 
									0, MAKELPARAM (0, prs->lOrigNeeded));
			SendDlgItemMessage (hDlg, IDC_BITSPROGBAR, PBM_SETPOS, 0, 0);
		}
		return TRUE;

	case WM_TIMER :
		prs = (PRANDSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

		if(	TIMERID == wParam)
		{
			KillTimer (hDlg, TIMERID);
			EndDialog (hDlg, kPGPError_NoErr);
		}

		if(	UPDATETIMER == wParam)
		{
			prs->lStillNeeded = 
				prs->lTotalNeeded - PGPGlobalRandomPoolGetEntropy ();

			if (prs->lStillNeeded > 0) 
			{
				l = prs->lOrigNeeded - prs->lStillNeeded;
				SendDlgItemMessage (hDlg, IDC_BITSPROGBAR, PBM_SETPOS, l, 0);
			}
			else 
			{
				SendDlgItemMessage (hDlg, IDC_BITSPROGBAR, PBM_SETPOS, prs->lOrigNeeded, 0);
				LoadString (gPGPsdkUILibInst, IDS_RANDOMDONE, sz, sizeof(sz));
				SetDlgItemText (hDlg, IDC_TEXTPROMPT, sz); 
				KillTimer (hDlg, UPDATETIMER);
				SetTimer (hDlg, TIMERID, DELAYTIME, NULL);
			}
		}
		break;

	case WM_COMMAND:
		switch(LOWORD (wParam)) 
		{
			case IDC_BEGIN :
				prs = (PRANDSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

				if(!prs->bCollectionEnabled)
				{
					prs->bCollectionEnabled = TRUE;
					EnableWindow (GetDlgItem (hDlg, IDC_BEGIN), FALSE);
					EnableWindow (GetDlgItem (hDlg, IDCANCEL), FALSE);
					EnableWindow (GetDlgItem (hDlg, IDC_STATIC1), TRUE);
					EnableWindow (GetDlgItem (hDlg, IDC_STATIC2), TRUE);
					EnableWindow (GetDlgItem (hDlg, IDC_STATIC3), TRUE);
					EnableWindow (GetDlgItem (hDlg, IDC_TEXTPROMPT), TRUE);
					EnableWindow (GetDlgItem (hDlg, IDC_BITSPROGBAR), TRUE);
					LoadString (gPGPsdkUILibInst, IDS_RANDOMPROMPT, sz, sizeof(sz));
					SetDlgItemText (hDlg, IDC_TEXTPROMPT, sz);
					InitRandomKeyHook(&hhookKeyboard,&hhookMouse);
					SetTimer (hDlg, UPDATETIMER, UPDATETIME, NULL);
					SetFocus(hDlg);
				}
				break;

			case IDCANCEL:
				EndDialog (hDlg, kPGPError_UserAbort);
				break;
		}
		return TRUE;
	}
	return FALSE;
}

//	______________________________________________
//
//  Entropy collection routine -- posts dialog for 
//  mouse/keyboard entropy collection.

	PGPError
pgpCollectRandomDataDialogPlatform(
	PGPContextRef					context,
	CPGPRandomDataDialogOptions 	*options)
{
	if (options->mTextUI) {
		return pgpCollectRandomDataCL(context, options);
	} else {
		RANDSTRUCT	rs;
		PGPError	err;
		
		memset(&rs,0x00,sizeof(RANDSTRUCT));
		
		if (g_bEntropy) 
			return kPGPError_ItemAlreadyExists;
		
		g_bEntropy=TRUE;
		
		// if zero passed in, use default
		if (options->mNeededEntropyBits) 
			rs.lTotalNeeded = options->mNeededEntropyBits;
		else 
			rs.lTotalNeeded = PGPGlobalRandomPoolGetMinimumEntropy();
		
		// if we have enough already, then just return
		if (rs.lTotalNeeded < (LONG)PGPGlobalRandomPoolGetEntropy()) 
			return kPGPError_NoErr;
		
		// collect the entropy
		err = DialogBoxParam(gPGPsdkUILibInst,
							 MAKEINTRESOURCE (IDD_KEYRAND), 
							 options->mHwndParent,
							 (DLGPROC)KeyRandDlgProc,
							 (LPARAM)&rs);
		
		if(rs.bCollectionEnabled)
			UninitRandomKeyHook(hhookKeyboard,hhookMouse);
		
		g_bEntropy=FALSE;
		
		return err;
	}
}

