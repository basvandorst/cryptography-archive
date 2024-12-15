/*--- WVAUTH.C -- Routines for authentication in WinVN
 *
 *   Mark Riordan  14 August 1990
 */

#include <windows.h>

unsigned int newdes_buf(unsigned char *buf,unsigned int block_length);
void newdes_block(unsigned char *block);
void newdes_set_key_encipher(unsigned char *key);
void newdes_set_key_decipher(unsigned char *key);

int uuencode(unsigned char *bufin,unsigned int nbytes,
  unsigned char *bufcoded);
int uudecode(unsigned char *bufcoded,unsigned char *bufplain);


static unsigned char key[] = {
  0x58, 0x20, 0x41, 0xaf, 0xb4, 0xce, 0x58, 0xbb, 0xb0,
  0x1a, 0xaf, 0x6e, 0x00, 0x5e, 0x60
};


/*--- function AuthenticatePosting --------------------------------
 *
 *   Determine whether the user has sent valid authentication
 *   information to the host.
 *
 *    Entry    Authorized  is TRUE if we're already authorized.
 *
 *    Exit     Returns TRUE if we are authorized, else FALSE.
 *
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
AuthenticatePosting()
{

}

/*-- function WinVnAuthDlg ---------------------------------------
 *
 *  Window function to handle Save Article dialog box.
 */

BOOL FAR PASCAL WinVnAuthDlg(hDlg, iMessage, wParam, lParam)
HWND      hDlg;
unsigned  iMessage;
WORD      wParam;
LONG      lParam;
{

}
