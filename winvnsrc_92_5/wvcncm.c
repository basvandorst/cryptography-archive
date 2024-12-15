/*
 *
 * $Id: wvcncm.c 1.30 1994/09/06 12:46:59 rushing Exp $
 * $Log: wvcncm.c $
 * Revision 1.30  1994/09/06  12:46:59  rushing
 * fix maildlg for win32
 *
 * Revision 1.29  1994/08/24  18:00:29  jcooper
 * misc encoding/decoding changes
 *
 * Revision 1.28  1994/08/11  00:09:17  jcooper
 * Enhancements to Mime and article encoding/encoding
 *
 * Revision 1.27  1994/07/25  22:41:22  pearse_w_r
 * ShowUnreadOnly option
 *
 * Revision 1.26  1994/07/25  18:51:48  jcooper
 * execution of decoded files
 *
 * Revision 1.25  1994/06/08  21:01:45  gardnerd
 * more scrolling changes...
 *
 * Revision 1.24  1994/05/26  22:11:04  jglasser
 * warnings
 *
 * Revision 1.23  1994/05/23  19:51:51  rushing
 * NNTPPort becomes NNTPService, and it's a string.
 *
 * Revision 1.22  1994/05/23  18:37:00  jcooper
 * new attach code, session [dis]connect
 *
 * Revision 1.21  1994/05/02  19:49:56  rushing
 * changes from jody glasser
 *
 * Revision 1.20  1994/03/01  19:10:19  rushing
 * ThreadFullSubject option added
 *
 * Revision 1.19  1994/02/24  21:27:28  jcoop
 * jcoop changes
 *
 * Revision 1.18  1994/01/24  17:39:47  jcoop
 * 90.2 changes
 *
 * Revision 1.17  1994/01/16  12:02:59  jcoop
 * INI stuff now all written in winvn.c.  New dialogs for en/decoding
 *
 * Revision 1.16  1994/01/12  19:27:32  mrr
 * mrr mods 4
 * 
 * Revision 1.15  1993/12/08  01:27:21  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.14  1993/08/25  18:53:17  mbretherton
 * MRB merge, mail & post logging
 *
 *
 * Bretherton
 *  activate mail type after user change and add logging dialog
 *
 * Revision 1.13  1993/08/25  17:05:17  mbretherton
 * merge from first newsrc.zip
 *
 *
 * Bretherton
 * activate mail type after user change and add logging dialog
 *
 * Revision 1.12  1993/08/05  20:06:07  jcoop
 * save multiple articles changes by jcoop@oakb2s01.apl.com (John S Cooper)
 *
 * Revision 1.11  1993/07/13  16:03:04  riordan
 * MRR mods
 *
 * Revision 1.10  1993/06/26  00:25:11  rushing
 * warnings
 *
 * Revision 1.9  1993/06/25  20:49:28  dumoulin
 * Cleaned up compiler warnings
 *
 * Revision 1.8  1993/06/22  19:44:42  rushing
 * fixed IDD_MAIL_SELECT_AUTO
 *
 * Revision 1.7  1993/06/22  19:09:14  rushing
 * mail force type selection via dialog
 *
 * Revision 1.6  1993/05/28  17:18:00  rushing
 * Added Misc... option to enable threading
 *
 * Revision 1.5  1993/05/24  23:24:26  rushing
 * change NNTPHost->SMTP Host for Comm... Dialog
 *
 * Revision 1.4  1993/05/18  22:10:45  rushing
 * smtp support
 *
 * Revision 1.3  1993/05/13  19:59:11  rushing
 * fancy 'from' in group window
 *
 * Revision 1.2  1993/05/13  16:17:00  rushing
 * article fetch limit dialog box
 *
 * Revision 1.1  1993/02/16  20:53:50  rushing
 * Initial revision
 *
 *
 */

/*-- This is the first line of WVCNCM.C -------------------------------*/

#include <stdlib.h>
#include <limits.h>
#include <windows.h>
#include <windowsx.h>
#include "wvglob.h"
#include "winvn.h"

extern int WinVnDoComm (char *);
int saveMailForceType ;
void enableDemandLogonCtl( HWND hDlg, int forceType);
void SetupEncodingOptions (HWND hDlg);
BOOL ProcessEncodingOptions (HWND hDlg);

/*-- function WinVnCommDlg ---------------------------------------------
 *
 *   Dialog function to process the Configure Communications
 *   dialog box.
 */

BOOL FAR PASCAL 
WinVnCommDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
#define MAXSTR  32
  static int MyCommPortID;
  static int MyCommParityID;
  static int MyCommModeID;
  static int SaveDemand ;
  static char pszMyCommSpeed[MAXCOMMSPEEDCHARS];
  static char pszMyNNTPService[MAXSTR];
  int mail_force_radio;
  int write_ini = FALSE;
  char NNTPPassword[MAXNNTPSIZE];
// char *cptr;

  switch (iMessage)
    {

    case WM_INITDIALOG:
#if 0
      if (AskComm == ASK_COMM_INITIAL)
   AskComm = ASK_COMM_NEVER;
      MyCommPortID = CommPortID;
      MyCommParityID = CommParityID;
      strcpy (pszMyCommSpeed, pszCommSpeed);
      MyCommModeID = (UsingSocket ? ID_COMMTCP : ID_COMMSERIAL);

      CheckRadioButton (hDlg, IDD_COM1, IDD_COM2, CommPortID);
      CheckRadioButton (hDlg, IDD_7EVEN, IDD_8NONE, CommParityID);
      CheckRadioButton (hDlg, ID_COMMTCP, ID_COMMSERIAL, MyCommModeID);
      CheckDlgButton (hDlg, ID_CONFIG_ASK_COMM, AskComm);
      SetDlgItemText (hDlg, IDD_SPEED, pszMyCommSpeed);
#endif
      SetDlgItemText (hDlg, ID_CONFIG_NNTP_SERVER, NNTPHost);
      SetDlgItemText (hDlg, ID_CONFIG_SMTP_SERVER, SMTPHost);
      SetDlgItemText (hDlg, ID_CONFIG_NNTP_PORT, NNTPService);
      SetDlgItemText (hDlg, ID_CONFIG_AUTH_USERNAME, NNTPUserName);
      MRRDecrypt(NNTPPasswordEncrypted,(unsigned char *)NNTPPassword, MAXNNTPSIZE);
      SetDlgItemText (hDlg, ID_CONFIG_AUTH_PASSWORD, NNTPPassword);

      saveMailForceType=MailForceType ;
      if (MailForceType == -1)
   mail_force_radio = IDD_MAIL_SELECT_AUTO;
      else
   mail_force_radio = MailForceType + IDD_MAIL_SELECT_NONE;

      CheckRadioButton (hDlg, IDD_MAIL_SELECT_NONE,
         IDD_MAIL_SELECT_AUTO, mail_force_radio);

      SaveDemand = MailDemandLogon ;
      CheckDlgButton (hDlg, IDD_DEMANDLOGON, SaveDemand);
      enableDemandLogonCtl(hDlg,saveMailForceType) ;

      return TRUE;

      break;

    case WM_COMMAND:
      switch (wParam)
   {
   case ID_OK_SAVE:
     write_ini = TRUE;
   case IDOK:
     GetDlgItemText(hDlg, IDD_SPEED, pszCommSpeed, MAXCOMMSPEEDCHARS - 1);
#if 0
/* disabled serial code... */
     CommPortID = MyCommPortID;
     CommParityID = MyCommParityID;
     CommIDtoStr (CommPortID, CommParityID, pszCommSpeed, szCommString);
     WinVnDoComm (szCommString);

     if (Initializing)
       UsingSocket = (MyCommModeID == ID_COMMTCP);
#endif
     UsingSocket = 1;
     GetDlgItemText (hDlg, ID_CONFIG_NNTP_SERVER, NNTPHost, MAXNNTPSIZE);
     GetDlgItemText (hDlg, ID_CONFIG_SMTP_SERVER, SMTPHost, MAXNNTPSIZE); 
     GetDlgItemText (hDlg, ID_CONFIG_NNTP_PORT, NNTPService, MAXNNTPSIZE);
     GetDlgItemText (hDlg, ID_CONFIG_AUTH_USERNAME, NNTPUserName, MAXNNTPSIZE);
     GetDlgItemText (hDlg, ID_CONFIG_AUTH_PASSWORD, NNTPPassword, MAXNNTPSIZE);
     MRREncrypt((unsigned char *)NNTPPassword,strlen(NNTPPassword)+1,
     NNTPPasswordEncrypted);
     AskComm = IsDlgButtonChecked (hDlg, ID_CONFIG_ASK_COMM);
     MailDemandLogon = SaveDemand ;


/*	  if (write_ini)	(JSC - Now write on exit of app)
	    {
//	      WritePrivateProfileString
//		(szAppName, "CommString", szCommString, szAppProFile);
	      WritePrivateProfileString
		(szAppName, "NNTPHost", NNTPHost, szAppProFile);
	      WritePrivateProfileString
		(szAppName, "SMTPHost", SMTPHost, szAppProFile);
	      WritePrivateProfileString
		(szAppName, "NNTPService", NNTPService, szAppProFile);
		WritePrivateProfileString
		(szAppName, "NNTPUserName", NNTPUserName, szAppProFile);
		WritePrivateProfileString
		(szAppName, "NNTPPassword", NNTPPasswordEncrypted, szAppProFile);
	      WritePrivateProfileInt (szAppName, "MailForceType",
				      saveMailForceType, szAppProFile);
	      WritePrivateProfileInt (szAppName, "MailDemandLogon",
				      MailDemandLogon, szAppProFile);


//         cptr = MyCommModeID == ID_COMMTCP ? "1" : "0";
//         WritePrivateProfileString (szAppName, "UseSocket", cptr, szAppProFile);
//         WritePrivateProfileInt (szAppName, "AskComm", AskComm, szAppProFile);

       }
*/
     if (saveMailForceType!=MailForceType)
     {/* change of mail transport medium
         Not entirely correct as may change from
         explicit selection to auto detect which
         in this case will (but in reality may not)
         affect a relogin to mail)  */
         MailForceType=saveMailForceType;
         MailCrashExit(hDlg);
         MailInit (hDlg);
     }
     EndDialog (hDlg, TRUE);
     break;

   case IDCANCEL:
     EndDialog (hDlg, FALSE);
     break;

#if 0
   case ID_COMMTCP:
   case ID_COMMSERIAL:
     MyCommModeID = wParam;
     CheckRadioButton (hDlg, ID_COMMTCP, ID_COMMSERIAL, MyCommModeID);
     break;

   case IDD_COM1:
   case IDD_COM2:
     MyCommPortID = wParam;
     CheckRadioButton (hDlg, IDD_COM1, IDD_COM2, wParam);
     break;

   case IDD_7EVEN:
   case IDD_8NONE:
     MyCommParityID = wParam;
     CheckRadioButton (hDlg, IDD_7EVEN, IDD_8NONE, wParam);
     break;
#endif

   case IDD_MAIL_SELECT_NONE:
   case IDD_MAIL_SELECT_MAPI:   
   case IDD_MAIL_SELECT_SMTP:
     saveMailForceType = wParam - IDD_MAIL_SELECT_NONE;
     enableDemandLogonCtl(hDlg,saveMailForceType) ;
     break;

   case IDD_MAIL_SELECT_AUTO:
     saveMailForceType = -1;
     enableDemandLogonCtl(hDlg,saveMailForceType) ;
          break;

   case IDD_DEMANDLOGON:
     SaveDemand = !SaveDemand;
     CheckDlgButton (hDlg, IDD_DEMANDLOGON, SaveDemand);
     break;

   default:
     return FALSE;
   }
   break;
   
 default:
   return FALSE;
   break;
 }
  return TRUE;
}
/*
 *  enable/disable the check box for the DemandLogon variable
 *  as requested
 *
 *  some mail transports do not require authentification (logon)
 *  so demand (deferred ) logon is not an option 
 *
 */  
void enableDemandLogonCtl( HWND hDlg, int forceType)
{
   EnableWindow(GetDlgItem(hDlg,IDD_DEMANDLOGON),
      (forceType==-1) || (forceType==MT_MAPI) );
}

/*--- Function CommIDtoStr --------------------------------------
 *
 *   Takes information relating to comm port configuration and creates
 *   a string of the form to give to the MODE command.
 *
 *   Entry    Port      is the port (an IDD_* variable)
 *            Parity    is the parity/data bits infor (an IDD_* variable)
 *            szSpeed     is the speed, in character form
 *
 *   Exit     CommStr    is the string; e.g., "COM1:2400,n,8"
 *            Function value is non-zero if error.
 */
int
CommIDtoStr (Port, Parity, szSpeed, CommStr)
     int Port, Parity;
     char *szSpeed;
     char *CommStr;
{
  register char *ptr;

  ptr = CommStr;
  strcpy (ptr, "COM");
  ptr += 3;
  *(ptr++) = (char) (Port == IDD_COM1 ? '1' : '2');
  *(ptr++) = ':';

  for (; *szSpeed; *(ptr++) = *(szSpeed++));
  *(ptr++) = ',';
  if (Parity == IDD_7EVEN)
    {
      strcpy (ptr, "e,7");
    }
  else
    {
      strcpy (ptr, "n,8");
    }

  return (0);
}

/*-- function WinVnSaveArtDlg ---------------------------------------
 *
 *  Dialog function to handle Save Article dialog box.
 */

BOOL FAR PASCAL 
WinVnSaveArtDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
  static int MyAppend;

  switch (iMessage)
    {

    case WM_INITDIALOG:
      MyAppend = SaveArtAppend;

      CheckDlgButton (hDlg, IDD_APPEND, MyAppend);
      SetDlgItemText (hDlg, IDD_FILENAME, SaveArtFileName);
      return TRUE;
      break;

    case WM_COMMAND:
      switch (wParam)
   {
   case IDOK:
     GetDlgItemText (hDlg, IDD_FILENAME, SaveArtFileName, MAXFILENAME - 1);

     SaveArtAppend = MyAppend;
     if (!MRRWriteDocument (ActiveArticleDoc, sizeof (TypText), SaveArtFileName, SaveArtAppend))
       {
         MessageBox (hWndConf, "Could not write to file", "Problems saving file", MB_OK | MB_ICONEXCLAMATION);
       }
     EndDialog (hDlg, TRUE);
     break;

   case ID_BROWSE:
     if (AskForNewFileName (hDlg, SaveArtFileName, "") == SUCCESS)
       SetDlgItemText (hDlg, IDD_FILENAME, SaveArtFileName);
     break;

   case IDCANCEL:
     EndDialog (hDlg, FALSE);
     break;

   case IDD_APPEND:
     MyAppend = !MyAppend;
     CheckDlgButton (hDlg, IDD_APPEND, MyAppend);
     break;

   default:
     return FALSE;
   }
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}

/*-- function WinVnSaveArtsDlg ---------------------------------------
 *
 *  Dialog function to handle Save Articles dialog box.
 *  Same as Save Article dialog, except actual save is deferred until
 *  later (When IDM_RETRIEVE_COMPLETE Message gets sent to Group win)
 *  (JSC)
 */

BOOL FAR PASCAL 
WinVnSaveArtsDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
  static int MyAppend;

  switch (iMessage)
    {

    case WM_INITDIALOG:
      MyAppend = SaveArtAppend;

      CheckDlgButton (hDlg, IDD_APPEND, MyAppend);
      SetDlgItemText (hDlg, IDD_FILENAME, SaveArtFileName);
      return TRUE;
      break;

    case WM_COMMAND:
      switch (wParam)
   {
   case IDOK:
     GetDlgItemText (hDlg, IDD_FILENAME, SaveArtFileName, MAXFILENAME - 1);

     SaveArtAppend = MyAppend;
     EndDialog (hDlg, TRUE);
     break;

   case ID_BROWSE:
     if (AskForNewFileName (hDlg, SaveArtFileName, "") == SUCCESS)
       SetDlgItemText (hDlg, IDD_FILENAME, SaveArtFileName);
     break;
   
   case IDCANCEL:
     EndDialog (hDlg, FALSE);
     break;

   case IDD_APPEND:
     MyAppend = !MyAppend;
     CheckDlgButton (hDlg, IDD_APPEND, MyAppend);
     break;

   default:
     return FALSE;
   }
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}

/*-- function WinVnDecodeArtsDlg ---------------------------------------
 *
 *  Dialog function to handle Decode Articles dialog box.
 *  (JSC)
 */

BOOL FAR PASCAL 
WinVnDecodeArtsDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
  int len;

  switch (iMessage)
    {

    case WM_INITDIALOG:
      SetDlgItemText (hDlg, IDD_DECODE_PATH, DecodePathName);
      CheckDlgButton (hDlg, IDD_DUMB_DECODE, DumbDecode);
      CheckDlgButton (hDlg, IDD_VERBOSE_STATUS, CodingStatusVerbose);
      CheckDlgButton (hDlg, IDD_EXECUTE_DECODED, ExecuteDecodedFiles);
      return TRUE;
      break;

    case WM_COMMAND:
      switch (wParam)
	{
	case IDOK:
	  GetDlgItemText (hDlg, IDD_DECODE_PATH, DecodePathName, MAXFILENAME - 1);
	  RemoveTrailingWhiteSpace (DecodePathName);
          len = strlen(DecodePathName);
	  if (len == 0)
	  {
 		MessageBox (hDlg, "Please enter a path name before continuing", "Decode Path", ID_OK);
		break;                                                                                                          
	  }

	  if (str[len-1] == '\\')
	     str[len-1] = '\0';

      	  DumbDecode = IsDlgButtonChecked (hDlg, IDD_DUMB_DECODE);
      	  CodingStatusVerbose = IsDlgButtonChecked (hDlg, IDD_VERBOSE_STATUS);
	  ExecuteDecodedFiles = IsDlgButtonChecked (hDlg, IDD_EXECUTE_DECODED);
	  EndDialog (hDlg, TRUE);
	  break;

	
	case ID_BROWSE:
	  if (AskForFilePath (hDlg, DecodePathName, "Select Decode Path") == SUCCESS)
	     SetDlgItemText (hDlg, IDD_DECODE_PATH, DecodePathName);
	  break;

	case IDCANCEL:
	  EndDialog (hDlg, FALSE);
	  break;

	default:
	  return FALSE;
	}
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}
/*-- function WinVnAttachDlg ---------------------------------------
 *
 *  Dialog function to handle attachments to postings
 *  (JSC)
 */

BOOL FAR PASCAL 
WinVnAttachDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
  char temp[MAXINTERNALLINE];
  extern char *GetFileExtension ();
  extern BOOL isnumber();
  long newLen;
  register int i;
  int genMIME, result;
  extern char *ContentTypes[NUM_CONTENT_TYPES];
    
  switch (iMessage)
    {
    case WM_INITDIALOG:
	sprintf (temp, "Attachment options for file %s", AttachFileName);
	SetWindowText (hDlg, (LPSTR) temp);

	CheckRadioButton (hDlg,IDD_ATTACH_NOW, IDD_ATTACH_NEXT, AttachInNewArt?IDD_ATTACH_NEXT:IDD_ATTACH_NOW);

	// this feature is only available when posting (not when mailing)
        if (!getWndEdit(WndMails, GetWindow (hDlg, GW_OWNER), MAXMAILWNDS))
	{
		EnableWindow (GetDlgItem (hDlg, IDD_REVIEW), TRUE);		// posting
		EnableWindow (GetDlgItem (hDlg, IDD_NO_REVIEW), TRUE);
	}
	else
	{
		EnableWindow (GetDlgItem (hDlg, IDD_REVIEW), FALSE);	// mailing
		EnableWindow (GetDlgItem (hDlg, IDD_NO_REVIEW), FALSE);
	}
			
	CheckRadioButton (hDlg,IDD_REVIEW, IDD_NO_REVIEW, ReviewAttach?IDD_REVIEW:IDD_NO_REVIEW);

	SendDlgItemMessage(hDlg, IDD_ARTICLE_SPLIT_LENGTH,
	    CB_ADDSTRING, 0, (LPARAM) ((LPSTR) "32000"));
	SendDlgItemMessage(hDlg, IDD_ARTICLE_SPLIT_LENGTH,
	    CB_ADDSTRING, 0, (LPARAM) ((LPSTR) uitoa(MAXPOSTLENGTH, str, 10)));

	SetDlgItemText (hDlg, IDD_ARTICLE_SPLIT_LENGTH, uitoa (ArticleSplitLength, str, 10));
		
	SetupEncodingOptions (hDlg);

	SetDlgItemText (hDlg, IDD_SUBJECT_TEMPLATE, SubjectTemplate);

        CheckDlgButton (hDlg, IDD_GENERATE_MIME, GenerateMIME);
        if (!GenerateMIME)
        {
		EnableWindow (GetDlgItem (hDlg, IDD_MIME_BOUNDARY), FALSE);
		EnableWindow (GetDlgItem (hDlg, IDD_CONTENT_TYPE), FALSE);
		EnableWindow (GetDlgItem (hDlg, IDD_MAKE_DEFAULT), FALSE);
	}        
        CheckDlgButton (hDlg, IDD_MIME_USAGE_SUGGESTIONS, MIMEUsageSuggestions);

	for (i = 0; i < NUM_CONTENT_TYPES; i++)
		SendDlgItemMessage(hDlg, IDD_CONTENT_TYPE,
		    CB_ADDSTRING, 0, (LPARAM) ((LPSTR) ContentTypes[i]));

	if (_stricmp (DefaultContentType, "Other"))
		SendDlgItemMessage(hDlg, IDD_CONTENT_TYPE,
	    	CB_ADDSTRING, 0, (LPARAM) ((LPSTR) DefaultContentType));
	
   	CheckDlgButton (hDlg, IDD_MAKE_DEFAULT, 0);

	GetFileExtension (temp, AttachFileName);
	if (!_stricmp (temp, "gif"))
		SendDlgItemMessage(hDlg, IDD_CONTENT_TYPE, CB_SELECTSTRING, (WPARAM)-1, (LPARAM) ((LPSTR) "Image/GIF"));
	else if (!_stricmp (temp, "jpg") || !_strnicmp (temp, "jpe", 3))
		SendDlgItemMessage(hDlg, IDD_CONTENT_TYPE, CB_SELECTSTRING, (WPARAM)-1, (LPARAM) ((LPSTR) "Image/JPEG"));
	else if (!_stricmp (temp, "zip"))
		SendDlgItemMessage(hDlg, IDD_CONTENT_TYPE, CB_SELECTSTRING, (WPARAM)-1, (LPARAM) ((LPSTR) "Application/Zip"));
	else if (!_stricmp (temp, "mpg") || !_strnicmp (temp, "mpe", 3))
		SendDlgItemMessage(hDlg, IDD_CONTENT_TYPE, CB_SELECTSTRING, (WPARAM)-1, (LPARAM) ((LPSTR) "Video/MPEG"));
	else if (!_stricmp (temp, "avi"))                  
		SendDlgItemMessage(hDlg, IDD_CONTENT_TYPE, CB_SELECTSTRING, (WPARAM)-1, (LPARAM) ((LPSTR) "Video/AVI"));
	else if (!_stricmp (temp, "ps"))
		SendDlgItemMessage(hDlg, IDD_CONTENT_TYPE, CB_SELECTSTRING, (WPARAM)-1, (LPARAM) ((LPSTR) "Application/PostScript"));
	else if (!_stricmp (temp, "txt") || !_stricmp (temp, "bat") || !_stricmp (temp,"c") || !_stricmp (temp,"cpp") || !_stricmp (temp,"h"))
		SendDlgItemMessage(hDlg, IDD_CONTENT_TYPE, CB_SELECTSTRING, (WPARAM)-1, (LPARAM) ((LPSTR) "Text/Plain"));
	else
		SendDlgItemMessage(hDlg, IDD_CONTENT_TYPE, CB_SELECTSTRING, (WPARAM)-1, (LPARAM) ((LPSTR) DefaultContentType));
	
	SetDlgItemText (hDlg, IDD_MIME_BOUNDARY, MIMEBoundary);

      
      return TRUE;
      break;

    case WM_COMMAND:
      switch (wParam)
	{
	case ID_OK:
  	  AttachInNewArt = IsDlgButtonChecked (hDlg, IDD_ATTACH_NEXT);
	  // this feature is only available when posting (not when mailing)
	  if (!getWndEdit(WndMails, GetWindow (hDlg, GW_OWNER), MAXMAILWNDS))
  	    ReviewAttach = IsDlgButtonChecked (hDlg, IDD_REVIEW); // posting
	    
	  if (ProcessEncodingOptions (hDlg) == FAIL)
	  	break;
	    
	  GetDlgItemText (hDlg, IDD_ARTICLE_SPLIT_LENGTH, temp, MAXINTERNALLINE);
       	  if (!_stricmp (temp, "None"))
		ArticleSplitLength = 0;
	  else	
	  	if (isnumber (temp))
		{
			newLen = atol (temp);
			if (newLen <= 0 || newLen > MAXPOSTLENGTH)
			{
		  		sprintf (temp, "The article split length must be between 1 and %u", MAXPOSTLENGTH);
		  		MessageBox (hDlg, temp, "Article Split Length Error", ID_OK);
	  			break;                                                                                                          
	  		}
	  		ArticleSplitLength = (unsigned int) newLen;
		}
		else
		{
	  		MessageBox (hDlg, "The article split length must be a NUMBER of bytes", "Article Split Length Error", ID_OK);
	  		break;                                                                                                          
	  	}
		
	  
	  GetDlgItemText (hDlg, IDD_SUBJECT_TEMPLATE, SubjectTemplate, MAXINTERNALLINE);

  	  GenerateMIME = IsDlgButtonChecked (hDlg, IDD_GENERATE_MIME);
	  GetDlgItemText (hDlg, IDD_MIME_BOUNDARY, MIMEBoundary, MAXBOUNDARYLEN);
	  // *** should check here that characters in boundary are all valid
	  GetDlgItemText (hDlg, IDD_CONTENT_TYPE, (LPSTR) ContentType, MAXINTERNALLINE);
          if (IsDlgButtonChecked (hDlg, IDD_MAKE_DEFAULT))
            strcpy (DefaultContentType, ContentType);
          
  	  MIMEUsageSuggestions = IsDlgButtonChecked (hDlg, IDD_MIME_USAGE_SUGGESTIONS);
          if (MIMEUsageSuggestions)
          {
            if (GenerateMIME && EncodingTypeNum != CODE_NONE && EncodingTypeNum != CODE_BASE64)
            {
		sprintf (str, "When using %s encoding, MIME headers are not recommended.\nWould you like to deactivate MIME header generation?", EncodingType);
          	result = MessageBox (hDlg, str, "MIME Usage Suggestion", MB_YESNOCANCEL|MB_ICONINFORMATION);
          	if (result == IDYES) 
          	    GenerateMIME = FALSE;
          	else if (result == IDCANCEL)
          	    break;
          
            } 
            if (!GenerateMIME && EncodingTypeNum == CODE_BASE64)
            {
          	result = MessageBox (hDlg, "When using Base-64 encoding, MIME header generation is recommended.\nWould you like to activate MIME header generation?", 
                            "MIME Usage Suggestion", MB_YESNOCANCEL|MB_ICONINFORMATION);
          	if (result == IDYES)	
          	    GenerateMIME = TRUE;
          	else if (result == IDCANCEL)
          	    break;
            }
          }
	  EndDialog (hDlg, TRUE);
	  break;

	case IDD_GENERATE_MIME:
          genMIME = (BOOL)IsDlgButtonChecked (hDlg, IDD_GENERATE_MIME);
          EnableWindow (GetDlgItem (hDlg, IDD_MIME_BOUNDARY), genMIME);
	  EnableWindow (GetDlgItem (hDlg, IDD_CONTENT_TYPE), genMIME);
	  EnableWindow (GetDlgItem (hDlg, IDD_MAKE_DEFAULT), genMIME);
	  break;        

	case IDCANCEL:
	  EndDialog (hDlg, FALSE);
	  break;

	default:
	  return FALSE;
	}
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}
/*-- function WinVnEncodeDlg ---------------------------------------
 *
 *  Dialog function to handle encoding to a file
 *  Very similar to WinVnAttachDlg
 *  (JSC)
 */

BOOL FAR PASCAL 
WinVnEncodeDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
  char temp[MAXINTERNALLINE];
  
  switch (iMessage)
    {
    case WM_INITDIALOG:
	sprintf (temp, "Encoding options for file %s", AttachFileName);
	SetWindowText (hDlg, (LPSTR) temp);

        SetDlgItemText (hDlg, IDD_FILENAME, AttachFileName);
	SetupEncodingOptions (hDlg);
	return TRUE;
	break;

    case WM_COMMAND:
      switch (wParam)
	{
	case ID_OK:
	  GetDlgItemText (hDlg, IDD_FILENAME, AttachFileName, MAXFILENAME - 1);
	  if (strlen (AttachFileName) == 0)
          {
	  	MessageBox (hDlg, "Please enter a filename to encode", "Filename error", ID_OK);
	  	break;                                                                                                          
          }
          
	  if (ProcessEncodingOptions (hDlg) == FAIL)
	  	break;

	  EndDialog (hDlg, TRUE);
	  break;

	case IDCANCEL:
	  EndDialog (hDlg, FALSE);
	  break;

	case ID_BROWSE:
	  if (AskForExistingFileName (hDlg, AttachFileName, "Open file to be encoded") == SUCCESS)
	     SetDlgItemText (hDlg, IDD_FILENAME, AttachFileName);
	  break;

	default:
	  return FALSE;
	}
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}

/*----------------------------------------------------------------------
 * Encoding options functions for use by two dialogs
 * wvattachdlg and wvencodedlg
 * (JSC)
 */
void
SetupEncodingOptions (HWND hDlg)
{
  register int i;
  extern char *EncodingTypes[NUM_ENCODING_TYPES];

	// This combo box is NOSORT, so entries can be indexed in order they 
	// added here. But BE CAREFUL!
	for (i = 0; i < NUM_ENCODING_TYPES; i++)
		SendDlgItemMessage(hDlg, IDD_CODING_TYPE,	
		    CB_ADDSTRING, 0, (LPARAM) ((LPSTR) EncodingTypes[i]));

	if (EncodingTypeToNum (EncodingType) == CODE_UNKNOWN)
		strcpy (EncodingType, "None");

	SendDlgItemMessage(hDlg, IDD_CODING_TYPE, CB_SELECTSTRING, (WPARAM)-1, (LPARAM) ((LPSTR) EncodingType));

	if (UserCodingTable[0] != '\0')
		SetDlgItemText (hDlg, IDD_CUSTOM_TABLE, UserCodingTable);
}

BOOL
ProcessEncodingOptions (HWND hDlg)
{
	char temp[MAXINTERNALLINE];
	int select, x;
	char dummyMap[128];
	
	select = (int)SendDlgItemMessage(hDlg, IDD_CODING_TYPE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	if (select != CB_ERR)
		SendDlgItemMessage(hDlg, IDD_CODING_TYPE,
       			CB_GETLBTEXT, (WPARAM) select, (LPARAM) ((LPSTR) temp));

       	if (!_stricmp (temp, "Custom"))
		if (GetDlgItemText (hDlg, IDD_CUSTOM_TABLE, UserCodingTable, CODINGTABLESIZE+1))
	  	{
			if (strlen(UserCodingTable) != CODINGTABLESIZE)
	  		{
		  		MessageBox (hDlg, "Invalid encoding table.  The table must contain 64 characters.", "Custom Encoding Error", ID_OK);
	  			return (FAIL);
	  		}
	  		if ((x = CreateCodingMap(dummyMap, UserCodingTable)) != -1)
	  		{
		  		sprintf (temp, "Invalid encoding table.  The table must have no duplicate entries (duplicate is %c).", x);
		  		MessageBox (hDlg, temp, "Custom Encoding Error", ID_OK);
	  			return (FAIL);
	  		}
	  	} else {
	  		MessageBox (hDlg, "To use a custom encoding, you must enter an encoding table", "Custom Encoding Error", ID_OK);
			return (FAIL);
	 	}
	
	strcpy (EncodingType, temp);
	EncodingTypeNum = EncodingTypeToNum (EncodingType);
		  	
  	return (OK);
}

/*-- function WinVnSigFileDlg ---------------------------------------
 *
 *  Dialog function to handle signature files
 *  (JSC 1/8/94)
 */

BOOL FAR PASCAL 
WinVnSigFileDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
  static int MyEnable;

  switch (iMessage)
    {
    MyEnable = EnableSig;

    case WM_INITDIALOG:
      CheckDlgButton (hDlg, IDD_ENABLE_SIG, EnableSig);
      SetDlgItemText (hDlg, IDD_FILENAME, SigFileName);
      return TRUE;
      break;

    case WM_COMMAND:
      switch (wParam)
	{
	case IDOK:
	  GetDlgItemText (hDlg, IDD_FILENAME, SigFileName, MAXFILENAME - 1);

          ResetTextBlock (Signature);
          if (strlen (SigFileName) == 0)
          {
            EnableSig = FALSE;
            EndDialog (hDlg, TRUE);
            break;
          }

	  EnableSig = MyEnable;
	  if (ReadFileToTextBlock (hDlg, Signature, SigFileName, ADD_PLAIN) == FAIL)
	    break;

          if (Signature->numLines > BIG_SIG_FILE)
	  {
  		sprintf (str, "Do you really want a %lu line signature?!?", Signature->numLines);
  		if (MessageBox (hDlg, str, "BIG Signature!", MB_YESNO|MB_ICONQUESTION) == IDNO)
  		{
		  ResetTextBlock (Signature);
		  break;
		}
	  }          
	  EndDialog (hDlg, TRUE);
	  break;

	case ID_BROWSE:
	  if (AskForExistingFileName (hDlg, SigFileName, "Select Signature File") == SUCCESS)
	    SetDlgItemText (hDlg, IDD_FILENAME, SigFileName);
	  break;

	case IDCANCEL:
	  EndDialog (hDlg, FALSE);
	  break;

	case IDD_ENABLE_SIG:
	  MyEnable = !MyEnable;
	  CheckDlgButton (hDlg, IDD_ENABLE_SIG, MyEnable);
	  break;

	default:
	  return FALSE;
	}
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}

/*-- function WinVnExitDlg---------------------------------------
 *
 *  Dialog function to handle exit from WinVn
 *  (JSC 1/8/94)
 */

BOOL FAR PASCAL 
WinVnExitDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
  static int SaveNewsrc, SaveConfig;

  switch (iMessage)
    {
    case WM_INITDIALOG:
      SaveNewsrc = SaveConfig = 1;
      CheckDlgButton (hDlg, IDD_SAVE_NEWSRC, 1);
      CheckDlgButton (hDlg, IDD_SAVE_CONFIG, 1);
      return TRUE;
      break;

    case WM_COMMAND:
      switch (wParam)
	{
	case IDOK:
	  if (SaveNewsrc && !Initializing)
		WriteNewsrc ();
	  
	  if (SaveConfig)
	  	WriteWinvnProfile();
		
	  EndDialog (hDlg, TRUE);
	  break;

	case IDD_SAVE_NEWSRC:
	  SaveNewsrc = !SaveNewsrc;
	  CheckDlgButton (hDlg, IDD_SAVE_NEWSRC, SaveNewsrc);
	  break;

	case IDD_SAVE_CONFIG:
	  SaveConfig = !SaveConfig;
	  CheckDlgButton (hDlg, IDD_SAVE_CONFIG, SaveConfig);
	  break;

	case IDCANCEL:
	  EndDialog (hDlg, FALSE);
	  break;

	default:
	  return FALSE;
	}
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}

/*-- function WinVnFindDlg ---------------------------------------
 *
 *  Dialog function to handle Search dialog box.
 */

BOOL FAR PASCAL 
WinVnFindDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
  static int MyAppend;

  switch (iMessage)
    {

    case WM_INITDIALOG:
      SetDlgItemText (hDlg, IDD_SEARCH_STRING, FindDoc->SearchStr);
      return TRUE;
      break;

    case WM_COMMAND:
      switch (wParam)
   {
   case IDOK:
     GetDlgItemText (hDlg, IDD_SEARCH_STRING, FindDoc->SearchStr, MAXFINDSTRING - 1);

     EndDialog (hDlg, TRUE);
     break;

   case IDCANCEL:
     EndDialog (hDlg, FALSE);
     break;

   default:
     return FALSE;
   }
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}

/*-- function WinVnGenericDlg ---------------------------------------
 *
 *  Dialog function to query the user for a posting's subject
 */

BOOL FAR PASCAL 
WinVnGenericDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
  switch (iMessage)
    {

    case WM_INITDIALOG:
      /* set the prompt, DialogBoxParam passes in lParam */
      SetDlgItemText (hDlg, IDD_SUBJECT_PROMPT, (char far *) lParam);
      SetDlgItemText (hDlg, IDD_SUBJECT_STRING, DialogString);
      return TRUE;
      break;

    case WM_COMMAND:
      switch (wParam)
   {
   case IDOK:
     GetDlgItemText (hDlg, IDD_SUBJECT_STRING, DialogString, 
                     MAXDIALOGSTRING - 1);

     EndDialog (hDlg, TRUE);
     break;

   case IDCANCEL:
     EndDialog (hDlg, FALSE);
     break;

   default:
     return FALSE;
   }
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}

/*-- function WinVnSubjectDlg ---------------------------------------
 *
 *  Dialog function to query the user for a posting's subject
 */

BOOL FAR PASCAL 
WinVnSubjectDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
  switch (iMessage)
    {

    case WM_INITDIALOG:
      SetDlgItemText (hDlg, IDD_SUBJECT_STRING, DialogString);
      if (ReplyTo[0] == '\0')
         SetDlgItemText (hDlg, IDD_REPLY_TO, MailAddress);
      else
         SetDlgItemText (hDlg, IDD_REPLY_TO, ReplyTo);
      return TRUE;
      break;

    case WM_COMMAND:
      switch (wParam)
   {
   case IDOK:
     GetDlgItemText (hDlg, IDD_SUBJECT_STRING, DialogString, 
                     MAXDIALOGSTRING - 1);
     GetDlgItemText (hDlg, IDD_REPLY_TO, str, MAXDIALOGSTRING - 1);
     if (str[0] == '\0' || !strcmp (str, MailAddress))
        ReplyTo[0] = '\0';
     else      
        strcpy (ReplyTo, str);

     EndDialog (hDlg, TRUE);
     break;

   case IDCANCEL:
     EndDialog (hDlg, FALSE);
     break;

   default:
     return FALSE;
   }
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}
/*-- function WinVnDoListDlg ---------------------------------------
 *
 *  Dialog function to ask whether we should check for new
 *  newsgroups.
 */

BOOL FAR PASCAL 
WinVnDoListDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{

  switch (iMessage)
    {

    case WM_COMMAND:
      switch (wParam)
   {
   case IDOK:

     EndDialog (hDlg, TRUE);
     break;

   case IDCANCEL:

     EndDialog (hDlg, FALSE);
     break;

   default:
     return FALSE;
   }
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}


/*--- Function WinVnPersonalInfoDlg ----------------------------------
 *
 *  Dialog function to obtain personal configuration info
 *  (Name, email address, etc.) from the user.
 */

BOOL FAR PASCAL 
WinVnPersonalInfoDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
#define MAXSTR  32

  int write_ini = FALSE;
  int dialog_val;

  switch (iMessage)
    {

    case WM_INITDIALOG:

      SetDlgItemText (hDlg, ID_CONFIG_EMAIL, MailAddress);
      SetDlgItemText (hDlg, ID_CONFIG_NAME, UserName);
      SetDlgItemText (hDlg, ID_CONFIG_ORG, Organization);
      return TRUE;
      break;

    case WM_COMMAND:
      switch (wParam)
   {
   case ID_OK_SAVE:
     write_ini = TRUE;
   case IDOK:
     GetDlgItemText (hDlg, ID_CONFIG_NAME, UserName, MAILLEN - 1);
     GetDlgItemText (hDlg, ID_CONFIG_EMAIL, MailAddress, MAILLEN - 1);
     GetDlgItemText (hDlg, ID_CONFIG_ORG, Organization, MAILLEN - 1);

/*   if (write_ini)  		(JSC - Now save on exit of app)
       {
         WritePrivateProfileString (szAppName, "UserName", UserName, szAppProFile);
         WritePrivateProfileString (szAppName, "MailAddress", MailAddress, szAppProFile);
         WritePrivateProfileString (szAppName, "Organization", Organization, szAppProFile);
       }
*/   dialog_val = TRUE;
     goto endit;
     break;

   case IDCANCEL:
     dialog_val = FALSE;
   endit:;
     EndDialog (hDlg, dialog_val);
     break;

   default:
     return FALSE;
   }
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}



/*--- Function WinVnMiscDlg ---------------------------------------------
 *
 *  Dialog function to obtain miscellaneous configuration info
 *  (whether we should open a new window for each group, etc.)
 *  from the user.
 */

BOOL FAR PASCAL 
WinVnMiscDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
  int write_ini = FALSE;
  int dialog_val;
  int item;
  int temp;
  char threshold[5];

  switch (iMessage)
    {

    case WM_INITDIALOG:
      CheckRadioButton (hDlg, ID_DOLIST_BASE, ID_DOLIST_ASK, DoList + ID_DOLIST_BASE);
      CheckDlgButton (hDlg, ID_CONFIG_APPEND, SaveArtAppend);
      CheckDlgButton (hDlg, ID_CONFIG_NEW_WND_GROUP, ViewNew);
      CheckDlgButton (hDlg, ID_CONFIG_NEW_WND_ARTICLE, NewArticleWindow);
      CheckDlgButton (hDlg, ID_CONFIG_ARTICLE_FIXED_FONT, ArticleFixedFont);
      CheckDlgButton (hDlg, ID_CONFIG_FULLNAMEFROM, FullNameFrom);
      CheckDlgButton (hDlg, ID_CONFIG_THREADS, threadp);  
      CheckDlgButton (hDlg, ID_CONFIG_THREADS_FULL_SUBJECT, ThreadFullSubject);
      CheckDlgButton (hDlg, ID_CONFIG_SHOWUNSUB, ShowUnsubscribed);
      CheckDlgButton (hDlg, ID_CONFIG_CONFIRM_BATCH, ConfirmBatchOps);
      CheckDlgButton (hDlg, ID_CONFIG_CONFIRM_EXIT, ConfirmExit);
      CheckDlgButton (hDlg, ID_CONFIG_CONFIRM_REPLY_TO, ConfirmReplyTo);
      CheckDlgButton (hDlg, ID_CONFIG_GROUP_MULTI_SELECT, GroupMultiSelect);
      CheckDlgButton (hDlg, ID_CONFIG_CONNECT_AT_START, ConnectAtStartup);
      CheckDlgButton (hDlg, IDD_EXECUTE_DECODED, ExecuteDecodedFiles);
      CheckDlgButton (hDlg, ID_CONFIG_SHOWUNREADONLY, ShowUnreadOnly);

      sprintf (threshold, "%d", article_threshold);
      SetDlgItemText (hDlg, IDD_ART_THRESHOLD, threshold);

      return TRUE;
      break;

    case WM_COMMAND:
      switch (wParam)
   {
   case ID_OK_SAVE:
     write_ini = TRUE;
   case IDOK:
     SaveArtAppend = (IsDlgButtonChecked (hDlg, ID_CONFIG_APPEND) != 0);
     ViewNew = (IsDlgButtonChecked (hDlg, ID_CONFIG_NEW_WND_GROUP) != 0);
     NewArticleWindow = (IsDlgButtonChecked (hDlg, ID_CONFIG_NEW_WND_ARTICLE) != 0);
     ArticleFixedFont = (IsDlgButtonChecked (hDlg, ID_CONFIG_ARTICLE_FIXED_FONT) != 0);
     FullNameFrom = (IsDlgButtonChecked (hDlg, ID_CONFIG_FULLNAMEFROM) != 0); 
     ConfirmBatchOps = (IsDlgButtonChecked (hDlg, ID_CONFIG_CONFIRM_BATCH) != 0); 
     ConfirmExit = (IsDlgButtonChecked (hDlg, ID_CONFIG_CONFIRM_EXIT) != 0); 
     ConfirmReplyTo = (IsDlgButtonChecked (hDlg, ID_CONFIG_CONFIRM_REPLY_TO) != 0); 
     ThreadFullSubject = (IsDlgButtonChecked (hDlg, ID_CONFIG_THREADS_FULL_SUBJECT) != 0);
     GroupMultiSelect = (IsDlgButtonChecked (hDlg, ID_CONFIG_GROUP_MULTI_SELECT) != 0);
     ConnectAtStartup = (IsDlgButtonChecked (hDlg, ID_CONFIG_CONNECT_AT_START) != 0);
     ExecuteDecodedFiles = (IsDlgButtonChecked (hDlg, IDD_EXECUTE_DECODED) != 0);
     ShowUnreadOnly   = (IsDlgButtonChecked (hDlg, ID_CONFIG_SHOWUNREADONLY) != 0);

     
     /* If the user has changed the ShowUnsubscribed option, we must
      * recompute which lines are active and redisplay.  
      * Reset the window to the top to make sure that everything
      * will display OK.
      * There is some question in my mind whether this code will
      * work OK if the first screen's worth of groups are unsubscribed,
      * but it *should* work.
      */
     temp = (IsDlgButtonChecked (hDlg,ID_CONFIG_SHOWUNSUB) != 0);
     if(temp != ShowUnsubscribed) {
      ShowUnsubscribed = temp;
      NetDoc.LongestLine = 0;
      SetGroupActiveLines();
      ScreenToTop(&NetDoc);              
      InvalidateRect(hWndConf,NULL,FALSE);
     }
     
     threadp = (IsDlgButtonChecked (hDlg, ID_CONFIG_THREADS) != 0);
     GetDlgItemText(hDlg,IDD_ART_THRESHOLD,threshold,5);

     /* 0 or a bogus value means they don't want a threshold */
     article_threshold = atoi (threshold);
     if (!article_threshold)
       article_threshold = INT_MAX;

     for (item = ID_DOLIST_BASE; item <= ID_DOLIST_ASK; item++)
       {
         if (IsDlgButtonChecked (hDlg, item))
         {
           DoList = item - ID_DOLIST_BASE;
         }
       }

/*  if (write_ini)         (JSC - Now save on exit of app)
    {
      WritePrivateProfileInt
	(szAppName, "SaveArtAppend", SaveArtAppend, szAppProFile);
      WritePrivateProfileInt
	(szAppName, "NewWndGroup", ViewNew, szAppProFile);
      WritePrivateProfileInt
	(szAppName, "NewWndArticle", NewArticleWindow, szAppProFile);
      WritePrivateProfileInt
	(szAppName, "DoList", DoList, szAppProFile);
      WritePrivateProfileInt
	(szAppName, "ArticleFixedFont", ArticleFixedFont, szAppProFile);
      WritePrivateProfileInt
	(szAppName, "ArticleThreshold", article_threshold, szAppProFile);
      WritePrivateProfileInt
	(szAppName, "FullNameFrom", FullNameFrom, szAppProFile);
      WritePrivateProfileInt
        (szAppName, "EnableThreading", threadp, szAppProFile);
      WritePrivateProfileInt
        (szAppName, "ShowUnsubscribed", ShowUnsubscribed, szAppProFile);
    }
*/  dialog_val = TRUE;
    goto endit;
    break;

   case IDCANCEL:
     dialog_val = FALSE;
   endit:;
     EndDialog (hDlg, dialog_val);
     break;

   default:
     return FALSE;
   }
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;

}

/*-- function WinVnThresholdDlg ---------------------------------------
 *
 *  Dialog function to ask how many article headers we
 *  want to retrieve.
 */

BOOL FAR PASCAL 
WinVnThresholdDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{

  char manual[6];

  switch (iMessage)
    {

    case WM_INITDIALOG:
        sprintf (str, "%u articles in group", arts_to_retrieve);
        SetWindowText(hDlg, (LPCSTR)str);
	break;
    case WM_COMMAND:
      switch (wParam)
   {
   case ID_OK:
     GetDlgItemText (hDlg, IDD_ARTS_TO_GRAB, manual, 6);
     arts_to_retrieve = (unsigned int) atoi (manual);
     if (arts_to_retrieve > 0)
       EndDialog (hDlg, TRUE);
     break;

   case ID_CANCEL:
     EndDialog (hDlg, FALSE);
     break;
     
   case ID_100_ARTS:

     arts_to_retrieve = 100;
     EndDialog (hDlg, TRUE);
     break;

   case ID_250_ARTS:

     arts_to_retrieve = 250;
     EndDialog (hDlg, TRUE);
     break;

   case ID_500_ARTS:

     arts_to_retrieve = 500;
     EndDialog (hDlg, TRUE);
     break;

   case ID_THRESHOLD_ALL:
     arts_to_retrieve = -1;
     EndDialog (hDlg, TRUE);
     break;
     
   case ID_THRESHOLD_UNREAD:
     arts_to_retrieve = -2;
     EndDialog (hDlg, TRUE);
     break;

   default:
     return FALSE;
   }
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}


/*--- Function WinVnAppearanceDlg ----------------------------------
 *
 *  Dialog function to obtain window appearance configuration info
 *  (font, color, etc.)
 *  from the user.
 */

BOOL FAR PASCAL 
WinVnAppearanceDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
  return (TRUE);
}

BOOL FAR PASCAL
WinVnLogOptDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
  int write_ini = FALSE;
  static int SaveMailLog, SavePostLog ;

  switch (iMessage)
    {

    case WM_INITDIALOG:
      SaveMailLog = MailLog ;
      SavePostLog = PostLog ;
      CheckDlgButton (hDlg, IDD_MAILLOG, SaveMailLog);
      CheckDlgButton (hDlg, IDD_POSTLOG, SavePostLog);
      SetDlgItemText (hDlg, IDD_MAILNAME, MailLogFile);
      SetDlgItemText (hDlg, IDD_POSTNAME, PostLogFile);
//   EnableWindow(GetDlgItem(hDlg,IDD_MAILNAME),SaveMailLog);
      EnableWindow(GetDlgItem(hDlg,IDD_POSTNAME),SavePostLog);
  /* mail logging only works for non mapi at present */
     EnableWindow(GetDlgItem(hDlg,IDD_MAILLOG),(MailCtrl.MailType != MT_MAPI));
     EnableWindow(GetDlgItem(hDlg,IDD_MAILNAME),
       (MailCtrl.MailType != MT_MAPI)&&SaveMailLog);

      return TRUE;
      break;

    case WM_COMMAND:
      switch (wParam)
   {
   case ID_OK_SAVE:
     write_ini = TRUE;
   case IDOK:
     GetDlgItemText (hDlg,  IDD_MAILNAME, MailLogFile, MAXFILENAME - 1);
     GetDlgItemText (hDlg,  IDD_POSTNAME, PostLogFile, MAXFILENAME - 1);

     MailLog = SaveMailLog;
          PostLog = SavePostLog;
/*        Should we check sensibility of files before exiting ?
     if (!MRRWriteDocument (ActiveArticleDoc, sizeof (TypText), SaveArtFileName, SaveArtAppend))
       {
         MessageBox (hWndConf, "Could not write to file", "Problems saving file", MB_OK | MB_ICONEXCLAMATION);
       }
*/
/*  if (write_ini)	(JSC - now save on exit of app)
    {
      WritePrivateProfileString
	(szAppName, "MailLogFile", MailLogFile, szAppProFile);
      WritePrivateProfileString
	(szAppName, "PostLogFile", PostLogFile, szAppProFile);
      WritePrivateProfileInt (szAppName, "MailLog",
         MailLog, szAppProFile);
      WritePrivateProfileInt (szAppName, "PostLog",
         PostLog, szAppProFile);
       }
*/
	  EndDialog (hDlg, TRUE);
	  break;

	case IDCANCEL:
	  EndDialog (hDlg, FALSE);
	  break;

	case IDD_MAILLOG:
	  SaveMailLog = !SaveMailLog;
	  CheckDlgButton (hDlg, IDD_MAILLOG, SaveMailLog);
	  EnableWindow(GetDlgItem(hDlg,IDD_MAILNAME),SaveMailLog);
	  break;
	case IDD_POSTLOG:
	  SavePostLog = !SavePostLog;
	  CheckDlgButton (hDlg, IDD_POSTLOG, SavePostLog);
	  EnableWindow(GetDlgItem(hDlg,IDD_POSTNAME),SavePostLog);
	  break;

	default:
	  return FALSE;
	}
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}


/*-- function WinVnMailDialog ---------------------------------------
 *
 *  Dialog function to query for mail destination, and manage list
 *  of mail addresses
 *  (JSC 2/1/94)
 */

BOOL FAR PASCAL 
WinVnMailDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WPARAM wParam;
     LPARAM lParam;
{
  HWND hDlgList;       /* Handle to child list box window. */
  register int j; 
  int count, len, startLen, found;
  char *ptr;
  char mybuf[MAXDIALOGSTRING];

  switch (iMessage)
    {
    case WM_INITDIALOG:
      hDlgList = GetDlgItem (hDlg, IDD_MAIL_LIST);

      SendMessage (hDlgList, WM_SETREDRAW, FALSE, 0L);
      for (j = 0; j < (int)MailList->numLines; j++)
	SendMessage (hDlgList, LB_ADDSTRING, 0, (LONG) TextBlockLine (MailList, j));

      SendMessage (hDlgList, WM_SETREDRAW, TRUE, 0L);
      return TRUE;
      break;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDD_MAIL_LIST:
#ifdef WIN32
	if (GET_WM_COMMAND_CMD(wParam,lParam) != LBN_DBLCLK)
#else
	if (HIWORD(lParam) != LBN_DBLCLK)
#endif /* WIN32 */
      	break;
        // a double click means they want to add a name to the to-line
        // only add it if it's not already on the to-line

        j = (int)SendDlgItemMessage (hDlg, IDD_MAIL_LIST, LB_GETCARETINDEX, 0, 0L);
        SendDlgItemMessage (hDlg, IDD_MAIL_LIST, LB_GETTEXT, j, (LPARAM) ((LPSTR) mybuf));

        GetDlgItemText (hDlg, IDD_MAIL_DEST, DialogString, MAXDIALOGSTRING-1);
        strcpy (str, DialogString);
	// test if name already on "To:" line
	// can't to a test with strstr() because mybuf may be a substring
	// of another address (i.e. 'jcoop' is a substr of 'jcooper@apl.com')
        for (found = FALSE, ptr = str; *ptr; ptr += len + 1)
        {                                         
          // if there's only one token left, then len will = startLen, 
          // and we'll iterate once only
          startLen = strlen (ptr);
          if ((len = strcspn (ptr, " ,\n\t\r")) != startLen)
          { 
            ptr[len] = '\0';			// replace delim with NULL char
            while (strchr (" ,\n\t\r", ptr[len+1]))	// eat white space
              ptr[len++] = '\0';
          }
	  if (!strcmp (ptr, mybuf))
          {
	    found = TRUE;
	    break;
	  }
          if (len == startLen)		// last token, we're done
            break;
	}          
        if (!found)
	{
          if (strlen (mybuf) + strlen (DialogString) + 2 > MAXDIALOGSTRING-1)
	  {
	    MessageBox (hDlg, "Cannot add another address", "Address line is too long", ID_OK);        		
            return FALSE;
          }
          if (DialogString[0])	// if this isn't the first one, prepend a comma
            strcat (DialogString, ", ");
          strcat (DialogString, mybuf);
          SetDlgItemText (hDlg, IDD_MAIL_DEST, DialogString);
        }
	break;               
 
      case IDD_ADD:
	// check addresses listed on to-line, add each one to list box 
	// (if not already in the list)
        GetDlgItemText (hDlg, IDD_MAIL_DEST, DialogString, MAXDIALOGSTRING-1);
        hDlgList = GetDlgItem (hDlg, IDD_MAIL_LIST);
        for (ptr = DialogString; *ptr; ptr += len + 1)
        {                                         
          // if there's only one token left, then len will = startLen, 
          // and we'll iterate once only
          startLen = strlen (ptr);
          if ((len = strcspn (ptr, " ,\n\t\r")) != startLen)
          { 
            ptr[len] = '\0';			// replace delim with NULL char
            while (strchr (" ,\n\t\r", ptr[len+1]))	// eat white space
              ptr[len++] = '\0';
          }
          if (*ptr != '\0')	// if not a blank
          {
            // check if it's already in the list
            count = (int)SendDlgItemMessage (hDlg, IDD_MAIL_LIST, LB_GETCOUNT, 0, 0L);
            for (j = 0; j < count; j++)
            {
              SendDlgItemMessage (hDlg, IDD_MAIL_LIST, LB_GETTEXT, j, (LPARAM) ((LPSTR) mybuf));
              if (!stricmp (ptr, mybuf))
                break;
            }
            if (j == count)		// if not already in list, add it
              SendMessage (hDlgList, LB_ADDSTRING, 0, (LONG) ptr);
          }          
          if (len == startLen)		// last token, we're done
            break;
	}          
        break;
        	
      case IDD_DELETE:
	// remove all selected addresses from list box
        count = (int)SendDlgItemMessage (hDlg, IDD_MAIL_LIST, LB_GETCOUNT, 0, 0L);
        
        hDlgList = GetDlgItem (hDlg, IDD_MAIL_LIST);
	SendMessage (hDlgList, WM_SETREDRAW, FALSE, 0L);
	
	// working backwards, delete selected items
        for (j = count-1; j >= 0; j--)
          if (SendDlgItemMessage (hDlg, IDD_MAIL_LIST, LB_GETSEL, j, 0L))
            SendDlgItemMessage (hDlg, IDD_MAIL_LIST, LB_DELETESTRING, j, 0L);

        SendMessage (hDlgList, WM_SETREDRAW, TRUE, 0L);
        break;
     
      case IDOK:
        GetDlgItemText (hDlg, IDD_MAIL_DEST, mybuf, MAXDIALOGSTRING-1);
	
	// make sure the address (if more than one) is comma separated
	DialogString[0]='\0';  
        for (ptr = mybuf; *ptr; ptr += len + 1)
        {                                         
          // if there's only one token left, then len will = startLen, 
          // and we'll iterate once only
          startLen = strlen (ptr);
          if ((len = strcspn (ptr, " ,\n\t\r")) != startLen)
          { 
            ptr[len] = '\0';			// replace delim with NULL char
            while (strchr (" ,\n\t\r", ptr[len+1]))	// eat white space
              ptr[len++] = '\0';
          }
	  
	  strcat (DialogString, ptr);
          
          if (len == startLen)		// last token, we're done
            break;
          else
            strcat (DialogString, ", ");
	}          

	// commit changes in listbox to internal MailList
	ResetTextBlock (MailList);
	count = (int)SendDlgItemMessage (hDlg, IDD_MAIL_LIST, LB_GETCOUNT, 0, 0L);
        for (j = 0; j < count; j++)
        {
	  SendDlgItemMessage (hDlg, IDD_MAIL_LIST, LB_GETTEXT, j, (LPARAM) ((LPSTR) mybuf));
	  AddLineToTextBlock (MailList, mybuf);
	}
        
        EndDialog (hDlg, TRUE);
        break;


      case IDCANCEL:
        // changes to MailList are _not_ committed on Cancel
        EndDialog (hDlg, FALSE);
        break;

   default:
     return FALSE;
   }
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}
                                   
                                   
