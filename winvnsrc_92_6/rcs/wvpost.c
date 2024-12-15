head	 1.24;
branch   ;
access   ;
symbols  V80:1.4 V76d:1.1;
locks    ; strict;
comment  @ * @;


1.24
date	 94.09.18.22.49.37;  author jcooper;  state Exp;
branches ;
next	 1.23;

1.23
date	 94.09.16.01.05.38;  author jcooper;  state Exp;
branches ;
next	 1.22;

1.22
date	 94.08.24.18.40.15;  author mrr;  state Exp;
branches ;
next	 1.21;

1.21
date	 94.06.09.18.39.34;  author rushing;  state Exp;
branches ;
next	 1.20;

1.20
date	 94.06.06.22.15.31;  author rushing;  state Exp;
branches ;
next	 1.19;

1.19
date	 94.06.03.20.44.34;  author rushing;  state Exp;
branches ;
next	 1.18;

1.18
date	 94.05.27.01.29.29;  author rushing;  state Exp;
branches ;
next	 1.17;

1.17
date	 94.05.26.22.19.08;  author jglasser;  state Exp;
branches ;
next	 1.16;

1.16
date	 94.05.19.02.02.10;  author rushing;  state Exp;
branches ;
next	 1.15;

1.15
date	 94.02.09.18.39.32;  author rushing;  state Exp;
branches ;
next	 1.14;

1.14
date	 94.02.09.18.01.08;  author cnolan;  state Exp;
branches ;
next	 1.13;

1.13
date	 94.01.24.17.40.28;  author jcoop;  state Exp;
branches ;
next	 1.12;

1.12
date	 94.01.15.20.35.34;  author jcoop;  state Exp;
branches ;
next	 1.11;

1.11
date	 94.01.11.18.51.45;  author cnolan;  state Exp;
branches ;
next	 1.10;

1.10
date	 93.12.08.01.28.01;  author rushing;  state Exp;
branches ;
next	 1.9;

1.9
date	 93.08.25.18.53.17;  author mbretherton;  state Exp;
branches ;
next	 1.8;

1.8
date	 93.08.25.17.05.17;  author mbretherton;  state Exp;
branches ;
next	 1.7;

1.7
date	 93.07.06.21.09.50;  author cnolan;  state Exp;
branches ;
next	 1.6;

1.6
date	 93.07.06.17.11.56;  author rushing;  state Exp;
branches ;
next	 1.5;

1.5
date	 93.06.28.17.52.17;  author rushing;  state Exp;
branches ;
next	 1.4;

1.4
date	 93.06.11.00.10.35;  author rushing;  state Exp;
branches ;
next	 1.3;

1.3
date	 93.05.25.00.00.45;  author rushing;  state Exp;
branches ;
next	 1.2;

1.2
date	 93.05.08.17.11.54;  author rushing;  state Exp;
branches ;
next	 1.1;

1.1
date	 93.02.16.20.54.22;  author rushing;  state Exp;
branches ;
next	 ;


desc
@winvn version 0.76 placed into RCS
@


1.24
log
@support for cascading windows
@
text
@
/*
 * $Id: wvpost.c 1.23 1994/09/16 01:05:38 jcooper Exp $
 */

/*--- WVPOST.C ------------------------------------------------------------
 *
 *    This file contains the window procedure for the "posting" windows,
 *    windows in which the user edits the text of an article to be posted.
 *
 *    Mark Riordan   27 Jan 1990
 */
#include <windows.h>
#include <windowsx.h>
#include "wvglob.h"
#include "winvn.h"
#pragma hdrstop
#include <commdlg.h>

/* globals */
static char CurrentPostTitle[MAXINTERNALLINE];
static char *CurrentPostEdit;

long FAR PASCAL 
WinVnPostWndProc (hWnd, message, wParam, lParam)
     HWND hWnd;
     unsigned message;
     WPARAM wParam;
     LPARAM lParam;
{

  WndEdit * WndPost;
  int ih;
 
  WndPost = getWndEdit(WndPosts,hWnd,MAXPOSTWNDS) ;

  if ((!WndPost) && message == WM_CREATE)
    {
      for (ih = 0; ih < MAXPOSTWNDS; ih++)
   {
     if (!WndPosts[ih].hWnd)
       {
         WndPost = &WndPosts[ih] ;
         WndPost->hWnd = hWnd;
         WndPost->Doc = PostDoc ;

         break;
       }
   }
    }

  switch (message)
    {
    case WM_SYSCOMMAND:
      return (DefWindowProc (hWnd, message, wParam, lParam));

    case WM_CREATE:
      if (CreateEditWnd (WndPost) == FAIL)
         break;
      
      NumPostWnds++;

      SendMessage (WndPost->hWndEdit, WM_SETFONT, (WPARAM)hFontArtNormal, TRUE);
      
      WndPost->dirty = DT_CLEAN ;
      if (!Attaching)
         CreatePostingText (PostDoc, hWnd, WndPost->hWndEdit, DOCTYPE_POSTING);

      /* Subclass the edit window to pass acceleratprs to parent */
      SetWindowLong(WndPost->hWndEdit, GWL_WNDPROC, (LONG) lpfnEditSubClass);
      break;

#ifdef _WIN32
    case WM_CTLCOLOREDIT:
#else
   case WM_CTLCOLOR:    //WIN32 doesn't handle this message anymore
      // Set foreground and background colors of edit control
      if (HIWORD(lParam) == CTLCOLOR_EDIT)
#endif
      {
         SetTextColor((HDC)wParam, ArticleTextColor);
         SetBkColor((HDC)wParam, ArticleBackgroundColor);
         // Return handle to background brush for the edit control
         return (LRESULT)hArticleBackgroundBrush;
      }
      break;

    case WM_SIZE:

      MoveWindow (WndPost->hWndEdit, 0, 0, LOWORD (lParam),
        HIWORD (lParam), TRUE);
      break;

    case WM_SETFOCUS:

      SetFocus (WndPost->hWndEdit);
      break;

    case WM_KEYDOWN:

      if (wParam == VK_F6)
   {
     NextWindow (WndPost->Doc);
   }
      break;

    case WM_INITMENUPOPUP :

      prepareEditMenu (hWnd, WndPost->hWndEdit);
      break ;


    case WM_COMMAND:
    // Edit Window notifications to parent
    if ((HWND)(UINT)lParam == WndPost  ->hWndEdit){     
#ifdef WIN32
       if (HIWORD(wParam) == EN_CHANGE)
#else
       if (HIWORD(lParam) == EN_CHANGE)
#endif
        WndPost->dirty = DT_DIRTY ;
    return(0L) ;
      }

      switch (LOWORD(wParam))
      {
   case IDM_CANCEL:

   case IDV_EXIT:
        DoEditClose(WndPost->hWnd,WndPost->dirty);
     break;

   case IDM_SEND:
     StartPost (WndPost);
     break;

   case IDM_PRINT:
     PrintFile (WndPost->hWndEdit);
     break;

   case IDM_ATTACH:
     if (TestDecodeBusy(hWndConf, "Can't make attachment"))
       break;

     if (AskForExistingFileName (hWnd, AttachFileName, "Open file to attach") == FAIL)
      break;
           if (DialogBox (hInst, "WinVnAttach", hWnd, lpfnWinVnAttachDlg))
           Attach (WndPost, AttachFileName, DOCTYPE_POSTING);
     
     InvalidateRect (hWnd, NULL, TRUE);
          break;

   default:
     DoEditCommands (WndPost->hWndEdit, wParam, lParam);
   break;
      }

      break;
      
    case WM_DESTROY:
//    editDS = GetWindowWord (WndPost->hWndEdit, GWW_HINSTANCE);
//    GlobalFree (editDS); // must manually free global GMEM_SHARE instance
      WndPost->hWnd = 0;
      WndPost->hWndEdit = 0;
      NumPostWnds--;
      break;

    case WM_CLOSE:
      if (CommBusy && PostEdit == WndPost)
   MessageBox (hWnd,
     "Please wait until posting is complete",
     "Cannot close posting window", MB_OK|MB_ICONSTOP);
      else
   DoEditClose(WndPost->hWnd,WndPost->dirty);
      break;

    default:
      return (DefWindowProc (hWnd, message, wParam, lParam));

    }
  return (0L);
}

/*--- function StartPost ----------------------------------------------
 *
 *    Start the posting procedure for an article that has been created
 *    by the user.
 *
 */
BOOL
StartPost (WndEdit *thisPost)
{
  BOOL PostOK = FALSE;
  char outline[MAXOUTLINE];

  if (!TestCommBusy(thisPost->hWnd, "Can't post article right now"))
    {
      CommLinePtr = CommLineIn;
      CommBusy = TRUE;
      CommState = ST_POST_WAIT_PERMISSION;
      CommWnd = thisPost->hWnd;
      PostEdit = thisPost;
      strcpy (outline, "POST");
      PutCommLine (outline);
      PostOK = TRUE;
    }
  return (PostOK);
}

/*--- function PostText -------------------------------------------------
 *
 *    Post the text of an article.  (Second phase of posting process.)
 *
 *    Entry    ih    is the index into hWndPostEdits of the child
 *                   edit window containing the article.
 */
BOOL
PostText (WndEdit * WndPost)
{
  BOOL EndBuf = FALSE;
  
  GetWindowText(WndPost->hWnd, CurrentPostTitle, MAXINTERNALLINE-1);
  strcpy(str, CurrentPostTitle);
  strcat(str, "  [Posting...]");
  SetWindowText (WndPost->hWnd, str);
  
  CommState = ST_POST_WAIT_END;
  
  /* If authentication was required, stuff a header line into
   * the beginning of the message so that people reading it can
   * know who really posted it.  (The From: line may be a lie.)
   */
  if(AuthReqPost) {
    AddAuthHeader(WndPost->hWndEdit);
  }

  /* retrieve the edit text, and send it out the socket.
   * post_edit_data handles transformations (dot-doubling and
   * soft line breaks).
   */

  CurrentPostEdit = GetEditText (WndPost->hWndEdit);

  post_edit_data (CurrentPostEdit);
  
  return (TRUE);
}

void
CompletePost(WndEdit * PostEdit, int result)
{
   /* note CommLineIn currently contains the server result "code: text" */
   
   switch (result) {
   case SUCCESS:
#if 0   
     MessageBox(hPostEdit,CommLineIn+4,"Article Posted OK",MB_OK);
#endif   

     /* if attaching in non-review mode, then there is no edit window */
     if (!Attaching || (Attaching && ReviewAttach)) {
       if (PostLog) 
         WriteEditLog (PostEdit->hWnd, PostLogFile, CurrentPostEdit, strlen(CurrentPostEdit));
     
       if (CcByMail && CcAddress[0]) {
         strcpy(str, CurrentPostTitle);
         strcat(str, "  [Mailing...]");
         SetWindowText (PostEdit->hWnd, str);

         cc_smtp_message(CcAddress, CurrentPostEdit);
       }

       PostEdit->dirty = DT_CLEAN;
       DestroyWindow (PostEdit->hWnd);
     }
     break;

   case FAIL:
     SetWindowText (PostEdit->hWnd, CurrentPostTitle);
     MessageBox (PostEdit->hWnd, CommLineIn + 4, "Posting Failed", MB_OK | MB_ICONEXCLAMATION);
     /* let 'em fix the post! */
     SendMessage (PostEdit->hWndEdit, EM_SETREADONLY, FALSE, 0L);
     break;

   case ABORT:
     break;
   }
   PostEdit = (WndEdit *) NULL;
   GlobalFreePtr (CurrentPostEdit);
}                             
@


1.23
log
@New cc-by-mail handler.  New CompletePost() routine for better
abort/cancel handling.  general cleanup for 92.6
@
text
@d3 1
a3 1
 * $Id: wvpost.c 1.22 1994/08/24 18:40:15 mrr Exp $
d60 2
d165 1
@


1.22
log
@authorization enables post/mail
@
text
@d3 1
a3 1
 * $Id: wvpost.c 1.21 1994/06/09 18:39:34 rushing Exp $
d14 1
a14 2
#include <windowsx.h>   // for GlobalFreePtr
#include <commdlg.h>
d17 2
d20 4
d61 1
a61 1
      SendMessage (WndPost->hWndEdit, WM_SETFONT, hFontArtNormal, TRUE);
d82 1
a82 1
         return (DWORD)hArticleBackgroundBrush;
d140 3
d215 1
a215 2
PostText (WndPost)
    WndEdit * WndPost;
a217 1
  char *editptr;
d219 5
d239 1
a239 1
  editptr = GetEditText (WndPost->hWndEdit);
d241 4
a244 1
  post_edit_data (editptr);
d246 10
a255 1
  GlobalFreePtr (editptr);
d257 31
a287 2
  return (TRUE);
}
@


1.21
log
@word-wrap, dot-doubling, and default port numbers
@
text
@d3 1
a3 1
 * $Id: wvpost.c 1.20 1994/06/06 22:15:31 rushing Exp rushing $
d14 1
a14 1
#include <windowsx.h>	// for GlobalFreePtr
d35 10
a44 10
	{
	  if (!WndPosts[ih].hWnd)
	    {
	      WndPost = &WndPosts[ih] ;
	      WndPost->hWnd = hWnd;
	      WndPost->Doc = PostDoc ;

	      break;
	    }
	}
d60 1
a60 1
      	CreatePostingText (PostDoc, hWnd, WndPost->hWndEdit, DOCTYPE_POSTING);
d69 1
a69 1
  	case WM_CTLCOLOR:		//WIN32 doesn't handle this message anymore
d84 1
a84 1
		  HIWORD (lParam), TRUE);
d95 3
a97 3
	{
	  NextWindow (WndPost->Doc);
	}
d108 1
a108 1
    if ((HWND)(UINT)lParam == WndPost	->hWndEdit){     
d114 2
a115 2
	     WndPost->dirty = DT_DIRTY ;
	 return(0L) ;
d120 1
a120 1
	case IDM_CANCEL:
d122 19
a140 19
	case IDV_EXIT:
	     DoEditClose(WndPost->hWnd,WndPost->dirty);
	  break;

	case IDM_SEND:
	  StartPost (WndPost);
	  break;

	case IDM_PRINT:
	  PrintFile (WndPost->hWndEdit);
	  break;

	case IDM_ATTACH:
	  if (AskForExistingFileName (hWnd, AttachFileName, "Open file to attach") == FAIL)
	  	break;
       	  if (DialogBox (hInst, "WinVnAttach", hWnd, lpfnWinVnAttachDlg))
     	     Attach (WndPost, AttachFileName, DOCTYPE_POSTING);
	  
	  InvalidateRect (hWnd, NULL, TRUE);
d143 3
a145 3
	default:
	  DoEditCommands (WndPost->hWndEdit, wParam, lParam);
	break;
d159 3
a161 3
 	MessageBox (hWnd,
	  "Please wait until posting is complete",
	  "Cannot close posting window", MB_OK|MB_ICONSTOP);
d163 1
a163 1
	DoEditClose(WndPost->hWnd,WndPost->dirty);
d214 8
@


1.20
log
@oops.
@
text
@d3 1
a3 1
 * $Id: wvpost.c 1.19 1994/06/03 20:44:34 rushing Exp rushing $
d212 1
a212 2
  unsigned int postlen;

d215 3
a217 2
  /* Retrieve the text of the edit window, and run through it
   * line by line, sending the lines to the server.
d222 1
a222 18
/* The data in a TextEdit window is already in netascii format,
 * so it can just be sent to PutCommData(), which is really
 * gensock_put_data(), which is really winsock.send().
 */

  postlen = strlen(editptr);
  PutCommData (editptr, postlen);

  /* this handles the case where the user doesn't hit return
   * on the last line...
   */

  if (editptr[postlen] != '\n') {
    PutCommData ("\r\n.\r\n", 5);
  }
  else {
    PutCommData (".\r\n", 3);
  }
@


1.19
log
@catch missing newline on the end of a post.
@
text
@d3 1
a3 59
 *
 * $Id: wvpost.c 1.18 1994/05/27 01:29:29 rushing Exp rushing $
 * $Log: wvpost.c $
 * Revision 1.18  1994/05/27  01:29:29  rushing
 * unnecessary winundoc.h
 *
 * Revision 1.17  1994/05/26  22:19:08  jglasser
 * warnings
 *
 * Revision 1.16  1994/05/19  02:02:10  rushing
 * changes for gensock & version 0.91
 *
 * Revision 1.15  1994/02/09  18:39:32  rushing
 * ':' missing on case
 *
 * Revision 1.14  1994/02/09  18:01:08  cnolan
 * cnolan 90.2 changes
 *
 * Revision 1.13  1994/01/24  17:40:28  jcoop
 * 90.2 changes
 *
 * Revision 1.12  1994/01/15  20:35:34  jcoop
 * Attachments, font/colors, global editWnd
 *
 * Revision 1.11  1994/01/11  18:51:45  cnolan
 * from cnolan (missing changes)
 * 
 * Revision 1.10  1993/12/08  01:28:01  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.9  1993/08/25  18:53:17  mbretherton
 * MRB merge, mail & post logging
 *
 * Revision 1.8  1993/08/25  17:05:17  mbretherton
 * merge from first newsrc.zip
 *
 * Revision 1.7  1993/07/06  21:09:50  cnolan
 * win32 support
 *
 * Revision 1.6  1993/07/06  17:11:56  rushing
 * plugged doeditcommand in to posting window.
 *
 * Revision 1.5  1993/06/28  17:52:17  rushing
 * fixed compiler warnings
 *
 * Revision 1.4  1993/06/11  00:10:35  rushing
 * second merge from Matt Bretherton sources
 *
 *
 * Revision 1.3  1993/05/25  00:00:45  rushing
 * rot13 merge (MRB)
 *
 * Revision 1.2  1993/05/08  17:11:54  rushing
 * correct handling of NNTP data lines beginning with '.'
 *
 * Revision 1.1  1993/02/16  20:54:22  rushing
 * Initial revision
 *
 *
d234 1
a234 1
  if (editptr[strlen] != '\n') {
d239 1
@


1.18
log
@unnecessary winundoc.h
@
text
@d4 1
a4 1
 * $Id: wvpost.c 1.17 1994/05/26 22:19:08 jglasser Exp rushing $
d6 3
d270 1
d273 1
a273 1
  
d279 1
a279 1
  
d285 12
a296 1
  PutCommData (editptr, strlen(editptr));
a297 1
  PutCommLine (".");
@


1.17
log
@warnings
@
text
@d4 1
a4 1
 * $Id: wvpost.c 1.16 1994/05/19 02:02:10 rushing Exp jglasser $
d6 3
a72 2
#include "winundoc.h"

@


1.16
log
@changes for gensock & version 0.91
@
text
@d4 1
a4 1
 * $Id: wvpost.c 1.15 1994/02/09 18:39:32 rushing Exp $
d6 3
d264 1
a264 2
  char outline[MAXOUTLINE];
  char *outptr;
a265 2
  int linelen;
  BOOL EndBuf = FALSE;
d267 1
a267 2

   CommState = ST_POST_WAIT_END;
a280 31

#if 0
  while (!EndBuf)
    {
      for (linelen = 0, outptr = outline;
	   *editptr && *editptr != '\r' && linelen < MAXOUTLINE;
	   linelen++)
	{ /* special case of a line starting with '.' */
	  if ((outptr == outline) && (*editptr == '.'))
	    *(outptr++) = '.';
	  else
	    *(outptr++) = *(editptr++);
	}

      if (linelen == 0) {
	linelen = 1;
	outline[0] = '\r';
      }

      PutCommLine (outline, linelen);

      if (*editptr == '\r')
	editptr++;
      if (*editptr == '\n')
	editptr++;
      if (!*editptr)
	{
	  EndBuf = TRUE;
	}
    }
#endif
@


1.15
log
@':' missing on case
@
text
@d4 1
a4 1
 * $Id: wvpost.c 1.14 1994/02/09 18:01:08 cnolan Exp rushing $
d6 3
d243 2
a244 2
      strcpy (outline, "POST\r");
      PutCommLine (outline, strlen (outline));
d276 8
d312 1
d314 1
a314 1
  PutCommLine (".\r", 2);
@


1.14
log
@cnolan 90.2 changes
@
text
@d4 1
a4 1
 * $Id: wvpost.c 1.13 1994/01/24 17:40:28 jcoop Exp $
d6 3
d117 1
a117 1
  	case WM_CTLCOLOR		//WIN32 doesn't handle this message anymore
@


1.13
log
@90.2 changes
@
text
@d4 1
a4 1
 * $Id: wvpost.c 1.12 1994/01/15 20:35:34 jcoop Exp $
d6 3
d111 4
a114 1
    case WM_CTLCOLOR:
d117 1
d119 2
a120 2
         SetTextColor(wParam, ArticleTextColor);
         SetBkColor(wParam, ArticleBackgroundColor);
@


1.12
log
@Attachments, font/colors, global editWnd
@
text
@d4 1
a4 1
 * $Id: wvpost.c 1.11 1994/01/11 18:51:45 cnolan Exp $
d6 3
d165 1
a165 1
	  StartPost (hWnd);
d196 6
a201 1
	 DoEditClose(WndPost->hWnd,WndPost->dirty);
a215 2
 *    Entry    ih    is the index into hWndPostEdits of the child
 *                   edit window containing the article.
d218 1
a218 1
StartPost (HWND hWnd)
d223 1
a223 1
  if (!TestCommBusy(hWnd, "Can't post article right now"))
d228 2
a229 2
      CommWnd = hWnd;

@


1.11
log
@from cnolan (missing changes)
@
text
@d4 1
a4 1
 * $Id: wvpost.c 1.10 1993/12/08 01:28:01 rushing Exp $
d6 3
d50 3
a52 2

#include "windows.h"
a66 1

d92 2
d95 2
a96 8
      WndPost->hWndEdit = CreateWindow ("edit", (char *) NULL,
		 WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_BORDER
						  | ES_LEFT | ES_MULTILINE |
					     ES_AUTOHSCROLL | ES_AUTOVSCROLL
						       ,
						       0, 0, 0, 0,
						 hWnd, EDITID, hInst, NULL);
 
d98 2
a99 1
      CreatePostingText (PostDoc, hWnd, WndPost->hWndEdit, DOCTYPE_POSTING);
d105 11
d169 9
d186 2
a189 1
      
d217 1
a217 1
  if (CommBusy)
a218 5
      MessageBox (hWnd, "Sorry, I am already busy retrieving information from the server.\n\
Try again in a little while.", "Can't post article right now", MB_OK | MB_ICONASTERISK);
    }
  else
    {
a244 1
  HANDLE hEditBuffer;
d255 2
a256 3
  hEditBuffer = (HANDLE) SendMessage (WndPost->hWndEdit, EM_GETHANDLE, 0, 0L);
  editptr = LocalLock (hEditBuffer);

d286 2
a287 1
  LocalUnlock (hEditBuffer);
a288 1
  PutCommLine (".\r", 2);
@


1.10
log
@new version box and cr lf consistency
@
text
@d4 1
a4 1
 * $Id: wvpost.c 1.9 1993/08/25 18:53:17 mbretherton Exp rushing $
d6 3
d131 7
a137 4

      if (LOWORD(lParam) == WndPost->hWndEdit)
      {  // Edit Window notifications to parent
	 if (HIWORD(lParam) == EN_CHANGE)
@


1.9
log
@MRB merge, mail & post logging
@
text
@d1 1
d4 1
a4 1
 * $Id: wvpost.c 1.8 1993/08/25 17:05:17 mbretherton Exp $
d6 3
d202 1
a202 1
      strcpy (outline, "POST");
@


1.8
log
@merge from first newsrc.zip
@
text
@d3 1
a3 1
 * $Id: wvpost.c 1.7 1993/07/06 21:09:50 cnolan Exp $
d5 3
d127 1
a127 1
         if (HIWORD(lParam) == EN_CHANGE)
d129 1
a129 1
         return(0L) ;
d134 2
a135 1
       	case IDM_CANCEL:
d150 1
a150 1
        break;
d152 1
d213 2
a214 3
PostText (ih, DocType)
     int ih;
     int DocType;
a215 2

  HWND hWndPostEdit;
d224 2
a225 6
  if (DocType == DOCTYPE_POSTING)
    {
      hWndPostEdit = WndPosts[ih].hWndEdit;
      CommState = ST_POST_WAIT_END;
    }

d230 1
a230 1
  hEditBuffer = (HANDLE) SendMessage (hWndPostEdit, EM_GETHANDLE, 0, 0L);
@


1.7
log
@win32 support
@
text
@d3 1
a3 1
 * $Id: wvpost.c 1.6 1993/07/06 17:11:56 rushing Exp $
d5 3
d52 2
a53 2
  HWND hWndPostEdit;
  int found;
d55 2
a56 1
  char mesbuf[348];		/*** debug */
d58 1
a58 10
  for (ih = 0, found = FALSE; !found && ih < MAXPOSTWNDS; ih++)
    {
      if (hWndPosts[ih] == hWnd)
	{
	  hWndPostEdit = hWndPostEdits[ih];
	  found = TRUE;
	  break;
	}
    }
  if (!found && message == WM_CREATE)
d60 1
a60 1
      for (ih = 0; !found && ih < MAXVIEWS; ih++)
d62 1
a62 1
	  if (!hWndPosts[ih])
d64 4
a67 2
	      found = TRUE;
	      hWndPosts[ih] = hWnd;
d80 1
a80 1
      hWndPostEdit = hWndPostEdits[ih] = CreateWindow ("edit", (char *) NULL,
d87 3
a89 3
      sprintf (mesbuf, "hWndPostEdits[%d] = %5d\r\n       ", ih, hWndPostEdits[ih]);
      /*   MessageBox(hWnd,mesbuf,"Edit Window Results",MB_OK|MB_ICONASTERISK); */
      SetWindowText (hWndPostEdits[ih], mesbuf);
a90 2
      CreatePostingText (PostDoc, hWnd, hWndPostEdit, DOCTYPE_POSTING);
      /*   SetFocus(hWndEdits[ih]);  */
d92 1
a92 2
      SetWindowLong(hWndPostEdit, GWL_WNDPROC, (LONG) lpfnEditSubClass);

d97 1
a97 1
      MoveWindow (hWndPostEdit, 0, 0, LOWORD (lParam),
a98 1

d102 2
a103 1
      SetFocus (hWndPostEdit);
d107 1
d110 1
a110 1
	  NextWindow (&(PostingDocs[ih]));
d115 2
a116 33
 /*                        looks better than WM_MENUSELECT:
                           but possibly WM_INITMENU is better !   */
	{  DWORD dwResult ;
	   UINT action;
	   HMENU hMenu = GetMenu(hWnd);

           dwResult = SendMessage (hWndPostEdit, EM_GETSEL, 0, 0L);

	   if ( HIWORD (dwResult) == LOWORD (dwResult) )
	   {   /* No selection to ROT,Copy or Cut */
	       action=MF_GRAYED ;
	   } else
           {
	       action=MF_ENABLED ;
	   }
           EnableMenuItem(hMenu,IDM_ROT13,action) ;
	   EnableMenuItem(hMenu,IDM_COPY,action) ;
	   EnableMenuItem(hMenu,IDM_CUT,action) ;
	   EnableMenuItem(hMenu,IDM_CLEAR,action) ;

	   if (IsClipboardFormatAvailable(CF_TEXT))
	   { 
	       action=MF_ENABLED ;
           } else
	   {   /* No Clip board contents to paste */
 	       action=MF_GRAYED ;
	   }
           EnableMenuItem(hMenu,IDM_PASTE,action) ;
 
	   /* and should do an EM_CANUNDO to see if Undo available
	     but don't have the edit window handle  */
              
        }
d119 1
d121 8
d130 2
a131 3
	{

	case IDM_CANCEL:
d133 1
a133 1
	  DestroyWindow (hWnd);
d136 2
a137 2
	case IDM_POST:
	  StartPost (ih);
d140 3
a142 3
        case IDM_PRINT:
	  PrintFile (hWndPostEdit);
          break;
d144 4
a147 5
 
 	default:
	  DoEditCommands (hWndPostEdit, wParam, lParam);
	  break;
	}
d151 2
a152 2
      hWndPosts[ih] = 0;
      hWndPostEdits[ih] = 0;
d156 4
d162 1
a162 2
      break;
      
d176 1
a176 2
StartPost (ih)
     int ih;
d183 1
a183 1
      MessageBox (hWndPosts[ih], "Sorry, I am already busy retrieving information from the server.\n\
d191 1
a191 1
      CommDoc = &(PostingDocs[ih]);
d224 1
a224 1
      hWndPostEdit = hWndPostEdits[ih];
d248 2
a249 2
        linelen = 1;
        outline[0] = '\r';
@


1.6
log
@plugged doeditcommand in to posting window.
@
text
@d3 1
a3 1
 * $Id: wvpost.c 1.5 1993/06/28 17:52:17 rushing Exp rushing $
d5 3
d156 1
a156 1
      switch (wParam)
@


1.5
log
@fixed compiler warnings
@
text
@d3 1
a3 1
 * $Id: wvpost.c 1.4 1993/06/11 00:10:35 rushing Exp rushing $
d5 3
d169 3
a171 18
	case IDM_ROT13:
	  { DWORD dwResult ;
	    WORD  wStart, wEnd ;
            char *editptr;
	    HANDLE hEditBuffer;

	    dwResult = SendMessage (hWndPostEdit, EM_GETSEL, 0, 0L);
	    wStart = LOWORD (dwResult) ;
	    wEnd   = HIWORD (dwResult) ;

            hEditBuffer = (HANDLE) SendMessage (hWndPostEdit, EM_GETHANDLE, 0, 0L);
            editptr = LocalLock (hEditBuffer);

	    strnROT13(editptr + wStart, wEnd - wStart) ;

	    LocalUnlock (hEditBuffer);
	    InvalidateRect (hWndPostEdit, NULL, TRUE);
	  }
a172 1

d175 1
a175 2


a177 1

d179 1
a179 1

a181 1

d184 2
@


1.4
log
@second merge from Matt Bretherton sources
@
text
@d3 1
a3 1
 * $Id: wvpost.c 1.3 1993/05/25 00:00:45 rushing Exp $
d5 2
d8 1
d91 1
a91 1
      SetWindowLong(hWndPostEdit, GWL_WNDPROC, (FARPROC) lpfnEditSubClass);
@


1.3
log
@rot13 merge (MRB)
@
text
@d3 1
a3 1
 * $Id: wvpost.c 1.2 1993/05/08 17:11:54 rushing Exp $
d5 4
d87 3
d149 1
a158 25
	case IDM_UNDO:
	  SendMessage (hWndPostEdit, WM_UNDO, 0, 0L);
	  break;

	case IDM_CUT:
	  SendMessage (hWndPostEdit, WM_CUT, 0, 0L);
	  break;

	case IDM_COPY:
	  SendMessage (hWndPostEdit, WM_COPY, 0, 0L);
	  break;

	case IDM_PASTE:
	  SendMessage (hWndPostEdit, WM_PASTE, 0, 0L);
	  break;

	case IDM_CLEAR:
	  SendMessage (hWndPostEdit, WM_CLEAR, 0, 0L);
	  break;

	case IDM_SELALL:
	  SendMessage (hWndPostEdit, EM_SETSEL, 0,
		       MAKELONG (0, 32767));
	  break;

a171 1

@


1.2
log
@correct handling of NNTP data lines beginning with '.'
@
text
@d3 1
a3 1
 * $Id: wvpost.c 1.1 1993/02/16 20:54:22 rushing Exp rushing $
d5 3
d103 36
d177 1
a177 1
          PrintFile (hWndPostEdit);
d179 21
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
d230 9
a238 3
      for (linelen = 0, outptr = outline; *editptr && *editptr != '\r' &&
	   linelen < MAXOUTLINE; linelen++)
	*(outptr++) = *(editptr++);
d244 1
@
