
/*
 *
 * $Id: wvscreen.c 1.6 1993/12/08 01:28:01 rushing Exp $
 * $Log: wvscreen.c $
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

     ScrollWindow (Document->hDocWnd, 0, LinesGone * LineHeight, NULL, NULL);

     Rect.top = TopSpace;
     Rect.bottom = TopSpace + LinesGone * LineHeight;

     InvalidateRect (Document->hDocWnd, &Rect, FALSE);

     /* Make sure garbage at bottom is erased in WM_PAINT processing */
     Rect.top = TopSpace + Document->ScYLines * LineHeight;
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
     ScrollWindow (Document->hDocWnd, 0, -LinesGone * LineHeight, NULL, NULL);
     Rect.top = TopSpace + (Document->ScYLines - LinesGone) * LineHeight;
     Rect.bottom = Document->ScYHeight;
     InvalidateRect (Document->hDocWnd, &Rect, FALSE);
     /* Make sure garbage at top is erased in WM_PAINT processing */
     Rect.top = 0;
     Rect.bottom = TopSpace;
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
  int window_lineheight;

  if (Document->DocType == DOCTYPE_ARTICLE)
   window_lineheight = ArtLineHeight;
  else
   window_lineheight = LineHeight;

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
     ScrollWindow (Document->hDocWnd, 0, LinesGone * window_lineheight, NULL, NULL);

     Rect.top = TopSpace;
     Rect.bottom = TopSpace + LinesGone * window_lineheight;

     InvalidateRect (Document->hDocWnd, &Rect, FALSE);
     /* Make sure garbage at bottom is erased in WM_PAINT processing */
     Rect.top = TopSpace + Document->ScYLines * window_lineheight;
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
     ScrollWindow (Document->hDocWnd, 0, -LinesGone * window_lineheight, NULL, NULL);
     Rect.top = TopSpace + (Document->ScYLines - LinesGone) * window_lineheight;
     Rect.bottom = Document->ScYHeight;
     InvalidateRect (Document->hDocWnd, &Rect, FALSE);
     /* Make sure garbage at top is erased in WM_PAINT processing */
     Rect.top = 0;
     Rect.bottom = TopSpace;
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
  if (Doc->ActiveLines > Doc->ScYLines)
    {
      lineord = WhatLine (BlockPtr, LinePtr);

      while (lineord > Doc->ActiveLines - Doc->ScYLines)
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
 

/*-- Last Line of WVSCREEN.C ----------------- */
