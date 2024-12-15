head	1.47;
access;
symbols
	V80:1.15
	V76d:1.1;
locks; strict;
comment	@ * @;


1.47
date	94.08.24.18.00.29;	author jcooper;	state Exp;
branches;
next	1.46;

1.46
date	94.08.11.00.16.48;	author jcooper;	state Exp;
branches;
next	1.45;

1.45
date	94.07.27.21.10.27;	author gardnerd;	state Exp;
branches;
next	1.44;

1.44
date	94.07.25.18.51.48;	author jcooper;	state Exp;
branches;
next	1.43;

1.43
date	94.06.30.21.29.38;	author gardnerd;	state Exp;
branches;
next	1.42;

1.42
date	94.06.06.22.06.13;	author gardnerd;	state Exp;
branches;
next	1.41;

1.41
date	94.06.01.19.07.05;	author gardnerd;	state Exp;
branches;
next	1.40;

1.40
date	94.05.27.01.29.29;	author rushing;	state Exp;
branches;
next	1.39;

1.39
date	94.05.23.18.36.00;	author jcooper;	state Exp;
branches;
next	1.38;

1.38
date	94.05.19.02.02.10;	author rushing;	state Exp;
branches;
next	1.37;

1.37
date	94.05.02.20.57.30;	author rushing;	state Exp;
branches;
next	1.36;

1.36
date	94.03.01.19.10.19;	author rushing;	state Exp;
branches;
next	1.35;

1.35
date	94.02.24.21.30.55;	author jcoop;	state Exp;
branches;
next	1.34;

1.34
date	94.02.16.21.04.46;	author cnolan;	state Exp;
branches;
next	1.33;

1.33
date	94.02.09.18.01.08;	author cnolan;	state Exp;
branches;
next	1.32;

1.32
date	94.01.24.17.40.28;	author jcoop;	state Exp;
branches;
next	1.31;

1.31
date	94.01.15.21.53.23;	author jcoop;	state Exp;
branches;
next	1.30;

1.30
date	93.12.27.20.16.15;	author jcoop;	state Exp;
branches;
next	1.29;

1.29
date	93.12.08.20.21.15;	author dumoulin;	state Exp;
branches;
next	1.28;

1.28
date	93.12.08.01.27.21;	author rushing;	state Exp;
branches;
next	1.27;

1.27
date	93.09.08.22.37.30;	author rushing;	state Exp;
branches;
next	1.26;

1.26
date	93.08.25.18.54.36;	author mbretherton;	state Exp;
branches;
next	1.25;

1.25
date	93.08.24.17.57.22;	author rushing;	state Exp;
branches;
next	1.24;

1.24
date	93.08.12.22.37.51;	author dumoulin;	state Exp;
branches;
next	1.23;

1.23
date	93.08.05.20.06.07;	author jcoop;	state Exp;
branches;
next	1.22;

1.22
date	93.07.06.21.09.50;	author cnolan;	state Exp;
branches;
next	1.21;

1.21
date	93.06.28.17.52.17;	author rushing;	state Exp;
branches;
next	1.20;

1.20
date	93.06.25.20.25.06;	author dumoulin;	state Exp;
branches;
next	1.19;

1.19
date	93.06.24.17.13.14;	author riordan;	state Exp;
branches;
next	1.18;

1.18
date	93.06.22.16.43.28;	author bretherton;	state Exp;
branches;
next	1.17;

1.17
date	93.06.22.16.13.40;	author rushing;	state Exp;
branches;
next	1.16;

1.16
date	93.06.14.18.51.32;	author rushing;	state Exp;
branches;
next	1.15;

1.15
date	93.06.11.01.10.21;	author rushing;	state Exp;
branches;
next	1.14;

1.14
date	93.06.11.00.11.26;	author rushing;	state Exp;
branches;
next	1.13;

1.13
date	93.06.10.18.27.18;	author rushing;	state Exp;
branches;
next	1.12;

1.12
date	93.06.08.19.39.45;	author rushing;	state Exp;
branches;
next	1.11;

1.11
date	93.06.07.21.36.22;	author rushing;	state Exp;
branches;
next	1.10;

1.10
date	93.06.05.03.18.04;	author rushing;	state Exp;
branches;
next	1.9;

1.9
date	93.06.01.18.23.04;	author rushing;	state Exp;
branches;
next	1.8;

1.8
date	93.05.27.00.00.09;	author rushing;	state Exp;
branches;
next	1.7;

1.7
date	93.05.25.00.00.45;	author rushing;	state Exp;
branches;
next	1.6;

1.6
date	93.05.19.23.48.36;	author rushing;	state Exp;
branches;
next	1.5;

1.5
date	93.05.13.20.01.02;	author SOMEONE;	state Exp;
branches;
next	1.4;

1.4
date	93.05.06.20.58.15;	author rushing;	state Exp;
branches;
next	1.3;

1.3
date	93.05.01.01.38.17;	author rushing;	state Exp;
branches;
next	1.2;

1.2
date	93.03.30.20.05.39;	author rushing;	state Exp;
branches;
next	1.1;

1.1
date	93.02.16.20.53.50;	author rushing;	state Exp;
branches;
next	;


desc
@winvn version 0.76 placed into RCS
@


1.47
log
@misc encoding/decoding changes
@
text
@/********************************************************************
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
 * $Id: wvgroup.c 1.46 1994/08/11 00:16:48 jcooper Exp $
 *
 */

#include "windows.h"
#include "WVglob.h"
#include "WinVn.h"

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
	int MyColorMask = 0;
	int PrevColorMask = MyColorMask;  
	
	/* MyColors and MyBack are arrays of colors used to display text
	 * foreground and background.
	 * The ColorMask variables are indices into these arrays.
	 * We set and clear bits in these indices depending upon
	 * whether the article has been selected or seen.
	 */

#define SEEN_MASK 1
#define SELECT_MASK 2
	hDC = BeginPaint (hWnd, &ps);
        SetBkColor (hDC, ListBackgroundColor);

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
           MyBack[2] = RGB(200,200,200);	// selected = white background
        else
           MyBack[2] = RGB(0,0,0);		// selected = black background
	MyBack[3] = MyBack[2];


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

	  if (LOWORD(wParam) == IDM_FIND)
	    DialogBox (hInst, "WinVnFind", hWnd, lpfnWinVnFindDlg);
	  if(strcmp(FindDoc->SearchStr,"")) {
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
	  if (!DialogBox (hInst, "WinVnDecodeArts", hWnd, lpfnWinVnDecodeArtsDlg)) {
	     InvalidateRect (hWnd, NULL, TRUE);
	  } else {
	     if (TestCommBusy(hWnd, "Can't decode selected articles"))
	       break;
	     if (TestDecodeBusy(hWnd, "Can't decode selected articles"))
	       break;
	       
	     DecodeInit();
	     CommDoc = ThisDoc;
	     CommDecoding = TRUE;
	     numFilesDecoded = 0;
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
     HANDLE hFile;
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
@


1.46
log
@added support for IDM_RETRIEVE_COMPLETE
@
text
@d41 1
a41 1
 * $Id: wvgroup.c 1.45 1994/07/27 21:10:27 gardnerd Exp $
d72 1
d145 1
a145 1
   case WM_INITMENUPOPUP:
d148 1
d776 2
d798 2
a856 1

d859 1
d878 3
a880 2
	  HANDLE thread_handle;	    

d894 2
d898 1
a898 1
	  } else 
d903 2
d907 1
d909 1
d921 4
a924 1
		
d933 1
a933 8
	        if (CodingState != INACTIVE)
	        {       
	        	;
/*			sprintf (mybuf, "Decoded %d file%c", 
				numFilesDecoded, (numFilesDecoded==1)?' ':'s');
			MessageBox (hWnd, mybuf, "Done", MB_OK);
*/		}
		else
a942 1
            {
d944 2
a945 1
	    }
d947 3
a949 2
	    for (savingArtIndex=0; savingArtIndex < group->total_headers; savingArtIndex++ )
		(header_elt(headers, savingArtIndex))->Selected = FALSE;
d956 1
d965 3
a967 1
// FREE UP ANY STRAY DecodeObjects		
d1038 31
@


1.45
log
@copy to clipboard
@
text
@d41 1
a41 1
 * $Id: wvgroup.c 1.44 1994/07/25 18:51:48 jcooper Exp $
d71 1
a72 2
extern void 	     TestStatusText(HWND hWnd);

d131 4
d610 4
d626 1
a626 1
	  DestroyWindow (hWnd);
a702 1

d789 1
d866 1
a866 1
	case IDM_RETRIEVE_COMPLETE:
a934 1
                CommDecoding = FALSE;
d986 40
d1286 1
a1286 1
      SendMessage(hWndGroup, IDM_RETRIEVE_COMPLETE, 0, 0);
@


1.44
log
@execution of decoded files
@
text
@d41 1
a41 1
 * $Id: wvgroup.c 1.43 1994/06/30 21:29:38 gardnerd Exp $
d1274 3
@


1.43
log
@All scrolling by pixels instead of characters
@
text
@d41 1
a41 1
 * $Id: wvgroup.c 1.42 1994/06/06 22:06:13 gardnerd Exp $
d974 1
a974 1
	  "Please wait until group header retrieval is complete",
@


1.42
log
@horizontal scrolling support
@
text
@d41 1
a41 1
 * $Id: wvgroup.c 1.41 1994/06/01 19:07:05 gardnerd Exp $
a496 2
	

d514 1
a514 1
	X = SideSpace;
d583 4
a586 11
	      if((int)MyLen - (int)ThisDoc->ScXOffset < 0)
	      {
            ExtTextOut (hDC, X, Y, ETO_OPAQUE|ETO_CLIPPED, &aRect, 
	                    " ", 1, (LPINT)NULL);
	      }
	      else
	      {
            ExtTextOut (hDC, X, Y, ETO_OPAQUE|ETO_CLIPPED, &aRect, 
	                    scratch_line+ThisDoc->ScXOffset, MyLen-ThisDoc->ScXOffset,
	                    (LPINT)NULL);
	      }  
@


1.41
log
@horizontal scrolling support
@
text
@d41 1
a41 1
 * $Id: $
d410 4
d440 2
a441 2
	int PrevColorMask = MyColorMask;

d487 11
d585 11
a595 2
	      ExtTextOut (hDC, X, Y, ETO_OPAQUE|ETO_CLIPPED, &aRect, 
	                  scratch_line, MyLen, (LPINT)NULL);
@


1.40
log
@unnecessary winundoc.h
@
text
@d41 1
a42 118
 * Revision 1.38  1994/05/19  02:02:10  rushing
 * changes for gensock & version 0.91
 *
 * Revision 1.37  1994/05/02  20:57:30  rushing
 * mysterious F3 bug fixed.
 *
 * Revision 1.36  1994/03/01  19:10:19  rushing
 * ThreadFullSubject option added
 *
 * Revision 1.35  1994/02/24  21:30:55  jcoop
 * jcoop changes (overlap in merge, I went with jcoop's latest over cnolan's change)
 *
 * Revision 1.34  1994/02/16  21:04:46  cnolan
 * cnolan changes to 90.3
 *
 * Revision 1.33  1994/02/09  18:01:08  cnolan
 * cnolan 90.2 changes
 *
 * Revision 1.32  1994/01/24  17:40:28  jcoop
 * 90.2 changes
 *
 * Revision 1.31  1994/01/15  21:53:23  jcoop
 * Colors/fonts, integrated decoding, general cleanup
 *
 * Revision 1.30  1993/12/27  20:16:15  jcoop
 * copied wrong string into CurrentGroup for ST_GROUP_REJOIN
 * 
 * Revision 1.29  1993/12/08  20:21:15  dumoulin
 * Added support for using Shift-Mouse-Left inplace of Mouse-Middle and
 * support for dragging mouse to set article Seen flag
 *
 * Revision 1.28  1993/12/08  01:27:21  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.27  1993/09/08  22:37:30  rushing
 * correctly handle empty newsgroup windows destruction (cnolan)
 *
 * Revision 1.26  1993/08/25  18:54:36  mbretherton
 * MRB merge, mail & post logging
 *
 * Revision 1.25  1993/08/24  17:57:22  rushing
 * check for valid header element in article_operation
 *
 * Revision 1.24  1993/08/12  22:37:51  dumoulin
 * Added Mouse dragging to select/deselect code
 *
 * Revision 1.23  1993/08/05  20:06:07  jcoop
 * save multiple articles changes by jcoop@@oakb2s01.apl.com (John S Cooper)
 *
 * Revision 1.22  1993/07/06  21:09:50  cnolan
 * win32 support
 *
 * Revision 1.21  1993/06/28  17:52:17  rushing
 * fixed compiler warnings
 *
 * Revision 1.20  1993/06/25  20:25:06  dumoulin
 * changed dates from strings to POSIX standard numeric dates
 *
 * Revision 1.19  1993/06/24  17:13:14  riordan
 * Save window positions between sessions.
 *
 * Revision 1.18  1993/06/22  16:43:28  bretherton
 * save one character on sprintf for date
 *
 *
 * Revision 1.17  1993/06/22  16:13:40  rushing
 * unsigned long in sprintf
 *
 * Revision 1.16  1993/06/14  18:51:32  rushing
 * better handling of WM_DESTROY to 0-article group window
 *
 * Revision 1.15  1993/06/11  01:10:21  rushing
 * fixed boneheaded CommDoc bug again
 *
 * Revision 1.14  1993/06/11  00:11:26  rushing
 * second merge from Matt Bretherton sources
 *
 *
 * Revision 1.13  1993/06/10  18:27:18  rushing
 * fixed sort logic and range-writing
 *
 * Revision 1.12  1993/06/08  19:39:45  rushing
 * added the Sort... menu
 *
 * Revision 1.11  1993/06/07  21:36:22  rushing
 * fixed subject searches
 *
 * Revision 1.10  1993/06/05  03:18:04  rushing
 * primitive functional threading.
 *
 * Revision 1.9  1993/06/01  18:23:04  rushing
 * threading support
 *
 * Revision 1.8  1993/05/27  00:00:09  rushing
 * added support for 'mark article as read', and 'catch up'
 *
 * Revision 1.7  1993/05/25  00:00:45  rushing
 * rot13 merge (MRB)
 *
 * Revision 1.6  1993/05/19  23:48:36  rushing
 * range check in ViewArticle (artindex)
 *
 * Revision 1.5  1993/05/13  20:01:02  SOMEONE
 * fancy 'from' in group window
 *
 * Revision 1.4  1993/05/06  20:58:15  rushing
 * call UpdateSeenArts before processing the IDM_UPDATE: message
 *
 * Revision 1.3  1993/05/01  01:38:17  rushing
 * added 'update' feature.  doesn't copy headers yet.
 *
 * Revision 1.2  1993/03/30  20:05:39  rushing
 * MAPI
 *
 * Revision 1.1  1993/02/16  20:53:50  rushing
 * Initial revision
 *
 *
d159 1
a159 1

d1195 1
d1257 2
d1300 1
d1303 1
a1303 1
			      WS_OVERLAPPEDWINDOW | WS_VSCROLL ,
d1324 1
a1324 1
      hWndArt = CommDoc->hDocWnd;
@


1.39
log
@new attach code, session [dis]connect
@
text
@a162 1
#include "winundoc.h"
@


1.38
log
@changes for gensock & version 0.91
@
text
@d42 3
a44 2
 * $Id: wvgroup.c 1.37 1994/05/02 20:57:30 rushing Exp rushing $
 * $Log: wvgroup.c $
d188 1
a188 1

d272 1
a272 6
      if (CommBusy && ThisDoc == CommDoc)
 	MessageBox (hWnd,
	  "Please wait until group header retrieval is complete",
	  "Cannot close group window", MB_OK|MB_ICONSTOP);
      else
        DestroyWindow (hWnd);
d714 1
a714 1
	  DestroyWindow (hWnd);
d1068 14
@


1.37
log
@mysterious F3 bug fixed.
@
text
@d42 1
a42 1
 * $Id: wvgroup.c 1.36 1994/03/01 19:10:19 rushing Exp rushing $
d44 3
d959 1
a959 2
	    lstrcat (mybuf, "\r");
	    PutCommLine (mybuf, lstrlen (mybuf));
a1501 1
      lstrcat(mybuf,"\r");
d1503 1
a1503 1
      PutCommLine (mybuf, lstrlen (mybuf));
d1508 2
a1509 2
  sprintf (mybuf, "ARTICLE %s\r", articleId);
  PutCommLine (mybuf, lstrlen (mybuf));
@


1.36
log
@ThreadFullSubject option added
@
text
@d42 1
a42 1
 * $Id: wvgroup.c 1.35 1994/02/24 21:30:55 jcoop Exp rushing $
d44 3
d300 4
@


1.35
log
@jcoop changes (overlap in merge, I went with jcoop's latest over cnolan's change)
@
text
@d42 1
a42 1
 * $Id: wvgroup.c 1.34 1994/02/16 21:04:46 cnolan Exp $
d44 3
d646 5
a650 1
		       (header->thread_depth ? "\020" : header->subject));
@


1.34
log
@cnolan changes to 90.3
@
text
@d42 1
a42 1
 * $Id: wvgroup.c 1.33 1994/02/09 18:01:08 cnolan Exp $
d44 3
d404 1
d435 1
d445 1
d453 1
d523 1
a523 1
	int MyLen, width;
d528 1
a528 1
	int RestX, indicatorwidth, Xtext;
d543 1
a543 2
//	DWORD Rop;
	HBRUSH SelectedBrush, UnSelectedBrush, hOldBr;
d557 1
a557 1
	SetBkColor (hDC, ListBackgroundColor);
d570 1
a570 1
	MyColors[2] = GetBkColor (hDC);		// unseen/selected
a580 5
        SelectedBrush = CreateSolidBrush (MyBack[2]);
        UnSelectedBrush = CreateSolidBrush (MyBack[0]);
	SetTextColor (hDC, MyColors[MyColorMask]);
	SetBkColor (hDC, MyBack[MyColorMask]);
        hOldBr = SelectObject (hDC, UnSelectedBrush);
d593 2
a594 2
    GetTextExtentPoint(hDC, "s 99999 ", 8, &sz);                       
    indicatorwidth = sz.cx;                                            
a644 1

a669 4
	          if (MyColorMask >= SELECT_MASK)
	            SelectObject (hDC, SelectedBrush);
	          else
      	            SelectObject (hDC, UnSelectedBrush);
d674 3
a676 7
	  GetTextExtentPoint (hDC, scratch_line, MyLen, &sz);               
	  width = sz.cx;                                                    
	      TextOut (hDC, X, Y, scratch_line, MyLen);

	      RestX = X + width;
	      PatBlt (hDC, RestX, Y, clientRect.right - RestX - 1, LineHeight, PATCOPY);

d678 1
a678 2

	    artindex++;
d690 1
a690 4
	SetTextColor (hDC, MyColors[0]);
	SetBkColor (hDC, MyBack[0]);
        SelectObject (hDC, UnSelectedBrush);
	
a691 3
        SelectObject (hDC, hOldBr);
	DeleteObject (SelectedBrush);
	DeleteObject (UnSelectedBrush);
d1043 1
a1279 1
  long int oldindex;   
d1293 1
a1293 1
   if (articleId == NULL)   /* if retrieving artindex, only try if in range */
d1303 1
a1303 1
  if (articleId == NULL && (MyDoc = header->ArtDoc))
d1349 4
a1352 4
      if (CommDoc->ParentDoc == Doc) {
	oldindex = CommDoc->ParentOffset;       /* more slot abusage */
	(header_elt (headers,oldindex))->ArtDoc = (TypDoc *) NULL;
      }
d1360 1
a1360 1
  if (articleId == NULL)
d1433 1
a1433 1
  CommDoc->ParentOffset = (int) artindex;   /* should this be iff articleId==NULL ? */
d1437 8
a1444 8
  sprintf(mybuf, "%ld", header->number);
//InitiateReceiveArticle (Doc, mybuf);	// CN temp fix ?

   if (articleId)							// CN
      InitiateReceiveArticle (Doc, lpsz);	// CN
   else										// CN
      InitiateReceiveArticle (Doc, mybuf);

@


1.33
log
@cnolan 90.2 changes
@
text
@d42 1
a42 1
 * $Id: wvgroup.c 1.32 1994/01/24 17:40:28 jcoop Exp $
d44 3
d174 1
d722 5
d1168 15
d1453 6
a1458 1
  InitiateReceiveArticle (Doc, mybuf);
@


1.32
log
@90.2 changes
@
text
@d42 1
a42 1
 * $Id: wvgroup.c 1.31 1994/01/15 21:53:23 jcoop Exp $
d44 3
d1398 1
a1398 1
      SetClassWord (hWndArt, GCW_HBRBACKGROUND, hArticleBackgroundBrush);
@


1.31
log
@Colors/fonts, integrated decoding, general cleanup
@
text
@d42 1
a42 1
 * $Id: wvgroup.c 1.30 1993/12/27 20:16:15 jcoop Exp $
d44 3
d251 9
d1005 2
a1006 3
	        {
			DecodeDone();
	 	        CommDecoding = FALSE;
d1020 6
@


1.30
log
@copied wrong string into CurrentGroup for ST_GROUP_REJOIN
@
text
@a17 3
 *   view_article_by_message_id()     -  Views an article given     *
 *                                       only its message id        *
 *                                                                  *
d42 1
a42 1
 * $Id: wvgroup.c 1.29 1993/12/08 20:21:15 dumoulin Exp jcoop $
d44 3
d151 1
d166 3
a201 1

d443 1
a443 1
	  ViewArticle (ThisDoc, artindex, NO_REUSE, SHOW);
d494 1
a494 1
      {
d516 3
a518 2
	DWORD MyColors[4], MyBack[4];
	DWORD Rop;
d532 1
d535 1
a535 1
	SelectObject (hDC, hFont);
d543 4
a546 4
	MyColors[0] = GetTextColor (hDC);       /* black */
	MyColors[1] = GroupSeenColor;
	MyColors[2] = GetBkColor (hDC); /* white */
	MyColors[3] = MyColors[1];      /* blue  */
d548 1
a548 1
	MyBack[0] = MyColors[2];/* white */
d550 5
a554 2
	MyBack[2] = MyColors[0];
	MyBack[3] = MyColors[0];
d556 6
a640 1
		  Rop = BLACKNESS;
a644 1
		  Rop = WHITENESS;
d651 4
d662 1
d664 1
a664 1
	      PatBlt (hDC, RestX, Y, clientRect.right - RestX, LineHeight, Rop);
d682 6
a687 1
	PatBlt (hDC, 0, Y, clientRect.right, clientRect.bottom - Y, PATCOPY);
d853 19
d944 2
a945 1
	  header_p headers, header;
d952 1
a952 6
	  /* If this is the 1st article saved, then value of Append depends
	     on what user selected in dialog.  If > 1st, always append */
	  if (!MRRWriteDocument (ActiveArticleDoc,
				 sizeof (TypText),
				 SaveArtFileName,
				 (numArtsSaved==0)?SaveArtAppend:TRUE))
d954 18
a971 3
	     MessageBox (hWnd, "Could not write to file.  Operation canceled", "Problems saving file", MB_OK | MB_ICONEXCLAMATION);
	     break;
	  }
d984 1
d986 1
a986 1
     		((header = header_elt(headers,savingArtIndex))->Selected == FALSE))
d989 1
a989 1
	  if (savingArtIndex >= group->total_headers)
d992 14
a1005 2
		sprintf (str, "Saved %d article%c to file %s", numArtsSaved, (numArtsSaved==1)?' ':'s',SaveArtFileName);
		MessageBox (hWnd, str, "Done", MB_OK);
a1013 3
	  } else {
		unlock_headers (header_handle,thread_handle);
 	     	ViewArticle (ThisDoc, savingArtIndex, REUSE, NO_SHOW);
d1015 17
d1040 1
a1040 1
}
d1220 2
d1229 2
d1233 1
a1233 1
ViewArticle (Doc, artindex, Reuse, showArt)
d1238 1
a1244 3
  HANDLE hBlock;
  unsigned int Offset;
  TypLineID MyLineID;
d1250 1
a1250 1
  long int artnum,oldindex;
a1252 1
  char far *lpszGroupName;
d1263 4
a1266 3
  /* only try if in range */
  if ((artindex < 0) || (artindex >= GroupDoc->total_headers))
    return;
d1274 1
a1274 1
  if (MyDoc = header->ArtDoc)
a1275 1

a1278 1
      setArticleFocus( MyDoc->hDocWnd );
d1280 2
d1283 1
a1283 1
      else
d1290 2
a1291 10
  if (CommBusy)
    {
      MessageBox (hWndGroup,
		  "Sorry, I am already busy retrieving information from the server.\n"
		  "Try again in a little while.",
		  "Can't request text of article",
		  MB_OK | MB_ICONASTERISK);
      goto endit;
    }

d1331 5
a1335 1
  lpsz = (char far *) header->subject;
d1345 2
a1346 2
      if (xScreen > 88 * CharWidth) {
	width = 88 * CharWidth;
d1348 1
a1348 1
	width = xScreen - 1 * CharWidth;
d1352 1
a1352 1
      height = (int) (yScreen * 5 / 8) - (1 * LineHeight);
d1364 1
a1364 1
			      x - (docnum * CharWidth),
d1366 1
a1366 1
			      y + docnum * LineHeight,
d1375 1
a1375 1
      if (!hWndArt)
d1377 2
a1378 1
		
a1395 1
  setArticleFocus( hWndArt) ;
d1397 2
d1400 1
a1400 1
  else
d1404 1
a1404 1
  CommDoc->ParentOffset = (int) artindex;
a1406 3
  CommLinePtr = CommLineIn;
  CommBusy = TRUE;
  CommState = ST_ARTICLE_RESP;
d1408 2
a1410 24
  /* If we're not already in this group on the server,
   * send out a GROUP command for this window so we get back
   * into the right Group.
   */
  LockLine (Doc->hParentBlock, Doc->ParentOffset, Doc->ParentLineID,
	    &BlockPtr, &LinePtr);
  lpszGroupName = (char far *) LinePtr + sizeof (TypLine) + sizeof (TypGroup);
  if (lstrcmp (CurrentGroup, lpszGroupName))
    {
      CommState = ST_GROUP_REJOIN;
      strcpy (mybuf, "GROUP ");
      lstrcat (mybuf, lpszGroupName);
      lstrcat(mybuf,"\r");
      mylstrncpy (CurrentGroup, lpszGroupName, MAXGROUPNAME);
      PutCommLine (mybuf, lstrlen (mybuf));
    }

  UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);

  artnum = header->number;

  sprintf (mybuf, "ARTICLE %ld\r", artnum);
  PutCommLine (mybuf, lstrlen (mybuf));

a1415 1
/*  view_article_by_message_id   */
d1417 1
a1417 6
view_article_by_message_id (Doc, msg_id, artindex)
     TypDoc *Doc;
     char far *msg_id;
     long artindex;    /*  previous index not new one as in ViewArticle
			   a handle to the window rather than
			   possible index for reuse */
d1419 12
a1430 4
  TypGroup far *GroupDoc;
  BOOL newdoc;
  BOOL found;
  int docnum;
a1433 11
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
  HWND hWndArt;
  int width;
  char mybuf[MAXINTERNALLINE];
  char far *lpsz;
  HWND hWndGroup = Doc->hDocWnd;
  char far *lpszGroupName;
  HANDLE header_handle;
  HANDLE thread_handle;
  header_p headers;
a1434 106
  LockLine (Doc->hParentBlock, Doc->ParentOffset, Doc->ParentLineID,
	    &BlockPtr, &LinePtr);

  GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));

  header_handle = GroupDoc->header_handle;
  thread_handle = GroupDoc->thread_handle;
  headers = lock_headers (header_handle,thread_handle);

  if (CommBusy)
    {
      MessageBox (hWndGroup, "Sorry, I am already busy retrieving information"
		  "from the server.\nTry again in a little while.",
		  "Can't request text of article", MB_OK | MB_ICONASTERISK);
      goto endit;
    }

  newdoc = FALSE;
  if ((NewArticleWindow) || !ActiveArticleDoc || !(ActiveArticleDoc->InUse))
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
	  MessageBox (hWndGroup, "You have too many article windows \
active;\nClose one or uncheck the option \"New Window for each Article\".", "Can't open new window", MB_OK | MB_ICONASTERISK);
	  goto endit;
	}
    }
  else
    {
      /* Must reuse old window for this article.         */
      CommDoc = ActiveArticleDoc;
      /* sever the article/artindex connection */
      (header_elt (headers,CommDoc->ParentOffset))->ArtDoc = (TypDoc *) NULL;
      /* clear out old doc */
      FreeDoc (CommDoc);
    }

  header_elt (headers,artindex)->Seen = TRUE;

  InvalidateRect (hWndGroup, NULL, FALSE);

  strcpy (mybuf, "Retrieving \"");
  lstrcat (mybuf, msg_id);
  lstrcat (mybuf, "\"");

  if (newdoc)
    {
      if (xScreen > 88 * CharWidth)
	{
	  width = 88 * CharWidth;
	}
      else
	{
	  width = xScreen - 1 * CharWidth;
	}
      hWndArt = CreateWindow ("WinVnArt",
			      mybuf,
			      WS_OVERLAPPEDWINDOW | WS_VSCROLL ,
			      xScreen - (width + (docnum) * CharWidth), /* Initial X pos */
			    (int) (yScreen * 3 / 8) + (docnum) * LineHeight,    /* Initial Y pos */
			      (int) width,      /* Initial X Width */
			      (int) (yScreen * 5 / 8) - (1 * LineHeight),       /* Initial Y height */
			      NULL,
			      NULL,
			      hInst,
			      NULL);

      if (!hWndArt)
	return;                 /* ??? */

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


  setArticleFocus(hWndArt) ;
  
  (header_elt(headers,artindex))->ArtDoc = CommDoc;
  InvalidateRect (hWndArt, NULL, FALSE);
  UpdateWindow (hWndArt);

d1439 2
a1445 11
  /* WHAT - two similtaneous transmissions to the NNTP server
   * without waiting checking response.
   *
   * Looks more like corruption of ST_COMSTATE to me   mrb 7/5/93
   */

  /*  MessageBox (hWndArt, msg_id, "Debug - Article Msg Id", MB_OK); */


  LockLine (Doc->hParentBlock, Doc->ParentOffset, Doc->ParentLineID,
	    &BlockPtr, &LinePtr);
d1453 1
a1453 1
      mylstrncpy (CurrentGroup, lpsz, MAXGROUPNAME);
d1459 1
a1459 1
  sprintf (mybuf, "ARTICLE %s\r", msg_id);
a1460 3

endit:
  unlock_headers (header_handle, thread_handle);
a1462 8

void
setArticleFocus(HWND hWndArt) 
{
  SetArticleRot13Mode(hWndArt,FALSE) ;
  SetActiveWindow (hWndArt);
  SetFocus (hWndArt);
}
@


1.29
log
@Added support for using Shift-Mouse-Left inplace of Mouse-Middle and
support for dragging mouse to set article Seen flag
@
text
@d45 1
a45 1
 * $Id: wvgroup.c 1.28 1993/12/08 01:27:21 rushing Exp dumoulin $
d47 4
d1346 1
a1346 1
      mylstrncpy (CurrentGroup, lpsz, MAXGROUPNAME);
@


1.28
log
@new version box and cr lf consistency
@
text
@d45 1
a45 1
 * $Id: wvgroup.c 1.27 1993/09/08 22:37:30 rushing Exp rushing $
d47 3
d157 2
d344 1
a344 1
      /* double middle click will mark all up to here as read */
d362 2
a363 5
      X = LOWORD (lParam);
      Y = HIWORD (lParam);

      artindex = NewCursorToTextLine (X, Y, ThisDoc);
	if (artindex > -1 )
d365 3
a367 3
	  article_operation (ThisDoc, artindex, toggle_read_unread);
	  InvalidateRect(ThisDoc->hDocWnd,NULL,FALSE);	  
	}
d369 8
d380 1
a380 1
      /*  Clicking the left button on a group name toggles the
d395 10
a404 2
	     status = article_operation (ThisDoc, artindex, toggle_selected);
             DragMouseAction = status ? DRAG_SELECT : DRAG_DESELECT;
d411 1
a411 1
      /*  Letting up on the left button on a group name 
d417 7
d465 10
d1066 30
@


1.27
log
@correctly handle empty newsgroup windows destruction (cnolan)
@
text
@d42 1
d45 1
a45 1
 * $Id: wvgroup.c 1.26 1993/08/25 18:54:36 mbretherton Exp rushing $
d47 3
d853 1
d1276 1
a1276 1
      /* lstrcat(mybuf,"\r"); */
d1453 1
a1453 1
      /* lstrcat(mybuf,"\r"); */
@


1.26
log
@MRB merge, mail & post logging
@
text
@d44 1
a44 1
 * $Id: wvgroup.c 1.25 1993/08/24 17:57:22 rushing Exp $
d46 3
d245 1
d252 1
d257 2
a258 1
	if (group->total_headers != 0) {
@


1.25
log
@check for valid header element in article_operation
@
text
@d44 1
a44 1
 * $Id: wvgroup.c 1.24 1993/08/12 22:37:51 dumoulin Exp rushing $
d46 3
d165 1
a165 1
  PAINTSTRUCT ps;		/* paint structure          */
d167 2
a168 2
  HDC hDC;			/* handle to display context */
  RECT clientRect;		/* selection rectangle      */
d260 1
a260 1
	  group->thread_handle = (HANDLE) NULL;	
d338 1
a338 1
	  InvalidateRect(ThisDoc->hDocWnd,NULL,FALSE);	  
d453 1
a453 1
      	long int artnum;
d457 1
a457 1
      	header_p headers;
d460 3
a462 3
	HANDLE thread_handle;	
      	char scratch_line [MAXINTERNALLINE];
        char date_string [80];
d470 5
a474 5
         * foreground and background.
         * The ColorMask variables are indices into these arrays.
         * We set and clear bits in these indices depending upon
         * whether the article has been selected or seen.
         */
d484 2
a485 2
                     ? (ThisDoc->TotalLines - ThisDoc->TopLineOrd )
                     : ThisDoc->ScYLines);
d489 1
a489 1
	MyColors[0] = GetTextColor (hDC);	/* black */
d491 2
a492 2
	MyColors[2] = GetBkColor (hDC);	/* white */
	MyColors[3] = MyColors[1];	/* blue  */
d516 1
a516 1
        header_handle = group->header_handle;
d519 2
a520 2
      	if (header_handle)
          headers = lock_headers (header_handle,thread_handle);
d533 2
a534 2
      	if (ThisDoc->TotalLines)
      	do
d540 8
a547 8
	    	indicator = ' ';
               	if(header->Seen) {
               		indicator = 's';
               	} else if(OldHighestSeen) {
               		if(header->number > OldHighestSeen) {
               			indicator = 'n';
               		}
               	}
d549 2
a550 2
               if(ThisDoc->FindOffset == artindex)
                 indicator = '>';
d594 2
a595 2
          GetTextExtentPoint (hDC, scratch_line, MyLen, &sz);               
          width = sz.cx;                                                    
d607 2
a608 2
      	if (header_handle)
      	  unlock_headers (header_handle, thread_handle);
d707 1
a707 1
          FindDoc = ThisDoc;
d710 2
a711 2
            DialogBox (hInst, "WinVnFind", hWnd, lpfnWinVnFindDlg);
          if(strcmp(FindDoc->SearchStr,"")) {
d715 1
a715 1
	    HANDLE thread_handle;	    
d751 1
a751 1
          }
d754 1
a754 1
	case IDV_CREATE:
d756 6
a761 6
           * Most of the work is done by CreatePostingWnd and
           * CreatePostingText.  Here we have to identify
           * the newsgroup for those routines.
           * Get the newsgroup from the line in NetDoc that
           * points to this document.
           */
d862 4
a865 2
	  if (!MRRWriteDocument (ActiveArticleDoc, sizeof (TypText), SaveArtFileName, SaveArtvRef, 
	  						(numArtsSaved==0)?SaveArtAppend:TRUE))
d1099 1
a1099 1
            &BlockPtr, &LinePtr);
d1168 1
a1168 1
        oldindex = CommDoc->ParentOffset;	/* more slot abusage */
d1184 2
a1185 2
	 {
		char poschars[MAXINTERNALLINE];
d1187 17
a1203 17
		/* Compute default screen position. */
		if (xScreen > 88 * CharWidth) {
			width = 88 * CharWidth;
		} else {
			width = xScreen - 1 * CharWidth;
		}
		x = xScreen - width;
		y = (int) (yScreen * 3 / 8);
		height = (int) (yScreen * 5 / 8) - (1 * LineHeight);

		/* If the screen position has been saved, use that instead. */
		GetPrivateProfileString (szAppName, "ArticleWindowPos", "!",
		 poschars,MAXINTERNALLINE,szAppProFile);
		if(poschars[0] != '!') {
		  sscanf(poschars,"%d,%d,%d,%d",&x,&y,&width,&height);
		}
		hWndArt = CreateWindow ("WinVnArt",
d1207 1
a1207 1
					x - (docnum * CharWidth),
d1209 2
a1210 2
					y + docnum * LineHeight,
					width,	/* Initial X Width */
d1212 1
a1212 1
					height,
d1219 3
a1221 1
	return;			/* ??? */
d1236 1
a1236 1
  CommDoc->LastSeenLineID = artindex;	/* Keep an index with the article */
d1288 2
a1289 2
                           a handle to the window rather than
                           possible index for reuse */
d1311 1
a1311 1
            &BlockPtr, &LinePtr);
d1379 4
a1382 4
			      xScreen - (width + (docnum) * CharWidth),	/* Initial X pos */
			    (int) (yScreen * 3 / 8) + (docnum) * LineHeight,	/* Initial Y pos */
			      (int) width,	/* Initial X Width */
			      (int) (yScreen * 5 / 8) - (1 * LineHeight),	/* Initial Y height */
d1389 1
a1389 1
	return;			/* ??? */
d1407 1
a1407 1
  CommDoc->LastSeenLineID = artindex;	/* Keep an index with the article */
d1682 1
a1682 1
                long artindex, long num_headers)
d1690 1
a1690 1
      return (artindex);	/* return the index */
d1693 1
a1693 1
  return (-1);			/* not found */
@


1.24
log
@Added Mouse dragging to select/deselect code
@
text
@d44 1
a44 1
 * $Id: wvgroup.c 1.23 1993/08/05 20:06:07 jcoop Exp dumoulin $
d46 3
d948 1
a948 1
  BOOL result;
d957 4
a960 2
  thread_handle = GroupDoc->thread_handle;
  headers = lock_headers (header_handle,thread_handle);
d962 1
a962 1
  result = art_fun(headers, GroupDoc, artindex);
d964 3
a966 2
  unlock_headers (header_handle,thread_handle);
  UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
@


1.23
log
@save multiple articles changes by jcoop@@oakb2s01.apl.com (John S Cooper)
@
text
@d1 41
d44 1
a44 1
 * $Id: wvgroup.c 1.22 1993/07/06 21:09:50 cnolan Exp $
d46 3
a119 5
/*---  wvgroup.c ------------------------------------------- */
/*  This file contains the window procedure for the Group Viewing window
 *  for WinVN.
 */

d131 1
a131 1
void article_operation (TypDoc * Doc,
d133 1
a133 1
			void (*art_fun) (header_p headers,
d137 5
a141 3
void toggle_read_unread (header_p headers, TypGroup * group, long artindex);
void toggle_selected (header_p headers, TypGroup * group, long artindex);
void mark_read_to_here (header_p headers, TypGroup * group, long artindex);
d359 1
d362 1
d369 3
a371 2
		article_operation (ThisDoc, artindex, toggle_selected);
		InvalidateRect(ThisDoc->hDocWnd,NULL,FALSE);	  
d376 7
d394 1
d400 31
d905 1
a905 28

/* --- function CopyNonBlank ----------------------------------------
 *
 *    Copy a string, terminating at the first blank or zero byte.
 *
 *    Entry    strtarg     FWA of target area.
 *             strsource   FWA of source string.
 *             maxchars    the maximum number of characters to copy
 *                         if no blank or zero is encountered.
 *
 *    Exit     returns the number of characters copied.
 *
 *    I believe this routine is no longer used, but I'm leaving it here.
 */

int
CopyNonBlank (strtarg, strsource, maxchars)
     char *strtarg, *strsource;
     int maxchars;
{
  int j;

  for (j = 0; j < maxchars && strsource[j] != ' '; j++)
    strtarg[j] = strsource[j];
  return (j);
}

/* --- function FileLength -------------------------------------------
d929 1
a929 1
void
d932 1
a932 1
		   void (*art_fun) (header_p headers,
d945 1
a945 1

d957 1
a957 1
  art_fun(headers, GroupDoc, artindex);
d961 1
d964 1
a964 1
void
d974 1
d976 1
d979 1
a979 1
void
d993 1
d995 1
d998 1
a998 1
void
d1008 31
d1040 1
@


1.22
log
@win32 support
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.21 1993/06/28 17:52:17 rushing Exp $
d5 3
d89 2
d312 20
d341 1
a341 1
	if (artindex > -1 )
d343 1
a343 1
	  ViewArticle (ThisDoc, artindex, FALSE);
d682 35
d749 1
d761 44
d806 10
a919 1

d944 4
a947 2
  if ((artindex > -1 ) && (artindex < GroupDoc->total_headers))
    (header_elt (headers,artindex))->Selected = TRUE;
d959 1
a959 1
 *	       artindex       index into header array for this group.
d962 2
d966 1
a966 1
ViewArticle (Doc, artindex, Reuse)
d970 1
a1014 1
      /*  ShowWindow(MyDoc->hDocWnd,SW_SHOW); */
d1016 7
a1071 1

a1115 2

      ShowWindow (hWndArt, SW_SHOWNORMAL);
d1133 4
a1141 1

d1303 1
a1536 1

d1602 36
@


1.21
log
@fixed compiler warnings
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.20 1993/06/25 20:25:06 dumoulin Exp rushing $
d5 3
d330 1
d396 2
a397 1
	indicatorwidth = LOWORD (GetTextExtent (hDC, "s 99999 ", 8));
d480 2
a481 1
	      width = LOWORD (GetTextExtent (hDC, scratch_line, MyLen));
d509 1
a509 1
      switch (wParam)
d550 1
a550 1
	    switch (wParam) {
d595 1
a595 1
	  if (wParam == IDM_FIND)
d616 1
a616 1
	    starting_at = ThisDoc->TopLineOrd + ((wParam == IDM_FIND) ? 0 : 1);
@


1.20
log
@changed dates from strings to POSIX standard numeric dates
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.19 1993/06/24 17:13:14 riordan Exp dumoulin $
d5 3
d345 1
a345 1
	long int OldHighestSeen;
d1290 1
a1290 1
  long artindex;
d1377 1
a1377 1
	while ((group->nRanges < maxRanges) &&
d1428 1
a1428 1
  if (Y < TopSpace || Y > TopSpace + DocPtr->ScYLines * LineHeight ||
@


1.19
log
@Save window positions between sessions.
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.18 1993/06/22 16:43:28 bretherton Exp $
d5 3
d114 2
a115 1
  int found;
d341 1
a341 1
	char neat_from [80];
d433 1
a433 1
		       header->date,
d613 2
a614 2
	    found = search_headers (ThisDoc, headers, starting_at, group->total_headers);
	    if (found == -1)
d622 2
a623 2
	      ThisDoc->TopLineOrd = found;
	      ThisDoc->FindOffset = found;
d963 1
a963 1
		if(mybuf != '!') {
d1004 1
a1004 1
  CommDoc->ParentOffset = artindex;
a1062 1
  long int artnum;
@


1.18
log
@save one character on sprintf for date
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.17 1993/06/22 16:13:40 rushing Exp $
d5 2
d8 1
d851 1
a851 1
  int width;
d943 20
a962 10
    {
      if (xScreen > 88 * CharWidth)
	{
	  width = 88 * CharWidth;
	}
      else
	{
	  width = xScreen - 1 * CharWidth;
	}
      hWndArt = CreateWindow ("WinVnArt",
d966 1
a966 1
			      xScreen - (width + (docnum) * CharWidth),
d968 2
a969 2
			      (int) (yScreen * 3 / 8) + (docnum) * LineHeight,
			      (int) width,	/* Initial X Width */
d971 1
a971 1
			      (int) (yScreen * 5 / 8) - (1 * LineHeight),
@


1.17
log
@unsigned long in sprintf
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.16 1993/06/14 18:51:32 rushing Exp rushing $
d5 4
d423 1
a423 1
	      sprintf (scratch_line, "%c%5Flu %-6.6Fs %-18.18Fs %4Fd %-*s%-Fs ",
a432 2
	      /* keep global variable tidy */
	      NameString[0] = AddressString[0] = '\0' ;
@


1.16
log
@better handling of WM_DESTROY to 0-article group window
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.15 1993/06/11 01:10:21 rushing Exp rushing $
d5 3
d419 1
a419 1
	      sprintf (scratch_line, "%c%5u %-6.6Fs %-18.18Fs %4Fd %-*s%-Fs ",
d1172 1
a1172 1
  MessageBox (hWndArt, msg_id, "Debug - Article Msg Id", MB_OK);
@


1.15
log
@fixed boneheaded CommDoc bug again
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.14 1993/06/11 00:11:26 rushing Exp rushing $
d5 3
a159 13
      UpdateSeenArts (ThisDoc);
      UnlinkArtsInGroup (ThisDoc);
      ThisDoc->InUse = FALSE;
      if (ThisDoc == CommDoc)
	{
	  CommBusy = FALSE;
	  CommDoc = (TypDoc *) NULL;
	}
      /* Clear the pointer in the line for this group in the   */
      /* NetDoc document.  This pointer currently points       */
      /* to the current document, which we are wiping out      */
      /* with the destruction of this window.                  */

d162 1
d164 4
d169 1
a169 1
         (TypGroup far *) (((char far *) LinePtr) + sizeof (TypLine));
d172 14
a185 3
        /* Free the header table */
	if (group->header_handle)
	  free_headers (group->header_handle, group->thread_handle);
d187 4
a190 3
	group->header_handle = (HANDLE) NULL;
	group->thread_handle = (HANDLE) NULL;	
	group->total_headers = 0;
@


1.14
log
@second merge from Matt Bretherton sources
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.13 1993/06/10 18:27:18 rushing Exp $
d5 2
d8 1
d517 1
a517 1
	    for (i=0;i<CommDoc->TotalLines;i++)
@


1.13
log
@fixed sort logic and range-writing
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.12 1993/06/08 19:39:45 rushing Exp rushing $
d5 4
d139 1
a139 1
   case WM_CREATE:
d406 1
a406 2

	      sprintf (scratch_line, "%c%5u %-6.6Fs %18.18Fs %4Fd %-*s%-Fs ",
a766 2
  header_p header = header_elt (headers, artindex);

d768 2
a769 1
  if ((artindex > -1 ) && (artindex < GroupDoc->total_headers))
d771 1
d849 4
a858 4
  /* only try if in range */
  if ((artindex < 0) || (artindex >= GroupDoc->total_headers))
    return;

a1008 1

@


1.12
log
@added the Sort... menu
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.11 1993/06/07 21:36:22 rushing Exp rushing $
d5 3
d519 1
a519 1
				      CommDoc->TotalLines, compare_date);
d524 1
a524 1
				      CommDoc->TotalLines, compare_subject);
d529 1
a529 1
				      CommDoc->TotalLines, compare_lines);
d534 1
a534 1
		sort_by_threads (header_handle, thread_handle, CommDoc->TotalLines);
d542 1
a542 1
				      CommDoc->TotalLines, compare_artnum);
d547 1
a547 1
				      CommDoc->TotalLines, compare_from);
d1271 9
a1279 3
  header_handle = group->header_handle;
  thread_handle = group->thread_handle;
  headers = lock_headers (header_handle,thread_handle);
d1281 2
a1282 1
  group->nRanges = 0;
d1284 3
a1286 2
  maxRanges = ((Doc->BlockSize - Doc->SplitSize) - ParentLine->length +
	       group->nRanges * sizeof (TypRange)) / sizeof (TypRange) - 1;
d1288 5
a1292 9
  RangePtr = (TypRange *) ((char *) LocalLinePtr + sizeof (TypLine) +
			   RangeOffset (group->NameLen));
  MyRange.First = 1;

  /* Get the first line in this document.
   * If it cannot be found, just set Last=First and skip the
   * proceeding processing.  Otherwise, assume we've seen everything
   * up to but not including the first article in the document.
   */
d1294 1
a1294 1
/*  LockLine (Doc->hFirstBlock, sizeof (TypBlock), 0L, &BlockPtr, &LinePtr); */
d1296 9
a1304 9
  artindex = 0;
  if (!Doc->TotalLines)
    {
      MyRange.Last = 1;
    }
  else
    {
      header = (header_elt (headers,artindex));
      MyRange.Last = header->number - 1;
d1306 35
a1340 34
      /* Loop to scan through the document, fabricating article ranges.
       */
      do
	{
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
d1344 8
a1351 6
      if (InSeen)
	{
	  *(RangePtr++) = MyRange;
	  (group->nRanges)++;
	}
    }
@


1.11
log
@fixed subject searches
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.10 1993/06/05 03:18:04 rushing Exp rushing $
d5 3
d479 72
@


1.10
log
@primitive functional threading.
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.9 1993/06/01 18:23:04 rushing Exp rushing $
d5 3
d51 1
a51 1
LPSTR string_compare_insensitive (LPSTR a, LPSTR b);
d497 1
a497 1
	    header_handle = group->thread_handle;
d1327 2
d1330 2
a1331 2
    if (string_compare_insensitive ((header_elt(headers,artindex))->subject,
				    HeaderDoc->SearchStr))
d1338 2
a1339 2
LPSTR
string_compare_insensitive (LPSTR a, LPSTR b)
d1346 1
a1346 1
   if (_fstrnicmp (a,b,lenb) == 0)
@


1.9
log
@threading support
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.8 1993/05/27 00:00:09 rushing Exp rushing $
d5 3
d394 1
a394 1
	      sprintf (scratch_line, "%c%5u %-6.6Fs %8.8Fs %8.8Fs %18.18Fs %4Fd %-Fs ",
a397 2
		       header->message_id,
		       header->references,
d400 3
a402 1
		       header->subject);
d968 1
a968 1
		  "from the server.\n\Try again in a little while.",
@


1.8
log
@added support for 'mark article as read', and 'catch up'
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.7 1993/05/25 00:00:45 rushing Exp rushing $
d5 3
d44 1
a44 1
long search_headers (TypDoc * HeaderDoc, TypHeader huge *headers, long artindex, long num_headers);
d48 1
a48 1
			void (*art_fun) (TypHeader huge * headers,
d52 3
a54 3
void toggle_read_unread (TypHeader huge * headers, TypGroup * group, long artindex);
void toggle_selected (TypHeader huge * headers, TypGroup * group, long artindex);
void mark_read_to_here (TypHeader huge * headers, TypGroup * group, long artindex);
d151 2
a152 1
      LockLine (ThisDoc->hParentBlock, ThisDoc->ParentOffset, ThisDoc->ParentLineID, &BlockPtr, &LinePtr);
d160 2
a161 1
          GlobalFree (group->header_handle);
d163 1
d296 2
a297 1
      	TypHeader huge *headers;
d299 1
d355 1
d358 1
a358 1
          headers = (TypHeader huge *) (GlobalLock (header_handle) + sizeof(char huge *));
d374 3
a376 1
	      artnum = headers[artindex].number;
d379 1
a379 1
               	if(headers[artindex].Seen) {
d382 1
a382 1
               		if(headers[artindex].number > OldHighestSeen) {
d391 1
a391 1
	      sprintf (scratch_line, "%c%5u %-6.6Fs %18.18Fs %4Fd %-Fs ",
d393 7
a399 5
		       headers[artindex].number,
		       headers[artindex].date,
		       headers[artindex].from,
		       headers[artindex].lines,
		       headers[artindex].subject);
d408 1
a408 1
	      if (headers[artindex].Seen)
d416 1
a416 1
	      if (headers[artindex].Selected)
d448 1
a448 1
      	  GlobalUnlock (header_handle);
d481 1
a481 1
	    TypHeader huge *headers;
d483 1
d491 3
a493 1
	    headers = (TypHeader huge *) (GlobalLock (header_handle) + sizeof(char huge *));
d518 1
a518 1
            GlobalUnlock (header_handle);
d558 2
a559 1
	      GlobalFree (group->header_handle);
d561 2
a562 1

d643 3
a645 3
		   void (*art_fun) (TypHeader huge * headers,
			       TypGroup * group,
			       long artindex))
d655 3
a657 1
  TypHeader huge * headers;
d665 2
a666 1
  headers = (TypHeader huge *) (GlobalLock (header_handle) + sizeof(char huge *)) ;
d670 1
a670 1
  GlobalUnlock (header_handle);
d675 1
a675 1
toggle_read_unread (TypHeader huge * headers,
d680 2
d684 1
a684 1
    headers[artindex].Seen = ( headers[artindex].Seen ? FALSE : TRUE );
d689 1
a689 1
mark_read_to_here (TypHeader huge * headers,
d693 1
d698 4
a701 2
    for (i = 0; i <= artindex; i++)
      headers[i].Seen = TRUE;
a704 2


d706 1
a706 1
toggle_selected (TypHeader huge * headers,
d710 1
d713 1
a713 1
    headers[artindex].Selected = TRUE;
a716 1

d753 3
a755 1
  TypHeader huge * headers;
d763 1
a763 1
  headers = (TypHeader huge *) (GlobalLock (header_handle) + sizeof(char huge *)) ;
d765 3
d772 1
a772 1
  if (MyDoc = headers[artindex].ArtDoc)
d784 5
a788 2
      MessageBox (hWndGroup, "Sorry, I am already busy retrieving information from the server.\n\
Try again in a little while.", "Can't request text of article", MB_OK | MB_ICONASTERISK);
d808 5
a812 2
	  MessageBox (hWndGroup, "You have too many article windows \
active;\nClose one or uncheck the option \"New Window for each Article\".", "Can't open new window", MB_OK | MB_ICONASTERISK);
d822 1
a822 1
	headers[oldindex].ArtDoc = (TypDoc *) NULL;
d828 1
a828 1
  headers[artindex].Seen = TRUE;
d832 1
a832 1
  lpsz = (char far *) headers[artindex].subject;
d850 4
a853 2
			      xScreen - (width + (docnum) * CharWidth),	/* Initial X pos */
			    (int) (yScreen * 3 / 8) + (docnum) * LineHeight,	/* Initial Y pos */
d855 2
a856 1
			      (int) (yScreen * 5 / 8) - (1 * LineHeight),	/* Initial Y height */
d884 1
a884 1
  headers[artindex].ArtDoc = CommDoc;
d913 1
a913 1
  artnum = headers[artindex].number;
d918 3
a920 2
endit:
  GlobalUnlock (header_handle);
d923 1
d950 2
a951 1
  TypHeader huge * headers;
d959 2
a960 1
  headers = (TypHeader huge *) (GlobalLock (header_handle) + sizeof(char huge *)) ;
d964 3
a966 2
      MessageBox (hWndGroup, "Sorry, I am already busy retrieving information from the server.\n\
Try again in a little while.", "Can't request text of article", MB_OK | MB_ICONASTERISK);
d996 1
a996 1
      headers[CommDoc->ParentOffset].ArtDoc = (TypDoc *) NULL;
d1001 1
a1001 1
  headers[artindex].Seen = TRUE;
d1054 1
a1054 1
  headers[artindex].ArtDoc = CommDoc;
d1095 1
a1095 1
  GlobalUnlock (header_handle);
d1163 1
a1163 1
  HANDLE hLine,header_handle;
d1165 2
a1166 1
  TypHeader huge * headers;
d1188 2
a1189 1
  headers = (TypHeader huge *) (GlobalLock (header_handle) + sizeof (char huge *));
d1215 2
a1216 1
      MyRange.Last = headers[artindex].number - 1;
d1222 1
a1222 1
	  if (headers[artindex].Seen)
d1227 1
a1227 1
		  MyRange.Last = headers[artindex].number;
d1232 2
a1233 2
		  MyRange.First = headers[artindex].number;
		  MyRange.Last = headers[artindex].number;
d1262 1
a1262 2
	MyLength = sizeof (TypLine) + RangeOffset (group->NameLen) +
           sizeof (TypRange) * (group->nRanges) + sizeof (int);
d1264 3
d1314 2
d1317 2
a1318 4
search_headers (TypDoc * HeaderDoc,
                TypHeader huge *headers,
                long artindex,
                long num_headers)
d1321 5
a1325 5
	do {
	if (string_compare_insensitive (headers[artindex].subject,
                                        HeaderDoc->SearchStr))
	       return (artindex);		/* return the index */
	} while (artindex++ < (num_headers - 1));
d1327 1
a1327 1
	return (-1);			/* not found */
@


1.7
log
@rot13 merge (MRB)
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.6 1993/05/19 23:48:36 rushing Exp $
d5 3
d43 9
a51 2


d224 29
a252 5
    case WM_LBUTTONDOWN:
      /*  Clicking the left button on an article name toggles the
       *  selected/not selected status of that article.
       *  Currently selected articles are displayed in reverse video.
       */
d622 71
d883 1
@


1.6
log
@range check in ViewArticle (artindex)
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.5 1993/05/13 20:01:02 SOMEONE Exp rushing $
d5 3
d36 1
d345 2
a346 1
	      sprintf (scratch_line, "%c %u %6.6Fs %20.20Fs %4Fd %Fs ",
d354 3
d605 1
a605 1
{
d645 1
a645 2
      SetActiveWindow (MyDoc->hDocWnd);
      SetFocus (MyDoc->hDocWnd);
d682 1
a682 1
        headers[oldindex].ArtDoc = (TypDoc *) NULL;
d739 1
a739 2
  SetActiveWindow (hWndArt);
  SetFocus (hWndArt);
d776 1
a776 1
	GlobalUnlock (header_handle);
d778 1
a778 1

d780 1
a780 1
view_article_by_message_id (Doc, article_request, artindex)
d782 4
a785 2
     char far * article_request;
     long artindex;
d857 1
a857 1
  lstrcat (mybuf, article_request);
d903 2
a904 3
  SetActiveWindow (hWndArt);
  SetFocus (hWndArt);

d918 9
d942 1
a942 3
  artnum = headers[artindex].number;

  lstrcpy (mybuf, article_request);
d946 10
a955 1
	GlobalUnlock (header_handle);
@


1.5
log
@fancy 'from' in group window
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.4 1993/05/06 20:58:15 rushing Exp SOMEONE $
d5 3
d625 4
@


1.4
log
@call UpdateSeenArts before processing the IDM_UPDATE: message
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.3 1993/05/01 01:38:17 rushing Exp rushing $
d5 3
d34 2
d252 1
d338 7
a344 7
	    	sprintf (scratch_line, "%c %u %6.6Fs %20.20Fs %4Fd %Fs ",
	    			indicator,
			    	headers[artindex].number,
			    	headers[artindex].date,
			    	headers[artindex].from,
				headers[artindex].lines,
			    	headers[artindex].subject);
@


1.3
log
@added 'update' feature.  doesn't copy headers yet.
@
text
@d3 1
a3 1
 * $Id: wvgroup.c 1.2 1993/03/30 20:05:39 rushing Exp rushing $
d5 3
d481 2
@


1.2
log
@MAPI
@
text
@d3 5
a7 2
 * $Id: wvgroup.c%v 1.1 1993/02/16 20:53:50 rushing Exp $
 * $Log: wvgroup.c%v $
d413 34
a446 20
          TypGroup far * group;
          TypHeader huge *headers;
          HANDLE header_handle;
          int starting_at;

          LockLine (ThisDoc->hParentBlock, ThisDoc->ParentOffset, ThisDoc->ParentLineID,
		    &BlockPtr, &LinePtr);
          group = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));

          header_handle = group->header_handle;
          headers = (TypHeader huge *) (GlobalLock (header_handle) + sizeof(char huge *));

          UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);

          /* 'Find Next' will start one line after */
          starting_at = ThisDoc->TopLineOrd + ((wParam == IDM_FIND) ? 0 : 1);

          /* back up one if we're at the end */
          if (starting_at >= group->total_headers)
            starting_at--;
a447 14
          found = search_headers (ThisDoc, headers, starting_at, group->total_headers);
 	      if (found == -1)
		{
		  strcpy (mybuf, "\"");
		  strcat (mybuf, ThisDoc->SearchStr);
		  strcat (mybuf, "\" not found.");
		  MessageBox (hWnd, mybuf, "Not found", MB_OK);
		}
               else {
                 ThisDoc->TopLineOrd = found;
                 ThisDoc->FindOffset = found;
                 InvalidateRect(ThisDoc->hDocWnd,NULL,FALSE);
               }

d472 34
a505 1
	}
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
d92 4
d463 5
a467 1
	  CreatePostingWnd ((TypDoc *) NULL, DOCTYPE_POSTING);
@
