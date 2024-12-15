/********************************************************************
 *                                                                  *
 *  MODULE    :  WVGROUP.C                                          *
 *                                                                  *
 *  PURPOSE   : This file contains the window procedure for the     *
 *              Group viewing window for WinVN.                     *
 *                                                                  *
 *  FUNCTIONS :                                                     *
 *              WinVnViewWndProc()    -  Window Procedure for any   *
 *                                       of WinVN's group windows   *
 *                                                                  *
 *              FileLength()          -  Find the size in bytes of  *
 *                                       a file                     *
 *                                                                  *
 *              ViewArticle()         -  Requests and views an      *
 *                                       article from the server    * 
 *                                                                  *
 *              setArticleFocus()     -  Makes an article active    *
 *                                                                  *
 *              UnlinkArtsInGroup()   -  Prepares article window    *
 *                                       for deletion               *
 *                                                                  *
 *              UpdateSeenArts()      -  Save seen info in document *
 *                                                                  *
 *              CursorToTextLine()    -  Locates text in a document *
 *                                       based on cursor position   *
 *                                                                  *
 *              search_headers()      -  Search all headers for a   *
 *                                       given substring            *
 *                                                                  *
 *    string_compare_insensitive()    -  Compare two strings while  *
 *                                       ignoring case sensitivity  *
 *                                                                  *
 *               AffectSelected()     -  Set selected flag based on *
 *                                       search criteria            *
 *                                                                  *
 ********************************************************************/


/*
 * $Id: wvgroup.c 1.49 1994/09/18 22:49:02 jcooper Exp $
 *
 */

#include <windows.h>
#include <windowsx.h>      // for GlobalFreePtr (JSC)
#include "wvglob.h"
#include "winvn.h"
#pragma hdrstop

void setArticleFocus(HWND hWndArt);
long NewCursorToTextLine (int X, int Y, TypDoc * DocPtr);
long search_headers (TypDoc * HeaderDoc, header_p headers, long artindex, long num_headers);
void SaveSelectedArts (HWND hwnd, TypDoc *Doc, header_p headers, long num_headers);
long AffectSelected (TypDoc *Doc, BOOL value, BOOL compare);
char_p string_compare_insensitive (char_p a, LPSTR b);
void InitiateReceiveArticle (TypDoc *Doc, char far *articleId);
BOOL article_operation (TypDoc * Doc,
			long artindex,
			BOOL (*art_fun) (header_p headers,
					 TypGroup * group,
					 long artindex));

BOOL toggle_read_unread (header_p headers, TypGroup * group, long artindex);
BOOL toggle_selected (header_p headers, TypGroup * group, long artindex);
BOOL selected_true (header_p headers, TypGroup * group, long artindex);
BOOL selected_false (header_p headers, TypGroup * group, long artindex);
BOOL seen_true (header_p headers, TypGroup * group, long artindex);
BOOL seen_false (header_p headers, TypGroup * group, long artindex);
BOOL mark_read_to_here (header_p headers, TypGroup * group, long artindex);
BOOL mark_read_all (header_p headers, TypGroup * group, long artindex);
void CloseGroupWnd(HWND hWnd, TypDoc *ThisDoc);
void SetGroupMenus (HWND hWnd, int enable);
void SetMenusForMultiArticleOperation (HWND hWnd, int enable);

/*--- FUNCTION: WinVnViewWndProc --------------------------------------------
 *
 *    Window procedure for a Group window, which contains the subjects
 *    of the various articles in a newsgroup.
 *    Note that there may be several different Group windows active;
 *    this routine gets called any time anything happens to any of them.
 */

long FAR PASCAL 
WinVnViewWndProc (hWnd, message, wParam, lParam)
     HWND hWnd;
     UINT message;
     WPARAM wParam;
     LPARAM lParam;
{

  PAINTSTRUCT ps;               /* paint structure          */

  HDC hDC;                      /* handle to display context */
  RECT clientRect;              /* selection rectangle      */
  TypDoc *ThisDoc;
  int ih, j;
  long artindex;
  BOOL found;
  long found_artindex;
  int CtrlState;
  TypBlock far *BlockPtr;
  HANDLE hBlock;
  unsigned int Offset;
  BOOL continueFind;
  char mybuf[MAXINTERNALLINE];
  TypLineID MyLineID;
  POINT ptCursor;
  TypLine far * LinePtr;
  int X, Y;
  int OldSel = FALSE;

  /* We know what *window* is being acted on, but we must find
   * out which *document* is being acted on.  There's a one-to-one
   * relationship between the two, and we find out which document
   * corresponds to this window by scanning the GroupDocs array.
   */

  for (ih = 0, found = FALSE; !found && ih < MAXGROUPWNDS; ih++)
    {
      if (GroupDocs[ih].hDocWnd == hWnd)
	{
	  found = TRUE;
	  ThisDoc = &(GroupDocs[ih]);
	}
    }

  if (!found)
    {
      ThisDoc = CommDoc;
    }

  switch (message)
    {
    case WM_CREATE:
      SetGroupMenus(hWnd, DISABLE);
      NumGroupWnds++;
      break;

    case WM_ACTIVATE:
      if (wParam)
	{
	  ActiveGroupDoc = ThisDoc;
	}
      /* fall through */
    case WM_SYSCOMMAND:
      return (DefWindowProc (hWnd, message, wParam, lParam));

/*   case WM_INITMENUPOPUP:				// (JSC) now handled by SetGroupMenus
      EnableMenuItem(GetMenu(hWnd),IDM_MAIL,MailCtrl.enableMail) ;
      break;
*/

    case WM_SIZE:
      GetClientRect (hWnd, &clientRect);
      ThisDoc->ScXWidth = clientRect.right;
      ThisDoc->ScYHeight = clientRect.bottom;
      ThisDoc->ScYLines = (clientRect.bottom - clientRect.top - TopSpace) / LineHeight;
      ThisDoc->ScXChars = (clientRect.right - clientRect.left - SideSpace) / CharWidth;
      break;

    case WM_CLOSE:
      CloseGroupWnd(hWnd, ThisDoc);
      break;

    case WM_DESTROY:
      /* Unlink all the article windows that belong to this group */
      NumGroupWnds--;
      
      LockLine (ThisDoc->hParentBlock, ThisDoc->ParentOffset,
		ThisDoc->ParentLineID, &BlockPtr, &LinePtr);

      {
	/* Clear the pointer in the line for this group in the   */
	/* NetDoc document.  This pointer currently points       */
	/* to the current document, which we are wiping out      */
	/* with the destruction of this window.                  */
	TypGroup far *group =
	  (TypGroup far *) (((char far *) LinePtr) + sizeof (TypLine));
	long int CN_Total_Lines;

	group->SubjDoc = (TypDoc *) NULL;
	
	  ThisDoc->InUse = FALSE;
	  if (ThisDoc == CommDoc)
	    {
	      CN_Total_Lines = CommDoc->TotalLines;
	      CommBusy = FALSE;
	      CommDoc = (TypDoc *) NULL;
	    }
          /* SMR 940420 cause of mysterious F3 bug */
      	  else {
              CN_Total_Lines = group->total_headers;
      	  }

/*CN	if (group->total_headers != 0) { */
	if (CN_Total_Lines != 0) {	/* CN */

	  UpdateSeenArts (ThisDoc);
	  UnlinkArtsInGroup (ThisDoc);
	  if (group->header_handle)
	    free_headers (group->header_handle, group->thread_handle);
	
	  group->header_handle = (HANDLE) NULL;
	  group->thread_handle = (HANDLE) NULL; 
	  group->total_headers = 0;
	}
      }

      UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
      /* Clear document                                        */
      FreeDoc (ThisDoc);

      /* If there's another group window, make it the active   */
      /* group window so we don't create a new one if the      */
      /* New Group flag is FALSE.                              */

      for (j = MAXGROUPWNDS - 1; j >= 0; j--)
	{
	  if (GroupDocs[j].InUse)
	    {
	      ActiveGroupDoc = &(GroupDocs[j]);
	      break;
	    }
	}
      break;

    case WM_KEYDOWN:
      /* See if this key should be mapped to a scrolling event
       * for which we have programmed the mouse.  If so,
       * construct the appropriate mouse call and call the mouse code.
       */

      if (wParam == VK_F6)
	{
	  NextWindow (ThisDoc);
	}
      else
	{
	  CtrlState = GetKeyState (VK_CONTROL) < 0;
	  for (j = 0; j < NUMKEYS; j++)
	    {
	      if (wParam == key2scroll[j].wVirtKey &&
		  CtrlState == key2scroll[j].CtlState)
		{
		  SendMessage (hWnd, key2scroll[j].iMessage,
			       key2scroll[j].wRequest, 0L);
		  break;
		}
	    }
	}

      break;


    case WM_CHAR:
      /* Carriage Return means the same as double-clicking
       * on where the cursor is currently pointing.
       */
      if (wParam == '\r')
	{
	  GetCursorPos (&ptCursor);
	  ScreenToClient (hWnd, &ptCursor);
	  X = ptCursor.x;
	  Y = ptCursor.y;
	  goto getarticle;
	}
      else
	{
	}
      break;

    case WM_RBUTTONDBLCLK:
      /* double right click will mark all up to here as read */

      X = LOWORD (lParam);
      Y = HIWORD (lParam);

      artindex = NewCursorToTextLine (X, Y, ThisDoc);
	if (artindex > -1 )
	{
	  article_operation (ThisDoc, artindex, mark_read_to_here);
	  InvalidateRect(ThisDoc->hDocWnd,NULL,FALSE);    
	}

      break;

    case WM_MBUTTONDOWN:
      /* Single middle click will toggle the read/unread status of the */
      /* article */ 

      DragMouseAction = DRAG_NONE;
      if (!Initializing)
	{
          BOOL status;
          X = LOWORD (lParam);
          Y = HIWORD (lParam);

          artindex = NewCursorToTextLine (X, Y, ThisDoc);
	  if (artindex > -1 )
	   {
	    status = article_operation (ThisDoc, artindex, toggle_read_unread);
            DragMouseAction = status ? SEEN_SELECT : SEEN_DESELECT;
	    InvalidateRect(ThisDoc->hDocWnd,NULL,FALSE);	  
            SetCapture(hWnd);	// release capture on button up
	   }
        }
      break;

    case WM_LBUTTONDOWN:
      /*  Clicking the left button on an article name toggles the
       *  selected/not selected status of that group.
       *  Currently selected groups are displayed in reverse video.
       */

      DragMouseAction = DRAG_NONE;
      if (!Initializing)
	{
          BOOL status;
	  X = LOWORD (lParam);
	  Y = HIWORD (lParam);

	  artindex = NewCursorToTextLine (X, Y, ThisDoc);
	  if (artindex > -1 )
	    {
             if (MK_SHIFT & wParam) 
              {
               status = article_operation(ThisDoc,artindex,toggle_read_unread);
               DragMouseAction = status ? SEEN_SELECT : SEEN_DESELECT;
	      }
             else
	       {
                status = article_operation(ThisDoc, artindex,toggle_selected);
                DragMouseAction = status ? DRAG_SELECT : DRAG_DESELECT;
	       }
             SetCapture(hWnd);	// release capture on button up
	     InvalidateRect(ThisDoc->hDocWnd,NULL,FALSE);
	    }
	}
      break;

    case WM_LBUTTONUP:
      /*  Letting up on the left button on an article name 
       *  gets us out of Dragging mode   */

      ReleaseCapture();
      DragMouseAction = DRAG_NONE;
      break;

    case WM_MBUTTONUP:
      /*  Letting up on the middle button on an article name
       *  gets us out of Dragging mode   */

      ReleaseCapture();
      DragMouseAction = DRAG_NONE;
      break;

    case WM_LBUTTONDBLCLK:
      /*  Double-clicking on an article subject creates an "Article"
       *  window, whose purpose is to display the article.
       */
      X = LOWORD (lParam);
      Y = HIWORD (lParam);
    getarticle:;

      artindex = NewCursorToTextLine (X, Y, ThisDoc);
      if (artindex > -1 )
	{
	  article_operation (ThisDoc, artindex, selected_false);
	  ViewArticle (ThisDoc, artindex, NO_REUSE, SHOW, NO_ID);
	}

      break;

    case WM_MOUSEMOVE:
      /*  Code to drag the mouse and change the select/not selected
       *  status of that group.
       */

      if ((!Initializing) && (DragMouseAction != DRAG_NONE))
	{
	  X = LOWORD (lParam);
	  Y = HIWORD (lParam);

	  artindex = NewCursorToTextLine (X, Y, ThisDoc);
	  if (artindex > -1 )
	    {
                switch (DragMouseAction)
                  {
                    case DRAG_SELECT:
 		       article_operation (ThisDoc, artindex, selected_true);
		       InvalidateRect(ThisDoc->hDocWnd,NULL,FALSE);
                       break;

                    case DRAG_DESELECT:
      		       article_operation (ThisDoc, artindex, selected_false);
		       InvalidateRect(ThisDoc->hDocWnd,NULL,FALSE);
                       break;

                    case SEEN_SELECT:
      		       article_operation (ThisDoc, artindex, seen_true);
		       InvalidateRect(ThisDoc->hDocWnd,NULL,FALSE);
                       break;

                    case SEEN_DESELECT:
      		       article_operation (ThisDoc, artindex, seen_false);
		       InvalidateRect(ThisDoc->hDocWnd,NULL,FALSE);
                       break;
                  }

	    }
	}

      break;

    case WM_VSCROLL:
      NewScrollIt (ThisDoc, wParam, lParam);
      break;

    case WM_HSCROLL:
      HScrollIt (ThisDoc, wParam, lParam);
      break;

    case WM_PAINT:
       {
	HANDLE hBlock;
	SIZE sz;
	int MyLen;
	unsigned int Offset;
	int VertLines, HorzChars;
	int EndofDoc = FALSE;
	int RangeHigh, CurPos;
	int indicatorwidth, Xtext;
	char indicator;
	unsigned int artindex;
	long int artnum;
	TypBlock far *NetBlockPtr;
	TypLine far *NetLinePtr;
	TypGroup far *group;
	header_p headers;
	header_p header;
	HANDLE header_handle;
	HANDLE thread_handle;   
	char scratch_line [MAXINTERNALLINE];
	char date_string [80];
	unsigned long OldHighestSeen;
	COLORREF MyColors[4], MyBack[4];
	RECT aRect;
	int MyColorMask = 0, PrevColorMask = MyColorMask;  
	
	/* MyColors and MyBack are arrays of colors used to display text
	 * foreground and background.
	 * The ColorMask variables are indices into these arrays.
	 * We set and clear bits in these indices depending upon
	 * whether the article has been selected or seen.
	 */

#define SEEN_MASK 1
#define SELECT_MASK 2
	hDC = BeginPaint (hWnd, &ps);
 
	GetClientRect (hWnd, &clientRect);
	SelectObject (hDC, hListFont);

	VertLines = ( (ThisDoc->ScYLines > (ThisDoc->TotalLines - ThisDoc->TopLineOrd))
		     ? (ThisDoc->TotalLines - ThisDoc->TopLineOrd )
		     : ThisDoc->ScYLines);

	HorzChars = ThisDoc->ScXChars;

	MyColors[0] = ArticleUnSeenColor;	// unseen/unselected
	MyColors[1] = ArticleSeenColor;		// seen/unselected
	MyColors[2] = ListBackgroundColor;	// unseen/selected
	MyColors[3] = MyColors[1];		// seen/selected

	MyBack[0] = MyColors[2];
	MyBack[1] = MyColors[2];
        if (ListBackgroundColor == RGB(0,0,0))
           MyBack[2] = RGB(200,200,200);	// selected = soft/white background
        else
           MyBack[2] = RGB(0,0,0);		// selected = black background
	MyBack[3] = MyBack[2];

        SetTextColor (hDC, MyColors[MyColorMask]);
        SetBkColor (hDC, MyBack[MyColorMask]);

	/* Update the scroll bar thumb position.                 */

	CurPos = ThisDoc->TopLineOrd;
	if (CurPos < 0)
	  CurPos = 0;
	RangeHigh = ThisDoc->TotalLines - VertLines;
	if (RangeHigh < 0)
	  RangeHigh = 0;
	SetScrollRange (hWnd, SB_VERT, 0, RangeHigh, FALSE);
	SetScrollPos (hWnd, SB_VERT, CurPos, TRUE);
	                   
    RangeHigh = ThisDoc->LongestLine - ThisDoc->ScXChars; 
    if (RangeHigh < 0) 
    {
      RangeHigh = 0;
      ThisDoc->ScXOffset = 0;
    }
    SetScrollRange (hWnd, SB_HORZ, 0, RangeHigh, FALSE);
	SetScrollPos (hWnd, SB_HORZ, ThisDoc->ScXOffset, TRUE); 
	
	GetTextExtentPoint(hDC, "s 99999 ", 8, &sz);                       
	indicatorwidth = sz.cx;                                            
	LockLine (ThisDoc->hParentBlock, ThisDoc->ParentOffset, ThisDoc->ParentLineID,
		  &NetBlockPtr, &NetLinePtr);
	group = (TypGroup far *) ((char far *) NetLinePtr + sizeof (TypLine));

	header_handle = group->header_handle;
	thread_handle = group->thread_handle;

	if (header_handle)
	  headers = lock_headers (header_handle,thread_handle);

	OldHighestSeen = group->HighestPrevSeen;
	UnlockLine (NetBlockPtr, NetLinePtr, &hBlock, &Offset, &MyLineID);

	/* Now paint this stuff on the screen.               */

	X = SideSpace - ThisDoc->ScXOffset * (CharWidth+2);
	Xtext = X + indicatorwidth;
	Y = StartPen;

	artindex = ThisDoc->TopLineOrd;

	if (ThisDoc->TotalLines)
	do
	    {
	      header = header_elt (headers, artindex);

	      artnum = header->number;

		indicator = ' ';
		if(header->Seen) {
			indicator = 's';
		} else if(OldHighestSeen) {
			if(header->number > OldHighestSeen) {
				indicator = 'n';
			}
		}

	       if(ThisDoc->FindOffset == artindex)
		 indicator = '>';

	      sprintf (scratch_line, "%c%5Flu %-5.5Fs %-18.18Fs %4Fd %-*s%-Fs ",
		       indicator,
		       header->number,
		       StringDate(date_string,header->date),
		       header->from,
		       header->lines,
		       header->thread_depth * 2,
		       "",
		       /* ack! */
		       (ThreadFullSubject || !(header->thread_depth))
		       ? header->subject
		       : "\020"
		       );

	      MyLen = lstrlen (scratch_line);

	      /* Figure out the color of this line.                 */

	      MyColorMask = 0;
	      if (header->Seen)
		{
		  MyColorMask |= SEEN_MASK;
		}
	      else
		{
		  MyColorMask &= (0xff - SEEN_MASK);
		}
	      if (header->Selected)
		{
		  MyColorMask |= SELECT_MASK;
		}
	      else
		{
		  MyColorMask &= 0xff - SELECT_MASK;
		}
	      if (MyColorMask != PrevColorMask)
		{
		  SetTextColor (hDC, MyColors[MyColorMask]);
		  SetBkColor (hDC, MyBack[MyColorMask]);
		  PrevColorMask = MyColorMask;
		}

	      /* Now write out the line.                            */

	      SetRect (&aRect, 0, Y, clientRect.right, Y+LineHeight);
	      
	      ExtTextOut (hDC, X, Y, ETO_OPAQUE|ETO_CLIPPED, &aRect,
	                  scratch_line, MyLen, (LPINT)NULL);
	        
	      Y += LineHeight;
	      artindex++;

	    }
	  while (--VertLines > 0 );

	if (header_handle)
	  unlock_headers (header_handle, thread_handle);

	/* We've reached the end of the data to be displayed     */
	/* on this window.  If there's more screen real estate   */
	/* left, just blank it out.                              */

        SelectObject (hDC, hListBackgroundBrush);
	PatBlt (hDC, 0, Y, clientRect.right - 1, clientRect.bottom - Y, PATCOPY);

	EndPaint (hWnd, &ps);
	break;
      }

    case IDM_RETRIEVE_COMPLETE:
      SetGroupMenus(hWnd, ENABLE);
      break;

    case WM_COMMAND:
      switch (LOWORD(wParam))
	{
	case IDV_EXIT:
	  CloseGroupWnd(hWnd, ThisDoc);
	  break;

	case IDV_NEXT:
	  break;

	case IDM_MARK_ALL:
	  article_operation (ThisDoc, 0, mark_read_all);
	  CloseGroupWnd(hWnd, ThisDoc);
	  break;
	
	case IDM_SORT_DATE:
	case IDM_SORT_SUBJECT:
	case IDM_SORT_LINES:
	case IDM_SORT_THREADS:
	case IDM_SORT_ARTNUM:
	case IDM_SORT_FROM:
	  
	  { TypGroup far * group;
	    HANDLE header_handle,thread_handle;
	    header_p headers;
	    thread_array thread_index;
	    long i;

	    LockLine (ThisDoc->hParentBlock, ThisDoc->ParentOffset,
		      ThisDoc->ParentLineID, &BlockPtr, &LinePtr);
	    group = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));

	    header_handle = group->header_handle;
	    thread_handle = group->thread_handle;

	    /* this dependency on [thread_array_p][head0][head1][head2]... */
	    /* should be moved into headarry.c */

	    headers = lock_headers (header_handle,thread_handle);
	    thread_index = * ((thread_array_p) ((char_p) headers - sizeof (char_p)));

	    /* clear thread_depth info */
	    for (i=0;i<group->total_headers;i++)
	      headers[i].thread_depth = 0;

	    UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
	  
	    switch (LOWORD(wParam)) {
	    case IDM_SORT_DATE:
	      shell_sort_index_array (headers, thread_index,
				      ThisDoc->TotalLines, compare_date);
	      break;

	    case IDM_SORT_SUBJECT:
	      shell_sort_index_array (headers, thread_index,
				      ThisDoc->TotalLines, compare_subject);
	      break;

	    case IDM_SORT_LINES:
	      shell_sort_index_array (headers, thread_index,
				      ThisDoc->TotalLines, compare_lines);
	      break;

	    case IDM_SORT_THREADS:
	      if (threadp)
		sort_by_threads (header_handle, thread_handle, ThisDoc->TotalLines);
	      else
		MessageBox (hWnd, "Threading disabled", "WinVN", MB_OK);

	      break;

	    case IDM_SORT_ARTNUM:
	      shell_sort_index_array (headers, thread_index,
				      ThisDoc->TotalLines, compare_artnum);
	      break;

	    case IDM_SORT_FROM:
	      shell_sort_index_array (headers, thread_index,
				      ThisDoc->TotalLines, compare_from);
	      break;
	    }
	    unlock_headers (header_handle, thread_handle);
	    InvalidateRect(ThisDoc->hDocWnd,NULL,FALSE);
	  }

	  break;

	case IDM_FIND:
	case IDM_FIND_NEXT_SAME:
	  FindDoc = ThisDoc;
      continueFind = TRUE;
	  if (!FindDoc->SearchStr[0] || LOWORD(wParam) == IDM_FIND) {
	    if (!(FindDoc->SearchStr[0]) && LastArticleHeaderFind[0]) 
	      strcpy(FindDoc->SearchStr, LastArticleHeaderFind);
	    continueFind = DialogBox (hInst, "WinVnFind", hWnd, lpfnWinVnFindDlg);
	  }
	  if (continueFind && FindDoc->SearchStr[0]) {
	    TypGroup far * group;
	    header_p headers;
	    HANDLE header_handle;
	    HANDLE thread_handle;           
	    int starting_at;

	    LockLine (ThisDoc->hParentBlock, ThisDoc->ParentOffset, ThisDoc->ParentLineID,
		      &BlockPtr, &LinePtr);
	    group = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));

	    header_handle = group->header_handle;
	    thread_handle = group->thread_handle;

	    headers = lock_headers (header_handle,thread_handle);

	    UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);

	    /* 'Find Next' will start one line after */
	    starting_at = ThisDoc->TopLineOrd + ((LOWORD(wParam) == IDM_FIND) ? 0 : 1);

	    /* back up one if we're at the end */
	    if (starting_at >= group->total_headers)
	      starting_at--;

	    found_artindex = search_headers (ThisDoc, headers, starting_at, group->total_headers);
	    if (found_artindex == -1)
	      {
		strcpy (mybuf, "\"");
		strcat (mybuf, ThisDoc->SearchStr);
		strcat (mybuf, "\" not found.");
		MessageBox (hWnd, mybuf, "Not found", MB_OK);
	      }
	    else {
	      ThisDoc->TopLineOrd = (int) found_artindex;
	      ThisDoc->FindOffset = (int) found_artindex;
  	      strcpy(LastArticleHeaderFind, FindDoc->SearchStr);
	      InvalidateRect(ThisDoc->hDocWnd,NULL,FALSE);
	    }

	    unlock_headers (header_handle,thread_handle);
	  }
	  break;

	case IDM_POST:
	  /* We are creating the skeleton text of a new posting.
	   * Most of the work is done by CreatePostingWnd and
	   * CreatePostingText.  Here we have to identify
	   * the newsgroup for those routines.
	   * Get the newsgroup from the line in NetDoc that
	   * points to this document.
	   */
	  LockLine (ThisDoc->hParentBlock, ThisDoc->ParentOffset,
		    ThisDoc->ParentLineID, &BlockPtr, &LinePtr);
	  ExtractTextLine (ThisDoc->ParentDoc, LinePtr,
			   mybuf, MAXINTERNALLINE);
	  UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
	  NewsgroupsPtr = mybuf;
	  CreatePostingWnd (hWnd, (TypDoc *) NULL, DOCTYPE_POSTING);
	  break;

	case IDM_SAVE_SELECTED:
	  /* Query user for file name */
	  if (!DialogBox (hInst, "WinVnSaveArts", hWnd, lpfnWinVnSaveArtsDlg)) {
	     InvalidateRect (hWnd, NULL, TRUE);
	  } else {
	     numArtsSaved = 0;
	     savingArtIndex = 0;
	     SetMenusForMultiArticleOperation (hWnd, DISABLE);

	     goto doretrieve;
	 }
	 break;

	case IDM_DECODE_SELECTED:
	  /* Query user for file name */
	  if (!DialogBoxParam (hInst, "WinVnDecodeArts", hWnd, lpfnWinVnDecodeArtsDlg, 1)) {
	     InvalidateRect (hWnd, NULL, TRUE);
	  } else {
	     if (TestCommBusy(hWnd, "Can't decode selected articles"))
	       break;
	     if (TestDecodeBusy(hWnd, "Can't decode selected articles"))
	       break;
	       
	     DecodeInit();
	     CommDoc = ThisDoc;
	     CommDecoding = TRUE;
	     numArtsSaved = 0;
	     savingArtIndex = 0;
	     SetMenusForMultiArticleOperation (hWnd, DISABLE);

	     goto doretrieve;
	 }
	 break;

	case IDM_SELECT_ALL:
	  /* select all articles */   
	  AffectSelected (ThisDoc, SELECT, NO_COMPARE);
	break;

	case IDM_DESELECT_ALL:
	  /* deselect all articles */   
	   AffectSelected (ThisDoc, DESELECT, NO_COMPARE);
	break;

	case IDM_SELECT_MATCH:
	  /* select all articles containing a string*/
	  FindDoc = ThisDoc;   
	  DialogBox (hInst, "WinVnFind", hWnd, lpfnWinVnFindDlg);
	  if(strcmp(FindDoc->SearchStr,"")) {
	    if (AffectSelected (FindDoc, SELECT, COMPARE)==0) {
		strcpy (mybuf, "\"");
		strcat (mybuf, FindDoc->SearchStr);
		strcat (mybuf, "\" not found.");
		MessageBox (hWnd, mybuf, "Not found", MB_OK);
	    }
	  }
	  break;

	case IDM_MAIL:
	  (MailCtrl.fnMlWinCreate)(hWnd, (TypDoc *) NULL, DOCTYPE_MAIL);
	  break;

	case IDM_UPDATE:
	  if (!CommBusy) {
	    TypGroup far * group;
	    char far * group_name;

	    /* update the newsrc data */
	    UpdateSeenArts (ThisDoc);
	    LockLine (ThisDoc->hParentBlock, ThisDoc->ParentOffset,
		      ThisDoc->ParentLineID,&BlockPtr, &LinePtr);
	    group = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));
	    group_name = (char far *) LinePtr
	      + sizeof (TypLine) + sizeof (TypGroup);

	    // Free whatever headers are there.
	    if (group->header_handle)
	      free_headers (group->header_handle, group->thread_handle);

	    group->header_handle = (HANDLE) NULL;
	    group->thread_handle = (HANDLE) NULL;
	    
	    // zero out the received line count
	    RcvLineCount = 0;

	    // capture the mouse to the usenet window, so that we keep
	    // the hourglass.
	    SetCapture (hWndConf);

	    SetGroupMenus(hWnd, DISABLE);

	    CommDoc = ThisDoc;
	    CommState = ST_GROUP_RESP;
	    strcpy (mybuf, "GROUP ");
	    lstrcat (mybuf, group_name);
	    PutCommLine (mybuf);
	  }
	  break;

	  }
      break;

	case IDM_ARTICLE_RETRIEVE_COMPLETE:
	{
	  TypGroup far * group;

	  header_p headers;
	  HANDLE header_handle;
	  HANDLE thread_handle;
          int justSavedIndex;
          
	  if (savingArtIndex == -1)	   /* this retrieve wasn't part of a save operation */
		 break;

	  if (CodingState == INACTIVE)
	  {
		/* If this is the 1st article saved, then value of Append depends
		 * on what the user selected in dialog.  If > 1st, always append 
		 */
		if (!MRRWriteDocument (ActiveArticleDoc, sizeof (TypText),
				       SaveArtFileName,
				       (numArtsSaved==0)?SaveArtAppend:TRUE))
	  	{
		     	MessageBox (hWnd, "Could not write to file.  Operation canceled", "Problems saving file", MB_OK | MB_ICONEXCLAMATION);
			SetMenusForMultiArticleOperation (hWnd, ENABLE);
		     	savingArtIndex = -1;
	     		break;
	  	}
	  } else {
                if (CompleteThisDecode() == FAIL)
		{
			DecodeDone ();                	
		     	MessageBox (hWnd, "Aborted decode", "Problems during decode", MB_OK | MB_ICONEXCLAMATION);
			SetMenusForMultiArticleOperation (hWnd, ENABLE);
		     	savingArtIndex = -1;
                	break;
                }
	  }

	  justSavedIndex = savingArtIndex;
	  numArtsSaved++;
	  savingArtIndex++;

	doretrieve:;
	  LockLine (ThisDoc->hParentBlock, ThisDoc->ParentOffset, ThisDoc->ParentLineID,
		      &BlockPtr, &LinePtr);
	  group = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));
	  header_handle = group->header_handle;
	  thread_handle = group->thread_handle;
	  headers = lock_headers (header_handle,thread_handle);
	  UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
	  
	  if (numArtsSaved > 0)		// deselect the article just finished
		(header_elt(headers, justSavedIndex))->Selected = FALSE;
	
	  /* skip to next selected article */
	  while (savingArtIndex < group->total_headers &&
     		(header_elt(headers, savingArtIndex))->Selected == FALSE)
		 savingArtIndex++;
		
	  if (savingArtIndex >= group->total_headers)	/* done */
	  {
	    if (numArtsSaved > 0) {
	        if (CodingState == INACTIVE)
		{
			sprintf (mybuf, "Saved %d article%c to file %s", 
				numArtsSaved, (numArtsSaved==1)?' ':'s',SaveArtFileName);
			MessageBox (hWnd, mybuf, "Done", MB_OK);
		}
	    } else
		MessageBox (hWnd, "No articles selected", "Done", MB_OK);
		  
            if (CodingState != INACTIVE)
		DecodeDone();

	    SetMenusForMultiArticleOperation (hWnd, ENABLE);

//      (JSC) now deselect article as soon as it's successfully finished 
//	    for (savingArtIndex=0; savingArtIndex < group->total_headers; savingArtIndex++ )
//		(header_elt(headers, savingArtIndex))->Selected = FALSE;

	    unlock_headers (header_handle,thread_handle);
  	    savingArtIndex = -1;		/* reset to non-saving state */
	  }
	  else
	  {
		if (KeepArticleHeaderVisible)
			AdjustTopScByDoc (ThisDoc, savingArtIndex);
			
		sprintf(mybuf, "%ld", (header_elt(headers, savingArtIndex))->number);
	  	(header_elt(headers, savingArtIndex))->Seen = TRUE;
		unlock_headers (header_handle, thread_handle);
		if (CodingState != INACTIVE)
		{ 
		  if ((currentCoded = InitCoded(hWnd)) == NULL)
		  {
			MessageBox (hWnd, "Unable to continue due to memory constraints.  Aborted", "Init Coded Object Error", MB_OK);
			DecodeDone();
			SetMenusForMultiArticleOperation (hWnd, ENABLE);
			savingArtIndex = -1;
			break;
		  }
		  InitiateReceiveArticle (ThisDoc, mybuf);
		  UpdateBlockStatus ();	// display some initial status
		} else 
 	     	  ViewArticle (ThisDoc, savingArtIndex, REUSE, NO_SHOW, NO_ID);
	  }
     	  InvalidateRect (hWnd, NULL, FALSE);
	  break;
	}	
	   
    default:
      return (DefWindowProc (hWnd, message, wParam, lParam));
    }
  return (0);
}             

/*------------ CloseGroupWnd ------------------------------
 *
 *  Make sure this Wnd is not the active Comm window, then destroy it
 */
void CloseGroupWnd(HWND hWnd, TypDoc *ThisDoc)
{
      if (CommBusy && ThisDoc == CommDoc)
 	MessageBox (hWnd,
	  "Please wait until group activity is complete",
	  "Cannot close group window", MB_OK|MB_ICONSTOP);
      else
        DestroyWindow (hWnd);
}

/*------------ SetGroupMenus ------------------------------
 * dis/enable menu items which depend on group list being completely 
 * retrieved
 */
void 
SetGroupMailMenu(HWND hWnd)
{
	HMENU hMenu, hSubMenu;

	hMenu = GetMenu (hWnd);
	hSubMenu = GetSubMenu (hMenu, 0);	// Articles menu
	EnableMenuItem (hSubMenu, IDM_MAIL, MailCtrl.enableMail);
}

void
SetGroupMenus (HWND hWnd, int enable)	
{
	HMENU hMenu, hSubMenu;
	UINT mode;
	
	if (enable == ENABLE)	
		mode = MF_BYCOMMAND|MF_ENABLED;
	else
		mode = MF_BYCOMMAND|MF_DISABLED|MF_GRAYED;
	
	hMenu = GetMenu (hWnd);
	hSubMenu = GetSubMenu (hMenu, 0);	// Articles menu
	EnableMenuItem (hSubMenu, IDM_UPDATE, mode);
	EnableMenuItem (hSubMenu, IDM_SAVE_SELECTED, mode);
	EnableMenuItem (hSubMenu, IDM_DECODE_SELECTED, mode);
	EnableMenuItem (hSubMenu, IDM_SELECT_ALL, mode);
	EnableMenuItem (hSubMenu, IDM_DESELECT_ALL, mode);
	EnableMenuItem (hSubMenu, IDM_SELECT_MATCH, mode);
	EnableMenuItem (hSubMenu, IDM_MARK_ALL, mode);
	EnableMenuItem (hSubMenu, IDV_EXIT, mode);
    SetGroupMailMenu(hWnd);
    
	hSubMenu = GetSubMenu (hMenu, 1);	// Sort menu
	EnableMenuItem (hSubMenu, IDM_SORT_DATE, mode);
	EnableMenuItem (hSubMenu, IDM_SORT_SUBJECT, mode);
	EnableMenuItem (hSubMenu, IDM_SORT_LINES, mode);
	EnableMenuItem (hSubMenu, IDM_SORT_THREADS, mode);
	EnableMenuItem (hSubMenu, IDM_SORT_ARTNUM, mode);
	EnableMenuItem (hSubMenu, IDM_SORT_FROM, mode);

	hSubMenu = GetSubMenu (hMenu, 2);	// Search menu
	EnableMenuItem (hSubMenu, IDM_FIND, mode);
	EnableMenuItem (hSubMenu, IDM_FIND_NEXT_SAME, mode);
}

/*------------ SetMenusForMultiArticleOperation ----------
 *  (JSC) killer function name, huh?!
 * During multi-article op (i.e. Save selected, Decode selected)
 * we don't want the user to do any of these things
 */
void
SetMenusForMultiArticleOperation (HWND hWnd, int enable)
{
	HMENU hMenu, hSubMenu;
	UINT mode;
	
	if (enable == ENABLE)	
		mode = MF_BYCOMMAND|MF_ENABLED;
	else
		mode = MF_BYCOMMAND|MF_DISABLED|MF_GRAYED;
	
	hMenu = GetMenu (hWnd);
	hSubMenu = GetSubMenu (hMenu, 0);	// Articles menu
	EnableMenuItem (hSubMenu, IDM_UPDATE, mode);
	EnableMenuItem (hSubMenu, IDM_SAVE_SELECTED, mode);
	EnableMenuItem (hSubMenu, IDM_DECODE_SELECTED, mode);
	EnableMenuItem (hSubMenu, IDV_EXIT, mode);
	EnableMenuItem (hSubMenu, IDM_MARK_ALL, mode);

	hSubMenu = GetSubMenu (hMenu, 1);	// Sort menu
	EnableMenuItem (hSubMenu, IDM_SORT_DATE, mode);
	EnableMenuItem (hSubMenu, IDM_SORT_SUBJECT, mode);
	EnableMenuItem (hSubMenu, IDM_SORT_LINES, mode);
	EnableMenuItem (hSubMenu, IDM_SORT_THREADS, mode);
	EnableMenuItem (hSubMenu, IDM_SORT_ARTNUM, mode);
	EnableMenuItem (hSubMenu, IDM_SORT_FROM, mode);
}

/* --- Function FileLength -------------------------------------------
 *
 *    Find the size, in bytes, of a file.
 *
 *    Entry    hFile    handle of the file in question.
 *
 *    Exit     returns the length of the file in bytes.
 *
 *    This routine is no longer used.
 */

long
FileLength (hFile)
     HFILE hFile;
{
  long lCurrentPos = _llseek (hFile, 0L, 1);
  long lFileLength = _llseek (hFile, 0L, 2);

  _llseek (hFile, lCurrentPos, 0);

  return lFileLength;

}

BOOL
article_operation (TypDoc * Doc,
		   long artindex,
		   BOOL (*art_fun) (header_p headers,
				    TypGroup * group,
				    long artindex))
{

  HANDLE hBlock;
  unsigned int Offset;
  TypLineID MyLineID;
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
  TypGroup * GroupDoc;
  HANDLE header_handle;
  HANDLE thread_handle;
  BOOL result = FALSE;
  header_p headers;

  LockLine (Doc->hParentBlock, Doc->ParentOffset,
	    Doc->ParentLineID, &BlockPtr, &LinePtr);

  GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));

  header_handle = GroupDoc->header_handle;
  if (header_handle) {

    thread_handle = GroupDoc->thread_handle;
    headers = lock_headers (header_handle,thread_handle);

    result = art_fun(headers, GroupDoc, artindex);

    unlock_headers (header_handle,thread_handle);
    UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
  }
  return (result);
}	

BOOL
toggle_read_unread (header_p headers,
		    TypGroup * GroupDoc,
		    long artindex)
{

  /* only try if in range.  All this work for these two lines 8^) */
  if ((artindex > -1 ) && (artindex < GroupDoc->total_headers)) {
    header_p header = header_elt (headers, artindex);
    header->Seen = ( header->Seen ? FALSE : TRUE );
    return (header->Seen);
  }
  return(FALSE);
}

BOOL
mark_read_to_here (header_p headers,
		   TypGroup * GroupDoc,
		   long artindex)
{
  header_p header;

  /* only try if in range.  All this work for these two lines 8^) */
  if ((artindex > -1 ) && (artindex < GroupDoc->total_headers)) {
    int i;
    for (i = 0; i <= artindex; i++) {
      header = header_elt (headers, i);
      header->Seen = TRUE;
    }
    return(TRUE);
  }
 return(FALSE);
}

BOOL
mark_read_all (header_p headers,
		   TypGroup * GroupDoc,
		   long artindex)
{
  header_p header; 

    int i;
    for (i = 0; i < GroupDoc->total_headers; i++) {
      header = header_elt (headers, i);
      header->Seen = TRUE;
    }
    return(TRUE);
}

BOOL
toggle_selected (header_p headers,
		 TypGroup * GroupDoc,
		 long artindex)
{

  /* only try if in range.  All this work for these two lines 8^) */
  if ((artindex > -1 ) && (artindex < GroupDoc->total_headers)) {
    header_p header = header_elt (headers, artindex);
    header->Selected = ( header->Selected ? FALSE : TRUE );
    return(header->Selected);
  }
 return(FALSE);
}

BOOL
seen_false (header_p headers,
		 TypGroup * GroupDoc,
		 long artindex)
{

  /* only try if in range. */
  if ((artindex > -1 ) && (artindex < GroupDoc->total_headers)) {
    header_p header = header_elt (headers, artindex);
    header->Seen = FALSE;
    return(TRUE);
  }
 return(FALSE);
}

BOOL
seen_true (header_p headers,
		 TypGroup * GroupDoc,
		 long artindex)
{

  /* only try if in range. */
  if ((artindex > -1 ) && (artindex < GroupDoc->total_headers)) {
    header_p header = header_elt (headers, artindex);
    header->Seen = TRUE;
    return(TRUE);
  }
 return(FALSE);
}

BOOL
selected_true (header_p headers,
		 TypGroup * GroupDoc,
		 long artindex)
{

  /* only try if in range. */
  if ((artindex > -1 ) && (artindex < GroupDoc->total_headers)) {
    header_p header = header_elt (headers, artindex);
    header->Selected = TRUE;
    return(TRUE);
  }
 return(FALSE);
}

BOOL
selected_false (header_p headers,
		 TypGroup * GroupDoc,
		 long artindex)
{

  /* only try if in range. */
  if ((artindex > -1 ) && (artindex < GroupDoc->total_headers)) {
    header_p header = header_elt (headers, artindex);
    header->Selected = FALSE;
    return(TRUE);
  }
 return(FALSE);
}


/*-- function ViewArticle -------------------------------------------------
 *
 *  View a given article.   Either create a new window for it or
 *  recycle an existing window.
 *  This function requests an article from the server, so there
 *  must not already be a transaction in progress.
 *  Can retrieve an article by artindex from the group header list,
 *  or any arbitrary article ID.
 *
 *    Entry    Doc            points to the document for this group.
 *             artindex       index into header array for this group.
 *             Reuse          is TRUE if we ought to reuse the
 *                            currently active article window (if any).
 *             showArt        is TRUE to retrieve maximized (shown)
 *                            is FALSE to retrieve minimized
 *             articleId      is NO_ID if wish to retrieve artindex->number
 *                            is some art ID if wish to retrieve article by ID
 */
void
ViewArticle (Doc, artindex, Reuse, showArt, articleId)
     TypDoc *Doc;
     long artindex;
     BOOL Reuse;
     BOOL showArt;
     char far *articleId;
{ 
  TypDoc *MyDoc;
  TypGroup far *GroupDoc;
  BOOL newdoc;
  BOOL found;
  int docnum;
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
  HWND hWndArt;
  int x,y,width,height;
  char mybuf[MAXINTERNALLINE];
  char far *lpsz;
  HWND hWndGroup = Doc->hDocWnd;
  HANDLE header_handle;
  HANDLE thread_handle;
  header_p headers;
  header_p header;

  
  LockLine (Doc->hParentBlock, Doc->ParentOffset, Doc->ParentLineID,
	    &BlockPtr, &LinePtr);

  GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));

  
   if (articleId == NO_ID)   /* if retrieving artindex, only try if in range */
      if ((artindex < 0) || (artindex >= GroupDoc->total_headers))
         return;

  header_handle = GroupDoc->header_handle;
  thread_handle = GroupDoc->thread_handle;

  headers = lock_headers (header_handle,thread_handle);
  header = header_elt(headers, artindex);

  if (articleId == NO_ID && (MyDoc = header->ArtDoc))
    {
      /* We already have a document containing the article */
      /* so just activate it.                */

      if (showArt)
      {
	 setArticleFocus( MyDoc->hDocWnd );
         ShowWindow(MyDoc->hDocWnd,SW_SHOWNORMAL); 
      } else
         ShowWindow(MyDoc->hDocWnd,SW_SHOWMINIMIZED);

      InvalidateRect (MyDoc->hDocWnd, NULL, FALSE);
      SendMessage(hWndGroup, IDM_ARTICLE_RETRIEVE_COMPLETE, 0, 0);
      goto endit;
    }
  if (TestCommBusy(hWndGroup, "Can't request text of article"))
     goto endit;
  newdoc = FALSE;
  if ((NewArticleWindow && !Reuse) || !ActiveArticleDoc || !(ActiveArticleDoc->InUse))
    {
      found = FALSE;
      for (docnum = 0; docnum < MAXARTICLEWNDS; docnum++)
	{
	  if (!ArticleDocs[docnum].InUse)
	    {
	      found = TRUE;
	      newdoc = TRUE;
	      CommDoc = &(ArticleDocs[docnum]);
	      break;
	    }
	}
      if (!found)
	{
	  MessageBox (hWndGroup,
		      "You have too many article windows active;\n"
		      "Close one or uncheck the option"
		      "'New Window for each Article'.",
		      "Can't open new window", MB_OK | MB_ICONASTERISK);
	  goto endit;
	}
    }
  else
    {
      /* Must reuse old window for this article.         */
      ActiveArticleDoc->LongestLine = 0;
      ActiveArticleDoc->ScXOffset = 0;
      ActiveArticleDoc->TextSelected = FALSE;
      EnableMenuItem(GetMenu(ActiveArticleDoc->hDocWnd),IDM_COPY,MF_GRAYED) ;
      EnableMenuItem(GetMenu(ActiveArticleDoc->hDocWnd),IDM_DESELECT_ALL,MF_GRAYED) ;
      CommDoc = ActiveArticleDoc;
      /* sever the article/artindex connection */
      if (CommDoc->ParentDoc == Doc)    	// (JSC) is this check really necessary?
	   (header_elt (headers,CommDoc->ParentOffset))->ArtDoc = (TypDoc *) NULL;

      /* clear out old doc */
      FreeDoc (CommDoc);
    }

  header->Seen = TRUE;
  InvalidateRect (hWndGroup, NULL, FALSE);

  if (articleId == NO_ID)
     lpsz = (char far *) header->subject;
  else
     lpsz = articleId;

  strcpy (mybuf, "Retrieving \"");
  lstrcat (mybuf, lpsz);
  lstrcat (mybuf, "\"");

  if (newdoc)
    {
      char poschars[MAXINTERNALLINE];

      /* Compute default screen position. */
      if (xScreen > 88 * ArtCharWidth) {
	width = 88 * ArtCharWidth;
      } else {
	width = xScreen - 1 * ArtCharWidth;
      }
      x = xScreen - width;
      y = (int) (yScreen * 3 / 8);
      height = (int) (yScreen * 5 / 8) - (1 * ArtLineHeight);

      /* If the screen position has been saved, use that instead. */
      GetPrivateProfileString (szAppName, "ArticleWindowPos", "!",
			       poschars,MAXINTERNALLINE,szAppProFile);
      if(poschars[0] != '!') {
	sscanf(poschars,"%d,%d,%d,%d",&x,&y,&width,&height);
      }

      hWndArt = CreateWindow ("WinVnArt",
			      mybuf,
			      WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
			      /* Initial X pos */
			      x - (docnum * ArtCharWidth),
			      /* Initial Y pos */
			      y + docnum * ArtLineHeight,
			      width, /* Initial X Width */
			      /* Initial Y height */
			      height,
			      NULL,
			      NULL,
			      hInst,
			      NULL);

      if (!hWndArt)      
	return;                 /* ??? */

	  SetHandleBkBrush (hWndArt, hArticleBackgroundBrush);
      ShowWindow (hWndArt, SW_SHOWNORMAL);
    }
  else
    {
      hWndArt = CommDoc->hDocWnd;               
      SetWindowText (hWndArt, mybuf);
    }

  /*  Now that we have created the window, create the corresponding
   *  document, and make the new window active.
   */

  InitDoc (CommDoc, hWndArt, Doc, DOCTYPE_ARTICLE);

  CommDoc->InUse = TRUE;
  CommDoc->LastSeenLineID = artindex;   /* Keep an index with the article */

  if (showArt)
  {
    setArticleFocus( hWndArt) ;
    ShowWindow (hWndArt, SW_SHOWNORMAL);
  } else
    ShowWindow (hWndArt, SW_SHOWMINIMIZED);

  header->ArtDoc = CommDoc;
  CommDoc->ParentOffset = (int) artindex;   /* should this be iff articleId==NO_ID ? */
  InvalidateRect (hWndArt, NULL, FALSE);
  UpdateWindow (hWndArt);

  if (articleId == NO_ID)
  {
    sprintf(mybuf, "%ld", header->number);
    InitiateReceiveArticle (Doc, mybuf);
  }
  else
    InitiateReceiveArticle (Doc, articleId);
  
 endit:
  unlock_headers (header_handle, thread_handle);
  
}

void
setArticleFocus(HWND hWndArt) 
{
  SetArticleRot13Mode(hWndArt,FALSE) ;
  SetActiveWindow (hWndArt);
  SetFocus (hWndArt);
}

void
InitiateReceiveArticle (TypDoc *Doc, char far *articleId)
{
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
  char far *lpszGroupName;
  char mybuf[MAXINTERNALLINE];
  HANDLE hBlock;
  unsigned int Offset;
  TypLineID MyLineID;

  CommLinePtr = CommLineIn;
  CommBusy = TRUE;
  CommState = ST_ARTICLE_RESP;

  LockLine (Doc->hParentBlock, Doc->ParentOffset, Doc->ParentLineID,
	    &BlockPtr, &LinePtr);

  /* If we're not already in this group on the server,
   * send out a GROUP command for this window so we get back
   * into the right Group.
   */
  lpszGroupName = (char far *) LinePtr + sizeof (TypLine) + sizeof (TypGroup);
  if (lstrcmp (CurrentGroup, lpszGroupName))
    {
      CommState = ST_GROUP_REJOIN;
      strcpy (mybuf, "GROUP ");
      lstrcat (mybuf, lpszGroupName);
      mylstrncpy (CurrentGroup, lpszGroupName, MAXGROUPNAME);
      PutCommLine (mybuf);
    }

  UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);

  sprintf (mybuf, "ARTICLE %s", articleId);
  PutCommLine (mybuf);
}


/*-- Function UnlinkArtsInGroup ---------------------------------------
 *
 *  Modify all the article documents and all the article windows currently
 *  associated with a group so that none of them points to that group.
 *  Used when the group window is going away or is being recycled.
 *
 *    Entry    GroupDoc    points to the document to which references
 *                         should be eliminated.
 */
void
UnlinkArtsInGroup (GroupDoc)
     TypDoc *GroupDoc;
{
  int iart;

  for (iart = 0; iart < MAXARTICLEWNDS; iart++)
    {
      if (ArticleDocs[iart].InUse && ArticleDocs[iart].ParentDoc == GroupDoc)
	{
	  ArticleDocs[iart].ParentDoc = (TypDoc *) NULL;
	  ArticleDocs[iart].hParentBlock = 0;
	}
    }
}

/*--- function UpdateSeenArts -------------------------------------------
 *
 *  Given a Group document, update the TypGroup line for
 *  that document in the Net document with respect to which
 *  articles have been seen.
 *  This routine would typically be called just before a Group document
 *  is going to be destroyed or erased.  That would be the time to
 *  take the information in the TypArticle structures of each line
 *  in the document and transfer it to the line in the NetDoc document
 *  corresponding to this group.
 *
 *  This routine has to take information of the form:
 *    123:Unseen;  124:Seen; 125:Unseen; 126:Unseen; 127:Seen; 128:Seen; 129:Seen
 *  found in the TypArticle structures in consecutive lines in the document
 *  and transform it to the general form used by .newsrc files:
 *    124,127-129
 *  (though we are using our internal representation & not ASCII characters).
 *
 *    Entry    Doc      points to the document for this group.
 *
 *    Exit     The line in the Net document corresponding to this
 *              group has been updated.
 */
void
UpdateSeenArts (Doc)
     TypDoc *Doc;
{
  TypRange MyRange, *RangePtr;
  TypGroup *group;
  TypLine far * ParentLine;
  TypBlock far * ParentBlock;
  HANDLE hLine,header_handle,thread_handle;
  TypLine *LocalLinePtr;
  header_p headers;
  header_p header;
  BOOL InSeen = TRUE;
  unsigned int MyLength;
  unsigned int maxRanges;
  unsigned long artindex;

  /*  Get the line in the Net document that corresponds to this
   *  group.  Make a local copy of it and set RangePtr to point to
   *  the first range in that line.  We will ignore the old line's
   *  "seen" data and create the information afresh from what we
   *  have in this document.
   */
  LockLine (Doc->hParentBlock, Doc->ParentOffset, Doc->ParentLineID,
	    &ParentBlock, &ParentLine);

  hLine = LocalAlloc (LMEM_MOVEABLE, BLOCK_SIZE);
  LocalLinePtr = (TypLine *) LocalLock (hLine);
  group = (TypGroup *) ((char *) LocalLinePtr + sizeof (TypLine));

  MoveBytes (ParentLine, LocalLinePtr, ParentLine->length);

  if (group->total_headers > 0) {

    header_handle = group->header_handle;
    thread_handle = group->thread_handle;
    set_index_to_identity (header_handle, thread_handle, group->total_headers);

    headers = lock_headers (header_handle,thread_handle);

    group->nRanges = 0;

    maxRanges = ((Doc->BlockSize - Doc->SplitSize) - ParentLine->length +
		 group->nRanges * sizeof (TypRange)) / sizeof (TypRange) - 1;

    RangePtr = (TypRange *) ((char *) LocalLinePtr + sizeof (TypLine) +
			     RangeOffset (group->NameLen));
    MyRange.First = 1;

    /* Get the first line in this document.
     * If it cannot be found, just set Last=First and skip the
     * proceeding processing.  Otherwise, assume we've seen everything
     * up to but not including the first article in the document.
     */

    /*  LockLine (Doc->hFirstBlock, sizeof (TypBlock), 0L, &BlockPtr, &LinePtr); */

    artindex = 0;
    if (!Doc->TotalLines)
      {
	MyRange.Last = 1;
      }
    else
      {
	header = (header_elt (headers,artindex));
	MyRange.Last = header->number - 1;

	/* Loop to scan through the document, fabricating article ranges.
	 */
	do
	  {
	    header = (header_elt (headers,artindex));
	    if (header->Seen)
	      {
		if (InSeen)
		  {
		    /* Continuing a sequence of seen articles.            */
		    MyRange.Last = header->number;
		  }
		else
		  {
		    /* Starting a new sequence of seen articles.          */
		    MyRange.First = header->number;
		    MyRange.Last = header->number;
		    InSeen = TRUE;
		  }
	      }
	    else
	      {
		if (InSeen)
		  {
		    /* Ending a sequence of seen articles.                   */
		    InSeen = FALSE;
		    *(RangePtr++) = MyRange;
		    (group->nRanges)++;
		  }
		else
		  {
		    /* Continuing a sequence of unseen articles.             */
		  }
	      }
	  }
	while (( group->nRanges < maxRanges) &&
	       ((++artindex < Doc->TotalLines)));

	if (InSeen)
	  {
	    *(RangePtr++) = MyRange;
	    (group->nRanges)++;
	  }
      }

  }

  unlock_headers (header_handle, thread_handle);

  MyLength = sizeof (TypLine) + RangeOffset (group->NameLen) +
    sizeof (TypRange) * (group->nRanges) + sizeof (int);

  LocalLinePtr->length = MyLength;
  *(int *) ((char *) LocalLinePtr + MyLength - sizeof (int)) = MyLength;

  ReplaceLine (LocalLinePtr, &ParentBlock, &ParentLine);
  GlobalUnlock (ParentBlock->hCurBlock);

  LocalUnlock (hLine);
  LocalFree (hLine);

}

/*-- function CursorToTextLine ----------------------------------------
 *
 *   Routine to locate a text line in a document, based on the
 *   cursor position.  Used to figure out which line is being selected
 *   when a user clicks a mouse button.
 *
 *   Entry    X, Y    are the position of the cursor.
 *            DocPtr  points to the current document.
 *
 *   Exit     *LinePtr points to the current line, if one was found.
 *            *BlockPtr points to the current block, if found.
 *            Function returns TRUE iff a line was found that corresponds
 *              to the cursor position.
 */
long
NewCursorToTextLine (X, Y, DocPtr)
     int X;
     int Y;
     TypDoc *DocPtr;
{
  int SelLine;

  if (Y < TopSpace || (unsigned) Y > TopSpace + DocPtr->ScYLines * LineHeight ||
      X < SideSpace)
    {
      /* Cursor is in no-man's-land at edge of window.               */
	return(-1);
    }
  else
    {
      SelLine = (Y - TopSpace) / LineHeight;
	return ( DocPtr->TopLineOrd + SelLine);
    }
}

/* this has been changed to use header_elt (SMR) */

long
search_headers (TypDoc * HeaderDoc, header_p headers,
		long artindex, long num_headers)
{

  char_p temp;

  do {
    temp = (header_elt(headers,artindex))->subject;
    if (string_compare_insensitive (temp, HeaderDoc->SearchStr))
      return (artindex);        /* return the index */
  } while (artindex++ < (num_headers - 1));

  return (-1);                  /* not found */
}

char_p
string_compare_insensitive (char_p a, char far * b)
{
  int lena = lstrlen (a);
  int lenb = lstrlen (b);
  int count;

  for ( count = lena - lenb + 1 ; count > 0 ; count--, a++)
   if (strnicmp (a,b,lenb) == 0)
     return a;

  return (NULL);
}

long
AffectSelected (TypDoc *Doc, BOOL value, BOOL compare)
{
  TypGroup far * group;
  HANDLE header_handle,thread_handle;
  header_p headers;
  TypBlock far *BlockPtr;
  TypLine far * LinePtr;
  char_p temp;
  long index, num_affected;  

  LockLine (Doc->hParentBlock, Doc->ParentOffset,
            Doc->ParentLineID, &BlockPtr, &LinePtr);
  group = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));

  header_handle = group->header_handle;
  thread_handle = group->thread_handle;

  headers = lock_headers (header_handle,thread_handle);
 
  for (index=0, num_affected=0; index < group->total_headers; index++ )
    if (compare) {
       temp = (header_elt(headers, index))->subject;
       if (string_compare_insensitive (temp, Doc->SearchStr)) {
          (header_elt(headers, index))->Selected = value;
          num_affected++;
       }
    } else {
       (header_elt(headers, index))->Selected = value;
       num_affected++;
    }	       

  InvalidateRect(Doc->hDocWnd,NULL,FALSE);		
  return (num_affected);
}
