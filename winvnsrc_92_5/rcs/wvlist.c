head	1.15;
access;
symbols
	V80:1.1
	V76d:1.1;
locks; strict;
comment	@ * @;


1.15
date	94.09.02.23.49.10;	author martin;	state Exp;
branches;
next	1.14;

1.14
date	94.08.26.22.14.43;	author dumoulin;	state Exp;
branches;
next	1.13;

1.13
date	94.08.23.23.17.01;	author martin;	state Exp;
branches;
next	1.12;

1.12
date	94.07.26.18.39.05;	author dumoulin;	state Exp;
branches;
next	1.11;

1.11
date	94.06.08.21.01.45;	author gardnerd;	state Exp;
branches;
next	1.10;

1.10
date	94.05.27.01.29.29;	author rushing;	state Exp;
branches;
next	1.9;

1.9
date	94.05.19.02.02.10;	author rushing;	state Exp;
branches;
next	1.8;

1.8
date	94.01.12.19.25.59;	author mrr;	state Exp;
branches;
next	1.7;

1.7
date	93.12.08.01.28.38;	author rushing;	state Exp;
branches;
next	1.6;

1.6
date	93.10.12.17.47.26;	author rushing;	state Exp;
branches;
next	1.5;

1.5
date	93.08.18.21.51.13;	author rushing;	state Exp;
branches;
next	1.4;

1.4
date	93.06.28.17.52.17;	author rushing;	state Exp;
branches;
next	1.3;

1.3
date	93.06.24.17.07.05;	author dumoulin;	state Exp;
branches;
next	1.2;

1.2
date	93.06.22.16.16.13;	author rushing;	state Exp;
branches;
next	1.1;

1.1
date	93.02.16.20.54.22;	author rushing;	state Exp;
branches;
next	;


desc
@winvn version 0.76 placed into RCS
@


1.15
log
@fixes for new hashtable stuff
@
text
@
/*
 *
 * $Id: wvlist.c 1.14 1994/08/26 22:14:43 dumoulin Exp martin $
 * $Log: wvlist.c $
 * Revision 1.14  1994/08/26  22:14:43  dumoulin
 * Attempts to fix horizontal scrolling in newgroups box
 *
 * Revision 1.13  1994/08/23  23:17:01  martin
 * new group list box & new hash table mechanism
 *
 * Revision 1.12  1994/07/26  18:39:05  dumoulin
 * Fixed WinVN bombing when number of new newsgroups exceeds
 * 3000.  Now the new newsgroups dialog box doesn't get called.
 *
 * Revision 1.11  1994/06/08  21:01:45  gardnerd
 * more scrolling changes...
 *
 * Revision 1.10  1994/05/27  01:29:29  rushing
 * unnecessary winundoc.h
 *
 * Revision 1.9  1994/05/19  02:02:10  rushing
 * changes for gensock & version 0.91
 *
 * Revision 1.8  1994/01/12  19:25:59  mrr
 * mrr mods 4
 *
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
#ifdef WIN32
#include "windowsx.h"
#endif /* WIN32 */
#include "wvglob.h"
#include "winvn.h"

#include "dos.h"

#include "primes.h"

#include <ctype.h>


/* The size of the chunks of memory that make up the new group list data */
#define CHUNKSIZE       64L*1024L

TypLine far *far * NetHashTable;
HANDLE hNetHashTable;

HANDLE htohNewGroupLines;
HANDLE far *lphNewGroupLines; /* array of handles to new group lines */

DWORD NewGroupDataSize; /* The size (in bytes) of NewGroupData */
void huge *CurrentSpot; /* The pointer to the next available spot in NewGroupData */
unsigned long   BytesSoFar;

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


  CommState = ST_LIST_RESP;
  CommBusy = TRUE;
  PutCommLine ("LIST");
  Initializing = INIT_SCANNING_NETDOC;
  InvalidateRect (hWndConf, NULL, FALSE);
  SendMessage (hWndConf, WM_PAINT, 0, 0L);

  /* Set up table of pointers to new group lines.*/
  InitGroupTable();

  /* Initialize the hash table */
  InitHashTable(LinesInRC);

  /* And set up the hash table for stuff from the newsrc */
  HashNetGroups (&NetDoc, NetHashTable);
  InvalidateRect (hWndConf, NULL, FALSE);
  Initializing = INIT_GETTING_LIST;
  SendMessage (hWndConf, WM_PAINT, 0, 0L);


}

/*--- function InitHashTable -------------------------------------------
 *
 *  Allocate and zero the NetHashTable. The argument is the number of
 *  elements to be hashed (NOT the number of elements in the hash table)
 *
 *  Exit:   hNetHashTable     is the handle
 *          NetHashTable      points to the table
 *                      HashTableSize     The number of elements in the hash table
 */
void
InitHashTable(unsigned int Elements){

  /* First, figure out how large the hashtable should be.
   * The basic algorithm used here is to double the size of what you're
   * hashing, rounded up to the next nearest prime number
   */
  HashTableSize = (unsigned int)FindNextPrime((unsigned long)Elements * 2L);

  /* Set up hash table for group names */
  hNetHashTable = GlobalAlloc (GMEM_MOVEABLE|GMEM_ZEROINIT,
					       (unsigned long) HashTableSize *
					       sizeof (TypLine far *));
  NetHashTable = (TypLine far * far *) GlobalLock (hNetHashTable);

}


/*--- function InitGroupTable -------------------------------------------
 *
 *  Allocate the first CHUNKSIZE bytes of memory for the
 * new group lines.
 *
 *  Exit:   hNewGroupData    is the handle
 *          NewGroupData     points to the table
 *          nNewGroups        has been initialized to 0.
 *                      NewGroupDataSize  is the current size of the NewGroupData
 */
void
InitGroupTable (void)
{
  /* Allocate the first chunk */
  NewGroupDataSize = CHUNKSIZE;
  hNewGroupData = GlobalAlloc (GMEM_MOVEABLE, NewGroupDataSize);
  NewGroupData = (void far *)GlobalLock(hNewGroupData);
  CurrentSpot = NewGroupData;
  BytesSoFar = 0;

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
	 hashval = (hashval + 1) % HashTableSize;
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

    CrackGroupLine (ListLine, (TypLine *) (mygroupline));
    mygroup = (TypGroup *) (mygroupline + sizeof (TypLine));
    GetNum (&restline, &(mygroup->ServerLast));
    GetNum (&restline, &(mygroup->ServerFirst));
    mygroup->ServerEstNum = mygroup->ServerLast - mygroup->ServerFirst+ 1;
    mygroup->HighestPrevSeen = 0;
    mygroup->nRanges = 0;

    AddGroupToTable (mygroupline);
    }
  else
    {
      textptr = ((char far *) NetHashTable[hashval] + sizeof (TypLine) + sizeof (TypGroup));
      if (lstrcmp (textptr, ListLine))
   {
     hashval = (hashval + 1) % HashTableSize;
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
  char huge *NextPtr;
  char huge *EndPtr;

  NextPtr = (char huge *)CurrentSpot + ((TypLine far *) GroupLine)->length;
  EndPtr = (char huge *)NewGroupData + NewGroupDataSize;

  /* Check to see if the current data will put us over the edge */
  if (NextPtr > EndPtr){
     GlobalUnlock(hNewGroupData);
     NewGroupDataSize += CHUNKSIZE;
	 hNewGroupData = GlobalReAlloc(hNewGroupData, NewGroupDataSize,
				       GMEM_MOVEABLE);
	 NewGroupData = GlobalLock(hNewGroupData);
	 CurrentSpot = (char huge *)NewGroupData + BytesSoFar;
	 NextPtr = (char huge *)CurrentSpot + ((TypLine far *) GroupLine)->length;
  }

  /* Create a copy of this line in far memory
   */
  MoveBytes((char far *)GroupLine, CurrentSpot,
					((TypLine far *) GroupLine)->length);
  nNewGroups++;

  BytesSoFar += ((TypLine huge *) CurrentSpot)->length;
  (char huge *)CurrentSpot = NextPtr;

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
     unsigned char huge *gname;
{
  long unsigned int sum = 0;
  unsigned int hash;

  for (; *gname; gname++)
    {
      sum = (sum << 1) + *gname;
    }
  hash = (unsigned int) (sum % HashTableSize);
  return (hash);
}

/*--- function BuildPtrList ------------------------------------------
 *
 *  Build both the array of pointers into the new group data, as well
 *  as build the hashtable.
 */
void
BuildPtrList ()
{
  int i;

  /* Build the pointer array after the fact */
  hNewGroupTable = GlobalAlloc (GMEM_MOVEABLE, (long) nNewGroups * sizeof (TypLine huge *));
  NewGroupTable = (void huge * huge *) GlobalLock (hNewGroupTable);
  CurrentSpot = NewGroupData;

  for(i=0 ; i < nNewGroups ; i++){
	NewGroupTable[i]=CurrentSpot;
	(char huge *)CurrentSpot += ((TypLine huge *) CurrentSpot)->length;
  }
}

/*--- function BuildHashTable ------------------------------------------
 *
 *  Build the hashtable from the NewGroupTable.
 */
void
BuildHashTable ()
{
  unsigned int hashval;
  char huge *textptr;
  int i;

  for(i=0 ; i < nNewGroups ; i++){
	/* Plus fill in the hash table */
    textptr = (char huge *)NewGroupTable[i] + sizeof(TypLine)
		+ sizeof(TypGroup);
    hashval = HashGroup (textptr);
    while (NetHashTable[hashval])
    {
       hashval = (hashval + 1) % HashTableSize;
    }
    NetHashTable[hashval] = NewGroupTable[i];
  }

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

  /* Well, we need a new and different hash table now, so close out the
     old one */
  LockCount = GMEM_LOCKCOUNT & GlobalFlags (hNetHashTable);
  MyGlobalUnlock (hNetHashTable);
  GlobalFree (hNetHashTable);

  InitHashTable (nNewGroups);
  BuildPtrList ();
  BuildHashTable ();

  ShellSort (NewGroupTable, nNewGroups, sizeof (void huge *), GroupCompare);

  lpfnWinVnGroupListDlg = MakeProcInstance (WinVnGroupListDlg, hInst);

  /* Display dialog box of new groups.  */
  if (nNewGroups && DialogBox (hInst, "WinVnGroupList", hWndConf, lpfnWinVnGroupListDlg))
  {
    /* The user has clicked OK, so add all the new groups to the
     * Net document.  Subscribed groups go at the end of the Subscribed
     * section at the top of the doc.
     * Unsubscribed groups go in the section below, in alphabetical order.
     */
    MergeGroups (ADD_SUBSCRIBED_END_OF_SUB);
    NetDoc.LongestLine = 0;
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
GroupCompare (TypLine const huge * huge * g1,
              TypLine const huge * huge * g2)
{
  char huge *gch1, huge * gch2;

  gch1 = (char huge *) *g1 + sizeof (TypLine) + sizeof (TypGroup);
  gch2 = (char huge *) *g2 + sizeof (TypLine) + sizeof (TypGroup);

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
     WPARAM wParam;
     LPARAM lParam;
{
  int j;
  LRESULT nItem;
  char TmpBuf[80];
  char far *CurName;
  char huge * cptr;
  TypHier far *CurHier;
  static HANDLE hStartHier,hCurHier;
  unsigned int hashval;
  TypGroup huge *group;
  HWND hDlgHierList;       /* Handle to child hierarchy list box window. */
  HWND hDlgSubList;
  HWND hDlgUnSubList;
  switch (iMessage)
    {

    case WM_INITDIALOG:
      /* First build the list of top-level hierarchies */
      CurHier =(TypHier *)NULL;
      hStartHier = hCurHier = (HANDLE)NULL;
      for (j = 0; j < nNewGroups; j++)
	  {
	cptr = 0;
	cptr = (NewGroupTable[j]);
	cptr += sizeof (TypLine) + sizeof (TypGroup);
	if((CurHier == NULL) ||
	   (strnicmp(CurName,cptr,strcspn(cptr,"."))
	    != 0)){
	   HANDLE hTmpHandle;
	   int NameLen;
	   /* Need a new link */
	   hTmpHandle = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,
								sizeof(TypHier));
	   if(CurHier){
		CurHier->hNext = hTmpHandle;
		GlobalUnlock(CurHier->hName);
		GlobalUnlock(hCurHier);
		hCurHier = hTmpHandle;
		CurHier = (TypHier far *)GlobalLock(hCurHier);
	   } else {
		hCurHier = hStartHier = hTmpHandle;
		CurHier = (TypHier far *)GlobalLock(hCurHier);
	   }
	   /* And now for the actual hierarchy name */
	   NameLen = strcspn(cptr,".");
	   CurHier->hName = GlobalAlloc(GMEM_MOVEABLE,NameLen+1);
	   CurName = GlobalLock(CurHier->hName);
	   strncpy(CurName,cptr,NameLen);
	   CurName[NameLen] = '\0';
		   /* And the start index */
		   CurHier->Start = j;
	}
	/* Increment the number of groups _every_ time */
	CurHier->NumGroups++;
	  }
      GlobalUnlock(CurHier->hName);
      GlobalUnlock(hCurHier);

      /* Now, load the HIERARCY box */
      hDlgHierList = GetDlgItem (hDlg, IDD_HIERARCHY_LISTBOX);
      SendMessage (hDlgHierList, WM_SETREDRAW, FALSE, 0L);
      hCurHier = hStartHier;
      while(hCurHier){
	HANDLE hTmpHandle;
	CurHier = (TypHier far *)GlobalLock(hCurHier);
	CurName = GlobalLock(CurHier->hName);
	SendMessage (hDlgHierList, LB_ADDSTRING, 0, (LPARAM)CurName);
	hTmpHandle = CurHier->hNext;
		GlobalUnlock(CurHier->hName);
	GlobalUnlock(hCurHier);
	hCurHier = hTmpHandle;
      }
      SendMessage (hDlgHierList, WM_SETREDRAW, TRUE, 0L);

      /* And select the 0th item to kick things off */
      SendMessage (hDlgHierList, LB_SETCURSEL, 0 , 0);
#ifdef WIN32
      SendMessage (hDlg,WM_COMMAND,(WPARAM)MAKEWPARAM(IDD_HIERARCHY_LISTBOX,
		   LBN_SELCHANGE),(LPARAM)GetDlgItem(hDlg,IDD_HIERARCHY_LISTBOX));
#else
      SendMessage (hDlg,WM_COMMAND,IDD_HIERARCHY_LISTBOX,
		   (LPARAM)MAKELPARAM(GetDlgItem(hDlg,IDD_HIERARCHY_LISTBOX),
							  LBN_SELCHANGE));
#endif /* WIN32 */

      return TRUE;
      break;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDOK:
	EndDialog (hDlg, TRUE);
	break;
      case IDCANCEL:
	EndDialog (hDlg, FALSE);
	break;
      case IDD_SUBSCRIBED_GROUP_LISTBOX:
#ifdef WIN32
	    switch(HIWORD(wParam))
#else
	    switch(HIWORD(lParam))
#endif /* WIN32 */
	    {
	    case LBN_DBLCLK:
	      /* switch to unsubscribed */
#ifdef WIN32
	  nItem = SendMessage((HWND)lParam,LB_GETCURSEL, 0 , 0);
	  SendMessage((HWND)lParam,LB_GETTEXT,(WPARAM)nItem,
		      (LPARAM) TmpBuf);
	  SendMessage((HWND)lParam,LB_DELETESTRING,(WPARAM)nItem,
		      (LPARAM) 0);

#else
	  nItem = SendMessage(LOWORD(lParam),LB_GETCURSEL, 0 , 0);
	  SendMessage(LOWORD(lParam),LB_GETTEXT,(WPARAM)nItem,
		      (LPARAM) TmpBuf);
	  SendMessage(LOWORD(lParam),LB_DELETESTRING,(WPARAM)nItem,
		      (LPARAM) 0);
#endif /* WIN32 */
		  /* Now pull the stucture out of the hash table */
	  hashval = HashGroup (TmpBuf);
	      while (strcmp(
	   (char huge *)NetHashTable[hashval]+sizeof(TypLine)+sizeof(TypGroup),
	   TmpBuf) != 0)
	  {
	     hashval = (hashval + 1) % HashTableSize;
	  }
	  group = (TypGroup huge *)
	       ((char huge *)NetHashTable[hashval] + sizeof(TypLine));
	  group->Subscribed = 0;

	  SendMessage(GetDlgItem(hDlg,IDD_UNSUBSCRIBED_GROUP_LISTBOX),
				  LB_ADDSTRING, 0, (LPARAM)TmpBuf);
	  break;
	default:
		return FALSE;

	}
	break;
      case IDD_UNSUBSCRIBED_GROUP_LISTBOX:
#ifdef WIN32
	    switch(HIWORD(wParam))
#else
	    switch(HIWORD(lParam))
#endif /* WIN32 */
	    {
	    case LBN_DBLCLK:
	      /* switch to subscribed */
#ifdef WIN32
	  nItem = SendMessage((HWND)lParam,LB_GETCURSEL, 0 , 0);
	  SendMessage((HWND)lParam,LB_GETTEXT,(WPARAM)nItem,
		      (LPARAM) TmpBuf);
	  SendMessage((HWND)lParam,LB_DELETESTRING,(WPARAM)nItem,
		      (LPARAM) 0);
#else
	  nItem = SendMessage(LOWORD(lParam),LB_GETCURSEL, 0 , 0);
	  SendMessage(LOWORD(lParam),LB_GETTEXT,(WPARAM)nItem,
		      (LPARAM) TmpBuf);
	  SendMessage(LOWORD(lParam),LB_DELETESTRING,(WPARAM)nItem,
		      (LPARAM) 0);
#endif /* WIN32 */
	  /* Now pull the stucture out of the hash table */
	  hashval = HashGroup (TmpBuf);
	  while (strcmp((char huge *)NetHashTable[hashval]+
					   sizeof(TypLine)+sizeof(TypGroup),
					   TmpBuf) != 0)
	  {
	     hashval = (hashval + 1) % HashTableSize;
	  }
	  group = (TypGroup huge *)
	       ((char huge *)NetHashTable[hashval] + sizeof(TypLine));
	  group->Subscribed = 1;

	  SendMessage(GetDlgItem(hDlg,IDD_SUBSCRIBED_GROUP_LISTBOX),
				  LB_ADDSTRING,0, (LPARAM)TmpBuf);
	  break;
	default:
		return FALSE;

	}
	break;
      case IDD_HIERARCHY_LISTBOX:
#ifdef WIN32
	    switch(HIWORD(wParam))
#else
	    switch(HIWORD(lParam))
#endif /* WIN32 */
	    {
	    case LBN_SELCHANGE:
	      /* Find the info for this hierarchy */
#ifdef WIN32
	  nItem = SendMessage((HWND)lParam,LB_GETCURSEL, 0 , 0);
	  SendMessage((HWND)lParam,LB_GETTEXT,(WPARAM)nItem,
		      (LPARAM) TmpBuf);
#else
	  nItem = SendMessage(LOWORD(lParam),LB_GETCURSEL, 0 , 0);
	  SendMessage(LOWORD(lParam),LB_GETTEXT,(WPARAM)nItem,
		      (LPARAM) TmpBuf);
#endif /* WIN32 */
	  hCurHier = hStartHier;
	  while(hCurHier){
	    HANDLE hTmpHandle;
	    CurHier = (TypHier far *)GlobalLock(hCurHier);
		CurName = GlobalLock(CurHier->hName);
			GlobalUnlock(CurHier->hName);
			if(stricmp(CurName,TmpBuf) == 0)
				break;
		hTmpHandle = CurHier->hNext;
		    GlobalUnlock(hCurHier);
	    hCurHier = hTmpHandle;
	  }
	      /* clear the existing listboxes */
#ifdef WIN32
	      SendMessage ((HWND)lParam, WM_SETREDRAW, FALSE, 0L);
#else
	      SendMessage (LOWORD(lParam), WM_SETREDRAW, FALSE, 0L);
#endif /* WIN32 */
	      hDlgSubList = GetDlgItem (hDlg, IDD_SUBSCRIBED_GROUP_LISTBOX);
	      hDlgUnSubList = GetDlgItem (hDlg, IDD_UNSUBSCRIBED_GROUP_LISTBOX);
	      SendMessage (hDlgUnSubList, LB_RESETCONTENT,0,0);
	      SendMessage (hDlgSubList, LB_RESETCONTENT,0,0);
	      /* load the subscribed/unsubscribed boxes */
	      for (j=CurHier->Start; j<(CurHier->Start + CurHier->NumGroups); j++){
			TypGroup huge *group;
			group = (TypGroup huge *)
				    ((char huge *)NewGroupTable[j] + sizeof(TypLine));
		if (group->Subscribed)
	      SendMessage (hDlgSubList, LB_ADDSTRING, 0,
			   (LPARAM)((char huge*)group + sizeof(TypGroup)));
	    else
	      SendMessage (hDlgUnSubList, LB_ADDSTRING, 0,
			   (LPARAM)((char huge *)group + sizeof(TypGroup)));
	  }

	//  Horizontal scrolling doesn't work.   These are all attempts to
	//  fix it that don't work.  However, it isn't worth holding up
	//  the release of .92-3 for this one....JD 8/26/94


	//  SendDlgItemMessage(GetDlgItem(hDlg,IDD_SUBSCRIBED_GROUP_LISTBOX), IDD_SUBSCRIBED_GROUP_LISTBOX,       // JD 8/25/94
	//                         LB_SETHORIZONTALEXTENT,(WPARAM)200,(LPARAM)0);
	//  SetScrollRange(GetDlgItem(hDlg,IDD_SUBSCRIBED_GROUP_LISTBOX),
	//             SB_CTL, (int) 0, (int) 1200, TRUE);   // JD 8/26/94
	//
	//  SetScrollRange(GetDlgItem(hDlg,IDD_UNSUBSCRIBED_GROUP_LISTBOX),
	//             SB_HORZ, (int) 0, (int) 20, TRUE);   // JD 8/26/94

#ifdef WIN32
	  SendMessage ((HWND)lParam, WM_SETREDRAW, TRUE, 0L);
#else
	  SendMessage (LOWORD(lParam), WM_SETREDRAW, TRUE, 0L);
#endif /* WIN32 */
		  GlobalUnlock(hCurHier);
		  break;
	default:
	  return FALSE;
	}
	break;
      default:
	return FALSE;
    }
    break;

    case WM_DESTROY:
      /* Ok, now we tear down the hierarchy structure */
	  hCurHier = hStartHier;
	  while(hCurHier){
		HANDLE  hTmpHandle;
		CurHier = (TypHier far *)GlobalLock(hCurHier);
		GlobalFree(CurHier->hName);
		hTmpHandle = CurHier->hNext;
		GlobalUnlock(hCurHier);
		GlobalFree(hCurHier);
		hCurHier = hTmpHandle;
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
      AllocPtr = (char far *) NewGroupTable[j];
      group = (TypGroup far *) ((char far *) AllocPtr + sizeof (TypLine));
      if (group->Subscribed)
   {
     /* This group has been selected and should be subscribed to.
	   */
     MoveBytes (AllocPtr, myline, ((TypLine far *) AllocPtr)->length);
     LinePtr->active = TRUE;
     AddLine ((TypLine *) myline, &BlockPtr, &LinePtr);
     NetDoc.ActiveLines++;
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

     AllocPtr = (char far *) NewGroupTable[j];
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

  /* Unlock and free the NewGroupTable and NewGroupData.  */

  GlobalUnlock (hNewGroupData);
  GlobalFree (hNewGroupData);
  MyGlobalUnlock (hNewGroupTable);
  GlobalFree (hNewGroupTable);

}

/*--- function FindNextPrime ------------------------------------
 *
 *  Find the smallest prime that is larger than the given argument.
 *  Extreemly handy for those annoying dynamic hash tables.
 */
unsigned long
FindNextPrime(unsigned long OrigVal)
{

	int MaxPrimeIndex = (sizeof(prime)/sizeof(prime[0]))-1;
	int     IndexA = 0;
	int IndexB = MaxPrimeIndex;
	int TmpIndex;

    /* If we're larger than the max prime we know of, just return it */
	if(OrigVal >= prime[MaxPrimeIndex])
		return(prime[MaxPrimeIndex]);

	/* We do this by doing a binary search of the prime array */
	do {
		TmpIndex = IndexA +((IndexB-IndexA)/2);
		if( OrigVal == prime[TmpIndex])
			return(prime[TmpIndex+1]);
		if( OrigVal < prime[TmpIndex])
			IndexB = TmpIndex;
		else
			IndexA = TmpIndex;
	} while ( (IndexB-IndexA) > 1);

	return(prime[IndexA+1]);
}
@


1.14
log
@Attempts to fix horizontal scrolling in newgroups box
@
text
@d4 1
a4 1
 * $Id: wvlist.c 1.13 1994/08/23 23:17:01 martin Exp dumoulin $
d6 3
d59 3
d73 1
a73 1
#define CHUNKSIZE	64L*1024L
d83 1
a83 1
unsigned long	BytesSoFar;
d130 1
a130 1
  
d137 1
a137 1
 
d142 1
a142 1
 *  Allocate and zero the NetHashTable. The argument is the number of 
d147 1
a147 1
 *			HashTableSize	  The number of elements in the hash table
d151 1
a151 1
 
d154 1
a154 1
   * hashing, rounded up to the next nearest prime number 
d156 2
a157 2
  HashTableSize = (unsigned int)FindNextPrime((unsigned long)Elements * 2L); 
 
d160 2
a161 1
  				               HashTableSize * sizeof (TypLine far *));
d175 1
a175 1
 *			NewGroupDataSize  is the current size of the NewGroupData
d182 1
a182 1
  hNewGroupData = GlobalAlloc (GMEM_FIXED, NewGroupDataSize);
d186 1
a186 1
  
d213 1
a213 1
 
d224 1
a224 1
  
d245 1
a245 1
         hashval = (hashval + 1) % HashTableSize;
d300 1
a300 1
    mygroup->ServerEstNum = mygroup->ServerLast - mygroup->ServerFirst+ 1; 
d317 2
a318 2
           * Update the ServerFirst and ServerLast fields.
           */
d345 1
a345 1
  
d350 1
a350 1
  if (NextPtr > EndPtr){ 
d353 4
a356 3
 	 hNewGroupData = GlobalReAlloc(hNewGroupData, NewGroupDataSize,0);
 	 NewGroupData = GlobalLock(hNewGroupData);
 	 CurrentSpot = (char huge *)NewGroupData + BytesSoFar;
d359 2
a360 2
 
  /* Create a copy of this line in far memory 
d363 1
a363 1
  					((TypLine far *) GroupLine)->length);
a364 3
  
  BytesSoFar += ((TypLine far *) CurrentSpot)->length;
  (char far *)CurrentSpot = NextPtr;
d366 3
d387 1
a387 1
     unsigned char far *gname;
d404 1
a404 1
 */ 
d411 2
a412 2
  hNewGroupTable = GlobalAlloc (GMEM_MOVEABLE, (long) nNewGroups * sizeof (TypLine far *));
  NewGroupTable = (void far * far *) GlobalLock (hNewGroupTable);
d414 1
a414 1
  
d417 1
a417 1
	(char huge *)CurrentSpot += ((TypLine far *) CurrentSpot)->length;
d419 1
a419 1
}  	
d424 1
a424 1
 */ 
d431 1
a431 1
      
d435 1
a435 1
                + sizeof(TypGroup);
d464 2
a465 2
  
  InitHashTable (nNewGroups);     
d469 2
a470 2
  ShellSort (NewGroupTable, nNewGroups, sizeof (void far *), GroupCompare);
        
d472 1
a472 1
  
d505 2
a506 3
GroupCompare (g1, g2)
     TypLine const far *far * g1;
     TypLine const far *far * g2;
d513 1
a513 1
  return (_fstrcmp (gch1, gch2));
d523 1
a523 1
BOOL FAR PASCAL 
d527 2
a528 2
     WORD wParam;
     LONG lParam;
d533 2
a534 1
  char far *cptr,far *CurName;
d544 1
a544 1
  
d550 34
a583 34
   	  {
     	cptr = 0;
     	cptr = (NewGroupTable[j]);
     	cptr += sizeof (TypLine) + sizeof (TypGroup);
     	if((CurHier == NULL) ||
     	   (_fstrnicmp(CurName,cptr,_fstrcspn(cptr,"."))
     	    != 0)){
     	   HANDLE hTmpHandle;
     	   int NameLen;
     	   /* Need a new link */
     	   hTmpHandle = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,
     	   							sizeof(TypHier));
     	   if(CurHier){
     	   	CurHier->hNext = hTmpHandle;
     	   	GlobalUnlock(CurHier->hName);
     	   	GlobalUnlock(hCurHier);
     	   	hCurHier = hTmpHandle;
      	   	CurHier = (TypHier far *)GlobalLock(hCurHier);
      	   } else {
      	   	hCurHier = hStartHier = hTmpHandle;
      	   	CurHier = (TypHier far *)GlobalLock(hCurHier);
      	   }
      	   /* And now for the actual hierarchy name */
      	   NameLen = _fstrcspn(cptr,".");
      	   CurHier->hName = GlobalAlloc(GMEM_MOVEABLE,NameLen+1);
      	   CurName = GlobalLock(CurHier->hName);
      	   _fstrncpy(CurName,cptr,NameLen);
      	   CurName[NameLen] = '\0';
 		   /* And the start index */
 		   CurHier->Start = j;
      	}
      	/* Increment the number of groups _every_ time */
      	CurHier->NumGroups++;     	
    	  }         
d585 1
a585 1
      GlobalUnlock(hCurHier);	  
d590 1
a590 1
      hCurHier = hStartHier;      
d592 5
a596 5
      	HANDLE hTmpHandle;
      	CurHier = (TypHier far *)GlobalLock(hCurHier);
      	CurName = GlobalLock(CurHier->hName);
      	SendMessage (hDlgHierList, LB_ADDSTRING, 0, (LPARAM)CurName);
      	hTmpHandle = CurHier->hNext;
d598 3
a600 3
      	GlobalUnlock(hCurHier);
      	hCurHier = hTmpHandle;
      }      
d605 4
d610 4
a613 2
                   (LPARAM)MAKELPARAM(GetDlgItem(hDlg,IDD_HIERARCHY_LISTBOX),
                   					  LBN_SELCHANGE));          					  
d616 1
a616 1
  
d618 1
a618 1
      switch (wParam)
d621 2
a622 2
        EndDialog (hDlg, TRUE);
        break;
d624 2
a625 2
        EndDialog (hDlg, FALSE);
        break;
d627 3
d631 1
d635 34
a668 25
          nItem = SendMessage(LOWORD(lParam),LB_GETCURSEL, 0 , 0);
          SendMessage(LOWORD(lParam),LB_GETTEXT,(WPARAM)nItem,
                      (LPARAM) TmpBuf);
          SendMessage(LOWORD(lParam),LB_DELETESTRING,(WPARAM)nItem,
                      (LPARAM) 0);
 		  /* Now pull the stucture out of the hash table */
    	  hashval = HashGroup (TmpBuf);
   	      while (_fstrcmp(
    	   (char huge *)NetHashTable[hashval]+sizeof(TypLine)+sizeof(TypGroup),
    	   TmpBuf) != 0)
          {
             hashval = (hashval + 1) % HashTableSize;
          }
          group = (TypGroup huge *)
               ((char huge *)NetHashTable[hashval] + sizeof(TypLine));
          group->Subscribed = 0;              

          SendMessage(GetDlgItem(hDlg,IDD_UNSUBSCRIBED_GROUP_LISTBOX),
          			  LB_ADDSTRING, 0, (LPARAM)TmpBuf);
          break;
        default:
        	return FALSE;          
                       	      
        }
        break;
d670 3
d674 1
d678 30
a707 22
          nItem = SendMessage(LOWORD(lParam),LB_GETCURSEL, 0 , 0);
          SendMessage(LOWORD(lParam),LB_GETTEXT,(WPARAM)nItem,
                      (LPARAM) TmpBuf);
          SendMessage(LOWORD(lParam),LB_DELETESTRING,(WPARAM)nItem,
                      (LPARAM) 0);
 		  /* Now pull the stucture out of the hash table */
    	  hashval = HashGroup (TmpBuf);
    	  while (_fstrcmp((char huge *)NetHashTable[hashval]+
    	  				   sizeof(TypLine)+sizeof(TypGroup),
    	   				   TmpBuf) != 0)
          {
             hashval = (hashval + 1) % HashTableSize;
          }
          group = (TypGroup huge *)
               ((char huge *)NetHashTable[hashval] + sizeof(TypLine));
          group->Subscribed = 1;
                     
          SendMessage(GetDlgItem(hDlg,IDD_SUBSCRIBED_GROUP_LISTBOX),         
          			  LB_ADDSTRING,0, (LPARAM)TmpBuf);
          break;
        default:
        	return FALSE;          
d709 2
a710 2
        }
        break;
d712 3
d716 1
d720 14
a733 8
          nItem = SendMessage(LOWORD(lParam),LB_GETCURSEL, 0 , 0);
          SendMessage(LOWORD(lParam),LB_GETTEXT,(WPARAM)nItem,
                      (LPARAM) TmpBuf);
          hCurHier = hStartHier;      
      	  while(hCurHier){
      	    HANDLE hTmpHandle;
      	    CurHier = (TypHier far *)GlobalLock(hCurHier);
      		CurName = GlobalLock(CurHier->hName);
d735 1
a735 1
			if(_fstricmp(CurName,TmpBuf) == 0)
d737 1
a737 1
      	   	hTmpHandle = CurHier->hNext;
d739 2
a740 2
       	    hCurHier = hTmpHandle;
          }      
d742 3
d746 1
d755 27
a781 24
				    ((char huge *)NewGroupTable[j] + sizeof(TypLine));	
	        if (group->Subscribed)
              SendMessage (hDlgSubList, LB_ADDSTRING, 0, 
                           (LPARAM)((char huge*)group + sizeof(TypGroup)));
            else
              SendMessage (hDlgUnSubList, LB_ADDSTRING, 0, 
                           (LPARAM)((char huge *)group + sizeof(TypGroup)));
          }
               
        //  Horizontal scrolling doesn't work.   These are all attempts to
        //  fix it that don't work.  However, it isn't worth holding up
        //  the release of .92-3 for this one....JD 8/26/94
        
                        					  
        //  SendDlgItemMessage(GetDlgItem(hDlg,IDD_SUBSCRIBED_GROUP_LISTBOX), IDD_SUBSCRIBED_GROUP_LISTBOX,       // JD 8/25/94
        //                         LB_SETHORIZONTALEXTENT,(WPARAM)200,(LPARAM)0); 
        //  SetScrollRange(GetDlgItem(hDlg,IDD_SUBSCRIBED_GROUP_LISTBOX),
        //             SB_CTL, (int) 0, (int) 1200, TRUE);   // JD 8/26/94
        //                                                                  
        //  SetScrollRange(GetDlgItem(hDlg,IDD_UNSUBSCRIBED_GROUP_LISTBOX),
        //             SB_HORZ, (int) 0, (int) 20, TRUE);   // JD 8/26/94 
        
                       
    	  SendMessage (LOWORD(lParam), WM_SETREDRAW, TRUE, 0L);
d784 4
a787 4
        default:
          return FALSE;
        }
        break;
d789 1
a789 1
        return FALSE;
d792 1
a792 1
  
d797 7
a803 7
	  	HANDLE	hTmpHandle;
	  	CurHier = (TypHier far *)GlobalLock(hCurHier);
	  	GlobalFree(CurHier->hName);
	  	hTmpHandle = CurHier->hNext;
	  	GlobalUnlock(hCurHier);
	  	GlobalFree(hCurHier);
	  	hCurHier = hTmpHandle;
d805 1
a805 1
        break;
d909 2
a910 2
           */
     MoveBytes (AllocPtr, myline, ((TypLine far *) AllocPtr)->length); 
d941 2
a942 2
         netcptr = ((char far *) LinePtr) + sizeof (TypLine) + sizeof (TypGroup);
         if (lstrcmp (grpcptr, netcptr) < 0)
d944 1
a944 1
        advance = FALSE;
d946 1
a946 1
         else
d948 2
a949 2
        advance = NextLine (&BlockPtr, &LinePtr);
        if (!advance) at_end = 1; /* possible bug, getting bad netcptr (smr) */
d956 1
a956 1
     MoveBytes (AllocPtr, myline, ((TypLine far *) AllocPtr)->length); 
d999 1
a999 1
 
d1010 1
a1010 1
	int	IndexA = 0;
d1013 1
a1013 1
		
d1022 2
a1023 2
			return(prime[TmpIndex+1]);			
 		if( OrigVal < prime[TmpIndex])
d1028 1
a1028 1
	
@


1.13
log
@new group list box & new hash table mechanism
@
text
@d4 1
a4 1
 * $Id: wvlist.c 1.12 1994/07/26 18:39:05 dumoulin Exp $
d6 3
d599 1
a599 1
                   					  LBN_SELCHANGE));
d662 4
a665 3
          group->Subscribed = 1;       
          SendMessage(GetDlgItem(hDlg,IDD_SUBSCRIBED_GROUP_LISTBOX),
          			  LB_ADDSTRING, 0, (LPARAM)TmpBuf);
d709 16
a724 1
          }            
@


1.12
log
@Fixed WinVN bombing when number of new newsgroups exceeds
3000.  Now the new newsgroups dialog box doesn't get called.
@
text
@d4 1
a4 1
 * $Id: wvlist.c 1.11 1994/06/08 21:01:45 gardnerd Exp dumoulin $
d6 4
d56 4
d63 2
a64 4
/* #define HASHMAX 9721 */
/* #define MAXGROUPS  3200 */
#define HASHMAX 16379
#define MAXGROUPS 6000
d72 3
a74 2

HWND hDlgList;       /* Handle to child list box window. */
d97 1
d107 1
a107 1
  unsigned int hashval;
d116 2
a117 1
  /* Set up hash table for group names */
d119 4
a122 7
  hNetHashTable = GlobalAlloc (GMEM_MOVEABLE, (long) HASHMAX * sizeof (TypLine far *));
  NetHashTable = (TypLine far * far *) GlobalLock (hNetHashTable);
  for (hashval = 0; hashval < HASHMAX; hashval++)
    {
      NetHashTable[hashval] = (TypLine far *) 0;
    }

d128 18
a145 1
  /* Set up table of pointers to new group lines.
d147 6
a153 1
  InitGroupTable ();
d156 1
d159 2
a160 1
 *  Allocate the NewGroupTable.
d162 2
a163 2
 *  Exit:   hNewGroupTable    is the handle
 *          NewGroupTable     points to the table
d165 1
d170 7
a176 3
  hNewGroupTable = GlobalAlloc (GMEM_MOVEABLE, (long) MAXGROUPS * sizeof (TypLine far *));
  NewGroupTable = (void far * far *) GlobalLock (hNewGroupTable);

d203 1
a203 1

d210 1
a210 1
    }
d214 1
d235 1
a235 1
         hashval = (hashval + 1) % HASHMAX;
d286 7
a292 1
      if (nNewGroups < MAXGROUPS) {
d294 1
a294 10
   CrackGroupLine (ListLine, (TypLine *) (mygroupline));
   mygroup = (TypGroup *) (mygroupline + sizeof (TypLine));
   GetNum (&restline, &(mygroup->ServerLast));
   GetNum (&restline, &(mygroup->ServerFirst));
   mygroup->ServerEstNum = mygroup->ServerLast - mygroup->ServerFirst+ 1; 
   mygroup->HighestPrevSeen = 0;
   mygroup->nRanges = 0;

   AddGroupToTable (mygroupline);
      }
d301 1
a301 1
     hashval = (hashval + 1) % HASHMAX;
d333 5
a337 3
  HANDLE hLine;
  char far *AllocPtr;
  int mylength;
d339 11
a349 2
  /* Create a copy of this line in far memory and place pointers
   * to in our arrays.
d351 6
a357 8
  mylength = ((TypLine far *) GroupLine)->length + sizeof (HANDLE);
  hLine = GlobalAlloc (GMEM_MOVEABLE, (long) mylength);
  AllocPtr = (char far *) GlobalLock (hLine);
  MoveBytes ((char far *) &hLine, AllocPtr, sizeof (hLine));
  MoveBytes ((char far *) GroupLine, AllocPtr + sizeof (hLine), mylength - sizeof (hLine));

  NewGroupTable[nNewGroups] = AllocPtr;
  nNewGroups++;
d385 1
a385 1
  hash = (unsigned) (sum % HASHMAX);
d389 46
d448 10
d460 1
a460 13
  if (nNewGroups > MAXNEWGROUPSASK) 
   {   
      MergeGroups (ADD_SUBSCRIBED_END_OF_SUB);
      NetDoc.LongestLine = 0;
      SetGroupActiveLines();
 /*     MessageBox(hWndConf, "Number of New Groups at startup exceeded maximum " 
                        "allowed for individual selection.  All new groups "
                        "will be added as Unsubscribed groups",
                        "New Group Limit Exceeded", MB_OK);            */
    }
   else
   {
    lpfnWinVnGroupListDlg = MakeProcInstance (WinVnGroupListDlg, hInst);
d462 13
a474 13
    /* Display dialog box of new groups.  */
    if (nNewGroups && DialogBox (hInst, "WinVnGroupList", hWndConf, lpfnWinVnGroupListDlg))
    {
      /* The user has clicked OK, so add all the new groups to the
       * Net document.  Subscribed groups go at the end of the Subscribed
       * section at the top of the doc.
       * Unsubscribed groups go in the section below, in alphabetical order.
       */
      MergeGroups (ADD_SUBSCRIBED_END_OF_SUB);
      NetDoc.LongestLine = 0;
      SetGroupActiveLines();
    }
   }
d498 1
a498 1
  char far *gch1, far * gch2;
d500 2
a501 2
  gch1 = (char far *) *g1 + sizeof (HANDLE) + sizeof (TypLine) + sizeof (TypGroup);
  gch2 = (char far *) *g2 + sizeof (HANDLE) + sizeof (TypLine) + sizeof (TypGroup);
d503 1
a503 1
  return (lstrcmp (gch1, gch2));
d520 11
a530 6
  int j, selstate;
  char far *cptr;
  void far *AllocPtr;
  TypGroup far *group;


d533 67
a599 17

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

d602 63
a664 4
   {
   case IDOK:
     EndDialog (hDlg, TRUE);
     break;
d666 1
a666 2
   case IDCANCEL:
      	EndDialog (hDlg, FALSE);
d668 50
a717 6

   default:
     return FALSE;
   }
      break;

d719 12
a730 11
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
d829 1
a829 1
      AllocPtr = (char far *) NewGroupTable[j] + sizeof (HANDLE);
a838 3
     hLine = *((HANDLE far *) NewGroupTable[j]);
     MyGlobalUnlock (hLine);
     GlobalFree (hLine);
d859 1
a859 1
     AllocPtr = (char far *) NewGroupTable[j] + sizeof (HANDLE);
a884 4
     /* Unlock and free this entry in NewGroupTable. */
     hLine = *((HANDLE far *) NewGroupTable[j]);
     MyGlobalUnlock (hLine);
     GlobalFree (hLine);
d916 1
a916 1
  /* Unlock and free the NewGroupTable itself.  */
d918 2
d923 32
@


1.11
log
@more scrolling changes...
@
text
@d4 1
a4 1
 * $Id: wvlist.c 1.10 1994/05/27 01:29:29 rushing Exp $
d6 3
a359 1

d361 17
a377 6

  lpfnWinVnGroupListDlg = MakeProcInstance (WinVnGroupListDlg, hInst);

  /* Display dialog box of new groups.
   */
  if (nNewGroups && DialogBox (hInst, "WinVnGroupList", hWndConf, lpfnWinVnGroupListDlg))
a386 1

d388 1
a388 1

d443 8
a450 9
    case WM_INITDIALOG:
      hDlgList = GetDlgItem (hDlg, IDD_GROUP_LISTBOX);

      SendMessage (hDlgList, WM_SETREDRAW, FALSE, 0L);
      for (j = 0; j < nNewGroups; j++)
   {
     cptr = 0;
     cptr = (NewGroupTable[j]);
     cptr += sizeof (HANDLE) + sizeof (TypLine) + sizeof (TypGroup);
d453 5
a457 5
     SendMessage (hDlgList, LB_ADDSTRING, 0, (LONG) cptr);
   }
      SendMessage (hDlgList, WM_SETREDRAW, TRUE, 0L);
      return TRUE;
      break;
d467 3
a469 2
     EndDialog (hDlg, FALSE);
     break;
@


1.10
log
@unnecessary winundoc.h
@
text
@d4 1
a4 1
 * $Id: wvlist.c 1.9 1994/05/19 02:02:10 rushing Exp rushing $
d6 3
d372 1
@


1.9
log
@changes for gensock & version 0.91
@
text
@d4 1
a4 1
 * $Id: wvlist.c 1.8 1994/01/12 19:25:59 mrr Exp rushing $
d6 3
a45 1
#include "winundoc.h"
@


1.8
log
@mrr mods 4
@
text
@d4 1
a4 1
 * $Id: wvlist.c 1.7 1993/12/08 01:28:38 rushing Exp $
d6 3
d95 1
a95 1
  PutCommLine ("LIST\r", 4);
@


1.7
log
@new version box and cr lf consistency
@
text
@d4 1
a4 1
 * $Id: wvlist.c 1.6 1993/10/12 17:47:26 rushing Exp rushing $
d6 3
d53 1
a53 1
HANDLE far *lphNewGroupLines;	/* array of handles to new group lines */
d56 1
a56 1
HWND hDlgList;			/* Handle to child list box window. */
d183 9
a191 9
	{
	  textptr = ((unsigned char far *) LinePtr) + TextOffset;
	  hashval = HashGroup (textptr);
	  while (HashTable[hashval])
	    {
	      hashval = (hashval + 1) % HASHMAX;
	    }
	  HashTable[hashval] = LinePtr;
	}
d221 3
a223 3
	{
	  UpdateWindow (hWndConf);
	}
d229 1
a229 1
  restline = cptr + 1;		/* points to highest art # */
d241 7
a247 7
	CrackGroupLine (ListLine, (TypLine *) (mygroupline));
	mygroup = (TypGroup *) (mygroupline + sizeof (TypLine));
	GetNum (&restline, &(mygroup->ServerLast));
	GetNum (&restline, &(mygroup->ServerFirst));
	mygroup->ServerEstNum = mygroup->ServerLast - mygroup->ServerFirst+ 1; 
	mygroup->HighestPrevSeen = 0;
	mygroup->nRanges = 0;
d249 1
a249 1
	AddGroupToTable (mygroupline);
d256 4
a259 4
	{
	  hashval = (hashval + 1) % HASHMAX;
	  goto checkhash;
	}
d261 2
a262 2
	{
	  /* This group is already present in NetDoc.
d265 7
a271 7
	  netgroup = (TypGroup far *) ((char far *) NetHashTable[hashval] + sizeof (TypLine));
	  GetNum (&restline, &ArtNum);
	  netgroup->ServerLast = ArtNum;
	  GetNum (&restline, &ArtNum);
	  netgroup->ServerFirst = ArtNum;
	  netgroup->ServerEstNum = (netgroup->ServerLast - netgroup->ServerFirst + 1);
	}
d364 2
d427 4
a430 4
	{
	  cptr = 0;
	  cptr = (NewGroupTable[j]);
	  cptr += sizeof (HANDLE) + sizeof (TypLine) + sizeof (TypGroup);
d432 3
a434 3
/* 	  SendMessage (hDlgList, LB_ADDSTRING, -1, (LONG) cptr); */
 	  SendMessage (hDlgList, LB_ADDSTRING, 0, (LONG) cptr);
	}
d441 11
a451 11
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
d459 6
a464 6
	{
	  selstate = (WORD) SendMessage (hDlgList, LB_GETSEL, j, 0L);
	  AllocPtr = (char far *) NewGroupTable[j] + sizeof (HANDLE);
	  group = (TypGroup far *) ((char far *) AllocPtr + sizeof (TypLine));
	  group->Subscribed = selstate;
	}
d498 3
a500 3
	{
	  advance = NextLine (BlockPtr, LinePtr);
	}
d502 3
a504 3
	{
	  advance = FALSE;
	}
d567 2
a568 2
	{
	  /* This group has been selected and should be subscribed to.
d570 9
a578 8
	  MoveBytes (AllocPtr, myline, ((TypLine far *) AllocPtr)->length);
	  AddLine ((TypLine *) myline, &BlockPtr, &LinePtr);
	  NetDoc.ActiveLines++;
	  hLine = *((HANDLE far *) NewGroupTable[j]);
	  MyGlobalUnlock (hLine);
	  GlobalFree (hLine);
	  NewGroupTable[j] = (void far *) 0;
	}
d594 2
a595 2
	{
	  /* Search for the right place to add this line. */
d597 31
a627 30
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
	  AddLine ((TypLine *) myline, &BlockPtr, &LinePtr);

	  /* Unlock and free this entry in NewGroupTable. */
	  hLine = *((HANDLE far *) NewGroupTable[j]);
	  MyGlobalUnlock (hLine);
	  GlobalFree (hLine);
	}
@


1.6
log
@make winvn grok servers that start article numbers at 0
@
text
@d1 1
d4 1
a4 1
 * $Id: wvlist.c 1.5 1993/08/18 21:51:13 rushing Exp rushing $
d6 3
d89 1
a89 1
  PutCommLine ("LIST", 4);
@


1.5
log
@more 16-bit article number fixes
@
text
@d3 1
a3 1
 * $Id: wvlist.c 1.4 1993/06/28 17:52:17 rushing Exp UNKNOWN $
d5 3
d238 1
@


1.4
log
@fixed compiler warnings
@
text
@d3 1
a3 1
 * $Id: wvlist.c 1.3 1993/06/24 17:07:05 dumoulin Exp rushing $
d5 3
d259 1
a259 1
	  netgroup->ServerEstNum = (unsigned int) (netgroup->ServerLast - netgroup->ServerFirst + 1);
@


1.3
log
@avoid max new groups > MAXGROUPS crash.
@
text
@d3 1
a3 1
 * $Id: wvlist.c 1.2 1993/06/22 16:16:13 rushing Exp dumoulin $
d5 3
d256 1
a256 1
	  netgroup->ServerEstNum = netgroup->ServerLast - netgroup->ServerFirst + 1;
d318 1
a318 1
  hash = sum % HASHMAX;
@


1.2
log
@raised MAXGROUPS (really MAXNEWGROUPS)
@
text
@d3 1
a3 1
 * $Id: wvlist.c 1.1 1993/02/16 20:54:22 rushing Exp rushing $
d5 3
d31 1
a31 1
#define MAXGROUPS 8000
d222 9
a230 6
      CrackGroupLine (ListLine, (TypLine *) (mygroupline));
      mygroup = (TypGroup *) (mygroupline + sizeof (TypLine));
      GetNum (&restline, &(mygroup->ServerLast));
      GetNum (&restline, &(mygroup->ServerFirst));
      mygroup->HighestPrevSeen = 0;
      mygroup->nRanges = 0;
d232 2
a233 1
      AddGroupToTable (mygroupline);
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
d25 4
a28 3

#define HASHMAX 9721
#define MAXGROUPS  3200
@
