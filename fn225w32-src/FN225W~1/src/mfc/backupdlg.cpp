/*
 # $Id: backupdlg.cpp,v 1.5 1998/03/10 11:39:42 fbm Exp fbm $
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

#include <sys/types.h>
#include "stdafx.h"
#include "fortify.h"
#include "BackupDlg.h"
#include "ProgressDlg.h"
#include "cp.h"
#include "log.h"
#include "msgs.h"
#include "callback.h"
#include "misc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BackupDlg::BackupDlg(CString _pathName)
{
	pathName = _pathName;
	
	backupName = pathName;
	int len = pathName.GetLength();
	if (len > 4 && pathName.Right(4) == ".exe")
		backupName = CString((LPCSTR) pathName, len - 4);
	backupName += ".sav";
}

#define PBAR_RANGE		50

void
backupMeter(void *vp, int range, int level)
{
	CProgressCtrl *ProgCtrl = (CProgressCtrl *) vp;

	ProgCtrl->SetPos(level);
}

int
BackupDlg::run()
{
	char	*title;
	char	prompt[4096];

	title = Msgs_get("MBACKUPTITLE");
	sprintf(prompt, Msgs_get("MBACKUPTO2"), backupName);

	ProgressDlg	dlg(title, prompt);

	CProgressCtrl *ProgCtrl = (CProgressCtrl *) dlg.GetDlgItem(IDC_PROGRESSMETER);
	ProgCtrl->SetPos(0);
	ProgCtrl->SetRange(0, PBAR_RANGE);

	Log(1, "t1>> Writing backup copy to \"%s\"\n", (LPCSTR) backupName);
	int rtn = cp(pathName, backupName, 0, backupMeter, (void *) ProgCtrl, PBAR_RANGE);
	if (rtn == 0) {
		Log(1, "t1>> Backup ok.\n");
	}

	dlg.DestroyWindow();
	return (rtn == 0)? IDOK: IDCANCEL;
}
