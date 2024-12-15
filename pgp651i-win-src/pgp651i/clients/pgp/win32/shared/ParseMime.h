/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: ParseMime.h,v 1.2 1999/03/10 02:46:37 heller Exp $
____________________________________________________________________________*/
#ifndef Included_ParseMime_h	/* [ */
#define Included_ParseMime_h

#include "pgpConfig.h"
#include "pgpErrors.h"

typedef enum _MimeContentType
{
	ContentType_TextPlain,
	ContentType_TextHTML,
	ContentType_MultipartMixed,
	ContentType_MultipartAlternative,
	ContentType_Other
} MimeContentType;

typedef struct MimePart	MimePart;

struct MimePart
{
	MimePart *	nextPart;
	MimePart *	previousPart;

	char *				szHeader;
	char *				szBody;
	char *				szFooter;
	UINT				nHeaderLength;
	UINT				nBodyLength;
	UINT				nFooterLength;
	MimeContentType		nContentType;
};


PGPError ParseMime(char *szInput, MimePart **ppMimeList);


#endif /* ] Included_ParseMime_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
