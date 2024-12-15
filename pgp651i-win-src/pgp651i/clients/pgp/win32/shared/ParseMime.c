/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: ParseMime.c,v 1.3 1999/03/10 02:46:35 heller Exp $
____________________________________________________________________________*/
// System Headers
#include <windows.h>

// Shared Headers 
#include "ParseMime.h"

static char * GetNextLine(char *szStart,
						  char *szOutput,
						  size_t nOutputSize);

static void MatchContentType(char *szLine, MimeContentType *pnContentType);


PGPError ParseMime(char *szInput, MimePart **ppMimeList)
{
	char szLine[256];
	char szBoundary[256];
	char *szLastLinePtr;
	char *szLinePtr;
	char *szBoundaryStart;
	MimePart *pMimeList;
	MimePart *pHead;
	BOOL bFoundBoundary = FALSE;

	if ((szInput == NULL) || (ppMimeList == NULL))
		return kPGPError_BadParams;

	*ppMimeList = NULL;
	szLinePtr = szInput;
	
	szLinePtr = GetNextLine(szLinePtr, szLine, sizeof(szLine));

	pMimeList = (MimePart *) calloc(sizeof(MimePart), 1);

	pHead = pMimeList;
	pMimeList->szHeader = szInput;
	strcpy(szBoundary, "\n\n\n\n\n");

	if ((szLinePtr != NULL) && strstr(szLine, "Mime-Version:"))
	{
		szLinePtr = GetNextLine(szLinePtr, szLine, sizeof(szLine));
		
		if (strstr(szLine, "Content-Type:"))
			MatchContentType(szLine, &(pMimeList->nContentType));
		else
			pMimeList->nContentType = ContentType_TextPlain;

		while (szLinePtr != NULL)
		{
			if ((pMimeList->szBody == NULL) && (strlen(szLine) < 3))
			{
				pMimeList->nHeaderLength = szLinePtr - pMimeList->szHeader;
				pMimeList->szBody = szLinePtr;
			}
	
			if (!bFoundBoundary)
			{
				if ((szBoundaryStart = strstr(szLine, "boundary=")) != NULL)
				{
					bFoundBoundary = TRUE;
					
					szBoundaryStart += strlen("boundary=");
					if (*szBoundaryStart == '\"')
						szBoundaryStart++;
					
					strncpy(szBoundary, szBoundaryStart, 
						strcspn(szBoundaryStart, "\";"));
				}
			}
			else if (strstr(szLine, szBoundary))
				break;

			szLastLinePtr = szLinePtr;
			szLinePtr = GetNextLine(szLinePtr, szLine, sizeof(szLine));
		}
	}

	while (szLinePtr != NULL)
	{
		if (strstr(szLine, szBoundary))
		{
			pMimeList->nBodyLength = szLastLinePtr - pMimeList->szBody;
			pMimeList->szFooter = szLastLinePtr;
			pMimeList->nFooterLength = szLinePtr - szLastLinePtr;
	
			pMimeList->nextPart = (MimePart *) calloc(sizeof(MimePart), 1);
			
			pMimeList->nextPart->previousPart = pMimeList;
			pMimeList = pMimeList->nextPart;

			pMimeList->szHeader = szLinePtr;
			pMimeList->nContentType = ContentType_TextPlain;
		}

		if (strstr(szLine, "Content-Type:"))
			MatchContentType(szLine, &(pMimeList->nContentType));

		if ((pMimeList->szBody == NULL) && (strlen(szLine) < 3))
		{
			pMimeList->nHeaderLength = szLinePtr - pMimeList->szHeader;
			pMimeList->szBody = szLinePtr;
		}

		szLastLinePtr = szLinePtr;
		szLinePtr = GetNextLine(szLinePtr, szLine, sizeof(szLine));
	}

	if (pMimeList->szBody == NULL)
	{
		if (pMimeList->previousPart != NULL)
			pMimeList->previousPart->nextPart = NULL;

		free(pMimeList);
	}
	else if (pMimeList->szFooter == NULL)
		pMimeList->nBodyLength = szLastLinePtr - pMimeList->szBody;

	*ppMimeList = pHead;
	return kPGPError_NoErr;
}


static char * GetNextLine(char *szStart,
						  char *szOutput,
						  size_t nOutputSize)
{
	char *szEnd = NULL;
	BOOL bStop = FALSE;

	if (szStart == NULL)
		return NULL;

	szEnd = strchr(szStart, '\n');
	if (szEnd == NULL)
	{
		szEnd = strchr(szStart, '\0');
		bStop = TRUE;
	}

	if ((szOutput != NULL) && (nOutputSize > 0))
	{
		size_t nLineSize;
		size_t nBuffSize;

		nLineSize = szEnd - szStart;
		if (nLineSize == 0)
			szOutput[0] = 0;
		else
		{
			if (nLineSize > nOutputSize)
				nBuffSize = nOutputSize;
			else
				nBuffSize = nLineSize;

			strncpy(szOutput, szStart, nBuffSize);
			szOutput[nBuffSize-1] = 0;
		}
	}

	if (bStop)
		return NULL;
	else
		return szEnd+1;
}


static void MatchContentType(char *szLine, MimeContentType *pnContentType)
{
	if (strstr(szLine, "text/plain"))
		*pnContentType = ContentType_TextPlain;
	else if (strstr(szLine, "text/html"))
		*pnContentType = ContentType_TextHTML;
	else if (strstr(szLine, "multipart/mixed"))
		*pnContentType = ContentType_MultipartMixed;
	else if (strstr(szLine, "multipart/alternative"))
		*pnContentType = ContentType_MultipartAlternative;
	else
		*pnContentType = ContentType_Other;

	return;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
