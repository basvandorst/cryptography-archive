
/*
 *
 * $Id: wvblock.c 1.4 1993/12/08 01:27:21 rushing Exp $
 * $Log: wvblock.c $
 * Revision 1.4  1993/12/08  01:27:21  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.3  1993/06/28  17:53:24  rushing
 * fixed compiler warnings
 *
 * Revision 1.2  1993/05/24  23:56:18  rushing
 * Doc->HeaderLines = 0 (MRB)
 *
 * Revision 1.1  1993/02/16  20:53:50  rushing
 * Initial revision
 *
 *
 */

/* -- WVBLOCK.C --------------------------------------------------
 *
 *  This file contains a collection of routines to manipulate textblocks
 *  and lines within textblocks.
 *  The routines here view lines as atomic units only (they don't
 *  look at the actual data in the lines).
 *
 *  Mark Riordan   20 September 1989.
 */

#include "windows.h"
#include "wvglob.h"
#include "winvn.h"
#include <ctype.h>

/*-- function NewBlock ------------------------------------------
 *
 *  Creates an empty, new textblock and links it into the list
 *  of blocks after a given block.
 *  After the call, both the old ("given") block and the new block
 *  are locked in memory.
 *
 *    Entry    CurBlockPtr    points to a block
 *
 *    Exit     NewBlockPtr    points to a block that has been linked
 *                            into the list just after CurBlockPtr.
 *             Returns TRUE if couldn't allocate a block, else FALSE.
 *             (I know I should fix that.)
 */
int
NewBlock (CurBlockPtr, NewBlockPtr)
     TypBlock far *CurBlockPtr, far ** NewBlockPtr;
{
  HANDLE hMyBlock;
  TypBlock far *MyBlock, far * MyNextBlock;

  hMyBlock = GlobalAlloc (GMEM_MOVEABLE, (long) (CurBlockPtr->OwnerDoc->BlockSize));
  if (hMyBlock)
    {
      MyBlock = (TypBlock far *) GlobalLock (hMyBlock);
      SetupEmptyBlock (MyBlock, hMyBlock, CurBlockPtr->hCurBlock,
             CurBlockPtr->hNextBlock, CurBlockPtr->OwnerDoc);
      CurBlockPtr->hNextBlock = hMyBlock;

      /* Change the next block's "previous" pointer to point to us. */

      if (MyBlock->hNextBlock)
   {
     MyNextBlock = (TypBlock far *) GlobalLock (MyBlock->hNextBlock);
     MyNextBlock->hPrevBlock = hMyBlock;
     GlobalUnlock (MyBlock->hNextBlock);
   }

      *NewBlockPtr = MyBlock;
    }
  else
    {
      MessageBox (CurBlockPtr->OwnerDoc->hDocWnd, "Could not allocate textblock", "Out of Memory Error", MB_OK | MB_ICONHAND);
      return (1);
    }
  return (0);
}

/*-- function SetupEmptyBlock -----------------------------------------
 *
 *  Initialize fields in a newly-allocated textblock.
 *  Set the fields to indicate an empty block.
 */
void
SetupEmptyBlock (BlockPtr, hCur, hPrev, hNext, DocPtr)
     TypBlock far *BlockPtr;
     HANDLE hCur, hPrev, hNext;
     TypDoc *DocPtr;
{
  BlockPtr->OwnerDoc = DocPtr;
  BlockPtr->hCurBlock = hCur;
  BlockPtr->hPrevBlock = hPrev;
  BlockPtr->hNextBlock = hNext;
  BlockPtr->LWAp1 = sizeof (TypBlock) + sizeof (TypLine);
  BlockPtr->NumLines = 0;
  BlockPtr->NumActiveLines = 0;
  BlockPtr->eob = END_OF_BLOCK;
  ((TypLine far *) ((char far *) BlockPtr + sizeof (TypBlock)))->length = END_OF_BLOCK;
  ((TypLine far *) ((char far *) BlockPtr + sizeof (TypBlock)))->LineID = NextLineID++;
}

/*-- function DeleteBlock ---------------------------------------------
 *
 *  Delete a textblock from a document.
 *
 *    Entry    CurBlockPtr points to the block to delete.
 *
 *    Exit     CurBlockPtr points to the next block, if any, else the
 *                         previous block.
 *             CurLinePtr  points to the first line of the next block
 *                         if there was one, else the last line of
 *                         the previous block.
 */
BOOL
DeleteBlock (TypBlock far ** CurBlockPtr, TypLine far ** CurLinePtr)
{
  TypBlock far *MyBlockPtr = *CurBlockPtr;
  TypBlock far *BlockPtr;
  HANDLE hMyBlock = MyBlockPtr->hCurBlock, hMyPrev = MyBlockPtr->hPrevBlock, hMyNext = MyBlockPtr->hNextBlock;
  BOOL set_cur_block = FALSE;

  /* Don't delete the only block in the document. */
  if (!hMyNext && !hMyPrev)
    return (FALSE);

  /* Update the prev pointer of the next block to point to
   * the block previous to the one being deleted.
   */

  if (hMyNext)
    {
      BlockPtr = (TypBlock far *) GlobalLock (hMyNext);
      BlockPtr->hPrevBlock = hMyPrev;
      GlobalUnlock (hMyNext);

      NextLine (CurBlockPtr, CurLinePtr);
      set_cur_block = TRUE;
    }
  else
    {
      /* The block we are deleting has no next block, so it
       * must be the last in the document.  Update document pointers.
       */
      MyBlockPtr->OwnerDoc->hLastBlock = hMyPrev;
    }

  /* Update the next pointer of the previous block to point to
   * the block after the one being deleted.
   */

  if (hMyPrev)
    {
      BlockPtr = (TypBlock far *) GlobalLock (hMyPrev);
      BlockPtr->hNextBlock = hMyNext;
      GlobalUnlock (hMyPrev);

      if (!set_cur_block)
   {
     /* There is no next block, so we want to position the
           * pointer to the end of the previous block.
           */
     PrevLine (CurBlockPtr, CurLinePtr);
     NextLine (CurBlockPtr, CurLinePtr);
   }
    }
  else
    {
      /* The block we are deleting has no previous block, so it
       * must be the first in the document.  Update document pointers.
       */
      MyBlockPtr->OwnerDoc->hFirstBlock = hMyNext;
    }

  GlobalFree (hMyBlock);
  return (TRUE);
}

/*-- function AddLine -------------------------------------------------
 *
 *  Add a line to a textblock.  Create a new textblock if necessary.
 *
 *    Entry    LineToAdd   points to a line to add.
 *             CurBlockPtr points to the block to which we want to add it.
 *             CurAddPtr   points to the place in the block to add the line.
 *
 *    Exit     CurBlockPtr & CurAddPtr point to right after the
 *                newly-added line.
 *
 *    Method   There are three cases:
 *             1. The new line will fit in the textblock.
 *                That's pretty easy.
 *             2. The new line won't fit, and we are at the end of
 *                the current block.
 *                I could have just treated this as I do case 3 (splitting
 *                the textblock), but that would have resulted in
 *                a lot of unnecessary internal fragmentation
 *                (i.e., wasted space inside textblocks).
 *                So, what I do is leave the contents of the old block
 *                alone and just create a new empty block, to which
 *                the line is added by a recursive call.
 *             3. The new line won't fit, and we are not at the end of
 *                the textblock.
 *                In this case, I split the textblock at the first line
 *                boundary after the split point (the split point is
 *                an attribute of the document--usually about 2/3 of the
 *                way through the textblock), creating two semi-full textblocks
 *                where there was one full one before.
 *                The routine then recursively calls itself.
 */
int
AddLine (LineToAdd, CurBlockPtr, CurAddPtr)
     TypLine *LineToAdd;
     TypBlock far **CurBlockPtr;
     TypLine far **CurAddPtr;
{
  TypBlock far *MyBlock = *CurBlockPtr;

  int left = (MyBlock->OwnerDoc->BlockSize - MyBlock->LWAp1);

  if (LineToAdd->length <= (int) left)
    {

      /* There's room in the current block for this line, so just    */
      /* move lines down to accomodate this line and copy it in.     */

      MoveBytes ((char far *) *CurAddPtr,
       ((char far *) *CurAddPtr) + LineToAdd->length,
       (((char far *) MyBlock + MyBlock->LWAp1)) - (char far *) *CurAddPtr);
      MoveBytes ((char far *) LineToAdd, (char far *) *CurAddPtr,
       LineToAdd->length);

      /* Adjust textblock counters.                                  */
      MyBlock->LWAp1 += LineToAdd->length;
      MyBlock->NumLines++;
      if(LineToAdd->active) MyBlock->NumActiveLines++;
      IncPtr ((*CurAddPtr), LineToAdd->length);
      MyBlock->OwnerDoc->TotalLines++;
    }
  else
    {

      /* There isn't enough room in the current textblock.           */
      /* If we are at the end of the current block, just create      */
      /* a new empty one; otherwise, split the current block.        */

      TypBlock far *NewBlockPtr;

      if ((*CurAddPtr)->length == END_OF_BLOCK)
   {
     /* We're at end of block; create a new empty one.           */
     /* This will be the current block, so release references    */
     /* to the now-current block                                 */
     if (NewBlock (MyBlock, CurBlockPtr))
       {
         return (1);
       }
     else
       {
         GlobalUnlock (MyBlock->hCurBlock);
         *CurAddPtr = (TypLine far *)
      ((char far *) *CurBlockPtr + sizeof (TypBlock));
         /* Now we can recursively try again to add the line.     */
         if (AddLine (LineToAdd, CurBlockPtr, CurAddPtr))
      {
        return (1);
      }
       }
   }
      else
   {
     /* We need to split the textblock.                          */
     /* Find a place to split the block by starting at the       */
     /* beginning of the block and skipping through the lines    */
     /* until we pass the number of bytes that marks the         */
     /* split point.  The previous line is the split point.      */

     TypLine far *MyLinePtr = (TypLine far *) ((char far *) MyBlock + sizeof (TypBlock));
     TypLine far *MyLastLine = MyLinePtr;
     int nOldLines = 0, nBytesMoved, MyAddOffset;

     while ( (unsigned)((char far *) MyLinePtr - (char far *) MyBlock) <
       MyBlock->OwnerDoc->SplitSize &&
       MyLinePtr->length != END_OF_BLOCK)
       {
         nOldLines++;
         MyLastLine = MyLinePtr;
         IncPtr (MyLinePtr, MyLinePtr->length);
       }

     /* Allocate the new block and copy the last portion of      */
     /* the current block to the new one.   The range to         */
     /* copy starts at the above-determined split point and      */
     /* goes until the LWA+1.                                    */
     /* Then adjust the new & old textblock fields.              */

     if (NewBlock (MyBlock, &NewBlockPtr))
       {
         return (1);
       }
     else
       {
         MoveBytes (MyLastLine, (char far *) NewBlockPtr + sizeof (TypBlock),
          nBytesMoved = (((char far *) MyBlock + MyBlock->LWAp1)) - (char far *) MyLastLine);
         MyBlock->LWAp1 = (char far *) MyLastLine - ((char far *) MyBlock)
      + sizeof (TypLine);
         ((TypLine far *) MyLastLine)->length = END_OF_BLOCK;
         ((TypLine far *) MyLastLine)->LineID = NextLineID++;
         NewBlockPtr->NumLines = MyBlock->NumLines - nOldLines;
         MyBlock->NumLines = nOldLines;
         NewBlockPtr->LWAp1 = nBytesMoved + sizeof (TypBlock);

         /* Should the new line go in the old block or the new?   */
         /* If the add point is beyond the end of the newly-      */
         /* truncated block, we must move the add point to the    */
         /* next block and make the new block the current one.    */
         /* The new position should be the same                   */
         /* number of line bytes past the beginning of the next   */
         /* block as it was past the split point when it was in   */
         /* the old block.                                        */
         /* Either way, one block (the old or the new one)        */
         /* must be unlocked.                                     */

         if (*CurAddPtr >= MyLastLine)
      {
        /* Add point is in new block.                         */

        MyAddOffset = (char far *) *CurAddPtr - (char far *) MyLastLine + sizeof (TypBlock);
        *CurAddPtr = (TypLine far *) ((char far *) NewBlockPtr + MyAddOffset);
        *CurBlockPtr = NewBlockPtr;
        GlobalUnlock (MyBlock->hCurBlock);
      }
         else
      {
        /* Add point is in current block.                     */
        GlobalUnlock (NewBlockPtr->hCurBlock);
      }
         return (AddLine (LineToAdd, CurBlockPtr, CurAddPtr));
       }
   }
    }
  return (0);
}

/*-- function ReplaceLine -------------------------------------------------
 *
 *  Replace a line in a textblock.  Create a new textblock if necessary.
 *
 *    Entry    LineToAdd   is the line to put into a textblock.
 *             CurBlockPtr points to the block containing the old copy.
 *             CurLinePtr  points to the old copy of the line.
 *
 *    Exit     returns TRUE if successful.
 *             CurBlockPtr points to the block containing the new copy of the line.
 *             CurLinePtr  points to the new copy of the line.
 *                         Usually, CurBlockPtr & CurAddPtr will be the
 *                         same upon exit as upon entry; however, sometimes
 *                         a textblock split is necessary.
 *
 *    Method   There are two cases:
 *             1. There is enough room in this textblock for the
 *                changed line.
 *                This is pretty simple.
 *             2. There is not enough room in this textblock for the
 *                changed line.  This requires a textblock split,
 *                as with AddLine.  There's a lot of common code
 *                here--should probably consolidate it in a single routine
 *                some day.
 */
BOOL
ReplaceLine (LineToAdd, CurBlockPtr, CurLinePtr)
     TypLine *LineToAdd;
     TypBlock far **CurBlockPtr;
     TypLine far **CurLinePtr;
{
  TypBlock far *MyBlockPtr = *CurBlockPtr;
  TypLine far *MyLinePtr = *CurLinePtr;
  int deltasize;
  int numbytes;
  char far *target, far * source;

  deltasize = LineToAdd->length - (MyLinePtr->length);

  if (deltasize <= (int) (MyBlockPtr->OwnerDoc->BlockSize - MyBlockPtr->LWAp1))
    {

      /* There's room in the current block for this line, so just    */
      /* move lines down to accomodate this line and copy it in.     */
      /* Move the data in the textblock up or down, starting with    */
      /* the line after the line being replaced.                     */

      source = (char far *) MyLinePtr + MyLinePtr->length;
      target = source + deltasize;
      numbytes = ((char far *) MyBlockPtr + MyBlockPtr->LWAp1) -
   (char far *) MyLinePtr - MyLinePtr->length;

      MoveBytes (source, target, numbytes);

      MoveBytes ((char far *) LineToAdd, (char far *) MyLinePtr,
       LineToAdd->length);
      MyBlockPtr->LWAp1 += deltasize;
    }
  else
    {

      /* There isn't enough room in the current textblock.           */
      /* We need to split the textblock.                          */
      /* Find a place to split the block by starting at the       */
      /* beginning of the block and skipping through the lines    */
      /* until we pass the number of bytes that marks the         */
      /* split point.  The previous line is the split point.      */

      TypBlock far *NewBlockPtr;

      TypLine far *MyLinePtr = (TypLine far *) ((char far *) MyBlockPtr + sizeof (TypBlock));
      TypLine far *MyLastLine = MyLinePtr;
      int nOldLines = 0, nBytesMoved, MyAddOffset;

      while ( (unsigned) ((char far *) MyLinePtr - (char far *) MyBlockPtr) <
        MyBlockPtr->OwnerDoc->SplitSize &&
        MyLinePtr->length != END_OF_BLOCK)
   {
     nOldLines++;
     MyLastLine = MyLinePtr;
     IncPtr (MyLinePtr, MyLinePtr->length);
   }

      /* Allocate the new block and copy the last portion of      */
      /* the current block to the new one.   The range to         */
      /* copy starts at the above-determined split point and      */
      /* goes until the LWA+1.                                    */
      /* Then adjust the new & old textblock fields.              */

      if (NewBlock (MyBlockPtr, &NewBlockPtr))
   {
     return (1);
   }
      else
   {
     MoveBytes (MyLastLine, (char far *) NewBlockPtr + sizeof (TypBlock),
           nBytesMoved = (((char far *) MyBlockPtr + MyBlockPtr->LWAp1)) - (char far *) MyLastLine);
     MyBlockPtr->LWAp1 = (char far *) MyLastLine - ((char far *) MyBlockPtr)
       + sizeof (TypLine);
     ((TypLine far *) MyLastLine)->length = END_OF_BLOCK;
     ((TypLine far *) MyLastLine)->LineID = NextLineID++;
     NewBlockPtr->NumLines = MyBlockPtr->NumLines - nOldLines;
     MyBlockPtr->NumLines = nOldLines;
     NewBlockPtr->LWAp1 = nBytesMoved + sizeof (TypBlock);

     /* Should this line go in the old block or the new?   */
     /* If the add point is beyond the end of the newly-      */
     /* truncated block, we must move the add point to the    */
     /* next block and make the new block the current one.    */
     /* The new position should be the same                   */
     /* number of line bytes past the beginning of the next   */
     /* block as it was past the split point when it was in   */
     /* the old block.                                        */
     /* Either way, one block (the old or the new one)        */
     /* must be unlocked.                                     */

     if (*CurLinePtr >= MyLastLine)
       {
         /* Replace point is in new block.                         */

         MyAddOffset = (char far *) *CurLinePtr - (char far *) MyLastLine + sizeof (TypBlock);
         *CurLinePtr = (TypLine far *) ((char far *) NewBlockPtr + MyAddOffset);
         *CurBlockPtr = NewBlockPtr;
         GlobalUnlock (MyBlockPtr->hCurBlock);
       }
     else
       {
         /* Add point is in current block.                     */
         GlobalUnlock (NewBlockPtr->hCurBlock);
       }
     return (ReplaceLine (LineToAdd, CurBlockPtr, CurLinePtr));
   }
    }
  return (TRUE);
}

/*-- function DeleteLine ----------------------------------------------
 *
 *  Delete a line from a textblock.
 *
 *    Entry    CurBlockPtr points to the block containing the line.
 *             CurLinePtr  points to the line to delete.
 *
 *    Exit     returns TRUE if successful.
 *             CurBlockPtr points to the block containing the next line,
 *                         if any.
 *             CurLinePtr  points to the next line, if any--else the
 *                         end of the block.
 *                         Usually, CurBlockPtr & CurLinePtr will be the
 *                         same upon exit as upon entry; however, sometimes
 *                         a textblock is emptied and the entire block
 *                         is deleted.
 */
BOOL
DeleteLine (TypBlock far ** CurBlockPtr, TypLine far ** CurLinePtr)
{
  TypBlock far *MyBlockPtr = *CurBlockPtr;
  TypLine far *MyLinePtr = *CurLinePtr;
  int bytes_to_end, bytes_to_copy;
  int cur_length = MyLinePtr->length;

  /* If we are (erroneously) at the end of a block, do nothing */

  if (MyLinePtr->length == END_OF_BLOCK)
    {
      return (FALSE);
    }

  /* Copy the remainder of the block on top of the line to be deleted. */

  bytes_to_end = ((char far *) MyBlockPtr + MyBlockPtr->LWAp1) -
    (char far *) MyLinePtr;
  bytes_to_copy = bytes_to_end - cur_length;
  MoveBytes ((char far *) MyLinePtr + cur_length, (char far *) MyLinePtr,
        bytes_to_copy);

  /* Update the block counters.  */

  MyBlockPtr->LWAp1 -= cur_length;
  MyBlockPtr->NumLines--;
  MyBlockPtr->OwnerDoc->TotalLines--;

  /* If we are now at the end of the block, we are faced with one of
   * two situations:
   * 1.  We are also at the beginning of the block, and hence
   *     the block is empty.  In this case we must delete the block
   *     unless it is the only block.
   * 2.  Otherwise, we must advance to the next block, if any.
   */

  if (MyLinePtr->length == END_OF_BLOCK)
    {

      /* We are at the end of the block.  */

      if (*((int far *) (MyLinePtr) - 1) == END_OF_BLOCK)
   {

     /* We have emptied the block.  We must check for whether
           * this is the last block in the document.
           */
     if (MyBlockPtr->OwnerDoc->TotalLines)
       {
         /* The document is not empty.  Delete this empty block.
               */
         DeleteBlock (CurBlockPtr, CurLinePtr);
       }
     else
       {
         /* The document is empty.  Don't delete this block.
               * Leave the pointer at the same place.  It is now pointing
               * at the next line.
               */
       }
   }
      else
   {
     /* We're at the end of the block, but the block is not empty.
           * Just advance to the next block, if any.
           */
     NextLine (CurBlockPtr, CurLinePtr);
   }
    }

  return (TRUE);
}


/*-- function NextLine ------------------------------------------------
 *
 *  Advance a pointer to point to the next line in a document.
 *
 *    Entry    BlockPtr    points to the current block.
 *             LinePtr     points to the current line.
 *
 *    Exit     BlockPtr & LinePtr point to the next line, if there
 *              was one.
 *             Returns TRUE iff pointer was moved.
 *
 *    Method   Must advance BlockPtr if LinePtr was at the end
 *             of a block to start with, or arrives at the end of a block
 *             after moving to the end of the current line.
 */
int
NextLine (BlockPtr, LinePtr)
     TypBlock far **BlockPtr;
     TypLine far **LinePtr;
{
  BOOL retcode = 0;

  if ((*LinePtr)->length != END_OF_BLOCK)
    {
/*      (char far *) *LinePtr += (*LinePtr)->length; */
      IncPtr (*LinePtr, (*LinePtr)->length);
    }
  if ((*LinePtr)->length == END_OF_BLOCK)
    {
      if ((*BlockPtr)->hNextBlock)
   {
     GlobalUnlock ((*BlockPtr)->hCurBlock);
     *BlockPtr = (TypBlock far *) GlobalLock ((*BlockPtr)->hNextBlock);
     *LinePtr = (TypLine far *) ((char far *) *BlockPtr + sizeof (TypBlock));
     retcode = 1;
   }
    }
  else
    {
      retcode = 1;
    }
  return (retcode);
}

/*-- function PrevLine ------------------------------------------------
 *
 *  Back up a pointer to point to the next line in a document.
 *
 *    Entry    BlockPtr    points to the current block.
 *             LinePtr     points to the current line.
 *
 *    Exit     BlockPtr & LinePtr point to the previous line, if there
 *              was one.
 *             Returns TRUE iff pointer was moved.
 *
 *    Method   Rely on the fact that the last field of the previous
 *             line is the length of that line.  Also, the last field
 *             in a textblock header (which is what you get if you
 *             try to look at the previous line in a textblock if you're
 *             at the beginning of a textblock) has the value END_OF_BLOCK.
 */
int
PrevLine (BlockPtr, LinePtr)
     TypBlock far **BlockPtr;
     TypLine far **LinePtr;
{
  if (*((int far *) (*LinePtr) - 1) != END_OF_BLOCK)
    {
/*      (char far *) *LinePtr -= *((int far *)(*LinePtr)-1); */
      IncPtr (*LinePtr, -(*((int far *) (*LinePtr) - 1)));
    }
  else
    {
      if ((*BlockPtr)->hPrevBlock)
   {
     GlobalUnlock ((*BlockPtr)->hCurBlock);
     *BlockPtr = (TypBlock far *) GlobalLock ((*BlockPtr)->hPrevBlock);
     *LinePtr = (TypLine far *)
       ((char far *) *BlockPtr + (*BlockPtr)->LWAp1 - sizeof (TypLine));
/*         (char far *) *LinePtr -= ( *((int far *)(*LinePtr)-1)); */
     IncPtr (*LinePtr, -(*((int far *) (*LinePtr) - 1)));
   }
      else
   {
     return (0);
   }
    }
  return (1);
}

/*--- function TopOfDoc ------------------------------------------------
 *
 *   Set pointers to the first line of a document.
 *
 *    Entry    Doc         points to a document.
 *
 *    Exit     BlockPtr, LinePtr   point to the first
 *                         line in the document.  This line is locked.
 */
void
TopOfDoc (Doc, BlockPtr, LinePtr)
     TypDoc *Doc;
     TypBlock far **BlockPtr;
     TypLine far **LinePtr;
{
  HANDLE hBlock;
  int Offset;
  TypLineID MyLineID;

  hBlock = Doc->hFirstBlock;
  Offset = sizeof (TypBlock);
  MyLineID = 0L;
  LockLine (hBlock, Offset, MyLineID, BlockPtr, LinePtr);
}

/*--- function ExtractTextLine -----------------------------------------------
 *
 *  Extract the text portion of a line to another buffer.
 *
 *    Entry    Doc      points to the document.
 *             LinePtr  points to a line.
 *             BufSize  is the size of the output buffer in bytes.
 *
 *    Exit     Buf      contains the text, terminated by a zero byte.
 *             Returns FALSE iff no text could be extracted
 *               (not a valid line).
 */
BOOL
ExtractTextLine (Doc, LinePtr, Buf, BufSize)
     TypDoc *Doc;
     TypLine far *LinePtr;
     char *Buf;
     int BufSize;
{
  char far *bptr;
  BOOL DidIt = FALSE;

  if (LinePtr->length == END_OF_BLOCK)
    {
    }
  else
    {
      if (Doc->DocType == DOCTYPE_NET)
   {
     bptr = (char far *) LinePtr + sizeof (TypLine) + sizeof (TypGroup);
     while (--BufSize > 1 && (*(Buf++) = *(bptr++)));
     *Buf = '\0';
     DidIt = TRUE;
   }
      else if (Doc->DocType == DOCTYPE_ARTICLE)
   {
     bptr = (char far *) LinePtr + sizeof (TypLine) + sizeof (TypText);
     while (--BufSize > 1 && (*(Buf++) = *(bptr++)));
     *Buf = '\0';
     DidIt = TRUE;
   }
    }
  return (DidIt);
}

/*-- function LockLine ---------------------------------------------
 *
 *  Find the specified line, and return a pointer to it.
 *  Lock the line in memory.
 *
 *  Entry:  hBlock      is the handle of a block we think contains
 *                      the desired line.
 *          LineOff     is the offset in bytes from the beginning of
 *                      the block for where we think the line is.
 *          FindLineID  is the LineID of the desired line.
 *                      If it is 0, we don't check line ID's (don't care).
 *
 *  Exit:   returns     TRUE iff the line was found.
 *          BlockPtr    points to the beginning of the block
 *                      in which the line was found.
 *          LinePtr     points to the line.
 */
BOOL
LockLine (hBlock, LineOff, FindLineID, BlockPtr, LinePtr)
     HANDLE hBlock;
     unsigned int LineOff;
     TypLineID FindLineID;
     TypBlock far **BlockPtr;
     TypLine far **LinePtr;
{
  *BlockPtr = (TypBlock far *) GlobalLock (hBlock);
  *LinePtr = (TypLine far *) ((char far *) *BlockPtr + LineOff);
  if (FindLineID && (*LinePtr)->LineID != FindLineID)
    {
      /* The location specified by hBlock and LineOff does not
       * contain the right line.  So, unlock that block and start
       * scanning the document from the top, looking for the line.
       */
      TypBlock far *MyBlockPtr;
      TypLine far *MyLinePtr;
      HANDLE hMyBlock;
      int MyOffset;

      hMyBlock = (*BlockPtr)->OwnerDoc->hFirstBlock;
      MyOffset = sizeof (TypBlock);
      GlobalUnlock (hBlock);

      MyBlockPtr = (TypBlock far *) GlobalLock (hMyBlock);
      MyLinePtr = (TypLine far *) ((char far *) MyBlockPtr + sizeof (TypBlock));

      while (MyLinePtr->LineID != FindLineID && NextLine (&MyBlockPtr, &MyLinePtr));

      if (MyLinePtr->LineID == FindLineID)
   {
     *BlockPtr = MyBlockPtr;
     *LinePtr = MyLinePtr;
   }
      else
   {
     MessageBox (hWndConf, "Can't find line", "in LockLine", MB_ICONHAND | MB_OK);
     return (FALSE);
   }
    }
  return (TRUE);
}

/*-- function UnlockLine ---------------------------------------------
 *
 *  Given a block pointer and a line pointer, unlock the block
 *  in memory and return a line ID, a block handle and an offset within the
 *  block to the line.
 *
 *    Entry    BlockPtr    points to a textblock
 *             LinePtr     points to a line in that textblock
 *
 *    Exit     TheLineID   is the LineID of the pointed-to line.
 *             hBlock      is the handle to the textblock.
 *             LineOff     is the offset (in bytes from the beginning
 *                         of the block) of the line.
 */
void
UnlockLine (BlockPtr, LinePtr, hBlock, LineOff, TheLineID)
     TypBlock far *BlockPtr;
     TypLine far *LinePtr;
     HANDLE *hBlock;
     unsigned int *LineOff;
     TypLineID *TheLineID;
{
  PtrToOffset (BlockPtr, LinePtr, hBlock, LineOff, TheLineID);
  GlobalUnlock (*hBlock);
}

/*-- function PtrToOffset ---------------------------------------------
 *
 *  Given a block pointer and a line pointer,
 *  return a line ID, a block handle and an offset within the
 *  block to the line.
 *
 *    Entry    BlockPtr    points to a textblock
 *             LinePtr     points to a line in that textblock
 *
 *    Exit     TheLineID   is the LineID of the pointed-to line.
 *             hBlock      is the handle to the textblock.
 *             LineOff     is the offset (in bytes from the beginning
 *                         of the block) of the line.
 */
void
PtrToOffset (BlockPtr, LinePtr, hBlock, LineOff, TheLineID)
     TypBlock far *BlockPtr;
     TypLine far *LinePtr;
     HANDLE *hBlock;
     unsigned int *LineOff;
     TypLineID *TheLineID;
{
  *TheLineID = LinePtr->LineID;
  *LineOff = (char far *) LinePtr - (char far *) BlockPtr;
  *hBlock = BlockPtr->hCurBlock;
}

/*-- function WhatLine ------------------------------------------------
 *
 *  Determine the ordinal number of a given line in the document.
 *
 *    Entry    BlockPtr    points to the block containing a line.
 *             LinePtr     points to the line.
 *
 *    Exit     Returns 0 = first line, 1 = second, and so on.
 *
 *  Strategy is to start at the beginning of the document and
 *  scan though the lines, counting lines until we reach the
 *  current line.  In more detail:
 *
 *  Save the current position.
 *  Go to the first block of the document.
 *  Number of lines = 0
 *  While we are not yet at the original block,
 *     Add in the number of lines in this block, just by looking at header.
 *  Now that we have reached the original block, start at the
 *     beginning of the block and scan forward line-by-line until
 *     we reach the original line.
 */
unsigned int
WhatLine (BlockPtr, LinePtr)
     TypBlock far *BlockPtr;
     TypLine far *LinePtr;
{     
#if 0
   /* This older code is faster, but ignores the distinction 
    * between active and inactive lines.  /mrr
    */
  unsigned int nLines = 0;
  TypBlock far *MyBlock;
  TypLine far *MyLine;
  TypDoc *MyDoc;
  HANDLE hOrgBlock, hMyBlock, hNewBlock;
  unsigned int OrgOffset, MyOffset;
  TypLineID OrgLineID, MyLineID = 0L;

  MyDoc = BlockPtr->OwnerDoc;
  UnlockLine (BlockPtr, LinePtr, &hOrgBlock, &OrgOffset, &OrgLineID);

  hMyBlock = MyDoc->hFirstBlock;
  while (hMyBlock != hOrgBlock)
    {
      MyBlock = (TypBlock far *) GlobalLock (hMyBlock);
      nLines += MyBlock->NumLines;
      hNewBlock = MyBlock->hNextBlock;
      if (!hNewBlock)
   {
     MessageBox (MyDoc->hDocWnd, "Hit end of document", "Error in WhatLine",
            MB_OK | MB_ICONHAND);
   }
      GlobalUnlock (hMyBlock);
      hMyBlock = hNewBlock;
    }

  /* The technique of scanning the NumLines field starts the        */
  /* counter at 1 rather than 0, so if we got any lines above,      */
  /* adjust the count by decrementing it.                           */

/* if(nLines) nLines--; */

  /* We have reached the original block.                            */
  /* Start scanning at the first line.                              */

  MyOffset = sizeof (TypBlock);
  LockLine (hMyBlock, MyOffset, MyLineID, &MyBlock, &MyLine);
  while (MyOffset != OrgOffset)
    {
      nLines++;
      NextLine (&MyBlock, &MyLine);
      MyOffset = (char far *) MyLine - (char far *) MyBlock;
    }
#else
  unsigned int nLines = 0;
  TypBlock far *MyBlock;
  TypLine far *MyLine;
  TypDoc *MyDoc;
            
   MyDoc = BlockPtr->OwnerDoc;
   TopOfDoc(MyDoc,&MyBlock,&MyLine);
   while(MyLine != LinePtr) {
      if(MyLine->active) nLines++;
      if(!NextLine(&MyBlock,&MyLine)) {
         MessageBox (MyDoc->hDocWnd, "Hit end of document", "Error in WhatLine",
            MB_OK | MB_ICONHAND);
      }
   }
#endif
  return (nLines);
}

/*--- function NumBlocksInDoc --------------------------------------------
 *
 *  Find the number of blocks in a document.
 *
 * Entry Doc points to a document.
 *
 * Exit  returns the number of textblocks in the document.
 */
int
NumBlocksInDoc (Doc)
     TypDoc *Doc;
{
  TypBlock far *MyBlock;
  HANDLE hMyBlock, hNewBlock;
  int nBlocks = 0;

  if (!Doc)
    return (0);
  hMyBlock = Doc->hFirstBlock;
  do
    {
      nBlocks++;
      MyBlock = (TypBlock far *) GlobalLock (hMyBlock);
      hNewBlock = MyBlock->hNextBlock;
      GlobalUnlock (hMyBlock);
      hMyBlock = hNewBlock;
    }
  while (hNewBlock);

  return (nBlocks);
}

/*--- function FindLineOrd ---------------------------------------------
 *
 *  Find the Nth line in a document.
 *
 *    Entry    Doc      points to a document.
 *             LineOrd  is the ordinal of the line to find.
 *                      0 = first line in document.
 *
 *    Exit     BlockPtr points to the block containing the line.
 *             LinePtr  points to the line.
 *             return value is TRUE iff we found the line.
 */
BOOL
FindLineOrd (Doc, LineOrd, BlockPtr, LinePtr)
     TypDoc *Doc;
     unsigned int LineOrd;
     TypBlock far **BlockPtr;
     TypLine far **LinePtr;
{
#if 1
   /* This old code is faster, but ignores the distinction between
    * active and inactive lines.   /mrr
    */
  unsigned int LinesSoFar = 0;
  TypBlock far *MyBlockPtr;
  TypLine far *MyLinePtr;
  HANDLE hBlock, hNextBlock;
  int retcode = 0;

  hBlock = Doc->hFirstBlock;
  do
    {
      MyBlockPtr = (TypBlock far *) GlobalLock (hBlock);
      if (LinesSoFar + MyBlockPtr->NumActiveLines > LineOrd)
   break;
      LinesSoFar += MyBlockPtr->NumActiveLines;
      hNextBlock = MyBlockPtr->hNextBlock;
      GlobalUnlock (hBlock);
      hBlock = hNextBlock;
    }
  while (hBlock);

  if (hBlock)
    {
      MyLinePtr = (TypLine far *) ((char far *) MyBlockPtr + sizeof (TypBlock));
      AdvanceToActive(&MyBlockPtr,&MyLinePtr);
      while (LinesSoFar < LineOrd)
   {
     if (!NextLine (&MyBlockPtr, &MyLinePtr))
       break;
     if(MyLinePtr->active)LinesSoFar++;
   }
      retcode = TRUE;
      *BlockPtr = MyBlockPtr;
      *LinePtr = MyLinePtr;
    }
#else
  TypBlock far *MyBlockPtr;
  TypLine far *MyLinePtr;
  HANDLE hMyBlock;
  unsigned int MyOffset;
  TypLineID MyLineID;
  int retcode = FALSE;
  
  TopOfDoc(Doc,BlockPtr,LinePtr);
  while(1) {
      if((*LinePtr)->active) {
         if(!LineOrd--) {
            retcode = TRUE;
            break;
         } 
      }
      if(!NextLine(BlockPtr,LinePtr)) {
         break;
      }
  }            
  /*UnlockLine(BlockPtr,LinePtr,&hMyBlock,&MyOffset,&MyLineID); */
#endif
  return (retcode);
}

/*-- function MoveBytes -----------------------------------------------
 *
 *   Move a region of bytes in memory from one place to another.
 *   Handle overlapping regions without destroying the source.
 *
 *    Entry    Source   points to the FWA of the source.
 *             Target   points to the FWA of the target.
 *             NumBytes is the number of bytes to copy (>= 0).
 */
void
MoveBytes (FSource, FTarget, NumBytes)
     void far *FSource, far * FTarget;
     int NumBytes;
{
  char far *Source = FSource;
  char far *Target = FTarget;

  if (Source < Target)
    {
      Source += NumBytes - 1;
      Target += NumBytes - 1;
      while (NumBytes--)
   *(Target--) = *(Source--);
    }
  else
    {
      while (NumBytes--)
   *(Target++) = *(Source++);
    }
}

/*--- function InitDoc ---------------------------------------------------
 *
 *  Initialize the fields of a document.
 */
int
InitDoc (Doc, hWnd, Parent, DType)
     TypDoc *Doc;
     HWND hWnd;
     TypDoc *Parent;
     int DType;
{
  TypBlock far *BlockPtr;
  HANDLE hBlock;

  Doc->hLastBlock = 0;
  Doc->TotalLines = 0;
  Doc->HeaderLines = 0;
  Doc->ActiveLines = 0;
  Doc->BlockSize = BLOCK_SIZE;
  Doc->SplitSize = (BLOCK_SIZE * 2) / 3;
  Doc->hDocWnd = hWnd;
  Doc->hLastSeenBlock = 0;
  Doc->TopLineOrd = 0;
  Doc->ParentDoc = Parent;
  Doc->ParentLineID = 0L;
  Doc->SearchStr[0] = '\0';
  Doc->FindLineID = 0L;
  Doc->TopScLineID = 0L;
  Doc->InUse = TRUE;
  Doc->DocType = DType;
  Doc->hFindBlock = 0;

  switch (DType)
    {
    case DOCTYPE_NET:
      Doc->OffsetToText = sizeof (TypLine) + sizeof (TypGroup);
      break;
    case DOCTYPE_GROUP:
      Doc->OffsetToText = sizeof (TypLine) + sizeof (TypArticle);
      break;
    case DOCTYPE_ARTICLE:
      Doc->OffsetToText = sizeof (TypLine) + sizeof (TypText);
      break;
    }

  hBlock = GlobalAlloc (GMEM_MOVEABLE, (long) BLOCK_SIZE);
  if (hBlock)
    {
      BlockPtr = (TypBlock far *) GlobalLock (hBlock);
      SetupEmptyBlock (BlockPtr, hBlock, 0, 0, Doc);

      Doc->hFirstBlock = hBlock;
      Doc->hLastBlock = hBlock;
      Doc->hCurAddBlock = hBlock;
      Doc->AddOffset = sizeof (TypBlock);
      Doc->AddLineID = 0L;
      Doc->hCurTopScBlock = hBlock;
      Doc->TopScOffset = sizeof (TypBlock);
      Doc->TopScLineID = 0L;
      Doc->LastSeenLineID = 0L;

      GlobalUnlock (hBlock);
    }
  else
    {
      MessageBox (hWnd, "Could not allocate textblock", "Out of Memory Error", MB_OK | MB_ICONHAND);
    }

  return (0);
}

/*-- function FreeDoc ----------------------------------------------
 *
 *  Free up all the text blocks associated with a document.
 *
 *    Entry    Doc   points to the document in question.
 */
void
FreeDoc (Doc)
     TypDoc *Doc;
{
  TypBlock far *BlockPtr;
  HANDLE hBlock, hNextBlock;

  /* Start at the first block of the document, and travel           */
  /* down the linked list of blocks, freeing them.                  */

  hBlock = Doc->hFirstBlock;
  while (hBlock)
    {
      BlockPtr = (TypBlock far *) GlobalLock (hBlock);
      hNextBlock = BlockPtr->hNextBlock;
      GlobalUnlock (hBlock);
      GlobalFree (hBlock);
      hBlock = hNextBlock;
    }
}

/*--- function ForAllLines ---------------------------------------------
 *
 *  Perform an operation for all lines in a document.  The operation
 *  to be performed is specified by a C function argument.
 *
 *    Entry Doc            is the document.
 *          lpfnFunc       is a pointer to the function to call for
 *                         each line.
 *          lFlag          is a flag that's passed to the function.
 *
 */
void
ForAllLines (TypDoc *Doc, 
       void lpfnFunc(TypDoc *Doc, TypBlock far ** BlockPtr, TypLine far ** LinePtr, int wFlag, int wValue),
       int wFlag, int wValue)
{
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
  TypLineID old_lineID;
  BOOL looping = TRUE;


  TopOfDoc (Doc, &BlockPtr, &LinePtr);
  if (LinePtr->length != END_OF_BLOCK)
    {
      do
   {
     old_lineID = LinePtr->LineID;
     lpfnFunc (Doc, &BlockPtr, &LinePtr, wFlag, wValue);
     if (old_lineID == LinePtr->LineID &&
         LinePtr->length != END_OF_BLOCK)
       {
         looping = NextLine (&BlockPtr, &LinePtr);
       }
     else if (LinePtr->length == END_OF_BLOCK)
       {
         looping = FALSE;
       }
   }
      while (looping);
    }
/*   UnlockLine (BlockPtr, LinePtr,
    &(FindDoc->hFindBlock), &(FindDoc->FindOffset), &(FindDoc->FindLineID)); */

}

/*--- function ForAllBlocks ---------------------------------------------
 *
 *  Perform an operation for all blocks in a document.  The operation
 *  to be performed is specified by a C function argument.
 *
 *    Entry Doc            is the document.
 *          lpfnFunc       is a pointer to the function to call for
 *                         each block.
 *          lFlag          is a flag that's passed to the function.
 *
 */
void
ForAllBlocks (TypDoc *Doc, 
       void lpfnFunc(TypDoc *Doc, TypBlock far ** BlockPtr, int wFlag, int wValue),
       int wFlag, int wValue)
{
  TypBlock far *BlockPtr; 
  HANDLE hMyBlock, hNewBlock;

  if (!Doc)
    return ;
    
  hMyBlock = Doc->hFirstBlock;
  do
    {
      BlockPtr = (TypBlock far *) GlobalLock (hMyBlock);
      lpfnFunc(Doc,&BlockPtr,wFlag,wValue);
      hNewBlock = BlockPtr->hNextBlock;
      GlobalUnlock (hMyBlock);
      hMyBlock = hNewBlock;
    }
  while (hNewBlock);

}

/*--- function SetForBlock -----------------------------------
 *
 *  Perform a function for a block.
 *  This is called by ForAllBlocks.
 *
 *  Entry:  Doc      points to a document
 *          BlockPtr points to a block
 *          wFlag    is a general-purpose flag. 
 *          wValue   is a general-purpose value.
 */
void
SetForBlock(TypDoc *Doc, TypBlock far ** BlockPtr, int wFlag, int wValue)
{
   switch (wFlag) {
      case BLOCK_ACTION_SET_ACTIVE:
         (*BlockPtr)->NumActiveLines = wValue;
         break;
   }
}


/*--- function FindString ----------------------------------------------
 *
 *   Locate a search string in a document.
 *
 *   Entry  StartAtTop        is TRUE iff we should start the search at
 *                            the top of the document.
 *          FindDoc           points to the document in which we are searching.
 *          ->hFindBlock      is the block to start at, if StartAtTop
 *                            is FALSE.
 *          ->FindOffset      is the offset within the block of the
 *                            line to start at, if StartAtTop is FALSE.
 *          ->SearchStr       has the string to search for.
 *
 *   Exit   returns -1 if the string was not found,
 *            else the offset of the string from the beginning of the line.
 *          FindDoc ...
 *          ->hFindBlock      has the block handle of the line which
 *                            was found (if any)
 *          ->FindOffset      has the offset of the found line (if any)
 *
 */
int
FindString (StartAtTop)
     BOOL StartAtTop;
{
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
  HANDLE hBlock;
  unsigned int Offset;
  unsigned int TextOffset;
  BOOL found = -1;
  int TargLen = 0;
  int SourceLen;
  char *Target;
  char sourceline[MAXINTERNALLINE];
  char targline[MAXFINDSTRING];
  char *sourceptr, far * orglineptr;
  char *targptr;
  TypLineID MyLineID;
  register char ch;

  if (StartAtTop)
    {
      hBlock = FindDoc->hFirstBlock;
      Offset = sizeof (TypBlock);
      MyLineID = 0L;
    }
  else
    {
      hBlock = FindDoc->hFindBlock;
      Offset = FindDoc->FindOffset;
      MyLineID = FindDoc->FindLineID;
    }
  LockLine (hBlock, Offset, MyLineID, &BlockPtr, &LinePtr);
  /* If doing a Find Next, skip forward one line before starting    */
  /* the search.                                                    */
  if (!StartAtTop)
    {
      NextLine (&BlockPtr, &LinePtr);
    }

  TextOffset = FindDoc->OffsetToText;

  Target = FindDoc->SearchStr;
  for (targptr = targline; ch = *Target, *(targptr++) = tolower (ch);
       TargLen++)
    Target++;

  if (LinePtr->length != END_OF_BLOCK)
    {
      do
   { 
    if(LinePtr->active) {
     orglineptr = (char far *) LinePtr + TextOffset;
     sourceptr = sourceline;
     for (SourceLen = 0; ch = *(orglineptr),
          *(sourceptr++) = tolower (ch); SourceLen++)
       orglineptr++;
     found = SearchLine (sourceline, SourceLen, targline, TargLen);
    }
   }
      while (found == -1 && NextLine (&BlockPtr, &LinePtr));
    }
  UnlockLine (BlockPtr, LinePtr,
    &(FindDoc->hFindBlock), &(FindDoc->FindOffset), &(FindDoc->FindLineID));
  return (found);
}

/*--- function SearchLine -----------------------------------------------
 *
 *  Search a line for a target string.
 *
 *  Entry   Line     is a zero-terminated string to search.
 *          LineLen  is strlen(Line).  Redundant, but passed for
 *                   efficiency.
 *          Target   is the Target string to search for, zero-terminated.
 *          TargLen  is strlen(Target), passed for efficiency.
 *
 *  Exit    returns  -1 if not found, else the character position
 *                   in which the string was found (0=first).
 */
int
SearchLine (Line, LineLen, Target, TargLen)
     char *Line;
     int LineLen;
     char *Target;
     int TargLen;
{
  char *stopptr = Line + LineLen - TargLen + 1;
  char *lineptr;
  char *searchptr;
  char *targptr;

  if (LineLen <= 0 || TargLen <= 0 || TargLen > LineLen)
    return (-1);

  for (lineptr = Line; lineptr != stopptr; lineptr++)
    {
      searchptr = lineptr;
      for (targptr = Target; *targptr && *(targptr) == *(searchptr);)
   {
     targptr++;
     searchptr++;
   }
      if (!(*targptr))
   {
     return (lineptr - Line);
   }
    }
  return (-1);
}

/*--- function DoFind ---------------------------------------------------
 *
 *  Controlling routine for searching for text.
 *  Takes care of displaying window properly when search is done.
 *
 *    Entry    StartAtTop  is TRUE iff we should start at the top
 *                         of the document.
 *             FindDoc     points to the document being searched.
 *                         All the info we need is in fields in this doc.
 */
BOOL
DoFind (StartAtTop)
     BOOL StartAtTop;
{
  int CharPos;
  int iline;
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
  int found = FALSE;
  BOOL refresh = FALSE;
  int goline;
  unsigned int LineOrd;
  unsigned int LastAllowedLineOrd;

  CharPos = FindString (StartAtTop);
  if (CharPos >= 0)
    {
      iline = LineOnScreen (FindDoc, FindDoc->hFindBlock, FindDoc->FindOffset, FindDoc->FindLineID);
      /* If this line wasn't on the screen, we are going to have
       * to adjust the top of the screen.
       * Make the found line the top of the screen, then back up a
       * little to give the user a context in which to view the line.
       */
      if (iline == -1)
   {
     FindDoc->hCurTopScBlock = FindDoc->hFindBlock;
     FindDoc->TopScOffset = FindDoc->FindOffset;
     FindDoc->TopScLineID = FindDoc->FindLineID;
     refresh = TRUE;
     LockLine (FindDoc->hCurTopScBlock, FindDoc->TopScOffset, FindDoc->TopScLineID,
          &BlockPtr, &LinePtr);
     for (goline = FindDoc->ScYLines / 4; goline; goline--)
       {
         PrevLine (&BlockPtr, &LinePtr);
       }
     /* Have we gone past the top of the last screen?
           * If so, move the top line back to the top of the last screen.
           */
     LineOrd = WhatLine (BlockPtr, LinePtr);
     LastAllowedLineOrd = FindDoc->TotalLines - FindDoc->ScYLines;
     if (LineOrd > LastAllowedLineOrd)
       {
         GlobalUnlock (BlockPtr->hCurBlock);
         FindLineOrd (FindDoc, LastAllowedLineOrd, &BlockPtr, &LinePtr);
         LineOrd = LastAllowedLineOrd;
       }
     UnlockLine (BlockPtr, LinePtr, &(FindDoc->hCurTopScBlock),
            &(FindDoc->TopScOffset), &(FindDoc->TopScLineID));
     FindDoc->TopLineOrd = LineOrd;
     iline = LineOnScreen (FindDoc, FindDoc->hFindBlock, FindDoc->FindOffset, FindDoc->FindLineID);
   }

      InvalidateRect (FindDoc->hDocWnd, NULL, FALSE);
      found = TRUE;
    }
  return (found);
}
