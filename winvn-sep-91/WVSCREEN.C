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
ScreenDown(nLines,LinesOnScreen,BlockPtr,LinePtr,LinesAdvanced)
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

   PtrToOffset(MyBlock,MyLine,&hBlock,&Offset,&MyLineID);
   TestAdvance = nLines + LinesOnScreen - 1;
   for(LinesGone=TestAdvance; LinesGone && NextLine(&MyBlock,&MyLine);
     LinesGone--);
   UnlockLine(MyBlock,MyLine,&hBlockGarbage,&OffsetGarbage,&LineIDGarbage);

   nLines -= LinesGone;
   if(nLines<0) nLines = 0;
   *LinesAdvanced = nLines;

   LockLine(hBlock,Offset,MyLineID,&MyBlock,&MyLine);
   while(nLines--) {
      NextLine(BlockPtr,LinePtr);
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
ScreenUp(nLines,BlockPtr,LinePtr,LinesBackedUp)
int nLines;
TypBlock far **BlockPtr;
TypLine far **LinePtr;
int *LinesBackedUp;
{
   *LinesBackedUp = 0;

   while(nLines-- && PrevLine(BlockPtr,LinePtr)) {
      (*LinesBackedUp)++;
   }
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
ScrollIt(Document,wParam,lParam)
TypDoc *Document;
WORD wParam;
DWORD lParam;
{
   TypBlock far *BlockPtr, far *NewBlockPtr;
   TypLine  far *LinePtr, far *NewLinePtr;
   int LinesGone;
   unsigned int LineOrd;
   RECT Rect;

   switch(wParam){
      case SB_LINEUP:
         /* Move up a line by scrolling the window down one line     */
         /* and introducing 1 new line at the top.                   */
         LockLine(Document->hCurTopScBlock,Document->TopScOffset,Document->TopScLineID,&BlockPtr,&LinePtr);
         ScreenUp(1,&BlockPtr,&LinePtr,&LinesGone);
        doscrollup:;
         if(LinesGone) {
            Rect.left = 0;
            Rect.right = Document->ScXWidth;
            Document->TopLineOrd -= LinesGone;
#if 0
            Rect.top = TopSpace;
            Rect.bottom = Document->ScYHeight;
            ScrollWindow(Document->hDocWnd,0,LinesGone*LineHeight,&Rect,&Rect);
#endif
            ScrollWindow(Document->hDocWnd,0,LinesGone*LineHeight,NULL,NULL);
#ifdef MAC
            Rect.top = 0;
            Rect.bottom = LinesGone*LineHeight;
#else
            Rect.top = TopSpace;
            Rect.bottom = TopSpace+LinesGone*LineHeight;
#endif
            InvalidateRect(Document->hDocWnd,&Rect,FALSE);
            /* Make sure garbage at bottom is erased in WM_PAINT processing */
            Rect.top = TopSpace + Document->ScYLines*LineHeight;
            Rect.bottom = Document->ScYHeight;
            InvalidateRect(Document->hDocWnd,&Rect,FALSE);
         }
         break;

      case SB_LINEDOWN:
         LockLine(Document->hCurTopScBlock,Document->TopScOffset,Document->TopScLineID,&BlockPtr,&LinePtr);
         ScreenDown(1,Document->ScYLines,&BlockPtr,&LinePtr,&LinesGone);
       doscrolldown:;
         if(LinesGone) {
            Document->TopLineOrd += LinesGone;
            Rect.left = 0;
            Rect.right = Document->ScXWidth;
#if 0
            Rect.top = TopSpace;
            Rect.bottom = Document->ScYHeight;
            ScrollWindow(Document->hDocWnd,0,-LinesGone*LineHeight,&Rect,&Rect);
#endif
            ScrollWindow(Document->hDocWnd,0,-LinesGone*LineHeight,NULL,NULL);
#ifdef MAC
            Rect.top = (Document->ScYLines-LinesGone)*LineHeight;
            Rect.bottom = Document->ScYHeight;
#else
            Rect.top = TopSpace + (Document->ScYLines-LinesGone)*LineHeight;
            Rect.bottom = Document->ScYHeight;
#endif
            InvalidateRect(Document->hDocWnd,&Rect,FALSE);
            /* Make sure garbage at top is erased in WM_PAINT processing */
            Rect.top = 0;
            Rect.bottom = TopSpace;
            InvalidateRect(Document->hDocWnd,&Rect,FALSE);
         }
         break;

      case SB_PAGEUP:
         LockLine(Document->hCurTopScBlock,Document->TopScOffset,Document->TopScLineID,&BlockPtr,&LinePtr);
         ScreenUp(Document->ScYLines-1,&BlockPtr,&LinePtr,&LinesGone);
         Document->TopLineOrd -= LinesGone;
         InvalidateRect(Document->hDocWnd, NULL, FALSE);
         break;

      case SB_PAGEDOWN:
         LockLine(Document->hCurTopScBlock,Document->TopScOffset,Document->TopScLineID,&BlockPtr,&LinePtr);
         ScreenDown(Document->ScYLines-1,Document->ScYLines,
          &BlockPtr,&LinePtr,&LinesGone);
         Document->TopLineOrd += LinesGone;
         InvalidateRect(Document->hDocWnd, NULL, FALSE);
         break;

      case SB_THUMBPOSITION:
         LineOrd = LOWORD(lParam);
         if(!FindLineOrd(Document,LineOrd,&BlockPtr,&LinePtr)) {
            return;
         }
       doposition:;
         Document->TopLineOrd = LineOrd;
         InvalidateRect(Document->hDocWnd, NULL, FALSE);
         break;

      case SB_THUMBTRACK:
         LineOrd = LOWORD(lParam);
         if(!FindLineOrd(Document,LineOrd,&BlockPtr,&LinePtr)) {
            return;
         }
         LinesGone = LineOrd - Document->TopLineOrd;
         if(LinesGone > 0) {
            if(LinesGone >= (Document->ScYLines-1)) {
               goto doposition;
            } else {
               goto doscrolldown;
            }
         } else if(LinesGone < 0) {
            LinesGone = -LinesGone;
            if(LinesGone >= (Document->ScYLines-1)) {
               goto doposition;
            } else {
               goto doscrollup;
            }
         }
         break;

      default:
         return;
         break;
   }

   UnlockLine(BlockPtr,LinePtr,&(Document->hCurTopScBlock),
     &(Document->TopScOffset),&(Document->TopScLineID));
#ifdef MAC
/* SendMessage(Document->hDocWnd,WM_PAINT,0,0L); */
   MyUpdateWindow(Document->hDocWnd);
#endif

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
LineOnScreen(Doc,hTargBlock,TargOffset,TargLineID)
TypDoc *Doc;
HANDLE hTargBlock;
unsigned int TargOffset;
TypLineID TargLineID;
{
   TypBlock far *CurBlockPtr, far *TargBlockPtr;
   TypLine  far *CurLinePtr,  far *TargLinePtr;
   int iline;
   BOOL found=FALSE;
   HANDLE hBlock;
   unsigned int Offset;
   TypLineID  MyLineID;

   LockLine(Doc->hCurTopScBlock,Doc->TopScOffset,Doc->TopScLineID,&CurBlockPtr,&CurLinePtr);
   LockLine(hTargBlock,TargOffset,TargLineID,&TargBlockPtr,&TargLinePtr);

   for(iline=0; !found && iline < Doc->ScYLines; iline++) {
      found = (TargLinePtr == CurLinePtr);
      if(!found) {
         NextLine(&CurBlockPtr,&CurLinePtr);
      }
   }

   UnlockLine(CurBlockPtr,CurLinePtr,&hBlock,&Offset,&MyLineID);
   UnlockLine(TargBlockPtr,TargLinePtr,&hBlock,&Offset,&MyLineID);

   if(!found) iline = 0;
   return(iline-1);
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
NextWindow(Doc)
TypDoc *Doc;
{
   int idoc;
   int trydoctype;
   TypDoc *NewDoc;

   /* First, locate the current window in our data structures.       */

   switch(Doc->DocType) {
      case DOCTYPE_NET:
         idoc = 0;
         trydoctype = DOCTYPE_GROUP;
         break;
      case DOCTYPE_GROUP:
         for(idoc=0; idoc < MAXGROUPWNDS && Doc != &(GroupDocs[idoc]); idoc++);
         if(idoc >= MAXGROUPWNDS) {
            MessageBox(Doc->hDocWnd,"Error finding next window", "System error",MB_ICONHAND|MB_OK);
         }
         trydoctype = DOCTYPE_GROUP;
         idoc++;
         break;
      case DOCTYPE_ARTICLE:
         for(idoc=0; idoc < MAXARTICLEWNDS && Doc != &(ArticleDocs[idoc]); idoc++);
         if(idoc >= MAXARTICLEWNDS) {
            MessageBox(Doc->hDocWnd,"Error finding next window", "System error",MB_ICONHAND|MB_OK);
         }
         trydoctype = DOCTYPE_ARTICLE;
         idoc++;
         break;
   }

   /* Now, find the next window in the sequence.                     */

   if(trydoctype == DOCTYPE_GROUP) {
      for(;idoc < MAXGROUPWNDS ; idoc++) {
         if(GroupDocs[idoc].InUse) {
            NewDoc = ActiveGroupDoc = &(GroupDocs[idoc]);
            goto foundit;
         }
      }
      idoc = 0;
   }

   /* Not found yet--try to find an Article doc.                     */

   for(;idoc < MAXARTICLEWNDS ; idoc++) {
      if(ArticleDocs[idoc].InUse) {
         NewDoc = ActiveArticleDoc = &(ArticleDocs[idoc]);
         goto foundit;
      }
   }

   /* Still not found--just make the Net document the next one.      */

   NewDoc = &NetDoc;

foundit:;

   SetActiveWindow(NewDoc->hDocWnd);
   SetFocus(NewDoc->hDocWnd);

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
AdjustTopSc(BlockPtr,LinePtr)
TypBlock far *BlockPtr;
TypLine far *LinePtr;
{
   unsigned int lineord;
   TypDoc *Doc;
   HANDLE hBlock;
   unsigned int Offset;
   TypLineID MyLineID;

   Doc = BlockPtr->OwnerDoc;
   if(Doc->TotalLines > Doc->ScYLines) {
      lineord = WhatLine(BlockPtr,LinePtr);

      while(lineord > Doc->TotalLines - Doc->ScYLines) {
         PrevLine(&BlockPtr,&LinePtr);
         lineord--;
      }
      Doc->TopLineOrd = lineord;
      UnlockLine(BlockPtr,LinePtr,&(Doc->hCurTopScBlock),
         &(Doc->TopScOffset),&(Doc->TopScLineID));
   } else {
      UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
   }
}

/*--- function ScreenToTop -------------------------------------
 *
 *  Sets a document so that the screen is scrolled to the top.
 *
 *  Entry   Doc     points to the document.
 *
 *  Exit 	The document has been set to display starting at
 * 			  the first line.
 */

void
ScreenToTop(TypDoc *Doc)
{
	Doc->hCurTopScBlock = Doc->hFirstBlock;
	Doc->TopScOffset = sizeof(TypBlock);
	Doc->TopScLineID = 0;
	Doc->TopLineOrd = 0;

}
/*-- Last Line of WVSCREEN.C ----------------- */
