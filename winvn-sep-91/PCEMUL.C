/*---  PCEMUL.C -- This file contains routines to emulate standard
 *  Unix or PC 'C' functions.
 *  Necessary because the verdammt FTP Software PC/TCP routines make
 *  some calls to non-Windows routines that aren't emulated in
 *  Microsoft's xLIBCEW.LIB.
 *
 *  Mark Riordan   11 August 1990
 */

#include <windows.h>
#include "wvglob.h"
#include <stdio.h>

/*--- function printf -------------------------------------------------
 *
 *  Very simple, incomplete emulation of printf.
 */
int
printf(fmt,arg1)
char *fmt;
char *arg1;
{
   char mybuf[256];

   sprintf(mybuf,fmt,arg1);
   MessageBox(hWndConf,mybuf,"printf",MB_OK);

   return(strlen(mybuf));
}

