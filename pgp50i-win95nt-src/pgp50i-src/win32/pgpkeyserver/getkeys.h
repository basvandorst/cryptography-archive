/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#ifndef __GETKEYS_H

#define __GETKEYS_H

UINT PGPExport PGPkeyserverGetAndAddKeys(HWND hwnd,
										 char *UserId,
										 PGPKeySet *pAddFromKeyRing,
										 PGPKeySet *pKeyRings,
										 unsigned long *NumKeysFound);
UINT PGPExport PGPkeyserverGetKeys(HWND hwnd, char *UserId, char **KeyBuffer);
UINT PGPExport PGPkeyserverPutKeys(HWND hwnd, char *UserIds,
									PGPKeySet *pKeyRings);
UINT PGPExport PGPkeyserverPutKey(HWND hwnd, char *UserId,
									PGPKeySet *pKeyRings);
UINT PGPExport PGPkeyserverAddKeyBuffer(HWND hwnd,
										char *Buffer,
										PGPKeySet *pKeyRings,
										unsigned long *NumKeysFound);
KSERR MassageUserId(char **UserId, char *Start, char *End);
LRESULT WINAPI GetEmailDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
								LPARAM lParam);
void PutKeyInfoThread(void *pArgs);

typedef struct _wdplt
{
	HWND hWorkingDlg;
	HWND hParent;
	BOOL CancelPending;
	char *UserId;
	char **KeyBuffer;
	UINT ReturnCode;
	BOOL IsGet;
	BOOL HideMainWindowAtEnd;
}WDPLT;

void GetKeyInfoThread(void *pArgs);

#endif //__GETKEYS_H
