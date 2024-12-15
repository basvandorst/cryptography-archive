/********************************************************************
 *                                                                  *
 *  MODULE    :  WVATTACH.C                                         *
 *                                                                  *
 *  PURPOSE   : This file contains functions for attachments to     *
 *              mail or news posting                                *
 *                                                                  *
 *  ENTRY POINTS: Attach ()                                         *
 *                                                                  *
 * Author: John S. Cooper (jcooper@netcom.com)                      *
 *   Date: Nov 28, 1993                                             *
 ********************************************************************/
/* 
 * $Id: wvattach.c 1.9 1994/09/16 00:57:09 jcooper Exp $
 * $Log: wvattach.c $
 * Revision 1.9  1994/09/16  00:57:09  jcooper
 * zero-padding in subject template, cleanup for 92.6
 *
 * Revision 1.8  1994/08/24  17:59:26  jcooper
 * misc encoding/decoding changes
 *
 * Revision 1.7  1994/08/11  00:09:17  jcooper
 * Enhancements to Mime and article encoding/encoding
 *
 * Revision 1.6  1994/06/09  18:51:30  rushing
 * problem with symbol 'header' on AXP/NT
 *
 * Revision 1.5  1994/05/23  18:37:00  jcooper
 * new attach code, session [dis]connect
 *
 * Revision 1.3  1994/02/24  21:27:10  jcoop
 * jcoop changes
 *
 * Revision 1.2  1994/01/22  01:30:03  jcoop
 * 90.2 changes
 *
 * Revision 1.1  1994/01/16  12:10:08  jcoop
 * Initial revision
 *
 */ 
#include <windows.h>
#include <windowsx.h>
#include "wvglob.h"
#include "winvn.h"
#pragma hdrstop
#include <string.h>
#include <ctype.h>		/* for isspace, isalnum, etc */
#include <stdlib.h>		/* for itoa */

extern void 	UpdateBlockStatus ();	// in wvcoding.c
/*
 * Forward Declarations
 */
void	AppendPlainFileToPost (char *fileName);
BOOL	SplitCurrentHeader (TypTextBlock *header, TypTextBlock *body, TypTextBlock *tail, char *origSubject);
BOOL	AddMIMEVersion (TypTextBlock *header);
BOOL	InitiateAttach (int useWnd, int DocType);
void	EndAttach ();
void	PostTextBlock (TypTextBlock *block);
unsigned long PostPartialTextBlock (TypTextBlock *block, unsigned long start, unsigned long maxBytes);
int 	PostOneLine (char *str, unsigned long *byteCount, unsigned long maxBytes);
void	GenerateSubject (TypTextBlock *header, char *origSubject, unsigned int part, unsigned int numParts);
HWND	CreateGlobalPostingWnd (HWND hWnd);
void 	FlushCommSpool ();

/*
 * Globals 
 */
// NUM_ENCODING_TYPES should be set in wvglob.h to the # items here
char *EncodingTypes[] = { "Base-64", "UU", "XX", "Custom", "None" };

// NUM_CONTENT_TYPES should be set in wvglob.h to the # items here
char *ContentTypes[] =  
    {	"Text/Plain",
		"Text/Richtext",
	    "Video/MPEG",
	    "Video/AVI",
	    "Image/JPEG",
	    "Image/GIF",
	    "Audio/Basic",
	    "Application/Zip",
	    "Application/PostScript",
	    "Other" };

#define CURRENT_WND 	1
#define NEW_WND		2
#define EDIT_SIZE_INC	512

#define ATTACH_NONE              0
#define ATTACH_START             1 	// state constants for ProcessAttachment
#define ATTACH_FIRST_IN_THIS_WND 2	
#define ATTACH_FIRST_IN_NEXT_WND 3 
#define ATTACH_WAIT              4
#define ATTACH_MAIN              5
#define ATTACH_DONE              6

HWND hThisEditWnd;
HWND hParentWnd;
TypTextBlock *headerBlock, *body, *attachment, *tail;
unsigned int thisPart, numParts;
int AttachmentState;
char attachmentID[MAXINTERNALLINE], origSubject[MAXINTERNALLINE];	
WndEdit *thisPostWnd;
int thisDocType;

unsigned int editSize;		// cannot exceed 64k in an edit window
unsigned int editMaxSize;
char *editMem;
int aMode;				// attachment mode for ended line chars
int saveReviewAttach;

#define MAX_COMM_SPOOL 1000
unsigned int commSpoolLen;
char *commSpool;

/* ---------------------------------------------------------------------------
 * EncodingTypeToNum converts a string describing a coding type into the
 * internal numeric representation
 */
int
EncodingTypeToNum (char *str)
{
	if (!stricmp (str, "Base-64"))
		return CODE_BASE64;
	else if (!stricmp (str, "UU"))
		return CODE_UU;
	else if (!stricmp (str, "XX"))
		return CODE_XX;
	else if (!stricmp (str, "Custom"))
		return CODE_CUSTOM;
	else if (!stricmp (str, "None"))
		return CODE_NONE;
	else
		return CODE_UNKNOWN;
}

/* ---------------------------------------------------------------------------
 * hParentWnd is handle to multi-line edit posting window
 *	Called by wvpost.c after file selected and attachment dialog completed
 *	Based on globals set in attachment dialog, creates attachment
 *	possibly encoded, with MIME headers, and posts it
 *	
 *	If ReviewAttach is set, add attachment to post/mail windows (creating
 *	new windows as necessary for message/partial)
 */
void
Attach (WndEdit *WndPost, char *fileName, int DocType)
{
	time_t theTime;
	unsigned long offset, mimeUsage;
			
	hParentWnd = WndPost->hWnd;
	hThisEditWnd = WndPost->hWndEdit;
	WndPost->dirty = DT_DIRTY;
	thisDocType = DocType;
	thisPostWnd = WndPost;	
	// Initialize text blocks
	if ((headerBlock= InitTextBlock (hCodedBlockWnd)) == NULL)
		return;
	if ((body       = InitTextBlock (hCodedBlockWnd)) == NULL)
		return;
	if ((tail       = InitTextBlock (hCodedBlockWnd)) == NULL)
		return;
	if ((attachment = InitTextBlock (hCodedBlockWnd)) == NULL)
		return;

	aMode = ADD_TO_EDIT;	// end all lines in \r\n

	if (!ReviewAttach)
	{
		if ((commSpool = (char *)GlobalAllocPtr (GMEM_MOVEABLE, MAX_COMM_SPOOL*sizeof (char))) == NULL)
		{
			MessageBox (hParentWnd, "Memory allocation failure", "Comm Spool Init Failed", MB_OK);
			return;
		}			
		commSpool[0] = '\0';
		commSpoolLen = 0;
	}
	
	saveReviewAttach = ReviewAttach;
	if (DocType == DOCTYPE_MAIL)
		ReviewAttach = TRUE;			// must review when mailing
		
	CreateStatusArea (hParentWnd);		// init hCodedBlockWnd and currentCoded
	strcpy (currentCoded->ident, fileName);	// status info

	if (EncodingTypeNum == CODE_NONE)
	{		
		if (ReadFileToTextBlock (hParentWnd, attachment, fileName, aMode) == FAIL)
			{  FinishAttachment (ABORT); return;  }
	}
	else 	if (Encode (attachment, fileName, aMode) == FAIL)
		{  FinishAttachment (ABORT); return;  }

	CodingState = ATTACH_POSTING;
	InvalidateRect (hCodedBlockWnd, NULL, TRUE);	// clear background

	time (&theTime);
	sprintf (attachmentID, "\"%ld@%s\"", theTime, UserName);
        
	if (SplitCurrentHeader (headerBlock, body, tail, origSubject))
		{  FinishAttachment (ABORT); return;  }
	
	if (!GenerateMIME || BlankBeforeMIME)
	   if (AddEndedLineToTextBlock (headerBlock, "", aMode))
		{  FinishAttachment (ABORT); return;  }

        if (GenerateMIME && AddMIMEVersion (headerBlock))
        	{  FinishAttachment (ABORT); return;  }
  
	// if header+tail won't fit in ArticleSplitLength bytes, can't do it
	if (headerBlock->numBytes + tail->numBytes + 200 > ArticleSplitLength)
	{
		MessageBox (hCodedBlockWnd, "Article split length too short to contain headers", "Article Split Length Too Small", MB_OK);
		FinishAttachment (ABORT); return;
	}

	// if the attachment won't fit in this article, force start in next
	if (!AttachInNewArt && 
	   (body->numBytes + headerBlock->numBytes + tail->numBytes + 200 > ArticleSplitLength))
	{
		MessageBox (hCodedBlockWnd, "Beginning attachment in next article", "Edit Window Space Shortage", MB_OK);
		AttachInNewArt == TRUE;
	}
	
	if (ArticleSplitLength > 0)
	{
		offset = (AttachInNewArt) ? 0 : body->numBytes;
		// calc encodelength/splitlength rounded up
		numParts = (int)((attachment->numBytes + offset + 
			 ArticleSplitLength) / ArticleSplitLength);
		// now add in the length of header+sig for each part
		// the extra 200 bytes is rough avg for the MIME headers
		// on each part.  Round up to next articlesplitlen
		mimeUsage = GenerateMIME ? 200L : 0L;
		numParts = (int)((attachment->numBytes + offset + 
			 (long)numParts*(headerBlock->numBytes + tail->numBytes + mimeUsage) + 
			 ArticleSplitLength) / ArticleSplitLength);
        } else
        	numParts = 1;

	AttachmentState = ATTACH_START;
	ProcessAttach (CONTINUE);
}

/* ---------------------------------------------------------------------------
 * ProcessAttach
 * this is written as a state machine to allow interruption in !ReviewMode
 * to wait for News Server comm responses.
 * Thus in !ReviewMode, after an Initiate/EndAttach, we give up control
 * and wvutil calls ProcessAttach when ready to continue
 */
void
ProcessAttach (int action)
{
	static unsigned long attachLine, byteCount, saveHeaderLines, saveHeaderBytes, dummyLong;
	static BOOL MultipartMixed;
	char temp[MAXINTERNALLINE];
	static int abort;
	int i, found;
		
	if (action == ABORT) 
	{
		MessageBox (hCodedBlockWnd, "Posting with attachment aborted", "Attachment", MB_OK|MB_ICONHAND);
		AttachmentState = ATTACH_DONE;
	}

  while (AttachmentState != ATTACH_NONE) {
	switch (AttachmentState) {

	case ATTACH_START:
		abort = ABORT;
		byteCount = 0;
		MultipartMixed = FALSE;
		attachLine = 0;
		saveHeaderLines = headerBlock->numLines;
		saveHeaderBytes = headerBlock->numBytes;
		currentCoded->numLines = 0;
		currentCoded->numBytes = 0;

		if (AttachInNewArt)
		{
			thisPart = 0;
			AttachmentState = ATTACH_FIRST_IN_NEXT_WND;
		}
		else
		{
			thisPart = 1;
			AttachmentState = ATTACH_FIRST_IN_THIS_WND;
		}
		currentCoded->sequence = thisPart;	// status window info

		if (InitiateAttach (CURRENT_WND, thisDocType))
			{  AttachmentState = ATTACH_DONE; break;  }
		if (!ReviewAttach)
			return;

		break;

	case ATTACH_FIRST_IN_THIS_WND:
		if (!ReviewAttach)
			CommState = ST_POST_WAIT_END;
			
		if (body->numLines != 0)	// if have body, then mixed
		{
		  GenerateSubject (headerBlock, origSubject, thisPart, numParts);
		  saveHeaderBytes = headerBlock->numBytes;

		  if (GenerateMIME)
		  {
		    if (AddEndedLineToTextBlock (headerBlock, "Content-Type: multipart/mixed;",aMode))
	  		{  AttachmentState = ATTACH_DONE; break;  }

		    sprintf (temp, "     Boundary=\"%s\"", MIMEBoundary);
		    if (AddEndedLineToTextBlock (headerBlock, temp, aMode))
			{  AttachmentState = ATTACH_DONE; break;  }
		  
		    // blank preamble here
		    // each boundary must be preceded by a CRLF (add "")
		    if (AddEndedLineToTextBlock (headerBlock, "", aMode))
			{  AttachmentState = ATTACH_DONE; break;  }
		    sprintf (temp, "--%s", MIMEBoundary);
		    if (AddEndedLineToTextBlock (headerBlock, temp, aMode))
			{  AttachmentState = ATTACH_DONE; break;  }
                  
                    // explicitly type the encapsulated section
		    if (AddEndedLineToTextBlock (headerBlock, "Content-Type: text/plain", aMode))
			{  AttachmentState = ATTACH_DONE; break;  }
		    if (AddEndedLineToTextBlock (headerBlock, "", aMode))
			{  AttachmentState = ATTACH_DONE; break;  }
		  
		    if (AddEndedLineToTextBlock (body, "", aMode))
			{  AttachmentState = ATTACH_DONE; break;  }
		    sprintf (temp, "--%s", MIMEBoundary);
		    if (AddEndedLineToTextBlock (body, temp, aMode))
			{  AttachmentState = ATTACH_DONE; break;  }
		  } 
		  else	// no MIME
		  {
  		    if (AddEndedLineToTextBlock (body, "", aMode))
			{  AttachmentState = ATTACH_DONE; break;  }
  		    if (AddEndedLineToTextBlock (body, "BEGIN --- CUT HERE --- Cut Here --- cut here ---", aMode))
			{  AttachmentState = ATTACH_DONE; break;  }
		  }
		  PostTextBlock (headerBlock);
		  PostTextBlock (body);
		  MultipartMixed = TRUE;
		  byteCount = headerBlock->numBytes + body->numBytes;
		}
		AttachmentState = ATTACH_MAIN;
		break;

	case ATTACH_FIRST_IN_NEXT_WND:
		// post current stuff, begin attachment in NEXT article
		if (!ReviewAttach)
			CommState = ST_POST_WAIT_END;

		GenerateSubject (headerBlock, origSubject, thisPart, numParts);
		saveHeaderBytes = headerBlock->numBytes;
		
		if (GenerateMIME)
		{
                  if (AddEndedLineToTextBlock (headerBlock, "Content-Type: text/plain", aMode))
			{  AttachmentState = ATTACH_DONE; break;  }
		}
		if (AddEndedLineToTextBlock (headerBlock, "", aMode))
			{  AttachmentState = ATTACH_DONE; break;  }
		PostTextBlock (headerBlock);
		PostTextBlock (body);  
		PostTextBlock (tail);  

		thisPart = 1;
		currentCoded->sequence = thisPart;	// status window info
		AttachmentState = ATTACH_WAIT;

		EndAttach ();
		if (!ReviewAttach)
			return;

		break;

	case ATTACH_WAIT:
		if (InitiateAttach (NEW_WND, thisDocType))
			{  AttachmentState = ATTACH_DONE; break;  }
		AttachmentState = ATTACH_MAIN;
		if (!ReviewAttach)
			return;

		break;

	case ATTACH_MAIN:
		if (!ReviewAttach)
			CommState = ST_POST_WAIT_END;

		headerBlock->numLines = saveHeaderLines;	// reset to orig header
		headerBlock->numBytes = saveHeaderBytes;

		GenerateSubject (headerBlock, origSubject, thisPart, numParts);
		saveHeaderBytes = headerBlock->numBytes;
                if (GenerateMIME)
                {
		  if (numParts > 1)
		  {
			if (AddEndedLineToTextBlock (headerBlock, "Content-Type: message/partial;", aMode))
				{  AttachmentState = ATTACH_DONE; break;  }
			sprintf (temp, "     id=%s;", attachmentID);
			if (AddEndedLineToTextBlock (headerBlock, temp, aMode))
				{  AttachmentState = ATTACH_DONE; break;  }
			sprintf (temp, "     number=%d; total=%d", thisPart, numParts);
			if (AddEndedLineToTextBlock (headerBlock, temp, aMode))
				{  AttachmentState = ATTACH_DONE; break;  }

			if (AddEndedLineToTextBlock (headerBlock, "", aMode))
				{  AttachmentState = ATTACH_DONE; break;  }
		  }
		  if (thisPart == 1)
		  {
	 		for (i = 0, found = FALSE; i < NUM_CONTENT_TYPES && !found; i++)
				if (!_stricmp (ContentType, ContentTypes[i]))
					found = TRUE;

			if (!_stricmp (ContentType, "Other") || !found)
				sprintf (temp, "Content-Type: Application/octet-stream");
			else					
				sprintf (temp, "Content-Type: %s", ContentType);

			if (AddEndedLineToTextBlock (headerBlock, temp, aMode))
				{  AttachmentState = ATTACH_DONE; break;  }
			         
			switch (EncodingTypeNum)
			{
			case CODE_UU:	
				sprintf (temp, "Content-Transfer-Encoding: %s", MIMEUUType);
				break;
			case CODE_XX:
				sprintf (temp, "Content-Transfer-Encoding: %s", MIMEXXType);
				break;
			case CODE_CUSTOM:
				sprintf (temp, "Content-Transfer-Encoding: %s", MIMECustomType);
				break;
			case CODE_BASE64:
				strcpy (temp, "Content-Transfer-Encoding: Base64");
				break;
			case CODE_NONE:
				temp[0] = '\0';
			}
			if (temp[0] != '\0' &&
			   (AddEndedLineToTextBlock (headerBlock, temp, aMode)))
				{  AttachmentState = ATTACH_DONE; break;  }

			if (AddEndedLineToTextBlock (headerBlock, "",aMode))
				{  AttachmentState = ATTACH_DONE; break;  }
		  }
		}

		// if multipart/mixed and first part, then the main header 
		// was already displayed, so we now want to post from the end 
		// of that header on (byteCount already set above)
		if (MultipartMixed && thisPart == 1)
		{
			if (GenerateMIME)
				PostPartialTextBlock (headerBlock, saveHeaderLines, byteCount-saveHeaderBytes);
		}
		else                                
		{
			PostTextBlock (headerBlock);
			byteCount = headerBlock->numBytes;
		}

		if (numParts > 1)
		{
			if (attachLine == attachment->numLines)
			{
				sprintf (temp, "[WinVn: Humble apologies: the # of attachment blocks was estimated incorrectly]\r");
				if (aMode == ADD_TO_EDIT) 
					strcat (temp, "\n");
				PostOneLine (temp, &dummyLong, 0);
				thisPart = numParts;
			}
			else
			{
				// calculate number of bytes from attachment to use 
				// in this part: we've already used byteCount bytes
				// in headers, and we need to leave room for the tail
				byteCount = ArticleSplitLength - byteCount - tail->numBytes;
				if ((attachLine = PostPartialTextBlock (attachment, attachLine, byteCount)) == 0)
					{  AttachmentState = ATTACH_DONE; break;  }
			}
		}
		else
			PostTextBlock (attachment);
		
		// if this is the last section, don't end it since we may
		// still have a boundary and a tail to add
		if (thisPart + 1 <= numParts) 
		{
			thisPart++;
			AttachmentState = ATTACH_WAIT;

			currentCoded->sequence = thisPart;	// status window info
			currentCoded->numLines = 0;

			EndAttach ();
			if (!ReviewAttach)
				return;
		}
		else // Done with attachment
		{	
			// if multi-part/mixed then need to attach the final boundary
			if (GenerateMIME && MultipartMixed)
			{
				ResetTextBlock (body);
				if (AddEndedLineToTextBlock (body, "", aMode))
					{  AttachmentState = ATTACH_DONE; break;  }
				sprintf (temp, "--%s--", MIMEBoundary);
				if (AddEndedLineToTextBlock (body, temp, aMode))
					{  AttachmentState = ATTACH_DONE; break;  }
				PostTextBlock (body);
			}	
				
			// Signature/tail ends up after last boundary, in epilogue
			PostTextBlock (tail);
			AttachmentState = ATTACH_DONE;
			abort = CONTINUE;		// successful

			EndAttach ();
			if (!ReviewAttach)
				return;
		}
		break;

	case ATTACH_DONE:
		FinishAttachment (abort); 
		AttachmentState = ATTACH_NONE;
		break;
	
	} // end of switch (AttachmentState)
  
  } // end of while (AttachmentState != ATTACH_NONE)
  return;
}

void
FinishAttachment (int action)
{
	char temp[255];
	
	FreeTextBlock (headerBlock);
	FreeTextBlock (body);
	FreeTextBlock (attachment);
	
	CodingState = INACTIVE;
	DestroyStatusArea();

	// if we are managing the post (not reviewing), then we must close the
	// one post window when we're through (don't close if aborted)
	if (!ReviewAttach)
	{
		if (action == CONTINUE)
		{
			sprintf (temp, "Posting completed in %d part", numParts);
			if (numParts > 1)
				strcat (temp, "s");
				
			MessageBox (thisPostWnd->hWnd, temp, "Posting done", MB_OK|MB_ICONINFORMATION);
			DestroyWindow (thisPostWnd->hWnd);
		}
		GlobalFreePtr (commSpool);
	}		
	ReviewAttach = saveReviewAttach;
}

/* ------------------------------------------------------------------------
 * 	Reads current text from the posting edit buffer, 
 *	splits all RFC822 header stuff into header block,
 *		save off subject line from header during copy
 *	and body up to any signature into the body block.
 *	from signature on goes into tail block
 *	This is so we can insert our MIME stuff right in between
 *	Assumes there is a null (blank) line between RFC822 header and body
 *	Tailing signature
 */
#define READING_HEADER 	1
#define READING_BODY    2
#define READING_TAIL    3
BOOL
SplitCurrentHeader (TypTextBlock *header, TypTextBlock *body, TypTextBlock *tail, char *origSubject)
{
        char *editBuf, *ptr, *end;
        int state;
	char temp[MAXINTERNALLINE];
	        
	origSubject[0] = '\0';
	if ((editBuf = GetEditText (hThisEditWnd)) == NULL)
		return (FAIL);
	
	for (state = READING_HEADER, ptr = editBuf; *ptr != '\0';)
	{
		end = strstr (ptr, "\r\n");     // all edit buf lines end in \r\n
		if (end == NULL)
		{
			strcpy (temp, ptr);	// last line - no \r\n
			*ptr = '\0';		// finish loop, then stop
		}
		else
		{
			*end = '\0';            // strip \r\n
			strcpy (temp, ptr);
			ptr = end + 2;          // skip \r\n for next line
		}
		switch (state)
		{
		  case READING_HEADER:
			if (!IsBlankStr(temp))
			{
				if (AddEndedLineToTextBlock (header, temp, aMode))
					return (FAIL);;
				if (!_strnicmp (temp, "subject:", 8))
					strcpy (origSubject, temp);
			}
			else	// switch to reading body				    
			{
				state = READING_BODY;
				continue;
			}
			break;
		
		  case READING_BODY:
			if (body->numLines == 0 && IsBlankStr (temp))
				continue;	// skip leading blank lines
				
			if (EnableSig && Signature->numLines > 0 && 
			    !strcmp (temp, TextBlockLine (Signature, 0)))
			{
				if (AddEndedLineToTextBlock (tail, temp, aMode))
					return (FAIL);
				state = READING_TAIL;
				continue;
			}
			if (AddEndedLineToTextBlock (body, temp, aMode))
				return (FAIL);
			break;
		
		  case READING_TAIL:
			if (AddEndedLineToTextBlock (tail, temp, aMode))
				return (FAIL);
        	}       

	}
	GlobalFreePtr (editBuf);
	return (SUCCESS);
}	
	
/* ------------------------------------------------------------------------
 *	Start/end posting
 */
BOOL
InitiateAttach (int useWnd, int DocType)
{
	WndEdit *NewWndPost;
		
	if (ReviewAttach)
	{
		/* create new window for each posted section
		 * if attach-now or thisPart==0 use existing window for 1st section
		 */
		if (useWnd == NEW_WND)
		{
			if ((hParentWnd = CreatePostingWnd (hParentWnd, NULL, DocType)) == NULL)
			{
				MessageBox (hParentWnd, "Failed to create new attachment window", "Posting Window Creation", MB_OK);
				return (FAIL);
			}
							
			if (DocType == DOCTYPE_POSTING)
			   NewWndPost = getWndEdit(WndPosts, hParentWnd, MAXPOSTWNDS);
			else
			   NewWndPost = getWndEdit(WndMails, hParentWnd, MAXMAILWNDS);
			
			NewWndPost->dirty = DT_DIRTY;
			hThisEditWnd = NewWndPost->hWndEdit;
		}
		sprintf (str, "Review Attachment Part %d of %d", thisPart, numParts);
		SetWindowText (hParentWnd, str);
		if ((editMem = (char *) GlobalAllocPtr (GMEM_MOVEABLE|GMEM_ZEROINIT, EDIT_SIZE_INC*sizeof(char))) == NULL)
		{
			MessageBox (hParentWnd, "Memory allocation failure", "Attachment", MB_OK);
			return (FAIL);
		}
		editMaxSize = EDIT_SIZE_INC;
		editSize = 0;
	}
	else		
		if (!StartPost (thisPostWnd))
			return (FAIL);

	return (SUCCESS);
}

void
EndAttach (HWND hParentWnd)
{
	unsigned long dummyLong;
	
	if (ReviewAttach)
	{
		SetEditText (hThisEditWnd, editMem);
		GlobalFreePtr (editMem);
	}
	else
	{
		PostOneLine (".\r\n", &dummyLong, 0);
		FlushCommSpool ();
	}
}

void
FlushCommSpool ()
{
	if (commSpoolLen > 0)
	{
		PutCommData (commSpool, commSpoolLen);
		commSpoolLen = 0;
		commSpool[0] = '\0';
	}
}

/* ------------------------------------------------------------------------
 * Post the contents of a text block
 *	Assumes posting already initiated, and does not end the posting
 *	Lines must end in \r\n
 */

int 
PostOneLine (char *str, unsigned long *byteCount, unsigned long maxBytes)
{
	unsigned int len;

	len = strlen (str);
		
	*byteCount += (long)len;
	if (maxBytes > 0 && *byteCount >= maxBytes)
		return (SUCCESS);

	if (ReviewAttach)
	{
		editSize += len;
		if (editSize >= editMaxSize)
		{
			editMaxSize += max(EDIT_SIZE_INC, len);
			if ((editMem = GlobalReAllocPtr (editMem, editMaxSize*sizeof(char), GMEM_MOVEABLE)) == NULL)
			{
				MessageBox (hThisEditWnd, "Memory allocation failure", "Build Attachment", MB_OK);
				return (FAIL);
			}
		}
		strcat (editMem, str);
		currentCoded->numBytes = (long)editSize;
	}
	else
	{
		if (commSpoolLen + len >= MAX_COMM_SPOOL)
			FlushCommSpool ();

		strcat (commSpool, str);
		commSpoolLen += len;
		currentCoded->numBytes += len;
	}

	return (SUCCESS);
}

void
PostTextBlock (TypTextBlock *block)
{
	PostPartialTextBlock (block, 0L, 0L);
}

unsigned long
PostPartialTextBlock (TypTextBlock *block, unsigned long start, unsigned long maxBytes)
{
	register unsigned long i;
	unsigned long byteCount;

	byteCount = 0;	
			
	for (i = start; i < block->numLines; i++)
	{
		if (PostOneLine (TextBlockLine (block, i), &byteCount, maxBytes) == FAIL)
			return (0);
		
		if (maxBytes > 0 && byteCount >= maxBytes)
			break;
			
		currentCoded->numLines++;
		if (currentCoded->numLines % STATUS_UPDATE_FREQ == 0)
			UpdateBlockStatus();
	}
	UpdateBlockStatus();	// show final size
	return (i);
}
/* ------------------------------------------------------------------------
 *	Add a line to the given header block which contains the MIME-Version
 */
BOOL
AddMIMEVersion (TypTextBlock *header)
{
	char temp[MAXINTERNALLINE];
	
	sprintf (temp, "MIME-Version: %s", MIME_VERSION);
	if (AddEndedLineToTextBlock (header, temp, aMode))
		return (FAIL);;

	return (SUCCESS);
}

/* ------------------------------------------------------------------------
 *	Generate a subject line based on orig subject line, and the
 * 	subject template
 *		Replace %f with AttachFileName
 *		Replace %p with part #
 *		Replace %0p with part # 0 padded to length of total # parts
 *		Replace %t with total # parts
 *		Replace %s with orig subject content
 *
 */
void
GenerateSubject (TypTextBlock *header, char *origSubject, 
		 unsigned int part, unsigned int numParts)
{
	register char *src, *dest, *ptr;
	register unsigned long num;
	char newSubject[MAXINTERNALLINE];
	char endLine[3], numStr[10];
	int numZeros;
	extern char *NameWithoutPath ();

	for (num = 0; num < header->numLines; num++)
		if (!_strnicmp (TextBlockLine (header, num), "subject:", 8))
			break;
		
	ptr = strpbrk (TextBlockLine (header, num), "\n\r");
	strcpy (endLine, ptr);

	for (src = SubjectTemplate, dest = newSubject; *src != NULL;)
	{
		if (*src == '%')
		{
			src++;
			switch (*src)
			{
			case '%':		// literal percent is %%
				*dest++ = '%';
				break;
			case 's':
				for (ptr = origSubject; *ptr; *dest++ = *ptr++);
				break;
			case 'f':
				NameWithoutPath (str, AttachFileName);				
				for (ptr = str; *ptr; *dest++ = *ptr++);
				break;

			case '0':
				if (*(src+1) == 'p')
				{
					src++;	// skip zero
					// zero-pad number to length of numParts string
					itoa (numParts, numStr, 10);	// longer or equal in length to part
					itoa (part, str, 10);
					
					for (numZeros = strlen(numStr) - strlen(str); numZeros; numZeros--)
						*dest++ = '0';
					
					for (ptr = str; *ptr; *dest++ = *ptr++);
				}
				break;
				
			case 'p':
				itoa (part, numStr, 10);
				for (ptr = numStr; *ptr; *dest++ = *ptr++);
				break;

			case 't':
				itoa (numParts, numStr, 10);
				for (ptr = numStr; *ptr; *dest++ = *ptr++);
				break;

			default:
				*dest++ = '%';
				*dest++ = *src;
				break;

			}
			src++;	// skip control char after %
			continue;
		}
		*dest++ = *src++;	
	}
	*dest = '\0';   
	strcat (dest, endLine);	// replace end of line

	ReplaceLineInTextBlock (header, num, newSubject);
}


#if OLD_PLAIN_FILE
/* ------------------------------------------------------------------------
 * 	Directly modifies the Edit buffer of the parent multi-line edit wnd
 *	Appends contents of fileName to buffer
 */
void
AppendPlainFileToPost (char *fileName)
{
	HFILE hFile;
	register unsigned int i;
        char *editBuf, *inBuf;
        unsigned int endPtr, size, numRead;
        
        inBuf = (char *) GlobalAllocPtr(GMEM_FIXED, 2048*sizeof(char));
	editBuf = GetEditText (hThisEditWnd);
	
	size = strlen (editBuf);        
	if ((hFile = _lopen (fileName, READ)) == HFILE_ERROR)
	{
		sprintf(str, "Could not open file %s for read", fileName);
		MessageBox (hParentWnd, str, "Attachment Open File Error", MB_OK);
		return;
	}
	
    	while (1)
    	{
   		if ((numRead = _lread(hFile, inBuf, 2048)) == 0)
    			break;
       		for (i = 0; i < numRead; i++)
		  if (!__isascii (inBuf[i]))
		  {
			sprintf(str, "File %s appears to contain non-ASCII data.\nBinary files must be encoded for attachment", fileName);
			MessageBox (hParentWnd, str, "Non-ASCII File Error", MB_OK);
			goto endAppend;
		  }
		
		endPtr = size;
		size += numRead;
		editBuf = GlobalReAllocPtr (editBuf, size, GMEM_MOVEABLE);
		memmove (editBuf + endPtr, inBuf, numRead);
	}
   
   endAppend:;
	_lclose(hFile);

	SetEditText (hThisEditWnd, editBuf);
        GlobalFreePtr (editBuf);
        GlobalFreePtr (inBuf);
}
#endif                                                                       

#if OLD_HEADER
/* ------------------------------------------------------------------------
 * Returns true for lines which look like standard header lines
 * Returns non-zero if should skip this line, else zero
 * A binary search through a list of words would be really smart
 * Lazy approach
 */
int
TestRfc822HeaderLine (char *line)
{
	switch (tolower(line[0]))
	{
	case 'd': 
		return (!_strnicmp(line, "date", 4));
	case 'f': 
		return (!_strnicmp(line, "follow", 6) ||
 		 	!_strnicmp(line, "from", 4));
 	case 'k':
		return (!_strnicmp(line, "keywords", 8));
		
	case 'l':
		return (!_strnicmp(line, "lines", 5));
	case 'm':
		return (!_strnicmp(line, "message", 7));
	case 'n':
		return (!_strnicmp(line, "newsgrou", 8) ||
		  	!_strnicmp(line, "nntp", 4));
	case 'o':
		return (!_strnicmp(line, "organiza", 8) ||
		 	!_strnicmp(line, "originat", 8));
        case 'p':
		return (!_strnicmp(line, "path", 4));
	case 'r':
		return (!_strnicmp(line, "referenc", 8) ||
			!_strnicmp(line, "reply", 5));
	case 's':
 		return (!_strnicmp(line, "sender", 6) ||
 			!_strnicmp(line, "subject:", 8) ||
 			!_strnicmp(line, "summary", 7));
	case 'x':
		return (!_strnicmp(line, "xref:", 5) ||
		        !_strnicmp(line, "x-news", 6));
	default:
		return (0);
	}
}	

#endif                                                                       
