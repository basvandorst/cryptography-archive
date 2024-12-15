head	 1.22;
branch   ;
access   ;
symbols  V80:1.4 V76d:1.3;
locks    ; strict;
comment  @ * @;


1.22
date	 94.09.16.01.06.33;  author jcooper;  state Exp;
branches ;
next	 1.21;

1.21
date	 94.08.03.22.17.18;  author dumoulin;  state Exp;
branches ;
next	 1.20;

1.20
date	 94.08.03.00.35.20;  author dumoulin;  state Exp;
branches ;
next	 1.19;

1.19
date	 94.08.02.23.30.11;  author dumoulin;  state Exp;
branches ;
next	 1.18;

1.18
date	 94.08.02.05.01.48;  author dumoulin;  state Exp;
branches ;
next	 1.17;

1.17
date	 94.07.28.21.19.12;  author dumoulin;  state Exp;
branches ;
next	 1.16;

1.16
date	 94.07.28.21.14.05;  author dumoulin;  state Exp;
branches ;
next	 1.15;

1.15
date	 94.07.26.22.17.28;  author dumoulin;  state Exp;
branches ;
next	 1.14;

1.14
date	 94.06.30.16.32.13;  author dumoulin;  state Exp;
branches ;
next	 1.13;

1.13
date	 94.06.23.23.11.30;  author dumoulin;  state Exp;
branches ;
next	 1.12;

1.12
date	 94.06.17.09.08.14;  author dumoulin;  state Exp;
branches ;
next	 1.11;

1.11
date	 94.06.14.20.22.37;  author dumoulin;  state Exp;
branches ;
next	 1.10;

1.10
date	 94.05.19.02.06.10;  author dumoulin;  state Exp;
branches ;
next	 1.9;

1.9
date	 94.01.15.20.41.22;  author jcoop;  state Exp;
branches ;
next	 1.8;

1.8
date	 93.12.08.01.28.38;  author rushing;  state Exp;
branches ;
next	 1.7;

1.7
date	 93.08.09.17.57.47;  author dumoulin;  state Exp;
branches ;
next	 1.6;

1.6
date	 93.08.05.15.46.42;  author dumoulin;  state Exp;
branches ;
next	 1.5;

1.5
date	 93.07.06.21.09.09;  author cnolan;  state Exp;
branches ;
next	 1.4;

1.4
date	 93.05.25.00.00.55;  author rushing;  state Exp;
branches ;
next	 1.3;

1.3
date	 93.03.09.01.24.21;  author dumoulin;  state Exp;
branches ;
next	 1.2;

1.2
date	 93.02.16.22.52.25;  author dumoulin;  state Exp;
branches ;
next	 1.1;

1.1
date	 93.02.16.20.54.22;  author rushing;  state Exp;
branches ;
next	 ;


desc
@winvn version 0.76 placed into RCS
@


1.22
log
@rearranged headers to allow use of precompiled headers
@
text
@/***************************************************************************
 *									   *
 *  MODULE	: WVPRINT.C						   *
 *									   *
 *                  Jim Dumoulin  NASA/KSC                                 *
 *                                                                         *
 *                                                                         *
 *  PURPOSE	: Printing code for WinVN.                                 *
 *									   *
 *  FUNCTIONS	: FreePrinterMemory ()	   -  Frees all memory associated  *
 *                                            with a printer device context*
 *                                                                         *
 *  		: GetPrinterDC ()	   -  Creates a printer DC for the *
 *					      default device.		   *
 *									   *
 *  		: DeletePrinterDC ()	   -  Deletes a printer DC for the *
 *					      default device.		   *
 *									   *
 *		  AbortProc ()		   -  Export proc. for GDI to check*
 *									   *
 *					      print abort.		   *
 *									   *
 *		  PrintDlgProc ()	   -  Dialog function for the print*
 *					      cancel dialog.		   *
 *                                                                         *
 *                ReportPrintError ()     -  Decodes err codes for calls   *
 *                                            to Windows print functions   *
 *									   *
 *                PrintHeaderP ()          -  Determines if header string  *
 *                                            is one that gets printed     *
 *                                                                         *
 *		  PrintArticle ()          -  Prints the contents of the   *
 *					      an article window.	   *
 ***************************************************************************/


/*
 *
 * $Id: wvprint.c 1.21 1994/08/03 22:17:18 dumoulin Exp $
 * $Log: wvprint.c $
 * Revision 1.21  1994/08/03  22:17:18  dumoulin
 * Cleaned up word wrapping in error messages
 *
 * Revision 1.20  1994/08/03  00:35:20  dumoulin
 * Fixed bug upon exit and startup for folks that don't have
 * any printers.
 *
 * Revision 1.19  1994/08/02  23:30:11  dumoulin
 * Fixed problem with AbortProc that could cause Printing to not
 * work with some print drivers
 *
 * Revision 1.18  1994/08/02  05:01:48  dumoulin
 * More testing to debug WinVN printing problem with some drivers
 *
 * Revision 1.17  1994/07/28  21:19:12  dumoulin
 * removed print.h
 *
 * Revision 1.16  1994/07/28  21:14:05  dumoulin
 * yet another attempt at fixing printing
 *
 * Revision 1.15  1994/07/26  22:17:28  dumoulin
 * Changed function prototype for printing function
 *
 * Revision 1.14  1994/06/30  16:32:13  dumoulin
 * Fix Printing Bug - Call stack problem with AbortProc Procedure
 *
 * Revision 1.13  1994/06/23  23:11:30  dumoulin
 * Added support for new Print Dialog box and features
 *
 * Revision 1.12  1994/06/17  09:08:14  dumoulin
 * Attempts to fix the Unable to Initialize Document printing bug
 *
 * Revision 1.11  1994/06/14  20:22:37  dumoulin
 * Fix various printer problems due to short instead of int variable
 *
 * Revision 1.10  1994/05/19  02:06:10  dumoulin
 * *** empty log message ***
 *
 * Revision 1.9  1994/01/15  20:41:22  jcoop
 * Use GetEditText instead of EM_GETHANDLE with global editWnds
 *
 * Revision 1.8  1993/12/08  01:28:38  rushing
 * new version box and cr lf consistency
 * 
 * Revision 1.7  1993/08/09  17:57:47  dumoulin
 * Fix to AbortProc and PrintArticle to fix GPFs
 *
 * Revision 1.6  1993/08/05  15:46:42  dumoulin
 * Rewrote PrintArticle to solve print crashes
 *
 * Revision 1.5  1993/07/06  21:09:09  cnolan
 * win32 support
 *
 * Revision 1.4  1993/05/25  00:00:55  rushing
 * rot13 merge (MRB)
 *
 * Revision 1.3  1993/03/09  01:24:21  dumoulin
 * Added italics for quoted regions in prints
 *
 * Revision 1.2  1993/02/16  22:52:25  dumoulin
 * moved rcs ident comment below file description
 *
 * Revision 1.1  1993/02/16  20:54:22  rushing
 * Initial revision
 *
 *
 */ 
#include <windows.h>
#include <windowsx.h>
#include "wvglob.h"
#include "winvn.h"
#pragma hdrstop

BOOL PdlgAbort = FALSE;        	/* TRUE if the user has aborted the print job	 */
HWND hwndPDlg = NULL;     	/* Handle to the cancel print dialog		 */

/****************************************************************************
 *									    *          
 *  FUNCTION   : AbortProc()						    *
 *									    *
 *  PURPOSE    : To be called by GDI print code to check for user abort.    *
 *               Returns TRUE to continue Printing, FALSE to cancel.        *
 *									    *
 ****************************************************************************/
BOOL CALLBACK AbortProc ( HDC hdc,int nCode)
{
  MSG msg; 
  char mes[60];
    
    if ((nCode < 0) && (nCode != SP_OUTOFDISK))
      {
        sprintf(mes,"AbortProc Error %d", (int) nCode);
        MessageBox(NULL, "Your Windows Print Driver CallBack \n procedure "
                         "returned an error",mes,MB_OK | MB_ICONEXCLAMATION);
        return FALSE; 
      }
    else                  
    /* Allow other apps to run, or get abort messages */
  //  while (!PdlgAbort && PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
       while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
         {
	   if (!hwndPDlg || !IsDialogMessage (hwndPDlg, &msg))
	        {
	         TranslateMessage (&msg);
	         DispatchMessage  (&msg);
	        }
          }

      if (PdlgAbort == TRUE) return FALSE;
       else
        return TRUE;
}


/****************************************************************************
 *									    *
 *  FUNCTION   : PrinterInit ()				   	            *
 *									    *
 *  PURPOSE    : Initializes Global Variables used by the Printing Code     *
 *									    *
 *  RETURNS    : TRUE if successful, FALSE if not                     	    *
 *									    *
 ****************************************************************************/
BOOL WINAPI PrinterInit(void)
 {
   pd.lStructSize = (DWORD) sizeof(PRINTDLG);
   pd.hDevMode = NULL;
   pd.hDevNames = NULL;
   pd.Flags = PD_RETURNDC;
   pd.hDC = (HDC) NULL;
   pd.nFromPage = 0;
   pd.nToPage = 0;
   pd.nMinPage = 0;
   pd.nMaxPage = 0xFFFE;
   pd.nCopies = 1;
   pd.hInstance = (HANDLE) NULL;
   pd.lCustData = 0L;
   pd.lpfnPrintHook = (UINT) NULL;
   pd.lpfnSetupHook = (UINT) NULL;
   pd.lpPrintTemplateName = (LPSTR) NULL;
   pd.lpSetupTemplateName = (LPSTR) NULL;
   pd.hPrintTemplate = (HANDLE) NULL;
   pd.hSetupTemplate = (HANDLE) NULL; 
   
   hFontPrint = NULL;
   hFontPrintB = NULL;
   hFontPrintI = NULL;
   hFontPrintS = NULL;
   
   return TRUE;
 }
      
/****************************************************************************
 *									    *
 *  FUNCTION   : FreePrinterMemory ()					    *
 *									    *
 *  PURPOSE    : Frees any memory structures allocated for the Printer      *
 *		 Device context.                                            *
 *		 It also sets iPrinter to the supported level of printing.  *
 *									    *
 *  RETURNS    : TRUE if successful else FALSE                  	    *
 *									    *
 ****************************************************************************/
BOOL WINAPI FreePrinterMemory(void)
 {
  if (pd.hDevMode)  GlobalFree(pd.hDevMode);
  if (pd.hDevNames) GlobalFree(pd.hDevNames);
  if (pd.hDC) DeletePrinterDC(pd.hDC);              
  PrinterInit();
  return TRUE;
 }
 
/****************************************************************************
 *									    *
 *  FUNCTION   : PrinterSetup ()					    *
 *									    *
 *  PURPOSE    : Creates a printer display context for the default device.  *
 *		 As a side effect, it sets the szDevice and szPort variables*
 *		 It also sets iPrinter to the supported level of printing.  *
 *									    *
 *  RETURNS    : Zero if successful, 1 = cancel, else Extended Error Code   *
 *									    *
 ****************************************************************************/
DWORD WINAPI PrinterSetup(HWND hwnd,DWORD flags)
 { 
  char mes[60];
  DWORD  cError = 0;
            
  pd.hwndOwner = hwnd;
  pd.Flags = flags;
                                           
  if (pd.hDC != 0) DeletePrinterDC(pd.hDC);
   
  if (PrintDlg(&pd) == 0)
    {                              
      cError = CommDlgExtendedError();
      if (cError != 0)
       { 
        sprintf(mes,"Comm Dialog Box Extended Error %d", (DWORD) cError);
        MessageBox(hwnd, "WinVN was unable to either display a \n"
                         "Printer Device Context Dialog Box or \n"
                         "to get a Printer Device Context"
                         ,mes,MB_OK | MB_ICONEXCLAMATION);
        FreePrinterMemory();
        return cError;
       }
       else return 1; 
    } 
    else
      return 0;
 }
 
/****************************************************************************
 *									    *
 *  FUNCTION   : GetPrinterDC ()					    *
 *									    *
 *  PURPOSE    : Finds or creates a printer display context for the         *
 *               selected printer.                                          *
 *									    *
 *  RETURNS    : HDC - A handle to printer DC or Null if error        	    *
 *									    *
 ****************************************************************************/
HDC WINAPI GetPrinterDC(HWND hwnd)
 {        
  DWORD pError = 0;
  
  if (pd.hDC)
      return pd.hDC;
  else 
    {
     if ((pd.hDevMode == NULL) && (pd.hDevNames == NULL))
       pError = PrinterSetup(hwnd,PD_RETURNDC | PD_RETURNDEFAULT);
     else 
       pError = PrinterSetup(hwnd,PD_RETURNDC);
      
     if (pError > 0) return NULL;
     else  return pd.hDC;
    }
 }

/****************************************************************************
 *									    *
 *  FUNCTION   : DeletePrinterDC ()					    *
 *									    *
 *  PURPOSE    : Releases a printer display context for the selected        *
 *               printer.                                                   *
 *									    *
 *  RETURNS    : 0 if Successful, 1 if Error                    	    *
 *									    *
 ****************************************************************************/

BOOL WINAPI DeletePrinterDC(HDC hDC)
 {
  BOOL err1 = FALSE;
  BOOL err2 = FALSE;

  if (pd.hDC != hDC)
    err1 = DeleteDC(pd.hDC);
    
  err2 = DeleteDC (hDC);
  pd.hDC = NULL; 
  return err1 & err2;
  }


/****************************************************************************
 *									    *
 *  FUNCTION   : PrintDlgProc ()					    *
 *									    *
 *  PURPOSE    : Dialog function for the print cancel dialog box.	    *
 *									    *
 *  RETURNS    : TRUE  - OK to abort/ not OK to abort			    *
 *		 FALSE - otherwise.					    *
 *									    *
 ****************************************************************************/
LRESULT CALLBACK PrintDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
 {                    
    switch (msg)
      {                    
	case WM_INITDIALOG:
	    hwndPDlg = hwnd;
	    ShowWindow (hwnd, SW_SHOW);
	    break;
 
	case WM_COMMAND:   /* abort printing if the cancel button gets hit */
	    switch (LOWORD (wParam))
	     { 
	      case ID_OK:
	      case ID_CANCEL:
	            
	         PdlgAbort = TRUE;
	         EnableWindow(GetParent (hwnd), TRUE);
                 if (hwndPDlg) DestroyWindow (hwndPDlg);    /* Delete Cancel Dialog */            
                 hwndPDlg = NULL;
	         return TRUE; 
	      }
	      break;
      }
    return FALSE;
 }

/****************************************************************************
 *									    *
 *  FUNCTION   : ReportPrintError ()					    *
 *									    *
 *  PURPOSE    : Decodes error codes from calls to Windows Print functions  *
 *									    *
 *  RETURNS    : NULL  - No errors or Error has already been reported       *
 *		 short - Error code numbers (from Windows.H)                *
 *									    *
 ****************************************************************************/

void WINAPI ReportPrintError(int nError,HWND hWnd)
     {
      char mes[60];

	if (nError >= 0)
	   return;

	if ((nError & SP_NOTREPORTED) == 0)
	   return;

        switch(nError)
         {
          case  SP_ERROR:
              MessageBox(hWnd, "The Windows Printer Device driver \n"
                               "cannot begin printing your document. \n"
                               "Your printer may be off line or out of \n"
                               "paper.  It is also possible that the \n"
                               "Windows Print Manager isn't loaded and \n"
                               "another program is currently printing",
                               "General Printing Error",
                               MB_OK | MB_ICONEXCLAMATION);
              break;

          case  SP_APPABORT:
              MessageBox(hWnd, "Your Print request has been canceled \n"
                               "via an Abort request from your application",
                               "Print Canceled by Application",
                               MB_OK | MB_ICONEXCLAMATION);
              break;

          case  SP_USERABORT:
              MessageBox(hWnd, "Your Print request has been canceled \n"
                               "via a User cancel request from the \n"
                               "Windows Print Manager",
                               "Print Canceled by User",
                               MB_OK | MB_ICONEXCLAMATION);
              break;

          case  SP_OUTOFDISK:
              MessageBox(hWnd, "Your Print request has been aborted \n"
                               "due to insufficient disk space in your \n"
                               "Windows TEMP subdirectory",
                               "Out of Disk Space",
                               MB_OK | MB_ICONEXCLAMATION);
              break;

          case  SP_OUTOFMEMORY:
              MessageBox(hWnd, "Your Print request has been aborted \n"
                               "due to insufficient Windows memory.\n"
                               "Close some applications and try again",
                               "Out of Memory Space",
                               MB_OK | MB_ICONEXCLAMATION);
              break;

         default:

              sprintf(mes,"Unknown Print Error %d", (int) nError);
              MessageBox(hWnd, "Your Print request has been aborted due to \n"
                               "the Windows Print function returning an error \n"
                               "code that is Undocumented",mes,
                               MB_OK | MB_ICONEXCLAMATION);
              break;

         }
	return;
     }


/****************************************************************************
 *									    *
 *  FUNCTION   : PrintHeaderP ()					    *
 *									    *
 *  PURPOSE    : Determines if this is a header we are printing		    *
 *									    *
 *        Entry:  str    substring to search                                *
 *                limit  limits the search to no more than num characters   *
 *                                                                          *
 *        Exit:   BOOL   TRUE if we are not excluding print this header     *
 *                       FALSE if we are explicited excluding header        *
 *									    *
 ****************************************************************************/

BOOL WINAPI PrintHeaderP(char *str,int limit)
  {
    if (CompareStringNoCase(str,"Relay-Version:",limit) == 0
        || CompareStringNoCase(str,"Path:",limit) == 0
        || CompareStringNoCase(str,"References:",limit) == 0
        || CompareStringNoCase(str,"Xref:",limit) == 0)
      return(FALSE);
    else
      return(TRUE);
  } 


/****************************************************************************
 *									    *
 *  FUNCTION   : PrintFile ()						    *
 *									    *
 *  PURPOSE    : Prints the contents of the edit control.		    *
 *									    *
 ****************************************************************************/

void WINAPI PrintFile(HWND hwnd)
{
    HDC     hdc;
    int     yExtPage;
    char    sz[32];
    WORD    cch;
    WORD    ich;
    char *  pch;
    WORD    iLine;
    WORD    nLinesEc;
    char *  pT;
    ABORTPROC lpfnAbort;
    DLGPROC   lpfnPDlg;
    WORD    dy;
    int     yExtSoFar;
    WORD    fError = TRUE;
    HWND    hwndEdit;
    SIZE	size;

    hwndEdit = (HWND)GetWindowWord(hwnd,GWW_HWNDEDIT);
    lstrcpy(sz,"WinVN Print Request");
    cch = lstrlen(sz);
    PdlgAbort = FALSE;
    
    /* Make instances of the Abort proc. and the Print dialog function */
    lpfnAbort =(ABORTPROC) MakeProcInstance ((FARPROC) AbortProc, hInst);
    if (!lpfnAbort)
	goto getout;
    lpfnPDlg = (DLGPROC) MakeProcInstance ((FARPROC) PrintDlgProc, hInst);
    if (!lpfnPDlg)
	goto getout4;

    /* Initialize the printer */
 //   hdc = GetPrinterDC();
    if (!hdc)
	goto getout5;

    /* Disable the main application window and create the Cancel dialog */
  /*    EnableWindow (hwndFrame, FALSE); */
    hwndPDlg = CreateDialog (hInst, "PRINTDIALOG", hwnd, lpfnPDlg);
    if (!hwndPDlg)
	goto getout3;
    ShowWindow (hwndPDlg, SW_SHOW);
    UpdateWindow (hwndPDlg);

    /* Allow the app. to inform GDI of the escape function to call */
    if (Escape (hdc, SETABORTPROC, 0, (LPSTR)lpfnAbort, NULL) < 0)
	goto getout1;

    /* Initialize the document */
    if (Escape (hdc, STARTDOC, cch, (LPSTR)sz, NULL) < 0)
	goto getout1;

    /* Get the height of one line and the height of a page */
    GetTextExtentPoint (hdc, "CC", 2, &size);
    dy=size.cy;
    yExtPage = GetDeviceCaps (hdc, VERTRES);

    /* Get the lines in document and and a handle to the text buffer */
    iLine     = 0;
    yExtSoFar = 0;
    nLinesEc  = (WORD)SendMessage (hwndEdit, EM_GETLINECOUNT, 0, 0L);
//    hT      = (HANDLE)SendMessage (hwndEdit, EM_GETHANDLE, 0, 0L);
    pT = GetEditText (hwndEdit);
    
    /* While more lines print out the text */
    while ((iLine < nLinesEc) && !PdlgAbort) {
	if (yExtSoFar + (int)dy > yExtPage){
	    /* Reached the end of a page. Tell device driver to eject page */
	    if (Escape (hdc, NEWFRAME, 0, NULL, NULL) < 0 || PdlgAbort)
		goto getout2;
	    yExtSoFar = 0;
	}

	/* Get the length and position of the line in the buffer
	 * and lock from that offset into the buffer */
	ich = (WORD)SendMessage (hwndEdit, EM_LINEINDEX, iLine, 0L);
	cch = (WORD)SendMessage (hwndEdit, EM_LINELENGTH, ich, 0L);
//	pch = (char *)LocalLock(hT) + ich;
	pch = pT + ich;

	/* Print the line and unlock the text handle */
	TextOut (hdc, 0, yExtSoFar, (LPSTR)pch, cch);
//	LocalUnlock (hT);

	/* Move down the page */
	yExtSoFar += dy;
	iLine++;
    }
    GlobalFreePtr (pT);
    
    /* Eject the last page. */
    if (Escape (hdc, NEWFRAME, 0, NULL, NULL) < 0)
	goto getout2;

    /* Complete the document. */
    if (Escape (hdc, ENDDOC, 0, NULL, NULL) < 0){
getout2:
	/* Ran into a problem before NEWFRAME? Abort the document */
	Escape( hdc, ABORTDOC, 0, NULL, NULL);
    }
    else
	fError=FALSE;

getout3:
    /* Close the cancel dialog and re-enable main app. window */
    /*    EnableWindow (hwndFrame, TRUE);   */
    DestroyWindow (hwndPDlg);

getout1:
    DeleteDC(hdc);

getout5:
    /* Get rid of dialog procedure instances */
    FreeProcInstance ((FARPROC) lpfnPDlg);

getout4:
    FreeProcInstance ((FARPROC) lpfnAbort);

getout:

    /* Error? make sure the user knows... */
    if (fError)
        MessageBox(hwnd, "Printing Error","Error",MB_OK);
    return;
} 

/****************************************************************************
 *									    *
 *  FUNCTION   : PrintArticle ()					    *
 *									    *
 *  PURPOSE    : Prints the current article.             		    *
 *									    *
 ****************************************************************************/

#define LEFTMARGIN 6        /* Left Margin in characters on printed page */
#define TOPMARGIN 4         /* Top Margin in characters on printed page */
#define BOTTOMMARGIN 6      /* Bottom Margin in characters on printed page */
#define MAXHEADERSIZE 20    /* Largest number of chars allowed in a header name */

void WINAPI PrintArticle(HWND hwnd,TypDoc * Doc)
{    
    char    sz[100];
    char    szTitle[MAXHEADERLINE];
    DOCINFO  di;
    BOOL    found;
    BOOL    inheader = TRUE; 
    int     i,dy,dx,yExtPage,yExtSoFar,nPrintError;
    unsigned int LineLen,Offset,nTotalPages, PageNum, iLine,
                 nCharsPerLine,nLinesPerPage,nTotalLines;
    char    far *textptr;
    char    *loc;
    DWORD   cError = 0;
    TypBlock far *BlockPtr;
    TypLine  far *LinePtr;
    HANDLE hBlock;
    TypLineID MyLineID;
    TEXTMETRIC tm;
    DEVNAMES * dv;
    ABORTPROC lpfnAbortProc = NULL;
    DLGPROC   lpfnPrintDlgProc = NULL;
      
    /* Create the job title */
    di.cbSize = sizeof(di);
    di.lpszDocName = "WinVn Article";
    di.lpszOutput = NULL;

    /* Initialize the printer */ 
    nPrintError = 0;
    hwndPDlg = NULL;
    PdlgAbort = FALSE;
    cError = PrinterSetup(hwnd,PD_RETURNDC | PD_USEDEVMODECOPIES); 
    if ((cError != 0) || (pd.hDC == 0)) goto exitout;

    /* Reinitialize Fonts just in case the user changed printers on us */
    InitPrintFonts(); 
                                                     
    /*  Create the Cancel dialog and Disable the main application window */
    lpfnPrintDlgProc = (DLGPROC) MakeProcInstance((FARPROC) PrintDlgProc,(HINSTANCE) hInst);
    if (!lpfnPrintDlgProc) goto exitout;
    hwndPDlg = CreateDialog ((HINSTANCE) hInst,(LPCSTR) "PRINTDIALOG",hwnd, (DLGPROC) lpfnPrintDlgProc);
    if (!hwndPDlg) goto exitout;

    /* Allow the app. to inform GDI of the Abort function to call */ 
    lpfnAbortProc = (ABORTPROC) MakeProcInstance((FARPROC) AbortProc,(HINSTANCE) hInst);
    if (!lpfnAbortProc) goto exitout; 
    
    EnableWindow (hwnd, FALSE);     
    if (SetAbortProc(pd.hDC,(ABORTPROC) lpfnAbortProc) < 0)
      {
        MessageBox(hwnd, "Unable to Set Abort Procedure",
                         "Error",MB_OK | MB_ICONEXCLAMATION);
        nPrintError = 0;     /* don't print double error messages */
        goto exitout;
      }

    /* Get the Subject, printer description and Port number */ 
    found = GetHeaderLine(Doc,"Subject:",szTitle,sizeof(szTitle));
    if (!found) lstrcpy(szTitle,"Subject: No Subject");
    SetDlgItemText ((HWND) hwndPDlg, IDD_PRINTSUBJECT, (LPSTR)szTitle);
    dv = (DEVNAMES *) GlobalLock(pd.hDevNames);
    sprintf(sz,"To %s on %s",(LPSTR) dv+dv->wDeviceOffset,(LPSTR) dv+dv->wOutputOffset);    
    SetDlgItemText ((HWND) hwndPDlg, IDD_PRINTDEVICE, (LPSTR)sz);
    GlobalUnlock(pd.hDevNames);               
    sprintf(sz,"Initializing Document for Printing");
    SetDlgItemText ((HWND) hwndPDlg, IDD_PRINTSTATUS, (LPSTR)sz);

    /*  Initialize the Printer Device Context */
    nPrintError = StartDoc(pd.hDC,&di);
    if (hwndPDlg)
      {
       UpdateWindow (hwndPDlg);              /* print to file may overwrite dialog */
       SetFocus(hwndPDlg);
      } 
    if (nPrintError < 0) goto exitout;

    LockLine (Doc->hFirstBlock, sizeof(TypBlock), (TypLineID)0L,
               &BlockPtr, &LinePtr);

    /* Get the lines in document and and a handle to the text buffer */
    iLine     = 0;
    PageNum   = 1;
    nTotalLines = Doc->TotalLines;

    /* Get the height of one line and the height of a page */
    SelectObject(pd.hDC,hFontPrint);         /* Select Printer Font */
    GetTextMetrics(pd.hDC,&tm);
    dy = tm.tmHeight + tm.tmExternalLeading;
    dx = tm.tmAveCharWidth;
    nCharsPerLine = GetDeviceCaps (pd.hDC,HORZRES) / dx;
    nLinesPerPage = GetDeviceCaps (pd.hDC,VERTRES) / dy;
    nTotalPages = (nTotalLines + BOTTOMMARGIN + TOPMARGIN) / nLinesPerPage; 
    if (nTotalPages < 1) nTotalPages = 1;
    yExtPage = GetDeviceCaps (pd.hDC, VERTRES);
    nPrintError = StartPage(pd.hDC);
    if (nPrintError <= 0) goto abortout;
                
   /* If we are printing the First Page, place the Subject line at the Top */
    if ((((pd.Flags & PD_PAGENUMS) == 0) && ((pd.Flags & PD_SELECTION) == 0)) ||
        (pd.nFromPage == 0) || 
        (pd.nFromPage == 1))
      {     
        sprintf(sz,"Now Printing Page %u of %u",PageNum,nTotalPages);
        SetDlgItemText (hwndPDlg, IDD_PRINTSTATUS, (LPSTR)sz);
        SelectObject(pd.hDC,hFontPrintS);         /* Select Printer Font */ 
        TextOut (pd.hDC,LEFTMARGIN * dx,TOPMARGIN * dy,szTitle,lstrlen(szTitle));
        yExtSoFar = (int)dy * (TOPMARGIN + 4);
       }

    /* Print out text until no more lines or user aborts */
    while ((iLine < nTotalLines) && !PdlgAbort) {
	if ((yExtSoFar + (BOTTOMMARGIN + 1) * dy) >= yExtPage)
         {
	    /* Reached the end of a page, print Page number */ 
	    if  ((((pd.Flags & PD_PAGENUMS) == 0) && 
                  ((pd.Flags & PD_SELECTION) == 0))   ||
                 (((pd.nFromPage == 0) || (PageNum >= pd.nFromPage)) && 
	          ((pd.nToPage == 0)   || (PageNum <= pd.nToPage))))
	       {
             	 SelectObject(pd.hDC,hFontPrintB);
           	 sprintf(sz,"Page %u",PageNum);
           	 TextOut(pd.hDC, ((nCharsPerLine / 2) * dx),
                             	 yExtSoFar + (2 * dy), sz, lstrlen(sz));

            	/* Tell device driver to eject page */ 
            	 if (PageNum > nTotalPages) nTotalPages = PageNum;
            	 sprintf(sz,"Now Printing Page %u of %u",PageNum,nTotalPages);
                 SetDlgItemText (hwndPDlg, IDD_PRINTSTATUS, (LPSTR)sz);
           	 nPrintError = EndPage(pd.hDC);
           	 if ((nPrintError < 0) || PdlgAbort) break;
           	 nPrintError = StartPage(pd.hDC);
                 if ((nPrintError < 0) || PdlgAbort) break;
           	}
      
	    yExtSoFar = dy * TOPMARGIN;
            PageNum++;
	 }

	/* Print the line and unlock the text handle */
        if (LinePtr->length != END_OF_BLOCK)
          {
            textptr = (char far *) LinePtr + sizeof(TypLine) + sizeof(TypText);
            LineLen = lstrlen(textptr);
            if (IsLineBlank(textptr)) inheader = FALSE;
            
            if   ((((pd.Flags & PD_PAGENUMS) == 0) && 
                   ((pd.Flags & PD_SELECTION) == 0))   ||
                  (((pd.nFromPage == 0) || (PageNum >= pd.nFromPage)) && 
	           ((pd.nToPage == 0)   || (PageNum <= pd.nToPage))))
	      {
           	 if (inheader)
             	  {
	       	    loc = memchr(textptr,':',MAXHEADERSIZE);
               	    if (loc)
                     {
                   	i =  loc-textptr+1;
                   	if (PrintHeaderP(textptr,i))
                     	 {
                     	   SelectObject(pd.hDC,hFontPrintB);
                     	   TextOut (pd.hDC, (LEFTMARGIN * dx), yExtSoFar, textptr, i);
                     	   SelectObject(pd.hDC,hFontPrint);
                     	   TextOut (pd.hDC, (LEFTMARGIN + MAXHEADERSIZE + 2) * dx,
                     	           yExtSoFar, textptr+i, LineLen - i);
                     	 }
                     }
               	     else
                 	{
                   	  SelectObject(pd.hDC,hFontPrint);
                   	  TextOut (pd.hDC,(LEFTMARGIN + MAXHEADERSIZE + 2) * dx,
                                  yExtSoFar, textptr, LineLen);
                 	}
                   }
            	 else
              	   {
	            if (isLineQuotation(textptr)) SelectObject(pd.hDC,hFontPrintI);
                     else
                   	SelectObject(pd.hDC,hFontPrint);
                   	
               	    TextOut (pd.hDC, (LEFTMARGIN * dx), yExtSoFar, textptr, LineLen);      	
              	   }
            }
            yExtSoFar += dy;
            NextLine(&BlockPtr,&LinePtr);
          }

	/* Move down the page */
	iLine++;
    }
                 
abortout:                                                  

    UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
    if (!PdlgAbort && (nPrintError >= 0))
      {
        /* Eject the last page. */
        if ((((pd.Flags & PD_PAGENUMS) == 0) && 
             ((pd.Flags & PD_SELECTION) == 0))   ||
            (((pd.nFromPage == 0) || (PageNum >= pd.nFromPage)) && 
	     ((pd.nToPage == 0)   || (PageNum <= pd.nToPage))))
	  {
            SelectObject(pd.hDC,hFontPrintB);
      	    sprintf(sz,"Page %u",PageNum);
      	    TextOut(pd.hDC, ((nCharsPerLine / 2) * dx),
                      	    yExtPage - (BOTTOMMARGIN * dy), sz, lstrlen(sz));
            nPrintError = EndPage(pd.hDC);
          }

      	/* Complete the document. */
      	if (!PdlgAbort && (nPrintError >= 0))
          nPrintError = EndDoc(pd.hDC);
      }

exitout:
   
      if (PdlgAbort)
      {
        if (pd.hDC) AbortDoc(pd.hDC);
        MessageBox(hwnd, "Print Request Canceled",
                         "Canceled",MB_OK | MB_ICONEXCLAMATION);
       } 
      else
      {
       if (nPrintError < 0)  ReportPrintError(nPrintError,hwnd);
       EnableWindow (hwnd, TRUE);                    /* ReEnable main procedure */
       if (hwndPDlg) DestroyWindow ((HWND) hwndPDlg);       /* Delete Cancel Dialog */ 
      }
      
    if (lpfnPrintDlgProc) FreeProcInstance((FARPROC) lpfnPrintDlgProc);
    if (lpfnAbortProc) FreeProcInstance((FARPROC) lpfnAbortProc); 
        
    return;
}
@


1.21
log
@Cleaned up word wrapping in error messages
@
text
@d39 1
a39 1
 * $Id: wvprint.c 1.20 1994/08/03 00:35:20 dumoulin Exp dumoulin $
d41 3
d108 2
a109 4

#define STRICT
#include "windows.h"
#include <windowsx.h>	// for GlobalFreePtr
d112 1
@


1.20
log
@Fixed bug upon exit and startup for folks that don't have
any printers.
@
text
@d39 1
a39 1
 * $Id: wvprint.c 1.19 1994/08/02 23:30:11 dumoulin Exp dumoulin $
d41 4
d131 1
a131 1
        MessageBox(NULL, "Your Windows Print Driver CallBack procedure  "
d238 2
a239 2
        MessageBox(hwnd, "WinVN was unable to either display a "
                         "Printer Device Context Dialog Box or "
d364 6
a369 6
              MessageBox(hWnd, "The Windows Printer Device driver cannot "
                               "begin printing your document.  Your printer "
                               "may be off line or out of paper.  It is also "
                               "possible that the Windows Print Manager isn't "
                               "loaded and another program is currently "
                               "printing",
d375 2
a376 2
              MessageBox(hWnd, "Your Print request has been canceled via an "
                               "Abort request from your application",
d382 3
a384 2
              MessageBox(hWnd, "Your Print request has been canceled via a User"
                               "cancel request from the Windows Print Manager",
d390 3
a392 3
              MessageBox(hWnd, "Your Print request has been aborted due to "
                               "insufficient disk space in your Windows "
                               "TEMP subdirectory",
d398 3
a400 3
              MessageBox(hWnd, "Your Print request has been aborted due to "
                               "insufficient Windows memory.  Close some "
                               "applications and try again",
d408 2
a409 2
              MessageBox(hWnd, "Your Print request has been aborted due to "
                               "the Windows Print function returning an error "
@


1.19
log
@Fixed problem with AbortProc that could cause Printing to not
work with some print drivers
@
text
@d39 1
a39 1
 * $Id: wvprint.c 1.18 1994/08/02 05:01:48 dumoulin Exp dumoulin $
d41 4
d202 1
a202 1
  DeletePrinterDC(pd.hDC);              
@


1.18
log
@More testing to debug WinVN printing problem with some drivers
@
text
@d39 1
a39 1
 * $Id: wvprint.c 1.17 1994/07/28 21:19:12 dumoulin Exp dumoulin $
d41 3
d98 1
d104 1
a104 1
BOOL PdlgAbort = FALSE;      	/* TRUE if the user has aborted the print job	 */
d108 1
a108 1
 *									    *
d112 1
d123 1
a123 1
        MessageBox(hwndPDlg, "Your Windows Print Driver CallBack procedure  "
d129 10
a138 6
    while (!PdlgAbort && PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
	if (!hwndPDlg || !IsDialogMessage (hwndPDlg, &msg)){
	    TranslateMessage (&msg);
	    DispatchMessage  (&msg);
	}
	
d140 2
a141 5
        else
          return TRUE;
          
     // return TRUE;     // teseting JD 7/27/94
    //return !PdlgAbort;
d306 1
a306 1
BOOL CALLBACK PrintDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
d311 17
a327 18
            return TRUE;  
            
        case WM_SETTEXT:
            return TRUE;
                 
	case WM_COMMAND:
	    /* abort printing if the cancel button gets hit */
	    PdlgAbort = TRUE;
            if (hwndPDlg) 
             {
     	        EnableWindow(GetParent (hwnd), TRUE);
     //           DestroyWindow (hwndPDlg);    /* Delete Cancel Dialog */            
     //           hwndPDlg = NULL;
             }
	    return TRUE;

	default:
	    return FALSE;
d329 1
d466 2
a467 1

d614 2
a615 2
    cError = PrinterSetup(hwnd,PD_RETURNDC | PD_USEDEVMODECOPIES);
 
a619 1
    EnableWindow (hwnd, FALSE);                                                
d622 1
a622 1
    lpfnPrintDlgProc = (DLGPROC) MakeProcInstance((FARPROC) PrintDlgProc,hInst);
d624 1
a624 2
    hwndPDlg = CreateDialog ((HINSTANCE) hInst,(LPCSTR) "PRINTDIALOG",hwnd, (DLGPROC) lpfnPrintDlgProc); 
   // hwndPDlg = CreateDialog (hInst,"PRINTDIALOG",hwnd,  PrintDlgProc);
d627 5
a631 7
    ShowWindow (hwndPDlg, SW_SHOW);
    UpdateWindow (hwndPDlg);

    /* Allow the app. to inform GDI of the Abort function to call */
    PdlgAbort = FALSE;
    lpfnAbortProc = (ABORTPROC) MakeProcInstance(AbortProc,hInst);
    if (!lpfnAbortProc) goto exitout;
d656 1
a656 1
   //    SetFocus(hwndPDlg);
d811 2
a812 2
  //  if (lpfnPrintDlgProc) FreeProcInstance((FARPROC) lpfnPrintDlgProc);
  //  if (lpfnAbortProc) FreeProcInstance((FARPROC) lpfnAbortProc); 
@


1.17
log
@removed print.h
@
text
@a28 3
 *		  PrintFile ()		   -  Prints the contents of the   *
 *					      edit control.		   *
 *                                                                         *
d39 1
a39 1
 * $Id: wvprint.c 1.16 1994/07/28 21:14:05 dumoulin Exp dumoulin $
d41 3
d114 1
a114 2
 // hdc;		/* Needed to prevent compiler warning message */
     
d300 1
a300 1
BOOL WINAPI PrintDlgProc(HWND hwnd, WORD msg, WORD wParam, LPARAM lParam)
d305 3
a307 1
	    PdlgAbort = FALSE;  
d309 1
a309 1

d316 2
a317 2
                DestroyWindow (hwndPDlg);    /* Delete Cancel Dialog */            
                hwndPDlg = NULL;
d368 2
a369 2
              MessageBox(hWnd, "Your Print request has been canceled via a "
                               "cancel request from Windows Print Manager",
d406 26
d449 2
a450 2
    FARPROC lpfnAbort;
    FARPROC lpfnPDlg;
d462 1
a462 1
    lpfnAbort = MakeProcInstance (AbortProc, hInst);
d465 1
a465 1
    lpfnPDlg = MakeProcInstance (PrintDlgProc, hInst);
d551 1
a551 1
    FreeProcInstance (lpfnPDlg);
d554 1
a554 1
    FreeProcInstance (lpfnAbort);
d562 1
a562 28
}


/****************************************************************************
 *									    *
 *  FUNCTION   : PrintHeaderP ()					    *
 *									    *
 *  PURPOSE    : Determines if this is a header we are printing		    *
 *									    *
 *        Entry:  str    substring to search                                *
 *                limit  limits the search to no more than num characters   *
 *                                                                          *
 *        Exit:   BOOL   TRUE if we are not excluding print this header     *
 *                       FALSE if we are explicited excluding header        *
 *									    *
 ****************************************************************************/

BOOL WINAPI PrintHeaderP(char *str,int limit)
  {
    if (CompareStringNoCase(str,"Relay-Version:",limit) == 0
        || CompareStringNoCase(str,"Path:",limit) == 0
        || CompareStringNoCase(str,"References:",limit) == 0
        || CompareStringNoCase(str,"Xref:",limit) == 0)
      return(FALSE);
    else
      return(TRUE);
  } 

d583 4
a586 2
    BOOL    inheader = TRUE;
    unsigned int LineLen,Offset, PageNum;
a588 1
    WORD    fError = TRUE;
d595 4
a598 5
    int      i,dy,dx,yExtPage,yExtSoFar,iLine,nPrintError,
             nCharsPerLine,nLinesPerPage,nTotalLines,
             nTotalPages;
    DEVNAMES * dv;        

d606 1
d612 2
a613 1
    InitPrintFonts();                                                 
d616 4
a619 2
    hwndPDlg = CreateDialog (hInst,"PRINTDIALOG",hwnd,
                               (DLGPROC) PrintDlgProc);
a621 1
    EnableWindow (hwnd, FALSE);
d627 3
a629 1
    if (SetAbortProc(pd.hDC,AbortProc) < 0)
d633 1
d640 1
a640 1
    SetDlgItemText (hwndPDlg, IDD_PRINTSUBJECT, (LPSTR)szTitle);
d643 1
a643 1
    SetDlgItemText (hwndPDlg, IDD_PRINTDEVICE, (LPSTR)sz);
d646 1
a646 1
    SetDlgItemText (hwndPDlg, IDD_PRINTSTATUS, (LPSTR)sz);
d650 1
a650 1
    if (!PdlgAbort && (nPrintError < 0))
d652 4
a655 4
        MessageBox(hwnd, "Unable to Initialize Document",
                         "Error",MB_OK | MB_ICONEXCLAMATION);
        goto exitout;
      }
d676 1
a676 1
    if (nPrintError <= 0) goto errout;
d705 2
a706 1
            	/* Tell device driver to eject page */  
d769 2
d773 1
a773 8

    if (PdlgAbort)
     {
        AbortDoc(pd.hDC);
        MessageBox(hwnd, "Print Request Canceled",
                         "Canceled",MB_OK | MB_ICONEXCLAMATION);
     }
    else if (nPrintError >= 0)
a792 3
errout:
    if (!PdlgAbort && (nPrintError < 0))  ReportPrintError(nPrintError,hwnd);

d794 17
a810 2
    if (!PdlgAbort)  EnableWindow (hwnd, TRUE);   /* ReEnable main procedure */
    if (hwndPDlg) DestroyWindow (hwndPDlg);    /* Delete Cancel Dialog */         
@


1.16
log
@yet another attempt at fixing printing
@
text
@d42 1
a42 1
 * $Id: wvprint.c 1.15 1994/07/26 22:17:28 dumoulin Exp dumoulin $
d44 3
a95 1
#include <print.h>
@


1.15
log
@Changed function prototype for printing function
@
text
@d26 1
a26 1
 *                ReportPrintError ()     -  Decodes err codes for calls  *
d42 1
a42 1
 * $Id: wvprint.c 1.14 1994/06/30 16:32:13 dumoulin Exp dumoulin $
d44 3
d98 1
a98 1
BOOL fAbort = FALSE;      	/* TRUE if the user has aborted the print job	 */
d108 1
a108 1
BOOL WINAPI AbortProc ( HDC hdc,WORD nCode)
d110 12
a121 4
  MSG msg;
  hdc;		/* Needed to prevent compiler warning message */
  nCode;	/* Needed to prevent compiler warning message */  

d123 1
a123 1
    while (!fAbort && PeekMessage (&msg, NULL, NULL, NULL, TRUE))
d128 7
a134 1
    return !fAbort;
d222 1
a222 1
        sprintf(mes,"Comm Dialog Box Extended Error %d (%d)", (DWORD) cError, (DWORD) pd.lStructSize);
d304 1
d309 1
a309 1
	    fAbort = TRUE;
d474 1
a474 1
    while ((iLine < nLinesEc) && !fAbort) {
d477 1
a477 1
	    if (Escape (hdc, NEWFRAME, 0, NULL, NULL) < 0 || fAbort)
d621 1
a621 1
    fAbort = FALSE;
d642 1
a642 1
    if (!fAbort && (nPrintError < 0))
d683 1
a683 1
    while ((iLine < nTotalLines) && !fAbort) {
d701 1
a701 1
           	 if ((nPrintError < 0) || fAbort) break;
d703 1
a703 1
                 if ((nPrintError < 0) || fAbort) break;
d763 1
a763 1
    if (fAbort)
d785 1
a785 1
      	if (!fAbort && (nPrintError >= 0))
d790 1
a790 1
    if (!fAbort && (nPrintError < 0))  ReportPrintError(nPrintError,hwnd);
d793 1
a793 1
    if (!fAbort)  EnableWindow (hwnd, TRUE);   /* ReEnable main procedure */
@


1.14
log
@Fix Printing Bug - Call stack problem with AbortProc Procedure
@
text
@d42 1
a42 1
 * $Id: wvprint.c 1.13 1994/06/23 23:11:30 dumoulin Exp dumoulin $
d44 3
d391 1
a391 1
VOID WINAPI PrintFile(HWND hwnd)
d557 1
a557 1
VOID WINAPI PrintArticle(HWND hwnd,TypDoc * Doc)
@


1.13
log
@Added support for new Print Dialog box and features
@
text
@d10 1
a10 1
 *  FUNCTIONS	: FreePrinterDC ()	   -  Frees all memory associated  *
d16 3
d42 1
a42 1
 * $Id: wvprint.c 1.12 1994/06/17 09:08:14 dumoulin Exp dumoulin $
d44 3
d92 64
a155 11
BOOL fAbort = FALSE;	/* TRUE if the user has aborted the print job	 */
HWND hwndPDlg = NULL;	/* Handle to the cancel print dialog		 */
PRINTDLG pd;            /* Global structure to hold printer defaults     */
BOOL pSetup = FALSE;    /* TRUE when printer structure is initialized    */
char * szTitle;         /* Global pointer to job title                   */
char szExtDeviceMode[] = "EXTDEVICEMODE";
 
 
int FAR PASCAL AbortProc (HDC,WORD);		 /* Prototype  */ 
 
       
d158 1
a158 1
 *  FUNCTION   : FreePrinterDC ()					    *
d164 1
a164 1
 *  RETURNS    : TRUE if successful, FALSE if nothing to free    	    *
d167 7
a173 25
BOOL FAR PASCAL FreePrinterDC(void)
 { 
   if (pd.hDC)
      {              				/* Free Printer Context */
       if (pd.hDevMode)  
         {
           GlobalFree(pd.hDevMode);
           pd.hDevMode = NULL;
         }
       if (pd.hDevNames) 
         {
           GlobalFree(pd.hDevNames);
           pd.hDevNames = NULL;
         }
       if (pd.hDC)
       	 {
       	   DeleteDC(pd.hDC);
       	   pd.hDC = NULL;
       	  }           
       	  
        pSetup = FALSE;
        return TRUE;
       }
   else
   	return FALSE;
d184 1
a184 1
 *  RETURNS    : Zero if successful, else Extended Error Code on Failure    *
d187 22
a208 13
DWORD FAR PASCAL PrinterSetup(HWND hwnd,DWORD flags)
 {
  /*  Setup a handle to a printer if one hasn't been setup before */ 
    if (!pSetup) 
      {
    	memset((void *) &pd,0,sizeof(PRINTDLG));
    	pd.lStructSize = sizeof(PRINTDLG);
  
    	pd.hInstance = NULL; 
   	pd.hDevMode = NULL;
    	pd.hDevNames = NULL;
    	pd.nMaxPage = 0xFFFE;
    	pSetup = TRUE;
d210 2
a211 6
   	                      
    pd.hwndOwner = hwnd;
    pd.Flags = hwnd ? flags : flags | PD_SHOWHELP;                                          
                                                 
    if (PrintDlg(&pd) == 0)
      return CommDlgExtendedError();
d223 1
a223 1
 *  RETURNS    : HDC - A handle to printer DC                   	    *
d226 3
a228 4
HDC FAR PASCAL GetPrinterDC(HWND hwnd)
 {
  char mes[60];
  DWORD  cError = 0;
d230 12
a241 11
  if (!pSetup || (pd.hDC == 0))
    cError = PrinterSetup(hwnd,PD_RETURNDC | PD_RETURNDEFAULT);
    if ((cError != 0) && (hwnd != 0))
      { 
        sprintf(mes,"Comm Dialog Box Extended Error %d", (DWORD) cError);
        MessageBox(hwnd, "WinVN was unable to either display a "
                         "Printer Device Context Dialog Box or "
                         "to get a Printer Device Context"
                         ,mes,MB_OK | MB_ICONEXCLAMATION);
       }
  return pd.hDC;
d243 1
a243 2
 
 
d246 4
a249 1
 *  FUNCTION   : AbortProc()						    *
d251 1
a251 1
 *  PURPOSE    : To be called by GDI print code to check for user abort.    *
d254 13
a266 5
BOOL CALLBACK AbortProc ( HDC hdc,WORD nCode)
{
  MSG msg;
  hdc;		/* Needed to prevent compiler warning message */
  nCode;	/* Needed to prevent compiler warning message */  
a267 8
    /* Allow other apps to run, or get abort messages */
    while (!fAbort && PeekMessage (&msg, NULL, NULL, NULL, TRUE))
	if (!hwndPDlg || !IsDialogMessage (hwndPDlg, &msg)){
	    TranslateMessage (&msg);
	    DispatchMessage  (&msg);
	}
    return !fAbort;
}
d279 2
a280 7
BOOL FAR PASCAL PrintDlgProc(HWND hwnd, WORD msg, WORD wParam, LONG lParam)
 {               
 
    char szDevice[160];    /* contains the device driver info string */
    char * szDriver;       /* Pointer to the Driver Name */
    char * szPort;           /* contains the port of the default printer */
                    
a283 10
	    /* Set up information in dialog box */
    //	    CreateIC(szDriver, szDevice,szPort, &pd.hDevMode);   
            GetProfileString("windows", "device", "", szDevice, sizeof(szDevice));
            for (szDriver = szDevice; *szDriver && *szDriver != ','; szDriver++);
    //        if (*szDriver) *szDriver++ = 0;
            for (szPort = szDriver; *szPort && *szPort != ','; szPort++);
            if (*szPort) *szPort++ = 0;
   	    SetDlgItemText (hwnd, IDD_PRINTDEVICE, (LPSTR)szDevice);
    //	    SetDlgItemText (hwnd, IDD_PRINTPORT, (LPSTR)szPort);
	    SetDlgItemText (hwnd, IDD_PRINTSUBJECT, (LPSTR)szTitle);
d313 1
a313 1
void ReportPrintError(int nError,HWND hWnd)
d388 1
a388 1
VOID FAR PASCAL PrintFile(HWND hwnd)
a407 9

    /* Create the job title by loading the title string from STRINGTABLE */
  /* 
    cch = LoadString (hInst, IDS_PRINTJOB, sz, sizeof(sz)); 
    szTitle = sz + cch;
    cch += GetWindowText (hwnd, sz + cch, 32 - cch);
    sz[31] = 0;
  */

a510 2
/*	PPError (hwnd, MB_OK | MB_ICONEXCLAMATION, IDS_PRINTERROR, (LPSTR)szTitle);  */

d529 1
a529 3
  BOOL PrintHeaderP(str,limit)
    char *str;
    int  limit;
a549 1
// #define RIGHTMARGIN 10   /* Right Margin in characters on printed page */ 
d552 1
a552 1
#define MAXHEADERSIZE 20
d554 1
a554 1
VOID FAR PASCAL PrintArticle(HWND hwnd,TypDoc * Doc)
d556 2
a557 1
    char    sz[64],sbuf[MAXHEADERLINE];
d574 1
d576 3
a578 5
    /* Create the job title */ 
 
    strcpy(sz,"WinVN Print");
    di.cbSize = sizeof(DOCINFO);
    di.lpszDocName = "WinVn: Printing";
a580 5
    found = GetHeaderLine(Doc,"Subject:",sbuf,sizeof(sbuf));
    if (!found)
       lstrcpy(sbuf,"Subject: No Subject");
    szTitle = sbuf;

a581 1
    cError = PrinterSetup(hwnd,PD_RETURNDC);
d583 3
d587 3
a589 5
    if (!pd.hDC)
       {
	goto exitout;
       }

a590 1

d608 8
a615 1
    /* Initialize the document */                     
d618 2
d643 2
a644 1
    nTotalPages = (nTotalLines + nLinesPerPage -1) / nLinesPerPage;
d646 2
a647 1
    StartPage(pd.hDC);
d650 1
a650 1
    if (((pd.Flags & PD_ALLPAGES) != 0) ||
d656 2
a657 2
        SelectObject(pd.hDC,hFontPrintS);         /* Select Printer Font */
        TextOut (pd.hDC,LEFTMARGIN * dx,TOPMARGIN * dy,sbuf,lstrlen(sbuf));
d666 4
a669 3
	    if (((pd.Flags & PD_ALLPAGES) != 0) ||
	        (((pd.nFromPage == 0) || (PageNum >= pd.nFromPage))  && 
	        ((pd.nToPage == 0)   || (PageNum <= pd.nToPage))))
d696 4
a699 3
            if (((pd.Flags & PD_ALLPAGES) != 0) ||
                (((pd.nFromPage == 0) || (PageNum >= pd.nFromPage))  && 
	         ((pd.nToPage == 0)   || (PageNum <= pd.nToPage))))
d751 4
a754 3
        if (((pd.Flags & PD_ALLPAGES) != 0) ||
            (((pd.nFromPage == 0) || (PageNum >= pd.nFromPage))  && 
	    ((pd.nToPage == 0)   || (PageNum <= pd.nToPage))))
d768 2
a769 4
     if (!fAbort && (nPrintError < 0))
      {
       ReportPrintError(nPrintError,hwnd);
      }
d772 1
a772 1
    if (!fAbort) EnableWindow (hwnd, TRUE);    /* ReEnable main procedure */
@


1.12
log
@Attempts to fix the Unable to Initialize Document printing bug
@
text
@d39 1
a39 1
 * $Id: wvprint.c 1.11 1994/06/14 20:22:37 dumoulin Exp dumoulin $
d41 3
d80 1
a80 1
#include "windows.h"                                         
a81 1
#include <commdlg.h>
d146 1
a146 1
DWORD FAR PASCAL PrinterSetup(HWND hwnd)
d157 1
a159 2
    
    /*  If hwnd is NULL, use defaults and don't popup a Dialog Box */
d162 2
a163 2
    pd.Flags = hwnd ? PD_RETURNDC : PD_RETURNDEFAULT;
    
d182 13
a194 1
  if (!pSetup) PrinterSetup(hwnd);
d235 1
a235 1
    char * szDriver;        /* Pointer to the Driver Name */
d245 1
a245 1
            if (*szDriver) *szDriver++ = 0;
d249 2
a250 2
	    SetDlgItemText (hwnd, IDD_PRINTPORT, (LPSTR)szPort);
	    SetDlgItemText (hwnd, IDD_PRINTTITLE, (LPSTR)szTitle);
d541 1
a541 1
    unsigned int LineLen,Offset;
d553 1
a553 2
             nTotalPages,PageNum;
        
d568 1
a568 1
    cError = PrinterSetup(hwnd);
d595 3
a597 1
    /* Initialize the document */
a622 4

    SelectObject(pd.hDC,hFontPrintS);         /* Select Printer Font */
    TextOut (pd.hDC,LEFTMARGIN * dx,TOPMARGIN * dy,sbuf,lstrlen(sbuf));
    yExtSoFar = (int)dy * (TOPMARGIN + 4);
d624 12
d641 19
a659 9
	    /* Reached the end of a page, print Page number */
            SelectObject(pd.hDC,hFontPrintB);
            sprintf(sz,"Page %u",PageNum);
            TextOut(pd.hDC, ((nCharsPerLine / 2) * dx),
                              yExtSoFar + (2 * dy), sz, lstrlen(sz));

            /* Tell device driver to eject page */
            nPrintError = EndPage(pd.hDC);
            if ((nPrintError < 0) || fAbort) break;
a661 2
            nPrintError = StartPage(pd.hDC);
            if ((nPrintError < 0) || fAbort) break;
d670 9
a678 8

            if (inheader)
             {
	       loc = memchr(textptr,':',MAXHEADERSIZE);
               if (loc)
                 {
                   i =  loc-textptr+1;
                   if (PrintHeaderP(textptr,i))
d680 9
a688 6
                     	SelectObject(pd.hDC,hFontPrintB);
                     	TextOut (pd.hDC, (LEFTMARGIN * dx), yExtSoFar, textptr, i);
                     	SelectObject(pd.hDC,hFontPrint);
                     	TextOut (pd.hDC, (LEFTMARGIN + MAXHEADERSIZE + 2) * dx,
                     	         yExtSoFar, textptr+i, LineLen - i);
                        yExtSoFar += dy;
d690 17
a706 19
                 }
               else
                 {
                   SelectObject(pd.hDC,hFontPrint);
                   TextOut (pd.hDC,(LEFTMARGIN + MAXHEADERSIZE + 2) * dx,
                             yExtSoFar, textptr, LineLen);
                   yExtSoFar += dy;
                 }
             }
            else
              {
	       if (isLineQuotation(textptr)) SelectObject(pd.hDC,hFontPrintI);
                else
                   SelectObject(pd.hDC,hFontPrint);

               TextOut (pd.hDC, (LEFTMARGIN * dx), yExtSoFar, textptr, LineLen);
               yExtSoFar += dy;
              }

d725 10
a734 5
        SelectObject(pd.hDC,hFontPrintB);
      	sprintf(sz,"Page %u",PageNum);
      	TextOut(pd.hDC, ((nCharsPerLine / 2) * dx),
                      yExtPage - (BOTTOMMARGIN * dy), sz, lstrlen(sz));
        nPrintError = EndPage(pd.hDC);
d748 1
a748 2
    if (hwndPDlg) DestroyWindow (hwndPDlg);    /* Delete Cancel Dialog */
 //   FreePrinterDC();              
@


1.11
log
@Fix various printer problems due to short instead of int variable
@
text
@d10 4
a13 1
 *  FUNCTIONS	: GetPrinterDC ()	   -  Creates a printer DC for the *
d17 1
d39 1
a39 1
 * $Id: wvprint.c 1.10 1994/05/19 02:06:10 dumoulin Exp dumoulin $
d41 3
d75 1
a75 1
 */
d77 3
a79 1
#include "windows.h"
d86 3
a88 10
char szDevice[160];	/* Contains the device, the driver, and the port */
char * szDriver;	/* Pointer to the driver name			 */
char * szPort;		/* Port, ie, LPT1				 */
char * szTitle;		/* Global pointer to job title			 */
int iPrinter = 0;	/* level of available printer support.		 */
			/* 0 - no printer available			 */
			/* 1 - printer available			 */
			/* 2 - driver supports 3.0 device initialization */
HANDLE hInitData=NULL;	/* handle to initialization data		 */

d90 2
a91 1

d93 40
a132 1

d135 1
a135 1
 *  FUNCTION   : GetPrinterDC ()					    *
d141 34
a174 1
 *  RETURNS    : HDC   - A handle to printer DC.			    *
d176 2
d179 7
a185 68
HDC FAR PASCAL GetPrinterDC(void)
{
    HDC      hdc;
    LPSTR    lpdevmode = NULL;

    iPrinter = 0;

    /* Get the printer information from win.ini into a buffer and
     * null terminate it.
     */
    GetProfileString ( "windows", "device", "" ,szDevice, sizeof(szDevice));
    for (szDriver = szDevice; *szDriver && *szDriver != ','; szDriver++)
	;
    if (*szDriver)
	*szDriver++ = 0;

    /* From the current position in the buffer, null teminate the
     * list of ports
     */
    for (szPort = szDriver; *szPort && *szPort != ','; szPort++)
	;
    if (*szPort)
	*szPort++ = 0;

    /* if the device, driver and port buffers all contain meaningful data,
     * proceed.
     */
    if (!*szDevice || !*szDriver || !*szPort){
	*szDevice = 0;
	return NULL;
    }

    /* Create the printer display context */
    if (hInitData){
	/* Get a pointer to the initialization data */
	lpdevmode = (LPSTR) LocalLock (hInitData);

	if (lstrcmp (szDevice, lpdevmode)){
	    /* User has changed the device... cancel this setup, as it is
	     * invalid (although if we worked harder we could retain some
	     * of it).
	     */
	    lpdevmode = NULL;
	    LocalUnlock (hInitData);
	    LocalFree (hInitData);
	    hInitData = NULL;
	}
    }
    hdc = CreateDC (szDriver, szDevice, szPort, lpdevmode);

    /* Unlock initialization data */
    if (hInitData)
	LocalUnlock (hInitData);

    if (!hdc)
	return NULL;


    iPrinter = 1;

    /* Find out if ExtDeviceMode() is supported and set flag appropriately */
    if (GetProcAddress (GetModuleHandle (szDriver), szExtDeviceMode))
	iPrinter = 2;

    return hdc;

}

d219 6
a224 1
 {
d226 1
a226 1
      {
d229 7
a235 1
	    SetDlgItemText (hwnd, IDD_PRINTDEVICE, (LPSTR)szDevice);
d267 1
a267 1
void ReportPrintError(short nError,HWND hWnd)
d322 1
a322 1
              sprintf(mes,"Unknown Print Error %d", (short) nError);
d383 1
a383 1
    hdc = GetPrinterDC();
d523 2
a524 1
{
d526 2
a527 1
    HDC     hdcPrn = NULL;
a528 1
    char    sz[64],sbuf[MAXHEADERLINE];
a530 2
    BOOL    found;
    BOOL    inheader = TRUE;
d532 1
d538 2
a539 2
    int      i,dy,dx,yExtPage,yExtSoFar,iLine,nPrintError;
    short    nCharsPerLine,nLinesPerPage,nTotalLines,
d541 1
d543 2
a544 1
    /* Create the job title */
d547 1
a547 1
    di.lpszDocName = "WinVn Article";
d555 2
a556 1
    /* Initialize the printer */
d558 3
a560 2
    hdcPrn = GetPrinterDC();
    if (!hdcPrn)
d562 1
d566 1
a566 1
    hwndPDlg = CreateDialog (hInst, "PRINTDIALOG",hwnd,
d576 1
a576 1
    if (SetAbortProc(hdcPrn,AbortProc) < 0)
d584 2
a585 2
    nPrintError = StartDoc(hdcPrn,&di);
    if ((fAbort == FALSE) && (nPrintError < 0))
d601 2
a602 2
    SelectObject(hdcPrn,hFontPrint);         /* Select Printer Font */
    GetTextMetrics(hdcPrn,&tm);
d605 2
a606 2
    nCharsPerLine = GetDeviceCaps (hdcPrn,HORZRES) / dx;
    nLinesPerPage = GetDeviceCaps (hdcPrn,VERTRES) / dy;
d608 1
a608 1
    yExtPage = GetDeviceCaps (hdcPrn, VERTRES);
d610 2
a611 2
    SelectObject(hdcPrn,hFontPrintS);         /* Select Printer Font */
    TextOut (hdcPrn,LEFTMARGIN * dx,TOPMARGIN * dy,sbuf,lstrlen(sbuf));
d613 1
a613 1
    StartPage(hdcPrn);
d620 1
a620 1
            SelectObject(hdcPrn,hFontPrintB);
d622 1
a622 1
            TextOut(hdcPrn, ((nCharsPerLine / 2) * dx),
d626 1
a626 1
            nPrintError = EndPage(hdcPrn);
d630 1
a630 1
            nPrintError = StartPage(hdcPrn);
d649 4
a652 4
                     	SelectObject(hdcPrn,hFontPrintB);
                     	TextOut (hdcPrn, (LEFTMARGIN * dx), yExtSoFar, textptr, i);
                     	SelectObject(hdcPrn,hFontPrint);
                     	TextOut (hdcPrn, (LEFTMARGIN + MAXHEADERSIZE + 2) * dx,
d659 2
a660 2
                   SelectObject(hdcPrn,hFontPrint);
                   TextOut (hdcPrn,(LEFTMARGIN + MAXHEADERSIZE + 2) * dx,
d667 1
a667 1
	       if (isLineQuotation(textptr)) SelectObject(hdcPrn,hFontPrintI);
d669 1
a669 1
                   SelectObject(hdcPrn,hFontPrint);
d671 1
a671 1
               TextOut (hdcPrn, (LEFTMARGIN * dx), yExtSoFar, textptr, LineLen);
d686 1
a686 1
        AbortDoc(hdcPrn);
d693 1
a693 1
        SelectObject(hdcPrn,hFontPrintB);
d695 1
a695 1
      	TextOut(hdcPrn, ((nCharsPerLine / 2) * dx),
d697 1
a697 1
        nPrintError = EndPage(hdcPrn);
d700 2
a701 2
      	if (nPrintError >= 0)
          nPrintError = EndDoc(hdcPrn);
d706 1
a706 1
        ReportPrintError(nPrintError,hwnd);
d712 1
a712 1
    if (hdcPrn) DeleteDC(hdcPrn);              /* Free Printer Context */
@


1.10
log
@*** empty log message ***
@
text
@d35 1
a35 1
 * $Id: wvprint.c 1.9 1994/01/15 20:41:22 jcoop Exp dumoulin $
d37 3
d471 4
a474 4
    if (CompareStringNoCase(str,"Relay-Version",limit) == 0
        || CompareStringNoCase(str,"Path",limit) == 0
        || CompareStringNoCase(str,"References",limit) == 0
        || CompareStringNoCase(str,"Xref",limit) == 0)
d511 1
a511 1
    int      i,dy,dx,yExtPage,yExtSoFar,iLine;
d513 1
a513 1
             nTotalPages,PageNum,nPrintError;
d553 1
a553 1
    if (!fAbort && (nPrintError < 0))
@


1.9
log
@Use GetEditText instead of EM_GETHANDLE with global editWnds
@
text
@d35 1
a35 1
 * $Id: wvprint.c 1.8 1993/12/08 01:28:38 rushing Exp $
d37 3
a322 1
//    HANDLE  hT;
d471 1
a471 1
        || CompareStringNoCase(str,"Relay-Version",limit) == 0)
@


1.8
log
@new version box and cr lf consistency
@
text
@d35 1
a35 1
 * $Id: wvprint.c 1.7 1993/08/09 17:57:47 dumoulin Exp rushing $
d37 3
d65 1
d319 2
a320 1
    HANDLE  hT;
d380 3
a382 2
    hT	      = (HANDLE)SendMessage (hwndEdit, EM_GETHANDLE, 0, 0L);

d396 2
a397 1
	pch = (char *)LocalLock(hT) + ich;
d401 1
a401 1
	LocalUnlock (hT);
d407 2
a408 1

@


1.7
log
@Fix to AbortProc and PrintArticle to fix GPFs
@
text
@d32 1
d35 1
a35 1
 * $Id: wvprint.c 1.6 1993/08/05 15:46:42 dumoulin Exp dumoulin $
d37 3
a62 1
#include "wvdlg.h"
@


1.6
log
@Rewrote PrintArticle to solve print crashes
@
text
@d34 1
a34 1
 * $Id: wvprint.c 1.5 1993/07/06 21:09:09 cnolan Exp $
d36 3
d62 2
a63 2
BOOL fAbort;		/* TRUE if the user has aborted the print job	 */
HWND hwndPDlg;		/* Handle to the cancel print dialog		 */
d203 6
a314 1
    HWND    hwndPDlg;
a482 1
    FARPROC lpfnPDlg = NULL;
a488 1
    HWND    hwndPDlg;
a509 6
    /* Make instances of the Print dialog function */

    lpfnPDlg = MakeProcInstance (PrintDlgProc, hInst);
    if (!lpfnPDlg)
	goto exitout;

d518 2
a519 1
    hwndPDlg = CreateDialog (hInst, "PRINTDIALOG", hwnd, lpfnPDlg);
a529 2
        EnableWindow (hwnd, TRUE);
        DestroyWindow (hwndPDlg);
a538 2
        EnableWindow (hwnd, TRUE);
        DestroyWindow (hwndPDlg);
a635 4
    /* Get rid of Cancel Print Dialog Box */
    EnableWindow (hwnd, TRUE);
    DestroyWindow (hwndPDlg);

a660 1

d662 3
a664 2
    if (hdcPrn) DeleteDC(hdcPrn);                /* Free Printer Context */
    if (lpfnPDlg) FreeProcInstance (lpfnPDlg);   /* Free Dialog box proc */
@


1.5
log
@win32 support
@
text
@a6 2
 *                  Based on examples from Petzold's Programming Windows   *
 *                  book & code from the SDK Samples Directory             *
d18 3
d25 3
d34 1
a34 1
 * $Id: wvprint.c 1.4 1993/05/25 00:00:55 rushing Exp $
d36 3
d161 1
a161 3
int FAR PASCAL AbortProc ( hdc, reserved )
HDC hdc;
WORD reserved;
d163 3
a165 1
    MSG msg;
a166 3
	 hdc;		/* Needed to prevent compiler warning message */
	 reserved;	/* Needed to prevent compiler warning message */

d187 3
a189 5
{
 /*	 wParam;  */	/* Needed to prevent compiler warning message */
 /*	 lParam;  */	/* Needed to prevent compiler warning message */

    switch (msg){
d195 1
a195 1
	    break;
d198 1
a198 1
	    /* abort printing if the only button gets hit */
d200 1
a200 1
	    break;
d204 80
a283 3
    }
    return TRUE;
}
d356 2
a357 2
    GetTextExtentPoint (hdc, "CC", 2, &size);              
    dy=size.cy;                                            
d367 1
a367 1
    while (iLine < nLinesEc){
a384 4
	/* Test and see if the Abort flag has been set. If yes, exit. */
	if (fAbort)
	    goto getout2;

d461 1
a461 1
 *  PURPOSE    : Prints the contents of the edit control.		    *
d473 5
a477 5
    HDC     hdcPrn;
    int     yExtPage,yExtSoFar,iLine;
    unsigned int LineLen;
    char    sz[32];
    char    sbuf[MAXHEADERLINE];
a481 1
    FARPROC lpfnAbort,lpfnPDlg;
a482 1
    int     i,dy,dx;
a485 1
    unsigned int Offset;
d489 1
d491 1
a491 1
             nTotalPages,PageNum;
d494 5
a498 1
    lstrcpy(sz,"WinVN Print Request");
d504 2
a505 4
    /* Make instances of the Abort proc. and the Print dialog function */
    lpfnAbort = MakeProcInstance (AbortProc, hInst);
    if (!lpfnAbort)
	goto getout;
d508 1
a508 1
	goto getout4;
d511 1
d514 6
a519 1
	goto getout5;
a520 1
    /* Disable the main application window and create the Cancel dialog */
a521 3
    hwndPDlg = CreateDialog (hInst, "PRINTDIALOG", hwnd, lpfnPDlg);
    if (!hwndPDlg)
	goto getout3;
d525 10
a534 3
    /* Allow the app. to inform GDI of the escape function to call */
    if (Escape (hdcPrn, SETABORTPROC, 0, (LPSTR)lpfnAbort, NULL) < 0)
	goto getout1;
d537 9
a545 2
    if (Escape (hdcPrn, STARTDOC, lstrlen(sz), sz, NULL) < 0)
	goto getout1;
d550 1
a550 1
  /* Get the lines in document and and a handle to the text buffer */
d568 1
a568 4

    /* While more lines print out the text */
    while (iLine < nTotalLines){
	if ((yExtSoFar + (BOTTOMMARGIN + 1) * dy) >= yExtPage){
d570 4
d581 2
a582 2
            if (Escape (hdcPrn, NEWFRAME, 0, NULL, NULL) < 0 || fAbort)
		goto getout2;
d585 3
a587 1
	}
a593 1
     /*       if (LineLen == 0) inheader = FALSE; */
a632 4
	/* Test and see if the Abort flag has been set. If yes, exit. */
	if (fAbort)
	    goto getout2;

a636 11
    /* Eject the last page. */

    SelectObject(hdcPrn,hFontPrintB);
    sprintf(sz,"Page %u",PageNum);
    TextOut(hdcPrn, ((nCharsPerLine / 2) * dx),
                      yExtPage - (BOTTOMMARGIN * dy), sz, lstrlen(sz));

    if (Escape (hdcPrn, NEWFRAME, 0, NULL, NULL) < 0)
	goto getout2;

    /* Complete the document. */
d639 1
a639 10
    if (Escape (hdcPrn, ENDDOC, 0, NULL, NULL) < 0){
getout2:
	/* Ran into a problem before NEWFRAME? Abort the document */
	Escape( hdcPrn, ABORTDOC, 0, NULL, NULL);
    }
    else
	fError=FALSE;

getout3:
    /* Close the cancel dialog and re-enable main app. window */
d643 14
a656 15
getout1:
    DeleteDC(hdcPrn);

getout5:
    /* Get rid of dialog procedure instances */
    FreeProcInstance (lpfnPDlg);

getout4:
    FreeProcInstance (lpfnAbort);

getout:

    /* Error? make sure the user knows... */
    if (fError)
        MessageBox(hwnd, "Printing Error","Error",MB_OK);
d658 14
a673 3


 
@


1.4
log
@rot13 merge (MRB)
@
text
@d30 1
a30 1
 * $Id: wvprint.c 1.3 1993/03/09 01:24:21 dumoulin Exp $
d32 3
d232 1
d277 2
a278 1
    dy = HIWORD (GetTextExtent (hdc, "CC", 2));
d300 1
a300 1
	pch = LocalLock(hT) + ich;
d594 1
a594 1
 @


1.3
log
@Added italics for quoted regions in prints
@
text
@d30 5
a34 2
 * $Id: wvprint.c%v 1.2 1993/02/16 22:52:25 dumoulin Exp rushing $
 * $Log: wvprint.c%v $
d500 1
a500 1
               loc = memchr(textptr,':',MAXHEADERSIZE);
d524 1
a524 3
               loc = memchr(textptr,'>',2);
               if (!loc) loc = memchr(textptr,'|',2);
               if (loc) SelectObject(hdcPrn,hFontPrintI);
@


1.2
log
@moved rcs ident comment below file description
@
text
@d30 1
a30 1
 * $Id: wvprint.c%v 1.1 1993/02/16 20:54:22 rushing Exp dumoulin $
d32 3
d348 3
a350 1
 *  FUNCTION   : FindChar ()        					    *
d352 1
a352 4
 *  PURPOSE    : Finds the first occurance of a character in a buffer       *
 *                                                                          *
 *        Entry:  buf    points to a buffer                                 *
 *                ch     is the comparison character                        *
d355 2
a356 2
 *        Exit:   int    points to the location of the character if found   *
 *                       NULL if character is not found                     *
d360 2
a361 3
  int FindChar(buf,ch,limit)
    char far *buf;
    char ch;
d364 8
a371 11
    int  loc = 0;
    while (limit-- && *buf && (*buf != ch))
      {
        buf++;
        loc++;
      }
    if (limit == 0)
       return(NULL);
     else
       return(loc);
  }
d396 1
d492 2
a493 1
            if (LineLen == 0) inheader = FALSE;
d497 2
a498 2
               i = FindChar(textptr,':',MAXHEADERSIZE);
               if (i)
d500 10
a509 5
                   SelectObject(hdcPrn,hFontPrintB);
                   TextOut (hdcPrn, (LEFTMARGIN * dx), yExtSoFar, textptr, i+1);
                   SelectObject(hdcPrn,hFontPrint);
                   TextOut (hdcPrn, (LEFTMARGIN + MAXHEADERSIZE + 2) * dx,
                            yExtSoFar, textptr+i+1, LineLen - i - 1);
d516 1
d521 6
a526 1
               SelectObject(hdcPrn,hFontPrint);
d528 1
a538 1
	yExtSoFar += dy;
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

d27 10
@
