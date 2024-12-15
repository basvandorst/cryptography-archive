head	 1.11;
branch   ;
access   ;
symbols  V80:1.4;
locks    ; strict;
comment  @ * @;


1.11
date	 94.09.19.07.52.52;  author jcooper;  state Exp;
branches ;
next	 1.10;

1.10
date	 94.01.24.17.41.01;  author jcoop;  state Exp;
branches ;
next	 1.9;

1.9
date	 93.12.08.01.28.38;  author rushing;  state Exp;
branches ;
next	 1.8;

1.8
date	 93.08.25.18.54.36;  author mbretherton;  state Exp;
branches ;
next	 1.7;

1.7
date	 93.06.28.17.52.17;  author rushing;  state Exp;
branches ;
next	 1.6;

1.6
date	 93.06.22.19.44.16;  author rushing;  state Exp;
branches ;
next	 1.5;

1.5
date	 93.06.22.15.36.05;  author dumoulin;  state Exp;
branches ;
next	 1.4;

1.4
date	 93.06.11.00.10.35;  author rushing;  state Exp;
branches ;
next	 1.3;

1.3
date	 93.05.18.22.11.40;  author rushing;  state Exp;
branches ;
next	 1.2;

1.2
date	 93.04.30.22.08.01;  author rushing;  state Exp;
branches ;
next	 1.1;

1.1
date	 93.03.30.21.08.58;  author rushing;  state Exp;
branches ;
next	 ;


desc
@MAPI merge
@


1.11
log
@Massive cleanup for 92.6
@
text
@/* ---- WVMAPI.C ---------------------------------------------- 
 *
 *  This file contains the porocedures required for the MAPI
 *  (MS Mail) interface
 *
 *  MAtthew Bretherton     18/2/1993
 */


/*
 * $Id: wvmapi.c 1.11 1994/09/16 00:48:47 jcooper Exp $
 * $Log: wvmapi.c $
 * Revision 1.10  1994/01/24  17:41:01  jcoop
 * 90.2 changes
 *
 * Revision 1.9  1993/12/08  01:28:38  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.8  1993/08/25  18:54:36  mbretherton
 * MRB merge, mail & post logging
 *
 * Revision 1.8  1993/08/04 Bretherton
 * re reverse maildemandlogon (and change default in winvn.c)
 * to match comms dialog box settings (and consequent logic)
 * apologies Sam
 * Revision 1.7  1993/06/28  17:52:17  rushing
 * fixed compiler warnings
 *
 * Revision 1.6  1993/06/22  19:44:16  rushing
 * reverse logic of MailDemandLogon
 *
 * Revision 1.5  1993/06/22  15:36:05  dumoulin
 * Don't display MAPI error message when a user cancels his/her own message
 *
 * Revision 1.4  1993/06/11  00:10:35  rushing
 * second merge from Matt Bretherton sources
 *
 *
 * Revision 1.3  1993/05/18  22:11:40  rushing
 * smtp support
 *
 * Revision 1.2  1993/04/30  22:08:01  rushing
 * removed newbie-frightening messageboxes
 *
 * Revision 1.1  1993/03/30  21:08:58  rushing
 * Initial revision
 *
 *
 */

#include <windows.h>
#include <windowsx.h>
#include "wvglob.h"
#include "winvn.h"
#pragma hdrstop
#include "mapi.h"
#include "wvmapi.h"

 HINSTANCE hLibrary = 0;
 ULONG ulResult  ;
 LHANDLE hMAPISession  ;
 FLAGS flFlag = 0L;
 BOOL LoggedOn  ;           // required for demand login
 char buf[MAXOUTLINE];
 char mess[MAXOUTLINE];

 mlMAPIInit (HWND hWnd)
 {
/*   MessageBox (hWnd, "MS MAil Initialisation", "Debug!", MB_OK | MB_ICONHAND); */
   LoggedOn = FALSE ;


   if ( InitMAPI() ) 
       {
// this is confusing to newbies
//	   MessageBox (hWnd, "Error in Initialisation\r\nMail Interface Disabled", "Microsoft Mail", MB_OK | MB_ICONHAND);
	   return (-1) ;
	};

  if (MailDemandLogon) return (0)  ;
  else  return (mlMAPILogon(hWnd)) ;
 }

int
 mlMAPILogon (HWND hWnd)
 {
  flFlag = MAPI_LOGON_UI;

  ulResult = (ULONG)(*lpfnMAPILogon)((ULONG) hWnd, NULL, NULL, flFlag, 0L, (LPLHANDLE)&hMAPISession);
  if( ulResult != 0)
	   {
// this is confusing to newbies
//	   sprintf(mess,"Failure to Login\r\nMail Interface Disabled\r\nError = %ld",ulResult);
//	   MessageBox (hWnd,mess , "Microsoft Mail", MB_OK | MB_ICONHAND);
	   return (-1) ;
       }   
   LoggedOn = TRUE ;
   MailEnableLogout(LoggedOn);
   return(0) ;
 }

/* -------- mlMAPILogout ---------------
 *
 * Logout from MAPI service but leave resources available
 * (loaded)
 *
 * --------------------------------------*/

 mlMAPILogout (hWnd)
    HWND hWnd;
 {
/*   MessageBox (hWnd, "MS MAil Logout", "Debug!", MB_OK | MB_ICONHAND); */
   ulResult =(ULONG)(*lpfnMAPILogoff)(hMAPISession,(ULONG) hWnd, 0L, 0L) ;
   if (ulResult==0)
   {
      LoggedOn = FALSE ;
      MailEnableLogout(LoggedOn);
      return(0);
   }
   return(-1) ;
 }

/* -------- mlMAPIClose ---------------
 *
 * Remove all trace of MAPI services - log out (if necessary)
 * and unload resources
 *
 * --------------------------------------*/

 mlMAPIClose (HWND hWnd)
{  if (LoggedOn) mlMAPILogout(hWnd) ;
   DeInitMAPI();
   return(0) ;
}

/******** mlMAPISend *************************************************** 
 *   
 *  Send Mail for MAPI Interface 
 *
 *
 */

 mlMAPISend (hWnd ,Doc, DocType )
   HWND hWnd;
   TypDoc *Doc;
   int DocType;
 {
 static  MapiMessage mmMapiMessage;
 static  MapiRecipDesc recips;     /* Recipients 
					- only one To: at present */
 /*  Not yet required 
   MapiFileDesc files[2];
 */
   HANDLE hMemBuf ;
   char Address[MAXDIALOGSTRING] ;
   char far *mBuf;

   if (!LoggedOn)
   { 
     if( ! mlMAPILogon(hWnd) == 0 ) 
     { // No Logon - No Go !
	return(-1) ;
     } 
   }

   if (NULL == (hMemBuf = ExtractPostingText(Doc,DocType)))
    {
      MessageBox (hWnd, "Cannot allocate memory for text", "", MB_OK);
      return FALSE;
    }
   mBuf=GlobalLock(hMemBuf) ;

  if (strchr(AddressString,'@@') != NULL)
	 {
	   sprintf(Address,"SMTP:%s",AddressString) ;
	 }
  else
	 {
	 strcpy(Address,AddressString) ;
	 }
/*  mrb debugging   
  MessageBox (hWnd, Address, "Address", MB_OK | MB_ICONHAND);
  MessageBox (hWnd, NameString, "Name", MB_OK | MB_ICONHAND);
  MessageBox (hWnd, DialogString, "Subject", MB_OK | MB_ICONHAND);
  MessageBox (hWnd, mBuf, "Article", MB_OK | MB_ICONHAND);
*/
 /* need to build mmMApiMessage
       multiple recipients 
       and a copies command */ 

  if (!strcmp(DialogString,""))
     {
	   mmMapiMessage.lpszSubject = NULL ;
      }
  else
     { 
	   mmMapiMessage.lpszSubject = DialogString ;
	  }
  if (!strcmp(mBuf,""))
      {
		 mmMapiMessage.lpszNoteText = NULL ;
      }
  else
      {
	mmMapiMessage.lpszNoteText = mBuf;
       }

 /*  should parse Addressee and Copies to fill lpRecips 
    use MAPI_CC  for copies                             */ 


 if (!strcmp(Address,""))
     {
	   mmMapiMessage.nRecipCount = 0L ;
	   mmMapiMessage.lpRecips=NULL ;

     }
  else
     {
	   mmMapiMessage.nRecipCount = 1L ;
	   recips.ulReserved = 0L ;
	   recips.lpszAddress =  Address  ;
	   recips.ulRecipClass = MAPI_TO ;
	   recips.lpszName =  NameString  ;
	   recips.ulEIDSize = 0L ;
	   recips.lpEntryID = NULL ;
	   mmMapiMessage.lpRecips=&recips ;
     }


  mmMapiMessage.ulReserved = 0L;
  mmMapiMessage.lpszMessageType = NULL;
  mmMapiMessage.lpszDateReceived = NULL;
  mmMapiMessage.lpszConversationID = NULL ;
  mmMapiMessage.flFlags = 0L;
  mmMapiMessage.lpOriginator = NULL;
  mmMapiMessage.nFileCount = 0L;
  mmMapiMessage.lpFiles = NULL;

  flFlag =MAPI_DIALOG;   /* MAPI_DIALOG shows a windows for message
			    editing , addressing etc.
			    MAPI_LOGON_UI in helpreq.c example  */
  ulResult = (ULONG)(*lpfnMAPISendMail)(hMAPISession, (ULONG) hWnd, &mmMapiMessage, 
		 flFlag, 0L);
  FreePostingText (hMemBuf) ;
  MailEdit = (WndEdit *) NULL;

  if ( ulResult > 1 )
    {
       sprintf(buf,"MS MAil Error - No. %u",ulResult) ;
       MessageBox (hWnd, buf, "Microsoft Mail Send", MB_OK | MB_ICONHAND);
       return(-1) ;
     } ;

  return(0) ;

 }


/* Following code extracted from Microsofts MAPI Sample 
   MAPIAPP.C on the MS MAIL 3.0 distibution disks */
 
int FAR PASCAL InitMAPI()
{
  char *libraryName;
  		
  if ((hLibrary = LoadLibrary(MAPIDLL)) < HINSTANCE_ERROR)
    return(ERR_LOAD_LIB);

  if (((FARPROC)lpfnMAPILogon = GetProcAddress(hLibrary,SZ_MAPILOGON)) == NULL)
    return(ERR_LOAD_FUNC);

  if (((FARPROC)lpfnMAPILogoff= GetProcAddress(hLibrary,SZ_MAPILOGOFF)) == NULL)
    return(ERR_LOAD_FUNC);

  if (((FARPROC)lpfnMAPISendMail= GetProcAddress(hLibrary,SZ_MAPISENDMAIL)) == NULL)
    return(ERR_LOAD_FUNC);

  if (((FARPROC)lpfnMAPIAddress= GetProcAddress(hLibrary,SZ_MAPIADDRESS)) == NULL)
    return(ERR_LOAD_FUNC);

  if (((FARPROC)lpfnMAPIFreeBuffer= GetProcAddress(hLibrary,SZ_MAPIFREEBUFFER)) == NULL)
    return(ERR_LOAD_FUNC);

  /* Following Functions are not required 

  if ((lpfnMAPISendDocuments= GetProcAddress(hLibrary,SZ_MAPISENDDOC)) == NULL)
    return(ERR_LOAD_FUNC);

  if ((lpfnMAPIFindNext= GetProcAddress(hLibrary,SZ_MAPIFINDNEXT)) == NULL)
    return(ERR_LOAD_FUNC);

  if ((lpfnMAPIReadMail= GetProcAddress(hLibrary,SZ_MAPIREADMAIL)) == NULL)
    return(ERR_LOAD_FUNC);

  if ((lpfnMAPISaveMail= GetProcAddress(hLibrary,SZ_MAPISAVEMAIL)) == NULL)
    return(ERR_LOAD_FUNC);

  if ((lpfnMAPIDeleteMail= GetProcAddress(hLibrary,SZ_MAPIDELMAIL)) == NULL)
    return(ERR_LOAD_FUNC);

  if ((lpfnMAPIDetails= GetProcAddress(hLibrary,SZ_MAPIDETAILS)) == NULL)
    return(ERR_LOAD_FUNC);

  if ((lpfnMAPIResolveName= GetProcAddress(hLibrary,SZ_MAPIRESOLVENAME)) == NULL)
    return(ERR_LOAD_FUNC);

  */

  return(0);
}


int FAR PASCAL DeInitMAPI()
{
  lpfnMAPILogon = NULL;
  lpfnMAPILogoff= NULL;
  lpfnMAPISendMail= NULL;
  lpfnMAPIFreeBuffer = NULL;
  lpfnMAPIAddress= NULL;

  /* following functions are unused
  lpfnMAPISendDocuments= NULL;
  lpfnMAPIFindNext= NULL;
  lpfnMAPIReadMail= NULL;
  lpfnMAPISaveMail= NULL;
  lpfnMAPIDeleteMail= NULL;
  lpfnMAPIDetails = NULL;
  lpfnMAPIResolveName = NULL
  */

  FreeLibrary(hLibrary);

  return(0);
}
@


1.10
log
@90.2 changes
@
text
@d11 1
a11 1
 * $Id: wvmapi.c 1.9 1993/12/08 01:28:38 rushing Exp $
d13 3
d51 7
d59 1
a59 7
 #include "windows.h"
 #include "mapi.h"
 #include "wvglob.h"
 #include "winvn.h"
 #include "wvmapi.h"

 HANDLE hLibrary;
d61 1
a61 1
 HANDLE hMAPISession  ;
d89 1
a89 1
  ulResult = (*lpfnMAPILogon)((ULONG) hWnd, NULL, NULL, flFlag, 0L, (LPLHANDLE)&hMAPISession);
d113 1
a113 1
   ulResult =(*lpfnMAPILogoff)(hMAPISession,(ULONG) hWnd, 0L, 0L) ;
d243 1
a243 1
  ulResult = (*lpfnMAPISendMail)(hMAPISession, (ULONG) hWnd, &mmMapiMessage, 
d265 3
a267 2

  if ((hLibrary = LoadLibrary(MAPIDLL)) < 32)
@


1.9
log
@new version box and cr lf consistency
@
text
@d11 1
a11 1
 * $Id: wvmapi.c 1.8 1993/08/25 18:54:36 mbretherton Exp rushing $
d13 3
d242 1
@



1.8
log
@MRB merge, mail & post logging
@
text
@d9 1
d11 1
a11 1
 * $Id: wvmapi.c 1.7 1993/06/28 17:52:17 rushing Exp $
d13 3
@



1.7
log
@fixed compiler warnings
@
text
@d10 1
a10 1
 * $Id: wvmapi.c 1.6 1993/06/22 19:44:16 rushing Exp rushing $
d12 7
d69 1
a69 1
  if (!MailDemandLogon) return (0)  ;
@



1.6
log
@reverse logic of MailDemandLogon
@
text
@d10 1
a10 1
 * $Id: wvmapi.c 1.5 1993/06/22 15:36:05 dumoulin Exp rushing $
d12 3
a136 1
   BOOL Addressed  ;
d250 1
a250 1
  if ((lpfnMAPILogon = GetProcAddress(hLibrary,SZ_MAPILOGON)) == NULL)
d253 1
a253 1
  if ((lpfnMAPILogoff= GetProcAddress(hLibrary,SZ_MAPILOGOFF)) == NULL)
d256 1
a256 1
  if ((lpfnMAPISendMail= GetProcAddress(hLibrary,SZ_MAPISENDMAIL)) == NULL)
d259 1
a259 1
  if ((lpfnMAPIAddress= GetProcAddress(hLibrary,SZ_MAPIADDRESS)) == NULL)
d262 1
a262 1
  if ((lpfnMAPIFreeBuffer= GetProcAddress(hLibrary,SZ_MAPIFREEBUFFER)) == NULL)
@



1.5
log
@Don't display MAPI error message when a user cancels his/her own message
@
text
@d10 1
a10 1
 * $Id: wvmapi.c 1.4 1993/06/11 00:10:35 rushing Exp dumoulin $
d12 3
d59 1
a59 1
  if (MailDemandLogon) return (0)  ;
@



1.4
log
@second merge from Matt Bretherton sources
@
text
@d10 1
a10 1
 * $Id: wvmapi.c 1.3 1993/05/18 22:11:40 rushing Exp $
d12 2
d15 1
d224 1
a224 1
  if ( ulResult !=0 )
@



1.3
log
@smtp support
@
text
@d10 1
a10 1
 * $Id: wvmapi.c 1.2 1993/04/30 22:08:01 rushing Exp rushing $
d12 4
d27 3
a29 1
 #include "mapi.h" 
a30 1
 #include "wvglob.h"
d36 1
d40 2
a41 3
 mlMAPIInit (hWnd) 
    HWND hWnd;
 { 
d43 3
a45 1
 
d52 8
d70 13
a82 2
 } 
 mlMAPILogout (hWnd)  
d84 1
a84 1
 {  
d86 16
a101 2
   (*lpfnMAPILogoff)(hMAPISession,(ULONG) hWnd, 0L, 0L) ;
   DeInitMAPI() ;
d103 5
a107 1
 }
d132 7
a139 1
   
d213 1
a213 1
  
d219 2
d224 1
a224 1
       MessageBox (hWnd, buf, "Microsoft Mail", MB_OK | MB_ICONHAND);
d228 1
a228 1
  FreePostingText (hMemBuf) ;
a287 1

@



1.2
log
@removed newbie-frightening messageboxes
@
text
@d10 1
a10 1
 * $Id: wvmapi.c 1.1 1993/03/30 21:08:58 rushing Exp rushing $
d12 3
d85 1
a85 1
   char Address[MAXSUBJECTSTRING] ;
d108 1
a108 1
  MessageBox (hWnd, SubjectString, "Subject", MB_OK | MB_ICONHAND);
d115 1
a115 1
  if (!strcmp(SubjectString,""))
d121 1
a121 1
	   mmMapiMessage.lpszSubject = SubjectString ;
@



1.1
log
@Initial revision
@
text
@d10 4
a13 2
 * $Id$
 * $Log$
d15 1
d38 2
a39 1
	   MessageBox (hWnd, "Error in Initialisation\r\nMail Interface Disabled", "Microsoft Mail", MB_OK | MB_ICONHAND);
d47 3
a49 2
	   sprintf(mess,"Failure to Login\r\nMail Interface Disabled\r\nError = %ld",ulResult);
	   MessageBox (hWnd,mess , "Microsoft Mail", MB_OK | MB_ICONHAND);
@

