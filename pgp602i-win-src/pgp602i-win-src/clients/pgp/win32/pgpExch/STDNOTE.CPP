/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: STDNOTE.CPP,v 1.3.16.1 1998/11/12 03:14:02 heller Exp $
____________________________________________________________________________*/

#include "stdinc.h"
#include "Exchange.h"
#include "UIutils.h"
#include "resource.h"


// Property recording the decision of CMsgType

#define PR_INETXIDM_EXISTING_MESSAGE	PROP_TAG(PT_LONG, 0x7C00)



//	Heuristic class to guess the origin of the current note.

// Codes for CMsgType Read/Write

enum {MSGTYPE_COMPOSE_NEW=1, MSGTYPE_COMPOSE_REPLY, MSGTYPE_COMPOSE_FORWARD};

#define MASK_MSGTYPE_COMPOSE 0x00000003
#define MASK_MSGTYPE_SUBMITTED 0x00000004


HRESULT CMsgType::Read(IExchExtCallback* peecb)
{
	{
		ADRLIST* padrlist;
		HRESULT hr = peecb->GetRecipients(&padrlist);
		if (FAILED(hr))
			return hr;
		if (padrlist == NULL)
			_cRecips = 0;
		else
			_cRecips = (UINT) padrlist->cEntries;
		FreePadrlist(padrlist);
	}

	{
		IMessage* pmsg;
		HRESULT hr = peecb->GetObject(NULL, (IMAPIProp**)&pmsg);
		if (FAILED(hr))
			return hr;

		enum {ivalSubject = 0, ivalPrefix, ivalIsNotNew, cvalTotal};
		SizedSPropTagArray(cvalTotal, taga) = 
			{ 3, 
			{ PR_SUBJECT, PR_SUBJECT_PREFIX, PR_INETXIDM_EXISTING_MESSAGE }};
		SPropValue* pval;
		ULONG cval = cvalTotal;
		hr = pmsg->GetProps((SPropTagArray*)&taga, 0, &cval, &pval);
		if (FAILED(hr))
		{
			pmsg->Release();
			return hr;
		}

		_fSubjSet = 
			(pval[ivalSubject].ulPropTag == PR_SUBJECT
				&& pval[ivalPrefix].ulPropTag == PR_SUBJECT_PREFIX);
		_fNew = (PROP_TYPE(pval[ivalIsNotNew].ulPropTag) == PT_ERROR);
		if (!_fNew)
		{
			const ULONG ul = pval[ivalIsNotNew].Value.ul;

			_fIsReply = ((MASK_MSGTYPE_COMPOSE & ul) == 
							MSGTYPE_COMPOSE_REPLY);
			_fIsFwd = ((MASK_MSGTYPE_COMPOSE & ul) == 
							MSGTYPE_COMPOSE_FORWARD);
			_fIsSubmitted = !!(MASK_MSGTYPE_SUBMITTED & ul);
		}


		MAPIFreeBuffer(pval);
		pmsg->Release();
	}

	return NOERROR;
}


HRESULT CMsgType::Write(IExchExtCallback* peecb, BOOL fInSubmitState)
{
	HWND hwnd = NULL;
	peecb->GetWindow(&hwnd);

	IMessage *pmsg = 0;

	HRESULT hr = peecb->GetObject(NULL, (LPMAPIPROP *)&pmsg);
	if (FAILED(hr))
	{
		UIDisplayStringID(hwnd, IDS_E_NOMESSAGE);
		return hr;
	}

	// Stamp the message's souvenir booklet.

	SPropValue val;
	val.ulPropTag = PR_INETXIDM_EXISTING_MESSAGE;
	val.Value.ul =
		(IsReply() ? MSGTYPE_COMPOSE_REPLY 
			: (IsForward() ? MSGTYPE_COMPOSE_FORWARD 
				: MSGTYPE_COMPOSE_NEW));
	if (_fIsSubmitted || fInSubmitState)
		val.Value.ul |= MASK_MSGTYPE_SUBMITTED;
	hr = pmsg->SetProps(1, &val, NULL);
	if (FAILED(hr))
	{
		UIDisplayStringID(hwnd, IDS_E_STAMPMESSAGE);
		pmsg->Release();
		return hr;
	}

	// All client extensions operate as parasites within the client.
	// Only the client gets the privilege of calling SaveChanges.

	pmsg->Release();
	return NOERROR;
}


BOOL CMsgType::IsNew() const
{
	return _fNew;
}


BOOL CMsgType::IsReply() const
{
	if (!_fNew)
		return _fIsReply;
	else
		return (_cRecips > 0 && _fSubjSet);
}


BOOL CMsgType::IsForward() const
{
	if (!_fNew)
		return _fIsFwd;
	else
		return (_cRecips == 0 && _fSubjSet);
}


BOOL CMsgType::WasSubmittedOnce() const
{
	if (_fNew)
		return FALSE;
	else
		return _fIsSubmitted;
}



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
