head	1.13;
access;
symbols
	V80:1.2
	V76d:1.1;
locks; strict;
comment	@ * @;


1.13
date	94.08.24.18.00.29;	author jcooper;	state Exp;
branches;
next	1.12;

1.12
date	94.07.27.21.12.19;	author gardnerd;	state Exp;
branches;
next	1.11;

1.11
date	94.07.12.19.51.26;	author cnolan;	state Exp;
branches;
next	1.10;

1.10
date	94.06.01.19.08.20;	author gardnerd;	state Exp;
branches;
next	1.9;

1.9
date	94.02.24.21.34.08;	author jcoop;	state Exp;
branches;
next	1.8;

1.8
date	94.02.09.18.01.08;	author cnolan;	state Exp;
branches;
next	1.7;

1.7
date	94.01.12.19.27.32;	author mrr;	state Exp;
branches;
next	1.6;

1.6
date	93.12.08.01.28.01;	author rushing;	state Exp;
branches;
next	1.5;

1.5
date	93.07.06.20.53.23;	author rushing;	state Exp;
branches;
next	1.4;

1.4
date	93.07.06.20.52.28;	author cnolan;	state Exp;
branches;
next	1.3;

1.3
date	93.06.28.17.51.39;	author rushing;	state Exp;
branches;
next	1.2;

1.2
date	93.05.26.18.30.40;	author rushing;	state Exp;
branches;
next	1.1;

1.1
date	93.02.16.20.54.22;	author rushing;	state Exp;
branches;
next	;


desc
@winvn version 0.76 placed into RCS
@


1.13
log
@misc encoding/decoding changes
@
text
@
/*
 *
 * $Id: wvscreen.c 1.12 1994/07/27 21:12:19 gardnerd Exp $
 * $Log: wvscreen.c $
 * Revision 1.12  1994/07/27  21:12:19  gardnerd
 * copy to clipboard
 *
 * Revision 1.11  1994/07/12  19:51:26  cnolan
 * win32ism
 *
 * Revision 1.10  1994/06/01  19:08:20  gardnerd
 * horizontal scrolling support
 *
 * Revision 1.9  1994/02/24  21:34:08  jcoop
 * jcoop changes
 *
 * Revision 1.8  1994/02/09  18:01:08  cnolan
 * cnolan 90.2 changes
 *
 * Revision 1.7  1994/01/12  19:27:32  mrr
 * mrr mods 4
 *
 * Revision 1.6  1993/12/08  01:28:01  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.5  1993/07/06  20:53:23  rushing
 * type fixed
 *
 * Revision 1.4  1993/07/06  20:52:28  cnolan
 * win32 changes
 *
 * Revision 1.3  1993/06/28  17:51:39  rushing
 * fixed compiler warnings
 *
 * Revision 1.2  1993/05/26  18:30:40  rushing
 * fixed page down bug (GPE when #arts < what can fit in window)
 *
 * Revision 1.1  1993/02/16  20:54:22  rushing
 * Initial revision
 *
 *
 */

/*-- First Line of WVSCREEN.C -- Contains Screen-related functions.  */

#include "windows.h"
#include "wvglob.h"
#include "winvn.h"

/*--- function ScreenDown ----------------------------------------------
 *
 *  Advance a pointer "nLines" lines through the textlines.
 *  However, do not go so far that less than LinesOnScreen lines
 *  are left past the pointer.
 *
 *  This is used to implement "PageDown" and "LineDown" functions.
 *
 *    Entry    nLines         Number of lines to move the pointer.
 *             LinesOnScreen  is the number of lines on the screen.
 *             BlockPtr       Pointer to block containing line.
 *             LinePtr        Pointer to the given line.
 *
 *    Exit     BlockPtr and LinePtr (may) have been moved to a new line.
 *             LinesAdvanced  is the number of lines actually moved--can be
 *                            less than nLines if we hit the top of the
 *                            last screen.
 */
void
ScreenDown (nLines, LinesOnScreen, BlockPtr, LinePtr, LinesAdvanced)
     int nLines;
     int LinesOnScreen;
     TypBlock far **BlockPtr;
     TypLine far **LinePtr;
     int *LinesAdvanced;
{
  TypBlock far *MyBlock = *BlockPtr;
  TypLine far *MyLine = *LinePtr;
  int LinesGone;
  int TestAdvance;
  HANDLE hBlock, hBlockGarbage;
  unsigned int Offset, OffsetGarbage;
  TypLineID LineIDGarbage, MyLineID;

  /* Skip forward nLines, plus one screen's worth, just to see      */
  /* if there are enough lines ahead of us in the document.         */

  PtrToOffset (MyBlock, MyLine, &hBlock, &Offset, &MyLineID);
  TestAdvance = nLines + LinesOnScreen - 1;
  for (LinesGone = TestAdvance; LinesGone ;) {
      if(!NextLine (&MyBlock, &MyLine)) break;
      /* Count only active lines. */
      if(MyLine->active) LinesGone--;
  }
  UnlockLine (MyBlock, MyLine, &hBlockGarbage, &OffsetGarbage, &LineIDGarbage);

  nLines -= LinesGone;
  if (nLines < 0)
    nLines = 0;
  *LinesAdvanced = nLines;

  LockLine (hBlock, Offset, MyLineID, &MyBlock, &MyLine);
  while (nLines)
    {
      NextLine (BlockPtr, LinePtr);
      if((*LinePtr)->active) nLines--;
    }
}

/*--- function ScreenUp ----------------------------------------------
 *
 *  Back up a pointer "nLines" lines through the textlines.
 *  However, do not go past the beginning of the document.
 *
 *  This is used to implement "PageUp" and "LineUp" functions.
 *
 *    Entry    nLines         Number of lines to move the pointer.
 *             BlockPtr       Pointer to block containing line.
 *             LinePtr        Pointer to the given line.
 *
 *    Exit     BlockPtr and LinePtr (may) have been moved to a new line.
 *             LinesBackedUp  is the number of lines actually moved--can be
 *                            less than nLines if we hit beginning of doc.
 */
void
ScreenUp (nLines, BlockPtr, LinePtr, LinesBackedUp)
     int nLines;
     TypBlock far **BlockPtr;
     TypLine far **LinePtr;
     int *LinesBackedUp;
{
  *LinesBackedUp = 0;

  while (nLines && PrevLine (BlockPtr, LinePtr))
    {          
      if((*LinePtr)->active) {
         nLines--;
         (*LinesBackedUp)++;
      }
    }
}

/*--- function NewScrollIt ----------------------------------------------
 *
 *  Perform a scrolling action.
 *
 *    Entry    Document points to a document
 *             wParam   is the wParam argument given from a WM_VSCROLL
 *                      window message.  (One of the SB_ symbols.)
 *             lParam   is the lParam argument.
 *
 *   except in WIN32 ...
 *
 *          wParam   LOWORD: SB_code, HIWORD: position.
 */
void
NewScrollIt (Document, wParam, lParam)
     TypDoc *Document;
     WPARAM wParam;
     LPARAM lParam;
{
  int LinesGone;
  unsigned int LineOrd;
  RECT Rect;
  int window_lineheight, window_topspace;

  if (Document->DocType == DOCTYPE_ARTICLE)
  {
   window_lineheight = ArtLineHeight;
   window_topspace = ArtTopSpace; 
  }
  else
  {
   window_lineheight = LineHeight;
   window_topspace = TopSpace;
  }

  switch (LOWORD(wParam))
    {
    case SB_LINEUP:
      /* Move up a line by scrolling the window down one line     */
      /* and introducing 1 new line at the top.                   */

   if (Document->TopLineOrd > 0) {
      LinesGone = 1;
    }
    else LinesGone = 0;

    doscrollup:;
      if (LinesGone)
   {
     Rect.left = 0;
     Rect.right = Document->ScXWidth;
     Document->TopLineOrd -= LinesGone;

     ScrollWindow (Document->hDocWnd, 0, LinesGone * window_lineheight, NULL, NULL);

     Rect.top = window_topspace;
     Rect.bottom = window_topspace + LinesGone * window_lineheight;

     InvalidateRect (Document->hDocWnd, &Rect, FALSE);

     /* Make sure garbage at bottom is erased in WM_PAINT processing */
     Rect.top = window_topspace + Document->ScYLines * window_lineheight;
     Rect.bottom = Document->ScYHeight;
     InvalidateRect (Document->hDocWnd, &Rect, FALSE);
   }
      break;

    case SB_LINEDOWN:
      /* Should change TotalLines below to ActiveLines for 
       * suppression of already-read articles, but it's not that simple.
       *  /mrr
       */
   if (Document->TopLineOrd < (Document->ActiveLines - Document->ScYLines)) {
      LinesGone = 1;
    }
   else LinesGone = 0;

    doscrolldown:;
      if (LinesGone)
   {
     Document->TopLineOrd += LinesGone;
     Rect.left = 0;
     Rect.right = Document->ScXWidth;
     ScrollWindow (Document->hDocWnd, 0, -LinesGone * window_lineheight, NULL, NULL);
     Rect.top = window_topspace + (Document->ScYLines - LinesGone) * window_lineheight;
     Rect.bottom = Document->ScYHeight;
     InvalidateRect (Document->hDocWnd, &Rect, FALSE);
     /* Make sure garbage at top is erased in WM_PAINT processing */
     Rect.top = 0;
     Rect.bottom = window_topspace;
     InvalidateRect (Document->hDocWnd, &Rect, FALSE);
   }
      break;

    case SB_PAGEUP:
/*      LockLine (Document->hCurTopScBlock, Document->TopScOffset, Document->TopScLineID, &BlockPtr, &LinePtr); */
/*      ScreenUp (Document->ScYLines - 1, &BlockPtr, &LinePtr, &LinesGone); */
      LinesGone= Document->TopLineOrd - (Document->ScYLines - 1);
      if (LinesGone > 0) {
   LinesGone = Document->ScYLines - 1;
      }
   else {
   LinesGone = Document->TopLineOrd;
   }

      Document->TopLineOrd -= LinesGone;
      InvalidateRect (Document->hDocWnd, NULL, FALSE);
      break;

 case SB_PAGEDOWN:

   if (Document->ActiveLines > Document->ScYLines) { /* do we even need to scroll? */
     if ((Document->ActiveLines -
     (Document->TopLineOrd + Document->ScYLines)) > Document->ScYLines)
       LinesGone = Document->ScYLines - 1;
     else
       LinesGone = Document->ActiveLines - (Document->TopLineOrd + Document->ScYLines );

     Document->TopLineOrd += LinesGone;
     InvalidateRect (Document->hDocWnd, NULL, FALSE);
   }
   break;

 case SB_THUMBPOSITION:
#ifdef WIN32
       LineOrd = (int)HIWORD (wParam);
#else
       LineOrd = LOWORD (lParam);
#endif
/*       if (!FindLineOrd (Document, LineOrd, &BlockPtr, &LinePtr))
   {
     return;
   }
*/
    doposition:;
      Document->TopLineOrd = LineOrd;
      InvalidateRect (Document->hDocWnd, NULL, FALSE);
      break;

    case SB_THUMBTRACK:

#ifdef WIN32
       LineOrd = (int)HIWORD (wParam);
#else
       LineOrd = LOWORD (lParam);
#endif
/*      if (!FindLineOrd (Document, LineOrd, &BlockPtr, &LinePtr))
   {
     return;
   }
*/
      LinesGone = LineOrd - Document->TopLineOrd;
      if (LinesGone > 0)
   {
     if (LinesGone >= (int) (Document->ScYLines - 1))
       {
         goto doposition;
       }
     else
       {
         goto doscrolldown;
       }
   }
      else if (LinesGone < 0)
   {
     LinesGone = -LinesGone;
     if (LinesGone >= (int) (Document->ScYLines - 1))
       {
         goto doposition;
       }
     else
       {
         goto doscrollup;
       }
   }

      break;

    default:
      return;
      break;
    }

/*  UnlockLine (BlockPtr, LinePtr, &(Document->hCurTopScBlock),
         &(Document->TopScOffset), &(Document->TopScLineID));
*/

}
/*--- function ScrollIt ----------------------------------------------
 *
 *  Perform a scrolling action.
 *
 *    Entry    Document points to a document
 *             wParam   is the wParam argument given from a WM_VSCROLL
 *                      window message.  (One of the SB_ symbols.)
 *             lParam   is the lParam argument.
 */
void
ScrollIt (Document, wParam, lParam)
     TypDoc *Document;
     WPARAM wParam;
     LPARAM lParam;
{
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
  int LinesGone;
  unsigned int LineOrd;
  RECT Rect;
  int window_lineheight, window_topspace;
  
  if(Document->DocType == DOCTYPE_ARTICLE)
  {
    window_lineheight = ArtLineHeight;
    window_topspace = ArtTopSpace;
  }
  else
  {
    window_lineheight = LineHeight;
    window_topspace = TopSpace;
  }  
    
  switch (LOWORD(wParam))
    {
    case SB_LINEUP:
      /* Move up a line by scrolling the window down one line     */
      /* and introducing 1 new line at the top.                   */
      LockLine (Document->hCurTopScBlock, Document->TopScOffset, Document->TopScLineID, &BlockPtr, &LinePtr); 
      AdvanceToActive(&BlockPtr,&LinePtr);
      ScreenUp (1, &BlockPtr, &LinePtr, &LinesGone);
    doscrollup:;
      if (LinesGone)
   {
     Rect.left = 0;
     Rect.right = Document->ScXWidth;
     Document->TopLineOrd -= LinesGone;
     ScrollWindow (Document->hDocWnd, 0, LinesGone * (window_lineheight), NULL, NULL);

     Rect.top = window_topspace;
     Rect.bottom = window_topspace + LinesGone * window_lineheight;

     InvalidateRect (Document->hDocWnd, &Rect, FALSE);
     /* Make sure garbage at bottom is erased in WM_PAINT processing */
     Rect.top = window_topspace + Document->ScYLines * window_lineheight;
     Rect.bottom = Document->ScYHeight;
     InvalidateRect (Document->hDocWnd, &Rect, FALSE);
   }
      break;

    case SB_LINEDOWN:
      LockLine (Document->hCurTopScBlock, Document->TopScOffset, Document->TopScLineID, &BlockPtr, &LinePtr);
      AdvanceToActive(&BlockPtr,&LinePtr);
      ScreenDown (1, Document->ScYLines, &BlockPtr, &LinePtr, &LinesGone);
    doscrolldown:;
      if (LinesGone)
   {
     Document->TopLineOrd += LinesGone;
     Rect.left = 0;
     Rect.right = Document->ScXWidth;
     ScrollWindow (Document->hDocWnd, 0, -LinesGone * (window_lineheight), NULL, NULL);
     Rect.top = window_topspace + (Document->ScYLines - LinesGone) * window_lineheight;
     Rect.bottom = Document->ScYHeight;
     InvalidateRect (Document->hDocWnd, &Rect, FALSE);
     /* Make sure garbage at top is erased in WM_PAINT processing */
     Rect.top = 0;
     Rect.bottom = window_topspace;
     InvalidateRect (Document->hDocWnd, &Rect, FALSE);
   }
      break;

    case SB_PAGEUP:
      LockLine (Document->hCurTopScBlock, Document->TopScOffset, Document->TopScLineID, &BlockPtr, &LinePtr);
      AdvanceToActive(&BlockPtr,&LinePtr);
      ScreenUp (Document->ScYLines - 1, &BlockPtr, &LinePtr, &LinesGone);
      Document->TopLineOrd -= LinesGone;
      InvalidateRect (Document->hDocWnd, NULL, FALSE);
      break;

    case SB_PAGEDOWN:
      LockLine (Document->hCurTopScBlock, Document->TopScOffset, Document->TopScLineID, &BlockPtr, &LinePtr);
      AdvanceToActive(&BlockPtr,&LinePtr);
      ScreenDown (Document->ScYLines - 1, Document->ScYLines,
        &BlockPtr, &LinePtr, &LinesGone);
      Document->TopLineOrd += LinesGone;
      InvalidateRect (Document->hDocWnd, NULL, FALSE);
      break;

    case SB_THUMBPOSITION:
#ifdef WIN32
       LineOrd = (int)HIWORD (wParam);
#else
       LineOrd = LOWORD (lParam);
#endif
      if (!FindLineOrd (Document, LineOrd, &BlockPtr, &LinePtr))
   {
     return;
   }
    doposition:;
      Document->TopLineOrd = LineOrd;
      InvalidateRect (Document->hDocWnd, NULL, FALSE);
      break;

    case SB_THUMBTRACK:
                                                  
#ifdef WIN32
       LineOrd = (int)HIWORD (wParam);
#else
       LineOrd = LOWORD (lParam);
#endif
      if (!FindLineOrd (Document, LineOrd, &BlockPtr, &LinePtr))
   {
     return;
   }
      LinesGone = LineOrd - Document->TopLineOrd;
      if (LinesGone > 0)
   {
     if (LinesGone >= (int) (Document->ScYLines - 1))
       {
         goto doposition;
       }
     else
       {
         goto doscrolldown;
       }
   }
      else if (LinesGone < 0)
   {
     LinesGone = -LinesGone;
     if (LinesGone >= (int) (Document->ScYLines - 1))
       {
         goto doposition;
       }
     else
       {
         goto doscrollup;
       }
   }

      break;

    default:
      return;
      break;
    }

  UnlockLine (BlockPtr, LinePtr, &(Document->hCurTopScBlock),
         &(Document->TopScOffset), &(Document->TopScLineID));

}


 
/*--- function HScrollIt ----------------------------------------------
 *
 *  Perform a horizontal scrolling action.
 *
 *    Entry    Document points to a document
 *             wParam   is the wParam argument given from a WM_HSCROLL
 *                      window message.  (One of the SB_ symbols.)
 *             lParam   is the lParam argument (scroll box position).
 */
void
HScrollIt (Document, wParam, lParam)
     TypDoc *Document;
     WPARAM wParam;
     LPARAM lParam;
{ 

switch (LOWORD(wParam))
  {
  
  case SB_LINEUP:
    if(Document->ScXOffset > 0)
    { 
      RECT Rect;
      
      Document->ScXOffset--;          
      Rect.top = 0;
      Rect.bottom = Document->ScYHeight; 
      
      if(Document->DocType == DOCTYPE_ARTICLE)
      {
        Rect.left = -((int)ArtCharWidth + 1);
      }
      else
      {
        Rect.left = -((int)CharWidth + 1);
      }    
      Rect.right = Document->ScXWidth - Rect.left;  
      ScrollWindow(Document->hDocWnd, -Rect.left, 0, &Rect, NULL);
      Rect.right = -Rect.left;  

      InvalidateRect(Document->hDocWnd, &Rect, FALSE);
    }
    break;

  case SB_LINEDOWN:
    if(Document->ScXOffset < Document->LongestLine - Document->ScXChars)
    { 
      RECT Rect;
      
      Document->ScXOffset++; 
      Rect.top = 0;
      Rect.bottom = Document->ScYHeight; 
      
      if(Document->DocType == DOCTYPE_ARTICLE)
      {
        Rect.left = ArtCharWidth + 1;  
      }
      else
      {
        Rect.left = CharWidth +1;
      }    
      Rect.right =  Document->ScXWidth + Rect.left;
      ScrollWindow(Document->hDocWnd, -Rect.left, 0, &Rect, NULL);
      Rect.right = Rect.left;
      Rect.left = 0;    

      InvalidateRect(Document->hDocWnd, &Rect, FALSE);
    }
    break;                  
  
  case SB_PAGEUP:
    if(((int)Document->ScXOffset - (int)Document->ScXChars) > 0)
    {
      Document->ScXOffset = Document->ScXOffset -
                            Document->ScXChars + 5;
    }
    else
    {
      Document->ScXOffset = 0;
    }
    InvalidateRect(Document->hDocWnd, NULL, FALSE);
    break;
          
  case SB_PAGEDOWN:
    if((Document->ScXOffset + Document->ScXChars) < 
       (Document->LongestLine - Document->ScXChars))
    {
      Document->ScXOffset = Document->ScXOffset +
                            Document->ScXChars - 5;                            
    }
    else
    {
      Document->ScXOffset = Document->LongestLine - 
                            Document->ScXChars;
    }                                          
    InvalidateRect(Document->hDocWnd, NULL, FALSE);
    break;
  
  case SB_THUMBPOSITION:
  case SB_THUMBTRACK:

#ifdef _WIN32
    Document->ScXOffset = (int)HIWORD(wParam);
#else
    Document->ScXOffset = LOWORD(lParam);
#endif


    InvalidateRect (Document->hDocWnd, NULL, FALSE);
    break;

  default:         
    return;
    break;
      
  }
}




/*--- function LineOnScreen ----------------------------------------
 *
 *  Determine whether a given line is displayed on the screen.
 *  If it is, give the ordinal line number on the screen.
 *
 *  Entry   Doc         points to the document.
 *          hTargBlock  is the handle of the block containing the line.
 *          TargOffset  is the offset of the target line.
 *          TargLineID  is the line's ID.
 *
 *  Exit    returns -1 if the line is not on the screen, else
 *          the ordinal line number (0 = top line, etc.)
 */
int
LineOnScreen (Doc, hTargBlock, TargOffset, TargLineID)
     TypDoc *Doc;
     HANDLE hTargBlock;
     unsigned int TargOffset;
     TypLineID TargLineID;
{
  TypBlock far *CurBlockPtr, far * TargBlockPtr;
  TypLine far *CurLinePtr, far * TargLinePtr;
  int iline;
  BOOL found = FALSE, gotnext=TRUE;
  HANDLE hBlock;
  unsigned int Offset;
  TypLineID MyLineID;

  LockLine (Doc->hCurTopScBlock, Doc->TopScOffset, Doc->TopScLineID, &CurBlockPtr, &CurLinePtr);
  LockLine (hTargBlock, TargOffset, TargLineID, &TargBlockPtr, &TargLinePtr);

  for (iline = 0; gotnext && !found && (unsigned) iline < Doc->ScYLines;)
    {
      found = (TargLinePtr == CurLinePtr);
      if(CurLinePtr->active) iline++;
      if (!found)
   {
     gotnext = NextLine (&CurBlockPtr, &CurLinePtr);
   }
    }

  UnlockLine (CurBlockPtr, CurLinePtr, &hBlock, &Offset, &MyLineID);
  UnlockLine (TargBlockPtr, TargLinePtr, &hBlock, &Offset, &MyLineID);

  if (!found)
    iline = 0;
  return (iline - 1);
}

/*--- function NextWindow ----------------------------------------------
 *
 *  Makes another window the active window with input focus.
 *  This function would typically be used to implement a keystroke
 *  (usually F6) that switches windows.
 *
 *    Entry    Doc   points to the current document.
 */
void
NextWindow (Doc)
     TypDoc *Doc;
{
  int idoc;
  int trydoctype;
  TypDoc *NewDoc;

  /* First, locate the current window in our data structures.       */

  switch (Doc->DocType)
    {
    case DOCTYPE_NET:
      idoc = 0;
      trydoctype = DOCTYPE_GROUP;
      break;
    case DOCTYPE_GROUP:
      for (idoc = 0; idoc < MAXGROUPWNDS && Doc != &(GroupDocs[idoc]); idoc++);
      if (idoc >= MAXGROUPWNDS)
   {
     MessageBox (Doc->hDocWnd, "Error finding next window", "System error", MB_ICONHAND | MB_OK);
   }
      trydoctype = DOCTYPE_GROUP;
      idoc++;
      break;
    case DOCTYPE_ARTICLE:
      for (idoc = 0; idoc < MAXARTICLEWNDS && Doc != &(ArticleDocs[idoc]); idoc++);
      if (idoc >= MAXARTICLEWNDS)
   {
     MessageBox (Doc->hDocWnd, "Error finding next window", "System error", MB_ICONHAND | MB_OK);
   }
      trydoctype = DOCTYPE_ARTICLE;
      idoc++;
      break;
    }

  /* Now, find the next window in the sequence.                     */

  if (trydoctype == DOCTYPE_GROUP)
    {
      for (; idoc < MAXGROUPWNDS; idoc++)
   {
     if (GroupDocs[idoc].InUse)
       {
         NewDoc = ActiveGroupDoc = &(GroupDocs[idoc]);
         goto foundit;
       }
   }
      idoc = 0;
    }

  /* Not found yet--try to find an Article doc.                     */

  for (; idoc < MAXARTICLEWNDS; idoc++)
    {
      if (ArticleDocs[idoc].InUse)
   {
     NewDoc = ActiveArticleDoc = &(ArticleDocs[idoc]);
     goto foundit;
   }
    }

  /* Still not found--just make the Net document the next one.      */

  NewDoc = &NetDoc;

foundit:;

  SetActiveWindow (NewDoc->hDocWnd);
  SetFocus (NewDoc->hDocWnd);

}

/*--- function AdjustScTop -----------------------------------
 *
 *  Adjust the top line of the screen so that a given line is sure
 *  to appear on the screen.
 *  Don't do anything if the document is one screen's length or smaller.
 *
 *    Entry
 *             BlockPtr and LinePtr  point to the line we want to
 *                      make sure is visible.
 *
 *    Exit     Doc's TopSc fields have been set to ensure that
 *               the line will appear on the screen.
 *             The line has been unlocked.
 */

void
AdjustTopSc (BlockPtr, LinePtr)
     TypBlock far *BlockPtr;
     TypLine far *LinePtr;
{
  unsigned int lineord;
  TypDoc *Doc;
  HANDLE hBlock;
  unsigned int Offset;
  TypLineID MyLineID;

  Doc = BlockPtr->OwnerDoc;
  if (Doc->TotalLines > Doc->ScYLines)
  //if (Doc->ActiveLines > Doc->ScYLines)  //??
    {
      lineord = WhatLine (BlockPtr, LinePtr);

      while (lineord > Doc->TotalLines - Doc->ScYLines)
      //while (lineord > Doc->ActiveLines - Doc->ScYLines)  //??
   {
     PrevLine (&BlockPtr, &LinePtr);
     lineord--;
   }
      Doc->TopLineOrd = lineord;
      UnlockLine (BlockPtr, LinePtr, &(Doc->hCurTopScBlock),
        &(Doc->TopScOffset), &(Doc->TopScLineID));
    }
  else
    {
      UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
    }
}

/*
 * This is a Doc/line-index version of AdjustScTop
 * Don't you wish we could overload the function?!
 * JSC 8/13/94
 */
void
AdjustTopScByDoc (TypDoc *aDoc, unsigned int index)
{
	if (aDoc->TotalLines < aDoc->ScYLines ||
	   index >= aDoc->TopLineOrd && index < aDoc->TopLineOrd + aDoc->ScYLines)
		return;		// already on screen
	
	if (index <= aDoc->ScYLines/2)
		aDoc->TopLineOrd = 0;
	else
		aDoc->TopLineOrd = index - aDoc->ScYLines/2;	// center it

	InvalidateRect(aDoc->hDocWnd, NULL, FALSE);
}

/*--- function ScreenToTop -------------------------------------
 *
 *  Sets a document so that the screen is scrolled to the top.
 *
 *  Entry   Doc     points to the document.
 *
 *  Exit    The document has been set to display starting at
 *            the first line.
 */

void
ScreenToTop (TypDoc * Doc)
{ 
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
  
  Doc->hCurTopScBlock = Doc->hFirstBlock;
  Doc->TopScOffset = sizeof (TypBlock);
  Doc->TopScLineID = 0;
  Doc->TopLineOrd = 0;
  
  LockLine(Doc->hFirstBlock,Doc->TopScOffset,Doc->TopScLineID,&BlockPtr,&LinePtr);
  AdvanceToActive(&BlockPtr,&LinePtr);
  UnlockLine(BlockPtr,LinePtr,&(Doc->hFirstBlock),&(Doc->TopScOffset),&(Doc->TopScLineID));
}   

/*--- function AdvanceToActive ---------------------------------------
 *
 *  Advances a block/line pointer pair to an active line.
 *  The pointers are unchanged if the current line is active.
 *
 *  Entry:  BlockPtr, LinePtr points to a line.
 *
 *  Exit:   BlockPtr,LinePtr  points to an active line, if possible.
 *          Returns TRUE if successful, else FALSE if no line from
 *            this point to the end of the document was active.
 */
BOOL
AdvanceToActive(BlockPtr, LinePtr)
     TypBlock far **BlockPtr;
     TypLine far **LinePtr;
{
   BOOL ok=TRUE;
   
   while(!((*LinePtr)->active)) {
      if(!NextLine(BlockPtr,LinePtr)) {
         ok = FALSE;
         break;
      }
   }   
   
   return ok;
}
 

/*--- function SetHandleBkBrush ---------------------------------------
 *
 *  Sets a new handle of a background brush.
 *  This uses SetClassWord in a portable way and makes code look tidier.
 *
 *  Entry:  hwnd 	Identifies window whose brush is to be changed.
 *			handle	Handle to the new brush.
 *
 */
void SetHandleBkBrush(HWND hwnd, HBRUSH handle)
{
    #ifdef _WIN32
      SetClassLong (hwnd, GCL_HBRBACKGROUND, (LONG)handle);
	#else
      SetClassWord(hwnd, GCW_HBRBACKGROUND, handle);
	#endif
}


/*-- Last Line of WVSCREEN.C ----------------- */
@


1.12
log
@copy to clipboard
@
text
@d4 1
a4 1
 * $Id: wvscreen.c 1.11 1994/07/12 19:51:26 cnolan Exp $
d6 3
d791 20
@


1.11
log
@win32ism
@
text
@d4 1
a4 1
 * $Id: wvscreen.c 1.10 1994/06/01 19:08:20 gardnerd Exp $
d6 3
d349 2
a350 2

  if (Document->DocType == DOCTYPE_ARTICLE)
d352 2
a353 2
   window_lineheight = ArtLineHeight;
   window_topspace = ArtTopSpace; 
d357 4
a360 4
   window_lineheight = LineHeight;
   window_topspace = TopSpace;
  }

d375 1
a375 1
     ScrollWindow (Document->hDocWnd, 0, LinesGone * window_lineheight, NULL, NULL);
d398 1
a398 1
     ScrollWindow (Document->hDocWnd, 0, -LinesGone * window_lineheight, NULL, NULL);
d512 20
a531 3
    {
      Document->ScXOffset--; 
      InvalidateRect(Document->hDocWnd, NULL, FALSE);
d537 3
a539 1
    {
d541 17
a557 1
      InvalidateRect(Document->hDocWnd, NULL, FALSE);
@


1.10
log
@horizontal scrolling support
@
text
@d4 1
a4 1
 * $Id: wvscreen.c 1.9 1994/02/24 21:34:08 jcoop Exp $
d6 3
d553 4
d558 3
@


1.9
log
@jcoop changes
@
text
@d4 1
a4 1
 * $Id: wvscreen.c 1.8 1994/02/09 18:01:08 cnolan Exp $
d6 3
d436 1
a436 1

d482 80
@


1.8
log
@cnolan 90.2 changes
@
text
@d4 1
a4 1
 * $Id: wvscreen.c 1.7 1994/01/12 19:27:32 mrr Exp $
d6 3
d153 12
d184 1
a184 1
     ScrollWindow (Document->hDocWnd, 0, LinesGone * LineHeight, NULL, NULL);
d186 2
a187 2
     Rect.top = TopSpace;
     Rect.bottom = TopSpace + LinesGone * LineHeight;
d192 1
a192 1
     Rect.top = TopSpace + Document->ScYLines * LineHeight;
d214 2
a215 2
     ScrollWindow (Document->hDocWnd, 0, -LinesGone * LineHeight, NULL, NULL);
     Rect.top = TopSpace + (Document->ScYLines - LinesGone) * LineHeight;
d220 1
a220 1
     Rect.bottom = TopSpace;
d339 1
a339 1
  int window_lineheight;
d342 1
d344 2
d347 1
d349 2
d368 2
a369 2
     Rect.top = TopSpace;
     Rect.bottom = TopSpace + LinesGone * window_lineheight;
d373 1
a373 1
     Rect.top = TopSpace + Document->ScYLines * window_lineheight;
d390 1
a390 1
     Rect.top = TopSpace + (Document->ScYLines - LinesGone) * window_lineheight;
d395 1
a395 1
     Rect.bottom = TopSpace;
@


1.7
log
@mrr mods 4
@
text
@d4 1
a4 1
 * $Id: wvscreen.c 1.6 1993/12/08 01:28:01 rushing Exp $
d6 3
d617 2
a618 1
  if (Doc->ActiveLines > Doc->ScYLines)
d622 2
a623 1
      while (lineord > Doc->ActiveLines - Doc->ScYLines)
d692 19
@


1.6
log
@new version box and cr lf consistency
@
text
@d4 1
a4 1
 * $Id: wvscreen.c 1.5 1993/07/06 20:53:23 rushing Exp rushing $
d6 3
d72 5
a76 2
  for (LinesGone = TestAdvance; LinesGone && NextLine (&MyBlock, &MyLine);
       LinesGone--);
d85 1
a85 1
  while (nLines--)
d88 1
d116 6
a121 3
  while (nLines-- && PrevLine (BlockPtr, LinePtr))
    {
      (*LinesBackedUp)++;
d125 1
a125 1
/*--- function ScrollIt ----------------------------------------------
d134 1
a134 1
 *	  except in WIN32 ...
d136 1
a136 1
 *			   wParam	LOWORD: SB_code, HIWORD: position.
d161 17
a177 17
	{
	  Rect.left = 0;
	  Rect.right = Document->ScXWidth;
	  Document->TopLineOrd -= LinesGone;

	  ScrollWindow (Document->hDocWnd, 0, LinesGone * LineHeight, NULL, NULL);

	  Rect.top = TopSpace;
	  Rect.bottom = TopSpace + LinesGone * LineHeight;

	  InvalidateRect (Document->hDocWnd, &Rect, FALSE);

	  /* Make sure garbage at bottom is erased in WM_PAINT processing */
	  Rect.top = TopSpace + Document->ScYLines * LineHeight;
	  Rect.bottom = Document->ScYHeight;
	  InvalidateRect (Document->hDocWnd, &Rect, FALSE);
	}
d181 5
a185 2

   if (Document->TopLineOrd < (Document->TotalLines - Document->ScYLines)) {
d192 13
a204 13
	{
	  Document->TopLineOrd += LinesGone;
	  Rect.left = 0;
	  Rect.right = Document->ScXWidth;
	  ScrollWindow (Document->hDocWnd, 0, -LinesGone * LineHeight, NULL, NULL);
	  Rect.top = TopSpace + (Document->ScYLines - LinesGone) * LineHeight;
	  Rect.bottom = Document->ScYHeight;
	  InvalidateRect (Document->hDocWnd, &Rect, FALSE);
	  /* Make sure garbage at top is erased in WM_PAINT processing */
	  Rect.top = 0;
	  Rect.bottom = TopSpace;
	  InvalidateRect (Document->hDocWnd, &Rect, FALSE);
	}
d212 1
a212 1
	LinesGone = Document->ScYLines - 1;
d214 3
a216 3
	else {
	LinesGone = Document->TopLineOrd;
	}
d224 3
a226 3
   if (Document->TotalLines > Document->ScYLines) { /* do we even need to scroll? */
     if ((Document->TotalLines -
	  (Document->TopLineOrd + Document->ScYLines)) > Document->ScYLines)
d229 1
a229 1
       LinesGone = Document->TotalLines - (Document->TopLineOrd + Document->ScYLines );
d243 3
a245 3
	{
	  return;
	}
d260 3
a262 3
	{
	  return;
	}
d266 10
a275 10
	{
	  if (LinesGone >= (int) (Document->ScYLines - 1))
	    {
	      goto doposition;
	    }
	  else
	    {
	      goto doscrolldown;
	    }
	}
d277 11
a287 11
	{
	  LinesGone = -LinesGone;
	  if (LinesGone >= (int) (Document->ScYLines - 1))
	    {
	      goto doposition;
	    }
	  else
	    {
	      goto doscrollup;
	    }
	}
d297 1
a297 1
	      &(Document->TopScOffset), &(Document->TopScLineID));
d334 1
a334 1

d338 15
a352 15
	{
	  Rect.left = 0;
	  Rect.right = Document->ScXWidth;
	  Document->TopLineOrd -= LinesGone;
	  ScrollWindow (Document->hDocWnd, 0, LinesGone * window_lineheight, NULL, NULL);

	  Rect.top = TopSpace;
	  Rect.bottom = TopSpace + LinesGone * window_lineheight;

	  InvalidateRect (Document->hDocWnd, &Rect, FALSE);
	  /* Make sure garbage at bottom is erased in WM_PAINT processing */
	  Rect.top = TopSpace + Document->ScYLines * window_lineheight;
	  Rect.bottom = Document->ScYHeight;
	  InvalidateRect (Document->hDocWnd, &Rect, FALSE);
	}
d357 1
d361 13
a373 13
	{
	  Document->TopLineOrd += LinesGone;
	  Rect.left = 0;
	  Rect.right = Document->ScXWidth;
	  ScrollWindow (Document->hDocWnd, 0, -LinesGone * window_lineheight, NULL, NULL);
	  Rect.top = TopSpace + (Document->ScYLines - LinesGone) * window_lineheight;
	  Rect.bottom = Document->ScYHeight;
	  InvalidateRect (Document->hDocWnd, &Rect, FALSE);
	  /* Make sure garbage at top is erased in WM_PAINT processing */
	  Rect.top = 0;
	  Rect.bottom = TopSpace;
	  InvalidateRect (Document->hDocWnd, &Rect, FALSE);
	}
d378 1
d386 1
d388 1
a388 1
		  &BlockPtr, &LinePtr, &LinesGone);
d400 3
a402 3
	{
	  return;
	}
d416 3
a418 3
	{
	  return;
	}
d421 10
a430 10
	{
	  if (LinesGone >= (int) (Document->ScYLines - 1))
	    {
	      goto doposition;
	    }
	  else
	    {
	      goto doscrolldown;
	    }
	}
d432 11
a442 11
	{
	  LinesGone = -LinesGone;
	  if (LinesGone >= (int) (Document->ScYLines - 1))
	    {
	      goto doposition;
	    }
	  else
	    {
	      goto doscrollup;
	    }
	}
d452 1
a452 1
	      &(Document->TopScOffset), &(Document->TopScLineID));
d480 1
a480 1
  BOOL found = FALSE;
d488 1
a488 1
  for (iline = 0; !found && (unsigned) iline < Doc->ScYLines; iline++)
d491 1
d493 3
a495 3
	{
	  NextLine (&CurBlockPtr, &CurLinePtr);
	}
d533 3
a535 3
	{
	  MessageBox (Doc->hDocWnd, "Error finding next window", "System error", MB_ICONHAND | MB_OK);
	}
d542 3
a544 3
	{
	  MessageBox (Doc->hDocWnd, "Error finding next window", "System error", MB_ICONHAND | MB_OK);
	}
d555 7
a561 7
	{
	  if (GroupDocs[idoc].InUse)
	    {
	      NewDoc = ActiveGroupDoc = &(GroupDocs[idoc]);
	      goto foundit;
	    }
	}
d570 4
a573 4
	{
	  NewDoc = ActiveArticleDoc = &(ArticleDocs[idoc]);
	  goto foundit;
	}
d614 1
a614 1
  if (Doc->TotalLines > Doc->ScYLines)
d618 5
a622 5
      while (lineord > Doc->TotalLines - Doc->ScYLines)
	{
	  PrevLine (&BlockPtr, &LinePtr);
	  lineord--;
	}
d625 1
a625 1
		  &(Doc->TopScOffset), &(Doc->TopScLineID));
d639 2
a640 2
 *  Exit 	The document has been set to display starting at
 * 			  the first line.
d645 4
a648 1
{
d653 32
a684 1

d686 1
@


1.5
log
@type fixed
@
text
@d1 1
d4 1
a4 1
 * $Id: wvscreen.c 1.4 1993/07/06 20:52:28 cnolan Exp rushing $
d6 3
@


1.4
log
@win32 changes
@
text
@d3 1
a3 1
 * $Id: wvscreen.c 1.3 1993/06/28 17:51:39 rushing Exp $
d5 3
d223 1
a223 1
       LineOrd = LOWORS (lParam);
d240 1
a240 1
       LineOrd = LOWORS (lParam);
d377 1
a377 1
       LineOrd = LOWORS (lParam);
d393 1
a393 1
       LineOrd = LOWORS (lParam);
@


1.3
log
@fixed compiler warnings
@
text
@d3 1
a3 1
 * $Id: wvscreen.c 1.2 1993/05/26 18:30:40 rushing Exp rushing $
d5 3
d116 4
d124 2
a125 2
     WORD wParam;
     DWORD lParam;
d131 1
a131 1
  switch (wParam)
d217 5
a221 1
      LineOrd = LOWORD (lParam);
d234 5
a238 1
      LineOrd = LOWORD (lParam);
d293 2
a294 2
     WORD wParam;
     DWORD lParam;
d308 1
a308 1
  switch (wParam)
d371 5
a375 1
      LineOrd = LOWORD (lParam);
d387 5
a391 1
      LineOrd = LOWORD (lParam);
@


1.2
log
@fixed page down bug (GPE when #arts < what can fit in window)
@
text
@d3 1
a3 1
 * $Id: wvscreen.c 1.1 1993/02/16 20:54:22 rushing Exp rushing $
d5 3
d232 1
a232 1
	  if (LinesGone >= (Document->ScYLines - 1))
d244 1
a244 1
	  if (LinesGone >= (Document->ScYLines - 1))
d376 1
a376 1
	  if (LinesGone >= (Document->ScYLines - 1))
d388 1
a388 1
	  if (LinesGone >= (Document->ScYLines - 1))
d442 1
a442 1
  for (iline = 0; !found && iline < Doc->ScYLines; iline++)
@


1.1
log
@Initial revision
@
text
@d3 4
a6 2
 * $Id$
 * $Log$
d8 1
d192 1
a192 7
    case SB_PAGEDOWN:

      if ((Document->TotalLines - (Document->TopLineOrd + Document->ScYLines)) > Document->ScYLines)
	LinesGone = Document->ScYLines - 1;
      else
        LinesGone = Document->TotalLines
                    - (Document->TopLineOrd + Document->ScYLines );
d194 11
a204 3
      Document->TopLineOrd += LinesGone;
      InvalidateRect (Document->hDocWnd, NULL, FALSE);
      break;
d206 1
a206 1
    case SB_THUMBPOSITION:
@
