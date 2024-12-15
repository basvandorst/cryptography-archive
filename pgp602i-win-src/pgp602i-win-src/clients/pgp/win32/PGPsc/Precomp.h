/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Precomp.h,v 1.38.8.1 1998/11/12 03:13:27 heller Exp $
____________________________________________________________________________*/
#ifndef Included_PRECOMP_h	/* [ */
#define Included_PRECOMP_h

/*++

Module Name:

    precomp.h

Abstract:

    This file includes all the headers required to build the dll
    to ease the process of building a precompiled header.

Author:


Revision History:


--*/

#define WIN32_LEAN_AND_MEAN
#define PGPExport __declspec( dllexport )

// System headers
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h> 
#include <string.h>
#include <ShellAPI.h>
#include <Direct.h>
#include <CommDlg.h>
#include <OLE2.h>
#include <commctrl.h>
#include <process.h>

#include "resource.h"                

#define UNFINISHED_CODE_ALLOWED 1

// PGPlib headers

#include "..\..\shared\pgpVersionHeader.h"
#include "..\..\shared\pgpBuildFlags.h"
#include "pgpKeys.h"
#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpEncode.h"
#include "pgpUtilities.h"
#include "pgpSDKPrefs.h"
#include "pflPrefTypes.h"
#include "pgpPubTypes.h"

#include "pgpDiskWiper.h"


#ifndef WIN32

#define BST_CHECKED			1
#define ULONG unsigned long
#define USHORT unsigned short    
#define LPTSTR  char* 

#define MAX_PATH 256

#define CHAR char
#define INT int  
#define APIENTRY WINAPI   
#define BST_CHECKED 1
#define BST_UNCHECKED 0                 
#define UCHAR unsigned char
#define LPCTSTR char *

#endif
 
#include "..\include\PGPcl.h"
#include "..\..\shared\pgpClientPrefs.h"
#include "..\..\shared\pgpAdminPrefs.h"

// PGPtray headers

#include "ClipBrd.h"
#include "ClVwClip.h"
#include "..\include\PGPsc.h"
#include "working.h"
#include "recpass.h"
#include "events.h"
#include "saveopen.h"
#include "resource.h"
#include "..\shared\SigEvent.h"
#include "pgpFileSpec.h"
#include "pgpShareFile.h"

#define MAX_BUFFER_SIZE 500000

#define OPENKEYRINGFLAGS 0

HINSTANCE g_hinst;

#define FS_NOFS  0
#define FS_FAT   1 // Could be FAT12 or FAT16...
#define FS_FAT12 2 // If clusters <= 4079
#define FS_FAT16 3 // If clusters >= 4079
#define FS_FAT32 4 // New for OSR2

#define FS_NTFS 5 // WindowsNT

#define WFE_NOERROR             0
#define WFE_FSNOTSUPPORTED      1
#define WFE_DRIVENOTSUPPORTED   2
#define WFE_FATFAILED			3
#define WFE_COULDNOTLOCK        4
#define WFE_NODRIVESELECTED     5
#define WFE_USERCANCEL          6
#define WFE_DISKMODIFIED        7
#define WFE_COULDNOTWRITE       8
#define WFE_COULDNOTREAD        9
#define WFE_OUTOFMEMORY        10
#define WFE_CLUSTERERROR       11
#define WFE_NTNOTSUPPORTED     12
#define WFE_NTFSGREATER4K      13
#define WFE_INVALPARAM         14 // temp error
#define WFE_ALREADYCOMMIT      15 // temp error
BOOL bGlobalCancel;

typedef struct _DIRSTACK
{
	DWORD dwStartCluster;
	struct _DIRSTACK *next;
} DIRSTACK;

typedef struct
{
	char *name;
	ULONGLONG Vcn;
	ULONGLONG Orig;
} SLACKMOVE;

#define SLACKBUFFER 2000

typedef struct _VOLINFO
{
	PGPBoolean WinNT;
	FILELIST *FileList;
	SLACKMOVE *SlackMove;
	DWORD SlackMoveIndex;
	DWORD SlackMoveCount;
	ULONGLONG SlackBuffer[SLACKBUFFER+1];
	ULONGLONG PatternBuffer;
	DWORD SlackBufferIndex;
	HWND hwnd;
	HANDLE hVWin32;
	HANDLE VolHandle;
	DWORD vol0;
	DWORD vol1;
	DWORD dwFS;
	DWORD dwStartSector;
	unsigned short sectors_per_clus;
	unsigned short sector_size;
	unsigned short sectors_per_fat;
	int fat_count;
	DWORD dwReservedSectors;
	DWORD sectors_total;
	DWORD dwClusterSize;
	DWORD no_of_clusters;
	DWORD FAT32_DirCluster;
	DWORD FAT_DirSector;
	DWORD dwClustersWritten;
	unsigned short FAT_DirSectorCount;
	unsigned char *pFatBuf;
	unsigned char *pClusterBuf;
	float fTotalCapacity;
	DIRSTACK *ds;
	BOOL bCountClusters;
	DWORD dwClusterCount;
	DWORD pattern_buffer[256];

} VOLINFO;

#include "WFnt.h"
#include "WFfat.h"
#include "WFwiz.h"

BOOL DisplayDiskStats(VOLINFO *vi);
void StatusMessage(char *msg, BOOL bold);
void WipeProgress(int pos);
void PassProgress(int pass, int total);
DWORD WipeFree(HWND hwnd,VOLINFO *vi,char *szRoot,
			   DWORD* pattern, BOOL StartWipe);



#endif /* ] Included_PRECOMP_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
