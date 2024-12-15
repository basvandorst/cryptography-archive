//////////////////////////////////////////////////////////////////////////////
// StdAfx.h
//
// Contains global MFC includes.
//////////////////////////////////////////////////////////////////////////////

// $Id: StdAfx.h,v 1.1.2.1 1998/05/29 17:16:46 nryan Exp $

// Not really Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_StdAfx_h	// [
#define Included_StdAfx_h

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC OLE automation classes
#endif // _AFX_NO_OLE_SUPPORT


#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			// MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

//{{AFX_INSERT_LOCATION}}

#endif // Included_StdAfx_h
