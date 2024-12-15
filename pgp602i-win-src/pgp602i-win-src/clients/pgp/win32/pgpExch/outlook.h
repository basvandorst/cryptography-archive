/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: outlook.h,v 1.2.18.1 1998/11/12 03:14:04 heller Exp $
____________________________________________________________________________*/
#ifndef Included_outlook_h	/* [ */
#define Included_outlook_h

const IID IID_IOutlookGetObjectForExchExtCallback = 
	{0x0006720D,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

DECLARE_INTERFACE_(IOutlookGetObjectForExchExtCallback, IUnknown)
{
        // *** IUnknown methods ***
        STDMETHOD(QueryInterface) (THIS_ REFIID riid, void** ppvObj) PURE;
        STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
        STDMETHOD_(ULONG,Release) (THIS) PURE;
        // *** IOutlookGetObjectForExchExtCallback methods ***
        STDMETHOD(GetObject) (THIS_ IUnknown **ppunk) PURE;
        STDMETHOD(DontCallThis)(THIS_ void **ppv) PURE;
};

#endif /* ] Included_outlook_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
