/*
 # $Id: optionsDlg.cpp,v 1.5 1998/03/10 11:39:42 fbm Exp fbm $
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
#include "options.h"
#include "OptionsDlg.h"
#include "log.h"
#include "msgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

OptionsDlg::OptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(OptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(OptionsDlg)
	logging = _T("");
	noprompts = FALSE;
	nowrite = FALSE;
	noreadonly = FALSE;
	//}}AFX_DATA_INIT

	logging = Msgs_get("MLOGGINGHEADING");
	if (!Log_enabled()) {
		logging += Msgs_get("MLOGGINGOFF");
	}
	else {
		char	tmpbuf[1024];

		sprintf(tmpbuf, Msgs_get("MLOGGINGON"), options_getLogFile());
		logging += tmpbuf;
	}

	noprompts = options_noprompts();
	nowrite = options_nowrite();
	noreadonly = options_noreadonly();
}


void OptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(OptionsDlg)
	DDX_Text(pDX, IDC_OPTIONS_LOGGING, logging);
	DDX_Check(pDX, IDC_OPTIONS_NOPROMPTS, noprompts);
	DDX_Check(pDX, IDC_OPTIONS_NOWRITE, nowrite);
	DDX_Check(pDX, IDC_OPTIONS_NOREADONLY, noreadonly);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(OptionsDlg, CDialog)
	//{{AFX_MSG_MAP(OptionsDlg)
	ON_BN_CLICKED(IDC_OPTIONS_NOREADONLY, OnOptionsNoreadonly)
	ON_BN_CLICKED(IDC_OPTIONS_NOWRITE, OnOptionsNowrite)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OptionsDlg message handlers

BOOL OptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CWnd *w;
	w = (CWnd *) GetDlgItem(IDC_OPTIONS_GRPTEXT);
	w->SetWindowText(Msgs_get("MOPTIONSDLG_GRPTEXT"));
	w = (CWnd *) GetDlgItem(IDC_OPTIONS_NOPROMPTS);
	w->SetWindowText(Msgs_get("MOPTIONSDLG_NOPROMPTS"));
	w = (CWnd *) GetDlgItem(IDC_OPTIONS_NOWRITE);
	w->SetWindowText(Msgs_get("MOPTIONSDLG_NOWRITE"));
	w = (CWnd *) GetDlgItem(IDC_OPTIONS_NOREADONLY);
	w->SetWindowText(Msgs_get("MOPTIONSDLG_NOREADONLY"));
	w = (CWnd *) GetDlgItem(IDOK);
	w->SetWindowText(Msgs_get("MCAPOK"));
	w = (CWnd *) GetDlgItem(IDCANCEL);
	w->SetWindowText(Msgs_get("MCAPCANCEL"));

	SetWindowText(Msgs_get("MOPTIONSDLG_TITLE"));

	return TRUE;
}

void OptionsDlg::OnOK() 
{
	UpdateData(TRUE);
	options_setNoWrite(nowrite);
	options_setNoPrompts(noprompts);
	options_setNoReadOnly(noreadonly);
	CDialog::OnOK();
}

void OptionsDlg::OnOptionsNoreadonly() 
{
	UpdateData(TRUE);
	if (noreadonly) {
		nowrite = FALSE;
		UpdateData(FALSE);
	}
}

void OptionsDlg::OnOptionsNowrite() 
{
	UpdateData(TRUE);
	if (nowrite) {
		noreadonly = FALSE;
		UpdateData(FALSE);
	}
}

int OptionsDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}
