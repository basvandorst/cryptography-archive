/*
 * Copyright (C) 1995 - 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*  $Id: Pgpwctx.cpp,v 1.1.1.1 1997/04/29 00:24:10 philipn Exp $ */

// pgpwctx.cpp		Context Menu Handler

// Copied from shellext.cpp in Microsoft Shellext sample.

// Purpose:  Implements the class factory code as well as CShellExt::QI,
//           CShellExt::AddRef and CShellExt::Release code.

#include <windows.h>

#include "priv.h"
//#include "pgpwctx.hpp"
#include "resource.h"
#include <stdio.h>

//
// Initialize GUIDs (should be done only and at-least once per DLL/EXE)
//
#pragma data_seg(".text")
#define INITGUID
#include <initguid.h>
#include <shlguid.h>
DEFINE_GUID(CLSID_PGPext, 0x969223c0, 0x26aa, 0x11d0, 0x90, 0xee, 0x44, 0x45,
			0x53, 0x54, 0x00, 0x00);
#pragma data_seg()
#include "pgpwctx.hpp"
#include "..\include\config.h"
#include "..\include\pgpkeydb.h"
#include "..\include\pgpphrase.h"

//
// Global variables
//
UINT      g_cRefThisDll = 0;    // Reference count of this DLL.
HINSTANCE g_hmodThisDll = NULL;	// Handle to this DLL itself.
HMENU		hPlaintextMenu;
HMENU		hEncryptedMenu;
HMENU		hKeyfileMenu;
HANDLE		hPGPThreadMutex = NULL;

static FILE * pLog = NULL;

//extern "C" int APIENTRY
extern int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
     if (dwReason == DLL_PROCESS_ATTACH)
     {
#ifdef _DEBUG
//		pLog = fopen("d:\\CtxLog.txt", "wb");
         ODS("In DLLMain, DLL_PROCESS_ATTACH\r\n");
#endif
        // Extension DLL one-time initialization

        g_hmodThisDll = hInstance;
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        ODS("In DLLMain, DLL_PROCESS_DETACH\r\n");
		if (pLog)
			fclose(pLog);
    }

    return 1;   // ok
}

//---------------------------------------------------------------------------
// DllCanUnloadNow
//---------------------------------------------------------------------------

STDAPI DllCanUnloadNow(void)
{
    ODS("In DLLCanUnloadNow\r\n");
	BOOL CanUnload = S_FALSE;

	/*We don't want to unload until there are no more references to this DLL,
	 *and until the PGP pass phrase caching thread has ended.
	 */
	if(!g_cRefThisDll &&
	    (!hPGPThreadMutex || (WaitForSingleObject(hPGPThreadMutex, 0)
							    != WAIT_TIMEOUT)) &&
	    !PGPQueryCacheSecsRemaining())
	{
		CanUnload = S_OK;
	}

    return (CanUnload);
/*	return(S_FALSE);*/
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut)
{
    HRESULT hRes;
	ODS("In DllGetClassObject\r\n");

    if (g_cRefThisDll)		// if factory or extension still active
		return CLASS_E_CLASSNOTAVAILABLE;

	*ppvOut = NULL;

    if (IsEqualIID(rclsid, CLSID_PGPext))
    {
        CPGPClassFactory *pcf = new CPGPClassFactory;

        if (pcf)
		{
			hRes = pcf->QueryInterface(riid, ppvOut);
			return hRes;
		}
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}

CPGPClassFactory::CPGPClassFactory()
{
    ODS("CPGPClassFactory::CPGPClassFactory()\r\n");

    m_cRef = 0L;

    g_cRefThisDll++;	
}
																
CPGPClassFactory::~CPGPClassFactory()				
{
    ODS("CPGPClassFactory::~CPGPClassFactory()\r\n");
    g_cRefThisDll--;
}

STDMETHODIMP CPGPClassFactory::QueryInterface(REFIID riid,
                                                   LPVOID FAR *ppv)
{
    ODS("CPGPClassFactory::QueryInterface()\r\n");

    *ppv = NULL;

    // Any interface on this object is the object pointer

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;

        AddRef();

        return NOERROR;
    }

    return E_NOINTERFACE;
}	

STDMETHODIMP_(ULONG) CPGPClassFactory::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CPGPClassFactory::Release()
{
    ODS("CPGPClassFactory::Release()\r\n");
    if (--m_cRef)
        return m_cRef;

    delete this;

    return 0L;
}

STDMETHODIMP CPGPClassFactory::CreateInstance(LPUNKNOWN pUnkOuter,
                                                      REFIID riid,
                                                      LPVOID *ppvObj)
{
    ODS("CPGPClassFactory::CreateInstance()\r\n");

    *ppvObj = NULL;

    // Shell extensions typically don't support aggregation (inheritance)

    if (pUnkOuter)
    	return CLASS_E_NOAGGREGATION;

    // Create the main shell extension object.  The shell will then call
    // QueryInterface with IID_IShellExtInit--this is how shell extensions are
    // initialized.

    LPCSHELLEXT pShellExt = new CShellExt();  //Create the CShellExt object

    if (NULL == pShellExt)
    	return E_OUTOFMEMORY;

    return pShellExt->QueryInterface(riid, ppvObj);
}


STDMETHODIMP CPGPClassFactory::LockServer(BOOL fLock)
{
    return NOERROR;
}

// *********************** CShellExt *************************
CShellExt::CShellExt()
{
    ODS("CShellExt::CShellExt()\r\n");

    m_cRef = 0L;
    m_pDataObj = NULL;

    g_cRefThisDll++;
}

CShellExt::~CShellExt()
{
    ODS("CShellExt::~CShellExt()\r\n");

    g_cRefThisDll--;
}

STDMETHODIMP CShellExt::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown))
    {
        ODS("CShellExt::QueryInterface()==>IID_IShellExtInit\r\n");

    	*ppv = (LPSHELLEXTINIT)this;
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        ODS("CShellExt::QueryInterface()==>IID_IContextMenu\r\n");

        *ppv = (LPCONTEXTMENU)this;
    }
    if (*ppv)
    {
        AddRef();

        return NOERROR;
    }

    ODS("CShellExt::QueryInterface()==>Unknown Interface!\r\n");

	delete this;
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CShellExt::AddRef()
{
    ODS("CShellExt::AddRef()\r\n");

    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExt::Release()
{
    ODS("CShellExt::Release()\r\n");

    if (--m_cRef)
        return m_cRef;

    if (hSubMenu)
		if (!(DeleteMenu(hParentMenu, (UINT)hSubMenu, MF_BYCOMMAND)))
			ODS("DeleteMenu() failed.\r\n");
		else
			ODS("DeleteMenu() succeeded.\r\n");

    if (m_pDataObj)
        m_pDataObj->Release();

	delete this;

    return 0L;
}

#ifdef _DEBUG
void ODS(LPCSTR msg)
{
	//MessageBox(NULL, msg, "Hmmmm", MB_OK);
}
#endif
