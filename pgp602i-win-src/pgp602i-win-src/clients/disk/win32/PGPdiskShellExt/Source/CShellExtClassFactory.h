//////////////////////////////////////////////////////////////////////////////
// CShellExtClassFactory.h
//
// Declarations CShellExtClassFactory.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CShellExtClassFactory.h,v 1.1.2.2 1998/07/06 08:58:41 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CShellExtClassFactory_h // [
#define Included_CShellExtClassFactory_h


//////////////////////////////
// Class CShellExtClassFactory
//////////////////////////////

// This class factory object creates context menu handlers.

class CShellExtClassFactory : public IClassFactory
{
public:
							CShellExtClassFactory();
							~CShellExtClassFactory();

	// IUnknown members.
	STDMETHODIMP			QueryInterface(REFIID riid , LPVOID FAR* ppv);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	// IClassFactory members.
	STDMETHODIMP			CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, 
								LPVOID FAR *ppvObj);
	STDMETHODIMP			LockServer(BOOL);

protected:
	ULONG mRefCount;		// ref count for the object

};

typedef CShellExtClassFactory *LPCSHELLEXTCLASSFACTORY;

#endif	// ] Included_CShellExtClassFactory_h
