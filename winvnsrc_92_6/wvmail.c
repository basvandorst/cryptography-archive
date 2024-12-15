/*--- WVMAIL.C ------------------------------------------------------------
 *
 *    This file contains the window procedure for the windows
 *    used to edit outgoing mail messages.
 *
 *    Mark Riordan   11 June 1991
 *
 *    MAPI interface implemented by MAtthew Bretherton  March 1993
 */


/*
 * $Id: wvmail.c 1.24 1994/09/19 01:15:21 jcooper Exp $
 * $Log: wvmail.c $
 * Revision 1.24  1994/09/19  01:15:21  jcooper
 * ignore MAPI in auto-select if using win32s
 *
 * Revision 1.23  1994/09/18  22:49:31  jcooper
 * support for cascading windows
 * 
 * Revision 1.22  1994/09/16  01:05:02  jcooper
 * General cleanup for 92.6
 * 
 * Revision 1.21  1994/08/24  18:40:15  mrr
 * authorization enables post/mail
 *
 * Revision 1.20  1994/05/27  01:29:29  rushing
 * unnecessary winundoc.h
 *
 * Revision 1.19  1994/05/27  01:18:47  rushing
 * new smtp
 *
 * Revision 1.18  1994/02/24  21:32:14  jcoop
 * jcoop changes
 *
 * Revision 1.17  1994/02/09  18:39:32  rushing
 * ':' missing on case
 *
 * Revision 1.16  1994/02/09  18:01:08  cnolan
 * cnolan 90.2 changes
 *
 * Revision 1.15  1994/01/24  17:40:28  jcoop
 * 90.2 changes
 *
 * Revision 1.14  1994/01/15  20:32:30  jcoop
 * Attachments, color/font, global editWnd, bugfix in WM_SEND
 *
 * Revision 1.13  1994/01/11  18:26:19  cnolan
 * from conor nolan (missed changes)
 * 
 * Revision 1.12  1993/12/08  01:28:38  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.11  1993/08/25  18:54:36  mbretherton
 * MRB merge, mail & post logging
 *
 * Revision 1.10  1993/06/29  00:22:35  rushing
 * make smtp headers have Date: header
 *
 * Revision 1.9  1993/06/28  17:51:39  rushing
 * fixed compiler warnings
 *
 * Revision 1.8  1993/06/22  19:09:14  rushing
 * mail force type selection via dialog
 *
 * Revision 1.7  1993/06/22  16:45:51  bretherton
 * *** empty log message ***
 *
 * Revision 1.xx MBretherton - auto close mail window on
 *               successful dispatch of mail
 *               & tweak smtp/mapi cotnrol
 * Revision 1.6  1993/06/11  00:11:26  rushing
 * second merge from Matt Bretherton sources
 *
 * Revision 1.5  1993/05/19  23:49:08  rushing
 * notify user after succesfull forward_smtp_message
 *
 * Revision 1.4  1993/05/18  22:10:45  rushing
 * smtp support
 *
 * Revision 1.3  1993/04/27  21:29:23  rushing
 * added rcs log and id magikstrings
 *
 *
 */

#include <windows.h>
#include <windowsx.h>
#include "wvglob.h"
#include "winvn.h"
#pragma hdrstop

mlSMTPSend (HWND hWnd, char *mBuf);
mlSMTPForward (HWND hWnd, TypDoc *Doc);

long FAR PASCAL 
WinVnMailWndProc (hWnd, message, wParam, lParam)
     HWND hWnd;
     unsigned message;
     WPARAM wParam;
     LPARAM lParam;
{
  WndEdit * WndMail;
//  int ih, found;

  WndMail = getWndEdit(WndMails,hWnd,MAXMAILWNDS) ;

  if ((!WndMail) && message == WM_CREATE)
    {
      for (ih = 0; ih < MAXMAILWNDS; ih++)
   {
     if (!WndMails[ih].hWnd)
       {
         WndMail = &WndMails[ih];
         WndMail->hWnd = hWnd;
         WndMail->Doc = MailDoc ;

         break;
       }
   }
    }

  switch (message)
    {
    case WM_SYSCOMMAND:
      return (DefWindowProc (hWnd, message, wParam, lParam));

    case WM_CREATE:
      if (CreateEditWnd (WndMail) == FAIL)
         break;
      
      NumMailWnds++;
  
      SendMessage (WndMail->hWndEdit, WM_SETFONT, (WPARAM)hFontArtNormal, TRUE);
  
      if (!Attaching)
         CreatePostingText (MailDoc, hWnd, WndMail->hWndEdit, DOCTYPE_MAIL);
 
      /* Subclass the edit window to pass accelerators to parent */
      SetWindowLong (WndMail->hWndEdit,
           GWL_WNDPROC,
           (long) lpfnEditSubClass);
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

     MoveWindow (WndMail->hWndEdit, 0, 0, LOWORD (lParam),
        HIWORD (lParam), TRUE);

      break;

    case WM_SETFOCUS:
      SetFocus (WndMail->hWndEdit);
      break;

    case WM_KEYDOWN:
      if (wParam == VK_F6)
   {
     NextWindow (WndMail->Doc);
   }
      break;

    case WM_INITMENUPOPUP:
      prepareEditMenu (hWnd, WndMail->hWndEdit);
      break;

    case WM_COMMAND:
    // Edit Window notifications to parent
    if ((HWND)(UINT)lParam == WndMail->hWndEdit){     
#ifdef WIN32
       if (HIWORD(wParam) == EN_CHANGE)
#else
       if (HIWORD(lParam) == EN_CHANGE)
#endif
        WndMail->dirty = DT_DIRTY ;
       return(0L) ;
    }

      switch (LOWORD(wParam))
   {
   case IDM_CANCEL:
   case IDV_EXIT:
     DoEditClose(WndMail->hWnd,WndMail->dirty);
     break;

   case IDM_SEND:
     StartMail (WndMail);
     break;

   case IDM_ATTACH:
     if (TestDecodeBusy(hWndConf, "Can't make attachment"))
       break;

     if (AskForExistingFileName (hWnd, AttachFileName, "Open file to attach") == FAIL)
      break;
           if (DialogBox (hInst, "WinVnAttach", hWnd, lpfnWinVnAttachDlg))
           Attach (WndMail, AttachFileName, DOCTYPE_MAIL);
     
     InvalidateRect (hWnd, NULL, TRUE);
          break;
   
   default:
     DoEditCommands (WndMail->hWndEdit, wParam, lParam);
     break;
   }
      break;


    case WM_DESTROY:
      WndMail->hWnd = 0;                       
      WndMail->hWndEdit = 0;
      NumMailWnds--;
      break;

    case WM_CLOSE:
      if (CommBusy && MailEdit == WndMail)
   MessageBox (hWnd,
     "Please wait until mail send is complete",
     "Cannot close mail window", MB_OK|MB_ICONSTOP);
      else
    DoEditClose(WndMail->hWnd,WndMail->dirty);
      break;

    default:
      return (DefWindowProc (hWnd, message, wParam, lParam));
    }
  return (0L);
}

/*---- function MailLogout---------------------------------------------
 *
 *   Finish up the mail session
 *
 */
int
MailClose (hWnd) 
    HWND hWnd;
{  
  return ((MailCtrl.fnMlClose)(hWnd)) ;
}

/*---- function MailInit ----------------------------------------------
 *
 *
 *
 */
int MailInit (HWND hWnd) 
{  
  int Mapi_Type;

  if (MailForceType >= 0)
    { /* Accept Private Profile directive to override Mail negotiation */
      MailCtrl.MailType = MailForceType;
    }
  else
    { /* MAPI is next on the negotiation list */
#ifdef WIN32
      if (GetVersion() == 1) /* win32s */
         Mail_Type = 0
      else
#endif
         Mapi_Type =  GetProfileInt("Mail","MAPI",0) ;

      if (Mapi_Type > 0)
   { 
     MailCtrl.MailType = MT_MAPI ;
   } else
   if (strcmp(SMTPHost,""))
   {  /* SMTP next on the negotiation list
         and assume that if SMTP host is specified use it */
     MailCtrl.MailType = MT_SMTP;
   } else 
   {
     MailCtrl.MailType = MT_NONE ;
   } ;
    }

  SetMailType (MailCtrl.MailType) ;

  (MailCtrl.fnMlInit)(hWnd);

  return (0);
}

/* -------------- MailCrashClose -----------------
 *
 *  On serious error in mail interface crash mail and
 * disable mail interface - winvn Ok to run without mail
 *
 * -----------------------------------------------*/

void
MailCrashExit(HWND hWnd)
{
  MailCtrl.fnMlClose(hWnd) ;  // clear down mail resources in use
  SetMailType(MT_NONE)  ;
}

/*---- function MailxxxDefProc -------------------------------------------
 *
 *  Default Proc to support generic nature of dealing with multiple 
 *  mailing interfaces
 *                                                       
 */
MailDefCtlProc (HWND hWnd) {return (0);}
MailDefChrProc (HWND hWnd, char *mBuf) {return (0);}
MailDefDocProc (HWND hWnd, TypDoc * Doc, int DocType) {return (0);}
MailDefDocProc2 (HWND hWnd, TypDoc * Doc) { return 0; }

/* --- SetMailType  ----------------------------------------------------
 *
 *  set up logic attributes for selected mail interface
 *
 */ 

void
SetMailType (int mailType)
{ 
 static BOOL (*NoHeaderFuncs[])(TypDoc * Doc, char *Buf, long int BufLen) =
     {
      NULL
      };
 static BOOL (*MAPIHeaderFuncs[])(TypDoc * Doc, char *Buf, long int BufLen) =
     {
      GetArtHeaderTo, 
      GetArtHeaderSubject,
      NULL 
     };
 static BOOL (*SMTPHeaderFuncs[])(TypDoc * Doc, char *Buf, long int BufLen) =
     {
      MakeArtHeaderTo,
      MakeArtHeaderFrom,
      MakeArtHeaderSubject,
      MakeArtHeaderOrganization,
      MakeArtHeaderDate,
      MakeArtHeaderReplyTo,
      MakeArtHeaderXmailer,
      NULL
     };


  switch (mailType)
  {  
     case MT_MAPI :
   MailCtrl.enableMail = MF_ENABLED ;
   MailCtrl.enableLogout = MF_GRAYED ;
   MailCtrl.enableForward = MF_GRAYED ;
   MailCtrl.fnMlInit=mlMAPIInit ;
   MailCtrl.fnMlLogout=mlMAPILogout ;
   MailCtrl.fnMlClose=mlMAPIClose;
   MailCtrl.fnMlSend=MailDefChrProc;
   MailCtrl.fnMlForward=MailDefDocProc2;
   MailCtrl.fnMlWinCreate=mlMAPISend ;
   MailCtrl.MailHeaderFuncs = MAPIHeaderFuncs ;
   break ;
      case MT_SMTP :
   MailCtrl.enableMail = MF_ENABLED ;
   MailCtrl.enableLogout = MF_GRAYED;
   MailCtrl.enableForward = MF_ENABLED ;   
   MailCtrl.fnMlInit=MailCtrl.fnMlLogout=MailDefCtlProc ;
   MailCtrl.fnMlLogout=MailDefCtlProc;
   MailCtrl.fnMlClose=MailDefCtlProc;
   MailCtrl.fnMlSend=mlSMTPSend;
   MailCtrl.fnMlForward=mlSMTPForward;
   MailCtrl.fnMlWinCreate=(int (*)(HWND, TypDoc *, int))CreatePostingWnd;
   MailCtrl.MailHeaderFuncs = SMTPHeaderFuncs;
   break;
   /* error handling for Fall through case message box plus */
   /* default to none */ 
     case MT_NONE :
     default:
   MailCtrl.enableMail = MF_GRAYED ;
   MailCtrl.enableLogout = MF_GRAYED;
   MailCtrl.enableForward = MF_GRAYED ;
   MailCtrl.fnMlClose=MailDefCtlProc;
   MailCtrl.fnMlInit=MailCtrl.fnMlLogout=MailDefCtlProc ;
   MailCtrl.fnMlSend=MailDefChrProc ;
   MailCtrl.fnMlForward=MailDefDocProc2;
   MailCtrl.fnMlWinCreate=MailDefDocProc;
   MailCtrl.MailHeaderFuncs = NoHeaderFuncs ;
   break ;
  } ;
}

/* -------------- MailEnableLogout ------------------------
 *
 *  Allow discard (freeing) of mail resources during
 *  the session - Not necessarily available to all mail
 *  services
 *
 *--------------------------------------------------------*/  
void
MailEnableLogout(BOOL enable)
{
  if (enable) MailCtrl.enableLogout = MF_ENABLED ;
  else        MailCtrl.enableLogout = MF_GRAYED ;
}

/*--- function StartMail ----------------------------------------------
 *
 *    Start the Mailing procedure for an article that has been created
 *    by the user.
 */
BOOL
StartMail (WndEdit *thisMail)
{
  BOOL MailOK = FALSE ;
  char *editptr;
  char title[MAXINTERNALLINE];
  
  GetWindowText(thisMail->hWnd, title, MAXINTERNALLINE-1);
  strcpy(str, title);
  strcat(str, "  [Mailing...]");
  SetWindowText (thisMail->hWnd, str);

  MailEdit = thisMail;
  /* If authentication was required, stuff a header line into
   * the beginning of the message so that people reading it can
   * know who really posted it.  (The From: line may be a lie.)
   */
  if(AuthReqMail) {
    AddAuthHeader(MailEdit->hWndEdit);
  }
  
  // lock out changes
  SendMessage(MailEdit->hWndEdit, EM_SETREADONLY, TRUE, 0L);

  editptr = GetEditText (MailEdit->hWndEdit);
  
  MailOK=(MailCtrl.fnMlSend)(MailEdit->hWnd, editptr) ;

  if (!MailOK)
     // unlock to allow user modification
     SendMessage(MailEdit->hWndEdit, EM_SETREADONLY, FALSE, 0L);

  GlobalFreePtr (editptr);         
  
  SetWindowText (thisMail->hWnd, title);
  return (MailOK);
}

/*--- function GetMailHeaderString ------------------------------------
 *
 *    mailMsg   points to the mail message buffer (null terminated 
 *              string)
 *
 *              Prefix   is the character string which will identify the
 *                       line we are seeking.  It is the first word
 *                       (blank-delimited) in a line in the header.
 *              BufSize   is the number of bytes left in the buffer Buf.
 *
 *    Exit      Returns  TRUE if we returned a line of this prefix.
 *              Buf      if line was found, contains the line less
 *                       the Prefix null terminated.
 * 
  */
BOOL
GetMailHeaderString (mailMsg, Prefix, Buf, BufSize)
     char *mailMsg;
     char *Prefix;
     char *Buf;
     int BufSize;
{
  BOOL found = FALSE, got_To = FALSE;
  char outline[MAXOUTLINE];
  char  *outptr;
  int linelen;

 
      while (!got_To && *mailMsg)
   {
     for (linelen = 0, outptr = outline; *mailMsg && *mailMsg != '\r' &&
          linelen < MAXOUTLINE; linelen++)
       *(outptr++) = *(mailMsg++);
     *outptr = '\0';

     if (CompareStringNoCase (outline, Prefix, strlen(Prefix)) == 0)
       {
         got_To = TRUE;
      }
     if (*mailMsg='\r')
      {
     mailMsg++ ; mailMsg++ ;  /* jump CRLF */
     }
   }

      if (got_To)
   {
     outptr = outline;
     NextToken (&outptr);
     while (--BufSize > 1 && (*(Buf++) = *(outptr++)));
     found = TRUE;
   }
   *Buf = '\0';       /* whatever happens Buf is null delim string */
       
  return (found);
}

mlSMTPSend (HWND hWnd, char *mBuf )
{
  BOOL Addressed = FALSE;
  char Addressee[MAXOUTLINE];
// int res ;

  Addressed=GetMailHeaderString (mBuf, "To: ", Addressee, MAXOUTLINE) ;

  if (!Addressed)
    {
      MessageBox (hWnd, "Fill in the \"To:\" line", "Need a recipient",
        MB_OK | MB_ICONHAND);
      return(FALSE) ; 
    }
   
  if (!send_smtp_message (Addressee,mBuf))
  {
 //   res=MessageBox (hWnd, "Mail delivered to server", "WinVN", MB_OKCANCEL);
 //   if (res==ID_OK)
     if (MailLog) WriteEditLog(hWnd,MailLogFile,mBuf, lstrlen (mBuf)) ;
     
     MailEdit->dirty=DT_CLEAN ;
     MailEdit = (WndEdit *) NULL;
     
     DestroyWindow(hWnd) ;
     
     if (nextBatchIndex)      // if we're sending a batch, send the next
        BatchSend (DOCTYPE_MAIL);
     
     return (TRUE);
  }
  else
  {
    nextBatchIndex = 0;
    return (FALSE);
  }
}

int mlSMTPForward (HWND hWnd, TypDoc * Doc)
{

  DialogString[0]=(char)NULL;
  while (!strcmp(DialogString,""))
    if (!DialogBox (hInst, "WinVnMail", hWndConf, lpfnWinVnMailDlg))
      return (0);

  if (!forward_smtp_message (DialogString, Doc)) {
    MessageBox (hWnd, "Mail delivered to server", "WinVN", MB_OK);
    return (1);
  }
  else 
    return (0);
  
//  else {
//    MessageBox (hWndConf, "You must enter a recipient", "WinVN", MB_OK);
//    return (0);
//  }
}
