/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#ifndef _OUTLOOK_H
#define _OUTLOOK_H

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

#endif // _OUTLOOK_H
