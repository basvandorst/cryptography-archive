/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: ProgressDialog.h,v 1.4 1999/03/10 03:00:51 heller Exp $
____________________________________________________________________________*/
#ifndef Included_PROGRESSDIALOG_h	/* [ */
#define Included_PROGRESSDIALOG_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _PRGDLG
{
	HWND			hwnd;

	HWND			hwndParent;
	PGPError		(*UserFunction)(struct _PRGDLG *,void *);
	void			*pUserValue;
	DWORD			dwBar;
	char			*szTitle;
	char			*szFile;
	char			*szInfo;
	DWORD			dwAVIClip;

	DWORD			dwNewValue;
	DWORD			dwOldValue;
	HANDLE			hSemaphore;
	DWORD			dwThreadID;
	PGPError		err;
	BOOL			bCancel;
	BOOL			bAVIPlaying;
} PRGDLG, *HPRGDLG;

typedef PGPError (*USERFUNCTION)(HPRGDLG,void *);

PGPError SCProgressDialog(HWND hwndParent,USERFUNCTION UserFunction,
						  void *pUserValue,DWORD dwBar,char *szTitle,
						  char *szFile,char *szInfo,DWORD dwAVIClip);

PGPError SCSetProgressBar(HPRGDLG hPrgDlg,DWORD dwBarValue,BOOL bForce);

PGPError SCSetProgressTitle(HPRGDLG hPrgDlg,char *szTitle);

PGPError SCSetProgressFile(HPRGDLG hPrgDlg,char *szFile);

PGPError SCSetProgressInfo(HPRGDLG hPrgDlg,char *szInfo);

PGPError SCSetProgressAVI(HPRGDLG hPrgDlg,DWORD dwAVIClip);

HWND SCGetProgressHWND(HPRGDLG hPrgDlg);

BOOL SCGetProgressCancel(HPRGDLG hPrgDlg);

void *SCGetProgressUserValue(HPRGDLG hPrgDlg);

void SCSetProgressNewFilename(HPRGDLG hPrgDlg,char *szInfoFormat,
							  char *infile,BOOL bResetBar);

#ifdef __cplusplus
}
#endif

#endif /* ] Included_PROGRESSDIALOG_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
