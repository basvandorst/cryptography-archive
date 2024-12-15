head	1.21;
access;
symbols
	V80:1.6
	V76d:1.1;
locks; strict;
comment	@ * @;


1.21
date	94.08.24.18.40.15;	author mrr;	state Exp;
branches;
next	1.20;

1.20
date	94.05.27.01.29.29;	author rushing;	state Exp;
branches;
next	1.19;

1.19
date	94.05.27.01.18.47;	author rushing;	state Exp;
branches;
next	1.18;

1.18
date	94.02.24.21.32.14;	author jcoop;	state Exp;
branches;
next	1.17;

1.17
date	94.02.09.18.39.32;	author rushing;	state Exp;
branches;
next	1.16;

1.16
date	94.02.09.18.01.08;	author cnolan;	state Exp;
branches;
next	1.15;

1.15
date	94.01.24.17.40.28;	author jcoop;	state Exp;
branches;
next	1.14;

1.14
date	94.01.15.20.32.30;	author jcoop;	state Exp;
branches;
next	1.13;

1.13
date	94.01.11.18.26.19;	author cnolan;	state Exp;
branches;
next	1.12;

1.12
date	93.12.08.01.28.38;	author rushing;	state Exp;
branches;
next	1.11;

1.11
date	93.08.25.18.54.36;	author mbretherton;	state Exp;
branches;
next	1.10;

1.10
date	93.06.29.00.22.35;	author rushing;	state Exp;
branches;
next	1.9;

1.9
date	93.06.28.17.51.39;	author rushing;	state Exp;
branches;
next	1.8;

1.8
date	93.06.22.19.09.14;	author rushing;	state Exp;
branches;
next	1.7;

1.7
date	93.06.22.16.45.51;	author bretherton;	state Exp;
branches;
next	1.6;

1.6
date	93.06.11.00.11.26;	author rushing;	state Exp;
branches;
next	1.5;

1.5
date	93.05.19.23.49.08;	author rushing;	state Exp;
branches;
next	1.4;

1.4
date	93.05.18.22.10.45;	author rushing;	state Exp;
branches;
next	1.3;

1.3
date	93.04.27.21.29.23;	author rushing;	state Exp;
branches;
next	1.2;

1.2
date	93.03.30.20.09.08;	author rushing;	state Exp;
branches;
next	1.1;

1.1
date	93.02.16.20.54.22;	author rushing;	state Exp;
branches;
next	;


desc
@winvn version 0.76 placed into RCS
@


1.21
log
@authorization enables post/mail
@
text
@/*--- WVMAIL.C ------------------------------------------------------------
 *
 *    This file contains the window procedure for the windows
 *    used to edit outgoing mail messages.
 *
 *    Mark Riordan   11 June 1991
 *
 *    MAPI interface implemented by MAtthew Bretherton  March 1993
 */


/*
 * $Id: wvmail.c 1.20 1994/05/27 01:29:29 rushing Exp $
 * $Log: wvmail.c $
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

#include "windows.h"
#include <windowsx.h>   // for GlobalFreePtr
#include "wvglob.h"
#include "winvn.h"

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
  
      SendMessage (WndMail->hWndEdit, WM_SETFONT, hFontArtNormal, TRUE);
  
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
         return (DWORD)hArticleBackgroundBrush;
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
   MailCtrl.fnMlWinCreate=CreatePostingWnd ;
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
    if (!DialogBoxParam (hInst, "WinVnMail", hWndConf, lpfnWinVnMailDlg, (LPARAM) NULL))
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
@


1.20
log
@unnecessary winundoc.h
@
text
@d13 1
a13 1
 * $Id: wvmail.c 1.19 1994/05/27 01:18:47 rushing Exp rushing $
d15 3
d76 1
a76 1
#include <windowsx.h>	// for GlobalFreePtr
d98 10
a107 10
	{
	  if (!WndMails[ih].hWnd)
	    {
 	      WndMail = &WndMails[ih];
	      WndMail->hWnd = hWnd;
	      WndMail->Doc = MailDoc ;

	      break;
	    }
	}
d126 2
a127 2
		     GWL_WNDPROC,
		     (long) lpfnEditSubClass);
d133 1
a133 1
  	case WM_CTLCOLOR:		//WIN32 doesn't handle this message anymore
d148 1
a148 1
		  HIWORD (lParam), TRUE);
d158 3
a160 3
	{
	  NextWindow (WndMail->Doc);
	}
d175 1
a175 1
	     WndMail->dirty = DT_DIRTY ;
d180 17
a196 17
	{
	case IDM_CANCEL:
	case IDV_EXIT:
	  DoEditClose(WndMail->hWnd,WndMail->dirty);
	  break;

	case IDM_SEND:
	  StartMail (WndMail);
	  break;

	case IDM_ATTACH:
	  if (AskForExistingFileName (hWnd, AttachFileName, "Open file to attach") == FAIL)
	  	break;
       	  if (DialogBox (hInst, "WinVnAttach", hWnd, lpfnWinVnAttachDlg))
     	     Attach (WndMail, AttachFileName, DOCTYPE_MAIL);
	  
	  InvalidateRect (hWnd, NULL, TRUE);
d198 5
a202 5
	
	default:
	  DoEditCommands (WndMail->hWndEdit, wParam, lParam);
	  break;
	}
d214 3
a216 3
 	MessageBox (hWnd,
	  "Please wait until mail send is complete",
	  "Cannot close mail window", MB_OK|MB_ICONSTOP);
d218 1
a218 1
	 DoEditClose(WndMail->hWnd,WndMail->dirty);
d257 11
a267 11
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
d312 3
a314 3
	  {
		NULL
	   };
d316 5
a320 5
	  {
		GetArtHeaderTo, 
		GetArtHeaderSubject,
		NULL 
	  };
d322 10
a331 10
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
d337 11
a347 11
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
d349 13
a361 13
	MailCtrl.enableMail = MF_ENABLED ;
	MailCtrl.enableLogout = MF_GRAYED;
	MailCtrl.enableForward = MF_ENABLED ;   
	MailCtrl.fnMlInit=MailCtrl.fnMlLogout=MailDefCtlProc ;
	MailCtrl.fnMlLogout=MailDefCtlProc;
	MailCtrl.fnMlClose=MailDefCtlProc;
	MailCtrl.fnMlSend=mlSMTPSend;
	MailCtrl.fnMlForward=mlSMTPForward;
	MailCtrl.fnMlWinCreate=CreatePostingWnd ;
	MailCtrl.MailHeaderFuncs = SMTPHeaderFuncs;
	break;
	/* error handling for Fall through case message box plus */
	/* default to none */ 
d364 10
a373 10
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
d403 8
d456 11
a466 11
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
d468 3
a470 3
	  mailMsg++ ; mailMsg++ ;  /* jump CRLF */
	  }
	}
d473 8
a480 8
	{
	  outptr = outline;
	  NextToken (&outptr);
	  while (--BufSize > 1 && (*(Buf++) = *(outptr++)));
	  found = TRUE;
	}
	*Buf = '\0';       /* whatever happens Buf is null delim string */
	    
d495 1
a495 1
		  MB_OK | MB_ICONHAND);
d510 1
a510 1
     if (nextBatchIndex)		// if we're sending a batch, send the next
@


1.19
log
@new smtp
@
text
@d13 1
a13 1
 * $Id: wvmail.c 1.18 1994/02/24 21:32:14 jcoop Exp rushing $
d15 3
a75 1
#include "winundoc.h"
@


1.18
log
@jcoop changes
@
text
@d13 1
a13 1
 * $Id: wvmail.c 1.17 1994/02/09 18:39:32 rushing Exp $
d15 3
d407 1
a407 1
     SendMessage(PostEdit->hWndEdit, EM_SETREADONLY, FALSE, 0L);
d486 1
a486 1
  if (send_smtp_message (Addressee,mBuf))
d517 1
a517 1
  if (forward_smtp_message (DialogString, Doc)) {
@


1.17
log
@':' missing on case
@
text
@d13 1
a13 1
 * $Id: wvmail.c 1.16 1994/02/09 18:01:08 cnolan Exp rushing $
d15 3
d320 1
d401 5
d482 1
a482 1

d509 3
a511 8
  DialogBoxParam (hInst, "WinVnGeneric", hWnd, lpfnWinVnGenericDlg, 
		  (LPARAM) (char far *) "Enter recipient's email address:");
  if (strcmp(DialogString, "")) {
    if (forward_smtp_message (DialogString, Doc)) {
      MessageBox (hWnd, "Mail delivered to server", "WinVN", MB_OK);
      return (1);
  }
    else 
d513 4
d518 1
a518 2
  else {
    MessageBox (hWndConf, "You must enter a recipient", "WinVN", MB_OK);
d520 5
a524 1
  }
@


1.16
log
@cnolan 90.2 changes
@
text
@d13 1
a13 1
 * $Id: wvmail.c 1.15 1994/01/24 17:40:28 jcoop Exp $
d15 3
d122 1
a122 1
  	case WM_CTLCOLOR		//WIN32 doesn't handle this message anymore
@


1.15
log
@90.2 changes
@
text
@d13 1
a13 1
 * $Id: wvmail.c 1.14 1994/01/15 20:32:30 jcoop Exp $
d15 3
d116 4
a119 1
    case WM_CTLCOLOR:
d122 1
d124 2
a125 2
         SetTextColor(wParam, ArticleTextColor);
         SetBkColor(wParam, ArticleBackgroundColor);
@


1.14
log
@Attachments, color/font, global editWnd, bugfix in WM_SEND
@
text
@d13 1
a13 1
 * $Id: wvmail.c 1.13 1994/01/11 18:26:19 cnolan Exp $
d15 3
d192 5
a372 2
 *    MailWnd is the EditStructure containing info on this particular 
 *    mail window
d375 1
a375 1
StartMail (WndEdit *MailWnd)
d379 4
a382 1
  HWND hWndMailEdit = MailWnd->hWndEdit;
d384 4
a387 1
  editptr = GetEditText (hWndMailEdit);
a388 2
  MailOK=(MailCtrl.fnMlSend)(MailWnd->hWnd,editptr) ;
  GlobalFreePtr (editptr);
d469 4
a472 1
     getWndEdit(WndMails,hWnd,MAXMAILWNDS)->dirty=DT_CLEAN ;
d474 4
d481 2
d484 1
@


1.13
log
@from conor nolan (missed changes)
@
text
@d13 1
a13 1
 * $Id: wvmail.c 1.12 1993/12/08 01:28:38 rushing Exp $
d15 3
d55 1
d96 7
a102 11

      WndMail->hWndEdit = CreateWindow ("edit", (char *) NULL,
		      WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_BORDER
		      | ES_LEFT | ES_MULTILINE |
		      ES_AUTOHSCROLL | ES_AUTOVSCROLL
		      ,
		      0, 0, 0, 0,
		      hWnd, EDITID, hInst, NULL);

     WndMail->dirty = DT_CLEAN ;
     CreatePostingText (MailDoc, hWnd, WndMail->hWndEdit, DOCTYPE_MAIL);
d108 1
d110 9
d163 1
a163 1
	  StartMail (hWnd, ih);
d166 9
d183 1
a183 1
      WndMail->hWnd = 0;
d365 2
a366 3
 *
 *    Entry    ih    is the index into hWndMailEdits of the child
 *                   edit window containing the article.
d369 1
a369 3
StartMail (hWnd , ih)
    HWND hWnd ;
    int ih;
d373 1
a373 2
  HANDLE hEditBuffer;
  HWND hWndMailEdit = WndMails[ih].hWndEdit;
d375 4
a378 6
  hEditBuffer = (HANDLE) SendMessage (hWndMailEdit, EM_GETHANDLE, 0, 0L);
  editptr = LocalLock (hEditBuffer);

  MailOK=(MailCtrl.fnMlSend)(hWnd,editptr) ;

  LocalUnlock (hEditBuffer);
d458 1
a458 1
     if (MailLog) WriteEditLog(hWnd,MailLogFile,mBuf) ;
@


1.12
log
@new version box and cr lf consistency
@
text
@d13 1
a13 1
 * $Id: wvmail.c 1.11 1993/08/25 18:54:36 mbretherton Exp rushing $
d15 3
d134 5
a138 3

    if (LOWORD(lParam) == WndMail->hWndEdit)
    {  // Edit Window notifications to parent
d140 1
d145 1
a145 1
      switch (wParam)
@


1.11
log
@MRB merge, mail & post logging
@
text
@d11 1
d13 1
a13 1
 * $Id: wvmail.c 1.10 1993/06/29 00:22:35 rushing Exp $
d15 3
@


1.10
log
@make smtp headers have Date: header
@
text
@d12 1
a12 1
 * $Id: wvmail.c 1.9 1993/06/28 17:51:39 rushing Exp rushing $
d14 3
d59 2
a60 2
  HWND hWndMailEdit;
  int ih, found;
d62 1
a62 5
  for (ih = 0, found = FALSE; !found && ih < MAXMAILWNDS; ih++)
    {
      if (hWndMails[ih] == hWnd)
	{
	  hWndMailEdit = hWndMailEdits[ih];
d64 1
a64 5
	  found = TRUE;
	  break;
	}
    }
  if (!found && message == WM_CREATE)
d66 1
a66 1
      for (ih = 0; !found && ih < MAXVIEWS; ih++)
d68 1
a68 1
	  if (!hWndMails[ih])
d70 4
a73 2
	      found = TRUE;
	      hWndMails[ih] = hWnd;
d86 1
a86 2
      hWndMailEdit = hWndMailEdits[ih] =
	CreateWindow ("edit", (char *) NULL,
d94 3
a96 2
      CreatePostingText (MailDoc, hWnd, hWndMailEdit, DOCTYPE_MAIL);

d98 1
a98 1
      SetWindowLong (hWndMailEdit,
d106 1
a106 1
     MoveWindow (hWndMailEdit, 0, 0, LOWORD (lParam),
d112 1
a112 1
      SetFocus (hWndMailEdit);
d118 1
a118 1
	  NextWindow (&(MailDocs[ih]));
d123 1
a123 1
      prepareEditMenu (hWnd, hWndMailEdit);
d127 8
d139 1
a139 1
	  DestroyWindow (hWnd);
d142 1
a142 1
	case IDM_POST:
d147 1
a147 1
	  DoEditCommands (hWndMailEdit, wParam, lParam);
d154 2
a155 2
    hWndMails[ih] = 0;
    hWndMailEdits[ih] = 0;
d159 3
d202 1
a202 1
        if (strcmp(SMTPHost,""))
d205 2
a206 2
          MailCtrl.MailType = MT_SMTP;
        } else 
d292 1
a292 1
	MailCtrl.enableForward = MF_ENABLED ;	
d348 1
a348 1
  HWND hWndMailEdit = hWndMailEdits[ih];
d403 1
a403 1
 
d431 2
a432 1
  if (send_smtp_message (Addressee,mBuf)) {
d435 4
a438 2
      DestroyWindow(hWnd) ;
    return (TRUE);
@


1.9
log
@fixed compiler warnings
@
text
@d12 1
a12 1
 * $Id: wvmail.c 1.8 1993/06/22 19:09:14 rushing Exp rushing $
d14 3
d261 1
a261 1
//		MakeArtHeaderDate,
@


1.8
log
@mail force type selection via dialog
@
text
@d12 1
a12 1
 * $Id: wvmail.c 1.7 1993/06/22 16:45:51 bretherton Exp rushing $
d14 3
a54 2
  char mesbuf[348];		/*** debug */

a94 2
/*      SetWindowText (hWndMailEdits[ih], mesbuf); */

d98 3
a100 1
      SetWindowLong (hWndMailEdit, GWL_WNDPROC, lpfnEditSubClass);
d163 1
d167 1
a167 1
  (MailCtrl.fnMlClose)(hWnd) ;
d175 1
a175 2
MailInit (hWnd) 
    HWND hWnd;
d228 5
a232 18
MailDefCtlProc (hWnd) 
    HWND hWnd;
{  
   return (0);
}
MailDefChrProc (hWnd, mBuf) 
    HWND hWnd;
    char *mBuf;
{  
   return (0);
}
MailDefDocProc (hWnd, Doc, DocType) 
   HWND hWnd ;
   TypDoc *Doc;
   int DocType;
{  
   return (0);
}
d273 2
a274 4
	MailCtrl.fnMlSend=MailCtrl.fnMlForward=MailDefChrProc  ;
	MailCtrl.fnMlForward=MailDefDocProc;
		    // Easy to do but best get ver 80 out the door
                    // M Bretherton 
d299 2
a300 2
	MailCtrl.fnMlSend=MailCtrl.fnMlForward=MailDefChrProc ;
	MailCtrl.fnMlForward=MailDefDocProc;
d434 1
a434 1
		  (char far *) "Enter recipient's email address:");
@


1.7
log
@*** empty log message ***
@
text
@d12 1
a12 1
 * $Id: wvmail.c 1.6 1993/06/11 00:11:26 rushing Exp $
d14 3
d178 1
a178 1
  if (MailForceType > 0)
@


1.6
log
@second merge from Matt Bretherton sources
@
text
@d12 1
a12 1
 * $Id: wvmail.c 1.5 1993/05/19 23:49:08 rushing Exp rushing $
d14 6
a180 1

d186 6
a191 2
	}
      else
a195 3
  /* We assume that if they specify an SMTP host they want SMTP */
  if (strcmp(SMTPHost,""))
    MailCtrl.MailType = MT_SMTP;
d267 1
d278 1
d283 3
d291 2
a292 1
	MailCtrl.enableLogout = MF_GRAYED;	
d307 1
d311 1
d420 1
d432 3
a434 1
    MessageBox (hWnd, "Mail delivered to server", "WinVN", MB_OK);
@


1.5
log
@notify user after succesfull forward_smtp_message
@
text
@d12 1
a12 1
 * $Id: wvmail.c 1.4 1993/05/18 22:10:45 rushing Exp rushing $
d14 3
d88 4
a91 1
      
d112 4
d128 2
a129 18
	case IDM_UNDO:
	  SendMessage (hWndMailEdit, WM_UNDO, 0, 0L);
	  break;

	case IDM_CUT:
	  SendMessage (hWndMailEdit, WM_CUT, 0, 0L);
	  break;

	case IDM_COPY:
	  SendMessage (hWndMailEdit, WM_COPY, 0, 0L);
	  break;

	case IDM_PASTE:
	  SendMessage (hWndMailEdit, WM_PASTE, 0, 0L);
	  break;

	case IDM_CLEAR:
	  SendMessage (hWndMailEdit, WM_CLEAR, 0, 0L);
a130 5

	case IDM_SELALL:
	  SendMessage (hWndMailEdit, EM_SETSEL, 0,
		       MAKELONG (0, 32767));
	  break;
d153 1
a153 1
MailLogout (hWnd) 
d156 1
a156 1
  (MailCtrl.fnMlLogout)(hWnd) ;
d169 6
a174 1
  Mapi_Type =  GetProfileInt("Mail","MAPI",0) ;
d176 1
a176 8
  if (Mapi_Type == 1)
  { 
     MailCtrl.MailType = MT_MAPI ;
  }
  else
  {
     MailCtrl.MailType = MT_NONE ;
  } ;
d178 9
d193 1
a193 5
  if ((MailCtrl.fnMlInit)(hWnd))
    {
      /* On failure to initialise mail interface disable said interface */ 
      SetMailType(MT_NONE)  ;
    } ;
d197 15
d242 2
a243 2
SetMailType (mailType)
  int mailType ;
a267 7
     case MT_NONE :
	MailCtrl.enableMail = MF_GRAYED ;
	MailCtrl.fnMlInit=MailCtrl.fnMlLogout=MailDefCtlProc ;
	MailCtrl.fnMlSend=MailCtrl.fnMlForward=MailDefChrProc ;
	MailCtrl.fnMlWinCreate=MailDefDocProc;
	MailCtrl.MailHeaderFuncs = NoHeaderFuncs ;
	break ;
d270 1
d273 1
d280 1
d282 2
a288 1
	
d291 10
d303 15
@


1.4
log
@smtp support
@
text
@d12 1
a12 1
 * $Id: wvmail.c 1.3 1993/04/27 21:29:23 rushing Exp rushing $
d14 3
d401 2
a402 1
    if (forward_smtp_message (DialogString, Doc))
d404 1
@


1.3
log
@added rcs log and id magikstrings
@
text
@d12 4
a15 2
 * $Id$
 * $Log$
d17 1
d25 2
d70 10
a79 10
	 hWndMailEdit = hWndMailEdits[ih] = CreateWindow ("edit", (char *) NULL,
		 WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_BORDER
						  | ES_LEFT | ES_MULTILINE |
					     ES_AUTOHSCROLL | ES_AUTOVSCROLL
						       ,
						       0, 0, 0, 0,
						 hWnd, EDITID, hInst, NULL);
      sprintf (mesbuf, "hWndMailEdits[%d] = %5d\r\n       ", ih, hWndMailEdits[ih]);
      /*   MessageBox(hWnd,mesbuf,"Edit Window Results",MB_OK|MB_ICONASTERISK); */
      SetWindowText (hWndMailEdits[ih], mesbuf);
d82 1
a82 1
      /*   SetFocus(hWndEdits[ih]);  */
d175 1
a175 1
  int Mapi_Type ;
d188 3
a190 7
/* mrb debug */
  if (Mapi_Type == 2)
  { 
     MailCtrl.MailType = MT_POP ;
  }
/* end mrb debug */

d245 1
a245 1
 static BOOL (*POPHeaderFuncs[])(TypDoc * Doc, char *Buf, long int BufLen) =
d251 1
d255 1
d261 1
a261 1
    MailCtrl.fnMlSend=MailDefChrProc ;
a263 1

d269 1
a269 1
	MailCtrl.fnMlSend=MailDefChrProc  ;
d271 1
a271 1
    MailCtrl.MailHeaderFuncs = MAPIHeaderFuncs ;
d273 1
a273 1
     case MT_POP :
d276 2
a277 1
	MailCtrl.fnMlSend=mlPOPSend ;
d279 5
a283 4
    MailCtrl.MailHeaderFuncs = POPHeaderFuncs  ;
	break ;
     /* error handling for Fall through case 
	message box plus default to none */
a303 1

a306 1

d336 2
a337 2
  char outline[MAXOUTLINE], cmdline[MAXOUTLINE];
  char  *outptr, *cptr;
d370 3
a372 6
 mlPOPSend (hWnd ,mBuf )
    HWND hWnd;
    char *mBuf;
 {
  BOOL found;
  BOOL MailOK = FALSE, got_To = FALSE, Addressed = FALSE;
a373 3
  char outline[MAXOUTLINE], cmdline[MAXOUTLINE];
  char  *outptr, *cptr;
  int linelen;
d375 1
a375 1
    MessageBox (hWnd, "POP Send", "Debug!", MB_OK | MB_ICONHAND);
d377 1
a377 11
    Addressed=GetMailHeaderString (mBuf, "To: ", Addressee, MAXOUTLINE) ;

     if (!Addressed)
	{
	  MessageBox (hWnd, "Fill in the \"To:\" line", "Need a recipient",
		      MB_OK | MB_ICONHAND);
	      return(FALSE) ; 
	}

										  
  if (CommBusy)
d379 3
a381 2
      MessageBox (hWndMails[ih], "Sorry, I am already busy retrieving information from the server.\n\
Try again in a little while.", "Can't mail message right now", MB_OK | MB_ICONASTERISK);
d383 5
d389 2
a390 31
    {
      CommLinePtr = CommLineIn;
      CommBusy = TRUE;
      CommState = ST_MAIL_WAIT_PERMISSION;
      CommDoc = &(MailDocs[ih]);

      /* Create XMAIL command.  We need to extract the "To:" line
       * in order to find the recipient for the command.
       */


   strcpy (cmdline, "xMail ");
   outptr = Addressee ;
   for (cptr = cmdline + strlen (cmdline);
	 *outptr > ' ' && cptr - cmdline < MAXOUTLINE;
	 *(cptr++) = *(outptr++));
   *cptr++ = '\r' ;
   *cptr++ = '\n' ;
   *cptr = '\0';
   outptr = mBuf  ;
   for (cptr = cmdline + strlen (cmdline);
	    cptr - cmdline < MAXOUTLINE;
	 *(cptr++) = *(outptr++));
   *cptr = '\0';

  MessageBox (hWnd, cmdline, "XMail Message", MB_OK | MB_ICONHAND);
/*   This is silly ->
   Originally only put out the header
   But on my chnage only puts out a limited number of characters
   Should be line by line
			  mrb */
d392 2
a393 2
   PutCommLine (cmdline, strlen (cmdline));
   MailOK = TRUE;
d395 13
a407 4
    } 

return(MailOK) ;
 }
@


1.2
log
@MAPI
@
text
@d11 6
@


1.1
log
@Initial revision
@
text
@a0 7
/*
 *
 * $Id$
 * $Log$
 *
 */

d7 2
a23 1

d28 1
d33 1
a34 1
	  hWndMailEdit = hWndMailEdits[ih];
d59 1
a59 1
      hWndMailEdit = hWndMailEdits[ih] = CreateWindow ("edit", (char *) NULL,
d76 1
a76 1
      MoveWindow (hWndMailEdit, 0, 0, LOWORD (lParam),
a91 1

d101 1
a101 1
	  StartMail (ih);
d133 2
a134 3
      hWndMails[ih] = 0;

      hWndMailEdits[ih] = 0;
d145 131
d285 3
a287 2
StartMail (ih)
     int ih;
d289 2
a290 3
  BOOL MailOK = FALSE, got_To = FALSE;
  char outline[MAXOUTLINE], cmdline[MAXOUTLINE];
  char *editptr, *outptr, *cptr;
d293 78
d373 12
d401 24
a424 2
      hEditBuffer = (HANDLE) SendMessage (hWndMailEdit, EM_GETHANDLE, 0, 0L);
      editptr = LocalLock (hEditBuffer);
d426 1
a426 12
      while (!got_To && *editptr)
	{
	  for (linelen = 0, outptr = outline; *editptr && *editptr != '\r' &&
	       linelen < MAXOUTLINE; linelen++)
	    *(outptr++) = *(editptr++);
	  *outptr = '\0';
	  if (CompareStringNoCase (outline, "To: ", 4) == 0)
	    {
	      got_To = TRUE;
	    }
	}
      LocalUnlock (hEditBuffer);
d428 2
a429 20
      if (!got_To)
	{
	  MessageBox (hWndMails[ih], "Fill in the \"To:\" line", "Need a recipient",
		      MB_OK | MB_ICONHAND);
	}
      else
	{
	  outptr = outline;
	  NextToken (&outptr);
	  strcpy (cmdline, "xMail ");
	  for (cptr = cmdline + strlen (cmdline);
	       *outptr > ' ' && cptr - cmdline < MAXOUTLINE;
	       *(cptr++) = *(outptr++));
	  *cptr = '\0';
	  PutCommLine (cmdline, strlen (cmdline));
	  MailOK = TRUE;
	}
    }
  return (MailOK);
}
@
