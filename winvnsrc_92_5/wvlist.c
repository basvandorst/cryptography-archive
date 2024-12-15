
/*
 *
 * $Id: wvlist.c 1.15 1994/09/02 23:49:10 martin Exp $
 * $Log: wvlist.c $
 * Revision 1.15  1994/09/02  23:49:10  martin
 * fixes for new hashtable stuff
 *
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
