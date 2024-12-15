/*
 # $Id: fortify.cpp,v 1.5 1998/03/10 11:39:42 fbm Exp fbm $
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
#include <sys/types.h>
#include <ctype.h>
#include <direct.h>
#include <string.h>
#include <time.h>
#include "fortify.h"
#include "splashDlg.h"
#include "fortifyDlg.h"
#include "options.h"
#include "BackupDlg.h"
#include "sysobj.h"
#include "misc.h"
#include "morpher.h"
#include "index.h"
#include "log.h"
#include "msgs.h"
#include "callback.h"
#include <iostream.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFortifyApp

BEGIN_MESSAGE_MAP(CFortifyApp, CWinApp)
	//{{AFX_MSG_MAP(CFortifyApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


CFortifyApp::CFortifyApp()
{
}


CFortifyApp theApp;
sysObj		*sys;


BOOL CFortifyApp::InitInstance()
{
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
		
	char *progDir = getProgDir((char *) GetCommandLine());

	DoProgArgs();
	Log_init(options_getLogFile());
	Log_level(9);

	char		msgsfile[_MAX_PATH];

	sprintf(msgsfile, "%s/Messages", progDir);
	Msgs_init(msgsfile);

	options_setNoReadOnly(1);

	time_t tm;
	time(&tm);
	Log(0, "\n");
	Log(1, "t1>> %s", ctime(&tm));

	char currDir[_MAX_PATH];
	getcwd(currDir, (int) sizeof(currDir));
	Log(1, "t1>> Current dir: \"%s\"\n", currDir);

	int			answer;
	char		index[_MAX_PATH];
	
	sprintf(index, "%s/Index", progDir);
	ffy_vern  = "";
	if (build_index(index, (char **) &ffy_vern) == -1)
		exit(1);

	SplashDlg splash(ffy_vern);
	answer = splash.DoModal();
	if (answer == IDCANCEL)
		return FALSE;

	sys = new sysObj();
	selector = new CFortifyDlg(progDir, ffy_vern);
	checkRegistry();
	
	for (;;) {
		answer = selector->DoModal();
		if (answer == IDCANCEL)
			return FALSE;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


void
CFortifyApp::DoProgArgs(void)
{
	char *cmdLine = (char *) GetCommandLine();
	char *p = cmdLine;

	p = nextToken(p);
	if ((p[0] == '/' || p[0] == '-') && tolower(p[1]) == 'l') {
		p = nextToken(p);
		options_setLogFile((*p)? p: "fortify.log");
	}
}


void
CFortifyApp::checkDir(const char *dirName)
{
	char	progName[_MAX_PATH];

	strcpy(progName, dirName);
	strcat(progName, "\\netscape.exe");
	selector->AddToList(progName, FALSE);
	
	strcpy(progName, dirName);
	strcat(progName, "\\Program\\netscape.exe");
	selector->AddToList(progName, FALSE);
}

void
CFortifyApp::installDirKey(HKEY hKey, char *name)
{
	// Iter: HKEY_LOCAL_MACHINE\\SOFTWARE\\Netscape\\Netscape Navigator\\4.02 (en)\\Main
	//	key = "Install Directory"
	//  then add "\\Program\\netscape.exe"

	int			rtn;
	HKEY		subKey;
	DWORD		sz;
	char		installDir[_MAX_PATH];

	strcat(name, "\\Main");
	rtn = RegOpenKeyEx(hKey, name, 0, KEY_READ, &subKey);
	if (rtn == ERROR_SUCCESS) {
		sz = _MAX_PATH;
		rtn = RegQueryValueEx(subKey, "Install Directory", NULL, NULL, (unsigned char *) installDir, &sz);
		if (rtn == ERROR_SUCCESS) {
			Log(1, "t1>> Got InstallDir registry key \"%s\"\n", installDir);
			checkDir((const char *) installDir);
		}
		RegCloseKey(subKey);
	}
}

void
CFortifyApp::deinstallDirKey(HKEY hKey, char *name)
{
	// Iter: HKEY_LOCAL_MACHINE\\Software\\Netscape\\Deinstall\\Netscape Navigator"
	// Iter: HKEY_LOCAL_MACHINE\\Software\\Netscape\\Deinstall\\Netscape Communicator"
	// Iter: HKEY_LOCAL_MACHINE\\Software\\Netscape\\Deinstall\\Netscape Communicator Lite"
	//
	// For each version subkey look at Value name "Deinstall File Path"
	// Remove "\ISU" from value, then call checkDir()

	int			rtn, len;
	HKEY		subKey;
	DWORD		sz;
	char		deinstallPath[_MAX_PATH];

	rtn = RegOpenKeyEx(hKey, name, 0, KEY_READ, &subKey);
	if (rtn == ERROR_SUCCESS) {
		sz = _MAX_PATH;
		rtn = RegQueryValueEx(subKey, "Deinstall File Path", NULL, NULL, (unsigned char *) deinstallPath, &sz);
		if (rtn == ERROR_SUCCESS) {
			len = strlen(deinstallPath);
			if (len > 4 && strcmp(deinstallPath + len - 4, "\\ISU") == 0) {
				deinstallPath[len - 4] = '\0';
				Log(1, "t1>> Got DeinstallPath registry key \"%s\"\n", deinstallPath);
				checkDir((const char *) deinstallPath);
			}
		}
		RegCloseKey(subKey);
	}
}

void
CFortifyApp::iterateRegistryKey(LPCSTR keyName, pIterCallback fn)
{
	//"\\HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Netscape.exe"
	//	<noname> = ....
	//	Path = ....

	HKEY		hKey;
	int			rtn;

	rtn = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyName, 0, KEY_READ, &hKey);
	if (rtn == ERROR_SUCCESS) {

		DWORD		index;
		char		name[1024];
		DWORD		nameLen;
		FILETIME	tm;
		
		for (index = 0;; index++) {
			nameLen = sizeof(name);
			rtn = RegEnumKeyEx(hKey, index, 
						(char *) name, (LPDWORD) &nameLen,
						NULL, NULL,	NULL, (PFILETIME) &tm
					);
		
			if (rtn != ERROR_SUCCESS)
				break;

			(this->*fn)(hKey, name);
		}
	}
	RegCloseKey(hKey);
}

void
CFortifyApp::checkRegistry(void)
{
	// Firstly, check for the current version entry in the registry:
	//
	LPCSTR		subKeyName = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Netscape.exe";
	HKEY		subKey;
	DWORD		sz;
	int			rtn;
	char		value[_MAX_PATH];

	rtn = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subKeyName, 0, KEY_READ, &subKey);
	if (rtn == ERROR_SUCCESS) {
		sz = _MAX_PATH;
		rtn = RegQueryValueEx(subKey, "Path", NULL, NULL, (unsigned char *) value, &sz);
		if (rtn == ERROR_SUCCESS) {
			Log(1, "t1>> Got CurrentVersion registry key \"%s\"\n", value);
			checkDir((const char *) value);
		}
		RegCloseKey(subKey);
	}

	iterateRegistryKey("SOFTWARE\\Netscape\\Netscape Navigator", installDirKey);
	iterateRegistryKey("Software\\Netscape\\Deinstall\\Netscape Navigator", deinstallDirKey);
	iterateRegistryKey("Software\\Netscape\\Deinstall\\Netscape Communicator", deinstallDirKey);
	iterateRegistryKey("Software\\Netscape\\Deinstall\\Netscape Communicator Lite", deinstallDirKey);
}

