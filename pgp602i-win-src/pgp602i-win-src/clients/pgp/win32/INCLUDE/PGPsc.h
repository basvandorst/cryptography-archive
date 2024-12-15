/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPsc.h - include file for PGPsc DLL
	

	$Id: PGPsc.h,v 1.36.2.3 1998/10/09 16:05:01 wjb Exp $
____________________________________________________________________________*/
#ifndef _PGPSCH
#define _PGPSCH

#include <shellapi.h>

typedef struct _FILELIST
{
	char name[MAX_PATH];
	BOOL IsDirectory;
	struct _FILELIST *next;
} FILELIST;

typedef struct _OUTBUFFLIST {
	char *pBuff;
	DWORD dwBuffSize;
	BOOL FYEO;
	struct _OUTBUFFLIST *next;
} OUTBUFFLIST;

#ifdef __cplusplus
extern "C" {
#endif

// Library initialization routines
BOOL InitPGPsc(HWND hwnd,void **PGPsc,void **PGPtls);
BOOL UninitPGPsc(HWND hwnd,void *PGPsc,void *PGPtls);

BOOL CheckForPurge(UINT uMsg,WPARAM wParam);

// Clipboard routines
BOOL EncryptClipboard(HWND hwnd,void *PGPsc,void *PGPtls,
													BOOL Encrypt,BOOL Sign);
BOOL DecryptClipboard(HWND hwnd,void *PGPsc,void *PGPtls);
BOOL AddKeyClipboard(HWND hwnd,void *PGPsc,void *PGPtls);
BOOL ClipboardWipe(HWND hwnd,void *PGPsc);

// Filelist routines
BOOL EncryptFileList(HWND hwnd,void *PGPsc,void *PGPtls,
					 FILELIST *ListHead,BOOL bEncrypt,BOOL bSign);
BOOL DecryptFileList(HWND hwnd,void *PGPsc,void *PGPtls,FILELIST *ListHead);
BOOL AddKeyFileList(HWND hwnd,void *PGPsc,void *PGPtls,FILELIST *ListHead);
BOOL WipeFileList(HWND hwnd,void *PGPsc,FILELIST *ListHead);

// Filelist utility functions (Note that UserCancel can be NULL)
BOOL AddToFileList(FILELIST **filelist,char *filename,BOOL *UserCancel);
BOOL FreeFileList(FILELIST *FileList);

FILELIST *HDropToFileList(HDROP hDrop);
FILELIST *CmdLineToFileList(char *szCmdLine);

// Call-throughs to PGPcl
UINT PGPscQueryCacheSecsRemaining (void);
BOOL PGPscPreferences(HWND hwnd,void *PGPsc,void *PGPtls);
BOOL PGPscAbout (HWND hwnd,void *PGPsc);

// Returns filename from path
char *JustFile(char *filename);

// Grabs preferences from pref file
BOOL GetValidityDrawPrefs(void *PGPsc,BOOL *pbInputBool);
BOOL GetMarginalInvalidPref(void *PGPsc,BOOL *pbInputBool);

// Views the clipboard
void LaunchInternalViewer(HWND hwnd);

// Use secure viewer and/or anti-tempest font
UINT TempestViewer(void *PGPsc,HWND hwnd,char *pInput,DWORD dwInSize);

// Launches Keys/Tools
BOOL DoLaunchKeys(HWND hwnd);
BOOL DoLaunchTools(HWND hwnd);
BOOL DoLaunchDisk(HWND hwnd);

// Wipe free space on disk
void FreeSpaceWipeWizard(HWND hwnd, void* PGPsc);

// Detects if window exists
BOOL WindowExists (LPSTR lpszClassName, LPSTR lpszWindowTitle);

// Sends signature info to PGPlog
BOOL SendPGPlogMsg(HWND hwnd,char* szMsg);

// Returns path for PGPkeys
void PGPpath(char *szPath);

// Copy/Paste into current window
BOOL DoCopy(HWND hwnd,void *PGPsc,BOOL bUseCurrent,HWND *hwndFocus);
void DoPaste(BOOL bUseCurrent,HWND hwndFocus);
void DoFocus(BOOL bUseCurrent,HWND hwndFocus);

// Auto-update keyring routines
void StartUpdateTimer(HWND hwnd, UINT *pnLaunchKeysTimer);
VOID CALLBACK UpdateTimerProc(HWND hwnd, 
							  UINT uMsg, 
							  UINT idEvent, 
							  DWORD dwTime);

OUTBUFFLIST *MakeOutBuffItem(OUTBUFFLIST **obl);

BOOL ConcatOutBuffList(void *PGPsc,
					   OUTBUFFLIST *obl,
					   char **pBuff,
					   DWORD *dwBuffSize,
					   BOOL *FYEO);

#ifdef WIN32
#define DDElogServer "PGPlog"
#else
#define DDElogServer "PGPlog16"
#endif
#define DDElogTopic "Operation_Results"

#ifdef __cplusplus
}
#endif

#endif // PGPSCH
