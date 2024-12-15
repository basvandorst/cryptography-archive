/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PropertySheets.cpp,v 1.4 1997/09/20 02:32:46 dgal Exp $
____________________________________________________________________________*/
#include "stdinc.h"
#include "Exchange.h"

STDMETHODIMP_(ULONG) CExtImpl::GetMaxPageCount(ULONG ulFlags)          
{
	return 0;
}


STDMETHODIMP CExtImpl::GetPages(IExchExtCallback* peecb,
	ULONG ulFlags, LPPROPSHEETPAGE ppsp, ULONG * pcpsp)
{
	return S_FALSE;
}


STDMETHODIMP_(VOID) CExtImpl::FreePages(LPPROPSHEETPAGE ppsp, 
										ULONG ulFlags, ULONG cpsp)
{
	return;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
