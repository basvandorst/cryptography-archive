/***************************************************************************
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
 * $Id: wvprint.c 1.22 1994/09/16 01:06:33 jcooper Exp $
 * $Log: wvprint.c $
 * Revision 1.22  1994/09/16  01:06:33  jcooper
 * rearranged headers to allow use of precompiled headers
 *
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
