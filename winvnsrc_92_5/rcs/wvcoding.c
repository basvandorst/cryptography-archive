head	1.10;
access;
symbols;
locks; strict;
comment	@ * @;


1.10
date	94.08.24.18.00.29;	author jcooper;	state Exp;
branches;
next	1.9;

1.9
date	94.08.11.21.03.19;	author rushing;	state Exp;
branches;
next	1.8;

1.8
date	94.08.11.20.16.25;	author rushing;	state Exp;
branches;
next	1.7;

1.7
date	94.08.11.00.09.17;	author jcooper;	state Exp;
branches;
next	1.6;

1.6
date	94.07.25.20.05.13;	author jcooper;	state Exp;
branches;
next	1.5;

1.5
date	94.05.23.18.36.00;	author jcooper;	state Exp;
branches;
next	1.4;

1.4
date	94.03.01.19.14.22;	author rushing;	state Exp;
branches;
next	1.3;

1.3
date	94.02.24.21.28.04;	author jcoop;	state Exp;
branches;
next	1.2;

1.2
date	94.02.09.18.01.08;	author cnolan;	state Exp;
branches;
next	1.1;

1.1
date	94.01.18.09.54.20;	author jcoop;	state Exp;
branches;
next	;


desc
@jcoop's en/decoding routines
@


1.10
log
@misc encoding/decoding changes
@
text
@/********************************************************************
 *                                                                  *
 *  MODULE    :  WVCODING.C                                         *
 *                                                                  *
 *  PURPOSE   : This file contains functions for en/decoding files  *
 *                                                                  *
 *  ENTRY POINTS:                                                   *
 *    		DecodeInit  - initializes decoding resources	    *
 *		DecodeDone  - frees decoding resources              *
 *		InitCoded   - initializes a coded block             * 
 *			      called for each new art to decode     *
 *		DecodeLine  - Main decoding engine.  Called once    *
 *			      for each input line                   *
 *		DecodeFile  - Uses file instead of comm input as a  *
 *   			      line provider to the decoding engine  *
 *		DecodeDoc   - Uses a Doc instead of comm input as a *
 *   			      line provider to the decoding engine  *
 *	CompleteThisDecode  - Called at end of each article - adds  *
 *			      block to appropriate decode thread,   *
 *			      writes to disk if possible, etc       *
 *                  Encode  - Encodes a file into a text block      *
 *            EncodeToFile  - Calls Encode, then writes block to    * 
 *                            an output file                        *
 *                                                                  *
 *  NOTES:                                                          *
 *	Both the max # of batched decode files and the max #        * 
 *	threads per file are fixed.  This should be made dynamic    *
 *	at some point I guess.                                      *
 *                                                                  *
 * Author: John S. Cooper (jcooper@@netcom.com)                      *
 *   Date: Sept 13, 1993                                            *
 *                                                                  *
 * Let me know if you like this code, or if you re-use any piece of *
 * it. I'm just curious to know what people think.                  *
 ********************************************************************/
/* 
 * $Id: wvcoding.c 1.9 1994/08/11 21:03:19 rushing Exp $
 * $Log: wvcoding.c $
 * Revision 1.9  1994/08/11  21:03:19  rushing
 * FindExecutable fix for NT
 *
 * Revision 1.8  1994/08/11  20:16:25  rushing
 * bug fix
 *
 * Revision 1.6  1994/07/25  20:05:13  jcooper
 * execution of decoded files
 *
 * Revision 1.5  1994/05/23  18:36:00  jcooper
 * new attach code, session [dis]connect
 *
 * Revision 1.4  1994/03/01  19:14:22  rushing
 * ifdef'd the DEBUG info
 *
 * Revision 1.3  1994/02/24  21:28:04  jcoop
 * jcoop changes.
 *
 * Revision 1.2  1994/02/09  18:01:08  cnolan
 * cnolan 90.2 changes
 *
 * Revision 1.1  1994/01/18  09:54:20  jcoop
 * Initial revision
 *
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <ctype.h>		/* for isspace, isalnum, etc */
#include "wvglob.h"
#include "winvn.h"

#ifdef _DEBUG
#define DEBUG_MAP
#define DEBUG_MEM
#define DEBUG_DECODE
#endif

#define LOW		1	// sequence confidence levels
#define HIGH            2
#define UNUSED		127	// unused mapping slots

#define END_BLOCK	2
#define CODE_LINE_LEN	45	
/*
 * Globals for this func
 */
char
	xxTable[CODINGTABLESIZE] = "+-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
char
	uuTable[CODINGTABLESIZE] = "`!\"#$%&'()*+,-./0123456789:;<=>?@@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";
char
	base64Table[CODINGTABLESIZE] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char	*codingTable;
char 	customTable[CODINGTABLESIZE];

TypDecodeThread *threadList[MAX_DECODE_THREADS];
int	numDecodeThreads;		// number of active decode threads
int	currentDecodeThread;
int	numDumbDecoded;
BOOL    bossanovaMIME;                  // = TRUE if found MIME Version header
int  	thisNumBlocks; 			// MIME vars for current block
char 	thisContentType[80];
char	thisContentDesc[MAXINTERNALLINE];
char	thisBoundary[MAXINTERNALLINE];
int	thisContentEncoding, prevContentEncoding;
char 	prevBlockIdent[MAXFILENAME];
/*
 * Private functions
 */
int 	EncodeLine (unsigned char *outLine, unsigned char *line, int start, int num);
void	EncodeUnit (unsigned char *out, unsigned char *in, int num);
int	GetThreadByName (char *name);
int	AddToThreadList ();
void 	DestroyThread (int num);
void	DestroyCodedBlock (TypCoded **thisCoded);
void	InsertBlockInThread (int num, TypCoded *thisBlock, int index);
void	DeleteBlockFromThread (int num, int index);
BOOL	WriteDecodeThread(int num);
BOOL	WriteBlockToFile(int num, TypCoded *thisDecode);
BOOL	WriteTopBlockToFile (int num, int *endFlag);
BOOL	AddDataToBlock (TypCoded *thisDecode, char *newData, unsigned int dataLen);
void	ParseInfoLine (TypCoded *thisDecode, char *line, BOOL guessIdent);
BOOL	ParseAppSpecificLine (TypCoded *thisDecode, char *line);
char 	*ReadSubjectToken (char *dest, char **ptr);
BOOL	isnumber (char *str);
BOOL	SearchThreadNames (char *name);
int	ParseMimeHeaderLine (TypCoded *thisDecode, char *line);
int	ParseMimeContentType (TypCoded *thisDecode, char *line);

int	DecodeDataLine (TypCoded *decode, char *line);
void 	CreateUUTable();
BOOL	TestIgnoreLine (char *line);
int	IsDataLine (char *line);
BOOL 	TestDataLine (char *line);
int 	ThreadTable (char *dest, char *name);
void	UpdateThreadStatus (int num, char *str);

void	ExecuteDecodedFile (int num, char *fileName);

#if defined(DEBUG_DECODE) || defined(DEBUG_MEM)
#define DEBUG_FILE "coding.log"
void 	DebugLog();
void 	InitDebugLog();
char 	debug[MAXCOMMLINE];
OFSTRUCT debugFileStruct;
#endif

#if defined(DEBUG_DECODE) || defined(DEBUG_MEM)
/* ------------------------------------------------------------------------
 * Debug log handler
 * Open/close the file for every addition to log, so all info is safely
 * captured in case of GPF
 */
void
DebugLog ()
{
	HFILE hDebugFile;

        if ((hDebugFile = OpenFile ((char far *)NULL, &debugFileStruct, OF_REOPEN|OF_WRITE)) >= 0)
	{       
		_llseek(hDebugFile, 0L, 2);	/* go to end */
        	_lwrite(hDebugFile, debug, strlen(debug));
        	_lclose(hDebugFile);
        }
}
void
InitDebugLog ()
{
	HFILE hDebugFile;

        if ((hDebugFile = OpenFile ((char far *)DEBUG_FILE, &debugFileStruct, OF_CREATE)) >= 0)
	{
        	sprintf(debug, "New log");
        	_lwrite(hDebugFile, debug, strlen(debug));
        	_lclose(hDebugFile);
        }
}
#endif
/* ------------------------------------------------------------------------
 * Decode a file
 * Uses a file instead of the comm article as a line-provider to the 
 * decode routines
 */
void
DecodeFile (HWND hParentWnd)
{
	FILE *DecodeFile;
        char mybuf[MAXINTERNALLINE], fileName[MAXFILENAME], *ptr;
        int result;

        if (AskForExistingFileName (hParentWnd, fileName, "Open encoded file") == FAIL)
        	return;

	DecodeInit();
	if ((DecodeFile = fopen (fileName, "r")) == NULL)
	{
		MessageBox (hParentWnd, "Failure to read file", "File error", MB_OK);
		return;
	} 
	if ((currentCoded = InitCoded(hParentWnd)) == NULL)
	{
		MessageBox (hParentWnd, "Unable to continue due to memory constraints.  Aborted", "Init Coded Object Error", MB_OK);
		return;
	}
       	while (1)
       	{
       		if ((ptr = fgets(mybuf, MAXINTERNALLINE, DecodeFile)) == NULL)
       			strcpy (mybuf, "EOF");	// make sure wrap up occurs

		result = DecodeLine (currentCoded, mybuf);
		if (result == FAIL)
			goto abortDecodeFile;
		if (result == END_BLOCK)
		{
			CompleteThisDecode ();
	   		if ((currentCoded = InitCoded(hParentWnd)) == NULL)
			{
				MessageBox (hParentWnd, "Unable to continue due to memory constraints.  Aborted", "Init Coded Object Error", MB_OK);
				goto abortDecodeFile;
			}
		}
		if (ptr == NULL)	// EOF
			break;
	}

	if (currentCoded != NULL)	// finish final block
		CompleteThisDecode ();
   
   abortDecodeFile:;
      	fclose(DecodeFile);
	DecodeDone ();
}
/* ------------------------------------------------------------------------
 * Decode a doc
 * Uses a doc instead of the comm article as a line-provider to the 
 * decode routines
 */
void
DecodeDoc (HWND hParentWnd, TypDoc *Document)
{
        int result;
	TypBlock far *BlockPtr;
	TypLine far *LinePtr;
	        
	DecodeInit();
	if ((currentCoded = InitCoded(hParentWnd)) == NULL)
	{
		MessageBox (hParentWnd, "Unable to continue due to memory constraints.  Aborted", "Init Coded Object Error", MB_OK);
		return;
	}
	
	LockLine (Document->hFirstBlock, sizeof (TypBlock), (TypLineID) 0L, &BlockPtr, &LinePtr);
	while (LinePtr->length != END_OF_BLOCK)
	{
		result = DecodeLine (currentCoded, 
				((char far *) LinePtr) + sizeof (TypText) + sizeof (TypLine));

		if (result == FAIL)
			goto abortDecodeDoc;
		if (result == END_BLOCK)
		{
			CompleteThisDecode ();
	   		if ((currentCoded = InitCoded(hParentWnd)) == NULL)
			{
				MessageBox (hParentWnd, "Unable to continue due to memory constraints.  Aborted", "Init Coded Object Error", MB_OK);
				return;
			}
		}
		NextLine (&BlockPtr, &LinePtr);
	}
	GlobalUnlock (BlockPtr->hCurBlock);

	if (currentCoded != NULL)	// finish final block
		CompleteThisDecode ();
   
   abortDecodeDoc:;
	DecodeDone ();
}
/* ------------------------------------------------------------------------
 *	Encode a file and ask the user for a file to name to store it in
 */
void
EncodeToFile (HWND hParentWnd, char *inFile)
{
   register unsigned long i;
   char fileName[MAXFILENAME];
   OFSTRUCT outFileStruct;
   HFILE hAttachFile;
   TypTextBlock *encodeData;
 
   if ((encodeData = InitTextBlock (hParentWnd)) != NULL)
   {
      CreateStatusArea(hParentWnd);
      strcpy (currentCoded->ident, inFile);	// status window info

      if (Encode (encodeData, inFile, ADD_TO_FILE) != FAIL)
      {
	fileName[0]='\0';		
	if (AskForNewFileName (hParentWnd, fileName, "") != FAIL)
	{
	   if ((hAttachFile = OpenFile ((char far *) fileName, &outFileStruct, OF_CREATE)) < 0)
		MessageBox(hParentWnd, "Unable to open output file", "File Error", MB_OK|MB_ICONSTOP);
	   else
	   {
	      for (i = 0; i < encodeData->numLines; i++)
		  _lwrite (hAttachFile, TextBlockLine (encodeData, i), lstrlen(TextBlockLine (encodeData, i)));
	      _lclose (hAttachFile);
	   }
	}	
      }     
      FreeTextBlock (encodeData);
      DestroyStatusArea ();
   }   
}

/* ------------------------------------------------------------------------
 *	Create a status window a coding block used for status info
 *	the currentCoded block is purely for status in this case
 *	This is used by attach and EncodeFile and ReadFileToTextBlock
 *	to show status in progress
 *	Use UpdateBlockStatus to update/display status
 *	Use DestroyStatusArea to clean up when done
 */
void
CreateStatusArea (HWND hParentWnd)
{
	int x, y, width, height;
                                                                                                             
                                                                                                             
	if ((currentCoded = InitCoded(hParentWnd)) == NULL)
		return;
	currentCoded->sequence = 1;		// status window info

	// Create coding status window
	width = STATUSWIDTH;
	height = STATUSHEIGHT;
	x = (xScreen - width)/2;		// center it
	y = 1;
	hCodedBlockWnd = CreateWindow ("WinVnBlockCoding", "Block Encoding Status",
			WS_OVERLAPPEDWINDOW|WS_MINIMIZEBOX,
			x, y, width, height,
			(HWND)NULL, (HMENU)NULL, hInst, (void far *)NULL);
	
	SetHandleBkBrush( hCodedBlockWnd, hStatusBackgroundBrush);
	ShowWindow (hCodedBlockWnd, SW_SHOWNORMAL);
}
void
DestroyStatusArea ()
{
	DestroyCodedBlock (&currentCoded);
	DestroyWindow (hCodedBlockWnd);
	hCodedBlockWnd = (HWND)NULL;
}
/* ------------------------------------------------------------------------
 *	Encode a file 
 *	Called _after_ the attach dialog, so all globals for this encode
 *	are set  (sorry about the globals...)
 *	Result is encoded file in TextBlock 
 *	Assumes currentCoded is initialized
 */
int 
Encode (TypTextBlock *textBlock, char *fileName, int mode)
{
	HFILE hFile;
        int numRead, lineLen, start;
	register int i;
	unsigned char inBuf[CODE_LINE_LEN];
	unsigned char outLine[MAXINTERNALLINE];
	extern char *NameWithoutPath ();

	CodingState = ATTACH_PROCESSING;

	// Set up encoding map
	switch (EncodingTypeNum)
	{
	case CODE_BASE64:	
		codingTable = base64Table;
		break;
	case CODE_UU:
		codingTable = uuTable;
		break;
	case CODE_XX:
//	Note, XX table is hard coded in declarations for simplicity
		codingTable = xxTable;
		break;
	case CODE_CUSTOM:
		codingTable = UserCodingTable;
		// add 2 table lines
		if (AddEndedLineToTextBlock (textBlock, "table", mode))
			return (FAIL);
		memmove (str, UserCodingTable, 32);
		str[32] = '\0';
		if (AddEndedLineToTextBlock (textBlock, str, mode))
			return (FAIL);
		memmove (str, &UserCodingTable[32], 32);
		if (AddEndedLineToTextBlock (textBlock, str, mode))
			return (FAIL);
		break;
	}

	// Prepare for encoding
	switch (EncodingTypeNum)
	{
	case CODE_UU:
	case CODE_XX:
	case CODE_CUSTOM:
		// these are 3-to-4 encodings with 1st char indicating line len
		// and block starts with 'begin' line and ends with 'end' line
		// add 'begin' line
		NameWithoutPath (str, fileName);
		sprintf (outLine, "begin 755 %s", str);
		if (AddEndedLineToTextBlock (textBlock, outLine, mode))
			return (FAIL);
		currentCoded->numLines++;
	        // set 1st char to appropriate line length value
	        outLine[0] = codingTable[CODE_LINE_LEN];
	        start = 1;	// count includes the count char itself
		break;
	case CODE_BASE64:
		// base64 is a 3-to-4 encoding, with no line len indicator and 
		// no 'begin' or 'end' lines
		start = 0;
	}
	lineLen = CODE_LINE_LEN;

	// open the file and have at it	
        hFile = _lopen (fileName, OF_READ);
    	while (1)
    	{
    		if ((numRead = _lread(hFile, inBuf, lineLen)) == 0)
    			break;
		if (numRead < lineLen)	// last line length value
		{
			if (EncodingTypeNum != CODE_BASE64)
				outLine[0] = codingTable[numRead];
			
			for (i = numRead; i < lineLen; i++)
				inBuf[i] = 0;
		}
    		EncodeLine (outLine, inBuf, start, numRead);
		if (AddEndedLineToTextBlock (textBlock, outLine, mode))
			return (FAIL);
		currentCoded->numLines++;
		currentCoded->numBytes += numRead;
	    	if (currentCoded->numLines % STATUS_UPDATE_FREQ == 0)
	    		UpdateBlockStatus ();
	}    	
	_lclose(hFile);
	switch (EncodingTypeNum)
	{
	case CODE_UU:
	case CODE_XX:
	case CODE_CUSTOM:
	        // add zero length line
	        outLine[0] = codingTable[0];
	        outLine[1] = '\0';
		if (AddEndedLineToTextBlock (textBlock, outLine, mode))
			return (FAIL);
		// add 'end' line
		strcpy (outLine, "end");
		if (AddEndedLineToTextBlock (textBlock, outLine, mode))
			return (FAIL);
		currentCoded->numLines+=2;
        }
	UpdateBlockStatus ();
	
	CodingState = INACTIVE;
	return (SUCCESS);
}
	
/* ------------------------------------------------------------------------
 *	Encode num chars from line
 *	Put data in outLine starting at start
 */
int 
EncodeLine (unsigned char *outLine, unsigned char *line, int start, int num)
{
	register int i, j;
	
	for (j = start, i = 0; i < num; j+=4, i+=3)
		EncodeUnit (&outLine[j], &line[i], (i+3 > num) ? num - i : 3);

	outLine[j] = '\0';
	return (j);
}
	
void
EncodeUnit (unsigned char *out, unsigned char *in, int num)
{
       	out[0] = codingTable[((in[0]>>2) & 63)];
	out[1] = codingTable[(((in[0]<<4)|(in[1]>>4)) & 63)];
	if (num == 1)
	{
		if (EncodingTypeNum == CODE_BASE64)
		    strcpy (&out[2], "==");
		return;
	}

	out[2] = codingTable[(((in[1]<<2)|(in[2]>>6)) & 63)];
	if (num == 2)
	{
		if (EncodingTypeNum == CODE_BASE64)
		    strcpy (&out[3], "=");
		return;
	}

	out[3] = codingTable[(in[2] & 63)];
}
/* ------------------------------------------------------------------------
 *	Init for decoding
 * 	Initialize the coding tables
 *	Init threadList:  list of pointers to Thread lists 
 *	(one thread for each file being decoded)
 *		
 */
void
DecodeInit ()
{
	register int i;
	int x, y, width, height;
	
#ifdef DEBUG_MEM
	InitDebugLog();
#endif
	
        for (i = 0; i < MAX_DECODE_THREADS; i++)
        	threadList[i] = NULL;

	currentCoded = (TypCoded *) NULL;
	CodingState = DECODE_SKIPPING;
	numDecodeThreads = 0;
	numDumbDecoded = 0;
	currentDecodeThread = -1;	
	thisNumBlocks = -1;
	prevBlockIdent[0] = '\0';
	thisContentEncoding = CODE_UNKNOWN;
	prevContentEncoding = CODE_UNKNOWN;
        customTable[0] = '\0';

#ifdef DEBUG_MEM
	sprintf(debug, "\nInitialized decoding, %d threads", MAX_DECODE_THREADS);
	DebugLog();
#endif
	// Create coding status window (open for duration of coding only)
	width = STATUSWIDTH;
	height = STATUSHEIGHT;
	x = (xScreen - width)/2;		// center it
	y = 1;
	hCodedBlockWnd = CreateWindow ("WinVnBlockCoding", "Block Decoding Status",
			WS_OVERLAPPEDWINDOW|WS_MINIMIZEBOX,
			x, y, width, height,
			(HWND)NULL, (HMENU)NULL, hInst, (void far *)NULL);

	SetHandleBkBrush (hCodedBlockWnd, hStatusBackgroundBrush); 
	ShowWindow (hCodedBlockWnd, SW_SHOWNORMAL);
	UpdateBlockStatus ();
}
/* ------------------------------------------------------------------------
 * At this point, all threads in sequence should have been written and freed,
 * If any threads left, we either had an incomplete decode of some file,
 * or the file was encoded w/out sequence info, and we were waiting to 
 * hopefully receive it all.  If we had an endflag, then write it in order
 * received
 * Accepts handle of parent window so it can display any 'incomplete' messages
 */
void
DecodeDone ()
{
	char mybuf[MAXINTERNALLINE], name[MAXINTERNALLINE];

#ifdef DEBUG_MEM
	sprintf(debug, "\nDone coding,  Wrapping up stray threads");
	DebugLog();
#endif
	name[0]='\0';			// only show name if not verbose mode
	while (numDecodeThreads > 0)	// always work on top of list [0]
	{
		if (!CodingStatusVerbose)
			if (threadList[0]->name[0] != '\0')
				sprintf (name, "%s   ", threadList[0]->name);
			else
				sprintf (name, "%s   ", threadList[0]->ident);

		if (threadList[0]->numBlocks == 0)
		{	// premature end of file
                	WriteDecodeThread (0);
			sprintf (mybuf, "%sDecode is missing parts, written total size %ld", name, threadList[0]->totalBytes);
			UpdateThreadStatus (0, mybuf);
		}
		else if (threadList[0]->contentEncoding != CODE_BASE64 &&
		        !threadList[0]->codedBlockList[threadList[0]->numBlocks-1]->endFlag)
		{
			sprintf (mybuf, "%sEnd never found.  Cancelling", name);
			UpdateThreadStatus (0, mybuf);
		}
		else if (threadList[0]->expectedNumBlocks > 0)
		{	// there IS sequencing info, so we should not be here
                	WriteDecodeThread (0);
			sprintf (mybuf, "%sDecode is missing parts, written total size %ld", name, threadList[0]->totalBytes);
			UpdateThreadStatus (0, mybuf);
		}
		else
                {
			sprintf (mybuf, "%sCompleteness confidence is medium, written total size %ld", name, threadList[0]->totalBytes);
			UpdateThreadStatus (0, mybuf);
                	WriteDecodeThread (0);
		}			
		DestroyThread (0);	// decrements numDecodeThreads
	}
	currentCoded = NULL;
	CodingState = INACTIVE;
        CommDecoding = FALSE;
	DestroyWindow (hCodedBlockWnd);
	hCodedBlockWnd = (HWND)NULL;
}	

/* ------------------------------------------------------------------------
 * 	Initialize a coded object, and initial data space
 *	Returns ptr to object or NULL if failed
 */
TypCoded *
InitCoded (HWND hParentWnd)
{      
	TypCoded *thisCoded = NULL;
	
#ifdef DEBUG_MEM
	sprintf(debug, "\nInitializing a coded object");
	DebugLog();
#endif
	if ((thisCoded = (TypCoded *) GlobalAllocPtr ( GMEM_MOVEABLE, sizeof (TypCoded))) == NULL)
		return ((TypCoded *) NULL);
	
#ifdef DEBUG_MEM
	sprintf(debug, "\nAllocing data size %d", BASE_BLOCK_SIZE);
	DebugLog();
#endif
	if ((thisCoded->data = (char huge *) GlobalAllocPtr (GMEM_MOVEABLE, BASE_BLOCK_SIZE * sizeof (char))) == NULL)
		return ((TypCoded *) NULL);

	thisCoded->maxBytes = BASE_BLOCK_SIZE;
	thisCoded->numBytes = 0;
	thisCoded->numLines = 0;
	thisCoded->sequence = -1;
	thisCoded->seqConfidence = 0;
	thisCoded->name[0] = '\0';
	thisCoded->ident[0] = '\0';
	thisCoded->endFlag = thisCoded->beginFlag = FALSE;
	thisCoded->hParentWnd = hParentWnd;
        bossanovaMIME = FALSE;
	
	return (thisCoded);
}	

/* ------------------------------------------------------------------------
 *	Adds text to a thread coding status block, and refreshes
 *	the thread status window
 */	
void
UpdateThreadStatus (int num, char *str)
{
	AddLineToTextBlock (threadList[num]->statusText, str);
	InvalidateRect (threadList[num]->statusText->hTextWnd, NULL, FALSE);
	UpdateWindow (threadList[num]->statusText->hTextWnd);
}

/* ------------------------------------------------------------------------
 *	Refreshes the block status window
 */	
void
UpdateBlockStatus ()
{
	InvalidateRect (hCodedBlockWnd, NULL, FALSE);
	UpdateWindow (hCodedBlockWnd);
}
/* ------------------------------------------------------------------------
 * 	Destroy a thread
 *	Destroy all blocks in the thread
 *	Remove the thread from the threadList
 */
void
DestroyThread (int num)
{ 
	register int i;
	
#ifdef DEBUG_MEM
	sprintf(debug, "\nDestroying thread %d", num);
	DebugLog();
#endif
	for (i = 0; i < threadList[num]->numBlocks; i++)
		DestroyCodedBlock (&(threadList[num]->codedBlockList[i]));
		
	GlobalFreePtr (threadList[num]);

	for (i = num; i < numDecodeThreads; i++)
		threadList[i] = threadList[i + 1];
	
	numDecodeThreads--;
	if (currentDecodeThread == num)
		currentDecodeThread = max (0, currentDecodeThread - 1);
}
/* ------------------------------------------------------------------------
 * 	Destroy a coded block structure
 *	Free the huge data.  Takes a ptr to a ptr so it can NULL it when done
 */
void
DestroyCodedBlock (TypCoded **thisCoded)
{		
#ifdef DEBUG_MEM
	sprintf(debug, "\nDestroying block sequence %d", (*thisCoded)->sequence);
	DebugLog();
#endif
	GlobalFreePtr ((*thisCoded)->data);
	GlobalFreePtr (*thisCoded);
        *thisCoded = NULL;
}

/* ------------------------------------------------------------------------
 * 	Given a file name, find the associated thread number  
 *	Returns -1 if no thread by that name exists
 */
int
GetThreadByName (char *name)
{
	int i, result;
	
	for (result = -1, i = 0; i < numDecodeThreads && result == -1; i++)
		if (!_stricmp (name, threadList[i]->name))
			result = i;

	return (result);
}
/* ------------------------------------------------------------------------
 * 	Determine if a thread has already been started for this ident
 *	if yes, add decode object to that thread
 *	otherwise, alloc a new thread list structure, add this block to it
 *	At end, currentDecodeThread is set to handle of thread
 */
BOOL
AddToThreadList ()
{
	register int i;
	int num;
	char mybuf[MAXINTERNALLINE];
       	int x, y, width, height;
	
 	if (DumbDecode)
 	{	
		if (numDecodeThreads == 0)
		 	num = -1;
		else 
		 	num = 0;
	} else
 	/* Search existing threads for an ident match.  If
 	 * no match found, or no threads exist yet, add new thread.
 	 */
		for (num = -1, i = 0; i < numDecodeThreads; i++)
			if (!_stricmp (currentCoded->ident, threadList[i]->ident))
				num = i;

	if (num == -1)		/* add a new thread */
	{
		if ((numDecodeThreads + 1) > MAX_DECODE_THREADS)	
		{
			return (FAIL);
/*			GlobalUnlock(decodeObjectHandles);
  			decodeObjectHandles = GlobalRealloc (decodeFileHandles,
  				(numDecodeThreads + LIST_SIZE_INC) * sizeof (HGLOBAL), 
  				GMEM_MOVEABLE);
			threadList = (HGLOBAL *) GlobalLock (decodeFileHandles);
*/
		}

// create a new DecodeThreadStruct object, store its handle in threadList,
#ifdef DEBUG_MEM
		sprintf(debug, "\nCreating new thread ident %s", currentCoded->ident);
		DebugLog();
#endif
 		num = numDecodeThreads;
		numDecodeThreads++;

		threadList[num] = (TypDecodeThread *) GlobalAllocPtr (GMEM_MOVEABLE, sizeof (TypDecodeThread));
		threadList[num]->expectedNumBlocks = 0;
		threadList[num]->numBlocksWritten = 0;
		threadList[num]->numBlocks = 0;
		threadList[num]->totalBytes = 0;
		threadList[num]->dosFileName[0] = '\0';
		if (thisContentEncoding == CODE_CUSTOM)
		    strncpy (threadList[num]->customTable, customTable, CODINGTABLESIZE);
		threadList[num]->contentEncoding = thisContentEncoding;
		strcpy (threadList[num]->ident, currentCoded->ident);
                if (currentCoded->name[0] != '\0')
			strcpy (threadList[num]->name, currentCoded->name);
		else
			threadList[num]->name[0] = '\0';

        	if (CodingStatusVerbose || numStatusTexts == 0)
		{
		   if ((threadList[num]->statusText = InitTextBlock(hCodedBlockWnd)) == NULL)
			MessageBox (hCodedBlockWnd, "Memory allocation error in text block", "Memory Error", MB_OK);
                
                   if (numStatusTexts + 1 > MAX_DECODE_THREADS);
                   	// ?????
                
                   codingStatusText[numStatusTexts++] = threadList[num]->statusText;

		   width = (int)(xScreen * 1/2);
		   x = (numStatusTexts * StatusCharWidth) % (int) (xScreen * 3/4);
		   height = (int) (yScreen * 1/2) - (1 * StatusLineHeight);
		   y = (int) (yScreen * 1/4) + (numStatusTexts * StatusLineHeight) % (int)(yScreen * 3/4);

		   if (CodingStatusVerbose)
	   		sprintf(mybuf, "Decoding Status for file %s", (threadList[num]->name[0]!='\0')?threadList[num]->name:threadList[num]->ident);
		   else
		   	strcpy(mybuf, "Decoding Status");
		   
		   threadList[num]->statusText->hTextWnd =
			CreateWindow ("WinVnCoding", mybuf,
				WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
				x, y, width, height,
				(HWND)NULL, (HMENU)NULL, hInst, (void far *)NULL);

		   if (threadList[num]->statusText->hTextWnd == (HWND)NULL)
		   {
			FreeTextBlock(threadList[num]->statusText);	
			MessageBox (hCodedBlockWnd, "Couldn't create status text window","Window Creation Error", MB_OK);
			return (FAIL);			
		   }
		   SetHandleBkBrush (threadList[num]->statusText->hTextWnd,
				  	hStatusBackgroundBrush);
		   ShowWindow (threadList[num]->statusText->hTextWnd, SW_SHOWNORMAL);
        	   UpdateWindow (threadList[num]->statusText->hTextWnd);
		}
		else
		   threadList[num]->statusText = codingStatusText[0];

	}
	
/* If this new block enlightens us with any useful thread info, update
 * the thread
 */
	if (currentCoded->name[0] != '\0')
		strcpy (threadList[num]->name, currentCoded->name);
//	if (threadList[num]->contentType[0] == '\0')
//		strcpy (threadList[num]->contentType, thisContentType);
	if (threadList[num]->expectedNumBlocks == 0 && thisNumBlocks > 0)
		threadList[num]->expectedNumBlocks = thisNumBlocks;
	
	threadList[num]->totalBytes += currentCoded->numBytes;
/*
 * Now, add this decode object to the thread codedBlockList
 * Insert in correct sequence. 
 * If no sequence info available, add to end of list, but before any end block
 */
	if (DumbDecode)
		InsertBlockInThread(num, currentCoded, threadList[num]->numBlocks);
	else
	{	
	  if (threadList[num]->numBlocks != 0 && currentCoded->sequence == -1)
	  {	/* sequence unknown, add at end or before end block */
		if (currentCoded->beginFlag)
			InsertBlockInThread(num, currentCoded, 0);
		else 
		{
			if (threadList[num]->codedBlockList[threadList[num]->numBlocks - 1]->endFlag)
				InsertBlockInThread(num, currentCoded, threadList[num]->numBlocks - 1);
			else			
				InsertBlockInThread(num, currentCoded, threadList[num]->numBlocks);
		}
	  }
	  else
	  {
		for (i = 0; i < threadList[num]->numBlocks; i++)
		{
			if (threadList[num]->codedBlockList[i]->sequence == -1 || 
			    threadList[num]->codedBlockList[i]->sequence >= currentCoded->sequence)
				break;
		}
		InsertBlockInThread (num, currentCoded, i);
	  }                                                  
	}
	currentDecodeThread = num;	 	
	if (!CodingStatusVerbose)
		return (SUCCESS);
	
	sprintf (mybuf, "Decoded block");
	if (currentCoded->sequence == -1)
	{
		if (currentCoded->beginFlag || currentCoded->endFlag)
		{
			if (currentCoded->beginFlag)
				strcat (mybuf, ", begin block");
			if (currentCoded->endFlag)
				strcat (mybuf, ", end block");
		} else
			strcat (mybuf, " sequence unknown");
	}		
	else
	{
		sprintf (str, " %d", currentCoded->sequence);
		strcat (mybuf, str);
		if (threadList[num]->expectedNumBlocks > 0)
		{
			sprintf (str, " of %d.", threadList[num]->expectedNumBlocks);
			strcat (mybuf, str);
		}
		if (currentCoded->sequence == threadList[num]->numBlocksWritten + 1)
			strcat (mybuf, " In sequence");
		else
			strcat (mybuf, " Out of sequence");
	}
	UpdateThreadStatus (num, mybuf);
	thisNumBlocks = -1;

	return (SUCCESS);
}

/* ------------------------------------------------------------------------
 * 	Insert/delete a block in a thread list
 *	Inserts and deletes will be O(n).  Since n will never be big,
 *	it's not worth the trouble (and space usage) of using a linked list
 *
 *	index should never be -1 (case of unkown sequence handled by caller)
 *	to insert at front of list, index = 0
 *	to insert at end of list, index = threadList[num]->numBlocks
 */
void
InsertBlockInThread (int num, TypCoded *block, int index)
{
	register int i;
	
	for (i = threadList[num]->numBlocks; i > index && i > 0 ; i--)
		threadList[num]->codedBlockList[i] = threadList[num]->codedBlockList[i-1];
 	
#ifdef DEBUG_DECODE
	sprintf(debug, "\nInserting into thread %s at posn %d", threadList[num]->ident, i);
	DebugLog(); 
#endif
 	threadList[num]->codedBlockList[i] = block;
			
	threadList[num]->numBlocks++;		

	return;
}

void
DeleteBlockFromThread (int num, int index)
{
	register int i;
	
	for (i = index; i < threadList[num]->numBlocks; i++)
		threadList[num]->codedBlockList[i] = threadList[num]->codedBlockList[i+1];
 	
#ifdef DEBUG_DECODE
	sprintf(debug, "\nRemoved from thread %s at posn %d", threadList[num]->name, index);
	DebugLog();
#endif
	threadList[num]->numBlocks--;		

	return;
}

/* ------------------------------------------------------------------------
 * 	Complete a decode block
 *	Adds to thread list
 *	Writes block to disk if possible (and any others now in sequence)
 *	If it ends a thread, make sure everything is written, and
 *	free up the thread
 */
BOOL
CompleteThisDecode ()
{
	char mybuf[MAXINTERNALLINE];
	int num, singleBlockDone, foundEnd;

	UpdateBlockStatus ();	// final size displayed
#ifdef DEBUG_DECODE
	sprintf(debug, "\nCompleteThisDecode for file %s, block %d",
			currentCoded->ident, currentCoded->sequence);
	DebugLog();
#endif
	if (currentCoded->numBytes == 0)
	{
#ifdef DEBUG_DECODE
		sprintf(debug, "\nNon-data block.  Discarding it.");
		DebugLog();
#endif
		DestroyCodedBlock (&currentCoded);
		return (SUCCESS);
	}
	
	if (DumbDecode)
	{		
		if (numDumbDecoded == 0 && !currentCoded->beginFlag)
			return (SUCCESS); // Dumb skipping a data block
		if (numDumbDecoded > 0 && currentCoded->beginFlag)
		{		// found next begin without an end
			if (CodingStatusVerbose)
				sprintf (mybuf, "Decode missing end, total size %ld, wrote to file %s", threadList[num]->totalBytes, threadList[num]->dosFileName);
			else
				sprintf (mybuf, "%s   Decode missing end, total size %ld, wrote to file %s", 
					threadList[num]->name, threadList[num]->totalBytes, threadList[num]->dosFileName);
			UpdateThreadStatus (num, mybuf);
			DestroyThread(num);
			numDumbDecoded = 0;
		}
	}					
        else
	// if for some reason (?) we didn't get a begin, but this is block 1
	if (currentCoded->sequence == 1 && 
	    currentCoded->seqConfidence == HIGH && 
	    !currentCoded->beginFlag)
	{
		currentCoded->beginFlag;
		strcpy (currentCoded->name, currentCoded->ident);
	}    
	
	if (AddToThreadList () == FAIL)
		return (FAIL);
	
	prevContentEncoding = thisContentEncoding;
	thisContentEncoding = CODE_UNKNOWN;
	
	num = currentDecodeThread;	
	singleBlockDone = currentCoded->beginFlag && currentCoded->endFlag;

	if (!CodingStatusVerbose && 
	   (threadList[num]->numBlocksWritten == 0 && threadList[num]->numBlocks == 1))
	{
		sprintf (mybuf, "%s   Decode in progress", (threadList[num]->name[0]!='\0')?threadList[num]->name:threadList[num]->ident);
		UpdateThreadStatus (num, mybuf);
	}
	
	if (DumbDecode)
	{
		if (WriteTopBlockToFile (num, &foundEnd) == FAIL)
			return (FAIL);
		numDumbDecoded++;
	}
        else
        {
/* If this block is both begin and end, then go straight to WriteDecodeThread
 * If currentCoded is in sequence, then it was added as block 0 in the 
 * thread's block list, and may have caused other blocks to now be in 
 * sequence as well.  Write all blocks which are in sequence now.
 */
	  if (!singleBlockDone && currentCoded->sequence != -1 && 
	      currentCoded->seqConfidence != 0)
		while (threadList[num]->numBlocks != 0)
		{
			if (threadList[num]->codedBlockList[0]->sequence == threadList[num]->numBlocksWritten)
			{	// skip duplicate block sequence
				DestroyCodedBlock (&(threadList[num]->codedBlockList[0]));
        	                DeleteBlockFromThread (num, 0);
        	                break;
        	        }
			if (threadList[num]->codedBlockList[0]->sequence != threadList[num]->numBlocksWritten + 1)
				break;
#ifdef DEBUG_DECODE
			sprintf(debug, "\nBlock in sequence, writing it, file %s, block %d",
				threadList[num]->name, threadList[num]->codedBlockList[0]->sequence);
			DebugLog();
#endif
			if (WriteTopBlockToFile (num, &foundEnd) == FAIL)
				return (FAIL);
 		}
 	}	
	if (singleBlockDone || (foundEnd && DumbDecode) ||
	    (foundEnd && threadList[num]->expectedNumBlocks > 0 && 
	    threadList[num]->numBlocksWritten >= threadList[num]->expectedNumBlocks))
	{
#ifdef DEBUG_DECODE
		sprintf(debug, "\nThread complete: file %s", threadList[num]->name);
		DebugLog();
#endif
	   	if (WriteDecodeThread(num) == FAIL)
		{
			DestroyThread(num);
	   		return (FAIL);
	   	}

		if (CodingStatusVerbose)
			sprintf (mybuf, "Decode complete, total size %ld, wrote to file %s", threadList[num]->totalBytes, threadList[num]->dosFileName);
		else
			sprintf (mybuf, "%s   Decode complete, total size %ld, wrote to file %s", 
				threadList[num]->name, threadList[num]->totalBytes, threadList[num]->dosFileName);
		UpdateThreadStatus (num, mybuf);

		DestroyThread(num);
		numDumbDecoded = 0;
	}
	return (SUCCESS);
}			
			
/* ------------------------------------------------------------------------
 * 	Write data for the top block of a thread to a file, and remove
 *	the top block from the thread
 */
BOOL
WriteTopBlockToFile (int num, int *endFlag)
{
	char mybuf[MAXINTERNALLINE];

	if (WriteBlockToFile (num, threadList[num]->codedBlockList[0]) == FAIL)
	{
		sprintf (mybuf, "Could not write to file %s", threadList[num]->dosFileName);
		UpdateThreadStatus (num, mybuf);
		DestroyThread(num);
		return (FAIL);
	}
	
	*endFlag = threadList[num]->codedBlockList[0]->endFlag;
	DestroyCodedBlock (&(threadList[num]->codedBlockList[0]));
	DeleteBlockFromThread (num, 0);
	threadList[num]->numBlocksWritten++; 
	return (SUCCESS);
}
	
/* ------------------------------------------------------------------------
 * 	Write data for a thread to a file
 */
BOOL
WriteDecodeThread (int num)
{
	register int i;
	char mybuf[MAXINTERNALLINE];
		
	for (i = 0; i < threadList[num]->numBlocks; i++)
	{
#ifdef DEBUG_DECODE
		sprintf(debug, "\nSaving sequence %d", threadList[num]->codedBlockList[i]->sequence);
		DebugLog();
#endif
		if (WriteBlockToFile (num, threadList[num]->codedBlockList[i]) == FAIL)
		{
			sprintf (mybuf, "Could not write to file %s", threadList[num]->dosFileName);
			UpdateThreadStatus (num, mybuf);
			return (FAIL);
		}
	}
	if (ExecuteDecodedFiles)
		ExecuteDecodedFile(num, threadList[num]->dosFileName);

	return (SUCCESS);
}

/* ------------------------------------------------------------------------
 * 	Write block data to a file
 *	Verify thread file name will work in DOS, and request a new file
 *	name if necessary
 * 	Accepts handle of parent window to allow any necessary dialogs
 */
BOOL
WriteBlockToFile(int num, TypCoded *thisDecode)
{ 
	register unsigned long i;
	unsigned int chunkSize;
	char	actualFileName[MAXFILENAME], mybuf[MAXINTERNALLINE];
        OFSTRUCT outFileStruct;
	HFILE 	hWriteFile;
	UINT	fileMode;

#define	CHUNK_SIZE 65500	/* must not exceed UINT (65534) */		
	
	if (threadList[num]->dosFileName[0] != '\0') {
		fileMode = OF_WRITE;
		if (strchr (threadList[num]->dosFileName, '\\') == NULL)
		{
			strcpy (actualFileName, DecodePathName);
			if (actualFileName[strlen(actualFileName)-1] != '\\')
				strcat(actualFileName, "\\");
			strcat(actualFileName, threadList[num]->dosFileName);
        	}
		else
		   strcpy (actualFileName, threadList[num]->dosFileName);
	}
	else 
	{
	/* First time write to file
	 */
		fileMode = OF_CREATE;
                
                actualFileName[0] = '\0';
		if (threadList[num]->name[0] != '\0')
			strcpy (threadList[num]->dosFileName, threadList[num]->name);
		else if (threadList[num]->ident[0] != '\0')
			strcpy (threadList[num]->dosFileName, threadList[num]->ident);
		if (threadList[num]->dosFileName[0] != '\0')
		{
			strcpy (actualFileName, DecodePathName);
			if (actualFileName[strlen(actualFileName-1)] != '\\')
				strcat(actualFileName, "\\");
			strcat(actualFileName, threadList[num]->dosFileName);
		}

		if (AskForNewFileName (hCodedBlockWnd, actualFileName, DecodePathName) == FAIL)
			return (FAIL);                                  
		strcpy (threadList[num]->dosFileName, actualFileName);
	}
        
        if ((hWriteFile = OpenFile ((char far *) actualFileName, &outFileStruct, fileMode)) < 0)
	{
		MessageBox(hCodedBlockWnd, "Unable to open output file", "File Error", MB_OK|MB_ICONSTOP);
		return (FAIL);
        }
        if (fileMode == OF_WRITE)
        	_llseek (hWriteFile, 0L, 2);	/* append */
        	
	for (i = 0L; i < thisDecode->numBytes; i += chunkSize)
	{
	       	if (i + CHUNK_SIZE > thisDecode->numBytes)
	       		chunkSize = (unsigned int) (thisDecode->numBytes - i);
	       	else
	       		chunkSize = CHUNK_SIZE;

	       	if (_lwrite(hWriteFile, &(thisDecode->data[i]), chunkSize) != chunkSize)
		{
			MessageBox(hCodedBlockWnd, "Error writing to file", "File Error", MB_OK|MB_ICONSTOP);
		       	_lclose(hWriteFile);
			return (FAIL);
	        }
        }

	if (CodingStatusVerbose)
	{
		strcpy (mybuf, "     Wrote block ");
		if (thisDecode->sequence > 0)
		{
			sprintf (str, "%d ", thisDecode->sequence);
			strcat (mybuf, str);
		}
//		sprintf (str, "to file %s", threadList[num]->dosFileName);
//		strcat (mybuf, str);
		UpdateThreadStatus (num, mybuf);
	}
       	_lclose(hWriteFile);
	return (SUCCESS);
}

/* ------------------------------------------------------------------------
 * 	Decode a line in article.  Store in thisDecode
 */
int  
DecodeLine(TypCoded *thisDecode, char *line)
{
	register int i;
	static int table_count;
	char thisBlockName[MAXINTERNALLINE];
     	int mode;

	if (strlen(line) < 2)	/* skip over short/blank lines */
		return (SUCCESS);

	thisDecode->numLines++;
	if (thisDecode->numLines % STATUS_UPDATE_FREQ == 0)
		UpdateBlockStatus ();
	
	switch (CodingState)
	{
	case DECODE_SKIPPING: 
	// encoded UU/XX/Custom data always starts with line 
	// like 'begin <mode> <file-name>'
		if (!strncmp (line, "begin", 5) &&
		    sscanf (line, "%*s %d %s", &mode, thisBlockName) == 2)
                {
#ifdef DEBUG_DECODE
			sprintf(debug, "\nStart of new encoded file %s, mode %d", thisBlockName, mode);
			DebugLog();
#endif
			if (thisBlockName[0] != '\0')
				strcpy (thisDecode->name, strlwr(thisBlockName));
                        else
				strcpy (thisDecode->name, thisDecode->ident);
			                        
			thisDecode->sequence = 1;
			thisDecode->beginFlag = TRUE;
//			thisDecode->mode = mode;

			CodingState = DECODE_PROCESSING;
			return (SUCCESS);
		}     
/* Check for keywords in the non-data lines which may aid in describing
 * the name for this data, the sequence, a custom table, etc
 */
 		else if (!DumbDecode && !_strnicmp(line, "subject:", 8))
 			ParseInfoLine (thisDecode, &line[8], TRUE);
 		else if (!DumbDecode && !_strnicmp(line, "summary:", 8))
			ParseInfoLine (thisDecode, &line[8], FALSE);
 		else if (!DumbDecode && ParseAppSpecificLine (thisDecode, line))
			return (SUCCESS);
 		else if (!_strnicmp(line, "table", 5))
 		{	// next two lines are table
 			CodingState=DECODE_GET_TABLE;
			table_count = 0;
 			return (SUCCESS);
 		}
		 			
/*		else if (ParseMimeHeaderLine(thisDecode, line) == SUCCESS)
			return (SUCCESS);
*/
/* Skipping to find new block of data.
 * Skip until a data line (a line with the correct length) is found
 */		
 		else if (IsDataLine(line))
		{   
#ifdef DEBUG_DECODE
			sprintf(debug, "\nFound start of next data section", line);
			DebugLog();
#endif
                     	CodingState = DECODE_PROCESSING;
                       	// found good line, process it and continue processing
                       	return (DecodeDataLine(thisDecode, line));
		}
#ifdef DEBUG_DECODE
		sprintf(debug, "\nskipped: %s", line);
		DebugLog();
#endif
		return (SUCCESS);		// continue skipping 

	case DECODE_GET_TABLE:
	// This is one of two lines containing table info
		memmove ((customTable + table_count * 32), line, 32);

		if (++table_count == 2)
		{
#ifdef DEBUG_MAP
			strncpy (str, customTable, 64);
			str[64]='\0';
			sprintf(debug, "\nfound table: %s", str);
			DebugLog();
#endif
			CodingState = DECODE_SKIPPING;
			if (i = CreateCodingMap (CodingMap[CODE_CUSTOM], customTable) != -1)
			{
				sprintf (str, "Invalid decoding table in block.  Duplicate character %c.",  i);
				MessageBox (hCodedBlockWnd, str, "Invalid coding table", MB_OK);
				return (FAIL);
			} 
			thisContentEncoding = CODE_CUSTOM;
		}
		return (SUCCESS);
			
	case DECODE_PROCESSING:
/* Some encoders place an END line at end of a section, and length it like 
 * a proper encoded line
 * Make sure we catch it, don't decode it as data, and switch back to skipping
 * Note the only End statement which means End of entire file is lower case
 * "end".  The other tags are ignored, i.e. "END", "End_of_section", etc
 */
		if (!_strnicmp (line, "end", 3) || !IsDataLine(line) ||
		   (bossanovaMIME && !strcmp (&line[2], thisBoundary))) // skip '--'
		{
#ifdef DEBUG_DECODE	
			sprintf(debug, "\nSwitching back to skipping\nskipped: %s", line);
			DebugLog();
#endif
			CodingState = DECODE_SKIPPING;

/* If we are switching back to skipping and haven't really received any
 * data yet (< about a line's worth), then the line which gave us the coding type
 * was bogus (regular text line where first char happened to = encoded line 
 * length, Here's one: 'From: marnold@@cwis.unomaha.edu (Matthew Eldon Arnold)'
 * Ditch all data so far and the current table  and reprocess
 * this line.  Note if we have a custom table, then we trust it.
 */
			if (thisContentEncoding != CODE_CUSTOM && thisDecode->numBytes < 80)
			{
				thisDecode->numBytes = 0;
				thisContentEncoding = CODE_UNKNOWN;
#ifdef DEBUG_DECODE	
				sprintf(debug, "\nBogus table!  Scrapping so far, reprocessing line: %s", line);
				DebugLog();
#endif
				thisDecode->numLines--;
				return (DecodeLine (thisDecode, line));
			}
			strcpy (prevBlockIdent, thisDecode->ident);
		
       			if (!strncmp (line, "end", 3))
			{
				thisDecode->endFlag = TRUE;
				if (thisNumBlocks == -1 && thisDecode->sequence != -1)
					thisNumBlocks = thisDecode->sequence;
#ifdef DEBUG_DECODE
				sprintf(debug, "\nFound end of file %s", thisDecode->ident);
				DebugLog();
#endif
			}					
		        return (END_BLOCK);
		}
              	return (DecodeDataLine(thisDecode, line));
	}
}

/* ------------------------------------------------------------------------
 *  4 to 3 line decoder
 */
BOOL	DecodeDataLine (TypCoded *thisDecode, char *line)
{
    register unsigned int i, j;
    unsigned int decodedCount, numEncoded, numDecoded, checkSum, startNum, stop;
    unsigned char buf[4], outLine[120];
    char *thisMap = CodingMap[thisContentEncoding];
    
    numEncoded = strlen(line);
    switch (thisContentEncoding)
    {
    case CODE_QP:
   	// not implemented yet
	return (FAIL);
    
    case CODE_UU:
    case CODE_XX:
    case CODE_CUSTOM:
	// these are 4-to-3 decodings which have the line lengtrh encoded
	// as the first char (count is # of chars after decoding)
	checkSum = decodedCount = thisMap[line[0]];
	startNum = 1;
	break;
    case CODE_BASE64:
	// base64 is 4-to-3 decoding with nonexplicit line lengths 
	decodedCount = 3 * (numEncoded / 4);
	startNum = 0;
	break;
    }	
    for (i = startNum, numDecoded=0; numDecoded < decodedCount;)
    {
	// Get the next group of four characters 
	// Handle last group of characters in a base64 encoding specially -
	// padding '=' at end means we have fewer than 24 bits after decoding
	// Base64 end scenarios:
	//	'xx==' decodes to 8 bits
	//	'xxx=' decodes to 16 bits
	//	'xxxx' decodes to 24 bits
	//	'x===' can't happen
	stop = min(4, numEncoded - i + 1);
	for (j = 0; j < stop; j++, i++)
	{       
		if (thisContentEncoding == CODE_BASE64 && line[i] == '=')
		{
			buf[j] = 0;
			j--;
			break;
		}
		else
			buf[j] = thisMap[line[i]];
		checkSum += buf[j]; 
	}
	outLine[numDecoded++] = buf[0]<<2|buf[1]>>4;
	if (j == 1) break;
	outLine[numDecoded++] = buf[1]<<4|buf[2]>>2;
	if (j == 2) break;
	outLine[numDecoded++] = buf[2]<<6|buf[3];
    }
    if (AddDataToBlock (currentCoded, outLine, numDecoded) == FAIL)
	return (FAIL);

/*	if (checkSum%64 != line[i])
	{
		printf("\nChecksum error: %d vs. %d", checkSum%64, line[i]);
		break;
	} else
		printf("\nChecksum ok: %c vs. %c", checkSum%64, line[i]);
*/
    return (SUCCESS);
}

/* ------------------------------------------------------------------------
 * AddDataToBlock is called on a line-by-line basis.
 * dataLen should rarely be > 100 bytes
 * The data is type huge, but it's just pointing to a string of chars, so
 * the size does not have to be a power of 2
 */
BOOL
AddDataToBlock (TypCoded *thisDecode, char *newData, unsigned int dataLen)
{
	if (thisDecode->numBytes + (unsigned long) dataLen > thisDecode->maxBytes)
	{ 
	        thisDecode->maxBytes += BASE_BLOCK_SIZE;
#ifdef DEBUG_MEM
		sprintf(debug, "\nReallocing data to %ld", thisDecode->maxBytes);
		DebugLog();
#endif
		if ((thisDecode->data = (char huge *) GlobalReAllocPtr (thisDecode->data, thisDecode->maxBytes, GMEM_MOVEABLE)) == NULL)	
			return (FALSE);
	}
	memmove (thisDecode->data + thisDecode->numBytes, newData, dataLen);
	thisDecode->numBytes += dataLen;
        return (SUCCESS);
}	 
		
/* ------------------------------------------------------------------------
 * 	Determine if the line is a data line
 */
BOOL
IsDataLine (char *line)
{
	if (thisContentEncoding == CODE_UNKNOWN)
	{
/* If this block has same ident as prev block, use same decode table
 * If this block has same ident as any existing threads, use same decode table
 * as matching thread
 * Otherwise, test line for UU, XX, or Base64, returning if find a fit
 */			
		if (currentCoded->ident[0] != '\0' && 
		    !strcmp (currentCoded->ident, prevBlockIdent))
		{
			thisContentEncoding = prevContentEncoding;
#ifdef DEBUG_MAP
			strcpy (debug, "\nUsing same table as prev decode block");
			DebugLog();
#endif
		}
		else
			if ((thisContentEncoding = ThreadTable (customTable, currentCoded->ident)) != CODE_UNKNOWN)
			{
			   if (thisContentEncoding == CODE_CUSTOM)
				CreateCodingMap (CodingMap[CODE_CUSTOM], customTable);
#ifdef DEBUG_MAP
				strcpy (debug, "\nUsing stored thread table");
				DebugLog();
#endif
			}
			else
			{				
				thisContentEncoding = CODE_UU;
				if (TestDataLine (line))
				{
#ifdef DEBUG_MAP
					sprintf (debug, "\nUsing UU table: %s", line);
					DebugLog();
#endif
					return (TRUE);
				}
				thisContentEncoding = CODE_XX;
				if (TestDataLine (line))
				{
#ifdef DEBUG_MAP
					sprintf (debug, "\nUsing XX table: %s", line);
					DebugLog();
#endif
					return (TRUE);
				}
				thisContentEncoding = CODE_BASE64;
				if (TestDataLine (line))
				{
#ifdef DEBUG_MAP
					sprintf (debug, "\nUsing Base64 table: %s", line);
					DebugLog();
#endif
					return (TRUE);
				}
				thisContentEncoding = CODE_UNKNOWN;
				return (FALSE);
			}
	}
	return (TestDataLine(line));
}
		
BOOL
TestDataLine (char *line)
{		
   unsigned int expectedLineLen, dataLen, lineLen;
   register unsigned int i;
   char *thisMap;
   
   thisMap = CodingMap[thisContentEncoding];
   lineLen = strlen(line);
   switch (thisContentEncoding)
   {
   case CODE_UU:
   case CODE_XX:
   case CODE_CUSTOM:
/*
 * The first char of a good data UU/XX/Custom line is the encoded character
 * count for the line.  
 * example M = ascii 77, decodes to 45.  This line will decode to 45 characters,
 * so the # encoded chars should be (int)(4 * ((n+2) / 3)) - (round up and re-encode)
 * So if line[0] is 'M', there should be 60 characters on the line
 * The count does not include the first count char, so the expected line 
 * length is count+1 (i.e. 61). 
 * If the line length is actually 2 + count, then we have a checksum (?)
 */ 
	expectedLineLen = 4 * ((thisMap[line[0]] + 2) / 3) + 1;

	if (lineLen < expectedLineLen)
 		return (FALSE);
/*
 * Count the number of encoded data characters on the line (up to any whitespace)
 * (Some encoders pad with spaces at the end).  Don't forget, a space may be
 * a valid encoded char, so a proper line may very well end with a space.
 * Don't blindly remove all trailing white space!
 */		
 	for (dataLen = lineLen; 
 	     isspace (line[dataLen-1]) && dataLen > 0 && dataLen != expectedLineLen; 
 	     --dataLen);
 	
 	// May have a checksum character, so allow for one extra char
 	if (expectedLineLen != dataLen && expectedLineLen+1 != dataLen)
 		return (FALSE);
 	
 	line[dataLen] = '\0';	// permanently chop off the white space
 	// It's the right length, now check content for match w/ table
	for (i = 0; i < expectedLineLen; i++)
		if (thisMap[line[i]] == UNUSED)
			return (FALSE);
	return (TRUE);

    case CODE_BASE64:
	// permanently remove all trailing space
	while (lineLen > 0 && isspace (line[lineLen-1]))
		line[--lineLen] = '\0';
 	
 	// for base 64, just check if all chars in coding map (allow pad '=')
	for (i = 0; i < lineLen; i++)
		if (thisMap[line[i]] == UNUSED && line[i] != '=')
			return (FALSE);
	return (TRUE);

    case CODE_QP:
	// not implemented yet
    default:
        return (FALSE);
    }        
}                               

/* ------------------------------------------------------------------------
 * 	Mappings for encoding/decoding
 */
int
CreateCodingMap(char *map, char *table)
{
	register unsigned int i;
	
	for (i=0; i < 128; i++)
		map[i] = UNUSED;
	
	// if you see a table[i] character, decode it to i
	// i.e. UU, table[1] is a '!'.  So a '!' should decode to 1
	
	for (i = 0; i < CODINGTABLESIZE; i++)
        	if (map[table[i]] != UNUSED)
        		return (table[i]);	// table[i] is duplicate 
		else        		
			map[table[i]]=i;
	
#ifdef DEBUG_MAP2
	sprintf(debug, "\nCoding Map:\n"); DebugLog();
	for (i=0; i < 128; i++)
	{
		sprintf(debug, "%d ",map[i]);
		DebugLog();
	} 
#endif
	return (-1);			
}

int   
ThreadTable (char *dest, char *ident)
{
	int num;
	register int i;
	
	for (num = -1, i = 0; i < numDecodeThreads; i++)
		if (!_stricmp (ident, threadList[i]->ident))
			num = i;

	if (num == -1)		// no thread by that name 
		return (CODE_UNKNOWN);
	else
	{
		if (threadList[num]->contentEncoding == CODE_CUSTOM)
			strncpy (dest, threadList[num]->customTable, CODINGTABLESIZE);
		return (threadList[num]->contentEncoding);
	}
}
#if 0
void
CreateUUTable()
{
	register unsigned int i;

	for (i=0; i < CODINGTABLESIZE; i++)
		uuTable[i] = i + 32;
	
	uuTable[0] = 96;		/* set space to map to back-quote */	
}
#endif

/* ------------------------------------------------------------------------
 *	Search through a subject: or BEGIN line 
 *	(for a Subject: line,  "Subject:" has already been removed).
 *
 *	Attempt to obtain a block number, and a total # of blocks
 *	Generate an identifier based on the subject, preferably based on the
 *	actual file name.  If guessIdent is TRUE, choose an ident even if
 *	it doesn't look like a fileName.  If guessIdent is FALSE, only fill
 *	ident if we see a fileName (word containing a dot).
 *
 *	Note: at this point, we haven't seen any MIME header, and we're not
 *	really expecting one.  So now is the time to get what we can
 *
 *	Common Subject styles which this function deals with:
 *	filename.ext 1/2 comment
 *	filename.ext(1/2)comment
 *	filename.ext (1/2) comment
 *	filename.ext [1/2] comment
 *	filename.ext 1 of 2 comment
 *	filename.ext part 1 of 2 comment
 *	filename.ext 1 of 2 comment
 *	This is part 1/2 of filename.ext
 *
 *	Also handles the following common BEGIN lines:
 *	BEGIN --- CUT HERE --- Cut Here --- cut here --- abcd.efg n/N
 * 	BEGIN --- CUT HERE --- Cut Here --- cut here --- abcd.efg
 *	BEGIN-------cut here-------CUT HERE-------PART n               // This is TIN
 *	BEGIN -- CUT HERE -- cut here -- abcd.efg part n of N
 *	BEGIN---CUT HERE---BEGIN---CUT HERE---BEGIN PART n/N
 *
 *	For subject lines beginning with the comment, or free text, the
 *	name is harder to guess.  In these cases, prefer a word containing
 *	a dot (filename.ext), or if none, just take the first word.  Scan
 *	any existing threads to see if any of their identifiers matches.
 *	For example,
 *	another encoded file: filename.ext (1/2)
 *	testing encoded files (1/2)	i.e. ident would be testing
 *
 * 	Known to be incorrectly handled by this function:
 *	filename.ext 3.4 (1/2)		i.e. name w/ version number
 *	filename.ext 001		i.e. sequence w/ no # parts
 *	filename.ext1			i.e. part number appended to file name
 */ 
#define SEEK_PARTNUM	1
#define SEEK_NUMPARTS	2
#define IGNORE_NUMBERS	3
void
ParseInfoLine (TypCoded *thisDecode, char *line, BOOL guessIdent)
{
	char tok[MAXINTERNALLINE], next[MAXINTERNALLINE];
	char guessName[MAXINTERNALLINE];
	char *ptr, *thisTok, *tmp;
	int  numberMode;
	
	numberMode = SEEK_PARTNUM;
	ptr = line;
        
	guessName[0] = '\0';
        while (*ptr)
        {
        	if (ReadSubjectToken (thisTok = tok, &ptr) == NULL)
        		break;
        
//	NUMBERS
		if (numberMode != IGNORE_NUMBERS)
		   if (isnumber (thisTok))
		   {
			if (numberMode == SEEK_PARTNUM)
			{
				thisDecode->sequence = atoi (thisTok);
				thisDecode->seqConfidence = LOW;

				if (ReadSubjectToken (next, &ptr) == NULL)
					break;
				if (!_stricmp (next, "/") ||
				    !_stricmp (next, "of"))
				{
					numberMode = SEEK_NUMPARTS;		
					continue;
				}
				else	// no numParts immediately following
					thisTok = next;	// process the next tok
			}
			else	// numberMode == SEEK_NUMPARTS
			{
				thisNumBlocks = atoi (tok);
				numberMode = IGNORE_NUMBERS;
				thisDecode->seqConfidence = HIGH;
        			continue;
			}
		   }
		   else if (numberMode == SEEK_NUMPARTS)
//		   We had '# of non#', a red herring, seek again
			numberMode == SEEK_PARTNUM;
			
//	WORDS
		// Prefer a word containing a dot for the ident (but not a period
		// at the end of a sentence
	        if ((tmp = strchr (thisTok, '.')) && isalpha(*(tmp+1)))
			strcpy (thisDecode->ident, thisTok);
	        else if (guessIdent)
	        {
	        	// skip if already have name w/ confidence
	        	if (thisDecode->ident[0] == '\0')
			{
			// check if any threads have this token as name
        		   if (SearchThreadNames (thisTok))
        			strcpy (thisDecode->ident, thisTok);

			// Save first word found for thread name
			// prefer ident not starting with 're'
	        	   else if (guessName[0] == '\0' || !_strnicmp(guessName, "re", 2))
	        		strcpy (guessName, thisTok);
	        	}
	        }
	}
	if (guessIdent && thisDecode->ident[0] == '\0')
		strcpy (thisDecode->ident, guessName);

#ifdef DEBUG_DECODE
	sprintf(debug, "\nSubject header: name %s, part number %d (confidence %d), num parts %d", 
		thisDecode->ident, thisDecode->sequence, thisDecode->seqConfidence, thisNumBlocks);
	DebugLog();
#endif
}

char *
ReadSubjectToken (char *dest, char **ptr)
{
	int len;
	register int i;
	char *str = *ptr;
	
	if (*str == '\0')
		return ('\0');
		
	len = strcspn (str, " ()[]\\/:,\"'`{};\n\r");

/*	Ignore (skip) all of these delimiters except '/'
 *	Return '/' as a token of length 1
 */                      
	if (len == 0)
		if (*str == '/')
			len = 1;
		else
		{	// skip this delimiter
			(*ptr)++;
			return (ReadSubjectToken (dest, ptr));
		}

        for (i = 0; i < len; i++)
        	dest[i] = tolower(str[i]);
        dest[len] = '\0';

	*ptr = str + len;	// increment line ptr	
}

BOOL
SearchThreadNames (char *name)
{
	register int i;
	int len;
	
	len = strlen (name);

	for (i = 0; i < numDecodeThreads; i++)
		if (!_strnicmp (name, threadList[i]->ident, len))
			return (TRUE);

	return (FALSE);
}
        
/* ParseAppSpecificLine
 * returns TRUE if it handles the line, else FALSE
 *
 * Example application lines handled by this function:
 * 	section N of uuencode 5.10 of file abcd.efg   by R.E.M.
 *	section n/N   file abcd.efg   [ Wincode v2.3 ]
 *	section n/N abcd.efg  [EnUU 2.1]
 *      abcd.efg    section  n/N   UUXFER ver 2.0 by David M. Read
 *	POST V2.0.0 abcd.efg (Part n/N)
 *	part=n
 *	file=abc.def
 *	pfile=xyz.abc 
 *	Archive-name: fileident/part0n		// no extension included for filename
 */
BOOL
ParseAppSpecificLine (TypCoded *thisDecode, char *line)
{
	char *orig;
	int seq, num;
	char name[MAXINTERNALLINE];
	char copy[MAXINTERNALLINE];
	
   	for (orig = line; *orig && isspace(*orig); orig++);	// skip leading spaces
	
	if (*orig == '\0')        	// blank line (all spaces)
        	return (SUCCESS);
	
	/* note most application keywords will be compared case-sensitively
	 * (using orig), but then all following info-gathering will be 
	 * case-insensitive (using copy) 
	 */
	if (sscanf(orig, "part=%d", &seq) == 1)
	{
		thisDecode->sequence = seq;
		if (!thisDecode->seqConfidence)
			thisDecode->seqConfidence = LOW; // still don't have number of parts
		return (TRUE);
	}
	if (sscanf(orig, "file=%s", name) == 1)
	{
		strcpy (thisDecode->ident, name);
		return (TRUE);
	}
	if (sscanf(orig, "pfile=%s", name) == 1)
	{
		strcpy (thisDecode->ident, name);
		return (TRUE);
	}
	if (sscanf(orig, "Archive-name: %[^/]/part%d", name, &seq) == 2)		// %[^/] reads a string up to a slash
	{
		strcpy (thisDecode->ident, name);
		thisDecode->sequence = seq;
		if (!thisDecode->seqConfidence)
			thisDecode->seqConfidence = LOW; // still don't have number of parts
		return (TRUE);
	}

	strncpy (copy, orig, MAXINTERNALLINE-1);
	copy[MAXINTERNALLINE-1] = '\0';
	AnsiLowerBuff ((LPSTR)copy, (UINT)strlen (copy));	// make a lower-case copy

	if (strstr(orig, "R.E.M."))
	{
		if (sscanf (copy, "section %d of uuencode %*f of file %s", &seq, name) == 2)
		{	
			thisDecode->sequence = seq;
			if (!thisDecode->seqConfidence)
				thisDecode->seqConfidence = LOW; // still don't have number of parts
				
			strcpy (thisDecode->ident, name);
			thisContentEncoding = CODE_UU;
			return (TRUE);
		}
	}
	if (strstr(orig, "UUXFER"))
	{
		if (sscanf (copy, "%s section %d/%d", name, &seq, &num) == 3)
		{	
			thisDecode->sequence = seq;
			thisNumBlocks = num;
			thisDecode->seqConfidence = HIGH; 
			strcpy (thisDecode->ident, name);
			thisContentEncoding = CODE_UU;
			return (TRUE);
		}
	}
	if (!strncmp(orig, "BEGIN", 5))
	{
		ParseInfoLine(thisDecode, copy, FALSE);
		return (TRUE);
	}
	if (strstr(orig, "Wincode"))
	{
		if (sscanf (copy, "section %d/%d   file %s", &seq, &num, name) == 3)
		{
			thisDecode->sequence = seq;
			thisNumBlocks = num;
			thisDecode->seqConfidence = HIGH; 
			strcpy (thisDecode->ident, name);
			return (TRUE);
		}
	}
 	if (strstr(orig, "EnUU"))
	{
		if (sscanf (copy, "section %d/%d %s", &seq, &num, name) == 3)
		{
			thisDecode->sequence = seq;
			thisNumBlocks = num;
			thisDecode->seqConfidence = HIGH; 
			strcpy (thisDecode->ident, name);
			thisContentEncoding = CODE_UU;
			return (TRUE);
		}
	}
	if (!strncmp(orig, "POST", 4))
	{
		if (sscanf (copy, "POST V%*s %s (Part %d/%d)", name, &seq, &num) == 3)
		{
			thisDecode->sequence = seq;
			thisNumBlocks = num;
			thisDecode->seqConfidence = HIGH;
			strcpy (thisDecode->ident, name);
			return (TRUE);
		}
	}

	return (FALSE);
}
//-------------------------------------------------------------------------
void
ExecuteDecodedFile(int num, char *fileName)
{
	char ext[4];		// these are DOS specific...
//	char name[9];
	char association[MAXINTERNALLINE];
	char execute[MAXINTERNALLINE+MAXFILENAME];
	int len;
	unsigned int err;
	char *src, *dest, *src2;
	char mybuf[MAXINTERNALLINE];

#ifndef WIN32
	GetFileExtension (ext, fileName);

        if (!_stricmp (ext, "exe"))
        	strcpy (execute, fileName);
	else
	// example association: doc=C:\WIN\WINWORD\winword.exe ^.doc

	if ((len = GetProfileString ("Extensions", (LPCSTR)ext, "", association, MAXINTERNALLINE)) != 0)
	{
	  for (src = association, dest = execute; *src; src++)
	  {
		if (*src == '^')
		{
 		   for (src2 = fileName; *src2 && *src2 != '.'; src2++)
		   	*(dest++) = *src2;
		}
		else
		   *(dest++) = *src;
	  }
	  *dest = '\0';
        }
        else
        {
		sprintf(mybuf, "Cannot execute - no association for file type %s", ext);
		UpdateThreadStatus (num, mybuf);
		return;
	}

#else
        FindExecutable (fileName, ".", association);
	wsprintf (execute, "%s %s", association, fileName);
#endif
	if ((err = WinExec (execute, SW_SHOWNORMAL)) < 32)
	{
		sprintf(mybuf, "Error executing %s, error %u", execute, err);
		UpdateThreadStatus (num, mybuf);
	}
	else if (CodingStatusVerbose)
	{
		sprintf(mybuf, "Executed %s", execute);
		UpdateThreadStatus (num, mybuf);
	}
}
@


1.9
log
@FindExecutable fix for NT
@
text
@d37 1
a37 1
 * $Id: wvcoding.c 1.8 1994/08/11 20:16:25 rushing Exp $
d39 3
d116 1
a116 1
void	DestroyCodedBlock (TypCoded *thisCoded);
d220 1
a220 1
				return;
d351 1
a351 1
	DestroyCodedBlock (currentCoded);
d625 1
a625 1
	TypCoded *thisCoded;
d691 1
a691 1
		DestroyCodedBlock (threadList[num]->codedBlockList[i]);
d704 1
a704 1
 *	Free the huge data
d707 1
a707 1
DestroyCodedBlock (TypCoded *thisCoded)
a708 1
	
d710 1
a710 1
	sprintf(debug, "\nDestroying block sequence %d", thisCoded->sequence);
d713 3
a715 2
	GlobalFreePtr (thisCoded->data);
	GlobalFreePtr (thisCoded);
d844 3
a846 3
	if (threadList[num]->contentType[0] == '\0')
		strcpy (threadList[num]->contentType, thisContentType);
	if (threadList[num]->expectedNumBlocks == 0)
d988 1
a988 1
		DestroyCodedBlock (currentCoded);
d1053 1
a1053 1
				DestroyCodedBlock (threadList[num]->codedBlockList[0]);
d1113 1
a1113 1
	DestroyCodedBlock (threadList[num]->codedBlockList[0]);
d1382 2
d1604 1
a1604 1
	for (i = 0; i < dataLen; i++)
d1869 1
d1873 2
a1874 1
 *	pfile=xyz.abc
d1879 1
a1879 1
	char *orig, *ptr;
a1901 3
		if ((ptr = strpbrk(name, " \n\t\r\f")))	
			*ptr = '\0';

d1907 5
a1911 3
		if ((ptr = strpbrk(name, " \n\t\r\f")))	
			*ptr = '\0';

d1913 3
d1932 13
d1972 1
d1998 2
a1999 1
	int len, err;
d2038 1
a2038 1
		sprintf(mybuf, "Error executing %s, error %d", execute, err);
@


1.8
log
@bug fix
@
text
@d37 1
a37 1
 * $Id: wvcoding.c 1.6 1994/07/25 20:05:13 jcooper Exp $
d39 3
d1977 1
a1977 1
	char mybuf[MAXINTERNALLINE];	
d1979 1
d1986 1
a1986 1
		
d2001 1
a2001 1
        else 
d2004 1
a2004 1
		UpdateThreadStatus (num, mybuf);			
d2008 4
d2015 1
a2015 1
		UpdateThreadStatus (num, mybuf);			
d2020 2
a2021 2
		UpdateThreadStatus (num, mybuf);			
	}	
@


1.7
log
@Enhancements to Mime and article encoding/encoding
@
text
@a1814 1
	
a1853 1

d1874 5
a1878 2
   	for (orig = line; *orig && isspace(*orig); *orig++);	// skip leading spaces

@


1.6
log
@execution of decoded files
@
text
@d30 1
a30 1
 * Author: John S. Cooper (jcoop@@apl.com)                           *
a34 1
 * Please do not use this code for profit or damage                 *
d37 1
a37 1
 * $Id: wvcoding.c 1.5 1994/05/23 18:36:00 jcooper Exp $
d39 3
d101 1
a101 1
char 	thisBlockIdent[MAXFILENAME], prevBlockIdent[MAXFILENAME];
d117 2
a118 2
void	ParseSubjectLine (TypCoded *thisDecode, char *line);
void	ParseREMLine (TypCoded *thisDecode, char *line);
d139 1
a139 1
char 	debug[255];
d294 1
a294 1
	if (AskForNewFileName (hParentWnd, fileName, ""))
a529 1
	thisBlockIdent[0] = '\0';
d599 1
a599 1
			sprintf (mybuf, "%sCompleteness confidence is pretty mediocre, written total size %ld", name, threadList[0]->totalBytes);
d607 1
d816 1
a816 1
      
d907 1
d1042 1
a1042 1
	      currentCoded->seqConfidence == HIGH)
a1082 3
		if (ExecuteDecodedFiles)
			ExecuteDecodedFile(num, threadList[num]->dosFileName);

d1135 3
d1245 1
a1245 1
	if (strlen(line) < 3)	/* skip over short/blank lines */
d1280 5
a1284 3
 			ParseSubjectLine (thisDecode, &line[8]);
 		else if (!DumbDecode && strstr(line, "R.E.M."))
 			ParseREMLine (thisDecode, line);
a1371 1
			thisBlockIdent[0] = '\0';			
d1683 3
a1685 2
 *	Search through a subject line (the Subject: part has already been 
 *	removed).
d1687 5
a1691 2
 *	Generate an identifier based on the subejct, preferably based on the
 *	actual file name
d1695 1
a1695 1
 *	Common styles which this function deals with:
d1705 7
d1729 1
a1729 1
ParseSubjectLine (TypCoded *thisDecode, char *line)
a1738 2
	thisNumBlocks = -1;
	thisBlockIdent[0] = '\0';
d1782 2
a1783 1
	        else
d1796 1
d1798 1
a1798 2

	if (thisDecode->ident[0] == '\0')
d1823 1
a1823 1
 */
d1856 14
a1869 3
// example: "section N of uuencode 5.10 of file abcd.gef   by R.E.M."
void
ParseREMLine (TypCoded *thisDecode, char *line)
d1871 2
a1872 2
	char *ptr;
	int seq;
d1874 1
d1876 84
a1959 5
	AnsiLowerBuff ((LPSTR)line, (UINT)strlen(line));
   	ptr = line;
   	while (*ptr && isspace(*ptr)) *ptr++;
	if (sscanf (ptr, "section %d of uuencode %*f of file %s", &seq, name) != 2)
		return;
d1961 1
a1961 5
	thisDecode->sequence = seq;
	if (!thisDecode->seqConfidence)
		thisDecode->seqConfidence = LOW; // still don't have number of parts
		
	strcpy (thisDecode->ident, name);
d1998 1
a1998 1
		sprintf(mybuf, "No association for file type %s", ext);
d2005 1
a2005 1
		sprintf(mybuf, "Error running %s, error %d", execute, err);
d2010 1
a2010 1
		sprintf(mybuf, "Sucessfully executed %s", execute);
@


1.5
log
@new attach code, session [dis]connect
@
text
@d38 1
a38 1
 * $Id: wvcoding.c 1.4 1994/03/01 19:14:22 rushing Exp $
d40 3
d116 1
d131 2
d278 10
a287 8
	register unsigned long i;
	char fileName[MAXFILENAME];
        OFSTRUCT outFileStruct;
        HFILE hAttachFile;
        TypTextBlock *encodeData;

	if ((encodeData = InitTextBlock (hParentWnd)) == NULL)
		return;
d289 2
a290 6
	CreateStatusArea(hParentWnd);
        strcpy (currentCoded->ident, inFile);	// status window info

	if (Encode (encodeData, inFile, ADD_TO_FILE) == FAIL)
		return;

d292 1
a292 4
	if (AskForNewFileName (hParentWnd, fileName, "") == FAIL)
		return;
		
	if ((hAttachFile = OpenFile ((char far *) fileName, &outFileStruct, OF_CREATE)) < 0)
d294 1
d296 11
a306 9
		return;
       	}
	for (i = 0; i < encodeData->numLines; i++)
		_lwrite (hAttachFile, TextBlockLine (encodeData, i), lstrlen(TextBlockLine (encodeData, i)));
	
	_lclose (hAttachFile);
	FreeTextBlock (encodeData);
	DestroyCodedBlock (currentCoded);	// free status blk (created in Encode)
	DestroyWindow (hCodedBlockWnd);		// close status window
d321 2
a322 1

d1080 3
d1159 6
a1164 1
		   sprintf (actualFileName, "%s\\%s", DecodePathName, threadList[num]->dosFileName);
d1180 6
a1185 1
			sprintf (actualFileName, "%s\\%s", DecodePathName, threadList[num]->dosFileName);
d1188 1
a1188 1
			return (FAIL);
d1278 2
d1841 21
a1862 1
#ifdef OLD
d1864 1
a1864 1
OldParseSubjectLine (TypCoded *thisDecode, char *line)
d1866 23
a1888 50
	char *beg, *end;
	char temp[30];
	int len, thisBlockPart;
	
	beg = line;
	/* skip to beginning of filename */
	while (!isalnum(*beg)) beg++;	
	
	if ((end = strpbrk (beg, " ([:"))) == NULL)
	{
		thisBlockPart = -1;
		len = min (strlen (beg), MAXFILENAME-1);
		strncpy (thisBlockName, beg, len);
		thisBlockName[len] = '\0';
	}		
	else 
	{ 
		len = min (end-beg, MAXFILENAME-1);
		strncpy (thisBlockName, beg,len);
		thisBlockName[len] = '\0';
		RemoveTrailingWhiteSpace(thisBlockName);

		/* skip delimiter, then skip to part number */
		for (beg = end + 1; *beg != '\0' && !isdigit(*beg); beg++);
		if (*beg == '\0')
			thisBlockPart = -1;
                else {
                	/* find end of number, then put number in temp */
			for (end = beg + 1;  *end != '\0' && isdigit(*end); end++);
			strncpy (temp, beg, end-beg);
			temp[end-beg] = '\0';
			
			if ((thisBlockPart = atoi (temp)) == 0)
				thisBlockPart = -1;
			
			/* skip to next number (num parts) */
			for (beg = end + 1; *beg != '\0' && !isdigit(*beg); beg++);
			if (*beg == '\0')
				thisNumBlocks = -1;
			else {
	                	/* find end of number, then put number in temp */
				for (end = beg + 1;  *end != '\0' && isdigit(*end); end++);
				strncpy (temp, beg, end-beg);
				temp[end-beg] = '\0';

				if ((thisNumBlocks = atoi (temp)) == 0)
					thisNumBlocks = -1;
				else if (thisBlockPart > thisNumBlocks)
					thisBlockPart = -1;
			}
d1890 10
a1900 1
	strcpy (thisDecode->name, thisBlockName);
d1902 10
a1911 3
	thisDecode->sequence = thisBlockPart;
	SendMessage (hCodedBlockWnd, WM_PAINT, STATUS_NAME|STATUS_SEQUENCE, (LPARAM)NULL);
	InvalidateRect (hCodedBlockWnd, NULL, FALSE);
a1912 2

#endif
@


1.4
log
@ifdef'd the DEBUG info
@
text
@d38 1
a38 1
 * $Id: wvcoding.c 1.3 1994/02/24 21:28:04 jcoop Exp $
d40 3
d284 2
a285 1
	Encode (encodeData, inFile, ADD_TO_FILE);
d349 1
a349 1
void
d362 1
a362 1
	switch (EncodingType)
d377 2
a378 1
		AddEndedLineToTextBlock (textBlock, "table", mode);
d381 2
a382 1
		AddEndedLineToTextBlock (textBlock, str, mode);
d384 2
a385 1
		AddEndedLineToTextBlock (textBlock, str, mode);
d390 1
a390 1
	switch (EncodingType)
d400 2
a401 1
		AddEndedLineToTextBlock (textBlock, outLine, mode);
d422 1
a422 1
			if (EncodingType != CODE_BASE64)
d429 2
a430 1
		AddEndedLineToTextBlock (textBlock, outLine, mode);
d437 1
a437 1
	switch (EncodingType)
d445 2
a446 1
		AddEndedLineToTextBlock (textBlock, outLine, mode);
d449 2
a450 1
		AddEndedLineToTextBlock (textBlock, outLine, mode);
d456 1
d482 1
a482 1
		if (EncodingType == CODE_BASE64)
d490 1
a490 1
		if (EncodingType == CODE_BASE64)
@


1.3
log
@jcoop changes.
@
text
@d38 1
a38 1
 * $Id: wvcoding.c 1.2 1994/02/09 18:01:08 cnolan Exp $
d40 3
d58 2
d63 1
@


1.2
log
@cnolan 90.2 changes
@
text
@d38 1
a38 1
 * $Id: wvcoding.c 1.1 1994/01/18 09:54:20 jcoop Exp $
d40 3
d55 3
a57 3
//#define DEBUG_MAP
//#define DEBUG_MEM
//#define DEBUG_DECODE
d64 1
a64 1
#define ENCODE_LINE_LEN	45	
d68 1
a68 1
static char
d70 1
a70 1
static char
d72 1
a72 1
static char
d74 2
a75 3
char	xxMap[128], uuMap[128], thisMap[128];
char	*codingMap, *codingTable;
char	thisTable[CODINGTABLESIZE], prevTable[CODINGTABLESIZE];
a81 1
BOOL	customTable;			// = TRUE if found custom table
a83 1
int	thisContentEncoding;
d85 2
d92 1
a92 1
void	EncodeUnit (unsigned char *out, unsigned char *in);
d115 1
a115 1
BOOL	ThreadTable (char *dest, char *name);
d166 1
a166 1
        char mybuf[MAXINTERNALLINE], fileName[MAXFILENAME];
d183 1
a183 1
       	while (fgets(mybuf, MAXINTERNALLINE, DecodeFile) != NULL)
d185 3
d200 2
d313 2
a314 2
	width = 75 * StatusCharWidth;
	height = 10 * StatusLineHeight;
d344 2
a345 1
	unsigned char inBuf[ENCODE_LINE_LEN];
d354 1
a354 1
	case ENCODE_BASE64:	
d357 2
a358 2
	case ENCODE_UU:
		codingTable = uuTable;;
d360 1
a360 1
	case ENCODE_XX:
d362 1
a362 1
		codingTable = xxTable;;
d364 1
a364 1
	case ENCODE_CUSTOM:
d379 5
a383 3
	case ENCODE_UU:
	case ENCODE_XX:
	case ENCODE_CUSTOM:
d390 1
a390 1
	        outLine[0] = codingTable[ENCODE_LINE_LEN];
d393 3
a395 1
	case ENCODE_BASE64:
d398 1
a398 1
	lineLen = ENCODE_LINE_LEN;
d407 7
a413 2
			outLine[0] = codingTable[numRead];

d424 3
a426 3
	case ENCODE_UU:
	case ENCODE_XX:
	case ENCODE_CUSTOM:
d451 1
a451 1
		EncodeUnit (&outLine[j], &line[i]);
d458 1
a458 1
EncodeUnit (unsigned char *out, unsigned char *in)
d462 7
d470 7
a494 3
//	Note, XX table is hard coded in declarations for simplicity
	CreateCodingMap(uuMap, uuTable);
	CreateCodingMap(xxMap, xxTable);
d507 3
a509 2
	thisTable[0] = '\0';
	prevTable[0] = '\0';
d516 2
a517 2
	width = 75 * StatusCharWidth;
	height = 10 * StatusLineHeight;
d561 2
a562 1
		else if (!threadList[0]->codedBlockList[threadList[0]->numBlocks-1]->endFlag)
a616 1
//	thisCoded->altName[0] = '\0';
a618 1
        customTable = FALSE;
d757 3
a759 1
		strncpy (threadList[num]->codingTable, thisTable, CODINGTABLESIZE);
d989 2
a990 5
/* Save this coding table, and blank out the current table
 * This will cause the next block to start decoding w/ a clean slate
 */
	strncpy (prevTable, thisTable, CODINGTABLESIZE);
	thisTable[0] = '\0';
d1142 2
a1143 1

d1146 1
a1146 1
		else
d1148 2
a1149 1
		sprintf (actualFileName, "%s\\%s", DecodePathName, threadList[num]->dosFileName);
d1216 2
a1217 1
/* encoded data always starts with line like 'begin <mode> <file-name>' */
d1237 2
a1238 2
/* Check for keywords in the non-data lines which may aid in describing the line length,
 * the destination file for this data, the sequence, etc
d1273 1
a1273 1
		memmove ((thisTable + table_count * 32), line, 32);
d1278 1
a1278 1
			strncpy (str, thisTable, 64);
d1284 1
a1284 1
			if (i = CreateCodingMap (thisMap, thisTable) != -1)
d1290 1
a1290 2
			codingMap = thisMap;
			customTable = TRUE;
d1301 2
a1302 1
		if (!_strnicmp (line, "end", 3) || !IsDataLine(line))
d1311 1
a1311 1
 * data yet (< about a line's worth), then the line which gave us the table
d1315 1
a1315 1
 * this line.  Note if we have a customTable, then we trust it.
d1317 1
a1317 1
			if (!customTable && thisDecode->numBytes < 80)
d1320 1
a1320 1
				thisTable[0] = '\0';
d1346 1
a1346 1
 * Decode a data line
d1350 48
a1397 14
	register unsigned int i, j;
	unsigned int charCount, numDecoded, checkSum;
	unsigned char buf[4], outLine[120];
	
	checkSum = charCount = codingMap[line[0]];	// count is # of chars after decoding
	for (i = 1, numDecoded=0; numDecoded < charCount;)
	{
/* Get the next group of four characters */
		for (j = 0; j < 4; j++, i++)
			checkSum += (buf[j] = codingMap[line[i]]); 
        
        	outLine[numDecoded++] = buf[0]<<2|buf[1]>>4;
		outLine[numDecoded++] = buf[1]<<4|buf[2]>>2;
		outLine[numDecoded++] = buf[2]<<6|buf[3];
d1399 8
a1406 2
	if (AddDataToBlock (currentCoded, outLine, numDecoded) == FAIL)
		return (FAIL);
d1415 1
a1415 1
	return (SUCCESS);
a1428 1
//	        thisDecode->maxBytes *= 2L;	// must always be a power of 2 
a1443 2
 *	If this is a new block (thisTable is empty), auto-determine what
 *	encoding scheme is used
d1448 1
a1448 1
	if (thisTable[0] == '\0')
d1450 4
a1453 3
/* If this block has same ident as prev, use same decode table
 * If diff name, use table stored in thread w/ same name, or if none 
 * try to auto-detect
d1458 1
a1458 3
			strncpy (thisTable, prevTable, CODINGTABLESIZE);
			CreateCodingMap (thisMap, thisTable);
			codingMap = thisMap;
d1465 1
a1465 1
			if (ThreadTable (thisTable, currentCoded->ident) == SUCCESS)
d1467 2
a1468 2
				CreateCodingMap (thisMap, thisTable);
				codingMap = thisMap;
d1476 1
a1476 1
				codingMap = uuMap;
a1482 1
					strncpy (thisTable, uuTable, CODINGTABLESIZE);
d1485 1
a1485 1
				codingMap = xxMap;
a1491 1
					strncpy (thisTable, xxTable, CODINGTABLESIZE);
d1494 10
d1513 11
a1523 2
	unsigned int expectedLineLen, dataLen, lineLen;
	register unsigned int i;
d1525 2
a1526 2
 * The first char of a good data line is the encoded character count for the 
 * line.  
d1530 3
a1532 2
 * The count INCLUDES the first count char
 * If the line length is actually 1 + count, then we have a checksum
d1534 1
a1534 1
	expectedLineLen = 4 * ((codingMap[line[0]] + 2) / 3);
d1536 1
a1536 1
	if ((lineLen = strlen(line)) < expectedLineLen)
d1542 1
a1542 1
 * Don't remove all trailing white space!
d1544 3
a1546 3
 	for (dataLen = lineLen - 1; 
 	     strchr(" \t\n\r",line[dataLen]) && dataLen > 0 && dataLen != expectedLineLen; 
 	     dataLen--);
d1552 1
d1555 1
a1555 1
		if (codingMap[line[i]] == UNUSED)
d1558 17
a1579 11
void
CreateUUTable()
{
	register unsigned int i;

	for (i=0; i < CODINGTABLESIZE; i++)
		uuTable[i] = i + 32;
	
	uuTable[0] = 96;		/* set space to map to back-quote */	
}

a1596 2
	map[' ']=0;				// decode space to 0

d1608 1
a1608 1
BOOL   
d1619 1
a1619 4
	{
		dest[0]='\0';
		return (FAIL);
	}
d1622 3
a1624 2
		strncpy (dest, threadList[num]->codingTable, CODINGTABLESIZE);
		return (SUCCESS);
d1627 13
a1802 90
/* ------------------------------------------------------------------------
 * Check a line to see if it's MIME.  If so, do something intelligent with
 * it. Note, any MIME header MUST start with 'MIME-version'
 */
int
ParseMimeHeaderLine (TypCoded *thisDecode, char *line)
{
        char *ptr;

	// gotta start with MIME-version, or it ain't MIME
	if (!bossanovaMIME)			
		if (!_strnicmp (line, "mime-version", 12))
		{
			bossanovaMIME = TRUE;		// hooray
			return (SUCCESS);
		}
		else
			return (FAIL);
		
	if (ParseMimeContentType (thisDecode, line) == SUCCESS)
		return (SUCCESS);
		
	if (!_strnicmp (line, "content-transfer-encoding:", 26))
	{
		for (ptr = &line[26]; *ptr && isspace(*ptr); ptr++);
	        if (!_strnicmp (ptr, "x-uue", 5))
	        	thisContentEncoding = ENCODE_UU;
	        else if (!_strnicmp (ptr, "x-xxe", 5))
	        	thisContentEncoding = ENCODE_XX;
	        else if (!_strnicmp (ptr, "base64", 6))
	        	thisContentEncoding = ENCODE_BASE64;
	        else if (!_strnicmp (ptr, "quoted-printable",16))
	        	thisContentEncoding = ENCODE_QP;
	        else
	        	thisContentEncoding = ENCODE_UNKNOWN;
		return (SUCCESS);
	}
	if (!_strnicmp (line, "content-description:", 20))
	{
		strcpy (thisContentDesc, &line[20]);
		return (SUCCESS);
	}
	if (!_strnicmp (line, "content-length", 14))
		return (SUCCESS);

   return (FAIL);
}

int
ParseMimeContentType (TypCoded *thisDecode, char *line)
{
	static int mimeContentLevel = 0;
	BOOL recurse = FALSE;
	char *ptr;
	
	if (mimeContentLevel == 0)
		if (!_strnicmp (line, "content-type:", 13))
		{
			ptr = strchr (&line[13], ';');	// check for parameters
			if (*ptr != '\0')
			{
				*ptr = '\0';            // replace ; with \0
				if (*(ptr+1) != '\0')
					recurse = TRUE;
			}
			strcpy (thisContentType, &line[13]);
			mimeContentLevel++;
			if (recurse)		// parse same-line parameters
				ParseMimeContentType (thisDecode, ptr);
				
			return (SUCCESS);
		} else
			return (FAIL);
	
	if (!_strnicmp (line, "id=", 3))
	{
		ptr = strchr (&line[3], ';');	// check for parameters
		if (*ptr != '\0')
		{
			*ptr = '\0';            // replace ; with \0
			if (*(ptr+1) != '\0')
				recurse = TRUE;
		}
		strcpy (thisDecode->ident, &line[3]);
		if (recurse)		// parse same-line parameters
			ParseMimeContentType (thisDecode, ptr);
		return (SUCCESS);
	}
	return (FAIL);				
}
@


1.1
log
@Initial revision
@
text
@d38 5
a42 2
 * $Id:  $
 * $Log:  $
d315 1
a315 1
	SetClassWord (hCodedBlockWnd, GCW_HBRBACKGROUND, hStatusBackgroundBrush);
d389 1
a389 1
        hFile = _lopen (fileName, READ);
d496 1
a496 1
	SetClassWord (hCodedBlockWnd, GCW_HBRBACKGROUND, hStatusBackgroundBrush);
d768 2
a769 2
		   SetClassWord (threadList[num]->statusText->hTextWnd,
				 GCW_HBRBACKGROUND, hStatusBackgroundBrush);
@
