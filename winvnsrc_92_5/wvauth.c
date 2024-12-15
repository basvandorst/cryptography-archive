
/*
 *
 * $Id: wvauth.c 1.3 1994/08/24 18:40:15 mrr Exp $
 * $Log: wvauth.c $
 * Revision 1.3  1994/08/24  18:40:15  mrr
 * authorization enables post/mail
 *
 * Revision 1.2  1993/12/08  01:28:38  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.1  1993/02/16  20:53:50  rushing
 * Initial revision
 *
 *
 */

/*--- WVAUTH.C -- Routines for authentication in WinVN
 *
 *   Mark Riordan  14 August 1990
 */

#include <windows.h>
#include "wvglob.h"

unsigned int newdes_buf (unsigned char *buf, unsigned int block_length);
void newdes_block (unsigned char *block);
void newdes_set_key_encipher (unsigned char *key);
void newdes_set_key_decipher (unsigned char *key);

int uuencode (unsigned char *bufin, unsigned int nbytes,
         unsigned char *bufcoded);
int uudecode (unsigned char *bufcoded, unsigned char *bufplain);


static unsigned char key[]=
{
  0x58, 0x20, 0x41, 0xaf, 0xb4, 0xce, 0x58, 0xbb, 0xb0,
  0x1a, 0xaf, 0x6e, 0x00, 0x5e, 0x60
};


/*--- function AuthenticatePosting --------------------------------
 *
 *   Determine whether the user has sent valid authentication
 *   information to the host.
 *
 *    Entry    Authenticated  is TRUE if we're already authorized.
 *             AuthReq        is TRUE if we need authorization.
 *
 *    Exit     Returns TRUE if we are authorized, else FALSE.
 *                                                             
 *    This routine is largely unimplemented.
 *    Intended
 *    Method   If we're already authorized, just return TRUE.
 *             Otherwise, make sure the comm channel to the server
 *             isn't busy.  If it is, put up a message to that
 *             effect and quit.
 *             If not busy, get authorization information from the
 *             user and present it to the server (in encoded form).
 *             Wait until the server responds or until the Abort
 *             Protocol flag is set (by the user via a menu).
 *             If authorized, return TRUE, else put up a
 *             message to the user and return FALSE.
 */
BOOL
AuthenticatePosting (AuthReq)
BOOL AuthReq;
{  
   BOOL whetherOK=FALSE;
                      
   if(AuthReq && !Authenticated) {
      MessageBox(hWndConf,
      "Sorry, you must identify yourself to the news server \
before sending.  Choose the Config/Configure Comm... menu option \
and enter your username and password.  \
Then Choose Network/Disconnect and Network/Connect.",
"No permission",MB_OK);
   } else {
      whetherOK = TRUE;
   }
   
   return(whetherOK);
}

/*-- function WinVnAuthDlg ---------------------------------------
 *
 *  Window function to handle Save Article dialog box.
 */

BOOL FAR PASCAL 
WinVnAuthDlg (hDlg, iMessage, wParam, lParam)
     HWND hDlg;
     unsigned iMessage;
     WORD wParam;
     LONG lParam;
{
   return (TRUE);

}
   
/*--- function AddAuthHeader --------------------------------
 *
 *  Add an X-Authenticated-User: line to the beginning
 *  of an edit control.
 *
 *  Entry:  hWndEdit    is the handle of an edit control.
 *          AuthenticatedName    is the name of the user; it's been
 *                               checked by the server.
 */    
void
AddAuthHeader(hWndEdit)
HWND hWndEdit;
{
   char line[MAXHEADERLINE];
   
   sprintf(line,"X-Authenticated-User: %s\r\n",AuthenticatedName);
   SendMessage(hWndEdit,EM_SETSEL,1,MAKELPARAM(0,0));
   SendMessage(hWndEdit,EM_REPLACESEL,0, (LPARAM) ((LPCSTR) line));
   
} 