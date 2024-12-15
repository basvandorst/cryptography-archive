/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPsda.h - Header file for creating SDA's
	

	$Id: sea.h,v 1.8 1999/03/24 20:54:35 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_PGPsda_h	/* [ */
#define Included_PGPsda_h

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include "resource.h"
#include "PGPsea.h"
#if IS_SDA
#include "DeflateWrapper.h" // Compression only in SDAs
#endif

typedef struct _FILELIST
{
	char name[MAX_PATH];
	BOOL IsDirectory;
	struct _FILELIST *next;
} FILELIST;

BOOL FreeFileList(FILELIST *FileList);
BOOL AddToFileList(FILELIST **filelist,char *filename,BOOL *UserCancel);

#endif /* ] Included_PGPsda_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
