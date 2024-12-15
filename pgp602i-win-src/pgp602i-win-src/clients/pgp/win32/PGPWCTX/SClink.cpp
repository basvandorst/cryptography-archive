/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	SClink.cpp

	Provides soft linking here so no delay on menu right clicks

	$Id: SClink.cpp,v 1.6.2.1.2.1 1998/11/12 03:12:58 heller Exp $
____________________________________________________________________________*/

#include "precomp.h"

HINSTANCE hSC_DLL=NULL;

typedef BOOL (*ENCRYPTFILELIST)(HWND,void *,void *,
					 FILELIST *,BOOL,BOOL);

BOOL EncryptFileList(HWND hwnd,void *PGPsc,void *PGPtls,
					 FILELIST *ListHead,BOOL bEncrypt,BOOL bSign)
{
	ENCRYPTFILELIST SCEncryptFileList;
	BOOL RetVal;

	SCEncryptFileList  = (ENCRYPTFILELIST)GetProcAddress(hSC_DLL,"EncryptFileList");

	RetVal=(SCEncryptFileList)(hwnd,PGPsc,PGPtls,
					 ListHead,bEncrypt,bSign);

	return RetVal;
}

typedef BOOL (*DECRYPTFILELIST)(HWND,void *,void *,FILELIST *);

BOOL DecryptFileList(HWND hwnd,void *PGPsc,void *PGPtls,FILELIST *ListHead)
{
	DECRYPTFILELIST SCDecryptFileList;
	BOOL RetVal;

	SCDecryptFileList  = (DECRYPTFILELIST)GetProcAddress(hSC_DLL,"DecryptFileList");

	RetVal=(SCDecryptFileList)(hwnd,PGPsc,PGPtls,ListHead);

	return RetVal;
}

typedef BOOL (*WIPEFILELIST)(HWND,void *,FILELIST *);

BOOL WipeFileList(HWND hwnd,void *PGPsc,FILELIST *ListHead)
{
	WIPEFILELIST SCWipeFileList;
	BOOL RetVal;

	SCWipeFileList  = (WIPEFILELIST)GetProcAddress(hSC_DLL,"WipeFileList");

	RetVal=(SCWipeFileList)(hwnd,PGPsc,ListHead);

	return RetVal;
}

typedef BOOL (*ADDTOFILELIST)(FILELIST **,char *,BOOL *);

BOOL AddToFileList(FILELIST **filelist,char *filename,BOOL *UserCancel)
{
	ADDTOFILELIST SCAddToFileList;
	BOOL RetVal;

	SCAddToFileList  = (ADDTOFILELIST)GetProcAddress(hSC_DLL,"AddToFileList");

	RetVal=(SCAddToFileList)(filelist,filename,UserCancel);

	return RetVal;
}

typedef BOOL (*FREEFILELIST)(FILELIST *);

BOOL FreeFileList(FILELIST *FileList)
{
	FREEFILELIST SCFreeFileList;
	BOOL RetVal;

	SCFreeFileList  = (FREEFILELIST)GetProcAddress(hSC_DLL,"FreeFileList");

	RetVal=(SCFreeFileList)(FileList);

	return RetVal;
}

typedef BOOL (*INITPGPSC)(HWND,void **,void **);

BOOL InitPGPsc(HWND hwnd,void **PGPsc,void **PGPtls)
{
	INITPGPSC SCInitPGPsc;
	BOOL RetVal;

    hSC_DLL = LoadLibrary("PGP60SC.DLL");
    if (hSC_DLL == 0) 
	{
		MessageBox(hwnd,"LoadLibrary() failed: Unable to locate PGP60SC.DLL!",
			"PGP Error",MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

		return FALSE;
    }

	SCInitPGPsc  = (INITPGPSC)GetProcAddress(hSC_DLL,"InitPGPsc");

	RetVal=(SCInitPGPsc)(hwnd,PGPsc,PGPtls);

	return RetVal;
}

typedef BOOL (*UNINITPGPSC)(HWND,void *,void *);

BOOL UninitPGPsc(HWND hwnd,void *PGPsc,void *PGPtls)
{
	UNINITPGPSC SCUninitPGPsc;
	BOOL RetVal=TRUE;

	if(PGPsc)
	{
		SCUninitPGPsc  = (UNINITPGPSC)GetProcAddress(hSC_DLL,"UninitPGPsc");

		RetVal=(SCUninitPGPsc)(hwnd,PGPsc,PGPtls);
	}

	if(hSC_DLL)
	{
		FreeLibrary(hSC_DLL);
		hSC_DLL=NULL;
	}

	return RetVal;
}

typedef UINT (*PGPSCQUERYCACHESECSREMAINING)(void);

UINT PGPscQueryCacheSecsRemaining (void)
{
	PGPSCQUERYCACHESECSREMAINING SCPGPscQueryCacheSecsRemaining;
	UINT RetVal;

	SCPGPscQueryCacheSecsRemaining  = 
		(PGPSCQUERYCACHESECSREMAINING)GetProcAddress(hSC_DLL,"PGPscQueryCacheSecsRemaining");

	RetVal=(SCPGPscQueryCacheSecsRemaining)();

	return RetVal;
}

typedef BOOL (*CHECKFORPURGE)(UINT,WPARAM);

BOOL CheckForPurge (UINT uMsg,WPARAM wParam)
{
	CHECKFORPURGE SCCheckForPurge;
	UINT RetVal;

	SCCheckForPurge  = 
		(CHECKFORPURGE)GetProcAddress(hSC_DLL,"CheckForPurge");

	RetVal=(SCCheckForPurge)(uMsg,wParam);

	return RetVal;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

