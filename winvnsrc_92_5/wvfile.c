
/*
 *
 * $Id: wvfile.c 1.7 1994/05/27 01:29:29 rushing Exp $
 * $Log: wvfile.c $
 * Revision 1.7  1994/05/27  01:29:29  rushing
 * unnecessary winundoc.h
 *
 * Revision 1.6  1994/01/16  12:03:44  jcoop
 * Mod to MRRWriteLine to allow unsigned size (65536 byte) writes
 *
 * Revision 1.5  1994/01/12  19:27:32  mrr
 * mrr mods 4
 * 
 * Revision 1.4  1993/12/08  01:27:21  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.3  1993/08/25  18:54:36  mbretherton
 * MRB merge, mail & post logging
 *
 * Revision 1.x  1993/07/21 MBretherton
 *                remove vRef from MRROpenFile (Mac Stuff)
 * Revision 1.2  1993/07/06  21:09:09  cnolan
 * win32 support
 *
 * Revision 1.1  1993/02/16  20:53:50  rushing
 * Initial revision
 *
 *
 */

/*-- WVFILE.C -- File containing routines to do I/O under Windows.
 */

#include "windows.h"
#include "wvglob.h"
#include "winvn.h"
#include <io.h>


/*--- function MRROpenFile --------------------------------------------
 *
 *  Perform the same function as Windows' OpenFile, but also
 *  create an instance of a structure that keeps track of file-related
 *  information (most importantly, an file buffer so I don't have to
 *  do 1-byte system reads).
 *
 *    Entry    FileName    is the file name of the file to open.
 *             Mode        is the mode under which to open the file.
 *
 *    Exit     MRRFile     points to a dynamically-allocated structure
 *                         containing info about the file.
 *             Returns a handle to the file; 0 if failure.
 */
HANDLE
MRROpenFile (FileName, Mode, MRRFile)
     char *FileName;
     int Mode;
     TypMRRFile **MRRFile;
{
  HANDLE hMyFile, hMRR;
  TypMRRFile *MyMRRFile;
  int retcode;

  hMRR = LocalAlloc (LMEM_FIXED, sizeof (TypMRRFile));
  if (!hMRR)
    {
      return (0);
    }
  else
    {
      MyMRRFile = (TypMRRFile *) LocalLock (hMRR);
      MyMRRFile->hthis = hMRR;
      MyMRRFile->bufidx = 0;
      MyMRRFile->bytesread = 0;
      MyMRRFile->eofflag = FALSE;
      MyMRRFile->mode = Mode;

      if (Mode == OF_WRITE)
   {
     hMyFile = OpenFile (FileName, &(MyMRRFile->of), OF_EXIST);
     if (hMyFile == -1)
       Mode = OF_CREATE;
   }
      hMyFile = retcode = OpenFile ((char far *) FileName, &(MyMRRFile->of), Mode);
      if (retcode == (-1))
   {
     LocalUnlock (hMRR);
     LocalFree (hMRR);
     return (0);
   }
      else
   {
     MyMRRFile->hFile = hMyFile;
   }
    }
  *MRRFile = MyMRRFile;
  return ((HANDLE) hMyFile);
}

/*--- function MRRCloseFile --------------------------------------------
 *
 *  Perform the same function as the close function, but also
 *  deallocate the structure allocated by MRROpenFile.
 *
 *    Entry    MRRFile  points to a structure describing the file.
 */
void
MRRCloseFile (MRRFile)
     TypMRRFile *MRRFile;
{
  HANDLE hMyMRRFile;

  if (MRRFile->mode == OF_WRITE || MRRFile->mode == OF_CREATE)
    {
      _lwrite (MRRFile->hFile, MRRFile->buf, MRRFile->bufidx);
    }
  _lclose (MRRFile->hFile);

  hMyMRRFile = MRRFile->hthis;
  LocalUnlock (hMyMRRFile);
  LocalFree (hMyMRRFile);
}

/*--- function MRRReadLine ---------------------------------------------
 *
 *  Read in a line from a file, very much like "fgets".
 *  Lines are assumed to be terminated by CR/LF (except that this
 *  is optional for the last line in a file).
 *
 *  No CR, LF, or zero byte is placed in the user's buffer or
 *  counted as a data byte in the returned count.
 *
 *    Entry    MRRFile  points to a structure describing the file.
 *             Linebuf  is the place to put the line.
 *             Len      is the length of Linebuf.
 *
 *    Exit     Linebuf  contains a line.
 *             Returns number of characters read.  0 means an empty line;
 *              -1 means EOF.
 */
int
MRRReadLine (MRRFile, Linebuf, Len)
     TypMRRFile *MRRFile;
     char *Linebuf;
     int Len;
{
  int BytesReturned = 0;
  char ch;

  /* If we hit the EOF while reading last time, we might not have   */
  /* had to return an EOF indication then--but we certainly do now. */

  if (MRRFile->eofflag)
    return (-1);

  /* Read bytes until we exhaust the user's buffer,                 */
  /* empty our own internal buffer,                                 */
  /* or hit a CR (which hopefully belongs to a CR/LF pair).         */

readlp:;
  while (Len && MRRFile->bufidx < MRRFile->bytesread &&
    (ch = MRRFile->buf[MRRFile->bufidx]) != '\r' && ch != '\n')
    {
      *(Linebuf++) = ch;
      BytesReturned++;
      (MRRFile->bufidx)++;
      Len--;
    }

  /* If we emptied our own internal buffer, fill 'er up again       */
  /* from the file.  If the read hits EOF, return the user's        */
  /* data now (indicating EOF if we never got any data bytes        */
  /* else go back up and continue taking from the buffer.           */

  if (MRRFile->bufidx >= MRRFile->bytesread)
    {
      MRRFile->bufidx = 0;
      MRRFile->bytesread = _lread (MRRFile->hFile, MRRFile->buf, BUFSIZE);
      if (MRRFile->bytesread > 0)
   {
     goto readlp;
   }
      else
   {
     MRRFile->eofflag = TRUE;
     if (!BytesReturned)
       BytesReturned--;
     goto endit;
   }
    }

  /* If we reach here, we either filled the user's buffer or        */
  /* hit a CR.  No EOF was encountered.                             */
  /* Either way, we must now skip to the beginning of the next      */
  /* line.  This means skipping to the next LF.  Since in most      */
  /* cases the user does specify a big enough buffer, in most       */
  /* cases all we are doing here is reading up the next character   */
  /* (assuming it's a LF).                                          */
  /* All data that should go in the user's buffer is there by now.  */

skipLF:;

  while (MRRFile->bufidx < MRRFile->bytesread &&
    MRRFile->buf[MRRFile->bufidx] != '\n')
    {
      (MRRFile->bufidx)++;
    }

  /* We either found the LineFeed we were looking for, or hit       */
  /* the end of our internal buffer.  If the latter, fill 'er       */
  /* up and try again.                                              */

  if (MRRFile->bufidx >= MRRFile->bytesread)
    {
      MRRFile->bufidx = 0;
      MRRFile->bytesread = _lread (MRRFile->hFile, MRRFile->buf, BUFSIZE);
      if (MRRFile->bytesread > 0)
   {
     goto skipLF;
   }
      else
   {
     MRRFile->eofflag = TRUE;
     goto endit;
   }
    }

  /* The buffer pointer is now pointing at the LF.  Advance         */
  /* it by one so we'll get the first character of the next         */
  /* line next time.                                                */
  /* If this takes us past the end of the buffer, no problem.       */


  if (MRRFile->buf[MRRFile->bufidx] == '\n')
    (MRRFile->bufidx)++;

endit:;
  return (BytesReturned);
}

/*--- function MRRWriteLine ---------------------------------------------
 *
 *  Write out a line of text, followed by a CR and LF.
 *
 *    Entry    MRRFile  points to a structure describing the file.
 *             LineBuf  points to line buffer to write out.
 *             Len      is the number of bytes to write.
 */
BOOL
MRRWriteLine (MRRFile, LineBuf, Len)
     TypMRRFile *MRRFile;
     char far *LineBuf;
     unsigned int Len;
{
  unsigned int BytesToCopy;
  static NotFirst = 0;

  do
    {
      BytesToCopy = Len < (BUFSIZE - (unsigned int)MRRFile->bufidx) ?
	Len : BUFSIZE - (unsigned int)MRRFile->bufidx;

      MoveBytes (LineBuf, (char far *) (MRRFile->buf + MRRFile->bufidx), BytesToCopy);
      MRRFile->bufidx += BytesToCopy;
      LineBuf += BytesToCopy;
      Len -= BytesToCopy;
      if (MRRFile->bufidx >= BUFSIZE)
   {
     _lwrite (MRRFile->hFile, MRRFile->buf, BUFSIZE);
     MRRFile->bufidx = 0;
   }
    }
  while (Len > 0);

  if (!(NotFirst++))
    {
      MRRWriteLine (MRRFile, "\r\n", 2);
    }
  NotFirst--;
  return (1);
}

/*-- function MRRWriteDocument -----------------------------------------
 *
 *  Write out an entire document to disk.
 *
 *  Entry   Document    points to a document.
 *          Offset      is the number of bytes to skip at the beginning
 *                      of the line (between the end of the structure
 *                      described in TypLine and the beginning of text).
 *                      In most cases this will be zero.
 *          szFileName  points to the file name to save to.
 *          Append      is TRUE iff we should append to the file.
 *
 *    Returns TRUE iff we wrote the file OK.
 */
BOOL
MRRWriteDocument (Document, Offset, szFileName, Append)
     TypDoc *Document;
     int Offset;
     char *szFileName;
     BOOL Append;
{
  TypMRRFile *MRRFile;
  HANDLE hFile;
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
  int mode;

  if (Append)
    {
      mode = OF_WRITE;
    }
  else
    {
      mode = OF_CREATE;
    }
  hFile = MRROpenFile (szFileName, mode, &MRRFile);
  if (Append)
    {
      _llseek (hFile, 0L, 2);
    }

  if (hFile)
    {
      LockLine (Document->hFirstBlock, sizeof (TypBlock), (TypLineID) 0L, &BlockPtr, &LinePtr);
      while (LinePtr->length != END_OF_BLOCK)
   {
     MRRWriteLine (MRRFile, ((char far *) LinePtr) + Offset + sizeof (TypLine),

         lstrlen (((char far *) LinePtr) + sizeof (TypLine) + Offset));
     NextLine (&BlockPtr, &LinePtr);
   }
      GlobalUnlock (BlockPtr->hCurBlock);
      MRRCloseFile (MRRFile);
    }
  else
    {
      return (0);
    }
  return (TRUE);
}


/*
 *  Append the edit text to the end of the appropriate log file 
 *   hWnd  - handle to window for warning messages
 *   fName - name of the log file
 *           (will be created if does not exist) 
 *   MBuf  - pointer to string to be written to file
 */
BOOL
WriteEditLog(HWND hWnd, char *szFileName, char *mBuf, unsigned int size)
{
  TypMRRFile *MRRFile;
  HANDLE hFile;
    
  hFile = MRROpenFile (szFileName, OF_WRITE, &MRRFile);
  
  if (hFile)
  {
     _llseek (hFile, 0L, 2);
     MRRWriteLine (MRRFile, mBuf, size);
     MRRCloseFile (MRRFile);
  }
  else
  {
     return (0);
  }
  return (TRUE);
}
