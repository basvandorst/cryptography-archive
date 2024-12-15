
/*
 *
 * $Id: wvlist.c 1.7 1993/12/08 01:28:38 rushing Exp $
 * $Log: wvlist.c $
 * Revision 1.7  1993/12/08  01:28:38  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.6  1993/10/12  17:47:26  rushing
 * make winvn grok servers that start article numbers at 0
 *
 * Revision 1.5  1993/08/18  21:51:13  rushing
 * more 16-bit article number fixes
 *
 * Revision 1.4  1993/06/28  17:52:17  rushing
 * fixed compiler warnings
 *
 * Revision 1.3  1993/06/24  17:07:05  dumoulin
 * avoid max new groups > MAXGROUPS crash.
 *
 * Revision 1.2  1993/06/22  16:16:13  rushing
 * raised MAXGROUPS (really MAXNEWGROUPS)
 *
 * Revision 1.1  1993/02/16  20:54:22  rushing
 * Initial revision
 *
 *
 */

/*-- WVLIST.C -- File containing functions to deal with the NNTP LIST
 *   command, which lists all the newsgroups and their status.
 *
 *   Mark Riordan   25 October 1990
 */

#include "windows.h"
#include "wvglob.h"
#include "winvn.h"

#include "winundoc.h"
#include <ctype.h>


/* #define HASHMAX 9721 */
/* #define MAXGROUPS  3200 */
#define HASHMAX 16379
#define MAXGROUPS 6000

TypLine far *far * NetHashTable;
HANDLE hNetHashTable;

HANDLE htohNewGroupLines;
HANDLE far *lphNewGroupLines; /* array of handles to new group lines */


HWND hDlgList;       /* Handle to child list box window. */

#define MyGlobalUnlock(hWhat)  MRRGlobalUnlock(hWhat,__LINE__)

void MRRGlobalUnlock (HANDLE hWhat, WORD wLine);

void
MRRGlobalUnlock (hWhat, wLine)
     HANDLE hWhat;
     WORD wLine;
{
  WORD LockCount;

  if (!(LockCount = (GMEM_LOCKCOUNT & GlobalFlags (hWhat))))
    {
    }
  else
    {
      GlobalUnlock (hWhat);
    }

}

/*--- function StartList -----------------------------------------------
 *
 *  Initiate the process of sending a LIST command and using its
 *  output to update our list of news groups.
 */

void
StartList ()
{
  unsigned int hashval;

  CommState = ST_LIST_RESP;
  CommBusy = TRUE;
  PutCommLine ("LIST\r", 4);
  Initializing = INIT_SCANNING_NETDOC;
  InvalidateRect (hWndConf, NULL, FALSE);
  SendMessage (hWndConf, WM_PAINT, 0, 0L);

  /* Set up hash table for group names */

  hNetHashTable = GlobalAlloc (GMEM_MOVEABLE, (long) HASHMAX * sizeof (TypLine far *));
  NetHashTable = (TypLine far * far *) GlobalLock (hNetHashTable);
  for (hashval = 0; hashval < HASHMAX; hashval++)
    {
      NetHashTable[hashval] = (TypLine far *) 0;
    }

  HashNetGroups (&NetDoc, NetHashTable);
  InvalidateRect (hWndConf, NULL, FALSE);
  Initializing = INIT_GETTING_LIST;
  SendMessage (hWndConf, WM_PAINT, 0, 0L);

  /* Set up table of pointers to new group lines.
   */

  InitGroupTable ();
}

/*--- function InitGroupTable -------------------------------------------
 *
 *  Allocate the NewGroupTable.
 *
 *  Exit:   hNewGroupTable    is the handle
 *          NewGroupTable     points to the table
 *          nNewGroups        has been initialized to 0.
 */
void
InitGroupTable (void)
{
  hNewGroupTable = GlobalAlloc (GMEM_MOVEABLE, (long) MAXGROUPS * sizeof (TypLine far *));
  NewGroupTable = (void far * far *) GlobalLock (hNewGroupTable);

  nNewGroups = 0;
}


/*--- function HashNetGroups ------------------------------------------
 *
 *  Enter all the groups in the Net document into the hash table.
 *
 *  Exit    All blocks in the document are locked.
 *          HashTable   is (partially) filled with pointers to
 *                      each of the lines in "Doc".
 */

void
HashNetGroups (Doc, HashTable)
     TypDoc *Doc;
     TypLine far *far * HashTable;
{
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
  unsigned int hashval;
  HANDLE hBlock;
  unsigned int Offset;
  unsigned int TextOffset;
  unsigned char far *textptr;
  TypLineID MyLineID;

  /* Lock all blocks in the document */
  hBlock = Doc->hFirstBlock;
  do
    {
      BlockPtr = (TypBlock far *) GlobalLock (hBlock);
      hBlock = BlockPtr->hNextBlock;
    }
  while (hBlock);


  /* Now start at the beginning of the document, going through
   * each line in the document, hashing its group name into the table.
   */

  hBlock = Doc->hFirstBlock;
  Offset = sizeof (TypBlock);
  MyLineID = 0L;

  LockLine (hBlock, Offset, MyLineID, &BlockPtr, &LinePtr);

  TextOffset = Doc->OffsetToText;

  if (LinePtr->length != END_OF_BLOCK)
    {
      do
   {
     textptr = ((unsigned char far *) LinePtr) + TextOffset;
     hashval = HashGroup (textptr);
     while (HashTable[hashval])
       {
         hashval = (hashval + 1) % HASHMAX;
       }
     HashTable[hashval] = LinePtr;
   }
      while (NextLine (&BlockPtr, &LinePtr));
    }
  UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
}


/*--- function ProcListLine ---------------------------------------------
 *
 *  Process a line received from the NNTP LIST command output.
 *  Each line from the LIST command has the form:
 *   <groupname> <highest_art_#> <lowest_art_#> {y|n|m}
 */

void
ProcListLine (ListLine)
     unsigned char *ListLine;
{
  unsigned int hashval;
  char far *textptr;
  unsigned char *cptr, *restline;
  char mygroupline[BLOCK_SIZE];
  TypGroup *mygroup;
  TypGroup far *netgroup;
  long int ArtNum;

  if ((++RcvLineCount) % UPDATE_TITLE_FREQ == 0)
    {
      InvalidateRect (hWndConf, NULL, FALSE);
      if (RcvLineCount % (UPDATE_TITLE_FREQ * 25) == 0)
   {
     UpdateWindow (hWndConf);
   }
    }

  /* Replace the first blank in the input line with a zero. */
  for (cptr = ListLine; *cptr && *cptr != ' '; cptr++);
  *cptr = '\0';
  restline = cptr + 1;     /* points to highest art # */

  hashval = HashGroup (ListLine);
checkhash:;
  if (!NetHashTable[hashval])
    {
      /* This is a new group.
       * Create a Group line from the information in this line.
       */

      if (nNewGroups < MAXGROUPS) {

   CrackGroupLine (ListLine, (TypLine *) (mygroupline));
   mygroup = (TypGroup *) (mygroupline + sizeof (TypLine));
   GetNum (&restline, &(mygroup->ServerLast));
   GetNum (&restline, &(mygroup->ServerFirst));
   mygroup->ServerEstNum = mygroup->ServerLast - mygroup->ServerFirst+ 1; 
   mygroup->HighestPrevSeen = 0;
   mygroup->nRanges = 0;

   AddGroupToTable (mygroupline);
      }
    }
  else
    {
      textptr = ((char far *) NetHashTable[hashval] + sizeof (TypLine) + sizeof (TypGroup));
      if (lstrcmp (textptr, ListLine))
   {
     hashval = (hashval + 1) % HASHMAX;
     goto checkhash;
   }
      else
   {
     /* This group is already present in NetDoc.
           * Update the ServerFirst and ServerLast fields.
           */
     netgroup = (TypGroup far *) ((char far *) NetHashTable[hashval] + sizeof (TypLine));
     GetNum (&restline, &ArtNum);
     netgroup->ServerLast = ArtNum;
     GetNum (&restline, &ArtNum);
     netgroup->ServerFirst = ArtNum;
     netgroup->ServerEstNum = (netgroup->ServerLast - netgroup->ServerFirst + 1);
   }
    }

}

/*--- function AddGroupToTable ----------------------------------------
 *
 *  Add a group line, formatted for eventual inclusion in NetDoc,
 *  to NewGroupTable.
 *
 *  Entry:  GroupLine      is the line to add to the table.
 *
 *  Exit:   NewGroupTable  contains the line
 *          nNewGroups     has been incremented.
 */
void
AddGroupToTable (char far * GroupLine)
{
  HANDLE hLine;
  char far *AllocPtr;
  int mylength;

  /* Create a copy of this line in far memory and place pointers
   * to in our arrays.
   */

  mylength = ((TypLine far *) GroupLine)->length + sizeof (HANDLE);
  hLine = GlobalAlloc (GMEM_MOVEABLE, (long) mylength);
  AllocPtr = (char far *) GlobalLock (hLine);
  MoveBytes ((char far *) &hLine, AllocPtr, sizeof (hLine));
  MoveBytes ((char far *) GroupLine, AllocPtr + sizeof (hLine), mylength - sizeof (hLine));

  NewGroupTable[nNewGroups] = AllocPtr;
  nNewGroups++;
}

/*--- function HashGroup -------------------------------------------------
 *
 *  Hash a string into an unsigned integer.
 *
 *  This hash function is designed based on information from a
 *  handout for a UC Berkeley class CS 60C, Spring 1990, Clancy/
 *  Harrison.  I picked up the handout while wandering around on
 *  Berkeley's campus in May 1990.  The handout in turn is based
 *  on the McKenzie, et al., article "Selecting a Hashing Algorithm"
 *  in Software Practice and Experience, Vol 20, no 2, Feb 1990.
 *  The algorithm is similar to that used in the AT&T C++ compiler.
 *  /mrr
 */

unsigned int
HashGroup (gname)
     unsigned char far *gname;
{
  long unsigned int sum = 0;
  unsigned int hash;

  for (; *gname; gname++)
    {
      sum = (sum << 1) + *gname;
    }
  hash = (unsigned) (sum % HASHMAX);
  return (hash);
}


/*--- function ProcEndList -------------------------------------------
 *
 *  Do the final processing when we have reached the end of the
 *  list of newsgroups sent us via the LIST command.
 */

void
ProcEndList ()
{
  FARPROC lpfnWinVnGroupListDlg;
  WORD LockCount;


  ShellSort (NewGroupTable, nNewGroups, sizeof (void far *), GroupCompare);

  lpfnWinVnGroupListDlg = MakeProcInstance (WinVnGroupListDlg, hInst);

  /* Display dialog box of new groups.
   */
  if (nNewGroups && DialogBox (hInst, "WinVnGroupList", hWndConf, lpfnWinVnGroupListDlg))
    {
      /* The user has clicked OK, so add all the new groups to the
       * Net document.  Subscribed groups go at the end of the Subscribed
       * section at the top of the doc.
       * Unsubscribed groups go in the section below, in alphabetical order.
       */
      MergeGroups (ADD_SUBSCRIBED_END_OF_SUB);
      SetGroupActiveLines();

    }

  /* Unlock and/or free memory in NetDoc and NewGroupTable.  */

  CleanUpGroupTable ();

  /* Unlock and free the hash table.   */

  LockCount = GMEM_LOCKCOUNT & GlobalFlags (hNetHashTable);
  MyGlobalUnlock (hNetHashTable);
  GlobalFree (hNetHashTable);

  InvalidateRect (hWndConf, NULL, FALSE);
  SetNetDocTitle ();
}

/*--- function GroupCompare --------------------------------------------
 *
 *  Compare two group lines alphabetically by group name.
 */
int
GroupCompare (g1, g2)
     TypLine const far *far * g1;
     TypLine const far *far * g2;
{
  char far *gch1, far * gch2;

  gch1 = (char far *) *g1 + sizeof (HANDLE) + sizeof (TypLine) + sizeof (TypGroup);
  gch2 = (char far *) *g2 + sizeof (HANDLE) + sizeof (TypLine) + sizeof (TypGroup);

  return (lstrcmp (gch1, gch2));
}


/*-- function WinVnGroupListDlg ---------------------------------------
 *
 *  Dialog function to handle selection of new newsgroups to
 *  subscribe to.  (I know, don't end a sentence with "to".)
 */

BOOL FAR PASCAL 
WinVnGroupListDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
  int j, selstate;
  char far *cptr;
  void far *AllocPtr;
  TypGroup far *group;


  switch (iMessage)
    {

    case WM_INITDIALOG:
      hDlgList = GetDlgItem (hDlg, IDD_GROUP_LISTBOX);

      SendMessage (hDlgList, WM_SETREDRAW, FALSE, 0L);
      for (j = 0; j < nNewGroups; j++)
   {
     cptr = 0;
     cptr = (NewGroupTable[j]);
     cptr += sizeof (HANDLE) + sizeof (TypLine) + sizeof (TypGroup);
/* Petzold misdocuments this??? Win3.1 kernel complains about this -1 */
/*      SendMessage (hDlgList, LB_ADDSTRING, -1, (LONG) cptr); */
     SendMessage (hDlgList, LB_ADDSTRING, 0, (LONG) cptr);
   }
      SendMessage (hDlgList, WM_SETREDRAW, TRUE, 0L);
      return TRUE;
      break;

    case WM_COMMAND:
      switch (wParam)
   {
   case IDOK:
     EndDialog (hDlg, TRUE);
     break;

   case IDCANCEL:
     EndDialog (hDlg, FALSE);
     break;
   default:
     return FALSE;
   }
      break;

    case WM_DESTROY:
      /* Mark the groups pointed to by NewGroupTable as Subscribed
       * or not, depending upon whether they are now selected.
       */
      for (j = 0; j < nNewGroups; j++)
   {
     selstate = (WORD) SendMessage (hDlgList, LB_GETSEL, j, 0L);
     AllocPtr = (char far *) NewGroupTable[j] + sizeof (HANDLE);
     group = (TypGroup far *) ((char far *) AllocPtr + sizeof (TypLine));
     group->Subscribed = selstate;
   }
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}

/*--- function PositionEndSubscribed ----------------------------------
 *
 *  Position a pointer to the end of the subscribed section at the
 *  beginning of the net document.
 *
 *  Entry   None
 *
 *  Exit    BlockPtr and LinePtr  point to the place in NetDoc just
 *            beyond the last subscribed group.  We assume that
 *            all subscribed groups go at the beginning of the
 *            document.
 */
void
PositionEndSubscribed (TypBlock far ** BlockPtr, TypLine far ** LinePtr)
{
  BOOL advance;
  TypGroup far *group;

  TopOfDoc (&NetDoc, BlockPtr, LinePtr);
  advance = TRUE;
  do
    {
      group = (TypGroup far *) ((char far *) *LinePtr + sizeof (TypLine));
      if (group->Subscribed)
   {
     advance = NextLine (BlockPtr, LinePtr);
   }
      else
   {
     advance = FALSE;
   }
    }
  while (advance);
}

/*--- function MergeGroups ----------------------------------------
 *
 *  Merge a list of groups into NetDoc.
 *
 *  Entry:  NewGroupTable  is an array of pointers to TypGroup structures
 *                         of groups to be merged into NetDoc.
 *          hNewGroupTable is the handle to the above.
 *          nNewGroups     is the number of groups in the table.
 *          WhereSubscribed   indicates where new subscribed groups
 *                         should be added.
 *                         ADD_SUBSCRIBED_END_OF_SUB indicates that
 *                           they should be added at the end of the subscribed
 *                           list, before the unsubscribed groups.
 *                         ADD_SUBSCRIBED_TOP_OF_DOC indicates that they
 *                           should be added at the top of the document.
 *  Exit:   The groups in the table have been added to NetDoc, and
 *            the entries in GroupTable have been freed from memory.
 *            Also, GroupTable itself has been freed.
 */
void
MergeGroups (int WhereSubscribed)
{
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
  TypGroup far *group;
  char far *netcptr;
  char far *grpcptr;
  void far *AllocPtr;
  HANDLE hLine;
  unsigned int Offset;
  TypLineID MyLineID;
  char myline[BLOCK_SIZE];
  int j, advance, at_end = 0;

  switch (WhereSubscribed)
    {
    case ADD_SUBSCRIBED_END_OF_SUB:
      PositionEndSubscribed (&BlockPtr, &LinePtr);
      break;
    case ADD_SUBSCRIBED_TOP_OF_DOC:
      TopOfDoc (&NetDoc, &BlockPtr, &LinePtr);
      break;
    }

  /* BlockPtr and LinePtr point to the
   * place to add new subscribed groups.
   * Loop through the new groups; for subscribed groups, add
   * them to NetDoc at this point.
   * For each subscribed group, unlock and free the corresponding
   * line pointed to by NewGroupTable.  Set the table entry
   * to 0 to indicate that this group has been dealt with.
   */

  for (j = 0; j < nNewGroups; j++)
    {
      AllocPtr = (char far *) NewGroupTable[j] + sizeof (HANDLE);
      group = (TypGroup far *) ((char far *) AllocPtr + sizeof (TypLine));
      if (group->Subscribed)
   {
     /* This group has been selected and should be subscribed to.
           */
     MoveBytes (AllocPtr, myline, ((TypLine far *) AllocPtr)->length); 
     LinePtr->active = TRUE;
     AddLine ((TypLine *) myline, &BlockPtr, &LinePtr);
     NetDoc.ActiveLines++;
     hLine = *((HANDLE far *) NewGroupTable[j]);
     MyGlobalUnlock (hLine);
     GlobalFree (hLine);
     NewGroupTable[j] = (void far *) 0;
   }
    }

  PositionEndSubscribed (&BlockPtr, &LinePtr);

  /* Now take a second pass through NewGroupTable, for the
   * unsubscribed groups.  If NewGroupTable[j] is non-zero, then
   * that group should be entered into the second, unsubscribed
   * section of NetDoc, merged in in alphabetical order.
   *
   * BlockPtr and LinePtr point to the first unsubscribed group.
   */

  for (j = 0; j < nNewGroups; j++)
    {
      if (NewGroupTable[j])
   {
     /* Search for the right place to add this line. */

     AllocPtr = (char far *) NewGroupTable[j] + sizeof (HANDLE);
     grpcptr = ((char far *) AllocPtr) + sizeof (TypLine) + sizeof (TypGroup);
     advance = TRUE;

     if (!at_end) {
     do
       {
         netcptr = ((char far *) LinePtr) + sizeof (TypLine) + sizeof (TypGroup);
         if (lstrcmp (grpcptr, netcptr) < 0)
      {
        advance = FALSE;
      }
         else
      {
        advance = NextLine (&BlockPtr, &LinePtr);
        if (!advance) at_end = 1; /* possible bug, getting bad netcptr (smr) */
      }
       }
     while (advance);
     }

     /* Now add the new group at this point */
     MoveBytes (AllocPtr, myline, ((TypLine far *) AllocPtr)->length); 
     LinePtr->active = ShowUnsubscribed;
     AddLine ((TypLine *) myline, &BlockPtr, &LinePtr);

     /* Unlock and free this entry in NewGroupTable. */
     hLine = *((HANDLE far *) NewGroupTable[j]);
     MyGlobalUnlock (hLine);
     GlobalFree (hLine);
   }
    }

  UnlockLine (BlockPtr, LinePtr, &hLine, &Offset, &MyLineID);
}

/*--- function CleanUpGroupTable ------------------------------------
 *
 *  Clean up after doing processing to add or move groups in NetDoc.
 */
void
CleanUpGroupTable ()
{
  HANDLE hBlock, hBlockNext;
  TypBlock far *BlockPtr;

  /* Unlock all blocks in the NetDoc document.
   */

  hBlock = NetDoc.hFirstBlock;
  do
    {
      BlockPtr = (TypBlock far *) GlobalLock (hBlock);
      hBlockNext = BlockPtr->hNextBlock;

      MyGlobalUnlock (hBlock);
      MyGlobalUnlock (hBlock);
      hBlock = hBlockNext;
    }
  while (hBlock);

  /* Unlock and free the NewGroupTable itself.  */

  MyGlobalUnlock (hNewGroupTable);
  GlobalFree (hNewGroupTable);

}
