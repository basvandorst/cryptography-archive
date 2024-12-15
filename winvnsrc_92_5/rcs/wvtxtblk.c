head	1.3;
access;
symbols;
locks; strict;
comment	@ * @;


1.3
date	94.08.11.00.11.55;	author jcooper;	state Exp;
branches;
next	1.2;

1.2
date	94.05.23.18.36.00;	author jcooper;	state Exp;
branches;
next	1.1;

1.1
date	94.01.16.12.12.47;	author jcoop;	state Exp;
branches;
next	;


desc
@jcoop's block mem mgmt routines, alternative to MRR's block/doc paradigm
@


1.3
log
@Changes to author mail address
@
text
@/********************************************************************
 *                                                                  *
 *  MODULE    :  WVTXTBLK.C                                         *
 *                                                                  *
 *  PURPOSE   : This file contains Memory Management routines for   *
 *		text blocks                                         *                                 
 *                                                                  *
 *              The TypTextBlock struct is defined in wvglob.h      *
 *                                                                  *
 * Author: John S. Cooper (jcooper@@netcom.com)                      *
 *   Date: Sept 30, 1993                                            *
 ********************************************************************/
/* 
 * $Id: wvtxtblk.c 1.2 1994/05/23 18:36:00 jcooper Exp $
 * $Log: wvtxtblk.c $
 * Revision 1.2  1994/05/23  18:36:00  jcooper
 * new attach code, session [dis]connect
 *
 * Revision 1.1  1994/01/16  12:12:47  jcoop
 * Initial revision
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <ctype.h>
#include "wvglob.h"
#include "winvn.h"

#define BLOCK_NUM_LINES_INC	64
#define TEXT_SIZE_INC		1024

// Private functions
BOOL IncreaseTextSize (TypTextBlock *textBlock, unsigned long inc);
BOOL AllocTextBlock (TypTextBlock *textBlock);

/* ------------------------------------------------------------------------
 * 	Mem Management routines for text block
 *	This is my simple alternative M. Riordan's Doc paradigm
 */
TypTextBlock *
InitTextBlock (HWND hParentWnd) 
{
	TypTextBlock *textBlock;
        
	if ((textBlock = (TypTextBlock *) GlobalAllocPtr (GMEM_MOVEABLE, sizeof (TypTextBlock)))==NULL)
	{
		MessageBox (hParentWnd, "Memory Allocation Failure", "Text Block Create-Block", ID_OK);
		return (NULL);
	}
	textBlock->hTextWnd = hParentWnd;
	if (AllocTextBlock(textBlock) == FAIL)
		return (NULL);
	
	return (textBlock);
}	 

void
ResetTextBlock (TypTextBlock *textBlock) 
{
	GlobalFreePtr (textBlock->text);
	GlobalFreePtr (textBlock->offset);
	AllocTextBlock(textBlock);
}	 

BOOL
AllocTextBlock (TypTextBlock *textBlock)
{
	textBlock->maxLines = BLOCK_NUM_LINES_INC;
	if ((textBlock->offset = (unsigned long huge *) GlobalAllocPtr (GMEM_MOVEABLE, textBlock->maxLines * sizeof (unsigned long huge))) == NULL)
	{
		MessageBox (textBlock->hTextWnd, "Memory Allocation Failure", "Text Block Create-Offsets", ID_OK);
		return (FAIL);
	}
	textBlock->numLines = 0;

	textBlock->maxBytes = TEXT_SIZE_INC;
	if ((textBlock->text = (char huge *) GlobalAllocPtr (GMEM_MOVEABLE, textBlock->maxBytes * sizeof (char huge))) == NULL)
	{
		MessageBox (textBlock->hTextWnd, "Memory Allocation Failure", "Text Block Create-Text", ID_OK);
		return (FAIL);
	}
	textBlock->numBytes = 0;
	textBlock->maxLineLen = 0;
	return (SUCCESS);
}
void
FreeTextBlock (TypTextBlock *textBlock)
{
	GlobalFreePtr (textBlock->text);
	GlobalFreePtr (textBlock->offset);
	GlobalFreePtr (textBlock);
}

BOOL
AddLineToTextBlock (TypTextBlock *textBlock, char *line)
{		
	unsigned int len;
	unsigned long nextSeg;

	if (line == NULL || *line == '\0')
		return (SUCCESS);

	// manage offset list
	if (textBlock->numLines + 1 >= textBlock->maxLines)
	{
		textBlock->maxLines += BLOCK_NUM_LINES_INC;
		if ((textBlock->offset = (unsigned long huge *) GlobalReAllocPtr (textBlock->offset,
	   		     textBlock->maxLines * sizeof (unsigned long huge), GMEM_MOVEABLE)) == NULL)
		{
			MessageBox (textBlock->hTextWnd, "Memory Allocation Failure", "Text Block Add-Offsets", ID_OK);
			return (FAIL);
		}
	}

	// manage text string
	len = strlen (line) + 1;	// include the NULL in the count
	if (textBlock->numBytes + len >= textBlock->maxBytes)
		if (IncreaseTextSize (textBlock, max(len, TEXT_SIZE_INC)) == FAIL)
			return (FAIL);
		
	// check if this line would cause text to cross a 64k segment
	// if so, fill in with nulls, and start new line at next seg 
	// WHY DO I HAVE TO DO THIS?!?!? Windows is supposed to take
	// care of this isn't it?!  memcpy is supposed to work with huge
	nextSeg = ((textBlock->numBytes + 65536L) / 65536L) * 65536L;
	if (textBlock->numBytes + len > nextSeg) 
	{
	   if (nextSeg + len > textBlock->maxBytes)	// do we need to add some space?
	   	if (IncreaseTextSize (textBlock, max(nextSeg-textBlock->numBytes, TEXT_SIZE_INC)) == FAIL)
			return (FAIL);
	   while (textBlock->numBytes < nextSeg)
		textBlock->text[(textBlock->numBytes)++] = '\0';
	}	
        // set pointer offset to this new string
        textBlock->offset[textBlock->numLines] = textBlock->numBytes;
        
        // copy it in, including the NULL
	memcpy ((char huge *) &(textBlock->text[textBlock->numBytes]), line, len);

        // increment counters
	textBlock->numLines++;
	textBlock->numBytes += (unsigned long)len;
	
	// save max string len for use in scroll bar ranges in status wnd
	if (len > textBlock->maxLineLen)
		textBlock->maxLineLen = len;

	return (SUCCESS);
}

BOOL
IncreaseTextSize (TypTextBlock *textBlock, unsigned long inc)
{
	textBlock->maxBytes += inc;
	if ((textBlock->text = (char huge *) GlobalReAllocPtr (textBlock->text,
   		     textBlock->maxBytes * sizeof (char huge), GMEM_MOVEABLE)) == NULL)
	{
		MessageBox (textBlock->hTextWnd, "Memory Allocation Failure", "Text Block Add-Text", ID_OK);
		return (FAIL);
	}
}
/* ------------------------------------------------------------------------
 * AddEndedLineToAttach
 * Front end to AddLineToTextBlock which sets the end-of-line chars
 * Also, if posting and first char is '.', change it to '..'
 * To add blank line, send line parameter as ""
 */                   
BOOL
AddEndedLineToTextBlock (TypTextBlock *textBlock, char *line, int mode)
{
	char *new, *ptr;
	unsigned int len;
	BOOL result;
	
	len = strcspn (line, "\n\r");
	ptr = new = (char *) GlobalAllocPtr (GMEM_MOVEABLE, (len+4) * sizeof(char));
	if (mode == ADD_TO_POST && line[0] == '.')
		*(ptr++) = '.';

	if (len > 0)
		memcpy (ptr, line, len);
		
	switch (mode)
	{
	case ADD_TO_POST:
		ptr[len++] = '\r';	
		break;
	case ADD_TO_EDIT:
	case ADD_TO_FILE:
		ptr[len++] = '\r';	// to edit buf
		ptr[len++] = '\n';
		break;
//	case ADD_TO_FILE:
//		ptr[len++] = '\n';
//		break;
	}	
	ptr[len] = '\0';
	
	result = AddLineToTextBlock (textBlock, new);
	GlobalFreePtr (new);
	
	return (result);
}

/* ------------------------------------------------------------------------
 * ReadFileToTextBlock
 * Reads text file into text block
 * File must be all ASCII
 */
BOOL
ReadFileToTextBlock (HWND hParentWnd, TypTextBlock *block, char *fileName, int mode)
{
	register int i;
	TypMRRFile *MRRFile;
	HANDLE hRetCode;
	int numRead, result;
	char mybuf[TEMPBUFSIZE];
	BOOL leadingBlanks;
	
	hRetCode = MRROpenFile (fileName, OF_READ, &MRRFile);
	if ((int) hRetCode <= 0)
	{
		sprintf(str, "Could not open file %s for read", fileName);
		MessageBox (hParentWnd, str, "Open File Error", MB_OK);
		return (FAIL);
	}
	else
	{
		CodingState = ATTACH_READFILE;
		leadingBlanks = TRUE;
		result = SUCCESS;
		while ((numRead = MRRReadLine (MRRFile, mybuf, TEMPBUFSIZE)) > -1)
		{
			mybuf[numRead] = '\0';
			if (leadingBlanks)	// skip leading blanks
			  if (IsBlankStr (mybuf))
			  	continue;
			  else
			  	leadingBlanks = FALSE;
			  	
       			for (i = 0; i < numRead; i++)
			{
			  if (!__isascii (mybuf[i]))
			  {
				sprintf(str, "File %s appears to contain non-ASCII data.\nBinary files must be encoded for attachment", fileName);
				MessageBox (hParentWnd, str, "Non-ASCII File Error", MB_OK);
				result = FAIL;
				goto endRead;
			  }
			}
			if (AddEndedLineToTextBlock (block, mybuf, mode) == FAIL)
				break;
			if (hCodedBlockWnd != (HWND)NULL)
			{
				currentCoded->numLines++;
				currentCoded->numBytes+=numRead;
			    	if (currentCoded->numLines % STATUS_UPDATE_FREQ == 0)
			    		UpdateBlockStatus ();
			}
		}
	}

    endRead:;
	MRRCloseFile (MRRFile);
	CodingState = INACTIVE;
	return (result);
}

/* ------------------------------------------------------------------------
 * ReplaceLineInTextBlock
 * This is ugly, and really inefficient of the textblock is big
 * 1. If newline has length > oldline, then make sure text array has enough
 *    room, then shift right the string starting at the end of oldline by 
 *    n characters where n is len(newline)-len(oldline)
 * 2. If newline has length < oldline then shift the string starting at the
 *    end of oldline by n characters where n is len(oldline)-len(newline)
 * 3. If oldline and newline are same length, do no shifting
 * 4. Then copy in the new string
 * 5. Adjust all affected offsets (the lines after this one) by size diff
 * 6. God this is ugly, hope we don't use it much!
 */                   
BOOL
ReplaceLineInTextBlock (TypTextBlock *textBlock, unsigned long num, char *line)
{		
	unsigned int newLen, oldLen;
	int sizeDiff;
        register char huge *dst, *src;
	register unsigned long i;        
	
	if (line == NULL || *line == '\0')
		return (SUCCESS);
		
	newLen = strlen (line) + 1;
	oldLen = strlen(TextBlockLine(textBlock, num)) + 1;
	sizeDiff = newLen - oldLen;
	
	if (sizeDiff > 0)	// new line longer than old line
	{
		// increase text array size if necessary
		if (textBlock->numBytes + sizeDiff > textBlock->maxBytes)
		{
			textBlock->maxBytes *= 2;
			if ((textBlock->text = (char huge *) GlobalReAllocPtr (textBlock->text,
		   		     textBlock->maxBytes * sizeof (char), GMEM_MOVEABLE)) == NULL)
			{
				MessageBox (textBlock->hTextWnd, "Memory Allocation Failure", "Text Block Add", ID_OK);
				return (FAIL);
			}
		}
		// shift the text array right by sizeDiff chars
		// sizeDiff iterations, walking backwards from end
		for (src = textBlock->text + textBlock->numBytes,
		     dst = src + sizeDiff;;
		     dst--, src--)
		{			
			*dst = *src; 
		     	if (src == TextBlockLine(textBlock, num))
		     		break;
		}
		     
	}
	else if (sizeDiff < 0)
		// shift the text array left by sizeDiff chars
		// numBytes - &oldline iterations, walking forwards
		for (src = TextBlockLine(textBlock, num) + oldLen,
		     dst = src + sizeDiff;
		     dst < &textBlock->text[textBlock->numBytes + 1];
		     dst++, src++)
			*dst = *src;
	
	// adjust all affected offsets (lines after this line)
	if (sizeDiff != 0)
		for (i = num + 1; i < textBlock->numLines; i++)
			textBlock->offset[i] += sizeDiff;
	
	textBlock->numBytes += sizeDiff;

	// save max string len for use in scroll bar ranges in status wnd
	if (newLen > textBlock->maxLineLen)
		textBlock->maxLineLen = newLen;
				
        // copy it in, including the NULL
	memcpy (TextBlockLine(textBlock, num), line, newLen);

	return (SUCCESS);
}
@


1.2
log
@new attach code, session [dis]connect
@
text
@d10 1
a10 1
 * Author: John S. Cooper (jcoop@@apl.com)                           *
d14 1
a14 1
 * $Id: wvtxtblk.c 1.1 1994/01/16 12:12:47 jcoop Exp $
d16 3
@


1.1
log
@Initial revision
@
text
@d14 5
a18 2
 * $Id:  $
 * $Log:  $
d165 1
a165 3
 * to \n\r if posting to EditWnd, to \r if posting to net, or to \n if
 * posting to dos file
 * Also, if Attaching and first char is '.', change it to '..'
d171 1
a171 1
	char *new;
d176 3
a178 3
	new = (char *) GlobalAllocPtr (GMEM_MOVEABLE, (len+4) * sizeof(char));
	if (Attaching && line[0] == '.')
		new[len++] = '.';
d181 1
a181 1
		memcpy (new, line, len);
d186 1
a186 1
		new[len++] = '\r';	// \r if posting to net OR edit buf
d190 2
a191 2
		new[len++] = '\r';	// to edit buf
		new[len++] = '\n';
d194 1
a194 1
//		new[len++] = '\n';
d197 1
a197 1
	new[len] = '\0';
@
