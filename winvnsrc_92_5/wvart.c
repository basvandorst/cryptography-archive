/*
 *
 * $Id: wvart.c 1.30 1994/08/24 17:59:26 jcooper Exp $
 * $Log: wvart.c $
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
 * save multiple articles changes by jcoop@oakb2s01.apl.com (John S Cooper)
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

#include <ctype.h>
#include "windows.h"
#include "WVglob.h"
#include "WinVn.h"

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

/*  case WM_INITMENUPOPUP:		// JSC - now handled in SetArticleMenus 
      EnableMenuItem(GetMenu(hWnd),IDM_MAIL,MailCtrl.enableMail) ;
      EnableMenuItem(GetMenu(hWnd),IDM_FORWARD,MailCtrl.enableForward);
      break;
*/
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
      ThisDoc->TextSelected = FALSE;
      ThisDoc->InUse = FALSE;
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

	  if (!DialogBox (hInst, "WinVnDecodeArts", hWnd, lpfnWinVnDecodeArtsDlg))
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
	    }

	  break;

	case IDM_SEARCH_NEXT:

	  FindDoc = ThisDoc;
	  if (strcmp (FindDoc->SearchStr, ""))
	    {
	      found = DoFind (FALSE);
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
  else {
    ThisDoc->LongestLine = 0;
    ThisDoc->ScXOffset = 0;
    DestroyWindow (hWnd);
  }
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
	EnableMenuItem (hSubMenu, IDM_MAIL, mode);
	EnableMenuItem (hSubMenu, IDM_FORWARD, mode);
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
   
                             