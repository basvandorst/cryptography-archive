/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: Shexinit.cpp,v 1.5 1997/09/19 20:16:16 wjb Exp $
____________________________________________________________________________*/

//  MODULE:   shexinit.cpp
//
//  PURPOSE:   Implements the IShellExtInit member 
//             function necessary to support
//             the context menu and property sheet
//             portions of this shell extension.  
//
#include "precomp.h"


#include "priv.h"
#include "pgpwctx.hpp"

//
//  FUNCTION: CShellExt::Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY)
//
//  PURPOSE: Called by the shell when initializing a context menu or property
//           sheet extension.
//
//  PARAMETERS:
//    pIDFolder - Specifies the parent folder
//    pDataObj  - Spefifies the set of items selected in that folder.
//    hRegKey   - Specifies the type of the focused item in the selection.
//
//  RETURN VALUE:
//
//    NOERROR in all cases.
//
//  COMMENTS:   Note that at the time this function is called, 
//              we don't know (or care) what type of shell extension
//              is being initialized. It could be a context menu or
//              a property sheet.
//

STDMETHODIMP CShellExt::Initialize(LPCITEMIDLIST pIDFolder,
                                   LPDATAOBJECT pDataObj,
                                   HKEY hRegKey)
{
    FORMATETC fmte = {CF_HDROP,
        	          (DVTARGETDEVICE FAR *)NULL,
        	          DVASPECT_CONTENT,
        	          -1,
        	          TYMED_HGLOBAL 
        	         };
	HRESULT hres = 0;

    ODS("CShellExt::Initialize()\r\n");

    // Initialize can be called more than once

    if (m_pDataObj)
    	m_pDataObj->Release();

    // duplicate the object pointer and registry handle

    if (pDataObj)
    {
    	m_pDataObj = pDataObj;
    	pDataObj->AddRef();
    }

    // get the file name
	m_szFileUserClickedOn[0] = '\0';	// start with empty string
	medium.tymed = TYMED_HGLOBAL;
	medium.pUnkForRelease = NULL;
	medium.hGlobal = NULL;
	if (m_pDataObj)  
		//Paranoid check, m_pDataObj should have something by now...
       hres = m_pDataObj->GetData(&fmte, &medium);

    if (SUCCEEDED(hres))
    {
        //Find out how many files the user has selected...
        UINT cbFiles = 0;
        if (medium.hGlobal)
            cbFiles = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, 0, 0);

        //Get the name of the first file the user has clicked on
		if (cbFiles)
			DragQueryFile((HDROP)medium.hGlobal, 
                             0, 
                             m_szFileUserClickedOn,
                             sizeof(m_szFileUserClickedOn));

		ODS(m_szFileUserClickedOn);
		ODS("\r\n");
	}
 
	return NOERROR;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
