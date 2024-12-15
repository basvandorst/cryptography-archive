/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLprefs.c - handle PGP preferences dialogs
	

	$Id: CLprefs.c,v 1.77 1999/05/18 19:41:35 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"
#include <direct.h>
#include <shlobj.h>

// project header files
#include "pgpclx.h"
#include "pgpClientLib.h"
#include "pgpHotKeys.h"
#include "pgpTrayIPC.h"

// system header files
#include <commdlg.h>

#define ENABLE_TWOFISH			FALSE

#define MAXCACHETIMELIMIT		360000
#define MAXWIPEPASSES			32

#define PREFSEMAPHORENAME	("PGPprefsInstSem")

// default preferences
#define DEFAULT_ENCRYPTTOSELF			FALSE
#define DEFAULT_DECRYPTCACHEENABLE		TRUE
#define DEFAULT_DECRYPTCACHESECONDS		120
#define DEFAULT_SIGNCACHEENABLE			FALSE
#define DEFAULT_SIGNCACHESECONDS		120
#define DEFAULT_USEFASTKEYGEN			TRUE
#define	DEFAULT_WIPECONFIRM				TRUE
#define DEFAULT_MAILENCRYPTPGPMIME		FALSE
#define DEFAULT_USESECUREVIEWER			FALSE
#define DEFAULT_WORDWRAPWIDTH			70
#define DEFAULT_WORDWRAPENABLE			TRUE
#define DEFAULT_MAILENCRYPTDEFAULT		FALSE
#define DEFAULT_MAILSIGNDEFAULT			FALSE
#define DEFAULT_AUTODECRYPT				FALSE
#define DEFAULT_KEYSERVERSYNCONUNKNOWN	FALSE
#define DEFAULT_KEYSERVERSYNCONADDNAME	FALSE
#define DEFAULT_KEYSERVERSYNCONKEYSIGN	FALSE
#define DEFAULT_KEYSERVERSYNCONREVOKE	FALSE
#define DEFAULT_KEYSERVERSYNCONVERIFY	FALSE
#define DEFAULT_PREFERREDALGORITHM		kPGPCipherAlgorithm_CAST5
#define DEFAULT_DISPLAYMARGVALIDITY		TRUE
#define DEFAULT_MARGVALIDISINVALID		TRUE
#define DEFAULT_WARNONADK				TRUE
#define DEFAULT_WIPEPASSES				1
#define DEFAULT_EXPORTCOMPATIBLE		TRUE

#define COMMENTBLOCK_MAX_LENGTH			60

// external globals
extern HINSTANCE g_hInst;
extern CHAR g_szHelpFile[MAX_PATH];

// local globals
static PGPContextRef	ContextRef;

static HWND				hWndParent;
static HWND				hWndTree;
static HWND				hWndFocus;
static PGPKeySetRef		keysetMain;
static PGPError			returnErr;
static BOOL				bNeedsCentering;
static BOOL				bPrefsWritten;
static BOOL				bReloadKeyring;
static BOOL				bKeyserverPrefsWritten;
static BOOL				bUrlModified;
static BOOL				bRevUrlModified;
static PGPPrefRef		PrefRefClient;
static PGPPrefRef		PrefRefAdmin;
static CHAR				szInitDir[MAX_PATH];

typedef struct {
	PGPKeyRef	keyCert;
	PGPSigRef	sigCert;
} CAPREFSSTRUCT, *PCAPREFSSTRUCT;

typedef struct {
	HWND		hwndDecryptDuration;
	SYSTEMTIME	stDecryptDuration;
	HWND		hwndSignDuration;
	SYSTEMTIME	stSignDuration;
} CACHEDURATIONSTRUCT, *PCACHEDURATIONSTRUCT;


static DWORD aGeneralIds[] = {			// Help IDs
    IDC_ENCRYPTTOSELF,		IDH_PGPCLPREF_ENCRYPTTOSELF, 
	IDC_USEFASTKEYGEN,		IDH_PGPCLPREF_USEFASTKEYGEN,
	IDC_DECRYPTCACHEENABLE,	IDH_PGPCLPREF_PASSCACHEENABLE,
	IDC_DECRYPTCACHETIME,	IDH_PGPCLPREF_PASSCACHETIME,
	IDC_SIGNCACHEENABLE,	IDH_PGPCLPREF_SIGNCACHEENABLE,
	IDC_SIGNCACHETIME,		IDH_PGPCLPREF_SIGNCACHETIME,
	IDC_COMMENTBLOCK,		IDH_PGPCLPREF_COMMENTBLOCK,
	IDC_WIPECONFIRM,		IDH_PGPCLPREF_WIPECONFIRM,
	IDC_NUMPASSES,			IDH_PGPCLPREF_NUMWIPEPASSES,
	IDC_NUMPASSESSPIN,		IDH_PGPCLPREF_NUMWIPEPASSESSPIN,
	IDC_NUMPASSESTEXT,		IDH_PGPCLPREF_NUMWIPEPASSES,
	0,0
};

static DWORD aFileIds[] = {			// Help IDs
    IDC_PRIVATEKEYRING,		IDH_PGPCLPREF_PRIVATEKEYRING, 
	IDC_SETPRIVATEKEYRING,	IDH_PGPCLPREF_BROWSEPRIVATEKEYRING,
    IDC_PUBLICKEYRING,		IDH_PGPCLPREF_PUBLICKEYRING, 
	IDC_SETPUBLICKEYRING,	IDH_PGPCLPREF_BROWSEPUBLICKEYRING,
	IDC_RNGSEEDFILE,		IDH_PGPCLPREF_RANDOMSEEDFILE,
	IDC_SETRNGSEEDFILE,		IDH_PGPCLPREF_BROWSERANDOMSEEDFILE,
	0,0
};

static DWORD aEmailIds[] = {			// Help IDs
	IDC_USEPGPMIME,			IDH_PGPCLPREF_USEPGPMIME,
	IDC_ENCRYPTBYDEFAULT,	IDH_PGPCLPREF_ENCRYPTBYDEFAULT,
	IDC_SIGNBYDEFAULT,		IDH_PGPCLPREF_SIGNBYDEFAULT,
	IDC_AUTODECRYPT,		IDH_PGPCLPREF_AUTODECRYPT,
	IDC_USESECUREVIEWER,	IDH_PGPCLPREF_ALWAYSUSESECUREVIEWER,
	IDC_WRAPENABLE,			IDH_PGPCLPREF_WORDWRAPENABLE,
	IDC_WRAPNUMBER,			IDH_PGPCLPREF_WORDWRAPCOLUMN,
	0,0
};

static DWORD aHotkeyIds[] = {			// Help IDs
	IDC_ENABLEPURGECACHE,	IDH_PGPCLPREF_ENABLEPURGEHOTKEY,
	IDC_PURGEHOTKEY,		IDH_PGPCLPREF_PURGEHOTKEY,
	IDC_ENABLEENCRYPT,		IDH_PGPCLPREF_ENABLEENCRYPTHOTKEY,
	IDC_ENCRYPTHOTKEY,		IDH_PGPCLPREF_ENCRYPTHOTKEY,
	IDC_ENABLESIGN,			IDH_PGPCLPREF_ENABLESIGNHOTKEY,
	IDC_SIGNHOTKEY,			IDH_PGPCLPREF_SIGNHOTKEY,
	IDC_ENABLEENCRYPTSIGN,	IDH_PGPCLPREF_ENABLEESHOTKEY,
	IDC_ENCRYPTSIGNHOTKEY,	IDH_PGPCLPREF_ENCRYPTSIGNHOTKEY,
	IDC_ENABLEDECRYPT,		IDH_PGPCLPREF_ENABLEDECRYPTHOTKEY,
	IDC_DECRYPTHOTKEY,		IDH_PGPCLPREF_DECRYPTHOTKEY,
	0,0
};

static DWORD aServerIds[] = {			// Help IDs
	IDC_NEWKEYSERVER,		IDH_PGPCLPREF_CREATESERVER,
	IDC_REMOVEKEYSERVER,	IDH_PGPCLPREF_REMOVESERVER,
	IDC_EDITKEYSERVER,		IDH_PGPCLPREF_EDITSERVER,
	IDC_SETASROOT,			IDH_PGPCLPREF_SETROOTSERVER,
	IDC_MOVEUP,				IDH_PGPCLPREF_MOVESERVERUP,
	IDC_MOVEDOWN,			IDH_PGPCLPREF_MOVESERVERDOWN,
	IDC_SYNCONUNKNOWN,		IDH_PGPCLPREF_SYNCONUNKNOWNKEYS,
	IDC_SYNCONADDNAME,		IDH_PGPCLPREF_SYNCONADDNAME,
	IDC_SYNCONKEYSIGN,		IDH_PGPCLPREF_SYNCONSIGN,
	IDC_SYNCONREVOKE,		IDH_PGPCLPREF_SYNCONREVOKE,
	IDC_SYNCONVERIFY,		IDH_PGPCLPREF_SYNCONVERIFY,
	IDC_KSTREELIST,			IDH_PGPCLPREF_SERVERLIST,
	0,0
};

static DWORD aCAIds[] = {			// Help IDs
	IDC_URL,				IDH_PGPCLPREF_CAURL,
	IDC_REVURL,				IDH_PGPCLPREF_CAREVOCATIONURL,
	IDC_TYPE,				IDH_PGPCLPREF_CATYPE,
	IDC_ROOTCERT,			IDH_PGPCLPREF_CAROOTCERT,
	IDC_CLEARCERT,			IDH_PGPCLPREF_CACLEARROOTCERT,
	IDC_SETCERT,			IDH_PGPCLPREF_CASELECTROOTCERT,
    0,0 
}; 

static DWORD aAdvancedIds[] = {			// Help IDs
	IDC_PREFERREDALG,		IDH_PGPCLPREF_PREFERREDALG,
	IDC_ENABLECAST,			IDH_PGPCLPREF_ENABLECAST,
	IDC_ENABLE3DES,			IDH_PGPCLPREF_ENABLE3DES,
	IDC_ENABLEIDEA,			IDH_PGPCLPREF_ENABLEIDEA,
	IDC_ENABLETWOFISH,		IDH_PGPCLPREF_ENABLETWOFISH,
	IDC_DISPMARGVALIDITY,	IDH_PGPCLPREF_DISPMARGVALIDITY,
	IDC_MARGVALIDISINVALID,	IDH_PGPCLPREF_WARNONMARGINAL,
	IDC_WARNONADK,			IDH_PGPCLPREF_WARNONADK,
	IDC_EXPORTCOMPATIBLE,	IDH_PGPCLPREF_EXPORTCOMPATIBLE,
	IDC_EXPORTCOMPLETE,		IDH_PGPCLPREF_EXPORTCOMPLETE,
    0,0 
}; 


//	______________________________________________
//
//  create the specified path if it doesn't already exist

static BOOL
sCreatePath (
		LPSTR	pszPath)
{
	DWORD	dw;
	LPSTR	p;

	dw = GetFileAttributes (pszPath);
	if ((dw != 0xFFFFFFFF) &&
		(dw & FILE_ATTRIBUTE_DIRECTORY))
		return TRUE;

	if (dw != 0xFFFFFFFF)
		return FALSE;

	p = strchr (pszPath, '\\');
	while (p)
	{
		*p = '\0';
		CreateDirectory (pszPath, NULL);
		*p = '\\';
		p++;
		p = strchr (p, '\\');
	}
	return TRUE;
}


//	____________________________________
//
//	get prefs files location based on which version of windows

PGPError PGPclExport
PGPclGetClientPrefsPath (
		LPSTR	pszPath, 
		UINT	uLen) 
{
	OSVERSIONINFO	osid;
	CHAR			sz[64];

	osid.dwOSVersionInfoSize = sizeof (osid);
	GetVersionEx (&osid);

	switch (osid.dwPlatformId) {
	// Windows NT
	case VER_PLATFORM_WIN32_NT :
	{
		// add the folder name
		LoadString (g_hInst, IDS_CLIENTPREFSFOLDER, sz, sizeof(sz));

		// decrease available space in pszPath by length of folder name
		uLen -= (lstrlen (sz) +1);

		// get the user profile path (e.g. "C:\WINNT\Profiles\username")
		if (GetEnvironmentVariable ("USERPROFILE", pszPath, uLen) == 0)
			return kPGPError_FileNotFound;

		if (pszPath[lstrlen(pszPath)-1] != '\\') 
			lstrcat (pszPath, "\\");

		// append the folder name
		lstrcat (pszPath, sz);
		break;
	}

	// otherwise use the PGP install path or, 
	// if all else fails, the Windows path 
	default :
	{
		PGPError	err;

		err = PGPclGetPGPPath (pszPath, uLen);
		if (IsPGPError (err)) 
			GetWindowsDirectory (pszPath, (uLen-1));
		break;
	}
	}

	if (pszPath[lstrlen(pszPath)-1] != '\\') 
		lstrcat (pszPath, "\\");

	return kPGPError_NoErr;
}


//	____________________________________
//
//	open prefs files based on which version of windows

PGPError PGPclExport
PGPclOpenClientPrefs (
		PGPMemoryMgrRef	memMgr, 
		PGPPrefRef*		pPrefRef) 
{
	CHAR			szPath[MAX_PATH];
	CHAR			sz[64];
	PFLFileSpecRef	FileRef;
	PGPPrefRef		ClientPrefRef;
	PGPError		err;

	err = PGPclGetClientPrefsPath (szPath, sizeof(szPath));
	if (IsPGPError (err))
		return err;

	// create the path if it doesn't exist
	sCreatePath (szPath);

	// create the name of the prefs file
	LoadString (g_hInst, IDS_CLIENTPREFSFILE, sz, sizeof(sz));
	lstrcat (szPath, sz);

	PFLNewFileSpecFromFullPath (memMgr, szPath, &FileRef);

	err = PGPOpenPrefFile (
			FileRef, clientDefaults, clientDefaultsSize, &ClientPrefRef);

	if ((err == kPGPError_FileOpFailed) ||
		(err == kPGPError_FileNotFound)) 
	{
		err = PFLFileSpecCreate (FileRef);
		pgpAssert (IsntPGPError (err));
		err = PGPOpenPrefFile(FileRef, clientDefaults, clientDefaultsSize,
				&ClientPrefRef);
		pgpAssert (IsntPGPError (err));
	}
	PFLFreeFileSpec (FileRef);

	if (IsPGPError (err)) {
		if (PGPRefIsValid (ClientPrefRef)) 
			PGPClosePrefFile (ClientPrefRef);
	}
	else {	
		*pPrefRef = ClientPrefRef;
	}

	return err;
}

//	______________________________________________
//
//  The following functions are wrappers for
//  the pfl prefs routines. These are needed so
//  groupwise plugin can access prefs (Delphi)
//  using code in the PGPcl DLL.

PGPError PGPclGetPrefBoolean(PGPPrefRef prefRef,
						   PGPPrefIndex prefIndex,
						   PGPBoolean *data)
{
	return PGPGetPrefBoolean(prefRef,
						   prefIndex,
						   data);
}

PGPError PGPclSetPrefBoolean(PGPPrefRef prefRef,
						   PGPPrefIndex prefIndex,
						   PGPBoolean data)
{
	return PGPSetPrefBoolean(prefRef,
						   prefIndex,
						   data);
}

PGPError PGPclGetPrefNumber(PGPPrefRef prefRef,
						  PGPPrefIndex prefIndex,
						  PGPUInt32 *data)
{
	return PGPGetPrefNumber(prefRef,
						  prefIndex,
						  data);
}

PGPError PGPclSetPrefNumber(PGPPrefRef prefRef,
						  PGPPrefIndex prefIndex,
						  PGPUInt32 data)
{
	return PGPSetPrefNumber(prefRef,
						  prefIndex,
						  data);
}

PGPError PGPclGetPrefStringAlloc(PGPPrefRef prefRef,
							   PGPPrefIndex prefIndex,
							   char **string)
{
	return PGPGetPrefStringAlloc(prefRef,
							   prefIndex,
							   string);
}

PGPError PGPclGetPrefStringBuffer(PGPPrefRef prefRef,
								PGPPrefIndex prefIndex,
								PGPSize maxSize,
								char *string)
{
	return PGPGetPrefStringBuffer(prefRef,
								prefIndex,
								maxSize,
								string);
}

PGPError PGPclSetPrefString(PGPPrefRef prefRef,
						  PGPPrefIndex prefIndex,
						  const char *string)
{
	return PGPSetPrefString(prefRef,
						  prefIndex,
						  string);
}

PGPError PGPclGetPrefData(PGPPrefRef prefRef, 
						PGPPrefIndex prefIndex, 
						PGPSize *dataLength, 
						void **inBuffer)
{
	return PGPGetPrefData(prefRef, 
						prefIndex, 
						dataLength, 
						inBuffer);
}

PGPError PGPclGetPrefFileSpec(PGPPrefRef prefRef,
								PFLFileSpecRef *prefFileSpec)
{
	return PGPGetPrefFileSpec(prefRef,
								prefFileSpec);
}

PGPError PGPclSetPrefData(PGPPrefRef prefRef, 
						PGPPrefIndex prefIndex, 
						PGPSize dataLength, 
						const void *outBuffer)
{
	return PGPSetPrefData(prefRef, 
						prefIndex, 
						dataLength, 
						outBuffer);
}

PGPError PGPclRemovePref(PGPPrefRef prefRef, 
					   PGPPrefIndex prefIndex)
{
	return PGPRemovePref(prefRef, 
					   prefIndex);
}

PGPError PGPclDisposePrefData(PGPPrefRef prefRef, 
							void *dataBuffer)
{
	return PGPDisposePrefData(prefRef, 
							dataBuffer);
}


//	______________________________________________
//
//  Get standard admin preferences file reference

static PGPError
sOpenAdminPrefs95 (
		PGPMemoryMgrRef	memMgr,
		PGPPrefRef*		pPrefRef, 
		BOOL			bLoadDefaults) 
{
	PGPError		err					= kPGPError_NoErr;
	PGPPrefRef		AdminPrefRef		= kInvalidPGPPrefRef;
	CHAR			szPath[MAX_PATH];
	CHAR			sz[64];
	PFLFileSpecRef	FileRef;
	DWORD			dw;

	err = PGPclGetPGPPath (szPath, sizeof(szPath));
	if (IsPGPError (err)) {
		GetWindowsDirectory (szPath, sizeof(szPath));
		if (szPath[lstrlen(szPath)-1] != '\\') lstrcat (szPath, "\\");
	}

	LoadString (g_hInst, IDS_ADMINPREFSFILE, sz, sizeof(sz));
	lstrcat (szPath, sz);

	// check for file existence
	dw = GetFileAttributes (szPath);
	if ((dw == 0xFFFFFFFF) || (dw & FILE_ATTRIBUTE_DIRECTORY)) {
		if (!bLoadDefaults)
			err = kPGPError_FileNotFound;
	}

	if (IsntPGPError (err)) {
		PFLNewFileSpecFromFullPath (memMgr, szPath, &FileRef);

		err = PGPOpenPrefFile (FileRef, adminDefaults, adminDefaultsSize,
				&AdminPrefRef);

		PFLFreeFileSpec (FileRef);
	}

	if (IsPGPError (err)) {
		PGPclMessageBox (NULL, IDS_CAPTIONERROR, IDS_NOADMINFILE,
									MB_OK|MB_ICONSTOP);
		if (PGPRefIsValid (AdminPrefRef)) 
			PGPClosePrefFile (AdminPrefRef);
	}
	else {	
		*pPrefRef = AdminPrefRef;
	}

	return err;
}


//	______________________________________________
//
//  Get standard admin preferences file reference

static PGPError
sOpenAdminPrefsNT (
		PGPMemoryMgrRef	memMgr,
		PGPPrefRef*		pPrefRef, 
		BOOL			bLoadDefaults) 
{
	PGPError		err					= kPGPError_NoErr;
	PGPPrefRef		AdminPrefRef		= kInvalidPGPPrefRef;
	CHAR			szPath[MAX_PATH];
	CHAR			sz[64];
	PFLFileSpecRef	FileRef;
	DWORD			dw;
	LPSTR			p;

	// get the user profile path (e.g. "C:\WINNT\Profiles\username")
	if (GetEnvironmentVariable ("USERPROFILE", szPath, sizeof(szPath)) == 0)
		return kPGPError_FileNotFound;

	// remove the username part
	p = strrchr(szPath, '\\');
	p++;
	*p = '\0';

	// add the location of the PGP files
	LoadString (g_hInst, IDS_ADMINPREFSFOLDER, sz, sizeof(sz));
	lstrcat (szPath, sz);

	// create the path if it doesn't exist
	sCreatePath (szPath);

	LoadString (g_hInst, IDS_ADMINPREFSFILE, sz, sizeof(sz));
	lstrcat (szPath, sz);

	// check for file existence
	dw = GetFileAttributes (szPath);
	if ((dw == 0xFFFFFFFF) || (dw & FILE_ATTRIBUTE_DIRECTORY)) {
		if (!bLoadDefaults)
			err = kPGPError_FileNotFound;
	}

	if (IsntPGPError (err)) {
		PFLNewFileSpecFromFullPath (memMgr, szPath, &FileRef);

		err = PGPOpenPrefFile (FileRef, adminDefaults, adminDefaultsSize,
				&AdminPrefRef);

		// if an admin install and the file is corrupt, try deleting it
		// and reopening.
		if (bLoadDefaults && (err == kPGPError_CorruptData)) {
			DeleteFile (szPath);
			err = PGPOpenPrefFile (FileRef, adminDefaults, adminDefaultsSize,
				&AdminPrefRef);
		}

		PFLFreeFileSpec (FileRef);
	}

	if (IsPGPError (err)) {
		PGPclMessageBox (NULL, IDS_CAPTIONERROR, IDS_NOADMINFILE,
									MB_OK|MB_ICONSTOP);
		if (PGPRefIsValid (AdminPrefRef)) 
			PGPClosePrefFile (AdminPrefRef);
	}
	else {	
		*pPrefRef = AdminPrefRef;
	}

	return err;
}


//	____________________________________
//
//	open prefs files based on which version of windows

PGPError PGPclExport
PGPclOpenAdminPrefs (
		PGPMemoryMgrRef	memMgr, 
		PGPPrefRef*		pPrefRef,
		BOOL			bLoadDefaults) 
{
	OSVERSIONINFO osid;

	osid.dwOSVersionInfoSize = sizeof (osid);
	GetVersionEx (&osid);

	switch (osid.dwPlatformId) {
	// Windows NT
	case VER_PLATFORM_WIN32_NT :
		return sOpenAdminPrefsNT (memMgr, pPrefRef, bLoadDefaults);

	// otherwise
	default :
		return sOpenAdminPrefs95 (memMgr, pPrefRef, bLoadDefaults);
	}
}


//	_______________________________________
//
//  Save and close down client preferences file

PGPError PGPclExport
PGPclCloseClientPrefs (
		PGPPrefRef	PrefRef, 
		BOOL		bSave) 
{
	PGPError		err;

	if (bSave) 
	{
		err = PGPSavePrefFile (PrefRef);
		pgpAssert (IsntPGPError(err));
	}

	if (IsPGPError (err))
		PGPClosePrefFile (PrefRef);
	else
	{
		err = PGPClosePrefFile (PrefRef);
		pgpAssert (IsntPGPError(err));
	}

	return err;
}

//	_______________________________________
//
//  Save and close down admin preferences file

PGPError PGPclExport
PGPclCloseAdminPrefs (
		PGPPrefRef	PrefRef, 
		BOOL		bSave) 
{
	PGPError		err;

	if (bSave) 
	{
		err = PGPSavePrefFile (PrefRef);
		pgpAssert (IsntPGPError(err));
	}

	if (IsPGPError (err))
		PGPClosePrefFile (PrefRef);
	else
	{
		err = PGPClosePrefFile (PrefRef);
		pgpAssert (IsntPGPError(err));
	}

	return err;
}

//	_______________________________________
//
//  Query registry to determine if client or admin installation

BOOL PGPclExport
PGPclIsAdminInstall (VOID)
{
	HKEY		hKey;
	LONG		lResult;
	DWORD		dwValueType, dwSize;
	CHAR		szKey[128];

	LoadString (g_hInst, IDS_REGISTRYKEY, szKey, sizeof(szKey));
	lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey);

	if (lResult == ERROR_SUCCESS) {
		dwSize = sizeof(szKey);
		lResult = RegQueryValueEx (
			hKey, "InstallDate", 0, &dwValueType, (LPBYTE)szKey, &dwSize);
		RegCloseKey (hKey);
	}

	return (lResult == ERROR_SUCCESS);
}

BOOL PGPclExport
PGPclIsClientInstall (VOID)
{
	return !PGPclIsAdminInstall();
}

//	____________________________
//
//  Check for illegal file names

static BOOL 
sIsFileNameOK (LPSTR sz) 
{
	UINT	uLen;
	DWORD	dwAttrib;

	uLen = lstrlen (sz);

	if (uLen < 4) return FALSE;
	if ((sz[1] != ':') && (sz[1] != '\\')) return FALSE;
	if ((sz[1] == ':') && (sz[2] != '\\')) return FALSE;
	if (strcspn (sz, "/*?""<>|") != uLen) return FALSE;
	if (sz[uLen-1] == ':') return FALSE;
	if (sz[uLen-1] == '\\') return FALSE;

	dwAttrib = GetFileAttributes (sz);
	if (dwAttrib == 0xFFFFFFFF) return TRUE;	// file doesn't exist
	if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) return FALSE;
	if (dwAttrib & FILE_ATTRIBUTE_SYSTEM) return FALSE;

	return TRUE;
}

//	_____________________________________________________
//
//  verify that file or backup of file exists.  
//	if not, replace with default file name in install dir
//	if no install dir found, use Windows dir

static VOID
sVerifyFileExists (LPSTR lpszFile, PGPBoolean KeyRing)
{
	CHAR	sz[MAX_PATH];
	DWORD	dwAttrib;
	CHAR*	p;

	dwAttrib = GetFileAttributes (lpszFile);
	if ((dwAttrib == 0xFFFFFFFF) || 
			(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
		lstrcpy (sz, lpszFile);
		p = strrchr (sz, '\\');
		if (p) p++;
		else p = sz;

		if (IsPGPError (PGPclGetPGPPath (lpszFile, MAX_PATH))) 
			GetWindowsDirectory (lpszFile, MAX_PATH);
		else
		{
			if(KeyRing)
			{
				// We don't really want install directory.. we want 
				// PGP Keyrings subdirectory. Otherwise we'll just 
				// take windows like everything else -wjb

				CHAR	sz2[128];

				if (lpszFile[lstrlen (lpszFile) -1] != '\\') 
					lstrcat (lpszFile, "\\");

				LoadString (g_hInst, IDS_KEYRINGFOLDER, sz2, sizeof(sz2));
				lstrcat (lpszFile, sz2);

				_mkdir (lpszFile); // just in case user deleted it
			}
		}

		if (lpszFile[lstrlen (lpszFile) -1] != '\\') 
			lstrcat (lpszFile, "\\");
		lstrcat (lpszFile, p);
	}
}

//	_____________________________________________________
//
//  Message processing function for property sheet dialog

static LRESULT WINAPI 
sFilePropDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	OPENFILENAME	OpenFileName;
	CHAR			szFile[MAX_PATH];
	CHAR			szRing[MAX_PATH];
	CHAR			szTemp[MAX_PATH];
	CHAR			szFilter[128];
	CHAR			szTitle[128];
	CHAR			szDefExt[8];
	CHAR*			p;
	HWND			hParent;
	RECT			rc;
	PGPError		err;
	PGPFileSpecRef	fileRef;
	LPSTR			lpsz;
	HANDLE			hfile;
	
	switch (uMsg) {

	case WM_INITDIALOG:
		if (bNeedsCentering) {
			hParent = GetParent (hDlg);
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			bNeedsCentering = FALSE;
		}

		err = PGPsdkPrefGetFileSpec (ContextRef, kPGPsdkPref_PublicKeyring,
									&fileRef);
		if (IsntPGPError (err) && fileRef) {
			PGPGetFullPathFromFileSpec (fileRef, &lpsz);
			lstrcpy (szTemp, lpsz);
			PGPFreeData (lpsz);
			PGPFreeFileSpec (fileRef);

			sVerifyFileExists (szTemp, TRUE);
			SetDlgItemText (hDlg, IDC_PUBLICKEYRING, szTemp);
		}

		err = PGPsdkPrefGetFileSpec (ContextRef, kPGPsdkPref_PrivateKeyring,
									&fileRef);
		if (IsntPGPError (err) && fileRef) {
			PGPGetFullPathFromFileSpec (fileRef, &lpsz);
			lstrcpy (szTemp, lpsz);
			PGPFreeData (lpsz);
			PGPFreeFileSpec (fileRef);

			sVerifyFileExists (szTemp, TRUE);
			SetDlgItemText (hDlg, IDC_PRIVATEKEYRING, szTemp);
		}

		err = PGPsdkPrefGetFileSpec (ContextRef, kPGPsdkPref_RandomSeedFile,
									&fileRef);
		if (IsntPGPError (err) && fileRef) {
			PGPGetFullPathFromFileSpec (fileRef, &lpsz);
			lstrcpy (szTemp, lpsz);
			PGPFreeData (lpsz);
			PGPFreeFileSpec (fileRef);

			sVerifyFileExists (szTemp, FALSE);
			SetDlgItemText (hDlg, IDC_RNGSEEDFILE, szTemp);
		}

		szInitDir[0] = '\0';

		return TRUE;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aFileIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aFileIds); 
        break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDC_SETPRIVATEKEYRING :
			GetDlgItemText (hDlg, IDC_PRIVATEKEYRING, szFile, sizeof(szFile));
			if (GetFileAttributes (szFile) & FILE_ATTRIBUTE_DIRECTORY) {
				lstrcpy (szInitDir, szFile);
				szFile[0] = '\0';
			}
			else {
				if (szInitDir[0])
					szFile[0] = '\0';
			}
			LoadString (g_hInst, IDS_PRIVKEYRINGFILTER, szFilter, 
								sizeof(szFilter));
			while (p = strrchr (szFilter, '@')) *p = '\0';
			LoadString (g_hInst, IDS_PRIVKEYCAPTION, szTitle, 
								sizeof(szTitle));
			OpenFileName.lStructSize       = sizeof (OPENFILENAME);
			OpenFileName.hwndOwner         = hDlg;
			OpenFileName.hInstance         = (HANDLE)g_hInst;
			OpenFileName.lpstrFilter       = szFilter;
			OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
			OpenFileName.nMaxCustFilter    = 0L;
			OpenFileName.nFilterIndex      = 1L;
			OpenFileName.lpstrFile         = szFile;
			OpenFileName.nMaxFile          = sizeof (szFile);
			OpenFileName.lpstrFileTitle    = NULL;
			OpenFileName.nMaxFileTitle     = 0;
			OpenFileName.lpstrInitialDir   = szInitDir;
			OpenFileName.lpstrTitle        = szTitle;
			OpenFileName.Flags			   = OFN_HIDEREADONLY;
			OpenFileName.nFileOffset       = 0;
			OpenFileName.nFileExtension    = 0;
			LoadString (g_hInst, IDS_DEFPRIVRINGEXT, szDefExt, 
								sizeof(szDefExt));
			OpenFileName.lpstrDefExt       = szDefExt;
			OpenFileName.lCustData         = 0;
			if (GetOpenFileName (&OpenFileName)) {
				SetDlgItemText (hDlg, IDC_PRIVATEKEYRING, szFile);
				if (OpenFileName.nFileOffset > 0) 
					szFile[OpenFileName.nFileOffset-1] = '\0';
				lstrcpy (szInitDir, szFile);
			}
			break;

		case IDC_SETPUBLICKEYRING :
			GetDlgItemText (hDlg, IDC_PUBLICKEYRING, szFile, sizeof(szFile));
			if (GetFileAttributes (szFile) & FILE_ATTRIBUTE_DIRECTORY) {
				lstrcpy (szInitDir, szFile);
				szFile[0] = '\0';
			}
			else {
				if (szInitDir[0])
					szFile[0] = '\0';
			}
			LoadString (g_hInst, IDS_PUBKEYRINGFILTER, szFilter, 
								sizeof(szFilter));
			while (p = strrchr (szFilter, '@')) *p = '\0';
			LoadString (g_hInst, IDS_PUBKEYCAPTION, szTitle, sizeof(szTitle));
			OpenFileName.lStructSize       = sizeof (OPENFILENAME);
			OpenFileName.hwndOwner         = hDlg;
			OpenFileName.hInstance         = (HANDLE)g_hInst;
			OpenFileName.lpstrFilter       = szFilter;
			OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
			OpenFileName.nMaxCustFilter    = 0L;
			OpenFileName.nFilterIndex      = 1L;
			OpenFileName.lpstrFile         = szFile;
			OpenFileName.nMaxFile          = sizeof (szFile);
			OpenFileName.lpstrFileTitle    = NULL;
			OpenFileName.nMaxFileTitle     = 0;
			OpenFileName.lpstrInitialDir   = szInitDir;
			OpenFileName.lpstrTitle        = szTitle;
			OpenFileName.Flags			   = OFN_HIDEREADONLY;
			OpenFileName.nFileOffset       = 0;
			OpenFileName.nFileExtension    = 0;
			LoadString (g_hInst, IDS_DEFPUBRINGEXT, szDefExt, 
								sizeof(szDefExt));
			OpenFileName.lpstrDefExt       = szDefExt;
			OpenFileName.lCustData         = 0;
			if (GetOpenFileName (&OpenFileName)) {
				SetDlgItemText (hDlg, IDC_PUBLICKEYRING, szFile);
				if (OpenFileName.nFileOffset > 0) 
					szFile[OpenFileName.nFileOffset-1] = '\0';
				lstrcpy (szInitDir, szFile);
			}
			break;

		case IDC_SETRNGSEEDFILE :
			GetDlgItemText (hDlg, IDC_RNGSEEDFILE, szFile, sizeof(szFile));
			if (GetFileAttributes (szFile) & FILE_ATTRIBUTE_DIRECTORY) {
				lstrcpy (szInitDir, szFile);
				szFile[0] = '\0';
			}
			else {
				if (szInitDir[0])
					szFile[0] = '\0';
			}
			LoadString (g_hInst, IDS_SEEDFILEFILTER, szFilter, 
								sizeof(szFilter));
			while (p = strrchr (szFilter, '@')) *p = '\0';
			LoadString (g_hInst, IDS_SEEDFILECAPTION, szTitle, 
								sizeof(szTitle));
			OpenFileName.lStructSize       = sizeof (OPENFILENAME);
			OpenFileName.hwndOwner         = hDlg;
			OpenFileName.hInstance         = (HANDLE)g_hInst;
			OpenFileName.lpstrFilter       = szFilter;
			OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
			OpenFileName.nMaxCustFilter    = 0L;
			OpenFileName.nFilterIndex      = 1L;
			OpenFileName.lpstrFile         = szFile;
			OpenFileName.nMaxFile          = sizeof (szFile);
			OpenFileName.lpstrFileTitle    = NULL;
			OpenFileName.nMaxFileTitle     = 0;
			OpenFileName.lpstrInitialDir   = szInitDir;
			OpenFileName.lpstrTitle        = szTitle;
			OpenFileName.Flags			   = OFN_HIDEREADONLY;
			OpenFileName.nFileOffset       = 0;
			OpenFileName.nFileExtension    = 0;
			LoadString (g_hInst, IDS_DEFSEEDFILEEXT, szDefExt, 
								sizeof(szDefExt));
			OpenFileName.lpstrDefExt       = szDefExt;
			OpenFileName.lCustData         = 0;
			if (GetOpenFileName (&OpenFileName)) {
				SetDlgItemText (hDlg, IDC_RNGSEEDFILE, szFile);
				if (OpenFileName.nFileOffset > 0) 
					szFile[OpenFileName.nFileOffset-1] = '\0';
				lstrcpy (szInitDir, szFile);
			}
			break;
		}
		break;

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {

		case PSN_SETACTIVE :
			break;

		case PSN_HELP :
			WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
				IDH_PGPCLPREF_FILEDIALOG); 
			break;

		case PSN_APPLY :
			// get current private keyring file
			err = PGPsdkPrefGetFileSpec (ContextRef, 
							kPGPsdkPref_PrivateKeyring, &fileRef);
			if (IsntPGPError (err) && fileRef) {
				PGPGetFullPathFromFileSpec (fileRef, &lpsz);
				lstrcpy (szRing, lpsz);
				PGPFreeData (lpsz);
				// compare with selected file and set pref if different
				GetDlgItemText (hDlg, IDC_PRIVATEKEYRING, szFile, 
									sizeof(szFile));
				// create file if it doesn't exist -wjb
				hfile = CreateFile (szFile, GENERIC_READ|GENERIC_WRITE, 
								0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 
								NULL);
				CloseHandle (hfile);

				if (lstrcmpi (szFile, szRing) != 0) 
				{
					PGPFreeFileSpec (fileRef);
					err = PGPNewFileSpecFromFullPath (ContextRef, szFile, 
														&fileRef);
					if (PGPclErrorBox (hDlg, err)) 
						return TRUE;

					// create file if it doesn't exist 
					hfile = CreateFile (szFile, GENERIC_READ|GENERIC_WRITE, 
								0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 
								NULL);
					CloseHandle (hfile);

					err = PGPsdkPrefSetFileSpec (ContextRef, 
							kPGPsdkPref_PrivateKeyring, fileRef);
					if (!PGPclErrorBox (hDlg, err)) 
						bReloadKeyring = TRUE;
				}
				PGPFreeFileSpec (fileRef);
			}

			// get current public keyring file
			err = PGPsdkPrefGetFileSpec (ContextRef, 
							kPGPsdkPref_PublicKeyring, &fileRef);
			if (IsntPGPError (err) && fileRef) {
				PGPGetFullPathFromFileSpec (fileRef, &lpsz);
				lstrcpy (szRing, lpsz);
				PGPFreeData (lpsz);
				// compare with selected file and set pref if different
				GetDlgItemText (hDlg, IDC_PUBLICKEYRING, szFile, 
									sizeof(szFile));
				// create file if it doesn't exist -wjb
				hfile = CreateFile (szFile, GENERIC_READ|GENERIC_WRITE, 
								0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 
								NULL);
				CloseHandle (hfile);

				if (lstrcmpi (szFile, szRing) != 0) {
					PGPFreeFileSpec (fileRef);
					err = PGPNewFileSpecFromFullPath (ContextRef, szFile, 
														&fileRef);
					if (PGPclErrorBox (hDlg, err)) 
						return TRUE;

					// create file if it doesn't exist
					hfile = CreateFile (szFile, GENERIC_READ|GENERIC_WRITE, 
								0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 
								NULL);
					CloseHandle (hfile);

					err = PGPsdkPrefSetFileSpec (ContextRef, 
							kPGPsdkPref_PublicKeyring, fileRef);
					if (!PGPclErrorBox (hDlg, err)) 
						bReloadKeyring = TRUE;
				}
				PGPFreeFileSpec (fileRef);
			}

			// get RNG seed file
			err = PGPsdkPrefGetFileSpec (ContextRef, 
							kPGPsdkPref_RandomSeedFile, &fileRef);
			if (IsntPGPError (err) && fileRef) {
				PGPGetFullPathFromFileSpec (fileRef, &lpsz);
				lstrcpy (szRing, lpsz);
				PGPFreeData (lpsz);
				// compare with selected file and set pref if different
				GetDlgItemText (hDlg, IDC_RNGSEEDFILE, szFile, 
									sizeof(szFile));
				// create file if it doesn't exist -wjb
				hfile = CreateFile (szFile, GENERIC_READ|GENERIC_WRITE, 
								0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 
								NULL);
				CloseHandle (hfile);

				if (lstrcmpi (szFile, szRing) != 0) {
					PGPFreeFileSpec (fileRef);
					err = PGPNewFileSpecFromFullPath (ContextRef, szFile, 
														&fileRef);
					if (PGPclErrorBox (hDlg, err)) 
						return TRUE;

					// create file if it doesn't exist
					hfile = CreateFile (szFile, GENERIC_READ|GENERIC_WRITE, 
								0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 
								NULL);
					CloseHandle (hfile);

					PGPclErrorBox (hDlg, PGPsdkPrefSetFileSpec (ContextRef, 
							kPGPsdkPref_RandomSeedFile, fileRef));
				}
				PGPFreeFileSpec (fileRef);
			}

			if (bReloadKeyring) {
				// reset warn flag
				PGPSetPrefBoolean (PrefRefClient, 
					kPGPPrefWarnOnReadOnlyKeyRings, (PGPBoolean)TRUE);
			}
			returnErr = kPGPError_NoErr;
            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);

			return TRUE;

		case PSN_KILLACTIVE :
			GetDlgItemText (hDlg, IDC_PRIVATEKEYRING, szFile, sizeof(szFile));
			if (!sIsFileNameOK (szFile)) 
			{
				PGPclMessageBox (GetParent (hDlg), 
					IDS_CAPTION, IDS_ILLEGALFILENAME,
					MB_OK | MB_ICONHAND);
				SendDlgItemMessage (hDlg, IDC_PRIVATEKEYRING, EM_SETSEL, 0, -1);
				SetFocus (GetDlgItem (hDlg, IDC_PRIVATEKEYRING));
				SetWindowLong (hDlg, DWL_MSGRESULT, TRUE);
				return TRUE;
			}

			GetDlgItemText (hDlg, IDC_PUBLICKEYRING, szFile, sizeof(szFile));
			if (!sIsFileNameOK (szFile)) 
			{
				PGPclMessageBox (GetParent (hDlg), 
					IDS_CAPTION, IDS_ILLEGALFILENAME,
					MB_OK | MB_ICONHAND);
				SendDlgItemMessage (hDlg, IDC_PUBLICKEYRING, EM_SETSEL, 0, -1);
				SetFocus (GetDlgItem (hDlg, IDC_PUBLICKEYRING));
				SetWindowLong (hDlg, DWL_MSGRESULT, TRUE);
				return TRUE;
			}

			GetDlgItemText (hDlg, IDC_RNGSEEDFILE, szFile, sizeof(szFile));
			if (!sIsFileNameOK (szFile)) 
			{
				PGPclMessageBox (GetParent (hDlg), 
					IDS_CAPTION, IDS_ILLEGALFILENAME,
					MB_OK | MB_ICONHAND);
				SendDlgItemMessage (hDlg, IDC_RNGSEEDFILE, EM_SETSEL, 0, -1);
				SetFocus (GetDlgItem (hDlg, IDC_RNGSEEDFILE));
				SetWindowLong (hDlg, DWL_MSGRESULT, TRUE);
				return TRUE;
			}

			SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			return TRUE;

		case PSN_RESET :
			returnErr = kPGPError_UserAbort;
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;
		}

	}
	return FALSE;

}

//	_____________________________________________________
//
//  Message processing function for property sheet dialog

static LRESULT WINAPI 
sEncryptPropDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	RECT					rc;
	UINT					u, uSec, uEncrypt, uSign;
	PGPBoolean				b;
	CHAR					sz[64];
	PCACHEDURATIONSTRUCT	pcds;

	switch (uMsg) {

	case WM_INITDIALOG:
		
		pcds = clAlloc (sizeof(CACHEDURATIONSTRUCT));
		SetWindowLong (hDlg, GWL_USERDATA, (LONG)pcds);
		if (bNeedsCentering) 
		{
			HWND	hParent;
			RECT	rc;

			hParent = GetParent (hDlg);
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			bNeedsCentering = FALSE;
		}

		// initialize decrypt to self
		b = DEFAULT_ENCRYPTTOSELF;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefEncryptToSelf, &b);
		if (b) CheckDlgButton (hDlg, IDC_ENCRYPTTOSELF, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_ENCRYPTTOSELF, BST_UNCHECKED);

		// create and initialize decryption date/time picker control
		GetWindowRect (GetDlgItem (hDlg, IDC_DECRYPTCACHETIME), &rc);
		MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
		pcds->hwndDecryptDuration = CreateWindowEx (0, DATETIMEPICK_CLASS,
                             "DateTime",
                             WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP|
							 DTS_TIMEFORMAT,
                             rc.left, rc.top, 
							 rc.right-rc.left, rc.bottom-rc.top, 
							 hDlg, (HMENU)IDC_DECRYPTCACHETIME, 
							 g_hInst, NULL);
		SendMessage (pcds->hwndDecryptDuration, DTM_SETFORMAT, 0, 
						(LPARAM)"HH' : 'mm' : 'ss");
		SetWindowPos (pcds->hwndDecryptDuration, 
				GetDlgItem (hDlg, IDC_DECRYPTCACHEENABLE),
				0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		
		GetSystemTime (&(pcds->stDecryptDuration));
		uSec = DEFAULT_DECRYPTCACHESECONDS;
		PGPGetPrefNumber (PrefRefClient,kPGPPrefDecryptCacheSeconds, &uSec);
		u = uSec / 3600;
		pcds->stDecryptDuration.wHour = u;

		uSec -= (u * 3600);
		u = uSec / 60;
		pcds->stDecryptDuration.wMinute = u;

		uSec -= (u * 60);
		pcds->stDecryptDuration.wSecond = uSec;

		pcds->stDecryptDuration.wMilliseconds = 0;

		SendMessage (pcds->hwndDecryptDuration, DTM_SETSYSTEMTIME, GDT_VALID, 
						(LPARAM)&(pcds->stDecryptDuration));

		// initialize decrypt cache on/off
		b = DEFAULT_DECRYPTCACHEENABLE;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefDecryptCacheEnable, &b);
		if (b) {
			CheckDlgButton (hDlg, IDC_DECRYPTCACHEENABLE, BST_CHECKED);
			EnableWindow (pcds->hwndDecryptDuration, TRUE);
		}
		else {
			CheckDlgButton (hDlg, IDC_DECRYPTCACHEENABLE, BST_UNCHECKED);
			EnableWindow (pcds->hwndDecryptDuration, FALSE);
		}


		// create and initialize sign date/time picker control
		GetWindowRect (GetDlgItem (hDlg, IDC_SIGNCACHETIME), &rc);
		MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
		pcds->hwndSignDuration = CreateWindowEx (0, DATETIMEPICK_CLASS,
                             "DateTime",
                             WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP|
							 DTS_TIMEFORMAT,
                             rc.left, rc.top, 
							 rc.right-rc.left, rc.bottom-rc.top, 
							 hDlg, (HMENU)IDC_SIGNCACHETIME, 
							 g_hInst, NULL);
		SendMessage (pcds->hwndSignDuration, DTM_SETFORMAT, 0, 
						(LPARAM)"HH' : 'mm' : 'ss");
		SetWindowPos (pcds->hwndSignDuration, 
				GetDlgItem (hDlg, IDC_SIGNCACHEENABLE),
				0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		
		GetSystemTime (&(pcds->stSignDuration));
		uSec = DEFAULT_DECRYPTCACHESECONDS;
		PGPGetPrefNumber (PrefRefClient,kPGPPrefSignCacheSeconds, &uSec);
		u = uSec / 3600;
		pcds->stSignDuration.wHour = u;

		uSec -= (u * 3600);
		u = uSec / 60;
		pcds->stSignDuration.wMinute = u;

		uSec -= (u * 60);
		pcds->stSignDuration.wSecond = uSec;

		pcds->stSignDuration.wMilliseconds = 0;

		SendMessage (pcds->hwndSignDuration, DTM_SETSYSTEMTIME, GDT_VALID, 
						(LPARAM)&(pcds->stSignDuration));

		// initialize sign cache on/off
		b = DEFAULT_SIGNCACHEENABLE;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefSignCacheEnable, &b);
		if (b) {
			CheckDlgButton (hDlg, IDC_SIGNCACHEENABLE, BST_CHECKED);
			EnableWindow (pcds->hwndSignDuration, TRUE);
		}
		else {
			CheckDlgButton (hDlg, IDC_SIGNCACHEENABLE, BST_UNCHECKED);
			EnableWindow (pcds->hwndSignDuration, FALSE);
		}

		// comment block text
		SendDlgItemMessage (hDlg, IDC_COMMENTBLOCK, EM_SETLIMITTEXT, 
							COMMENTBLOCK_MAX_LENGTH, 0);
		sz[0] = '\0';
#if PGP_BUSINESS_SECURITY
		PGPGetPrefStringBuffer (PrefRefAdmin, kPGPPrefComments, 
								sizeof(sz), sz);
#endif
		if (sz[0]) 
			EnableWindow (GetDlgItem (hDlg, IDC_COMMENTBLOCK), FALSE);
		else 
			PGPGetPrefStringBuffer (PrefRefClient, kPGPPrefComment, 
									sizeof(sz), sz);
		SetDlgItemText (hDlg, IDC_COMMENTBLOCK, sz);

		b = DEFAULT_WIPECONFIRM;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefWarnOnWipe, &b);
		if (b) CheckDlgButton (hDlg, IDC_WIPECONFIRM, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_WIPECONFIRM, BST_UNCHECKED);

		// initialize fast key gen preference
		b = DEFAULT_USEFASTKEYGEN;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefFastKeyGen, &b);
		if (b) CheckDlgButton (hDlg, IDC_USEFASTKEYGEN, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_USEFASTKEYGEN, BST_UNCHECKED);

		// initialize wipe preferences
		SendDlgItemMessage (hDlg, IDC_NUMPASSESSPIN, UDM_SETRANGE,
				0, (LPARAM)MAKELONG (MAXWIPEPASSES, 1));

		u = DEFAULT_WIPEPASSES;
		PGPGetPrefNumber (PrefRefClient, kPGPPrefFileWipePasses, &u);
		SendDlgItemMessage (hDlg, IDC_NUMPASSESSPIN, UDM_SETPOS,
				0, (LPARAM)MAKELONG (u, 0));

		return TRUE;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aGeneralIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aGeneralIds); 
        break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {

		case IDC_DECRYPTCACHEENABLE :
			pcds = (PCACHEDURATIONSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			if (IsDlgButtonChecked (hDlg, 
						IDC_DECRYPTCACHEENABLE) == BST_CHECKED) 
				EnableWindow (pcds->hwndDecryptDuration, TRUE);
			else 
				EnableWindow (pcds->hwndDecryptDuration, FALSE);
			break;

		case IDC_SIGNCACHEENABLE :
			pcds = (PCACHEDURATIONSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			if (IsDlgButtonChecked (hDlg,
						IDC_SIGNCACHEENABLE) == BST_CHECKED) 
				EnableWindow (pcds->hwndSignDuration, TRUE);
			else 
				EnableWindow (pcds->hwndSignDuration, FALSE);
			break;
		}
		break;

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {
		case PSN_SETACTIVE :
			break;

		case PSN_HELP :
			WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
				IDH_PGPCLPREF_ENCRYPTDIALOG); 
			break;

		case PSN_APPLY :
			pcds = (PCACHEDURATIONSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

			SendMessage (pcds->hwndDecryptDuration, DTM_GETSYSTEMTIME, 0, 
						(LPARAM)&(pcds->stDecryptDuration));
			uEncrypt = pcds->stDecryptDuration.wHour * 3600;
			uEncrypt += (pcds->stDecryptDuration.wMinute * 60);
			uEncrypt += pcds->stDecryptDuration.wSecond;

			SendMessage (pcds->hwndSignDuration, DTM_GETSYSTEMTIME, 0, 
						(LPARAM)&(pcds->stSignDuration));
			uSign = pcds->stSignDuration.wHour * 3600;
			uSign += (pcds->stSignDuration.wMinute * 60);
			uSign += pcds->stSignDuration.wSecond;

			if (IsDlgButtonChecked (hDlg, IDC_ENCRYPTTOSELF) == BST_CHECKED)
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
								kPGPPrefEncryptToSelf, b));
			
			u = 0;
			b = FALSE;
			if (uEncrypt > 0) {
				if (IsDlgButtonChecked (hDlg, IDC_DECRYPTCACHEENABLE) ==
							BST_CHECKED) 
					b = TRUE;
			}
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefDecryptCacheEnable, b));
			PGPclErrorBox (hDlg, PGPSetPrefNumber (PrefRefClient,
									kPGPPrefDecryptCacheSeconds, uEncrypt));
			if (!b) u |= PGPCL_DECRYPTIONCACHE;

			b = FALSE;
			if (uSign > 0) {
				if (IsDlgButtonChecked (hDlg, IDC_SIGNCACHEENABLE) ==
							BST_CHECKED) 
					b = TRUE;
			}
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefSignCacheEnable, b));
			PGPclErrorBox (hDlg, PGPSetPrefNumber (PrefRefClient,
									kPGPPrefSignCacheSeconds, uSign));
			if (!b) u |= PGPCL_SIGNINGCACHE;

			if (u != 0)
				PGPclNotifyPurgePassphraseCache (u, 0);
			
			if (!GetDlgItemText (hDlg, IDC_COMMENTBLOCK, sz, sizeof(sz))) {
				sz[0] = '\0';
			}
			PGPclErrorBox (hDlg, PGPSetPrefString (PrefRefClient,
									kPGPPrefComment, sz));
			
			if (IsDlgButtonChecked (hDlg, IDC_USEFASTKEYGEN) ==
						BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefFastKeyGen, b));
            
			if (IsDlgButtonChecked (hDlg, IDC_WIPECONFIRM) ==
						BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefWarnOnWipe, b));

			u = GetDlgItemInt (hDlg, IDC_NUMPASSES, NULL, FALSE);
			if (u < 1) u = 1;
			if (u > MAXWIPEPASSES) u = MAXWIPEPASSES;
			PGPclErrorBox (hDlg, PGPSetPrefNumber (PrefRefClient,
									kPGPPrefFileWipePasses, u));

			returnErr = kPGPError_NoErr;
            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
            return TRUE;

		case PSN_KILLACTIVE :
			pcds = (PCACHEDURATIONSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

			SendMessage (pcds->hwndDecryptDuration, DTM_GETSYSTEMTIME, 0, 
						(LPARAM)&(pcds->stDecryptDuration));
			uEncrypt = pcds->stDecryptDuration.wHour * 3600;
			uEncrypt += (pcds->stDecryptDuration.wMinute * 60);
			uEncrypt += pcds->stDecryptDuration.wSecond;

			SendMessage (pcds->hwndSignDuration, DTM_GETSYSTEMTIME, 0, 
						(LPARAM)&(pcds->stSignDuration));
			uSign = pcds->stSignDuration.wHour * 3600;
			uSign += (pcds->stSignDuration.wMinute * 60);
			uSign += pcds->stSignDuration.wSecond;

			if (uEncrypt > MAXCACHETIMELIMIT)
			{
				PGPclMessageBox (hDlg, IDS_CAPTION, 
						IDS_INVALIDCACHEVALUE, MB_OK|MB_ICONHAND);
				SetFocus (pcds->hwndDecryptDuration); 
				SetWindowLong (hDlg, DWL_MSGRESULT, TRUE);
				return TRUE;
			}

			if (uSign > MAXCACHETIMELIMIT)
			{
				PGPclMessageBox (hDlg, IDS_CAPTION, 
						IDS_INVALIDCACHEVALUE, MB_OK|MB_ICONHAND);
				SetFocus (pcds->hwndSignDuration); 
				SetWindowLong (hDlg, DWL_MSGRESULT, TRUE);
				return TRUE;
			}

			SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			return TRUE;

		case PSN_RESET :
			returnErr = kPGPError_UserAbort;
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;
		}
	}

	return FALSE;

}

//	_____________________________________________________
//
//  Message processing function for property sheet dialog

static LRESULT WINAPI 
sEmailPropDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	PGPBoolean	b;
	UINT		u;

	switch (uMsg) {

	case WM_INITDIALOG:
		if (bNeedsCentering) 
		{
			HWND	hParent;
			RECT	rc;

			hParent = GetParent (hDlg);
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			bNeedsCentering = FALSE;
		}
	
		b = DEFAULT_MAILENCRYPTPGPMIME;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefMailEncryptPGPMIME, &b);
		if (b) CheckDlgButton (hDlg, IDC_ENCRYPTPGPMIME, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_ENCRYPTPGPMIME, BST_UNCHECKED);

		u = DEFAULT_WORDWRAPWIDTH;
		PGPGetPrefNumber (PrefRefClient, kPGPPrefWordWrapWidth, &u);
		SetDlgItemInt (hDlg, IDC_WRAPNUMBER, u, FALSE);

		b = DEFAULT_WORDWRAPENABLE;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefWordWrapEnable, &b);
		if (b) {
			CheckDlgButton (hDlg, IDC_WRAPENABLE, BST_CHECKED);
			EnableWindow (GetDlgItem (hDlg, IDC_WRAPNUMBER), TRUE);
		}
		else {
			CheckDlgButton (hDlg, IDC_WRAPENABLE, BST_UNCHECKED);
			EnableWindow (GetDlgItem (hDlg, IDC_WRAPNUMBER), FALSE);
		}

		b = DEFAULT_MAILENCRYPTDEFAULT;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefMailEncryptDefault, &b);
		if (b) CheckDlgButton (hDlg, IDC_ENCRYPTBYDEFAULT, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_ENCRYPTBYDEFAULT, BST_UNCHECKED);

		b = DEFAULT_MAILSIGNDEFAULT;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefMailSignDefault, &b);
		if (b) CheckDlgButton (hDlg, IDC_SIGNBYDEFAULT, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_SIGNBYDEFAULT, BST_UNCHECKED);

		b = DEFAULT_AUTODECRYPT;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefAutoDecrypt, &b);
		if (b) CheckDlgButton (hDlg, IDC_AUTODECRYPT, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_AUTODECRYPT, BST_UNCHECKED);

		b = DEFAULT_USESECUREVIEWER;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefAlwaysUseSecureViewer, &b);
		if (b) CheckDlgButton (hDlg, IDC_USESECUREVIEWER, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_USESECUREVIEWER, BST_UNCHECKED);

		return TRUE;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aEmailIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aEmailIds); 
        break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDC_WRAPENABLE :
			if (IsDlgButtonChecked (hDlg, IDC_WRAPENABLE) == BST_CHECKED)
				EnableWindow (GetDlgItem (hDlg, IDC_WRAPNUMBER), TRUE);
			else
				EnableWindow (GetDlgItem (hDlg, IDC_WRAPNUMBER), FALSE);
			break;
		}
		break;

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {
		case PSN_SETACTIVE :
			break;

		case PSN_HELP :
			WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
				IDH_PGPCLPREF_EMAILDIALOG); 
			break;

		case PSN_APPLY :
			if (IsDlgButtonChecked (hDlg, IDC_WRAPENABLE) == BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
						kPGPPrefWordWrapEnable, b));
			
			if (b) 
			{
				u = GetDlgItemInt (hDlg, IDC_WRAPNUMBER, NULL, FALSE);
				PGPclErrorBox (hDlg, PGPSetPrefNumber (PrefRefClient,
						kPGPPrefWordWrapWidth, u));
			}

			if (IsDlgButtonChecked (hDlg, IDC_ENCRYPTPGPMIME) == BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
						kPGPPrefMailEncryptPGPMIME, b));
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
						kPGPPrefMailSignPGPMIME, b));
						
			if (IsDlgButtonChecked (
						hDlg, IDC_ENCRYPTBYDEFAULT) == BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
						kPGPPrefMailEncryptDefault, b));
			
			if (IsDlgButtonChecked (hDlg, IDC_SIGNBYDEFAULT) ==  BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
						kPGPPrefMailSignDefault, b));
		
			if (IsDlgButtonChecked (hDlg, IDC_AUTODECRYPT) == BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
						kPGPPrefAutoDecrypt, b));
			
			if (IsDlgButtonChecked (
						hDlg, IDC_USESECUREVIEWER) == BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
						kPGPPrefAlwaysUseSecureViewer, b));
			
			returnErr = kPGPError_NoErr;
            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
            return TRUE;

		case PSN_KILLACTIVE :
			if (IsDlgButtonChecked (hDlg, IDC_WRAPENABLE) == BST_CHECKED)
			{
				u = GetDlgItemInt (hDlg, IDC_WRAPNUMBER, NULL, FALSE);
				if (u == 0) 
				{
					PGPclMessageBox (hDlg, IDS_CAPTION, 
							IDS_INVALIDWRAP, MB_OK|MB_ICONHAND);
					SendDlgItemMessage (hDlg, IDC_WRAPNUMBER, EM_SETSEL, 0, -1);
					SetFocus (GetDlgItem (hDlg, IDC_WRAPNUMBER));
					SetWindowLong (hDlg, DWL_MSGRESULT, TRUE);
					return TRUE;
				}
			}
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
            return TRUE;

		case PSN_RESET :
			returnErr = kPGPError_UserAbort;
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;
		}
	}

	return FALSE;

}

//	_____________________________________________________
//
//  Message processing function for property sheet dialog

static VOID 
sSetHotKeyControl (
		HWND		hwnd, 
		PGPUInt32	u)
{
	UINT	uKey	= LOWORD (u);
	UINT	uMod	= 0;

	if (u & kPGPHotKeyAltModifier)
		uMod |= HOTKEYF_ALT;

	if (u & kPGPHotKeyCtrlModifier)
		uMod |= HOTKEYF_CONTROL;

	if (u & kPGPHotKeyExtModifier)
		uMod |= HOTKEYF_EXT;

	if (u & kPGPHotKeyShiftModifier)
		uMod |= HOTKEYF_SHIFT;

	SendMessage (hwnd, HKM_SETHOTKEY, MAKEWORD (LOBYTE (uKey), uMod), 0);

	if (u & kPGPHotKeyEnabled)
		EnableWindow (hwnd, TRUE);
	else
		EnableWindow (hwnd, FALSE);
}


//	_____________________________________________________
//
//  Message processing function for property sheet dialog

static PGPUInt32 
sGetHotKeyControl (
		HWND		hwnd)
{
	UINT	uKey;
	WORD	wMod;

	uKey = SendMessage (hwnd, HKM_GETHOTKEY, 0, 0);

	wMod = HIBYTE (LOWORD (uKey));
	uKey &= 0xFF;

	if (wMod & HOTKEYF_ALT)
		uKey |= kPGPHotKeyAltModifier;

	if (wMod & HOTKEYF_CONTROL)
		uKey |= kPGPHotKeyCtrlModifier;

	if (wMod & HOTKEYF_EXT)
		uKey |= kPGPHotKeyExtModifier;

	if (wMod & HOTKEYF_SHIFT)
		uKey |= kPGPHotKeyShiftModifier;

	return uKey;
}


//	_____________________________________________________
//
//  Message processing function for property sheet dialog

static BOOL 
sDoHotKeysOverlap (
		PGPHotKeys* photkeys)
{
	PGPUInt32* pStart	= &(photkeys->hotkeyPurgePassphraseCache);
	PGPUInt32* pEnd		= &(photkeys->hotkeyDiskUnmount);
	PGPUInt32* p;

	do 
	{
		p = pStart+1;
		while (p <= pEnd) 
		{
			if ((*p & kPGPHotKeyEnabled) &&
				(*p == *pStart))
				return TRUE;
			p++;
		}
		pStart++;
	}
	while (pStart < pEnd);

	return FALSE;
}


//	_____________________________________________________
//
//  determine if hotkey is already in use -- this works because
//	we have already disabled all PGP hotkeys (except PGPdisk)

static BOOL 
sIsHotKeyInUse (
		PGPUInt32 uHotKey)
{
	UINT	uMod		= 0;
	BOOL	bInUse		= TRUE;

	if (uHotKey & kPGPHotKeyAltModifier)
		uMod |= MOD_ALT;
	if (uHotKey & kPGPHotKeyCtrlModifier)
		uMod |= MOD_CONTROL;
	if (uHotKey & kPGPHotKeyShiftModifier)
		uMod |= MOD_SHIFT;

	if (RegisterHotKey (NULL, 0xC000, uMod, (uHotKey & 0xFF)))
	{
		UnregisterHotKey (NULL, 0xC000);
		bInUse = FALSE;
	}

	return bInUse;
}


//	_____________________________________________________
//
//  Message processing function for property sheet dialog

static LRESULT WINAPI 
sHotkeyPropDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	switch (uMsg) {

	case WM_INITDIALOG:
	{
		PGPHotKeys*		phks		= NULL;
		PGPSize			size;
		PGPHotKeys		hotkeys;
		PGPError		err;

		if (bNeedsCentering) 
		{
			HWND	hParent;
			RECT	rc;

			hParent = GetParent (hDlg);
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			bNeedsCentering = FALSE;
		}
	
		pgpCopyMemory (&sDefaultHotKeys, &hotkeys, sizeof(hotkeys));
		err = PGPGetPrefData (PrefRefClient, kPGPPrefHotKeyData, &size, &phks);

		if ((IsntPGPError (err)) &&
			(size == sizeof (hotkeys)))
				pgpCopyMemory (phks, &hotkeys, size);

		// initialize purge passphrase hotkey
		sSetHotKeyControl (GetDlgItem (hDlg, IDC_PURGEHOTKEY), 
				hotkeys.hotkeyPurgePassphraseCache);

		if (hotkeys.hotkeyPurgePassphraseCache & kPGPHotKeyEnabled) 
			CheckDlgButton (hDlg, IDC_ENABLEPURGECACHE, BST_CHECKED);
		else 
			CheckDlgButton (hDlg, IDC_ENABLEPURGECACHE, BST_UNCHECKED);

		// initialize encrypt hotkey
		sSetHotKeyControl (GetDlgItem (hDlg, IDC_ENCRYPTHOTKEY), 
				hotkeys.hotkeyEncrypt);

		if (hotkeys.hotkeyEncrypt & kPGPHotKeyEnabled) 
			CheckDlgButton (hDlg, IDC_ENABLEENCRYPT, BST_CHECKED);
		else 
			CheckDlgButton (hDlg, IDC_ENABLEENCRYPT, BST_UNCHECKED);

		// initialize sign hotkey
		sSetHotKeyControl (GetDlgItem (hDlg, IDC_SIGNHOTKEY), 
				hotkeys.hotkeySign);

		if (hotkeys.hotkeySign & kPGPHotKeyEnabled) 
			CheckDlgButton (hDlg, IDC_ENABLESIGN, BST_CHECKED);
		else 
			CheckDlgButton (hDlg, IDC_ENABLESIGN, BST_UNCHECKED);

		// initialize encrypt & sign hotkey
		sSetHotKeyControl (GetDlgItem (hDlg, IDC_ENCRYPTSIGNHOTKEY), 
				hotkeys.hotkeyEncryptSign);

		if (hotkeys.hotkeyEncryptSign & kPGPHotKeyEnabled) 
			CheckDlgButton (hDlg, IDC_ENABLEENCRYPTSIGN, BST_CHECKED);
		else 
			CheckDlgButton (hDlg, IDC_ENABLEENCRYPTSIGN, BST_UNCHECKED);

		// initialize purge passphrase
		sSetHotKeyControl (GetDlgItem (hDlg, IDC_DECRYPTHOTKEY), 
				hotkeys.hotkeyDecrypt);

		if (hotkeys.hotkeyDecrypt & kPGPHotKeyEnabled) 
			CheckDlgButton (hDlg, IDC_ENABLEDECRYPT, BST_CHECKED);
		else 
			CheckDlgButton (hDlg, IDC_ENABLEDECRYPT, BST_UNCHECKED);

		if (IsntNull (phks))
			PGPDisposePrefData (PrefRefClient, phks);

		return TRUE;
	}

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aHotkeyIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aHotkeyIds); 
        break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDC_ENABLEPURGECACHE :
			EnableWindow (GetDlgItem (hDlg, IDC_PURGEHOTKEY), 
				(IsDlgButtonChecked (
						hDlg, IDC_ENABLEPURGECACHE) == BST_CHECKED));
			break;

		case IDC_ENABLEENCRYPT :
			EnableWindow (GetDlgItem (hDlg, IDC_ENCRYPTHOTKEY), 
				(IsDlgButtonChecked (
						hDlg, IDC_ENABLEENCRYPT) == BST_CHECKED));
			break;

		case IDC_ENABLESIGN :
			EnableWindow (GetDlgItem (hDlg, IDC_SIGNHOTKEY), 
				(IsDlgButtonChecked (
						hDlg, IDC_ENABLESIGN) == BST_CHECKED));
			break;

		case IDC_ENABLEENCRYPTSIGN :
			EnableWindow (GetDlgItem (hDlg, IDC_ENCRYPTSIGNHOTKEY), 
				(IsDlgButtonChecked (
						hDlg, IDC_ENABLEENCRYPTSIGN) == BST_CHECKED));
			break;

		case IDC_ENABLEDECRYPT :
			EnableWindow (GetDlgItem (hDlg, IDC_DECRYPTHOTKEY), 
				(IsDlgButtonChecked (
						hDlg, IDC_ENABLEDECRYPT) == BST_CHECKED));
			break;
		}
		break;

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {
		case PSN_SETACTIVE :
			if (hWndFocus)
			{
				SetFocus (hWndFocus);
				hWndFocus = NULL;
			}
			break;

		case PSN_HELP :
			WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
				IDH_PGPCLPREF_HOTKEYDIALOG); 
			break;

		case PSN_APPLY :
		{
			PGPHotKeys	hotkeys;

			// save purge passphrase hotkey
			hotkeys.hotkeyPurgePassphraseCache =
				sGetHotKeyControl (GetDlgItem (hDlg, IDC_PURGEHOTKEY));

			if (IsDlgButtonChecked (
					hDlg, IDC_ENABLEPURGECACHE) == BST_CHECKED)
			{
				hotkeys.hotkeyPurgePassphraseCache |= kPGPHotKeyEnabled;
			}

			// save encrypt hotkey
			hotkeys.hotkeyEncrypt =
				sGetHotKeyControl (GetDlgItem (hDlg, IDC_ENCRYPTHOTKEY));

			if (IsDlgButtonChecked (
					hDlg, IDC_ENABLEENCRYPT) == BST_CHECKED)
			{
				hotkeys.hotkeyEncrypt |= kPGPHotKeyEnabled;
			}

			// save sign hotkey
			hotkeys.hotkeySign =
				sGetHotKeyControl (GetDlgItem (hDlg, IDC_SIGNHOTKEY));

			if (IsDlgButtonChecked (
					hDlg, IDC_ENABLESIGN) == BST_CHECKED)
			{
				hotkeys.hotkeySign |= kPGPHotKeyEnabled;
			}

			// save encrypt & sign hotkey
			hotkeys.hotkeyEncryptSign =
				sGetHotKeyControl (GetDlgItem (hDlg, IDC_ENCRYPTSIGNHOTKEY));

			if (IsDlgButtonChecked (
					hDlg, IDC_ENABLEENCRYPTSIGN) == BST_CHECKED)
			{
				hotkeys.hotkeyEncryptSign |= kPGPHotKeyEnabled;
			}

			// save decrypt hotkey
			hotkeys.hotkeyDecrypt =
				sGetHotKeyControl (GetDlgItem (hDlg, IDC_DECRYPTHOTKEY));

			if (IsDlgButtonChecked (
					hDlg, IDC_ENABLEDECRYPT) == BST_CHECKED)
			{
				hotkeys.hotkeyDecrypt |= kPGPHotKeyEnabled;
			}

			// write prefs
			PGPclErrorBox (hDlg, PGPSetPrefData (PrefRefClient,
						kPGPPrefHotKeyData, sizeof(hotkeys), &hotkeys));

			returnErr = kPGPError_NoErr;

			// allow closure
            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
		}

		case PSN_KILLACTIVE :
		{
			PGPHotKeys	hotkeys;

			// get purge passphrase hotkey
			hotkeys.hotkeyPurgePassphraseCache =
				sGetHotKeyControl (GetDlgItem (hDlg, IDC_PURGEHOTKEY));

			if (IsDlgButtonChecked (
					hDlg, IDC_ENABLEPURGECACHE) == BST_CHECKED)
			{
				if (sIsHotKeyInUse (hotkeys.hotkeyPurgePassphraseCache))
				{
					PGPclMessageBox (hDlg, IDS_CAPTION, 
							IDS_PURGEHOTKEYINUSE, MB_ICONSTOP|MB_OK);
					SetFocus (GetDlgItem (hDlg, IDC_PURGEHOTKEY));
					SetWindowLong (hDlg, DWL_MSGRESULT, TRUE);
					return TRUE;
				}
				hotkeys.hotkeyPurgePassphraseCache |= kPGPHotKeyEnabled;
			}

			// get encrypt hotkey
			hotkeys.hotkeyEncrypt =
				sGetHotKeyControl (GetDlgItem (hDlg, IDC_ENCRYPTHOTKEY));

			if (IsDlgButtonChecked (
					hDlg, IDC_ENABLEENCRYPT) == BST_CHECKED)
			{
				if (sIsHotKeyInUse (hotkeys.hotkeyEncrypt))
				{
					PGPclMessageBox (hDlg, IDS_CAPTION, 
							IDS_ENCRYPTHOTKEYINUSE, MB_ICONSTOP|MB_OK);
					hWndFocus = GetDlgItem (hDlg, IDC_ENCRYPTHOTKEY);
					SetFocus (GetDlgItem (hDlg, IDC_ENCRYPTHOTKEY));
					SetWindowLong (hDlg, DWL_MSGRESULT, TRUE);
					return TRUE;
				}
				hotkeys.hotkeyEncrypt |= kPGPHotKeyEnabled;
			}

			// get sign hotkey
			hotkeys.hotkeySign =
				sGetHotKeyControl (GetDlgItem (hDlg, IDC_SIGNHOTKEY));

			if (IsDlgButtonChecked (
					hDlg, IDC_ENABLESIGN) == BST_CHECKED)
			{
				if (sIsHotKeyInUse (hotkeys.hotkeySign))
				{
					PGPclMessageBox (hDlg, IDS_CAPTION, 
							IDS_SIGNHOTKEYINUSE, MB_ICONSTOP|MB_OK);
					SetFocus (GetDlgItem (hDlg, IDC_SIGNHOTKEY));
					SetWindowLong (hDlg, DWL_MSGRESULT, TRUE);
					return TRUE;
				}
				hotkeys.hotkeySign |= kPGPHotKeyEnabled;
			}

			// get encrypt & sign hotkey
			hotkeys.hotkeyEncryptSign =
				sGetHotKeyControl (GetDlgItem (hDlg, IDC_ENCRYPTSIGNHOTKEY));

			if (IsDlgButtonChecked (
					hDlg, IDC_ENABLEENCRYPTSIGN) == BST_CHECKED)
			{
				if (sIsHotKeyInUse (hotkeys.hotkeyEncryptSign))
				{
					PGPclMessageBox (hDlg, IDS_CAPTION, 
							IDS_ENCRYPTSIGNHOTKEYINUSE, MB_ICONSTOP|MB_OK);
					SetFocus (GetDlgItem (hDlg, IDC_ENCRYPTSIGNHOTKEY));
					SetWindowLong (hDlg, DWL_MSGRESULT, TRUE);
					return TRUE;
				}
				hotkeys.hotkeyEncryptSign |= kPGPHotKeyEnabled;
			}

			// get decrypt hotkey
			hotkeys.hotkeyDecrypt =
				sGetHotKeyControl (GetDlgItem (hDlg, IDC_DECRYPTHOTKEY));

			if (IsDlgButtonChecked (
					hDlg, IDC_ENABLEDECRYPT) == BST_CHECKED)
			{
				if (sIsHotKeyInUse (hotkeys.hotkeyDecrypt))
				{
					PGPclMessageBox (hDlg, IDS_CAPTION, 
							IDS_DECRYPTHOTKEYINUSE, MB_ICONSTOP|MB_OK);
					SetFocus (GetDlgItem (hDlg, IDC_DECRYPTHOTKEY));
					SetWindowLong (hDlg, DWL_MSGRESULT, TRUE);
					return TRUE;
				}
				hotkeys.hotkeyDecrypt |= kPGPHotKeyEnabled;
			}

			// check for overlapping hotkeys
			if (sDoHotKeysOverlap (&hotkeys))
			{
				PGPclMessageBox (hDlg, IDS_CAPTION, IDS_OVERLAPPINGHOTKEYS,
						MB_ICONSTOP|MB_OK);
				SetWindowLong (hDlg, DWL_MSGRESULT, TRUE);
				return TRUE;
			}

			// allow closure
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
            return TRUE;
		}

		case PSN_RESET :
			returnErr = kPGPError_UserAbort;
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;
		}
	}

	return FALSE;

}

//	_____________________________________________________
//
//  Initialize preferred algorithm combo box on basis of
//	enabled algorithms

static VOID
sInitPreferredAlgComboBox (HWND hDlg, UINT uInit) {

	INT		iIdx, iPrevAlg;
	CHAR	sz[32];
	UINT	uIndex;

	if (uInit) iPrevAlg = uInit;
	else {
		iIdx = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
									CB_GETCURSEL, 0, 0);	
		if (iIdx != CB_ERR) 
			iPrevAlg = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
									CB_GETITEMDATA, iIdx, 0);
		else 
			iPrevAlg = kPGPCipherAlgorithm_None;
	}
	SendDlgItemMessage (hDlg, IDC_PREFERREDALG, CB_RESETCONTENT, 0, 0);

	iIdx = -1;

	// put item into combobox for each enabled algorithm
	if (IsDlgButtonChecked (hDlg, IDC_ENABLECAST) == BST_CHECKED) {
		LoadString (g_hInst, IDS_CAST, sz, sizeof(sz));
		uIndex = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
								CB_ADDSTRING, 0, (LPARAM)sz);
		if (uIndex != CB_ERR)
			SendDlgItemMessage (hDlg, IDC_PREFERREDALG, CB_SETITEMDATA, 
				uIndex, (LPARAM)kPGPCipherAlgorithm_CAST5);
		if (iPrevAlg == kPGPCipherAlgorithm_CAST5) iIdx = uIndex;
	}

	if (IsDlgButtonChecked (hDlg, IDC_ENABLEIDEA) == BST_CHECKED) {
		LoadString (g_hInst, IDS_IDEA, sz, sizeof(sz));
		uIndex = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
								CB_ADDSTRING, 0, (LPARAM)sz);
		if (uIndex != CB_ERR)
			SendDlgItemMessage (hDlg, IDC_PREFERREDALG, CB_SETITEMDATA, 
				uIndex, (LPARAM)kPGPCipherAlgorithm_IDEA);
		if (iPrevAlg == kPGPCipherAlgorithm_IDEA) iIdx = uIndex;
	}

	if (IsDlgButtonChecked (hDlg, IDC_ENABLE3DES) == BST_CHECKED) {
		LoadString (g_hInst, IDS_3DES, sz, sizeof(sz));
		uIndex = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
								CB_ADDSTRING, 0, (LPARAM)sz);
		if (uIndex != CB_ERR)
			SendDlgItemMessage (hDlg, IDC_PREFERREDALG, CB_SETITEMDATA, 
				uIndex, (LPARAM)kPGPCipherAlgorithm_3DES);
		if (iPrevAlg == kPGPCipherAlgorithm_3DES) iIdx = uIndex;
	}

#if ENABLE_TWOFISH
	if (IsDlgButtonChecked (hDlg, IDC_ENABLETWOFISH) == BST_CHECKED) {
		LoadString (g_hInst, IDS_TWOFISH, sz, sizeof(sz));
		uIndex = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
								CB_ADDSTRING, 0, (LPARAM)sz);
		if (uIndex != CB_ERR)
			SendDlgItemMessage (hDlg, IDC_PREFERREDALG, CB_SETITEMDATA, 
				uIndex, (LPARAM)kPGPCipherAlgorithm_Twofish256);
		if (iPrevAlg == kPGPCipherAlgorithm_Twofish256) iIdx = uIndex;
	}
#endif //ENABLE_TWOFISH

	if (iIdx < 0) iIdx = 0;
	SendDlgItemMessage (hDlg, IDC_PREFERREDALG, CB_SETCURSEL, iIdx, 0);

}

//	_____________________________________________________
//
//  Message processing function for property sheet dialog

static LRESULT WINAPI 
sAdvancedPropDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	PGPBoolean			b;
	UINT				u, uNumAlgs;
	INT					iIdx;
	PGPCipherAlgorithm*	pAlgs;
	PGPError			err;
	PGPSize				size;

	switch (uMsg) {

	case WM_INITDIALOG:
		if (bNeedsCentering) 
		{
			HWND	hParent;
			RECT	rc;

			hParent = GetParent (hDlg);
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			bNeedsCentering = FALSE;
		}

		// enable checkboxes of those ciphers supported by SDK
		err = 
			PGPclCheckSDKSupportForCipherAlg (kPGPCipherAlgorithm_CAST5);
		if (IsntPGPError (err))
			EnableWindow (GetDlgItem (hDlg, IDC_ENABLECAST), TRUE);
		err = 
			PGPclCheckSDKSupportForCipherAlg (kPGPCipherAlgorithm_3DES);
		if (IsntPGPError (err))
			EnableWindow (GetDlgItem (hDlg, IDC_ENABLE3DES), TRUE);
		err = 
			PGPclCheckSDKSupportForCipherAlg (kPGPCipherAlgorithm_IDEA);
		if (IsntPGPError (err))
			EnableWindow (GetDlgItem (hDlg, IDC_ENABLEIDEA), TRUE);
#if ENABLE_TWOFISH
		err = 
			PGPclCheckSDKSupportForCipherAlg (kPGPCipherAlgorithm_Twofish256);
		if (IsntPGPError (err))
			EnableWindow (GetDlgItem (hDlg, IDC_ENABLETWOFISH), TRUE);
#endif //ENABLE_TWOFISH
		
		// check boxes of enabled algorithms
		PGPGetPrefData (PrefRefClient, kPGPPrefAllowedAlgorithmsList,
							  &size, &pAlgs);
		uNumAlgs = size / sizeof(PGPCipherAlgorithm);

		for (u=0; u<uNumAlgs; u++) {
			err = PGPclCheckSDKSupportForCipherAlg (pAlgs[u]);
			if (IsntPGPError (err)) {
				switch (pAlgs[u]) {
				case kPGPCipherAlgorithm_CAST5 :
					CheckDlgButton (hDlg, IDC_ENABLECAST, BST_CHECKED);
					break;

				case kPGPCipherAlgorithm_3DES :
					CheckDlgButton (hDlg, IDC_ENABLE3DES, BST_CHECKED);
					break;

				case kPGPCipherAlgorithm_IDEA :
					CheckDlgButton (hDlg, IDC_ENABLEIDEA, BST_CHECKED);
					break;
#if ENABLE_TWOFISH
				case kPGPCipherAlgorithm_Twofish256 :
					CheckDlgButton (hDlg, IDC_ENABLETWOFISH, BST_CHECKED);
					break;
#endif //ENABLE_TWOFISH
				}
			}
		}
		if (pAlgs) PGPDisposePrefData (PrefRefClient, pAlgs);

		// initialize preferred algorithm
		u = DEFAULT_PREFERREDALGORITHM;
		PGPGetPrefNumber (PrefRefClient, kPGPPrefPreferredAlgorithm, &u);
		sInitPreferredAlgComboBox (hDlg, u);

		// initialize trust model preferences
		b = DEFAULT_DISPLAYMARGVALIDITY;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefDisplayMarginalValidity,&b);
		if (b) CheckDlgButton (hDlg, IDC_DISPMARGVALIDITY, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_DISPMARGVALIDITY, BST_UNCHECKED);

		b = DEFAULT_MARGVALIDISINVALID;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefMarginalIsInvalid, &b);
		if (b) CheckDlgButton (hDlg, IDC_MARGVALIDISINVALID, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_MARGVALIDISINVALID, BST_UNCHECKED);

		b = DEFAULT_WARNONADK;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefWarnOnADK, &b);
		if (b) CheckDlgButton (hDlg, IDC_WARNONADK, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_WARNONADK, BST_UNCHECKED);

		// initialize export format
		b = DEFAULT_EXPORTCOMPATIBLE;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefExportKeysCompatible, &b);
		if (b) 
			CheckRadioButton (hDlg, IDC_EXPORTCOMPATIBLE, 
							IDC_EXPORTCOMPLETE, IDC_EXPORTCOMPATIBLE);
		else 
			CheckRadioButton (hDlg, IDC_EXPORTCOMPATIBLE, 
							IDC_EXPORTCOMPLETE, IDC_EXPORTCOMPLETE);

		return TRUE;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aAdvancedIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aAdvancedIds); 
        break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDC_ENABLECAST :
			// if unchecked, check it
			if (IsDlgButtonChecked (hDlg, IDC_ENABLECAST) == BST_UNCHECKED) {
				CheckDlgButton (hDlg, IDC_ENABLECAST, BST_CHECKED);
				sInitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			// if checked, make sure it's not the "last" one
			if ((IsDlgButtonChecked (hDlg, IDC_ENABLE3DES) == BST_CHECKED) ||
				(IsDlgButtonChecked (hDlg, IDC_ENABLEIDEA) == BST_CHECKED) ||
				(IsDlgButtonChecked (hDlg, IDC_ENABLETWOFISH) == BST_CHECKED)) {
				CheckDlgButton (hDlg, IDC_ENABLECAST, BST_UNCHECKED);
				sInitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			PGPclMessageBox (hDlg, IDS_CAPTION, IDS_NEEDALGORITHM,
								MB_OK|MB_ICONEXCLAMATION);
			break;

		case IDC_ENABLE3DES :
			// if unchecked, check it
			if (IsDlgButtonChecked (hDlg, IDC_ENABLE3DES) == BST_UNCHECKED) {
				CheckDlgButton (hDlg, IDC_ENABLE3DES, BST_CHECKED);
				sInitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			// if checked, make sure it's not the "last" one
			if ((IsDlgButtonChecked (hDlg, IDC_ENABLECAST) == BST_CHECKED) ||
				(IsDlgButtonChecked (hDlg, IDC_ENABLEIDEA) == BST_CHECKED) ||
				(IsDlgButtonChecked (hDlg, IDC_ENABLETWOFISH) == BST_CHECKED)) {
				CheckDlgButton (hDlg, IDC_ENABLE3DES, BST_UNCHECKED);
				sInitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			PGPclMessageBox (hDlg, IDS_CAPTION, IDS_NEEDALGORITHM,
								MB_OK|MB_ICONEXCLAMATION);
			break;

		case IDC_ENABLEIDEA :
			// if unchecked, check it
			if (IsDlgButtonChecked (hDlg, IDC_ENABLEIDEA) == BST_UNCHECKED) {
				CheckDlgButton (hDlg, IDC_ENABLEIDEA, BST_CHECKED);
				sInitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			// if checked, make sure it's not the "last" one
			if ((IsDlgButtonChecked (hDlg, IDC_ENABLECAST) == BST_CHECKED) ||
				(IsDlgButtonChecked (hDlg, IDC_ENABLE3DES) == BST_CHECKED) ||
				(IsDlgButtonChecked (hDlg, IDC_ENABLETWOFISH) == BST_CHECKED)) {
				CheckDlgButton (hDlg, IDC_ENABLEIDEA, BST_UNCHECKED);
				sInitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			PGPclMessageBox (hDlg, IDS_CAPTION, IDS_NEEDALGORITHM,
								MB_OK|MB_ICONEXCLAMATION);
			break;

#if ENABLE_TWOFISH
		case IDC_ENABLETWOFISH :
			// if unchecked, check it
			if (IsDlgButtonChecked (hDlg, IDC_ENABLETWOFISH) == BST_UNCHECKED) {
				CheckDlgButton (hDlg, IDC_ENABLETWOFISH, BST_CHECKED);
				sInitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			// if checked, make sure it's not the "last" one
			if ((IsDlgButtonChecked (hDlg, IDC_ENABLECAST) == BST_CHECKED) ||
				(IsDlgButtonChecked (hDlg, IDC_ENABLE3DES) == BST_CHECKED) ||
				(IsDlgButtonChecked (hDlg, IDC_ENABLEIDEA) == BST_CHECKED)) {
				CheckDlgButton (hDlg, IDC_ENABLETWOFISH, BST_UNCHECKED);
				sInitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			PGPclMessageBox (hDlg, IDS_CAPTION, IDS_NEEDALGORITHM,
								MB_OK|MB_ICONEXCLAMATION);
			break;
#endif //ENABLE_TWOFISH
		}
		break;

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {
		case PSN_SETACTIVE :
			break;

		case PSN_HELP :
			WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
				IDH_PGPCLPREF_ADVANCEDDIALOG); 
			break;

		case PSN_APPLY :
			// get preferred algorithm
			iIdx = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
										CB_GETCURSEL, 0, 0);	
			if (iIdx != CB_ERR) 
				u = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
									CB_GETITEMDATA, iIdx, 0);
			else 
				u = kPGPCipherAlgorithm_None;
			PGPclErrorBox (hDlg, PGPSetPrefNumber (PrefRefClient, 
										kPGPPrefPreferredAlgorithm, u));
			
			// save enabled ciphers
			pAlgs = clAlloc (4 * sizeof(PGPCipherAlgorithm));
			pAlgs[0] = u;	// first element is preferred

			if (pAlgs[0] == kPGPCipherAlgorithm_None) u = 0;
			else u = 1;

			if (IsDlgButtonChecked (hDlg, IDC_ENABLECAST) == BST_CHECKED) {
				if (pAlgs[0] != kPGPCipherAlgorithm_CAST5) 
					pAlgs[u++] = kPGPCipherAlgorithm_CAST5;
			}
			if (IsDlgButtonChecked (hDlg, IDC_ENABLE3DES) == BST_CHECKED) {
				if (pAlgs[0] != kPGPCipherAlgorithm_3DES) 
					pAlgs[u++] = kPGPCipherAlgorithm_3DES;
			}
			if (IsDlgButtonChecked (hDlg, IDC_ENABLEIDEA) == BST_CHECKED) {
				if (pAlgs[0] != kPGPCipherAlgorithm_IDEA) 
					pAlgs[u++] = kPGPCipherAlgorithm_IDEA;
			}
#if ENABLE_TWOFISH
			if (IsDlgButtonChecked (hDlg, IDC_ENABLETWOFISH) == BST_CHECKED) {
				if (pAlgs[0] != kPGPCipherAlgorithm_Twofish256) 
					pAlgs[u++] = kPGPCipherAlgorithm_Twofish256;
			}
#endif //ENABLE_TWOFISH
			PGPSetPrefData (PrefRefClient, kPGPPrefAllowedAlgorithmsList,
							  u * sizeof(PGPCipherAlgorithm), pAlgs);
			clFree (pAlgs);

			// save trust model preferences
			if (IsDlgButtonChecked (
							hDlg, IDC_DISPMARGVALIDITY) == BST_CHECKED) 
				b = TRUE;
			else
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefDisplayMarginalValidity, b));
			if (IsDlgButtonChecked (
							hDlg, IDC_MARGVALIDISINVALID) == BST_CHECKED) 
				b = TRUE;
			else
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefMarginalIsInvalid, b));

			if (IsDlgButtonChecked (
							hDlg, IDC_WARNONADK) == BST_CHECKED) 
				b = TRUE;
			else
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefWarnOnADK, b));

			// save export format preferences
			if (IsDlgButtonChecked (
							hDlg, IDC_EXPORTCOMPATIBLE) == BST_CHECKED) 
				b = TRUE;
			else
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefExportKeysCompatible, b));

			returnErr = kPGPError_NoErr;
            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
			return TRUE;

		case PSN_KILLACTIVE :
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
            return TRUE;

		case PSN_RESET :
			returnErr = kPGPError_UserAbort;
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;
		}
	}

	return FALSE;

}


//	_____________________________________________________
//
//  Message processing function for property sheet dialog

static LRESULT WINAPI 
sKeyserverPropDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	PGPBoolean	b;

	switch (uMsg) {

	case WM_INITDIALOG:
		if (bNeedsCentering) 
		{
			HWND	hParent;
			RECT	rc;

			hParent = GetParent (hDlg);
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			bNeedsCentering = FALSE;
		}

		// load keyset if not already loaded
		if (!PGPKeySetRefIsValid (keysetMain))
		{
			HCURSOR	hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
			PGPclErrorBox (hDlg,
					PGPOpenDefaultKeyRings (ContextRef, 0, &keysetMain));
			SetCursor (hCursorOld);
		}

		hWndTree = CLCreateKeyserverTreeList (PrefRefClient, 
										ContextRef, hDlg, keysetMain);
		SetWindowPos (hWndTree, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		// initialize "Sync On ..." checkboxes
		b = DEFAULT_KEYSERVERSYNCONUNKNOWN;
		PGPGetPrefBoolean (PrefRefClient, 
										kPGPPrefKeyServerSyncUnknownKeys, &b);
		if (b) CheckDlgButton (hDlg, IDC_SYNCONUNKNOWN, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_SYNCONUNKNOWN, BST_UNCHECKED);

		b = DEFAULT_KEYSERVERSYNCONADDNAME;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefKeyServerSyncOnAdd, &b);
		if (b) CheckDlgButton (hDlg, IDC_SYNCONADDNAME, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_SYNCONADDNAME, BST_UNCHECKED);

		b = DEFAULT_KEYSERVERSYNCONKEYSIGN;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefKeyServerSyncOnKeySign, &b);
		if (b) CheckDlgButton (hDlg, IDC_SYNCONKEYSIGN, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_SYNCONKEYSIGN, BST_UNCHECKED);

		b = DEFAULT_KEYSERVERSYNCONREVOKE;
		PGPGetPrefBoolean (PrefRefClient, 
								kPGPPrefKeyServerSyncOnRevocation, &b);
		if (b) CheckDlgButton (hDlg, IDC_SYNCONREVOKE, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_SYNCONREVOKE, BST_UNCHECKED);

		b = DEFAULT_KEYSERVERSYNCONVERIFY;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefKeyServerSyncOnVerify, &b);
		if (b) CheckDlgButton (hDlg, IDC_SYNCONVERIFY, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_SYNCONVERIFY, BST_UNCHECKED);

		return TRUE;

	case WM_KEYDOWN :
	case WM_SYSKEYDOWN :
	case WM_KEYUP :
	case WM_CHAR :
		if (GetFocus () == hWndTree)
			SendMessage (hWndTree, uMsg, wParam, lParam);
		break;

	case WM_DESTROY :
		CLDestroyKeyserverTreeList (hWndTree);
		break;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aServerIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aServerIds); 
        break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDM_NEWSERVER :
		case IDC_NEWKEYSERVER :
			CLNewKeyserver (hDlg, hWndTree);
			break;

		case IDM_REMOVESERVER :
		case IDC_REMOVEKEYSERVER :
			CLRemoveKeyserver (hDlg, hWndTree);
			break;

		case IDM_EDITSERVER :
		case IDC_EDITKEYSERVER :
			CLEditKeyserver (hDlg, hWndTree);
			break;

		case IDM_SETASROOT :
		case IDC_SETASROOT :
			CLSetKeyserverAsRoot (hDlg, hWndTree);
			break;

		case IDM_MOVEUP :
		case IDC_MOVEUP :
			CLMoveKeyserverUp (hDlg, hWndTree);
			break;

		case IDM_MOVEDOWN :
		case IDC_MOVEDOWN :
			CLMoveKeyserverDown (hDlg, hWndTree);
			break;
		}
		break;

	case WM_NOTIFY :
		if (((NMHDR FAR *) lParam)->hwndFrom == hWndTree) {
			CLProcessKeyserverTreeList (hDlg, hWndTree, wParam, lParam);
		}
		else {
			switch (((NMHDR FAR *) lParam)->code) {
			case PSN_SETACTIVE :
				break;
	
			case PSN_HELP :
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
					IDH_PGPCLPREF_KEYSERVERDIALOG); 
				break;
	
			case PSN_APPLY :	
				CLSaveKeyserverPrefs (PrefRefClient, hDlg, hWndTree);

				// get and save checkbox values
				if (IsDlgButtonChecked (hDlg,IDC_SYNCONUNKNOWN)==BST_CHECKED) 
					b = TRUE;
				else 
					b = FALSE;
				PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefKeyServerSyncUnknownKeys, b));
			
				if (IsDlgButtonChecked (hDlg,IDC_SYNCONADDNAME) ==
															BST_CHECKED) 
					b = TRUE;
				else 
					b = FALSE;
				PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefKeyServerSyncOnAdd, b));
			
				if (IsDlgButtonChecked (hDlg,IDC_SYNCONKEYSIGN) ==
															BST_CHECKED) 
					b = TRUE;
				else 
					b = FALSE;
				PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefKeyServerSyncOnKeySign, b));
			
				if (IsDlgButtonChecked (hDlg,IDC_SYNCONREVOKE) == 
															BST_CHECKED) 
					b = TRUE;
				else 
					b = FALSE;
				PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefKeyServerSyncOnRevocation, b));
			
				if (IsDlgButtonChecked (hDlg,IDC_SYNCONVERIFY) == 
															BST_CHECKED) 
					b = TRUE;
				else 
					b = FALSE;
				PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefKeyServerSyncOnVerify, b));
				
				bKeyserverPrefsWritten = TRUE;

				returnErr = kPGPError_NoErr;
	            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
	            return TRUE;
	
			case PSN_KILLACTIVE :
	            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
	            return TRUE;
	
			case PSN_RESET :
				returnErr = kPGPError_UserAbort;
	            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
				break;
			}
		}
	}

	return FALSE;

}

//	_____________________________________________________
//
//  get root CA cert from prefs

PGPError PGPclExport
PGPclGetRootCACertPrefs (
		PGPContextRef	context,
		PGPPrefRef		prefref, 
		PGPKeySetRef	keyset,
		PGPKeyRef*		pkeyCert,
		PGPSigRef*		psigCert)
{
	PGPByte*		pIASNX509			= NULL;
	PGPByte*		pExpKeyIDX509		= NULL;

	PGPUInt32		uX509Alg;
	PGPUInt32		uX509IASNLength;
	PGPSize			size;
	PGPError		err;

	PGPGetPrefNumber (prefref, 
						kPGPPrefCARootKeyAlg, 
						&uX509Alg);

	if (uX509Alg == kPGPPublicKeyAlgorithm_Invalid)
		err = kPGPError_Win32_NoRootCACertPrefs;
	else
	{
		err = PGPGetPrefData (prefref,
						kPGPPrefCARootKeyID,
						&size, &pExpKeyIDX509); CKERR;

		err = PGPGetPrefData (prefref,
						kPGPPrefCARootX509IASN,
						&uX509IASNLength, &pIASNX509); CKERR;

		err = PGPX509CertFromExport (context, uX509Alg, pExpKeyIDX509, 
						pIASNX509, uX509IASNLength, keyset,
						pkeyCert, psigCert); 

		if (err == kPGPError_ItemNotFound)
			err = kPGPError_Win32_NoRootCACertPrefs;
	}

done:
	if (pExpKeyIDX509)
		PGPDisposePrefData (prefref, pExpKeyIDX509);
	if (pIASNX509)
		PGPDisposePrefData (prefref, pIASNX509);

	return err;
}


//	_____________________________________________________
//
//  set root CA cert prefs

static PGPError 
sSetRootCACertPrefs (
		PGPContextRef	context,
		PGPPrefRef		prefref, 
		PGPKeyRef		keyCert,
		PGPSigRef		sigCert)
{
	PGPUInt32		uX509Alg;
	PGPByte			ExpKeyIDX509[kPGPMaxExportedKeyIDSize];
	PGPByte*		pIASNX509;
	PGPUInt32		uX509IASNLength;
	PGPError		err;

	err = PGPX509CertToExport (context, 
				keyCert, sigCert,
				&uX509Alg, ExpKeyIDX509, &pIASNX509, &uX509IASNLength);

	if (IsntPGPError (err)) 
	{
		err = PGPSetPrefNumber (prefref,
					kPGPPrefCARootKeyAlg, uX509Alg); CKERR;

		if (uX509Alg != kPGPPublicKeyAlgorithm_Invalid)
		{
			err = PGPSetPrefData (prefref,
					kPGPPrefCARootKeyID,
					kPGPMaxExportedKeyIDSize, ExpKeyIDX509); CKERR;

			if (IsntNull (pIASNX509))
			{
				err = PGPSetPrefData (prefref,
							kPGPPrefCARootX509IASN, 
							uX509IASNLength, pIASNX509); CKERR;
				PGPFreeData (pIASNX509);
			}
		}
	}
done:
	return err;
}


//	_____________________________________________________
//
//  get CA server info from prefs

PGPError 
CLGetCAServerPrefs (
		PGPContextRef		context,
		PGPPrefRef			prefref, 
		PGPKeyServerEntry**	ppentry)
{
	PGPError		err			= kPGPError_BadParams;
	PGPByte*		pExpKeyID	= NULL;
	PGPSize			size;
	PGPKeyID		keyid;

	if (!ppentry)
		return kPGPError_BadParams;

	*ppentry = PGPNewData (PGPGetContextMemoryMgr (context),
				2*sizeof(PGPKeyServerEntry), kPGPMemoryMgrFlags_Clear);

	if (*ppentry)
	{
		// server
		// use protocol entry to hold CA type
		err = PGPGetPrefNumber (prefref, 
					kPGPPrefCAType, 
					&(*ppentry)[0].protocol); CKERR;

		err = PGPGetPrefStringBuffer (prefref, 
					kPGPPrefCAServerURL, 
					kMaxServerNameLength, 
					(*ppentry)[0].serverDNS); CKERR;

		err = PGPGetPrefNumber (prefref, 
					kPGPPrefCAServerAuthKeyAlg, 
					&(*ppentry)[0].authAlg); CKERR;

		if ((*ppentry)[0].authAlg != kPGPPublicKeyAlgorithm_Invalid)
		{
			err = PGPGetPrefData (prefref,
						kPGPPrefCAServerAuthKeyID,
						&size, &pExpKeyID); CKERR;

			if (pExpKeyID)
			{
				err = PGPImportKeyID (pExpKeyID, &keyid); CKERR;
				err = PGPGetKeyIDString (&keyid, 
								kPGPKeyIDString_Full,
								(*ppentry)[0].authKeyIDString); CKERR;
				PGPDisposePrefData (prefref, pExpKeyID);
			}
		}

		// revocation server
		// use protocol entry to hold CA type
		(*ppentry)[1].protocol = (*ppentry)[0].protocol;

		err = PGPGetPrefStringBuffer (prefref, 
					kPGPPrefCARevocationServerURL, 
					kMaxServerNameLength, 
					(*ppentry)[1].serverDNS); CKERR;

		err = PGPGetPrefNumber (prefref, 
					kPGPPrefCARevocationServerAuthKeyAlg, 
					&(*ppentry)[1].authAlg); CKERR;

		if ((*ppentry)[1].authAlg != kPGPPublicKeyAlgorithm_Invalid)
		{
			err = PGPGetPrefData (prefref,
						kPGPPrefCARevocationServerAuthKeyID,
						&size, &pExpKeyID); CKERR;
			if (pExpKeyID)
			{
				err = PGPImportKeyID (pExpKeyID, &keyid); CKERR;
				err = PGPGetKeyIDString (&keyid, 
								kPGPKeyIDString_Full,
								(*ppentry)[1].authKeyIDString); CKERR;
				PGPDisposePrefData (prefref, pExpKeyID);
			}
		}
	}
	else 
		err = kPGPError_OutOfMemory;

done:
	return err;
}


//	_____________________________________________________
//
//  set CA server info

PGPError 
CLSetCAServerPrefs (
		PGPContextRef		context,
		PGPPrefRef			prefref, 
		PGPKeyServerEntry*	pentry)
{
	PGPError		err		= kPGPError_NoErr;

	PGPByte			expkeyid[kPGPMaxExportedKeyIDSize];
	PGPSize			size;
	PGPKeyID		keyid;

	if (pentry)
	{
		// server
		// use protocol entry to hold CA type
		err = PGPSetPrefNumber (prefref, 
					kPGPPrefCAType, 
					pentry->protocol); CKERR;

		err = PGPSetPrefString (prefref, 
					kPGPPrefCAServerURL, 
					pentry->serverDNS); CKERR;

		err = PGPSetPrefNumber (prefref, 
					kPGPPrefCAServerAuthKeyAlg, 
					pentry->authAlg); CKERR;

		if (pentry->authAlg != kPGPPublicKeyAlgorithm_Invalid)
		{
			err = PGPGetKeyIDFromString (
						pentry->authKeyIDString, &keyid); CKERR;

			err = PGPExportKeyID (&keyid, expkeyid, &size);

			err = PGPSetPrefData (prefref,
						kPGPPrefCAServerAuthKeyID,
						size, expkeyid); CKERR;
		}
	}
	else
		return kPGPError_BadParams;

done:
	return err;
}


//	_____________________________________________________
//
//  set CA revocation server info

PGPError 
CLSetCARevocationServerPrefs (
		PGPContextRef		context,
		PGPPrefRef			prefref, 
		PGPKeyServerEntry*	pentry)
{
	PGPError		err			= kPGPError_NoErr;

	PGPByte			expkeyid[kPGPMaxExportedKeyIDSize];
	PGPSize			size;
	PGPKeyID		keyid;

	if (pentry)
	{
		// revocation server
		err = PGPSetPrefString (prefref, 
					kPGPPrefCARevocationServerURL, 
					pentry->serverDNS); CKERR;

		err = PGPSetPrefNumber (prefref, 
					kPGPPrefCARevocationServerAuthKeyAlg, 
					pentry->authAlg); CKERR;

		if (pentry->authAlg != kPGPPublicKeyAlgorithm_Invalid)
		{
			err = PGPGetKeyIDFromString (
						pentry->authKeyIDString, &keyid); CKERR;

			err = PGPExportKeyID (&keyid, expkeyid, &size);

			err = PGPSetPrefData (prefref,
						kPGPPrefCARevocationServerAuthKeyID,
						size, expkeyid); CKERR;
		}
	}
	else
		return kPGPError_BadParams;

done:
	return err;
}


//	_____________________________________________________
//
//  Message processing function for property sheet dialog

static LRESULT WINAPI 
sCAPropDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	PCAPREFSSTRUCT	pcaps;

	switch (uMsg) {

	case WM_INITDIALOG:
	{
		CHAR		sz[256];
		UINT		u, uSelect;
		PGPUInt32	uType;

		SetWindowLong (hDlg, GWL_USERDATA, ((PROPSHEETPAGE*)lParam)->lParam);
		pcaps = (PCAPREFSSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

		if (bNeedsCentering) 
		{
			HWND	hParent;
			RECT	rc;

			hParent = GetParent (hDlg);
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			bNeedsCentering = FALSE;
		}

		// load keyset if not already loaded
		if (!PGPKeySetRefIsValid (keysetMain))
		{
			HCURSOR	hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
			PGPclErrorBox (hDlg,
					PGPOpenDefaultKeyRings (ContextRef, 0, &keysetMain));
			SetCursor (hCursorOld);
		}

		// initialize URLs
		PGPGetPrefStringBuffer (PrefRefClient, 
								kPGPPrefCAServerURL, 
								sizeof(sz), sz);
		SetDlgItemText (hDlg, IDC_URL, sz);

		PGPGetPrefStringBuffer (PrefRefClient, 
								kPGPPrefCARevocationServerURL, 
								sizeof(sz), sz);
		SetDlgItemText (hDlg, IDC_REVURL, sz);

		// init CA server type combo box
		uSelect = 0;
		uType = kPGPKeyServerClass_Invalid;
#if PGP_BUSINESS_SECURITY
		PGPGetPrefNumber (PrefRefAdmin, kPGPPrefAdminCAType, &uType);
#endif
		if (uType != kPGPKeyServerClass_Invalid) 
			EnableWindow (GetDlgItem (hDlg, IDC_TYPE), FALSE);
		else 
			PGPGetPrefNumber (PrefRefClient, kPGPPrefCAType, &uType);

		LoadString (g_hInst, IDS_CATYPENETTOOLS, sz, sizeof(sz));
		u = SendDlgItemMessage (hDlg, IDC_TYPE, 
								CB_ADDSTRING, 0, (LPARAM)sz);
		if (u != CB_ERR)
		{
			SendDlgItemMessage (hDlg, IDC_TYPE, CB_SETITEMDATA, 
								u, (LPARAM)kPGPKeyServerClass_NetToolsCA);
			if (uType == kPGPKeyServerClass_NetToolsCA)
				uSelect = u;
		}

		LoadString (g_hInst, IDS_CATYPEVERISIGN, sz, sizeof(sz));
		u = SendDlgItemMessage (hDlg, IDC_TYPE, 
								CB_ADDSTRING, 0, (LPARAM)sz);
		if (u != CB_ERR)
		{
			SendDlgItemMessage (hDlg, IDC_TYPE, CB_SETITEMDATA, 
								u, (LPARAM)kPGPKeyServerClass_Verisign);
			if (uType == kPGPKeyServerClass_Verisign)
				uSelect = u;
		}

		LoadString (g_hInst, IDS_CATYPEENTRUST, sz, sizeof(sz));
		u = SendDlgItemMessage (hDlg, IDC_TYPE, 
								CB_ADDSTRING, 0, (LPARAM)sz);
		if (u != CB_ERR)
		{
			SendDlgItemMessage (hDlg, IDC_TYPE, CB_SETITEMDATA, 
								u, (LPARAM)kPGPKeyServerClass_Entrust);
			if (uType == kPGPKeyServerClass_Entrust)
				uSelect = u;
		}

		SendDlgItemMessage (hDlg, IDC_TYPE, CB_SETCURSEL, uSelect, 0);

		// initialize root cert edit box
		pcaps->keyCert = kInvalidPGPKeyRef;
		pcaps->sigCert = kInvalidPGPSigRef;

		PGPclGetRootCACertPrefs (ContextRef, PrefRefClient, keysetMain,
							&pcaps->keyCert, &pcaps->sigCert);

		PostMessage (hDlg, WM_APP, 0, 0);

		return TRUE;
	}

	case WM_APP :
		pcaps = (PCAPREFSSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		if (PGPSigRefIsValid (pcaps->sigCert))
		{
			CHAR	sz[256];
			PGPSize	size;

			PGPGetSigPropertyBuffer (pcaps->sigCert, 
					kPGPSigPropX509LongName, sizeof(sz), sz, &size);
			SetDlgItemText (hDlg, IDC_ROOTCERT, sz);
			EnableWindow (GetDlgItem (hDlg, IDC_CLEARCERT), TRUE);
		}
		else
		{
			SetDlgItemText (hDlg, IDC_ROOTCERT, "");
			EnableWindow (GetDlgItem (hDlg, IDC_CLEARCERT), FALSE);
		}
		break;

	case WM_HELP : 
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
			HELP_WM_HELP, (DWORD) (LPSTR) aCAIds); 
		break; 

	case WM_CONTEXTMENU : 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
			(DWORD) (LPVOID) aCAIds); 
		break; 

	case WM_COMMAND :
		pcaps = (PCAPREFSSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

		switch (LOWORD (wParam)) {
		case IDC_URL :
			if (HIWORD (wParam) == EN_CHANGE)
				bUrlModified = TRUE;
			break;

		case IDC_REVURL :
			if (HIWORD (wParam) == EN_CHANGE)
				bRevUrlModified = TRUE;
			break;

		case IDC_CLEARCERT :
			if (PGPKeyRefIsValid (pcaps->keyCert))
			{
				pcaps->keyCert = kInvalidPGPKeyRef;
				pcaps->sigCert = kInvalidPGPSigRef;
				PostMessage (hDlg, WM_APP, 0, 0);
			}
			break;

		case IDC_SETCERT :
		{
			PGPKeyRef	key;
			PGPSigRef	sig;
			CHAR		sz[128];

			LoadString (g_hInst, IDS_ROOTCAPROMPT, sz, sizeof(sz));

			if (IsntPGPError (PGPclSelectX509Cert (ContextRef, 
							hDlg, sz, keysetMain, 
							PGPCL_NOSPLITKEYS|PGPCL_CACERTSONLY, 
							&key, &sig)))
			{
				pcaps->keyCert = key;
				pcaps->sigCert = sig;
				PostMessage (hDlg, WM_APP, 0, 0);
			}
			break;
		}
		}
		break;

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {
		case PSN_SETACTIVE :
			bUrlModified = FALSE;
			bRevUrlModified = FALSE;
			break;

		case PSN_HELP :
			WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
				IDH_PGPCLPREF_CADIALOG); 
			break;

		case PSN_APPLY :			
		{
			CHAR			szNew[256];
			CHAR			szOld[256];
			UINT			u;
			PGPInt32		iType;
			PGPError		err;

			pcaps = (PCAPREFSSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

			// get old and new URL strings and compare
			PGPGetPrefStringBuffer (PrefRefClient, 
							kPGPPrefCAServerURL, 
							sizeof(szOld), szOld);
			GetDlgItemText (hDlg, IDC_URL, szNew, sizeof(szNew));
			if (lstrcmpi (szNew, szOld))
			{
				GetDlgItemText (hDlg, IDC_URL, szNew, sizeof(szNew));
				PGPclErrorBox (hDlg, PGPSetPrefString (PrefRefClient,
							kPGPPrefCAServerURL, szNew));

				PGPclErrorBox (hDlg, PGPSetPrefNumber (PrefRefClient,
							kPGPPrefCAServerAuthKeyAlg, 
							kPGPPublicKeyAlgorithm_Invalid));
			}

			// get old and new RevURL strings and compare
			PGPGetPrefStringBuffer (PrefRefClient, 
							kPGPPrefCARevocationServerURL, 
							sizeof(szOld), szOld);
			GetDlgItemText (hDlg, IDC_REVURL, szNew, sizeof(szNew));
			if (lstrcmpi (szNew, szOld))
			{
				GetDlgItemText (hDlg, IDC_REVURL, szNew, sizeof(szNew));
				PGPclErrorBox (hDlg, PGPSetPrefString (PrefRefClient,
							kPGPPrefCARevocationServerURL, szNew));

				PGPclErrorBox (hDlg, PGPSetPrefNumber (PrefRefClient,
							kPGPPrefCARevocationServerAuthKeyAlg, 
							kPGPPublicKeyAlgorithm_Invalid));
			}

			// get and save CA type
			u = SendDlgItemMessage (hDlg, IDC_TYPE, CB_GETCURSEL, 0, 0);
			iType = SendDlgItemMessage (hDlg, IDC_TYPE, CB_GETITEMDATA, 
										u, 0);
			PGPclErrorBox (hDlg, PGPSetPrefNumber (PrefRefClient,
							kPGPPrefCAType, iType));

			// get and save root cert
			err = sSetRootCACertPrefs (ContextRef, PrefRefClient,
							pcaps->keyCert, pcaps->sigCert);

			returnErr = kPGPError_NoErr;
            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
		}

		case PSN_KILLACTIVE :
		{
			CHAR			szURL[256];
			UINT			u;

			// if the user modified the URL, look for trailing slash
			if (bUrlModified)
			{
				u = GetDlgItemText (hDlg, IDC_URL, szURL, sizeof(szURL));
				if (u > 0)
				{
					if ((szURL[u-1] == '\\') || (szURL[u-1] == '/'))
					{
						if (PGPclMessageBox (hDlg, 
								IDS_CAPTION, IDS_TRAILINGCASLASH, 
								MB_ICONINFORMATION|MB_OKCANCEL) == IDCANCEL)
						{
							SendDlgItemMessage (
									hDlg, IDC_URL, EM_SETSEL, 0, -1);
							SetFocus (GetDlgItem (hDlg, IDC_URL));
							SetWindowLong (hDlg, DWL_MSGRESULT, TRUE);
							return TRUE;
						}
					}
				}
			}

			// if the user modified the Rev URL, look for trailing slash
			if (bRevUrlModified)
			{
				u = GetDlgItemText (hDlg, IDC_REVURL, szURL, sizeof(szURL));
				if (u > 0)
				{
					if ((szURL[u-1] == '\\') || (szURL[u-1] == '/'))
					{
						if (PGPclMessageBox (hDlg, 
								IDS_CAPTION, IDS_TRAILINGCASLASH, 
								MB_ICONINFORMATION|MB_OKCANCEL) == IDCANCEL)
						{
							SendDlgItemMessage (
									hDlg, IDC_REVURL, EM_SETSEL, 0, -1);
							SetFocus (GetDlgItem (hDlg, IDC_REVURL));
							SetWindowLong (hDlg, DWL_MSGRESULT, TRUE);
							return TRUE;
						}
					}
				}
			}

			returnErr = kPGPError_NoErr;
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
            return TRUE;
		}

		case PSN_RESET :
			returnErr = kPGPError_UserAbort;
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;
		}
	}

	return FALSE;
}


//	______________________________________
//
//	Check for previously running instances

static HANDLE 
sShowExistingPropertySheet (
		LPSTR lpszSemName, 
		LPSTR lpszWindowTitle) 
{
    HANDLE	hSem;
    HWND	hWndMe;

    // Create or open a named semaphore. 
    hSem = CreateSemaphore (NULL, 0, 1, lpszSemName);

    // Close handle and return NULL if existing semaphore was opened.
    if ((hSem != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS)) {
        CloseHandle(hSem);
        hWndMe = FindWindow ((LPCTSTR)32770L, lpszWindowTitle);
        if (hWndMe) {
			SetForegroundWindow (hWndMe);
		}
        return NULL;
    }

    // If new semaphore was created, return FALSE.
    return hSem;
}
 
//	____________________________
//
//  Create property sheet dialog

PGPError PGPclExport 
PGPclPreferences (
		PGPContextRef	Context, 
		HWND			hWnd, 
		INT				startsheet,
		PGPKeySetRef	keyset) 
{
    PROPSHEETPAGE	psp[7];
    PROPSHEETHEADER psh;
	UINT			uNumPages = 0;
	CHAR			szTitle[128];
	HANDLE			hSemaphore;
	PGPError		err;
	CAPREFSSTRUCT	caps;
	HWND			hwndTrayApp;

	//assume no KeyServer page
	uNumPages = (sizeof(psp) / sizeof(PROPSHEETPAGE)); 

	// get title of property sheet
	LoadString (g_hInst, IDS_PROPCAPTION, szTitle, sizeof(szTitle));

	// if preferences dialog already exists, move it to the foreground
	hSemaphore = sShowExistingPropertySheet (PREFSEMAPHORENAME, szTitle);
	if (!hSemaphore) return kPGPError_Win32_AlreadyOpen;

	hWndParent = hWnd;
	keysetMain = keyset;

	if (Context != NULL)
		ContextRef = Context;
	else
		PGPNewContext (kPGPsdkAPIVersion, &ContextRef);

	PGPsdkLoadDefaultPrefs (ContextRef);

	psp[PGPCL_GENERALPREFS].dwSize = sizeof(PROPSHEETPAGE);
	psp[PGPCL_GENERALPREFS].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[PGPCL_GENERALPREFS].hInstance = g_hInst;
	psp[PGPCL_GENERALPREFS].pszTemplate = MAKEINTRESOURCE(IDD_PREF_GENERAL);
	psp[PGPCL_GENERALPREFS].pszIcon = NULL;
	psp[PGPCL_GENERALPREFS].pfnDlgProc = sEncryptPropDlgProc;
	psp[PGPCL_GENERALPREFS].pszTitle = (LPSTR)IDS_PREF_GENERAL;
	psp[PGPCL_GENERALPREFS].lParam = 0;
	psp[PGPCL_GENERALPREFS].pfnCallback = NULL;

	psp[PGPCL_KEYRINGPREFS].dwSize = sizeof(PROPSHEETPAGE);
	psp[PGPCL_KEYRINGPREFS].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[PGPCL_KEYRINGPREFS].hInstance = g_hInst;
	psp[PGPCL_KEYRINGPREFS].pszTemplate = MAKEINTRESOURCE(IDD_PREF_FILES);
	psp[PGPCL_KEYRINGPREFS].pszIcon = NULL;
	psp[PGPCL_KEYRINGPREFS].pfnDlgProc = sFilePropDlgProc;
	psp[PGPCL_KEYRINGPREFS].pszTitle = (LPSTR)IDS_PREF_FILES;
	psp[PGPCL_KEYRINGPREFS].lParam = 0;
	psp[PGPCL_KEYRINGPREFS].pfnCallback = NULL;

	psp[PGPCL_EMAILPREFS].dwSize = sizeof(PROPSHEETPAGE);
	psp[PGPCL_EMAILPREFS].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[PGPCL_EMAILPREFS].hInstance = g_hInst;
	psp[PGPCL_EMAILPREFS].pszTemplate = MAKEINTRESOURCE(IDD_PREF_EMAIL);
	psp[PGPCL_EMAILPREFS].pszIcon = NULL;
	psp[PGPCL_EMAILPREFS].pfnDlgProc = sEmailPropDlgProc;
	psp[PGPCL_EMAILPREFS].pszTitle = (LPSTR)IDS_PREF_EMAIL;
	psp[PGPCL_EMAILPREFS].lParam = 0;
	psp[PGPCL_EMAILPREFS].pfnCallback = NULL;

	psp[PGPCL_HOTKEYPREFS].dwSize = sizeof(PROPSHEETPAGE);
	psp[PGPCL_HOTKEYPREFS].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[PGPCL_HOTKEYPREFS].hInstance = g_hInst;
	psp[PGPCL_HOTKEYPREFS].pszTemplate = MAKEINTRESOURCE(IDD_PREF_HOTKEYS);
	psp[PGPCL_HOTKEYPREFS].pszIcon = NULL;
	psp[PGPCL_HOTKEYPREFS].pfnDlgProc = sHotkeyPropDlgProc;
	psp[PGPCL_HOTKEYPREFS].pszTitle = (LPSTR)IDS_PREF_HOTKEYS;
	psp[PGPCL_HOTKEYPREFS].lParam = 0;
	psp[PGPCL_HOTKEYPREFS].pfnCallback = NULL;

	psp[PGPCL_KEYSERVERPREFS].dwSize = sizeof(PROPSHEETPAGE);
	psp[PGPCL_KEYSERVERPREFS].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[PGPCL_KEYSERVERPREFS].hInstance = g_hInst;
	psp[PGPCL_KEYSERVERPREFS].pszTemplate = MAKEINTRESOURCE(IDD_PREF_SERVER);
	psp[PGPCL_KEYSERVERPREFS].pszIcon = NULL;
	psp[PGPCL_KEYSERVERPREFS].pfnDlgProc = sKeyserverPropDlgProc;
	psp[PGPCL_KEYSERVERPREFS].pszTitle = (LPSTR)IDS_PREF_SERVER;
	psp[PGPCL_KEYSERVERPREFS].lParam = 0;
	psp[PGPCL_KEYSERVERPREFS].pfnCallback = NULL;

	psp[PGPCL_CAPREFS].dwSize = sizeof(PROPSHEETPAGE);
	psp[PGPCL_CAPREFS].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[PGPCL_CAPREFS].hInstance = g_hInst;
	psp[PGPCL_CAPREFS].pszTemplate = MAKEINTRESOURCE(IDD_PREF_CA);
	psp[PGPCL_CAPREFS].pszIcon = NULL;
	psp[PGPCL_CAPREFS].pfnDlgProc = sCAPropDlgProc;
	psp[PGPCL_CAPREFS].pszTitle = (LPSTR)IDS_PREF_CA;
	psp[PGPCL_CAPREFS].lParam = (LPARAM)&caps;
	psp[PGPCL_CAPREFS].pfnCallback = NULL;
	
	psp[PGPCL_ADVANCEDPREFS].dwSize = sizeof(PROPSHEETPAGE);
	psp[PGPCL_ADVANCEDPREFS].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[PGPCL_ADVANCEDPREFS].hInstance = g_hInst;
	psp[PGPCL_ADVANCEDPREFS].pszTemplate = MAKEINTRESOURCE(IDD_PREF_ADVANCED);
	psp[PGPCL_ADVANCEDPREFS].pszIcon = NULL;
	psp[PGPCL_ADVANCEDPREFS].pfnDlgProc = sAdvancedPropDlgProc;
	psp[PGPCL_ADVANCEDPREFS].pszTitle = (LPSTR)IDS_PREF_ADVANCED;
	psp[PGPCL_ADVANCEDPREFS].lParam = 0;
	psp[PGPCL_ADVANCEDPREFS].pfnCallback = NULL;
	
	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
	psh.hwndParent = hWndParent;
	psh.hInstance = g_hInst;
	psh.pszIcon = NULL;
	psh.pszCaption = (LPSTR) szTitle;
	psh.nPages = uNumPages;
	psh.nStartPage = startsheet;
	psh.ppsp = (LPCPROPSHEETPAGE) &psp;
	psh.pfnCallback = NULL;

	bNeedsCentering = TRUE;
	bReloadKeyring = FALSE;
	bKeyserverPrefsWritten = FALSE;

	// disable hotkeys because they can interfere with the hotkey panel
	hwndTrayApp = FindWindow (PGPTRAY_WINDOWNAME, PGPTRAY_WINDOWNAME);
	if (hwndTrayApp)
	{
		SendMessage (hwndTrayApp, PGPTRAY_M_DISABLEHOTKEYS, 0, 0);
	}

	err = kPGPError_NoErr;
#if PGP_BUSINESS_SECURITY
	err = PGPclOpenAdminPrefs (
			PGPGetContextMemoryMgr (ContextRef),
			&PrefRefAdmin, 
			PGPclIsAdminInstall());
#endif

	if (IsntPGPError (err)) {
		PGPclOpenClientPrefs (PGPGetContextMemoryMgr (ContextRef),
									&PrefRefClient);
		PropertySheet(&psh);

		PGPsdkSavePrefs (ContextRef);

		PGPclCloseClientPrefs (PrefRefClient, TRUE);
	}
#if PGP_BUSINESS_SECURITY
	PGPclCloseAdminPrefs (PrefRefAdmin, FALSE);
#endif

	if (bReloadKeyring) {
		PGPclNotifyKeyringChanges (0);
	}

	if (bKeyserverPrefsWritten) {
		PGPclNotifyKeyserverPrefsChanges (0);	
	}

	PGPclNotifyPrefsChanges (0);

	if (PGPKeySetRefIsValid (keysetMain) && !PGPKeySetRefIsValid (keyset))
		PGPFreeKeySet (keysetMain);

	if (Context == NULL)
		PGPFreeContext(ContextRef);

	CloseHandle (hSemaphore);

    return returnErr;
}
 


