head	1.33;
access;
symbols
	V80:1.8
	V76d:1.1;
locks; strict;
comment	@ * @;


1.33
date	94.09.22.00.09.56;	author jcooper;	state Exp;
branches;
next	1.32;

1.32
date	94.09.18.22.48.51;	author jcooper;	state Exp;
branches;
next	1.31;

1.31
date	94.09.16.00.53.16;	author jcooper;	state Exp;
branches;
next	1.30;

1.30
date	94.08.24.17.59.26;	author jcooper;	state Exp;
branches;
next	1.29;

1.29
date	94.08.11.00.18.04;	author jcooper;	state Exp;
branches;
next	1.28;

1.28
date	94.08.03.00.54.10;	author gardnerd;	state Exp;
branches;
next	1.27;

1.27
date	94.07.27.21.14.38;	author gardnerd;	state Exp;
branches;
next	1.26;

1.26
date	94.07.12.19.16.13;	author rushing;	state Exp;
branches;
next	1.25;

1.25
date	94.07.01.03.45.53;	author dumoulin;	state Exp;
branches;
next	1.24;

1.24
date	94.06.30.21.30.22;	author gardnerd;	state Exp;
branches;
next	1.23;

1.23
date	94.06.23.23.07.13;	author dumoulin;	state Exp;
branches;
next	1.22;

1.22
date	94.06.01.19.01.51;	author gardnerd;	state Exp;
branches;
next	1.21;

1.21
date	94.06.01.18.50.49;	author rushing;	state Exp;
branches;
next	1.20;

1.20
date	94.05.27.01.29.29;	author rushing;	state Exp;
branches;
next	1.19;

1.19
date	94.05.23.18.36.00;	author jcooper;	state Exp;
branches;
next	1.18;

1.18
date	94.02.24.21.26.54;	author jcoop;	state Exp;
branches;
next	1.17;

1.17
date	94.01.24.17.41.01;	author jcoop;	state Exp;
branches;
next	1.16;

1.16
date	94.01.16.12.23.43;	author jcoop;	state Exp;
branches;
next	1.15;

1.15
date	93.12.08.01.28.38;	author rushing;	state Exp;
branches;
next	1.14;

1.14
date	93.08.25.18.54.36;	author mbretherton;	state Exp;
branches;
next	1.13;

1.13
date	93.08.05.20.06.07;	author jcoop;	state Exp;
branches;
next	1.12;

1.12
date	93.07.06.21.09.50;	author cnolan;	state Exp;
branches;
next	1.11;

1.11
date	93.06.26.00.53.33;	author rushing;	state Exp;
branches;
next	1.10;

1.10
date	93.06.26.00.18.46;	author rushing;	state Exp;
branches;
next	1.9;

1.9
date	93.06.22.16.45.00;	author bretherton;	state Exp;
branches;
next	1.8;

1.8
date	93.06.11.00.11.26;	author rushing;	state Exp;
branches;
next	1.7;

1.7
date	93.05.29.03.32.30;	author rushing;	state Exp;
branches;
next	1.6;

1.6
date	93.05.28.18.23.40;	author rushing;	state Exp;
branches;
next	1.5;

1.5
date	93.05.24.23.28.24;	author rushing;	state Exp;
branches;
next	1.4;

1.4
date	93.05.18.22.11.40;	author rushing;	state Exp;
branches;
next	1.3;

1.3
date	93.05.06.19.43.41;	author rushing;	state Exp;
branches;
next	1.2;

1.2
date	93.03.30.20.08.25;	author rushing;	state Exp;
branches;
next	1.1;

1.1
date	93.02.16.20.53.50;	author rushing;	state Exp;
branches;
next	;


desc
@winvn version 0.76 placed into RCS
@


1.33
log
@Check if text is selected before allowing RMB click to copy
@
text
@/*
 *
 * $Id: wvart.c 1.33 1994/09/19 16:06:27 jcooper Exp $
 * $Log: wvart.c $
 * Revision 1.33  1994/09/19  16:06:27  jcooper
 * Check if text is selected before allowing RMB click to copy
 * 
 * Revision 1.32  1994/09/18  22:48:51  jcooper
 * support for cascading windows
 * 
 * Revision 1.31  1994/09/16  00:53:16  jcooper
 * massive cleanup for 92.6
 * 
 * Revision 1.30  1994/08/24  17:59:26  jcooper
 * misc encoding/decoding changes
 *
 * Revision 1.29  1994/08/11  00:18:04  jcooper
 * added support for article retreive complete message
 *
 * Revision 1.28  1994/08/03  00:54:10  gardnerd
 * Fixed bug with cut to clipboard at 1st and last line of text
 *
 * Revision 1.27  1994/07/27  21:14:38  gardnerd
 * copy to clipboard
 *
 * Revision 1.26  1994/07/12  19:16:13  rushing
 * missing curlies in CloseArtWnd()
 *
 * Revision 1.25  1994/07/01  03:45:53  dumoulin
 * Added the Find Next Unseen and concept of SPACE as a "power key"
 *
 * Revision 1.24  1994/06/30  21:30:22  gardnerd
 * Allow scrolling by pixels instead of characters
 *
 * Revision 1.23  1994/06/23  23:07:13  dumoulin
 * added menu item for Printer Setup
 *
 * Revision 1.22  1994/06/01  19:01:51  gardnerd
 * horizontal scrolling support
 *
 * Revision 1.21  1994/06/01  18:50:49  rushing
 * more f3 bugs
 *
 * Revision 1.20  1994/05/27  01:29:29  rushing
 * unnecessary winundoc.h
 *
 * Revision 1.19  1994/05/23  18:36:00  jcooper
 * new attach code, session [dis]connect
 *
 * Revision 1.18  1994/02/24  21:26:54  jcoop
 * jcoop changes
 *
 * Revision 1.17  1994/01/24  17:41:01  jcoop
 * 90.2 changes
 *
 * Revision 1.16  1994/01/16  12:23:43  jcoop
 * New font/color painting.  no longer use view_art_by_id
 *
 * Revision 1.15  1993/12/08  01:28:38  rushing
 * new version box and cr lf consistency
 * 
 * Revision 1.14  1993/08/25  18:54:36  mbretherton
 * MRB merge, mail & post logging
 *
 *
 * Revision 1.1x  1993/07/21 MBretherton
 *                remove vRef from MRROpenFile (Mac Stuff)
 *
 * Revision 1.13  1993/08/05  20:06:07  jcoop
 * save multiple articles changes by jcoop@@oakb2s01.apl.com (John S Cooper)
 *
 * Revision 1.12  1993/07/06  21:09:50  cnolan
 * win32 support
 *
 * Revision 1.11  1993/06/26  00:53:33  rushing
 * warnings
 *
 * Revision 1.10  1993/06/26  00:18:46  rushing
 * warnings
 *
 * Revision 1.9  1993/06/22  16:45:00  bretherton
 * *** empty log message ***
 *
 * Revision 1.xx M Bretherton  Fix
 *               Mail != Forward in terms of underlying mail support  
 * Revision 1.8  1993/06/11  00:11:26  rushing
 * second merge from Matt Bretherton sources
 *
 * Revision 1.7  1993/05/29  03:32:30  rushing
 * threading support
 *
 * Revision 1.6  1993/05/28  18:23:40  rushing
 * fixed far/huge problem in find_art_by_subject
 *
 * Revision 1.5  1993/05/24  23:28:24  rushing
 * header formatting (MRB)
 *
 *
 * Revision 1.4  1993/05/18  22:11:40  rushing
 * smtp support
 *
 * Revision 1.3  1993/05/06  19:43:41  rushing
 * added Re: fix from MB
 *
 * Revision 1.2  1993/03/30  20:08:25  rushing
 * MAPI
 *
 * Revision 1.1  1993/02/16  20:53:50  rushing
 * Initial revision
 *
 *
 */

/*---  wvart.c ------------------------------------------- */
/*  This file contains the window procedure for the Article Viewing window
 *  for WinVn.
 */

#include <windows.h>
#include <windowsx.h>
#include "wvglob.h"
#include "winvn.h"
#pragma hdrstop
#include <ctype.h>

int cursor_to_char_number (int X, int Y,TypDoc *DocPtr,TypBlock far **BlockPtr,TypLine far **LinePtr);
void CloseArtWnd(HWND hWnd, TypDoc *ThisDoc);
void UpdateHighlightedText(TypDoc far *DocPtr, TextSelect far *PreviousPos);
void HighlightText(TypDoc far *DocPtrs);
void CopyTextToClipBoard(TypDoc FAR *DocPtr);
void SetArticleMenus (HWND hWnd, int enable);
  
/************** find_article_geader_subject *************
 *
 *   Amended to ignore Re: in either heading line as part of match
 *   criteria
 *
 *  Note the duplication of Knowledge of the rules of subject header
 *  format with wvheader : GetArticleSubject
 *
 *  Ideally some object containing all knowledge of rules of format
 *  would be a more elegant implementation
 *  Also note heading changes with each find next of same heading
 *  request. Ideally want to maintain first heading in series
 *  allowing a more liberal interpreation of 'same' in same heading
 *
 *                       Matthew Bretherton 2nd March 1993
 *
 ********************************************************/

long find_article_by_subject (header_p headers,
			      long artindex,
			      long num_headers)
{
  char far *nextsub ;
  char far *thissub;

  thissub = (header_elt(headers,artindex++))->subject;
  if (CompareStringNoCase (thissub, "Re:", 3) == 0) thissub=thissub + 4;

  if (artindex >= num_headers) return (-1);

  do {
    nextsub = (header_elt(headers,artindex))->subject ;
    if (CompareStringNoCase (nextsub, "Re:", 3) == 0) nextsub=nextsub + 4;
    if (lstrcmpi (nextsub, thissub) == 0)
      return (artindex);        /* return the index */
  } while (artindex++ < num_headers);

  return (-1);                  /* not found */
}
 

/****************************************************************************

    FUNCTION: WinVnArtWndProc(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages

****************************************************************************/

long WINAPI WinVnArtWndProc (hWnd, message, wParam, lParam)
     HWND hWnd;
     unsigned message;
     WPARAM wParam;
     LPARAM lParam;
{
  PAINTSTRUCT ps;               /* paint structure          */

  HDC hDC;                      /* handle to display context */
  RECT myRect;                  /* selection rectangle      */
  TypDoc *ThisDoc;
  TypGroup far *GroupDoc;
  int ih, j;
  int found, X, Y, charnum;
  int CtrlState;
  TypLine far *LinePtr;
  TypBlock far *BlockPtr;
  HANDLE hBlock;
  char mybuf[MAXINTERNALLINE];
  unsigned int Offset;
  char far *textptr;
  int textlen;
  BOOL continueFind;
  TypLineID MyLineID,artindex;
  HANDLE header_handle;
  HANDLE thread_handle;
  header_p headers;
  header_p header;
  long header_num;
     
  for (ih = 0, found = FALSE; !found && ih < MAXARTICLEWNDS; ih++)
    {
      if (ArticleDocs[ih].hDocWnd == hWnd)
	{
	  found = TRUE;
	  ThisDoc = &(ArticleDocs[ih]);
	}
    }

  if (!found)
    {
      ThisDoc = CommDoc;
    }


  switch (message)
    {

    case WM_CREATE:
      NumArticleWnds++;
      SetArticleMenus(hWnd, DISABLE);
      break;
      
    case WM_ACTIVATE:
    case WM_SETFOCUS:
      if (wParam)
	{
	  ActiveArticleDoc = ThisDoc;
	}
      /* fall through */
    case WM_SYSCOMMAND:
      return (DefWindowProc (hWnd, message, wParam, lParam));

    case WM_SIZE:
      GetClientRect (hWnd, &myRect);
      ThisDoc->ScXWidth = myRect.right;
      ThisDoc->ScYHeight = myRect.bottom;
      ThisDoc->ScYLines = (myRect.bottom - myRect.top - ArtTopSpace) / ArtLineHeight;
      ThisDoc->ScXChars = (myRect.right - myRect.left - ArtSideSpace) / ArtCharWidth;
      break;

    case WM_CLOSE:
      ThisDoc->TextSelected = FALSE;
      CloseArtWnd(hWnd, ThisDoc);
      break;
      
    case WM_DESTROY:
      NumArticleWnds--;
      ThisDoc->TextSelected = FALSE;
      ThisDoc->InUse = FALSE;
      ThisDoc->LongestLine = 0;
      ThisDoc->ScXOffset = 0;
      if (ThisDoc == CommDoc)
	{
	  CommBusy = FALSE;
	  CommDoc = (TypDoc *) NULL;
	}
      /* Clear the pointer in the line for this article in the   */
      /* group  document.  This pointer currently points       */
      /* to the current document, which we are wiping out      */
      /* with the destruction of this window.                  */

      if (ThisDoc->ParentDoc)
	{
	LockLine (ThisDoc->ParentDoc->hParentBlock,
		  ThisDoc->ParentDoc->ParentOffset,
		  ThisDoc->ParentDoc->ParentLineID,
		  &BlockPtr, &LinePtr);

	GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));

	header_handle = GroupDoc->header_handle;
	thread_handle = GroupDoc->thread_handle;        
	headers = lock_headers (header_handle,thread_handle);

	(header_elt(headers,ThisDoc->LastSeenLineID))->ArtDoc = (TypDoc *) NULL;
	UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
	unlock_headers (header_handle, thread_handle);
	}
      /* Clear document                                        */
      FreeDoc (ThisDoc);

      /* If there's another article window, make it the active   */
      /* artcile window so we don't create a new one if the      */
      /* New Article flag is FALSE.                              */

      for (j = MAXARTICLEWNDS - 1; j >= 0; j--)
	{
	  if (ArticleDocs[j].InUse)
	    {
	      ActiveArticleDoc = &(ArticleDocs[j]);
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
        if (wParam == VK_SPACE)
         {   
           if (ThisDoc->ActiveLines > (ThisDoc->ScYLines + ThisDoc->TopLineOrd))
            {
             SendMessage (hWnd,(UINT) WM_KEYDOWN, (WPARAM) VK_NEXT, (LPARAM) 0L);
             break; 
            }
           else 
             { 
              if (!(CommBusy && ThisDoc == CommDoc))
                SendMessage (hWnd,(UINT) WM_COMMAND, (WPARAM) IDM_FIND_NEXT_UNSEEN, (LPARAM) 0L); 
              break;
              }
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
      
     case WM_RBUTTONDOWN:
       if (ThisDoc->TextSelected) {
          CopyTextToClipBoard(ThisDoc);
          SendMessage (hWnd, (UINT) WM_COMMAND, (WPARAM) IDM_DESELECT_ALL, (LPARAM) 0L);  //Un-mark selection
	   }
       break;


    case WM_LBUTTONDOWN: 
    {
      int TempX, TempY;
                 
      if (CommBusy && CommDoc == ThisDoc)
      {
        MessageBox (hWnd, "Please wait until article retrieval is complete before selecting text.", "Please Wait", MB_OK);
      }
      else	
      {
        DragMouseAction = DRAG_SELECT; 
        ThisDoc->TextSelected = FALSE; 
        EnableMenuItem(GetMenu(hWnd),IDM_COPY,MF_GRAYED) ;
        EnableMenuItem(GetMenu(hWnd),IDM_DESELECT_ALL,MF_GRAYED) ;
        X = LOWORD (lParam);
        Y = HIWORD (lParam);
  
        if (X < ArtSideSpace)
        {
          TempX = ArtSideSpace;
        }
        else
        {
          TempX = min(X, (int)ThisDoc->ScXWidth);
        }
  
        if (Y < ArtTopSpace)
        {
          TempY = ArtTopSpace;
        }
        else
        { 
          TempY = min(Y, (int)(ArtTopSpace + (ThisDoc->ScYLines-1) * ArtLineHeight));
          if(TempY > (int)(ArtTopSpace + ((int)ThisDoc->TotalLines - (int)ThisDoc->TopLineOrd - 1) * ArtLineHeight))
          {
            TempY = (int)ArtTopSpace + ((int)ThisDoc->TotalLines - (int)ThisDoc->TopLineOrd - 1) * (int)ArtLineHeight;
          }  
        }
  
        ThisDoc->BeginSelect.LineNum = (TempY - ArtTopSpace) / ArtLineHeight + ThisDoc->TopLineOrd;
        ThisDoc->BeginSelect.CharNum = cursor_to_char_number (TempX,TempY,ThisDoc,&BlockPtr,&LinePtr);
        ThisDoc->EndSelect.LineNum = -1; 
        SetRect(&myRect, 0, 0, ThisDoc->ScXWidth, ThisDoc->ScYHeight);   
        InvalidateRect(ThisDoc->hDocWnd, &myRect, FALSE);
        SetCursor(LoadCursor(NULL, IDC_IBEAM));
        SetCapture(hWnd);
      }
    }    
    break;  
              
    case WM_MOUSEMOVE:
    {
      int X, Y, TempX, TempY;
      TextSelect Previous;
      int ScMin, ScMax;
      POINT CursorLocation;
      MSG Message;
      BOOL ScrollText = FALSE;
      void CheckForUpdate();      
                 
      if(DragMouseAction == DRAG_SELECT)
      {  
        ThisDoc->TextSelected = TRUE;
        EnableMenuItem(GetMenu(hWnd),IDM_COPY,MF_ENABLED) ;
        EnableMenuItem(GetMenu(hWnd),IDM_DESELECT_ALL,MF_ENABLED) ;
        X = LOWORD (lParam);
        Y = HIWORD (lParam);
        
        // Check to see if text selection needs to scroll left.
        if((X < 0) && (ThisDoc->ScXOffset > 0))
        { 
          ScrollText = TRUE;

          do // keep scrolling until the mouse moves.
          { 
            TempX = ArtSideSpace;
            if (Y < ArtTopSpace)
            {
              TempY = ArtTopSpace;
            }
            else
            {
              TempY = min(Y, (int)(ArtTopSpace + (ThisDoc->ScYLines-1) * ArtLineHeight));
              if(TempY > (int)(ArtTopSpace + ((int)ThisDoc->TotalLines - (int)ThisDoc->TopLineOrd - 1) * ArtLineHeight))
              {
                TempY = (int)ArtTopSpace + ((int)ThisDoc->TotalLines - (int)ThisDoc->TopLineOrd - 1) * (int)ArtLineHeight;
              }  
            }
            
            Previous.LineNum = ThisDoc->EndSelect.LineNum;
            Previous.CharNum = ThisDoc->EndSelect.CharNum;
            SendMessage(ThisDoc->hDocWnd, WM_HSCROLL, SB_LINEUP, 0);
            ThisDoc->EndSelect.LineNum = (TempY - (int)ArtTopSpace) / (int)ArtLineHeight + ThisDoc->TopLineOrd; 
            ThisDoc->EndSelect.CharNum = cursor_to_char_number (TempX,TempY,ThisDoc,&BlockPtr,&LinePtr);
            UpdateHighlightedText(ThisDoc, &Previous);                       
            SendMessage(ThisDoc->hDocWnd, WM_PAINT, 0, 0);
            if(PeekMessage(&Message, ThisDoc->hDocWnd, WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE))
              break;
            GetCursorPos(&CursorLocation);
            ScreenToClient(ThisDoc->hDocWnd, &CursorLocation);
          }
          while((X == CursorLocation.x) && (Y == CursorLocation.y) &&
                (ThisDoc->ScXOffset > 0));
        }
        
        GetScrollRange(ThisDoc->hDocWnd, SB_HORZ, &ScMin, &ScMax);
        // Check to see if text selection needs to scroll right.
        if((X > (int)ThisDoc->ScXWidth) && ((int)ThisDoc->ScXOffset < ScMax))
        { 
          ScrollText = TRUE;
            
          do // keep scrolling until the mouse moves.
          { 
            TempX = ThisDoc->ScXWidth;
            if (Y < ArtTopSpace)
            {
              TempY = ArtTopSpace;
            }
            else
            {
              TempY = min(Y, (int)(ArtTopSpace + (ThisDoc->ScYLines-1) * ArtLineHeight));
              if(TempY > (int)(ArtTopSpace + ((int)ThisDoc->TotalLines - (int)ThisDoc->TopLineOrd - 1) * ArtLineHeight))
              {
                TempY = (int)ArtTopSpace + ((int)ThisDoc->TotalLines - (int)ThisDoc->TopLineOrd - 1) * (int)ArtLineHeight;
              }  
            }

            Previous.LineNum = ThisDoc->EndSelect.LineNum;
            Previous.CharNum = ThisDoc->EndSelect.CharNum;
            SendMessage(ThisDoc->hDocWnd, WM_HSCROLL, SB_LINEDOWN, 0);
            ThisDoc->EndSelect.LineNum = (TempY - (int)ArtTopSpace) / (int)ArtLineHeight + ThisDoc->TopLineOrd; 
            ThisDoc->EndSelect.CharNum = cursor_to_char_number (TempX,TempY,ThisDoc,&BlockPtr,&LinePtr);
                                    
            UpdateHighlightedText(ThisDoc, &Previous); 
            SendMessage(ThisDoc->hDocWnd, WM_PAINT, 0, 0);
            if(PeekMessage(&Message, ThisDoc->hDocWnd, WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE))
              break;
            GetCursorPos(&CursorLocation);
            ScreenToClient(ThisDoc->hDocWnd, &CursorLocation);
            }
            while((X == CursorLocation.x) && (Y == CursorLocation.y)
               && ((int)ThisDoc->ScXOffset < ScMax));
        }     
 
        // Check to see if text selection needs to scroll up.
        if((Y < 0) && (ThisDoc->TopLineOrd > 0))
        { 
          ScrollText = TRUE;
          
          do // keep scrolling until the mouse moves.
          { 
            TempY = ArtTopSpace;
            if (X < ArtSideSpace)
            {
              TempX = ArtSideSpace;
            }
            else
            {
              TempX = min(X, (int)ThisDoc->ScXWidth);
            }
                          
            Previous.LineNum = ThisDoc->EndSelect.LineNum;
            Previous.CharNum = ThisDoc->EndSelect.CharNum;
            SendMessage(ThisDoc->hDocWnd, WM_VSCROLL, SB_LINEUP, 0);
            ThisDoc->EndSelect.LineNum = (TempY - (int)ArtTopSpace) / (int)ArtLineHeight + ThisDoc->TopLineOrd; 
            ThisDoc->EndSelect.CharNum = cursor_to_char_number (TempX,TempY,ThisDoc,&BlockPtr,&LinePtr);
            UpdateHighlightedText(ThisDoc, &Previous);
            SendMessage(ThisDoc->hDocWnd, WM_PAINT, 0, 0);
            if(PeekMessage(&Message, ThisDoc->hDocWnd, WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE))
              break;
            GetCursorPos(&CursorLocation);
            ScreenToClient(ThisDoc->hDocWnd, &CursorLocation); 
          }
          while((X == CursorLocation.x) && (Y == CursorLocation.y)
             && (ThisDoc->TopLineOrd > 0));
        }
        
        GetScrollRange(ThisDoc->hDocWnd, SB_VERT, &ScMin, &ScMax);
        // Check to see if text selection needs to scroll down.
        if((Y > (int)ThisDoc->ScYHeight) && 
          ((int)ThisDoc->TopLineOrd < ScMax))
        {               
          ScrollText = TRUE;
              
          do // keep scrolling until the mouse moves.
          { 
            TempY = ArtTopSpace + (ThisDoc->ScYLines-1) * ArtLineHeight;
            if(TempY > (int)(ArtTopSpace + ((int)ThisDoc->TotalLines - (int)ThisDoc->TopLineOrd - 1) * ArtLineHeight))
            {
              TempY = (int)ArtTopSpace + ((int)ThisDoc->TotalLines - (int)ThisDoc->TopLineOrd - 1) * (int)ArtLineHeight;
            }  
            
            if (X < ArtSideSpace)
            {
              TempX = ArtSideSpace;
            }
            else
            {
              TempX = min(X, (int)ThisDoc->ScXWidth);
            }
                          
            Previous.LineNum = ThisDoc->EndSelect.LineNum;
            Previous.CharNum = ThisDoc->EndSelect.CharNum;
            SendMessage(ThisDoc->hDocWnd, WM_VSCROLL, SB_LINEDOWN, 0);
            ThisDoc->EndSelect.LineNum = (TempY - (int)ArtTopSpace) / (int)ArtLineHeight + ThisDoc->TopLineOrd; 
            ThisDoc->EndSelect.CharNum = cursor_to_char_number (TempX,TempY,ThisDoc,&BlockPtr,&LinePtr);
            UpdateHighlightedText(ThisDoc, &Previous);
            SendMessage(ThisDoc->hDocWnd, WM_PAINT, 0, 0); 
            if(PeekMessage(&Message, ThisDoc->hDocWnd, WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE))
              break;
            GetCursorPos(&CursorLocation);
            ScreenToClient(ThisDoc->hDocWnd, &CursorLocation);
          }
          while((X == CursorLocation.x) && (Y == CursorLocation.y)
             && ((int)ThisDoc->TopLineOrd < ScMax));
        }     
        
        if (!ScrollText)
        { 
          if (X < ArtSideSpace)
          {
            TempX = ArtSideSpace;
          }
          else
          {
            TempX = min(X, (int)ThisDoc->ScXWidth);
          }
          if (Y < ArtTopSpace)
          {
            TempY = ArtTopSpace;
          }
          else
          {
            TempY = min(Y, (int)(ArtTopSpace + (ThisDoc->ScYLines-1) * ArtLineHeight));
            if(TempY > (int)(ArtTopSpace + ((int)ThisDoc->TotalLines - (int)ThisDoc->TopLineOrd - 1) * ArtLineHeight))
            {
              TempY = (int)ArtTopSpace + ((int)ThisDoc->TotalLines - (int)ThisDoc->TopLineOrd - 1) * (int)ArtLineHeight;
            }  
          }
          
          Previous.LineNum = ThisDoc->EndSelect.LineNum;
          Previous.CharNum = ThisDoc->EndSelect.CharNum;                           
          ThisDoc->EndSelect.LineNum = (TempY - (int)ArtTopSpace) / (int)ArtLineHeight + ThisDoc->TopLineOrd; 
          ThisDoc->EndSelect.CharNum = cursor_to_char_number (TempX, TempY, ThisDoc, &BlockPtr, &LinePtr);
          UpdateHighlightedText(ThisDoc, &Previous);

        }
        
        if((ThisDoc->EndSelect.LineNum > ThisDoc->BeginSelect.LineNum) ||
          ((ThisDoc->EndSelect.LineNum == ThisDoc->BeginSelect.LineNum) &&
           (ThisDoc->EndSelect.CharNum >= ThisDoc->BeginSelect.CharNum)))
        {
          if(((Previous.LineNum < ThisDoc->BeginSelect.LineNum) ||
             ((Previous.LineNum == ThisDoc->BeginSelect.LineNum) &&
              (Previous.CharNum <= ThisDoc->BeginSelect.CharNum))))
          {
            InvalidateRect(hWnd, NULL, FALSE);
          } 
        }
        else
        {
          if(((Previous.LineNum > ThisDoc->BeginSelect.LineNum) ||
             ((Previous.LineNum == ThisDoc->BeginSelect.LineNum) &&
              (Previous.CharNum >= ThisDoc->BeginSelect.CharNum))))
          {
            InvalidateRect(hWnd, NULL, FALSE);
          } 
        }       
      }
    }  
    break;  
                    
    case WM_LBUTTONUP:
    {
      int TempX, TempY;
      
      if(DragMouseAction == DRAG_SELECT)
      { 
        DragMouseAction = DRAG_NONE; 
        if(ThisDoc->TextSelected == TRUE)
        {
          X = LOWORD (lParam);
          Y = HIWORD (lParam); 
        
          if (X < ArtSideSpace)
          {
            TempX = ArtSideSpace;
          }
          else
          {
            TempX = min(X, (int)ThisDoc->ScXWidth);
          }
          if (Y < ArtTopSpace)
          {
            TempY = ArtTopSpace;
          }
          else
          {
            TempY = min(Y, (int)(ArtTopSpace + (ThisDoc->ScYLines-1) * ArtLineHeight));
          }

          ThisDoc->EndSelect.LineNum = (TempY - (int)ArtTopSpace) / (int)ArtLineHeight + ThisDoc->TopLineOrd; 
          ThisDoc->EndSelect.CharNum = cursor_to_char_number (TempX, TempY, ThisDoc, &BlockPtr, &LinePtr);
          InvalidateRect(ThisDoc->hDocWnd, NULL, FALSE);
       }
        SetCursor(LoadCursor(NULL, IDC_ARROW)); 
        ReleaseCapture();  
      }
    }     
    break;  
       
    case WM_LBUTTONDBLCLK:
      X = LOWORD (lParam);
      Y = HIWORD (lParam);

      charnum = cursor_to_char_number (X,Y,ThisDoc,&BlockPtr,&LinePtr);

      if (charnum >= 0) {
	textlen = ((TypText far *) ((char far *) LinePtr +
				sizeof (TypLine)))->NameLen;

	textptr = (char far *) ((char far *) LinePtr +
			      sizeof (TypLine) + sizeof (TypText) );

	if (textlen)
	{                                       /* find a message-id */
	  char far *start, far *end;

	  for (start = textptr + charnum; start >= textptr; start--) 
	    if (*start == '<') break;

	  for (end = textptr + charnum; end < (textptr + textlen); end++) 
	    if (*end == '>') break;

	  if ((start >= textptr) && (end < (textptr + textlen))) {
	    sprintf (str, "%.*Fs", (int) ((long) end - (long) start + 1), start);
	    LockLine (ThisDoc->ParentDoc->hParentBlock,
			   ThisDoc->ParentDoc->ParentOffset,
			   ThisDoc->ParentDoc->ParentLineID,
			   &BlockPtr, &LinePtr);
	    GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));
            ViewArticle (ThisDoc->ParentDoc, ThisDoc->LastSeenLineID, REUSE, SHOW, str);
	  }
	}
      }

      break;

    case WM_VSCROLL:
      ScrollIt (ThisDoc, wParam, lParam);
      break;
      
    case WM_HSCROLL:
             HScrollIt (ThisDoc, wParam, lParam);
             break;

    case WM_PAINT:
      {
	HANDLE hBlock;
	unsigned int Offset, MyLen;
	int VertLines, HorzChars;
	int EndofDoc = FALSE;
	int RangeHigh, CurPos;
	int lineNum ;
	char far *textptr;
	TypBlock far *BlockPtr;
	TypLine far *LinePtr;
	int MyColorMask = 1, PrevColorMask = MyColorMask;
	BOOL ROT13Mode = GetArticleRot13Mode(hWnd) ;
	RECT aRect;
    TextSelect FAR *Start, *End;
    SIZE extent;
    HBRUSH hSolidBrush;
        
	hDC = BeginPaint (hWnd, &ps);
	SetBkColor (hDC, ArticleBackgroundColor);
        SetTextColor (hDC, ArticleTextColor);

	GetClientRect (hWnd, &myRect);
	VertLines = ThisDoc->ScYLines;
	HorzChars = ThisDoc->ScXChars;
    
    // Prepare to highlight text if it has been selected
    if(ThisDoc->TextSelected)
    {
      // check to see if selection is in a forward direction 
      // (i.e. top-to-bottom or left-to-right
      if((ThisDoc->EndSelect.LineNum > ThisDoc->BeginSelect.LineNum) ||
        ((ThisDoc->EndSelect.LineNum == ThisDoc->BeginSelect.LineNum) &&
         (ThisDoc->EndSelect.CharNum >= ThisDoc->BeginSelect.CharNum)))
      { 
        Start = &ThisDoc->BeginSelect;                                              
        End = &ThisDoc->EndSelect;                       
      }
      else
      {
         Start = &ThisDoc->EndSelect;
         End = &ThisDoc->BeginSelect;  
      }
    }           
    
	LockLine (ThisDoc->hCurTopScBlock, ThisDoc->TopScOffset, ThisDoc->TopScLineID,
		  &BlockPtr, &LinePtr);

	/* Update the scroll bar thumb position.                 */

	CurPos = lineNum = ThisDoc->TopLineOrd;
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
    
	
	/* Now paint this stuff on the screen for debugging. */

	X = ArtSideSpace - ThisDoc->ScXOffset * (ArtCharWidth+1);
	Y = ArtTopSpace;

	if (LinePtr->length != END_OF_BLOCK)
	  do
	    {
	//    MyLen = LinePtr->length - sizeof (TypLine) - sizeof (int) - 1;
	      MyLen = ((TypText far *) ((char far *) LinePtr +
					sizeof (TypLine)))->NameLen;
	      textptr = (char far *) LinePtr + sizeof (TypLine) + sizeof (TypText);

	      if  (ROT13Mode && ((unsigned)lineNum > ThisDoc->HeaderLines))
	      {  /*  within body of artcile take copy of string to
		     avoid changing original data */
		 strcpy (mybuf, textptr);
		 textptr = mybuf ;
		 strROT13(textptr) ;
	      }

	      if (isLineQuotation(textptr))
	      {  /* prepare to print a quotation Line */
		 SelectObject (hDC, hFontArtQuote);
	      } else
	      {  /* prepare to print a normal line */
		 SelectObject (hDC, hFontArtNormal);
	      }
         
          /* Now write out the line. */ 
          /* Check to see the line is to be highlighted */
          if((ThisDoc->TextSelected) && (lineNum >= Start->LineNum) &&
             (lineNum <= End->LineNum))  
	      {
	        if(MyLen != 0)
	        { 
	          if(Start->LineNum == End->LineNum)
	          { /* Only one line contains highlighted text */
	            GetTextExtentPoint(hDC, (LPSTR)textptr, Start->CharNum, &extent);
	            SetRect(&aRect, X, Y, X + extent.cx, Y+ArtLineHeight);
	            ExtTextOut (hDC, X, Y, ETO_OPAQUE, &aRect, textptr,
	                        Start->CharNum, (LPINT)NULL);  
	            SetBkColor(hDC, ArticleTextColor);
	            SetTextColor(hDC, ArticleBackgroundColor);
	            aRect.left = aRect.right;
	            GetTextExtentPoint(hDC, (LPSTR)textptr, End->CharNum+1, &extent);
	            aRect.right = X + extent.cx;
	            ExtTextOut (hDC, aRect.left, Y, ETO_OPAQUE, &aRect, 
	                        textptr + Start->CharNum, 
	                        End->CharNum - Start->CharNum + 1, (LPINT)NULL);
	            aRect.left = aRect.right;
	            GetTextExtentPoint(hDC, (LPSTR) textptr, MyLen, &extent);
	            aRect.right = X + extent.cx;
	            SetBkColor(hDC, ArticleBackgroundColor);
	            SetTextColor(hDC, ArticleTextColor);
	            ExtTextOut (hDC, aRect.left, Y, ETO_OPAQUE, &aRect, 
	                        textptr + End->CharNum + 1, MyLen - End->CharNum - 1,
	                        (LPINT)NULL);
	            Y += ArtLineHeight;
	            lineNum++;
	          }
	          else
	          {
	            if(lineNum == Start->LineNum)
	            { /* First line of more than one line to highlight */
	              GetTextExtentPoint(hDC, (LPSTR)textptr, Start->CharNum, &extent);
	              SetRect(&aRect, X, Y, X + extent.cx, Y+ArtLineHeight);
	              ExtTextOut(hDC, X, Y, ETO_OPAQUE, &aRect, textptr,
	                         Start->CharNum, (LPINT)NULL);  
	              SetBkColor(hDC, ArticleTextColor);
	              SetTextColor(hDC, ArticleBackgroundColor);
	              aRect.left = aRect.right;
	              GetTextExtentPoint(hDC, (LPSTR)textptr, MyLen, &extent);
	              aRect.right = X + extent.cx;
	              ExtTextOut(hDC, aRect.left, Y, ETO_OPAQUE, &aRect,
	                         textptr + Start->CharNum, MyLen - Start->CharNum,
	                         (LPINT)NULL);
	              SetBkColor(hDC, ArticleBackgroundColor);
	              SetTextColor(hDC, ArticleTextColor);
	            }
	            if((lineNum > Start->LineNum) && (lineNum < End->LineNum))
	            { /* Whole lines to be highlighted */ 
	              GetTextExtentPoint(hDC, (LPSTR)textptr, MyLen, &extent);
	              SetRect(&aRect, X, Y, X + extent.cx, Y+ArtLineHeight);
	              SetBkColor(hDC, ArticleTextColor);
	              SetTextColor(hDC, ArticleBackgroundColor);
	              ExtTextOut (hDC, X, Y, ETO_OPAQUE, &aRect, textptr,
	                          MyLen, (LPINT)NULL);                          
	              SetBkColor(hDC, ArticleBackgroundColor);
	              SetTextColor(hDC, ArticleTextColor);
	            }
	            if(lineNum == End->LineNum)
	            { /* Last line to be highlighted */ 
	              SetBkColor(hDC, ArticleTextColor);
	              SetTextColor(hDC, ArticleBackgroundColor);
	              GetTextExtentPoint(hDC, (LPSTR)textptr, End->CharNum + 1, &extent);
	              SetRect(&aRect, X, Y, X + extent.cx, Y+ArtLineHeight);
	              ExtTextOut(hDC, X, Y, ETO_OPAQUE, &aRect, textptr,
	                         End->CharNum + 1, (LPINT)NULL);  
	              aRect.left = aRect.right;
	              GetTextExtentPoint(hDC, (LPSTR)textptr, MyLen, &extent);
	              aRect.right = X + extent.cx;
	              SetBkColor(hDC, ArticleBackgroundColor);
	              SetTextColor(hDC, ArticleTextColor);
	              ExtTextOut(hDC, aRect.left, Y, ETO_OPAQUE, &aRect,
	                         textptr + End->CharNum + 1, MyLen - End->CharNum - 1,
	                         (LPINT)NULL);
	            }

                Y += ArtLineHeight;
                lineNum++;                    
	          
	          }    
	        }
	        else
	        { /* Blank Line -- Rectangle for CR/LF */
	          SetRect(&aRect, X, Y, X+4, Y+ArtLineHeight);
              hSolidBrush = CreateSolidBrush(ArticleTextColor);
              FillRect(hDC, &aRect, hSolidBrush); 
              DeleteObject((HGDIOBJ)hSolidBrush);
	          Y += ArtLineHeight;
	          lineNum++;
            }  
	      } 
	      else
	      { /* Line is not highlighted */
            GetTextExtentPoint(hDC, (LPSTR)textptr, MyLen, &extent);
	        SetRect (&aRect, X, Y, X + extent.cx, Y+ArtLineHeight);
	      
	        ExtTextOut (hDC, X, Y, ETO_OPAQUE, &aRect, textptr,
	                    MyLen, (LPINT)NULL);       
	        aRect.left = aRect.right;
            Y += ArtLineHeight;
            lineNum++ ; 
          }
                                         
        /* Fill in the space to the right of the end of the line *
         * with the article background color                     */                                         
        aRect.left = aRect.right;
        aRect.right = ThisDoc->ScXWidth;               
        hSolidBrush = CreateSolidBrush(ArticleBackgroundColor);
        FillRect(hDC, &aRect, hSolidBrush);
        DeleteObject((HGDIOBJ)hSolidBrush); 
	      
	    }
	  while (--VertLines > 0 && NextLine (&BlockPtr, &LinePtr));  
	  
	                                                        
	/* We've reached the end of the data to be displayed     */
	/* on this window.  If there's more screen real estate   */
	/* left, just blank it out and blank top space.          */
	SelectObject (hDC, hArticleBackgroundBrush);
	PatBlt (hDC, 0, Y, myRect.right - 1, myRect.bottom - Y, PATCOPY); 
	PatBlt (hDC, 0, 0, myRect.right - 1, ArtTopSpace, PATCOPY); 
    if(ThisDoc->ScXOffset == 0)
    {
      PatBlt (hDC, 0, 0, ArtSideSpace, ThisDoc->ScYHeight, PATCOPY);
    }  
       
	UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
	EndPaint (hWnd, &ps);
	break;
      }

    case IDM_ARTICLE_RETRIEVE_COMPLETE:
       SetArticleMenus(hWnd, ENABLE);
       break;

    case WM_COMMAND:
      switch (LOWORD(wParam))
	{
	case IDM_EXIT: 
	 ThisDoc->TextSelected = FALSE;
     CloseArtWnd(hWnd, ThisDoc);
	  break;

	case IDM_SAVE:
	  if (strcmp (SaveArtFileName, ""))
	    {
	      SaveArtAppend = TRUE;
	      MRRWriteDocument (ActiveArticleDoc, sizeof (TypText), SaveArtFileName, SaveArtAppend);
	      /* Should the 0 be sizeof(TypText) ? */
	      break;
	    }
	  else
	    {
	      goto saveas;
	    }

	case IDM_SAVEAS:
	saveas:;

	  if (DialogBox (hInst, "WinVnSaveArt", hWnd, lpfnWinVnSaveArtDlg))
	    {
	      InvalidateRect (hWnd, NULL, TRUE);
	    }
	  break;

	case IDM_PRINT:
	    PrintArticle(hWnd,ThisDoc);

	  break;
            
        case IDM_PRINT_SETUP:
	    PrinterSetup(hWnd,PD_PRINTSETUP);

	  break; 
	  
	case IDM_DECODE_ARTICLE: 
	  if (TestDecodeBusy(hWnd, "Can't decode this article"))
	     break;
	  if (CommBusy && CommDoc == ThisDoc)
	  {
	     MessageBox (hWnd, "Please wait until article retrieval is complete before decoding", "Please Wait", MB_OK);
	     break;
	  }	

	  if (!DialogBoxParam (hInst, "WinVnDecodeArts", hWnd, lpfnWinVnDecodeArtsDlg, 0))
	     InvalidateRect (hWnd, NULL, TRUE);
	  else
	     DecodeDoc (hWnd, ActiveArticleDoc);
	  break;
              
    case IDM_SELECT_ALL:
      EnableMenuItem(GetMenu(hWnd),IDM_COPY,MF_ENABLED) ;
      EnableMenuItem(GetMenu(hWnd),IDM_DESELECT_ALL,MF_ENABLED) ;
      ThisDoc->TextSelected = TRUE;
      ThisDoc->BeginSelect.LineNum = 0;
      ThisDoc->BeginSelect.CharNum = 0; 
      ThisDoc->EndSelect.LineNum = ThisDoc->TotalLines - 1;
      FindLineOrd(ThisDoc, (unsigned)ThisDoc->EndSelect.LineNum, &BlockPtr, &LinePtr);
      textlen = ((TypText far *) ((char far *) (LinePtr) +
                 sizeof (TypLine)))->NameLen;
      ThisDoc->EndSelect.CharNum = textlen - 1;
      GetClientRect(hWnd, &myRect);
      InvalidateRect(hWnd, &myRect, FALSE);         
      break;                                             
    
    case IDM_DESELECT_ALL:
      EnableMenuItem(GetMenu(hWnd),IDM_COPY,MF_GRAYED) ;
      EnableMenuItem(GetMenu(hWnd),IDM_DESELECT_ALL,MF_GRAYED) ;
      ThisDoc->TextSelected = FALSE;
      GetClientRect(hWnd, &myRect);
      InvalidateRect(hWnd, &myRect, FALSE);         
      break;                                             
    
    case IDM_COPY:
      CopyTextToClipBoard(ThisDoc);
      break;                                             
    
	case IDM_FIND_NEXT_SAME:
	case IDM_NEXT_ARTICLE:
	case IDM_PREV_ARTICLE:
     	case IDM_FIND_NEXT_UNSEEN:
     	
	  if (ThisDoc->ParentDoc)
	    {
	      LockLine (ThisDoc->ParentDoc->hParentBlock,
			ThisDoc->ParentDoc->ParentOffset,
			ThisDoc->ParentDoc->ParentLineID,
			&BlockPtr, &LinePtr);

	      GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));
              
              if (GroupDoc)
               {
               if (LOWORD(wParam) == IDM_FIND_NEXT_UNSEEN) {
		header_handle = GroupDoc->header_handle;
		thread_handle = GroupDoc->thread_handle;
		headers = lock_headers (header_handle,thread_handle);
		artindex = ThisDoc->LastSeenLineID;
	  	header_num = -1; 
	  	
	  	while ((++artindex < GroupDoc->total_headers) && header_num < 0)
	  	  {
                    header = header_elt (headers,artindex);
                    if (!header->Seen) header_num = artindex;
                  } 

		unlock_headers (header_handle, thread_handle); 
		if (header_num >= 0)
		{
		  ViewArticle (ThisDoc->ParentDoc, header_num, REUSE, SHOW, NO_ID);
		  AdjustTopScByDoc (ThisDoc->ParentDoc, (unsigned int)header_num);
		} else MessageBox (hWnd, "No more Unseen articles in this Group", "That's all!", MB_OK);
	      } 
	      else if (LOWORD(wParam) == IDM_FIND_NEXT_SAME) {
		header_handle = GroupDoc->header_handle;
		thread_handle = GroupDoc->thread_handle;
		headers = lock_headers (header_handle,thread_handle);

		header_num =
		  find_article_by_subject (headers,
					   ThisDoc->LastSeenLineID,
					   GroupDoc->total_headers-1);

		unlock_headers (header_handle, thread_handle); 
		if (header_num >= 0) {
		  ViewArticle (ThisDoc->ParentDoc, header_num, REUSE, SHOW, NO_ID);
		  AdjustTopScByDoc (ThisDoc->ParentDoc, (unsigned int)header_num);
		} else MessageBox (hWnd, "No more articles", "That's all!", MB_OK);
	       }

	      else if ((LOWORD(wParam) == IDM_NEXT_ARTICLE)
			&& (ThisDoc->LastSeenLineID < (GroupDoc->total_headers - 1) )) {
		       ViewArticle (ThisDoc->ParentDoc, ThisDoc->LastSeenLineID + 1, REUSE, SHOW, NO_ID);
		       AdjustTopScByDoc (ThisDoc->ParentDoc, (unsigned int)ThisDoc->LastSeenLineID + 1);
		      } else if ((LOWORD(wParam) == IDM_PREV_ARTICLE)
			       && (ThisDoc->LastSeenLineID > 0 )) {
		       ViewArticle (ThisDoc->ParentDoc, ThisDoc->LastSeenLineID - 1, REUSE, SHOW, NO_ID);
		       AdjustTopScByDoc (ThisDoc->ParentDoc, (unsigned int)ThisDoc->LastSeenLineID - 1);
		      } else MessageBox (hWnd, "No more articles", "That's all!", MB_OK);
	      }

           else  MessageBox (hWnd, "Sorry--you must have the group window around\n"
		          	   "\for me to be able to find the next article",
				   "Can't find next article", MB_OK);
				   
	   UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
	   }
	   
	  else
	    MessageBox (hWnd,
			"That Group's window is gone.  Reopen it.",
			"Error", MB_OK);

	  break;

	case IDM_POST:
	  CreatePostingWnd (hWnd, ThisDoc, DOCTYPE_POSTING);
	  break;

	case IDM_MAIL:
	  (MailCtrl.fnMlWinCreate)(hWnd, ThisDoc, DOCTYPE_MAIL);
	  break;

	case IDM_FORWARD:

	  (MailCtrl.fnMlForward)(hWnd, ThisDoc);
	  break;

	case IDM_SEARCH:

	  FindDoc = ThisDoc;                     
	    if (!(FindDoc->SearchStr[0]) && LastArticleTextFind[0]) 
	      strcpy(FindDoc->SearchStr, LastArticleTextFind);

	  if (DialogBox (hInst, "WinVnFind", hWnd, lpfnWinVnFindDlg))
	    {
	      found = DoFind (TRUE);
	      if (!found)
		{
		  strcpy (mybuf, "\"");
		  strcat (mybuf, FindDoc->SearchStr);
		  strcat (mybuf, "\" not found.");
		  MessageBox (hWnd, mybuf, "Not found", MB_OK);
		}
		else
	      strcpy(LastArticleTextFind, FindDoc->SearchStr);

	    }

	  break;

	case IDM_SEARCH_NEXT:

	  FindDoc = ThisDoc;
      continueFind = TRUE;
      if (!FindDoc->SearchStr[0]) {
         if (!(FindDoc->SearchStr[0]) && LastArticleTextFind[0]) 
  	        strcpy(FindDoc->SearchStr, LastArticleTextFind);
         continueFind = DialogBox (hInst, "WinVnFind", hWnd, lpfnWinVnFindDlg);
      }

	  if (continueFind && FindDoc->SearchStr[0])
	    {
	      found = DoFind (!FindDoc->hFindBlock && !FindDoc->FindLineID);
	      if (!found)
		{
		  strcpy (mybuf, "\"");
		  strcat (mybuf, FindDoc->SearchStr);
		  strcat (mybuf, "\" not found.");
		  MessageBox (hWnd, mybuf, "No more occurrences", MB_OK);
		}
	    }
	  break;


	case IDM_ROT13:
	   SetArticleRot13Mode(hWnd,GetArticleRot13Mode( hWnd) != TRUE)  ;
	   InvalidateRect (hWnd, NULL, TRUE);
	  break;


	}
	     
	break;    

    default:
      return (DefWindowProc (hWnd, message, wParam, lParam));
    }
  return (0);
}

/*------------ CloseArtWnd ------------------------------
 *
 *  Make sure this Wnd is not the active Comm window, then destroy it
 */
void CloseArtWnd(HWND hWnd, TypDoc *ThisDoc)
{
  if (CommBusy && ThisDoc == CommDoc)
    MessageBox (hWnd,
		"Please wait until article retrieval is complete",
		"Cannot close article window", MB_OK|MB_ICONSTOP);
  else 
    DestroyWindow (hWnd);
}

void 
SetArticleMailMenu(HWND hWnd)
{
	HMENU hMenu, hSubMenu;

	hMenu = GetMenu (hWnd);
	hSubMenu = GetSubMenu (hMenu, 4);	// respond menu
	EnableMenuItem (hSubMenu, IDM_MAIL, MailCtrl.enableMail);
	EnableMenuItem (hSubMenu, IDM_FORWARD, MailCtrl.enableForward);
}

// dis/enable menu items which depend on article being completely retrieved
void
SetArticleMenus (HWND hWnd, int enable)
{
	HMENU hMenu, hSubMenu;
	UINT mode;
	
	if (enable == ENABLE)	
		mode = MF_BYCOMMAND|MF_ENABLED;
	else
		mode = MF_BYCOMMAND|MF_DISABLED|MF_GRAYED;
	
	hMenu = GetMenu (hWnd);
	hSubMenu = GetSubMenu (hMenu, 0);	// file menu
	EnableMenuItem (hSubMenu, IDM_SAVE, mode);
	EnableMenuItem (hSubMenu, IDM_SAVEAS, mode);
	EnableMenuItem (hSubMenu, IDM_PRINT, mode);
	EnableMenuItem (hSubMenu, IDM_PRINT_SETUP, mode);
	EnableMenuItem (hSubMenu, IDM_DECODE_ARTICLE, mode);
	EnableMenuItem (hSubMenu, IDM_EXIT, mode);

	hSubMenu = GetSubMenu (hMenu, 1);	// edit menu
	EnableMenuItem (hSubMenu, IDM_SELECT_ALL, mode);

	hSubMenu = GetSubMenu (hMenu, 2);	// search menu
	EnableMenuItem (hSubMenu, IDM_SEARCH, mode);
	EnableMenuItem (hSubMenu, IDM_SEARCH_NEXT, mode);

	hSubMenu = GetSubMenu (hMenu, 3);	// view menu
	EnableMenuItem (hSubMenu, IDM_NEXT_ARTICLE, mode);
	EnableMenuItem (hSubMenu, IDM_FIND_NEXT_UNSEEN, mode);
	EnableMenuItem (hSubMenu, IDM_FIND_NEXT_SAME, mode);
	EnableMenuItem (hSubMenu, IDM_PREV_ARTICLE, mode);

	hSubMenu = GetSubMenu (hMenu, 4);	// respond menu
	EnableMenuItem (hSubMenu, IDM_POST, mode);
	
	SetArticleMailMenu(hWnd);
}

/*------GetArticleRot13Mode-------------------------------
 *
 *  Routine to get the this article window Rot mode.
 *  Note :
 *     interogation of windows menu state to determine
 *     ROT13 drawing mode
 *    
 *-----------------------------------------------------*/
       
BOOL
GetArticleRot13Mode(HANDLE hWnd)

{ 
 return (GetMenuState(GetMenu(hWnd),IDM_ROT13,NULL) == MF_CHECKED ) ;
}

/*------SetArticleRot13Mode-------------------------------
 *
 *  Routine to set the this article window into (or out of
 *  Rot mode).
 *  Note :
 *     interogation of windows menu state to determine
 *     ROT13 drawing mode
 *     need to invalidate window to force repaint to display
 *     in new mode
 *-----------------------------------------------------*/
       
void
SetArticleRot13Mode(HANDLE hWnd,BOOL RotMode)
{
  UINT action ;

  if (RotMode==TRUE) action = MF_CHECKED ;
  else action = MF_UNCHECKED ;

  CheckMenuItem( GetMenu(hWnd),IDM_ROT13,action) ;

}

/*            strROT13
 *
 * change the input string by ROT'ing each character
 *
 *
 */
void
strROT13 (char *cstring)

{
  char *cptr=cstring ;

  while (*cptr )
  {
      (*cptr++) = chROT13(*cptr);

  }
}

/*            chROT13
 *
 * return a new character that is the ROT(ation of) 13 characters
 * of the input character
 *
 *
 */
char chROT13(char achar)
{ char newchar ;

	if (isalpha(achar)) {
		if ((achar & 31) <= 13) {
			newchar = achar+13;
		 } else {
			newchar = achar-13;
		 }
	} else
		newchar = achar;
	
 return(newchar); 
}
/*            strnROT13
 *
 * change the input string by ROT'ating each character
 * for len or end of string (whichever is less)
 *
 *
 */
void strnROT13(char *cstring, int cstringlen)
{
  char *cptr = cstring ;
  int  clen = cstringlen ;

  while (*cptr  && clen)
  {
      (*cptr) = chROT13(*cptr);
      cptr++ ;
      clen-- ;
  }

} 
  
  

     
/* UpdateHighlightedText
 *
 * This function updates hightlighted text from the previous
 * position to the current position.
 *
 */
 
void UpdateHighlightedText(TypDoc far *DocPtr, TextSelect far *PreviousPos)
{
   SIZE extent;
   char far * textptr;
   int textlen;
   RECT Rect;
   int CurrentLineNum;
   BOOL ChangedSelectionDirection = FALSE;
   HBRUSH hSolidBrush;
   HDC hDC;
   TypBlock far *BlockPtr;
   TypLine far *LinePtr;
   TextSelect far *Start, *End, *Temp;
        
   hDC = GetDC(DocPtr->hDocWnd);               

   // check to see if selection is in a forward direction 
   // (i.e. top-to-bottom or left-to-right
   if((DocPtr->EndSelect.LineNum > DocPtr->BeginSelect.LineNum) ||
     ((DocPtr->EndSelect.LineNum == DocPtr->BeginSelect.LineNum) &&
      (DocPtr->EndSelect.CharNum >= DocPtr->BeginSelect.CharNum)))
   { 
     if((PreviousPos->LineNum == -1))
     {
       Start = &DocPtr->BeginSelect;                                              
       if(ChangedSelectionDirection) 
       {
         InvalidateRect(DocPtr->hDocWnd, NULL, FALSE);
       } 
        
     }
     else
     {  
       Start = PreviousPos;
     }  
     End = &DocPtr->EndSelect;                       
     
     // check to see if highlighting or unhighlighting
     if((End->LineNum > Start->LineNum) ||
       ((End->LineNum == Start->LineNum) &&
        (End->CharNum >= Start->CharNum)))
     {
       SetBkColor(hDC, ArticleTextColor);
       SetTextColor(hDC, ArticleBackgroundColor); 
     }
     else
     {
       SetBkColor(hDC, ArticleBackgroundColor);
       SetTextColor(hDC, ArticleTextColor); 
       Temp = Start;
       Start = End;
       End = Temp;
     }       
   }
   else
   {

     Start = &DocPtr->EndSelect;
     if((PreviousPos->LineNum == -1))
     {
       End = &DocPtr->BeginSelect;                                              
       if(ChangedSelectionDirection) 
       {
         InvalidateRect(DocPtr->hDocWnd, NULL, FALSE); 
       }  
     }                                                  
     else
     {  
       End = PreviousPos;  
     }
     
     // check to see if highlighting or unhighlighting
     if((End->LineNum > Start->LineNum) ||
       ((End->LineNum == Start->LineNum) &&
        (End->CharNum >= Start->CharNum)))
     {
       SetBkColor(hDC, ArticleTextColor);
       SetTextColor(hDC, ArticleBackgroundColor);
     }
     else       
     {
       SetBkColor(hDC, ArticleBackgroundColor);
       SetTextColor(hDC, ArticleTextColor); 
       Temp = Start;
       Start = End;
       End = Temp;
     }       
   }         

   FindLineOrd(DocPtr, (unsigned)Start->LineNum, &BlockPtr, &LinePtr);
   textlen = ((TypText far *) ((char far *) (LinePtr) +
               sizeof (TypLine)))->NameLen; 
   textptr = (char far *) ((char far *) (LinePtr) + sizeof (TypLine) + sizeof (TypText) );            
   if (isLineQuotation(textptr))
   {  /* prepare to print a quotation Line */
     SelectObject (hDC, hFontArtQuote);
   }
   else
   {  /* prepare to print a normal line */
     SelectObject (hDC, hFontArtNormal);
   }
    
   GetTextExtentPoint(hDC, (LPSTR)textptr, Start->CharNum, &extent);
   Rect.top = (int)ArtTopSpace + ((Start->LineNum - (int)DocPtr->TopLineOrd) * (int)ArtLineHeight);
   Rect.left = extent.cx - ((int)DocPtr->ScXOffset * (ArtCharWidth +1) - ArtSideSpace);

   if(Start->LineNum == End->LineNum)
   {
     GetTextExtentPoint(hDC, (LPSTR)textptr, End->CharNum +1, &extent);
     Rect.bottom = Rect.top + ArtLineHeight;
     Rect.right = extent.cx - ((int)DocPtr->ScXOffset * (ArtCharWidth +1) - ArtSideSpace);
   
     if(textlen != 0)
     {
       ExtTextOut(hDC, Rect.left, Rect.top, ETO_OPAQUE, &Rect, 
          textptr + Start->CharNum, End->CharNum - Start->CharNum +1, (LPINT)NULL);
     }
     else
     {
       Rect.right = Rect.left +4;
       hSolidBrush = CreateSolidBrush(GetBkColor(hDC));
       FillRect(hDC, &Rect, hSolidBrush);  /* Rectangle used to represent CR/LF */
       DeleteObject((HGDIOBJ)hSolidBrush);      
     }     
    
     ReleaseDC(DocPtr->hDocWnd, hDC);
   }
   else
   {
     GetTextExtentPoint(hDC, (LPSTR)textptr, textlen, &extent);
     Rect.bottom = Rect.top + ArtLineHeight;
     Rect.right = extent.cx - ((int)DocPtr->ScXOffset * (ArtCharWidth +1) - ArtSideSpace);
   
     if(textlen != 0)
     {
       ExtTextOut(hDC, Rect.left, Rect.top, ETO_OPAQUE, &Rect, 
          textptr + Start->CharNum, textlen - Start->CharNum, (LPINT)NULL);
     }
     else
     {
       Rect.right = Rect.left +4;
       hSolidBrush = CreateSolidBrush(GetBkColor(hDC));
       FillRect(hDC, &Rect, hSolidBrush);  /* Rectangle used to represent CR/LF */
       DeleteObject((HGDIOBJ)hSolidBrush);      
     }     
    
     
     CurrentLineNum = Start->LineNum + 1;
     
     while(CurrentLineNum < End->LineNum) 
     {             
       NextLine(&BlockPtr, &LinePtr);
       textlen = ((TypText far *) ((char far *) (LinePtr) +
                 sizeof (TypLine)))->NameLen; 
       textptr = (char far *) ((char far *) (LinePtr) + sizeof (TypLine) + sizeof (TypText) );            
       if (isLineQuotation(textptr))
       {  /* prepare to print a quotation Line */
         SelectObject (hDC, hFontArtQuote);
       }
       else
       {  /* prepare to print a normal line */
         SelectObject (hDC, hFontArtNormal);
       } 
       
       Rect.top = (int)ArtTopSpace + ((CurrentLineNum - (int)DocPtr->TopLineOrd) * (int)ArtLineHeight);
       Rect.left = ArtSideSpace - (int)DocPtr->ScXOffset * (ArtCharWidth +1);
       GetTextExtentPoint(hDC, (LPSTR)textptr, textlen, &extent);
       Rect.bottom = Rect.top + ArtLineHeight;
       Rect.right = extent.cx - ((int)DocPtr->ScXOffset * (ArtCharWidth +1) - ArtSideSpace);
       
       if(textlen != 0)
       {
         ExtTextOut(hDC, Rect.left, Rect.top, ETO_OPAQUE, &Rect, 
                    textptr, textlen, (LPINT)NULL);
       }
       else
       {
         Rect.right = Rect.left +4;
         hSolidBrush = CreateSolidBrush(GetBkColor(hDC));
         FillRect(hDC, &Rect, hSolidBrush);  /* Rectangle used to represent CR/LF */
         DeleteObject((HGDIOBJ)hSolidBrush);      
       }     
                  
       CurrentLineNum++;
       
     }             
     
     NextLine(&BlockPtr, &LinePtr);
     textlen = ((TypText far *) ((char far *) (LinePtr) +
               sizeof (TypLine)))->NameLen; 
     textptr = (char far *) ((char far *) (LinePtr) + sizeof (TypLine) + sizeof (TypText) );            
     if (isLineQuotation(textptr))
     {  /* prepare to print a quotation Line */
       SelectObject (hDC, hFontArtQuote);
     }
     else
     {  /* prepare to print a normal line */
       SelectObject (hDC, hFontArtNormal);
     } 
    
     Rect.top = (int)ArtTopSpace + ((CurrentLineNum - (int)DocPtr->TopLineOrd) * (int)ArtLineHeight);
     Rect.left = ArtSideSpace - (int)DocPtr->ScXOffset * (ArtCharWidth +1);
     GetTextExtentPoint(hDC, (LPSTR)textptr, End->CharNum + 1, &extent);
     Rect.bottom = Rect.top + ArtLineHeight;
     Rect.right = extent.cx - ((int)DocPtr->ScXOffset * (ArtCharWidth +1) - ArtSideSpace);
       
     if(textlen != 0)
     {
       ExtTextOut(hDC, Rect.left, Rect.top, ETO_OPAQUE, &Rect, 
                  textptr, End->CharNum + 1, (LPINT)NULL);
     }
     else
     {
       Rect.right = Rect.left +4;
       hSolidBrush = CreateSolidBrush(GetBkColor(hDC));
       FillRect(hDC, &Rect, hSolidBrush); /* Rectangle used to represent CR/LF */
       DeleteObject((HGDIOBJ)hSolidBrush);      
     }     

  }  
                                      
  ReleaseDC(DocPtr->hDocWnd, hDC);     
  
}   
   

void CopyTextToClipBoard(TypDoc FAR *DocPtr)   
{  
  TextSelect *Start, *End;
  TypBlock FAR *BlockPtr;
  TypLine FAR *LinePtr;
  int CurrentLineNum;
  int j;
  int textlen;
  char FAR *textptr;

  HGLOBAL hGlobalMemory;
  char FAR *GlobalMemoryPtr;           
  DWORD SelectedTextLength = 0;
      
  /* Check to see if selection is in a forward direction.           *
   * Start should always point to first selected position           *
   * when the document is seen from top-to-bottom and               *
   * left-to-right.                                                 */
  if((DocPtr->EndSelect.LineNum > DocPtr->BeginSelect.LineNum) ||
    ((DocPtr->EndSelect.LineNum == DocPtr->BeginSelect.LineNum) &&
     (DocPtr->EndSelect.CharNum >= DocPtr->BeginSelect.CharNum)))
  { 
    Start = &DocPtr->BeginSelect;                                              
    End = &DocPtr->EndSelect;                       
  }
  else
  {
    Start = &DocPtr->EndSelect;
    End = &DocPtr->BeginSelect;  
  }         

  /*Calculate the number of bytes to be moved to the clipboard.     */
  CurrentLineNum = Start->LineNum;
  FindLineOrd(DocPtr, (unsigned)CurrentLineNum, &BlockPtr, &LinePtr);
  textlen = ((TypText far *) ((char far *) (LinePtr) +
              sizeof (TypLine)))->NameLen;
      
  if(Start->LineNum == End->LineNum)                  
  {
    /* Text is selected on only one line.                           */
    SelectedTextLength = End->CharNum - Start->CharNum + 1;
  }
  else
  {
    /* Length of the first line. The 2 is for CR/LF.                */
    SelectedTextLength = textlen - Start->CharNum + 2;
                 
    NextLine(&BlockPtr, &LinePtr);
    CurrentLineNum++;
        
    /* Add in the lengths of all whole lines selected               */ 
    while(CurrentLineNum < End->LineNum)
    {
       textlen = ((TypText far *) ((char far *) (LinePtr) +
                  sizeof (TypLine)))->NameLen;
         
       SelectedTextLength += textlen + 2;
       /* The 2 is for CR/LF.                                       */
         
       NextLine(&BlockPtr, &LinePtr);
       CurrentLineNum++;
    }
        
    /* Add in the length of the last line selected.                 */ 
    textlen = ((TypText far *) ((char far *) (LinePtr) +
               sizeof (TypLine)))->NameLen;
    if(textlen != 0)
    {
       SelectedTextLength += End->CharNum + 1; 
    }
    else
    {
       SelectedTextLength += 2;
       /* The 2 is for CR/LF.                                       */
    }    
  }                            
       
  /* Allocate memory and move selected text to clipboard.           *
   * The 1 is for NULL string termination.                          */
  hGlobalMemory = GlobalAlloc(GHND, SelectedTextLength + 1);
  if(hGlobalMemory)
  {
    GlobalMemoryPtr = GlobalLock(hGlobalMemory);
    CurrentLineNum = Start->LineNum;
    FindLineOrd(DocPtr, (unsigned)CurrentLineNum, &BlockPtr, &LinePtr);
    textlen = ((TypText far *) ((char far *) (LinePtr) +
               sizeof (TypLine)))->NameLen;
    textptr = (char far *) ((char far *) (LinePtr) + sizeof (TypLine) + sizeof (TypText) );            
    /* Move textptr to the first character selected.                 */
    textptr += Start->CharNum;
      
    if(Start->LineNum == End->LineNum)                  
    { 
      /* Copy the text from the only line selected.                   */
      for(j = 0; j < End->CharNum - Start->CharNum + 1; j++)
      {
        *GlobalMemoryPtr++ = *textptr++;
      }  
    }
    else
    {                          
      /* Copy the first line and add CR/LF at the end of the line.    */  
      for(j = 0; j < textlen - Start->CharNum; j++)
      {
        *GlobalMemoryPtr++ = *textptr++;
      }  
      *GlobalMemoryPtr++ = '\r';
      *GlobalMemoryPtr++ = '\n';
      NextLine(&BlockPtr, &LinePtr);
      CurrentLineNum++;
      /* Copy all whole lines selected add add CR/LF at the end       *
      * of each line.                                                */
      while(CurrentLineNum < End->LineNum)
      {
         textlen = ((TypText far *) ((char far *) (LinePtr) +
                    sizeof (TypLine)))->NameLen;
         textptr = (char far *) ((char far *) (LinePtr) + 
                    sizeof (TypLine) + sizeof (TypText) );
         for(j = 0; j < textlen; j++)
         {
           *GlobalMemoryPtr++ = *textptr++;
         }  
         *GlobalMemoryPtr++ = '\r';
         *GlobalMemoryPtr++ = '\n';
        
         NextLine(&BlockPtr, &LinePtr);
         CurrentLineNum++;
      }
      /* Copy the last line selected                                  */
      textlen = ((TypText far *) ((char far *) (LinePtr) +
                 sizeof (TypLine)))->NameLen;
      textptr = (char far *) ((char far *) (LinePtr) + sizeof (TypLine) + sizeof (TypText) );
      if(textlen != 0)
      {
        for(j = 0; j < End->CharNum + 1; j++)
        {
          *GlobalMemoryPtr++ = *textptr++;
        }  
      }
      else
      {
        *GlobalMemoryPtr++ = '\r';
        *GlobalMemoryPtr++ = '\n';
      }                                 
    }                                        
      
    GlobalUnlock(hGlobalMemory); 
    OpenClipboard(DocPtr->hDocWnd); 
    EmptyClipboard();
      
    SetClipboardData(CF_TEXT, hGlobalMemory);
    CloseClipboard();
  }
  else
  {
    MessageBox(DocPtr->hDocWnd, "Not enough memory to copy selected text.", "Out Of Memory", MB_OK);
  }

}     
   
                             @


1.32
log
@support for cascading windows
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.31 1994/09/16 00:53:16 jcooper Exp $
d5 6
d351 4
a354 2
       CopyTextToClipBoard(ThisDoc);
       SendMessage (hWnd, (UINT) WM_COMMAND, (WPARAM) IDM_DESELECT_ALL, (LPARAM) 0L);  //Un-mark selection
@


1.31
log
@massive cleanup for 92.6
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.30 1994/08/24 17:59:26 jcooper Exp $
d5 3
d225 1
d252 2
a253 1
    case WM_DESTROY:                
d992 1
a992 1
	  if (!DialogBox (hInst, "WinVnDecodeArts", hWnd, lpfnWinVnDecodeArtsDlg))
@


1.30
log
@misc encoding/decoding changes
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.29 1994/08/11 00:18:04 jcooper Exp $
d5 3
d110 5
a115 3
#include "windows.h"
#include "WVglob.h"
#include "WinVn.h"
d195 1
a234 5
/*  case WM_INITMENUPOPUP:		// JSC - now handled in SetArticleMenus 
      EnableMenuItem(GetMenu(hWnd),IDM_MAIL,MailCtrl.enableMail) ;
      EnableMenuItem(GetMenu(hWnd),IDM_FORWARD,MailCtrl.enableForward);
      break;
*/
d251 2
d339 6
d1113 3
a1115 1
	  FindDoc = ThisDoc;
d1127 3
d1137 8
a1144 1
	  if (strcmp (FindDoc->SearchStr, ""))
d1146 1
a1146 1
	      found = DoFind (FALSE);
d1184 1
a1184 3
  else {
    ThisDoc->LongestLine = 0;
    ThisDoc->ScXOffset = 0;
a1185 1
  }
d1188 11
d1235 2
a1236 2
	EnableMenuItem (hSubMenu, IDM_MAIL, mode);
	EnableMenuItem (hSubMenu, IDM_FORWARD, mode);
@


1.29
log
@added support for article retreive complete message
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.28 1994/08/03 00:54:10 gardnerd Exp $
d5 3
d1042 1
d1044 3
a1046 3
		else MessageBox (hWnd, "No more Unseen articles in this Group", "That's all!", MB_OK);
	        } 
	      
d1058 1
a1058 1
		if (header_num >= 0)
d1060 2
a1061 1
		else MessageBox (hWnd, "No more articles", "That's all!", MB_OK);
d1065 8
a1072 7
			&& (ThisDoc->LastSeenLineID <
			    (GroupDoc->total_headers - 1) ))
		      ViewArticle (ThisDoc->ParentDoc, ThisDoc->LastSeenLineID + 1, REUSE, SHOW, NO_ID);
		      else if ((LOWORD(wParam) == IDM_PREV_ARTICLE)
			       && (ThisDoc->LastSeenLineID > 0 ))
		      ViewArticle (ThisDoc->ParentDoc, ThisDoc->LastSeenLineID - 1, REUSE, SHOW, NO_ID);
		      else MessageBox (hWnd, "No more articles", "That's all!", MB_OK);
d1074 1
a1074 1
	      
@



1.28
log
@Fixed bug with cut to clipboard at 1st and last line of text
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.27 1994/07/27 21:14:38 gardnerd Exp $
d5 3
d114 1
d212 4
d226 1
a226 1
    case WM_INITMENUPOPUP:
d230 1
a230 1

d310 3
a312 2
             {
              SendMessage (hWnd,(UINT) WM_COMMAND, (WPARAM) IDM_FIND_NEXT_UNSEEN, (LPARAM) 0L); 
d922 4
a928 1

d1162 40
@



1.27
log
@copy to clipboard
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.26 1994/07/12 19:16:13 rushing Exp $
d5 3
d324 9
a332 6
    case WM_LBUTTONDOWN:            
	  if (CommBusy && CommDoc == ThisDoc)
	  {
	     MessageBox (hWnd, "Please wait until article retrieval is complete before selecting text.", "Please Wait", MB_OK);
	  }
	  else	
d340 25
a364 2
        ThisDoc->BeginSelect.LineNum = (Y - ArtTopSpace) / ArtLineHeight + ThisDoc->TopLineOrd;
        ThisDoc->BeginSelect.CharNum = cursor_to_char_number (X,Y,ThisDoc,&BlockPtr,&LinePtr);
d370 3
a372 2
      }  
      break;  
d407 4
d445 4
d511 5
d558 4
@



1.26
log
@missing curlies in CloseArtWnd()
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.25 1994/07/01 03:45:53 dumoulin Exp rushing $
d5 3
d105 4
a108 1

d186 1
a186 2


d229 1
d233 2
a234 1
    case WM_DESTROY:
d319 203
a521 1
      break;
d523 65
d647 3
d658 20
a677 1

d730 100
a829 3

	      /* Now write out the line. */
	      SetRect (&aRect, 0, Y, myRect.right, Y+ArtLineHeight);
d831 14
a845 5
	      ExtTextOut (hDC, X, Y, ETO_OPAQUE, &aRect, textptr,
	                  MyLen, (LPINT)NULL);
          
          Y += ArtLineHeight;
	      lineNum++ ;
d847 2
a848 1
	  while (--VertLines > 0 && NextLine (&BlockPtr, &LinePtr));
d852 1
a852 1
	/* left, just blank it out.                              */
d855 6
a860 1

d870 2
a871 1
	case IDM_EXIT:
d921 28
a948 1

d1203 399
a1601 1
}
@



1.25
log
@Added the Find Next Unseen and concept of SPACE as a "power key"
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.24 1994/06/30 21:30:22 gardnerd Exp dumoulin $
d5 3
d660 7
a666 6
	  "Please wait until article retrieval is complete",
	  "Cannot close article window", MB_OK|MB_ICONSTOP);
  else
     ThisDoc->LongestLine = 0;
     ThisDoc->ScXOffset = 0;
     DestroyWindow (hWnd);
@



1.24
log
@Allow scrolling by pixels instead of characters
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.23 1994/06/23 23:07:13 dumoulin Exp $
d5 3
d149 1
a149 2
long FAR PASCAL 
WinVnArtWndProc (hWnd, message, wParam, lParam)
d171 1
a171 1
  TypLineID MyLineID;
d175 1
d280 14
d513 2
a514 1

d523 15
d539 7
a545 1
	      if (LOWORD(wParam) == IDM_FIND_NEXT_SAME) {
d559 1
a559 1
	      }
d561 1
a561 3
	      else {
		if (GroupDoc)
		  { if ((LOWORD(wParam) == IDM_NEXT_ARTICLE)
a568 6
		    }
		else MessageBox (hWnd,
				 "Sorry--you must have the group window around\n"
				 "\for me to be able to find the next article",
				 "Can't find next article", MB_OK);
		UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
d570 8
a577 1
	    }
@



1.23
log
@added menu item for Printer Setup
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.22 1994/06/01 19:01:51 gardnerd Exp dumoulin $
d5 3
d353 1
a353 1

d375 2
a376 2

    RangeHigh = ThisDoc->LongestLine - ThisDoc->ScXChars; 
d383 2
a384 1
	SetScrollPos (hWnd, SB_HORZ, ThisDoc->ScXOffset, TRUE); 
d388 1
a388 1
	X = ArtSideSpace;
d418 3
a420 11
	      /* Check Horz Scroll to see if a blank line is to be written */
	      if((int)MyLen - (int)ThisDoc->ScXOffset < 0)
	      {
	        ExtTextOut (hDC, X, Y, ETO_OPAQUE, &aRect, " ",
	                    1, (LPINT)NULL);
	      }
	      else
	      {
	        ExtTextOut (hDC, X, Y, ETO_OPAQUE, &aRect, textptr+ThisDoc->ScXOffset,
	                  MyLen-ThisDoc->ScXOffset, (LPINT)NULL);
          } 
d422 1
a422 1
	      Y += ArtLineHeight;
d426 1
a426 1

a429 1

d431 1
a431 1
	PatBlt (hDC, 0, Y, myRect.right - 1, myRect.bottom - Y, PATCOPY);
@



1.22
log
@horizontal scrolling support
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.21 1994/06/01 18:50:49 rushing Exp $
d5 3
d477 3
d481 2
@



1.21
log
@more f3 bugs
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.20 1994/05/27 01:29:29 rushing Exp rushing $
d5 3
d328 4
d366 1
a366 1
	  RangeHigh = 0;
d370 9
d410 13
a422 2
	      ExtTextOut (hDC, X, Y, ETO_OPAQUE, &aRect, textptr, MyLen, (LPINT)NULL);

d619 2
@



1.20
log
@unnecessary winundoc.h
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.19 1994/05/23 18:36:00 jcooper Exp rushing $
d5 3
d116 1
@



1.19
log
@new attach code, session [dis]connect
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.18 1994/02/24 21:26:54 jcoop Exp $
d5 3
a78 1
#include "winundoc.h"
@



1.18
log
@jcoop changes
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.17 1994/01/24 17:41:01 jcoop Exp $
d5 3
d81 1
d203 1
a203 6
      if (CommBusy && ThisDoc == CommDoc)
 	MessageBox (hWnd,
	  "Please wait until article retrieval is complete",
	  "Cannot close article window", MB_OK|MB_ICONSTOP);
      else
        DestroyWindow (hWnd);
d412 1
a412 1
	  DestroyWindow (hWnd);
d573 14
@



1.17
log
@90.2 changes
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.16 1994/01/16 12:23:43 jcoop Exp $
d5 3
d194 2
a195 2
      ThisDoc->ScYLines = (myRect.bottom - myRect.top - TopSpace) / ArtLineHeight;
      ThisDoc->ScXChars = (myRect.right - myRect.left - SideSpace) / ArtCharWidth;
d324 1
a324 2
	SIZE sz;
	unsigned int Offset, MyLen, width;
a327 1
	int RestX;
d334 1
d338 2
a339 1
	SelectObject (hDC, hArticleBackgroundBrush);
d360 2
a361 2
	X = SideSpace;
	Y = StartPen;
d388 2
a389 10

	  (GetTextExtentPoint (hDC, textptr, MyLen, &sz));       
	       width = sz.cx;                                     
	      SetTextColor (hDC, ArticleTextColor);

	      TextOut (hDC, X, Y, textptr, MyLen);

	      RestX = X + width;
	      
	      PatBlt (hDC, RestX, Y, myRect.right - RestX, ArtLineHeight, PATCOPY);
d400 1
a400 1

@



1.16
log
@New font/color painting.  no longer use view_art_by_id
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.16 1994/01/16 11:59:44 jcoop Exp $
d5 3
d195 9
a203 1

@



1.15
log
@new version box and cr lf consistency
@
text
@a0 1

d3 1
a3 1
 * $Id: wvart.c 1.14 1993/08/25 18:54:36 mbretherton Exp rushing $
d5 3
a71 1
void view_article_by_message_id (TypDoc *Doc, char far *article_request, long artindex);
d296 1
a296 1
	    view_article_by_message_id (ThisDoc->ParentDoc, str, ThisDoc->LastSeenLineID);
d324 2
a325 1

d377 1
d382 1
d395 1
a395 1
	PatBlt (hDC, 0, Y, myRect.right, myRect.bottom - Y, PATCOPY);
d437 15
d477 1
a477 1
		  ViewArticle (ThisDoc->ParentDoc, header_num, REUSE, SHOW);
d486 1
a486 1
		      ViewArticle (ThisDoc->ParentDoc, ThisDoc->LastSeenLineID + 1, TRUE, SHOW);
d489 1
a489 1
		      ViewArticle (ThisDoc->ParentDoc, ThisDoc->LastSeenLineID - 1, TRUE, SHOW);
@



1.14
log
@MRB merge, mail & post logging
@
text
@d1 1
d4 1
a4 1
 * $Id: wvart.c 1.13 1993/08/05 20:06:07 jcoop Exp $
d6 3
@



1.13
log
@save multiple articles changes by jcoop@@oakb2s01.apl.com (John S Cooper)
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.12 1993/07/06 21:09:50 cnolan Exp $
d5 7
d101 1
a101 1
      return (artindex);	/* return the index */
d104 1
a104 1
  return (-1);			/* not found */
d210 1
a210 1
	thread_handle = GroupDoc->thread_handle;	
d311 1
a311 1
        int lineNum ;
d318 1
a318 1
        hDC = BeginPaint (hWnd, &ps);
d349 1
a349 1
              textptr = (char far *) LinePtr + sizeof (TypLine) + sizeof (TypText);
d354 2
a355 2
	         strcpy (mybuf, textptr);
	         textptr = mybuf ;
d365 1
a365 1
              }
d367 1
a367 1
    	      /* Now write out the line. */
d369 1
a369 1
          (GetTextExtentPoint (hDC, textptr, MyLen, &sz));       
d406 1
a406 1
	      MRRWriteDocument (ActiveArticleDoc, sizeof (TypText), SaveArtFileName, SaveArtvRef, SaveArtAppend);
d431 1
a458 1

d460 9
a468 6
		  {
		    if (ThisDoc->LastSeenLineID <
			(GroupDoc->total_headers - 1) )
		      ViewArticle (ThisDoc->ParentDoc, ThisDoc->LastSeenLineID + 1, REUSE, SHOW);
		    else MessageBox (hWnd, "No more articles", "That's all!", MB_OK);
		  }
d483 1
a483 1
	case IDM_FOLLOWUP:
d537 2
a538 2
  	}
             
d615 2
a616 2
        if (isalpha(achar)) {
                if ((achar & 31) <= 13) {
d620 2
a621 2
                 }
        } else
d623 1
a623 1
        
@



1.12
log
@win32 support
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.11 1993/06/26 00:53:33 rushing Exp $
d5 3
d446 1
a446 1
		  ViewArticle (ThisDoc->ParentDoc, header_num, TRUE);
d456 1
a456 1
		      ViewArticle (ThisDoc->ParentDoc, ThisDoc->LastSeenLineID + 1, TRUE);
@



1.11
log
@warnings
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.10 1993/06/26 00:18:46 rushing Exp rushing $
d5 3
d295 1
d359 2
a360 1
	      width = LOWORD (GetTextExtent (hDC, textptr, MyLen));
d385 1
a385 1
      switch (wParam)
d431 1
a431 1
	      if (wParam == IDM_FIND_NEXT_SAME) {
@



1.10
log
@warnings
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.9 1993/06/22 16:45:00 bretherton Exp rushing $
d5 3
d337 1
a337 1
	      if  (ROT13Mode && (lineNum > ThisDoc->HeaderLines))
@



1.9
log
@*** empty log message ***
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.8 1993/06/11 00:11:26 rushing Exp $
d5 3
d555 1
a555 2

{ HMENU hMenu ;
@



1.8
log
@second merge from Matt Bretherton sources
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.7 1993/05/29 03:32:30 rushing Exp rushing $
d5 5
d157 1
a157 1
      EnableMenuItem(GetMenu(hWnd),IDM_FORWARD,MailCtrl.enableMail);
@



1.7
log
@threading support
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.6 1993/05/28 18:23:40 rushing Exp rushing $
d5 3
d150 1
a150 1
    case WM_CREATE:
@



1.6
log
@fixed far/huge problem in find_art_by_subject
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.5 1993/05/24 23:28:24 rushing Exp rushing $
d5 3
d59 1
a59 1
long find_article_by_subject (TypHeader huge *headers,
d64 3
a66 1
  char far *thissub = headers[artindex++].subject;
d71 1
a71 1
    nextsub = headers[artindex].subject ;
d114 2
a115 1
  TypHeader huge * headers;
d183 2
a184 1
	headers = (TypHeader huge *) (GlobalLock (header_handle) + sizeof(char huge *)) ;
d186 1
a186 1
	headers[ThisDoc->LastSeenLineID].ArtDoc = (TypDoc *) NULL;
d188 1
a188 1
	GlobalUnlock (header_handle);
d403 45
a447 34
		if (ThisDoc->ParentDoc)
		{
			LockLine (ThisDoc->ParentDoc->hParentBlock,
				  ThisDoc->ParentDoc->ParentOffset,
				  ThisDoc->ParentDoc->ParentLineID,
				  &BlockPtr, &LinePtr);

			GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));

			if (wParam == IDM_FIND_NEXT_SAME) {
				header_handle = GroupDoc->header_handle;
				headers = (TypHeader far *) (GlobalLock (header_handle) + sizeof(char huge *)) ;

				header_num = find_article_by_subject (headers,ThisDoc->LastSeenLineID,GroupDoc->total_headers - 1);

				GlobalUnlock (header_handle);
				if (header_num >= 0)
				ViewArticle (ThisDoc->ParentDoc, header_num, TRUE);
				else MessageBox (hWnd, "No more articles", "That's all!", MB_OK);
			}

			else {

				if (GroupDoc)
				{
				  if (ThisDoc->LastSeenLineID < (GroupDoc->total_headers - 1) )
				    ViewArticle (ThisDoc->ParentDoc, ThisDoc->LastSeenLineID + 1, TRUE);
				  else MessageBox (hWnd, "No more articles", "That's all!", MB_OK);
				}
				else MessageBox (hWnd, "Sorry--you must have the group window around\n\for me to be able to find the next article", "Can't find next article", MB_OK);
			UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
			}
		}
		else MessageBox (hWnd, "That Group's window is gone.  Reopen it.", "Error", MB_OK);
@



1.5
log
@header formatting (MRB)
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.4 1993/05/18 22:11:40 rushing Exp $
d5 2
d8 1
d56 3
a58 3
long find_article_by_subject (TypHeader far *headers,
				long artindex,
				long num_headers)
d72 1
a72 1
  return (-1);                    /* not found */
@



1.4
log
@smtp support
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.3 1993/05/06 19:43:41 rushing Exp rushing $
d5 4
d26 1
a31 1
long find_article_by_subject (TypHeader far *headers,long artindex,long num_headers);
d35 1
a35 1
/************** find_article_by_subject *************
d41 1
a41 1
 *  format with wvheadr : GetArticleSubject
d51 2
a52 2
********************************************************/
 
d54 2
a55 2
                long artindex,
                long num_headers)
d60 2
a61 2
 
 
d68 2
a69 2
 
  return (-1);			/* not found */
d88 1
a88 1
  PAINTSTRUCT ps;		/* paint structure          */
d90 2
a91 2
  HDC hDC;			/* handle to display context */
  RECT myRect;			/* selection rectangle      */
d140 1
d166 4
a169 4
       	LockLine (ThisDoc->ParentDoc->hParentBlock,
       	          ThisDoc->ParentDoc->ParentOffset,
       	          ThisDoc->ParentDoc->ParentLineID,
       	          &BlockPtr, &LinePtr);
d231 2
a232 2
        textlen = ((TypText far *) ((char far *) LinePtr +
                                sizeof (TypLine)))->NameLen;
d234 2
a235 2
        textptr = (char far *) ((char far *) LinePtr +
     	                      sizeof (TypLine) + sizeof (TypText) );
d237 20
a256 20
        if (textlen)
        {					/* find a message-id */
          char far *start, far *end;

          for (start = textptr + charnum; start >= textptr; start--) 
            if (*start == '<') break;

          for (end = textptr + charnum; end < (textptr + textlen); end++) 
            if (*end == '>') break;

          if ((start >= textptr) && (end < (textptr + textlen))) {
            sprintf (str, "ARTICLE %.*Fs\r", (int) ((long) end - (long) start + 1), start);
            LockLine (ThisDoc->ParentDoc->hParentBlock,
                           ThisDoc->ParentDoc->ParentOffset,
		           ThisDoc->ParentDoc->ParentLineID,
		           &BlockPtr, &LinePtr);
            GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));
            view_article_by_message_id (ThisDoc->ParentDoc, str, ThisDoc->LastSeenLineID);
          }
        }
d273 1
d278 1
d280 1
a280 1
	hDC = BeginPaint (hWnd, &ps);
a282 1
	SelectObject (hDC, hFontArt);
d291 1
a291 1
	CurPos = ThisDoc->TopLineOrd;
d311 1
a311 1
	      textptr = (char far *) LinePtr + sizeof (TypLine) + sizeof (TypText);
d313 15
a327 1
	      /* Now write out the line.                            */
d329 2
d339 1
d386 1
a386 1
            PrintArticle(hWnd,ThisDoc);
d396 3
a398 3
			          ThisDoc->ParentDoc->ParentOffset,
			          ThisDoc->ParentDoc->ParentLineID,
			          &BlockPtr, &LinePtr);
d411 1
a411 1
				else MessageBox (hWnd, "No more articles", "That's all!", MB_OK); 
d428 1
a428 1
          break;
d440 1
a440 1
	  (MailCtrl.fnMlForward)(hWnd, ThisDoc, DOCTYPE_MAIL);
d477 6
a482 1
	}
d484 3
a486 1
	break;
d494 39
d534 8
d543 2
d546 3
d550 2
d553 9
d563 22
d586 6
d593 1
@



1.3
log
@added Re: fix from MB
@
text
@d3 1
a3 1
 * $Id: wvart.c 1.2 1993/03/30 20:08:25 rushing Exp rushing $
d5 3
d413 5
@



1.2
log
@MAPI
@
text
@d3 5
a7 2
 * $Id: wvart.c%v 1.1 1993/02/16 20:53:50 rushing Exp $
 * $Log: wvart.c%v $
d28 21
a48 4
long
find_article_by_subject (TypHeader far *headers,
				long artindex,
				long num_headers)
d50 13
a62 8
char far *subject = headers[artindex++].subject;

	do {
		if (lstrcmpi (headers[artindex].subject, subject) == 0)
		return (artindex);		/* return the index */
	} while (artindex++ < num_headers);

	return (-1);			/* not found */
d64 1
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
d105 4
d379 1
a379 1
	  CreatePostingWnd (ThisDoc, DOCTYPE_POSTING);
d383 1
a383 1
	  CreatePostingWnd (ThisDoc, DOCTYPE_MAIL);
@

