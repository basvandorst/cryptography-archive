/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: PropertySheets.cpp,v 1.5 1999/03/10 03:00:51 heller Exp $
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
