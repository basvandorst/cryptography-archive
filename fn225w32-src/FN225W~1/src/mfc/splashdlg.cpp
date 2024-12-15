/*
 # $Id: splashDlg.cpp,v 1.5 1998/03/10 11:39:42 fbm Exp fbm $
 # Copyright (C) 1997-2000 Farrell McKay
 # All rights reserved.
 #
 # This file is part of the Fortify distribution, a toolkit for
 # upgrading the cryptographic strength of the Netscape web browsers.
 #
 # This toolkit is provided to the recipient under the
 # following terms and conditions:-
 #   1.  This copyright notice must not be removed or modified.
 #   2.  This toolkit may not be reproduced or included in any commercial
 #       media distribution, or commercial publication (for example CD-ROM,
 #       disk, book, magazine, journal) without first obtaining the author's
 #       express permission.
 #   3.  This toolkit, or any component of this toolkit, may not be
 #       used, resold, redeveloped, rewritten, enhanced or otherwise
 #       employed by a commercial organisation, or as part of a commercial
 #       venture, without first obtaining the author's express permission.
 #   4.  Subject to the above conditions being observed (1-3), this toolkit
 #       may be freely reproduced or redistributed.
 #   5.  To the extent permitted by applicable law, this software is
 #       provided "as-is", without warranty of any kind, including
 #       without limitation, the warrantees of merchantability,
 #       freedom from defect, non-infringement, and fitness for
 #       purpose.  In no event shall the author be liable for any
 #       direct, indirect or consequential damages however arising
 #       and however caused.
 #   6.  Subject to the above conditions being observed (1-5),
 #       this toolkit may be used at no cost to the recipient.
 #
 # Farrell McKay
 # Wayfarer Systems Pty Ltd		contact@fortify.net
 */

#include "stdafx.h"
#include "fortify.h"
#include "fortifydlg.h"
#include "log.h"
#include "msgs.h"
#include "options.h"
#include "optionsDlg.h"
#include "SplashDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TIMER_ID_SPLASH	42


SplashDlg::SplashDlg(char *_vern, CWnd* pParent /*=NULL*/)
	: CDialog(SplashDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(SplashDlg)
	text1 = _T("");	
	//}}AFX_DATA_INIT

	m_hIcon = AfxGetApp()->LoadIcon(IDI_F);
	
	vern = _vern;

	text1 += "\n";
	text1 += Msgs_get("MPRODNAME");
	text1 += "\n";
	text1 += Msgs_get("MCAPVERSION");
	text1 += " ";
	text1 += vern;
	text1 += "\n";

	text1 += "\n";
	text1 += "\n";
	text1 += "Copyright (C) 1997-2000 Farrell McKay\n";
	text1 += "Wayfarer Systems Pty Ltd\n";
	text1 += Msgs_get("MRIGHTS");

	if (strchr(vern, 'b') || strchr(vern, 'B')) {
		text1 += "\n";
		text1 += Msgs_get("MNOREDISTRIB");
		text1 += "\n";
	}

	text1 += "\n";
	text1 += Msgs_get("MTERMSSPLASHDLG");

	Log(0, "t0>> ====   Fortify %s; Copyright (C) 1997-2000 Farrell McKay   ====\n", vern);
	Log(0, "t0>> %s", Msgs_get("MTERMS"));

	GetBitmapAndPalette(IDB_SPLASH2, bitmap, palette);
}


void SplashDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SplashDlg)
	DDX_Text(pDX, IDC_SPLASH_TEXT1, text1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SplashDlg, CDialog)
	//{{AFX_MSG_MAP(SplashDlg)
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_SPLASH_ABOUT, OnAbout)
	ON_BN_CLICKED(IDC_SPLASH_OPTIONS, OnOptions)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_CAPTURECHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void SplashDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	//EndDialog(0);
	CDialog::OnLButtonDown(nFlags, point);
}

BOOL SplashDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	//SetTimer(TIMER_ID_SPLASH, 5000, NULL);

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CWnd *w;	
	w = (CWnd *) GetDlgItem(IDC_SPLASH_ABOUT);
	w->SetWindowText(Msgs_get("MABOUTDLG"));
	w = (CWnd *) GetDlgItem(IDOK);
	w->SetWindowText(Msgs_get("MCAPSTART"));
	w = (CWnd *) GetDlgItem(IDCANCEL);
	w->SetWindowText(Msgs_get("MCAPEXIT"));

	SetWindowText(Msgs_get("MSPLASHDLGTITLE"));

	return TRUE;
}

void SplashDlg::OnOptions() 
{
	OptionsDlg	dlg;
	dlg.DoModal();
}

void SplashDlg::OnAbout() 
{
	CAboutDlg dlg(vern);
	dlg.DoModal();
}

void SplashDlg::OnPaint() 
{
	CDialog::OnPaint();

	CRect rcWnd;
	CWnd *pWnd = (CWnd *) GetDlgItem(IDC_SPLASHDLG_ZONE);
	//CDC *pc = pWnd->GetDC();

	CPaintDC dc(pWnd);

    CDC memDC;
    memDC.CreateCompatibleDC(&dc);

#if 0
	int mode = memDC.GetStretchBltMode();
	if (mode == STRETCH_ANDSCANS)
		Log(4, "memDC is STRETCH_ANDSCANS\n");
	else if (mode == STRETCH_DELETESCANS)
		Log(4, "memDC is STRETCH_DELETESCANS\n");
	else if (mode == STRETCH_ORSCANS)
		Log(4, "memDC is STRETCH_ORSCANS\n");
	else if (mode == HALFTONE)
		Log(4, "memDC is HALFTONE\n");
	else
		Log(4, "memDC mode is unknown\n");
#endif

	dc.SetStretchBltMode(STRETCH_DELETESCANS);
	// dc.SetStretchBltMode(HALFTONE);
	// dc.SetBrushOrg(0, 0);

    CBitmap *pBmpOld = memDC.SelectObject(&bitmap);

    // Select and realize the palette
    if (dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE && palette.m_hObject != NULL) {
        dc.SelectPalette(&palette, FALSE);
        dc.RealizePalette();
    }

	pWnd->Invalidate();
	pWnd->UpdateWindow();
	pWnd->GetWindowRect(&rcWnd);
	
	static long firstPaint = 0;
	if (firstPaint++ == 0)
		Log(4, "t4>> OnPaint width=%d height=%d\n", rcWnd.Width(), rcWnd.Height());
	
	//pc->BitBlt(0, 0, rcWnd.Width(), rcWnd.Height(), &memDC, 0, 0,SRCCOPY);
	//dc.BitBlt(0, 0, rcWnd.Width(), rcWnd.Height(), &memDC, 0, 0,SRCCOPY);
	int x, y, w, h;
	x = y = 0;
	w = rcWnd.Width();
	h = rcWnd.Height();
	if (w > bitmapWidth) {
		x = (w - bitmapWidth) / 2;
		w = bitmapWidth;
	}
	if (h > bitmapHeight) {
		y = (h - bitmapHeight) / 2;
		h = bitmapHeight;
	}
	dc.StretchBlt(0, 0, rcWnd.Width(), rcWnd.Height(), &memDC,
		0, 0, 2, bitmapHeight, SRCCOPY);
	dc.StretchBlt(x, y, w, h, &memDC,
		0, 0, bitmapWidth, bitmapHeight, SRCCOPY);
	//pWnd->ReleaseDC(pc);
	
    // Restore bitmap in memDC
    memDC.SelectObject(pBmpOld);
}

void SplashDlg::OnTimer(UINT nIDEvent) 
{
	KillTimer(TIMER_ID_SPLASH);
	EndDialog(0);
	CDialog::OnTimer(nIDEvent);
}

void SplashDlg::OnCaptureChanged(CWnd *pWnd) 
{
	CDialog::OnCaptureChanged(pWnd);
}

BOOL SplashDlg::GetBitmapAndPalette(UINT nIDResource, CBitmap &bitmap, CPalette &pal)
{
    LPCTSTR lpszResourceName = (LPCTSTR) nIDResource;

    HBITMAP hBmp = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), 
                        lpszResourceName, IMAGE_BITMAP, 0,0, LR_CREATEDIBSECTION);

    if (hBmp == NULL) 
        return FALSE;

    bitmap.Attach(hBmp);

    // Create a logical palette for the bitmap
    DIBSECTION ds;
	BITMAP &bm = ds.dsBm;
    BITMAPINFOHEADER &bmInfo = ds.dsBmih;
    bitmap.GetObject(sizeof(ds), &ds);
	bitmapWidth = bm.bmWidth;
	bitmapHeight = bm.bmHeight;

    int nColors = bmInfo.biClrUsed ? bmInfo.biClrUsed : 1 << bmInfo.biBitCount;

    // Create a halftone palette if colors > 256. 
    CClientDC dc(NULL);                     // Desktop DC
    if (nColors > 256)
        pal.CreateHalftonePalette(&dc);
    else {
        // Create the palette
        RGBQUAD *pRGB = new RGBQUAD[nColors];
        CDC memDC;
        memDC.CreateCompatibleDC(&dc);

        memDC.SelectObject(&bitmap);
        ::GetDIBColorTable(memDC, 0, nColors, pRGB);

        UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
        LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];

        pLP->palVersion = 0x300;
        pLP->palNumEntries = nColors;

        for (int i = 0; i < nColors; i++) {
             pLP->palPalEntry[i].peRed = pRGB[i].rgbRed;
             pLP->palPalEntry[i].peGreen = pRGB[i].rgbGreen;
             pLP->palPalEntry[i].peBlue = pRGB[i].rgbBlue;
             pLP->palPalEntry[i].peFlags = 0;
        }

        pal.CreatePalette(pLP);

        delete [] pLP;
        delete [] pRGB;
    }
    return TRUE;
}
