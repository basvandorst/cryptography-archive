/*-- This is the first line of WVCNCM.C -------------------------------*/

#include <stdlib.h>
#include "windows.h"
#include "wvglob.h"
#include "winvn.h"

extern int WinVnDoComm(char *);


/*-- function WinVnCommDlg ---------------------------------------------
 *
 *   Dialog function to process the Configure Communications
 *   dialog box.
 */

BOOL FAR PASCAL WinVnCommDlg(hDlg, iMessage, wParam, lParam)
HWND      hDlg;
unsigned  iMessage;
WORD      wParam;
LONG      lParam;
{
#define MAXSTR  32
static int  MyCommPortID;
static int  MyCommParityID;
static int  MyCommModeID;
static char pszMyCommSpeed[MAXCOMMSPEEDCHARS];
static char pszMyNNTPPort[MAXSTR];

int write_ini = FALSE;
char *cptr;

#if 0
  MessageBox(hWndConf,"At Beginning of WinVncncm",
                     "(debug)",MB_OK | MB_ICONASTERISK);
#endif

  switch(iMessage) {

    case WM_INITDIALOG:
      MyCommPortID = CommPortID;
      MyCommParityID = CommParityID;
      strcpy(pszMyCommSpeed,pszCommSpeed);
      MyCommModeID = UsingSocket ? ID_COMMTCP : ID_COMMSERIAL;

      CheckRadioButton(hDlg, IDD_COM1, IDD_COM2, CommPortID);
      CheckRadioButton(hDlg, IDD_7EVEN, IDD_8NONE, CommParityID);
      CheckRadioButton(hDlg, ID_COMMTCP, ID_COMMSERIAL, MyCommModeID);
      SetDlgItemText(hDlg,IDD_SPEED,pszMyCommSpeed);
      SetDlgItemText(hDlg,ID_CONFIG_NNTP_SERVER,NNTPIPAddr);
      sprintf(pszMyNNTPPort,"%d", NNTPPort);
      SetDlgItemText(hDlg,ID_CONFIG_NNTP_PORT,pszMyNNTPPort);
      return TRUE;
      break;

    case WM_COMMAND:
      switch(wParam) {
        case ID_OK_SAVE:
            write_ini = TRUE;
        case IDOK:
          GetDlgItemText(hDlg,IDD_SPEED,pszCommSpeed,MAXCOMMSPEEDCHARS-1);
          CommPortID = MyCommPortID;
          CommParityID = MyCommParityID;
          CommIDtoStr(CommPortID,CommParityID,pszCommSpeed,szCommString);
          WinVnDoComm(szCommString);

          GetDlgItemText(hDlg,ID_CONFIG_NNTP_SERVER,NNTPIPAddr,MAXNNTPSIZE);
          GetDlgItemText(hDlg,ID_CONFIG_NNTP_PORT,pszMyNNTPPort,MAXSTR);
          NNTPPort = atoi(pszMyNNTPPort);

          if(write_ini) {
            WriteProfileString(szAppName,"CommString",szCommString);
            WriteProfileString(szAppName,"NNTPIPAddr",NNTPIPAddr);
            WriteProfileString(szAppName,"NNTPPort",pszMyNNTPPort);
            cptr = MyCommModeID==ID_COMMTCP ? "1" : "0";
            WriteProfileString(szAppName,"UseSocket",cptr);
          }
          EndDialog(hDlg,TRUE);
          break;

        case IDCANCEL:
          EndDialog(hDlg,FALSE);
          break;

        case ID_COMMTCP:
        case ID_COMMSERIAL:
          MyCommModeID = wParam;
          CheckRadioButton(hDlg, ID_COMMTCP, ID_COMMSERIAL, MyCommModeID);
          break;

        case IDD_COM1:
        case IDD_COM2:
          MyCommPortID = wParam;
          CheckRadioButton(hDlg,IDD_COM1,IDD_COM2,wParam);
          break;

        case IDD_7EVEN:
        case IDD_8NONE:
          MyCommParityID = wParam;
          CheckRadioButton(hDlg,IDD_7EVEN,IDD_8NONE,wParam);
          break;

        default:
          return FALSE;
      }
      break;

#if 0
    case WM_PAINT;
      InvalidateRect(
      break;
#endif
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
CommIDtoStr(Port,Parity,szSpeed,CommStr)
int Port, Parity;
char *szSpeed;
char *CommStr;
{
  register char *ptr;
  int myPort;

  ptr = CommStr;
  strcpy(ptr,"COM");
  ptr += 3;
  *(ptr++) = (char) (Port==IDD_COM1 ? '1' : '2');
  *(ptr++) = ':';

  for(;*szSpeed; *(ptr++) = *(szSpeed++));
  *(ptr++) = ',';
  if(Parity == IDD_7EVEN) {
    strcpy(ptr,"e,7");
  } else {
    strcpy(ptr,"n,8");
  }

  return (0);
}

/*-- function WinVnSaveArtDlg ---------------------------------------
 *
 *  Dialog function to handle Save Article dialog box.
 */

BOOL FAR PASCAL WinVnSaveArtDlg(hDlg, iMessage, wParam, lParam)
HWND      hDlg;
unsigned  iMessage;
WORD      wParam;
LONG      lParam;
{
static int  MyAppend;

#if 0
  MessageBox(hWndConf,"At Beginning of WinVnSaveArtDlg",
                     "(debug)",MB_OK | MB_ICONASTERISK);
#endif

  switch(iMessage) {

    case WM_INITDIALOG:
      MyAppend = SaveArtAppend;

      CheckDlgButton(hDlg, IDD_APPEND, MyAppend);
      SetDlgItemText(hDlg,IDD_FILENAME,SaveArtFileName);
      return TRUE;
      break;

    case WM_COMMAND:
      switch(wParam) {
        case IDOK:
          GetDlgItemText(hDlg,IDD_FILENAME,SaveArtFileName,MAXFILENAME-1);

          SaveArtAppend = MyAppend;
          if(!MRRWriteDocument(ActiveArticleDoc,sizeof(TypText),SaveArtFileName,SaveArtvRef,SaveArtAppend)) {
             MessageBox(hWndConf,"Could not write to file","Problems saving file",MB_OK|MB_ICONEXCLAMATION);
          }
          EndDialog(hDlg,TRUE);
          break;

        case IDCANCEL:
          EndDialog(hDlg,FALSE);
          break;

        case IDD_APPEND:
          MyAppend = !MyAppend;
          CheckDlgButton(hDlg,IDD_APPEND,MyAppend);
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

BOOL FAR PASCAL WinVnFindDlg(hDlg, iMessage, wParam, lParam)
HWND      hDlg;
unsigned  iMessage;
WORD      wParam;
LONG      lParam;
{
static int  MyAppend;

#if 0
  MessageBox(hWndConf,"At Beginning of WinVnSaveArtDlg",
                     "(debug)",MB_OK | MB_ICONASTERISK);
#endif

  switch(iMessage) {

   case WM_INITDIALOG:
      SetDlgItemText(hDlg,IDD_SEARCH_STRING,FindDoc->SearchStr);
      return TRUE;
      break;

   case WM_COMMAND:
      switch(wParam) {
         case IDOK:
            GetDlgItemText(hDlg,IDD_SEARCH_STRING,FindDoc->SearchStr,MAXFINDSTRING-1);

            EndDialog(hDlg,TRUE);
            break;

         case IDCANCEL:
            EndDialog(hDlg,FALSE);
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

BOOL FAR PASCAL WinVnDoListDlg(hDlg, iMessage, wParam, lParam)
HWND      hDlg;
unsigned  iMessage;
WORD      wParam;
LONG      lParam;
{

  switch(iMessage) {

   case WM_COMMAND:
      switch(wParam) {
         case IDOK:

            EndDialog(hDlg,TRUE);
            break;

         case IDCANCEL:

            EndDialog(hDlg,FALSE);
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

BOOL FAR PASCAL WinVnPersonalInfoDlg(hDlg, iMessage, wParam, lParam)
HWND      hDlg;
unsigned  iMessage;
WORD      wParam;
LONG      lParam;
{
#define MAXSTR  32
#if 0
static char *pszMyUserName;
static char *pszMyMailAddress;
static char *pszMyOrganization;
#endif

int write_ini = FALSE;
int dialog_val;
char *cptr;


  switch(iMessage) {

    case WM_INITDIALOG:
#if 0
      pszMyUserName = malloc(MAILLEN);
      pszMyMailAddress = malloc(MAILLEN);
      pszMyOrganization = malloc(MAILLEN);

      strcpy(pszMyOrganization,Organization);
      strcpy(pszMyUserName,UserName);
      strcpy(pszMyMailAddress,MailAddress);
#endif

      SetDlgItemText(hDlg,ID_CONFIG_EMAIL,MailAddress);
      SetDlgItemText(hDlg,ID_CONFIG_NAME,UserName);
      SetDlgItemText(hDlg,ID_CONFIG_ORG,Organization);
      return TRUE;
      break;

    case WM_COMMAND:
      switch(wParam) {
        case ID_OK_SAVE:
            write_ini = TRUE;
        case IDOK:
          GetDlgItemText(hDlg,ID_CONFIG_NAME,UserName,MAILLEN-1);
          GetDlgItemText(hDlg,ID_CONFIG_EMAIL,MailAddress,MAILLEN-1);
          GetDlgItemText(hDlg,ID_CONFIG_ORG,Organization,MAILLEN-1);

          if(write_ini) {
            WriteProfileString(szAppName,"UserName",UserName);
            WriteProfileString(szAppName,"MailAddress",MailAddress);
            WriteProfileString(szAppName,"Organization",Organization);
          }
          dialog_val = TRUE;
          goto endit;
          break;

        case IDCANCEL:
          dialog_val = FALSE;
       endit:;
#if 0
          free(pszMyUserName);
          free(pszMyMailAddress);
          free(pszMyOrganization);
#endif
          EndDialog(hDlg,dialog_val);
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

BOOL FAR PASCAL WinVnMiscDlg(hDlg, iMessage, wParam, lParam)
HWND      hDlg;
unsigned  iMessage;
WORD      wParam;
LONG      lParam;
{
int write_ini = FALSE;
int dialog_val;
int item;


  switch(iMessage) {

    case WM_INITDIALOG:
      CheckRadioButton(hDlg,ID_DOLIST_BASE,ID_DOLIST_ASK,DoList+ID_DOLIST_BASE);
      CheckDlgButton(hDlg, ID_CONFIG_APPEND, SaveArtAppend);
      CheckDlgButton(hDlg, ID_CONFIG_NEW_WND_GROUP, ViewNew);
      CheckDlgButton(hDlg, ID_CONFIG_NEW_WND_ARTICLE, NewArticleWindow);

      return TRUE;
      break;

    case WM_COMMAND:
      switch(wParam) {
        case ID_OK_SAVE:
            write_ini = TRUE;
        case IDOK:
          SaveArtAppend = (IsDlgButtonChecked(hDlg,ID_CONFIG_APPEND) != 0);
          ViewNew       = (IsDlgButtonChecked(hDlg,ID_CONFIG_NEW_WND_GROUP) != 0);
          NewArticleWindow = (IsDlgButtonChecked(hDlg,ID_CONFIG_NEW_WND_ARTICLE) != 0);

          for(item=ID_DOLIST_BASE; item<=ID_DOLIST_ASK; item++) {
            if(IsDlgButtonChecked(hDlg,item)) {
              DoList = item - ID_DOLIST_BASE;
            }
          }

          if(write_ini) {
            WriteProfileInt(szAppName,"SaveArtAppend",SaveArtAppend);
            WriteProfileInt(szAppName,"NewWndGroup",ViewNew);
            WriteProfileInt(szAppName,"NewWndArticle",NewArticleWindow);
            WriteProfileInt(szAppName,"DoList",DoList);
          }
          dialog_val = TRUE;
          goto endit;
          break;

        case IDCANCEL:
          dialog_val = FALSE;
       endit:;
          EndDialog(hDlg,dialog_val);
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

BOOL FAR PASCAL WinVnAppearanceDlg(hDlg, iMessage, wParam, lParam)
HWND      hDlg;
unsigned  iMessage;
WORD      wParam;
LONG      lParam;
{

}

BOOL
WriteProfileInt(lpAppName,lpKeyName,intval)
char far *lpAppName;
char far *lpKeyName;
int intval;
{
   char msg[20];

   itoa(intval,msg,10);
   return(WriteProfileString(lpAppName,lpKeyName,msg));
}
