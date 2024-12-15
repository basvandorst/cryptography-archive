/*
 *
 * $Id: wvcncm.c 1.10 1993/06/26 00:25:11 rushing Exp $
 * $Log: wvcncm.c $
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
#include "windows.h"
#include "wvglob.h"
#include "winvn.h"

extern int WinVnDoComm (char *);


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
  static char pszMyCommSpeed[MAXCOMMSPEEDCHARS];
  static char pszMyNNTPPort[MAXSTR];
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
      sprintf (pszMyNNTPPort, "%d", NNTPPort);
      SetDlgItemText (hDlg, ID_CONFIG_NNTP_PORT, pszMyNNTPPort);
		SetDlgItemText (hDlg, ID_CONFIG_AUTH_USERNAME, NNTPUserName);
		MRRDecrypt(NNTPPasswordEncrypted,(unsigned char *)NNTPPassword, MAXNNTPSIZE);
		SetDlgItemText (hDlg, ID_CONFIG_AUTH_PASSWORD, NNTPPassword);

      if (MailForceType == -1)
	mail_force_radio = IDD_MAIL_SELECT_AUTO;
      else
	mail_force_radio = MailForceType + IDD_MAIL_SELECT_NONE;

      CheckRadioButton (hDlg, IDD_MAIL_SELECT_NONE,
			IDD_MAIL_SELECT_AUTO, mail_force_radio);

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
	  GetDlgItemText (hDlg, ID_CONFIG_NNTP_PORT, pszMyNNTPPort, MAXSTR);
	  GetDlgItemText (hDlg, ID_CONFIG_AUTH_USERNAME, NNTPUserName, MAXNNTPSIZE);
	  GetDlgItemText (hDlg, ID_CONFIG_AUTH_PASSWORD, NNTPPassword, MAXNNTPSIZE);
	  MRREncrypt((unsigned char *)NNTPPassword,strlen(NNTPPassword)+1,
		NNTPPasswordEncrypted);
	  NNTPPort = atoi (pszMyNNTPPort);
	  AskComm = IsDlgButtonChecked (hDlg, ID_CONFIG_ASK_COMM);

	  if (write_ini)
	    {
/*	      WritePrivateProfileString
		(szAppName, "CommString", szCommString, szAppProFile);
*/
	      WritePrivateProfileString
		(szAppName, "NNTPHost", NNTPHost, szAppProFile);
	      WritePrivateProfileString
		(szAppName, "SMTPHost", SMTPHost, szAppProFile);
	      WritePrivateProfileString
		(szAppName, "NNTPPort", pszMyNNTPPort, szAppProFile);
			WritePrivateProfileString
		(szAppName, "NNTPUserName", NNTPUserName, szAppProFile);
			WritePrivateProfileString
		(szAppName, "NNTPPassword", NNTPPasswordEncrypted, szAppProFile);

	      WritePrivateProfileInt (szAppName, "MailForceType",
				      MailForceType, szAppProFile);

/*
	      cptr = MyCommModeID == ID_COMMTCP ? "1" : "0";
	      WritePrivateProfileString (szAppName, "UseSocket", cptr, szAppProFile);
	      WritePrivateProfileInt (szAppName, "AskComm", AskComm, szAppProFile);
*/
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
	  MailForceType = wParam - IDD_MAIL_SELECT_NONE;
	  break;

	case IDD_MAIL_SELECT_AUTO:
	  MailForceType = -1;
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
	  if (!MRRWriteDocument (ActiveArticleDoc, sizeof (TypText), SaveArtFileName, SaveArtvRef, SaveArtAppend))
	    {
	      MessageBox (hWndConf, "Could not write to file", "Problems saving file", MB_OK | MB_ICONEXCLAMATION);
	    }
	  EndDialog (hDlg, TRUE);
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

/*-- function WinVnSubjectDlg ---------------------------------------
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
	  GetDlgItemText (hDlg, IDD_SUBJECT_STRING,
			  DialogString, MAXDIALOGSTRING - 1);

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

	  if (write_ini)
	    {
	      WritePrivateProfileString (szAppName, "UserName", UserName, szAppProFile);
	      WritePrivateProfileString (szAppName, "MailAddress", MailAddress, szAppProFile);
	      WritePrivateProfileString (szAppName, "Organization", Organization, szAppProFile);
	    }
	  dialog_val = TRUE;
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

	  if (write_ini)
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

	    }
	  dialog_val = TRUE;
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

    case WM_COMMAND:
      switch (wParam)
	{
	case ID_OK:
	  GetDlgItemText (hDlg, IDD_ARTS_TO_GRAB, manual, 6);
	  arts_to_retrieve = atoi (manual);
	  if (arts_to_retrieve > 0)
	    EndDialog (hDlg, TRUE);
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

BOOL
WritePrivateProfileInt (lpAppName, lpKeyName, intval,lpProFile)
     char far *lpAppName;
     char far *lpKeyName;
     char far *lpProFile;
     int intval;
{
  char msg[20];

  itoa (intval, msg, 10);
  return (WritePrivateProfileString (lpAppName, lpKeyName, msg, lpProFile));
}
