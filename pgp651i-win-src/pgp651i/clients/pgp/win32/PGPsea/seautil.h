/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: seautil.h,v 1.5 1999/03/24 20:54:35 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_SEAUTIL_h	/* [ */
#define Included_SEAUTIL_h

void SEADoInstallerCommand(void);

void SEACreateExecDelete(FILE *fin,GETPUTINFO *gpi);

PGPError SEAGetTempPath(GETPUTINFO *gpi);

#if !IS_SDA
void Deflate_Decompress(void *gpi);
#endif

UINT FileListFromFile(FILELIST **filelist,char *filename,BOOL *UserCancel);

#endif /* ] Included_SEAUTIL_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
