/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: sdautil.h,v 1.4 1999/03/10 03:02:32 heller Exp $
____________________________________________________________________________*/
#ifndef Included_SDAUTIL_h	/* [ */
#define Included_SDAUTIL_h

#include "pgpSHA.h"
#include "pgpCAST5.h"

BOOL  SaveOutputFile(HWND hwnd, 
					 char *Title,
					 char *InputFile, 
					 char *OutputFile);

PGPError SDAGetPassphrase(SDAWORK *SDAWork,SDAHEADER *SDAHeader,PGPUInt32 **ExpandedKey);

void
DecryptBlock512(
	SDAHEADER				*SDAHeader,
	PGPUInt32				*ExpandedKey,
	PGPUInt32				blockNumber,
	const PGPUInt32 *		src,
	PGPUInt32 *				dest);

PGPError
SDAPassphraseDialog(HWND hwnd,char *szPrompt,char **ppszPassPhrase);

void GetPrefixPath(GETPUTINFO *gpi);

#endif /* ] Included_SDAUTIL_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
