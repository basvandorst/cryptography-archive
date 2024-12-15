/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	DecodeStub.h - Header file for decoding SDA/SEA's
	

	$Id: DecodeStub.h,v 1.4 1999/03/24 20:54:35 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_PGPsda_h	/* [ */
#define Included_PGPsda_h

#include <windows.h>
#include <windowsx.h>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <io.h>
#include <errno.h>
#include <commctrl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "resource.h"
#include "pgpErrors.h"
#include "ProgressDialog.h"

#if IS_SDA
#include "PGPsda.h"
#else
#include "PGPsea.h"
#endif

typedef struct
{
	char filename[MAX_PATH+1];
	HWND hwndWorking;
	SDAHEADER *SDAHeader;
} SDAWORK;

typedef struct _FILELIST
{
	char name[MAX_PATH];
	BOOL IsDirectory;
	struct _FILELIST *next;
} FILELIST;

// This is the structure that is passed into
// our getc_buffer and putc_buffer routines from
// LZSS. We need enough info here to read and write
// the cancel the file operations.
typedef struct
{
	FILE			*fin;
	FILE			*fout;
	SDAHEADER		*SDAHeader;
	PGPUInt32		*ExpandedKey;
	DWORD			blockindex;
	DWORD			from_pointer;
	DWORD			from_max;
	DWORD			to_pointer;
	DWORD			to_max;
	unsigned char	*outbuffer;
	unsigned char	*inbuffer;
	HWND			hwnd;
	BOOL			CancelOperation;
	BOOL			bHaveFileName;
	DWORD			NumFiles;
	SDAWORK			*SDAWork;
	BOOL			bFeedFilename;
	char			FileName[MAX_PATH+1];
	DWORD			FileSize;
	DWORD			FileIndex;
	DWORD			FeedIndex;
	char			szPrefixPath[MAX_PATH+1];
	FILELIST		*fl;
	HPRGDLG			hPrgDlg;
} GETPUTINFO;

int getc_buffer(void *pUserValue);
int putc_buffer(int c,void *pUserValue);

#define kBlockSize			512
#define	kBlockSizeInUInt32	(kBlockSize / sizeof( PGPUInt32 ))

extern HINSTANCE g_hinst;

#if IS_SDA
#include "sdautil.h"
#include "DeflateWrapper.h"
#else
#include "seautil.h"
#endif

#endif /* ] Included_PGPsda_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
