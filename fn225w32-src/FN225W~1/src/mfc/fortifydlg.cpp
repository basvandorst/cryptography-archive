/*
 # $Id: fortifyDlg.cpp,v 1.5 1998/03/10 11:39:42 fbm Exp fbm $
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
#include <sys/stat.h>
#include <time.h>
#include <process.h>
#include "stdafx.h"
#include "fortify.h"
#include "fortifyDlg.h"
#include "optionsDlg.h"
#include "options.h"
#include "BackupDlg.h"
#include "sysobj.h"
#include "misc.h"
#include "morpher.h"
#include "index.h"
#include "log.h"
#include "callback.h"
#include "msgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////


CAboutDlg::CAboutDlg(CString vern) : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	licenceText = _T("");
	titleText = _T("");
	//}}AFX_DATA_INIT

	titleText += Msgs_get("MPRODNAME");
	titleText += "; ";
	titleText += Msgs_get("MCAPVERSION");
	titleText += " ";
	titleText += vern;
	titleText += "\n";
	titleText += "Copyright (C) 1997-2000 Farrell McKay\n";
	titleText += "Wayfarer Systems Pty Ltd\n";
	titleText += Msgs_get("MRIGHTS");

	// The licence text box, being an edit field, needs
	// to have all linefeeds converted to CR/LF pairs.
	//
	CString tmp;
	tmp += Msgs_get("MTERMS");
	tmp += "\n";
	tmp += Msgs_get("MCOPYRIGHT");

	LPCTSTR s;
	for (s = (LPCTSTR) tmp; *s; s++) {
		if (*s == '\n')
			licenceText += "\r\n";
		else
			licenceText += *s;
	}
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_LICENCE2, licenceText);
	DDX_Text(pDX, IDC_ABOUT_TITLE, titleText);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CWnd *w;
	w = (CWnd *) GetDlgItem(IDOK);
	w->SetWindowText(Msgs_get("MCAPOK"));
	
	SetWindowText(Msgs_get("MABOUTDLG_TITLE"));

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CFortifyDlg dialog

CFortifyDlg::CFortifyDlg(char *_progDir, char *_vern, CWnd* pParent /*=NULL*/)
	: CDialog(CFortifyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFortifyDlg)
	pathName = _T("");
	helpText = _T("");
	selectedHdgText = _T("");
	//}}AFX_DATA_INIT

	m_hIcon = AfxGetApp()->LoadIcon(IDI_F);

	helpText = Msgs_get("MFORTIFYDLG_HELP");
	selectedHdgText = Msgs_get("MFORTIFYDLG_SELECTEDHDG");

	progDir = _progDir;
	vern = _vern;
}

void CFortifyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFortifyDlg)
	DDX_Text(pDX, IDC_PATHNAME, pathName);
	DDX_Text(pDX, IDC_SELECTORHELPTEXT, helpText);
	DDX_Text(pDX, IDC_FORTIFYDLG_SELECTEDHDG, selectedHdgText);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFortifyDlg, CDialog)
	//{{AFX_MSG_MAP(CFortifyDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST3, OnListSelect)
	ON_WM_SYSCOMMAND()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_OPTIONS, OnOptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFortifyDlg message handlers

BOOL CFortifyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		//strAboutMenu.LoadString(IDS_ABOUTBOX);
		strAboutMenu = Msgs_get("MABOUTDLG_TITLE");
		strAboutMenu += "...";
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	InitList();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CWnd *w;
	w = (CWnd *) GetDlgItem(IDC_FORTIFYDLG_GRPTEXT);
	w->SetWindowText(Msgs_get("MFORTIFYDLG_GRPTEXT"));
	w = (CWnd *) GetDlgItem(IDC_BROWSE);
	w->SetWindowText(Msgs_get("MBROWSEDLG"));
	w = (CWnd *) GetDlgItem(IDC_OPTIONS);
	w->SetWindowText(Msgs_get("MOPTIONSDLG"));
	w = (CWnd *) GetDlgItem(IDOK);
	w->SetWindowText(Msgs_get("MCAPPROCEED"));
	w = (CWnd *) GetDlgItem(IDCANCEL);
	w->SetWindowText(Msgs_get("MCAPEXIT"));

	SetWindowText(Msgs_get("MFORTIFYDLG_TITLE"));

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// data for the list view control

#define NUM_COLUMNS	3

static char *colLabelStrings[NUM_COLUMNS] = {
	"MFORTIFYDLG_COL1", "MFORTIFYDLG_COL2", "MFORTIFYDLG_COL3"
};

static int _gnColumnFmt[NUM_COLUMNS] = {
	LVCFMT_LEFT, LVCFMT_RIGHT, LVCFMT_LEFT
};

static int _gnColumnWidth[NUM_COLUMNS] = {
	410, 70, 100
};

void
CFortifyDlg::InitList(void) 
{
	CListCtrl *ListCtrl = (CListCtrl *) GetDlgItem(IDC_LIST3);

#if 0
	m_LargeImageList.Create(IDB_LARGEICONS, 32, 1, RGB(255, 255, 255));
	m_SmallImageList.Create(IDB_SMALLICONS, 16, 1, RGB(255, 255, 255));
	m_StateImageList.Create(IDB_STATEICONS, 16, 1, RGB(255, 0, 0));

	m_LargeImageList.SetOverlayImage(NUM_ITEMS, 1);
	m_SmallImageList.SetOverlayImage(NUM_ITEMS, 1);

	ListCtrl.SetImageList(&m_LargeImageList, LVSIL_NORMAL);
	ListCtrl.SetImageList(&m_SmallImageList, LVSIL_SMALL);
	ListCtrl.SetImageList(&m_StateImageList, LVSIL_STATE);
#endif

	int i;
	LV_COLUMN lvc;

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	for (i = 0; i < NUM_COLUMNS; i++)	{
		lvc.iSubItem = i;
		lvc.pszText = Msgs_get(colLabelStrings[i]);
		lvc.cx = _gnColumnWidth[i];
		lvc.fmt = _gnColumnFmt[i];
		ListCtrl->InsertColumn(i,&lvc);
	}

	if (!map.IsEmpty()) {
		CString	  pathname;
		fileAttrs *fp;

		POSITION pos = map.GetStartPosition();
		while (pos != NULL) {
			map.GetNextAssoc(pos, pathname, (void *&) fp);
			AddListRow(pathname, fp, FALSE);
		}
	}
}


void
CFortifyDlg::AddListRow(const char *pathName, fileAttrs *fp, BOOL doUpdate)
{
	CListCtrl *ListCtrl = (CListCtrl *) GetDlgItem(IDC_LIST3);
	LV_ITEM lvi;
	int	n;

	n = ListCtrl->GetItemCount();
	lvi.mask = LVIF_TEXT; // | LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = n;
	lvi.iSubItem = 0;
	lvi.pszText = (char *) pathName;
	//lvi.iImage = n;
	//lvi.stateMask = LVIS_STATEIMAGEMASK;
	//lvi.state = INDEXTOSTATEIMAGEMASK(1);

	ListCtrl->InsertItem(&lvi);
	ListCtrl->SetItemText(n, 1, fp->size);
	ListCtrl->SetItemText(n, 2, fp->datestamp);
	
	if (doUpdate)
		ListCtrl->Update(n);
}


void
CFortifyDlg::AddToList(const char *progName, BOOL updateListCtrl)
{
	struct stat	st;
	struct tm	*mtime;
	char		filesize[64];
	char		timestamp[64];

	if (stat(progName, &st) == 0) {
		Log(1, "t1>> Found executable: \"%s\"\n", progName);
		sprintf(filesize, "%d", st.st_size);

		mtime = localtime(&st.st_mtime);
		strftime(timestamp, sizeof(timestamp), "%x %X", mtime);
	
		void		*vp;
		fileAttrs	*fp = new fileAttrs;
		fp->size = filesize;
		fp->datestamp = timestamp;

		CString longName = ToLongName(progName);
		if (map.Lookup(longName, vp) == 0) {
			map.SetAt(longName, fp);
			if (updateListCtrl)
				AddListRow(longName, fp, TRUE);
		}
	}
}


CString
CFortifyDlg::ToLongName(CString pathName)
{
	const char *start = (LPCSTR) pathName;
	const char *end = start;
	CString longName;
	WIN32_FIND_DATA findData;
	HANDLE hFile;

	Log(4, "t4>> ToLongName: converting \"%s\"\n", (LPCSTR) pathName);

	// Drive letters in the path name.....
	if (pathName.GetLength() > 2 && start[1] == ':' && start[2] == '\\') {
		end += 2;
		longName += CString(start, end - start);
	}

	// Win95 and WinNT4 do not provide a GetLongFileName() function.
	// We have to convert from 8.3 pathnames to the long name the hard way.
	//
	while (*end) {
		if (*end == '\\')
			end++;
		while (*end && *end != '\\')
			end++;
		
		CString tmp(start, end - start);
		hFile = FindFirstFile((LPCTSTR) tmp, (LPWIN32_FIND_DATA) &findData);
		if (hFile == INVALID_HANDLE_VALUE) {
			Log(4, "t4>> ToLongName: failed to convert \"%s\"\n", (LPCSTR) tmp);
			longName = pathName;
			break;
		}
		if (!longName.IsEmpty())
			longName += '\\';
		longName += findData.cFileName;
	}

	longName.MakeLower();
	Log(4, "t4>> ToLongName: returning \"%s\"\n", (LPCSTR) longName);
	return longName;
}


void CFortifyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)	{
		CAboutDlg dlgAbout(vern);
		dlgAbout.DoModal();
	}
	else {
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CFortifyDlg::OnDestroy()
{
	WinHelp(0L, HELP_QUIT);
	CDialog::OnDestroy();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFortifyDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else {
		CDialog::OnPaint();
	}
}

void
CFortifyDlg::OnListSelect(LPNMHDR lParam, LRESULT *pResult)
{
	NM_LISTVIEW * pnmv = (NM_LISTVIEW *) lParam; 

	if ((pnmv->uNewState & LVIS_SELECTED) == 0)
		return;

	Log(9, "t9>> List item %d selected\n", pnmv->iItem);

	CListCtrl *ListCtrl = (CListCtrl *) GetDlgItem(IDC_LIST3);
	CString selectedPath = ListCtrl->GetItemText(pnmv->iItem, 0);

	Log(9, "t9>> List item %d text \"%s\"\n", pnmv->iItem, selectedPath);

	pathName = selectedPath;
	UpdateData(FALSE);
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFortifyDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CFortifyDlg::OnBrowse() 
{
	// static char BASED_CODE filter[] = "Executable files (*.exe)|*.exe|Executable files, backup files (*.exe;*.sav)|*.exe;*.sav|All Files (*.*)|*.*||";

	char			*filter = Msgs_get("MFORTIFYDLG_BROWSEFILTER");
	CFileDialog		dlg(TRUE, "exe", "netscape.exe", OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR, filter);

	int answer = dlg.DoModal();
	if (answer == IDOK) {
		pathName = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

void CFortifyDlg::OnOptions() 
{
	OptionsDlg	dlg;
	
	int	answer = dlg.DoModal();
}


void CFortifyDlg::OnOK() 
{
	//HCURSOR hCursor;
  	//Load the predefined Windows "up arrow" cursor.
	//hCursor = AfxGetApp()->LoadStandardCursor(IDC_WAIT);

	UpdateData(TRUE);
	Morph(pathName);
}

int
CFortifyDlg::GetCount(void)
{
	return map.GetCount();
}


int
CFortifyDlg::Morph(CString tgt)
{
	int				new_grade, err;
	char			dflt_morphs[1024];
	char			tmpbuf[8192];
	CString			msg, prompt;
	int				answer;
	char			*prod;
	char			*action;
	tgt_info_t		tgt_info;
	index_entry_t	*ent;
	extern sysObj	*sys;

	if (tgt.IsEmpty()) {
		report(R_ERR, Msgs_get("MEMPTYSELECTIONRESPONSE"));
		return 1;
	}

	if (options_noprompts())
		Log(0, "t0>> *** 'noprompts' mode enabled - user confirmations are assumed to be 'yes'.\n");
	if (options_nowrite())
		Log(0, "t0>> *** 'nowrite' mode enabled - no modifications will be made.\n");
	if (options_noreadonly())
		Log(0, "t0>> *** 'noreadonly' mode enabled - read-only attribute will be overridden.\n");

	AddToList((LPCSTR) tgt, TRUE);

	if (!options_nowrite()) {
		int				isRunning;
		static int		ntrys = 0;
		int				repeatCheck = (ntrys > 0);

		// Check that there is no running browser on the machine.
		// On Win32 it is not possible to concurrently run, say, a 4.0x
		// and a 4.5 browser.  To avoid confusion during verification,
		// it is therefore necessary to require that no netscape.exe
		// processes are running.
		// The retry logic is an attempt to cater for the long time it takes
		// for the netscape.exe process to disappear from the process table.
		//
		while ((isRunning = sys->procIsRunning("netscape.exe")) && ntrys > 0) {
			Sleep(500);
			ntrys--;
		}
		ntrys = 4;
		if (isRunning) {
			msg = Msgs_get("MISRUNNING");
			if (repeatCheck) {
				msg += "\n";
				msg += Msgs_get("MISRUNNINGNOTE");
			}
			report(R_ERR, msg);
			return 1;
		}
	}

	sprintf(tmpbuf, Msgs_get("MFILEIS"), tgt);
	msg = tmpbuf;
	err = index_lookup((LPCSTR) tgt, &tgt_info);
	ent = tgt_info.ip;
	if (ent == NULL) {
		if (err == ERR_OPEN) {
			msg += Msgs_get("MNOTRECOG");
			msg += "\n";
			sprintf(tmpbuf, Msgs_get("MFOPENERR"), tgt, syserr());
			msg += tmpbuf;
		}
		else if (err == ERR_LXCOMPR) {
			msg += "\n";
			sprintf(tmpbuf, Msgs_get("MLXCOMPR"), tgt);
			msg += tmpbuf;
		}
		else if (err == ERR_ISSCRIPT) {
			msg += Msgs_get("MNOTRECOG");
			msg += "\n";
			msg += Msgs_get("MISSCRIPT");
		}
		else {
			msg += Msgs_get("MNOTRECOG");
			msg += "\n";
			msg += Msgs_get("MNOTNETSC");
			msg += "\n";
			msg += Msgs_get("MNOTNETSCHELP");
		}
		report(R_ERR, msg);
		return 1;
	}

	if (strcmp(ent->flds[IDX_PROD], "nav") == 0)
		prod = Msgs_get("MPRODNAV");
	else if (strcmp(ent->flds[IDX_PROD], "gold") == 0)
		prod = Msgs_get("MPRODGOLD");
	else if (strcmp(ent->flds[IDX_PROD], "comm") == 0)
		prod = Msgs_get("MPRODCOMM");
	else if (strcmp(ent->flds[IDX_PROD], "pro") == 0)
		prod = Msgs_get("MPRODPRO");
	else {
		sprintf(tmpbuf, Msgs_get("MPRODUNKN"), ent->flds[IDX_PROD]);
		msg += tmpbuf;
		report(R_ERR, msg);
		return 1;
	}

	sprintf(tmpbuf, Msgs_get("MPRODDESC"),
		prod, ent->flds[IDX_VERN], ent->flds[IDX_ARCH],
		ent->flds[IDX_GRADE], ent->flds[IDX_COMMENTS]);
	msg += "\n\n";
	msg += tmpbuf;
		
	if (!have_morphs(ent)) {
		sprintf(tmpbuf, Msgs_get("MVERUNSUPP"), vern);
		msg += "\n\n";
		msg += tmpbuf;
		report(R_ERR, msg);
		return 1;
	}

	if (ent->grade == ent->max_grade) {
		if (!options_nowrite()) {
			msg += "\n\n";
			msg += Msgs_get("MDODEFORTIFY");
			answer = confirm(msg);
			if (answer == IDNO || answer == IDCANCEL) {
				Log(0, "t0>> As you wish.  \"%s\" not modified.\n", tgt);
				return 0;
			}
		}
		new_grade = 0;
		action = Msgs_get("MACTIONDEFORTIFIED");
	}
	else if (ent->grade == 0) {
		if (!options_nowrite()) {
			msg += "\n\n";
			msg += Msgs_get("MDOFORTIFY");
			answer = confirm(msg);
			if (answer == IDNO || answer == IDCANCEL) {
				Log(0, "t0>> As you wish.  \"%s\" not modified.\n", tgt);
				return 0;
			}
		}
		new_grade = ent->max_grade;
		action = Msgs_get("MACTIONFORTIFIED");
	}
	else {
		msg += "\n\n";
		msg += Msgs_get("MSSLHELPYESNO");
		answer = confirm(msg);

		if (answer == IDYES) {
			new_grade = ent->max_grade;
			action = Msgs_get("MACTIONUPGRADED");
		}
		if (answer == IDNO) {
			new_grade = 0;
			action = Msgs_get("MACTIONDEFORTIFIED");
		}
		if (answer == IDCANCEL) {
			Log(0, "t0>> As you wish.  \"%s\" not modified.\n", tgt);
			return 0;
		}
	}

	if (!options_nowrite()) {
		sprintf(tmpbuf, Msgs_get("MDOBACKUP"), pathName);
		msg = tmpbuf;
		answer = confirm(msg);
		if (answer == IDYES) {
			BackupDlg	bkup(tgt);
			Sleep(100);
			answer = bkup.run();
			Sleep(500);
		}
		if (answer == IDCANCEL)
			return 0;
	}

	DWORD currAttrs, newAttrs;
	BOOL attrsChanged = FALSE;

	if (options_noreadonly()) {
		currAttrs = GetFileAttributes((LPCTSTR) tgt);
		if (currAttrs != ~0 && (currAttrs & FILE_ATTRIBUTE_READONLY)) {
			Log(4, "t4>> Overriding read-only attribute. [0x%x]\n", currAttrs);
			newAttrs = currAttrs & (~FILE_ATTRIBUTE_READONLY);
			attrsChanged = SetFileAttributes((LPCTSTR) tgt, newAttrs);
			Log(4, "t4>> SetFileAttrs rtn: %d [0x%x]\n", attrsChanged, newAttrs);
		}
	}

	sprintf(dflt_morphs, "%s/%s/%s-%s/%s",
			progDir,
			ent->flds[IDX_ARCH], ent->flds[IDX_PROD],
			ent->flds[IDX_VERN], ent->flds[IDX_MORPHS]);

	err = morpher((LPCSTR) tgt, dflt_morphs, &tgt_info, ent->grade, new_grade, action);

	if (options_noreadonly() && attrsChanged)
		SetFileAttributes((LPCTSTR) tgt, currAttrs);

	if (!err && !options_nowrite() && (new_grade > ent->grade)) {
		sprintf(tmpbuf, Msgs_get("MDOTEST"), tgt);
		msg = tmpbuf;
		answer = confirm(msg);
		if (answer == IDYES) {
			CString quo = "\"" + tgt + "\"";
			CString arg = "https://www.fortify.net/sslcheck.html?fn-" + vern + "-mfc";
			spawnl(_P_NOWAIT, tgt, quo, (LPCSTR) arg, NULL);
			report(R_INFO, Msgs_get("MCONNECTING"), tgt);
		}
	}

	return err;
}

