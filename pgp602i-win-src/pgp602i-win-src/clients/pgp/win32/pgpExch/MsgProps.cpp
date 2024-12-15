/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: MsgProps.cpp,v 1.5.16.1 1998/11/12 03:13:59 heller Exp $
____________________________________________________________________________*/
#include "stdinc.h"
#include "guids.h"
#include "Exchange.h"
#include "resource.h"
#include "UIutils.h"


CDecrypt::CDecrypt(IMAPISession* psess) 
	: _psess(psess), _fInit(FALSE)
{
	if (psess)
		psess->AddRef();
}


CDecrypt::~CDecrypt()
{ 
	if (_psess)
		_psess->Release();
}


void CDecrypt::TakeSession(IMAPISession* psess)
{
	if (_psess)
		_psess->Release();
	_psess = psess;
	if (psess)
		psess->AddRef();
}


void CDecrypt::DropSession()
{
	if (_psess)
		_psess->Release();
	_psess = NULL;
}


void CDecrypt::Load()
{
	// If we hit any exceptions, initialize as empty anyway.

	_fInit = TRUE;

	IProfSect* pprof;
	HRESULT hr = _psess->OpenProfileSection(
		(LPMAPIUID)&GUID_PGPPlugin, NULL, 0, &pprof);
	if (FAILED(hr)) // Assume uninitialized
		return;

	pprof->Release();
}


void CDecrypt::Save()
{
	IProfSect* pprof;
	HRESULT hr = _psess->OpenProfileSection(
		(LPMAPIUID)&GUID_PGPPlugin, NULL, MAPI_MODIFY, &pprof);
	if (pprof)
		pprof->Release();

	if (FAILED(hr))
	{
		UIDisplayStringID(NULL, IDS_E_SAVEPROFILE );
	}

}




/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
