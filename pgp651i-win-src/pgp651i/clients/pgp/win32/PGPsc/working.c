/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: working.c,v 1.28 1999/03/10 03:04:58 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"

PGPError DoWorkThread(HPRGDLG hPrgDlg,void *pUserValue)
{
	MYSTATE *myState;
	PGPError err;
	DWORD nOleErr;

	err=kPGPError_UnknownError;

	myState=(MYSTATE *)pUserValue;

	myState->hPrgDlg=hPrgDlg;
	myState->hwndWorking=SCGetProgressHWND(hPrgDlg);

	nOleErr = OleInitialize(NULL);

	if ((nOleErr == S_OK) || (nOleErr == S_FALSE))
	{
		switch(myState->Operation)
		{
			case MS_ENCRYPTFILELIST:
				err=EncryptFileListStub (myState);
				break;

			case MS_ENCRYPTCLIPBOARD:
				err=EncryptClipboardStub (myState);
				break;

			case MS_DECRYPTFILELIST:
				err=DecryptFileListStub (myState);
				break;

			case MS_DECRYPTCLIPBOARD:
				err=DecryptClipboardStub (myState);
				break;

			case MS_ADDKEYFILELIST:
				err=AddKeyFileListStub (myState);
				break;

			case MS_ADDKEYCLIPBOARD:
				err=AddKeyClipboardStub (myState);
				break;

			case MS_WIPEFILELIST:
				err=WipeFileListStub (myState);
				break;
		}
		OleUninitialize();
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
