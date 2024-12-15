/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMIDropSource.cpp - implements OLE IDropSource for drag and drop

	$Id: KMIDropSource.cpp,v 1.11 1998/08/11 14:43:26 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// system header files
#include <windows.h>
#include <commctrl.h>

// project header files
#include "KMIDropSource.h"
#include "resource.h"
#include "..\include\pgpImage.h"
#include "..\include\treelist.h"

extern "C" {
typedef struct _KM *PKEYMAN;
extern HINSTANCE g_hInst;
VOID KMFindWindowFromPoint (PKEYMAN pKM, POINT* ppt, HWND* phwnd);
}

//	________________________________________
//
//	IUnknown Methods

STDMETHODIMP CDropSource::QueryInterface (
		REFIID 			iid, 
		void FAR* FAR* 	ppv) 
{
	if ((iid == IID_IUnknown) || (iid == IID_IDropSource)) {
		*ppv = this;
		++m_refs;
		return NOERROR;
	}
	*ppv = NULL;

	return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CDropSource::AddRef (void) 
{
	return ++m_refs;
}


STDMETHODIMP_(ULONG) CDropSource::Release (void) 
{
	if (--m_refs == 0) {
		delete this;
		return 0;
	}
	return m_refs;
}  


//	________________________________________
//
//	calculate cursor offsets

static VOID
sCalcOffsets (
		HWND 	hwnd, 
		ULONG* 	pulXOff, 
		ULONG* 	pulYOff)
{
	NONCLIENTMETRICS	ncm;

	// get offset number to convert client to window coordinates
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo (SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
							&ncm, 0);
	*pulXOff = ncm.iBorderWidth;
	*pulYOff = ncm.iBorderWidth + ncm.iCaptionHeight;
	if (GetMenu (hwnd))
		*pulYOff += ncm.iMenuHeight;
}
 
//	________________________________________
//
//	CDropSource Constructor
 
CDropSource::CDropSource (
		VOID*	pKeyMan, 
		HWND	hwnd, 
		HWND	hwndTree) 
{
	HIMAGELIST			hIml;
	TL_DRAGBITMAP		tldb;

    m_refs = 1; 
	m_hwnd = hwnd;
	m_pKeyMan = pKeyMan;

	sCalcOffsets (hwnd, &m_ncxoffset, &m_ncyoffset);

	GetCursorPos (&tldb.ptCursorPos);
	ScreenToClient (hwndTree, &tldb.ptCursorPos);
	tldb.ulFlags = TLRB_TOPLEVELONLY;
	TreeList_RenderDragBitmap (hwndTree, &tldb);
	hIml = ImageList_Create (tldb.sizeDrag.cx, tldb.sizeDrag.cy, 
							ILC_MASK|ILC_COLORDDB, 1, 1);
	ImageList_AddMasked (hIml, tldb.hbmDrag, GetSysColor (COLOR_WINDOW));
	DeleteObject (tldb.hbmDrag);
 
	ImageList_BeginDrag (hIml, 0, tldb.ptHotSpot.x, tldb.ptHotSpot.y);
	ImageList_Destroy (hIml);

	GetCursorPos (&tldb.ptCursorPos);
	ScreenToClient (hwnd, &tldb.ptCursorPos);
	ImageList_DragEnter (hwnd, tldb.ptCursorPos.x + m_ncxoffset, 
								tldb.ptCursorPos.y + m_ncyoffset);
}

CDropSource::~CDropSource() 
{
	ImageList_DragLeave (m_hwnd);
	ImageList_EndDrag ();
}


//	________________________________________
//
//	IDropSource Methods

STDMETHODIMP CDropSource::QueryContinueDrag (
		BOOL	fEscapePressed, 
		DWORD	grfKeyState) 
{  
    if (fEscapePressed)
        return DRAGDROP_S_CANCEL;
    else if (!(grfKeyState & MK_LBUTTON) && !(grfKeyState & MK_RBUTTON))
        return DRAGDROP_S_DROP;
    else
        return NOERROR;                  
}

STDMETHODIMP CDropSource::GiveFeedback (DWORD dwEffect) 
{
	POINT	pt;
	HWND	hwnd;
	RECT	rc;

	GetCursorPos (&pt);

	GetWindowRect (m_hwnd, &rc);
	if (!PtInRect (&rc, pt)) {
		hwnd = NULL;
		KMFindWindowFromPoint ((PKEYMAN)m_pKeyMan, &pt, &hwnd);

		if (hwnd != m_hwnd) {
			ImageList_DragLeave (m_hwnd);
			m_hwnd = hwnd;
			if (m_hwnd) {
				sCalcOffsets (hwnd, &m_ncxoffset, &m_ncyoffset);
				ScreenToClient (m_hwnd, &pt);
				ImageList_DragEnter (m_hwnd, 
						pt.x + m_ncxoffset, pt.y + m_ncyoffset);
			}
		}
	}

	if (!(dwEffect & DROPEFFECT_SCROLL)) {
		ScreenToClient (m_hwnd, &pt);
		ImageList_DragMove (pt.x + m_ncxoffset, pt.y + m_ncyoffset);
	}

	return DRAGDROP_S_USEDEFAULTCURSORS;
}


//	________________________________________
//
//	Interface to C code 

extern "C" {
LPDROPSOURCE KMCreateDropSource (
		VOID* 	pKeyMan, 
		HWND 	hwnd, 
		HWND 	hwndTree) 
{
   return ((LPDROPSOURCE) new CDropSource (pKeyMan, hwnd, hwndTree));
}
}

