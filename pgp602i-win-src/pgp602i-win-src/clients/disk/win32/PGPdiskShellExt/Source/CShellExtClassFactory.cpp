//////////////////////////////////////////////////////////////////////////////
// CShellExtClassFactory.cpp
//
// Definition of class CShellExtClassFactory.
//////////////////////////////////////////////////////////////////////////////

// $Id: CShellExtClassFactory.cpp,v 1.1.2.2.2.1 1998/08/22 01:55:39 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "CShellExt.h"
#include "CShellExtClassFactory.h"
#include "Globals.h"


//////////////////////////////
// Class CShellExtClassFactory
//////////////////////////////

// The CShellExtClassFactory default constructor.

CShellExtClassFactory::CShellExtClassFactory()
{
	mRefCount = 0;
	DLL->mGCRefThisDll++; 
}

// The CShellExtClassFactory destructor.
																
CShellExtClassFactory::~CShellExtClassFactory() 			
{
	DLL->mGCRefThisDll--;
}

// QueryInterface is called to ask the shell extension to return a pointer to
// its interface.

STDMETHODIMP 
CShellExtClassFactory::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	(* ppv) = NULL;

	// Any interface on this object is the object pointer.
	if (IsEqualIID(riid, IID_IUnknown) || 
		IsEqualIID(riid, IID_IClassFactory))
	{
		(* ppv) = (LPCLASSFACTORY) this;
		AddRef();

		return NOERROR;
	}
	else
	{
		return E_NOINTERFACE;
	}
}

// AddRef increases the reference count for this class object.

STDMETHODIMP_(ULONG) 
CShellExtClassFactory::AddRef()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return ++mRefCount;
}

// Release descreases the reference count for this class object.

STDMETHODIMP_(ULONG) 
CShellExtClassFactory::Release()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (--mRefCount)
		return mRefCount;

	delete this;

	return 0;
}

// CreateInstance creates an instance of this class object.

STDMETHODIMP 
CShellExtClassFactory::CreateInstance(
	LPUNKNOWN	pUnkOuter, 
	REFIID		riid, 
	LPVOID		*ppvObj)
{
	LPCSHELLEXT	pShellExt;
	PGPBoolean	madeShellExt;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	(* ppvObj) = NULL;

	// Shell extensions typically don't support aggregation (inheritance).
	if (pUnkOuter)
		return CLASS_E_NOAGGREGATION;

	// Create the main shell extension object.	The shell will then call
	// QueryInterface with IID_IShellExtInit--this is how shell extensions
	// are initialized.

	try
	{
		pShellExt = new CShellExt();		
		madeShellExt = TRUE;
	}
	catch (CMemoryException *ex)
	{
		madeShellExt = FALSE;
		ex->Delete();
	}

	if (madeShellExt)
		return pShellExt->QueryInterface(riid, ppvObj);
	else
		return E_OUTOFMEMORY;
}

// LockServer just returns since we don't support it.

STDMETHODIMP 
CShellExtClassFactory::LockServer(BOOL fLock)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return NOERROR;
}
