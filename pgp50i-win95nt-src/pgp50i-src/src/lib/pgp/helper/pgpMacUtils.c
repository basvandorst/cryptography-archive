/*
* pgpMacUtils.c -- Various Macintosh utilities
*
* Copyright (C) 1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpMacUtils.c,v 1.2.2.1 1997/06/07 09:50:09 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpErr.h"
#include "pgpMacUtils.h"

typedef struct MacErrorEntry_
{
	OSErr	 	macError;
	PGPError	pgpError;
	} MacErrorEntry;

static MacErrorEntry	macErrorTable[] = {
	{ opWrErr,			PGPERR_FILE_WRITELOCKED },
	{ permErr,			PGPERR_FILE_PERMISSIONS },
	{ afpAccessDenied,	PGPERR_FILE_PERMISSIONS },
{ 0, 0 }
};

	PGPError
pgpErrorFromMacError(
	OSErr	 	macError,
	PGPError	defaultError)
{
	int		i;
	
	if (macError == noErr)
		return PGPERR_OK;
	for (i = 0; macErrorTable[i].macError != 0; i++)
		if (macErrorTable[i].macError == macError)
			return macErrorTable[i].pgpError;
	return defaultError;
}

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
