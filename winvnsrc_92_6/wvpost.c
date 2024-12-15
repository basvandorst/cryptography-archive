
/*
 * $Id: wvpost.c 1.24 1994/09/18 22:49:37 jcooper Exp $
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
