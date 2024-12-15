/*
 *
 * $Id: winvn.c 1.50 1994/08/11 02:25:59 dumoulin Exp $
 * $Log: winvn.c $
 * Revision 1.50  1994/08/11  02:25:59  dumoulin
 * enabled STRICT compile and cleaned up parameter casting
 *
 * Revision 1.49  1994/08/11  00:09:17  jcooper
 * Enhancements to Mime and article encoding/encoding
 *
 * Revision 1.48  1994/08/04  05:51:54  dumoulin
 * Fixed problem with long mail addresses getting truncated to 32 chars
 *
 * Revision 1.47  1994/08/03  00:35:20  dumoulin
 * Fixed bug upon exit and startup for folks that don't have
 * any printers.
 *
 * Revision 1.46  1994/08/02  05:00:16  dumoulin
 * change HANDLE to HINSTANCE for better NT compatiblity
 *
 * Revision 1.45  1994/07/25  22:41:22  pearse_w_r
 * ShowUnreadOnly option
 *
 * Revision 1.44  1994/07/25  18:51:48  jcooper
 * execution of decoded files
 *
 * Revision 1.43  1994/06/30  16:33:35  dumoulin
 * Fix a problem with freeing printing fonts with an uninitialized variable
 *
 * Revision 1.42  1994/06/23  23:10:40  dumoulin
 * Changed code to not delete hardware device context for Printer device
 *
 * Revision 1.41  1994/06/17  09:08:49  dumoulin
 * Attempts to fix the Unable to Initialize Document printing bug
 *
 * Revision 1.40  1994/06/11  00:46:47  dumoulin
 * Fix defaults for GenSock.DLL and prompt if no username specified
 *
 * Revision 1.39  1994/06/06  22:06:13  gardnerd
 * horizontal scrolling support
 *
 * Revision 1.38  1994/05/27  01:29:29  rushing
 * unnecessary winundoc.h
 *
 * Revision 1.37  1994/05/26  22:02:38  jglasser
 * warnings
 *
 * Revision 1.36  1994/05/23  19:51:51  rushing
 * NNTPPort becomes NNTPService, and it's a string.
 *
 * Revision 1.35  1994/05/23  18:37:00  jcooper
 * new attach code, session [dis]connect
 *
 * Revision 1.34  1994/05/02  19:51:12  rushing
 * more changes from jg
 *
 * Revision 1.33  1994/05/02  19:45:42  rushing
 * redoing JD's print font size change
 *
 * Revision 1.32  1994/05/02  19:44:04  rushing
 * changes from jody glasser
 *
 * Revision 1.31  1994/04/22  05:02:23  dumoulin
 * Fix font size problem
 *
 * Revision 1.30  1994/04/22  04:55:02  dumoulin
 * Fix font conversion problem when converting older winvn.ini files
 *
 * Revision 1.29  1994/04/19  10:18:52  dumoulin
 * Updated converter for old to new INI files
 *
 * Revision 1.28  1994/04/18  20:49:44  rushing
 * check MailList for NULL on write of newsrc
 *
 * Revision 1.27  1994/03/01  19:10:19  rushing
 * ThreadFullSubject option added
 *
 * Revision 1.26  1994/02/24  21:24:47  jcoop
 * jcoop changes
 *
 * Revision 1.25  1994/02/16  21:58:56  rushing
 * call to profilestring was a little hosed for .sig file
 *
 * Revision 1.24  1994/02/16  20:59:17  rushing
 * Added .ini param to force use of XHDR over XOVER
 *
 * Revision 1.23  1994/02/09  01:25:15  cnolan
 * cnolan 90.2 changes
 *
 * Revision 1.22  1994/02/09  01:10:07  rushing
 * PREF->COMM for GenSockDLL profile string
 *
 * Revision 1.21  1994/02/04  02:18:13  rushing
 * added GenSockDLL .ini parameter
 *
 * Revision 1.20  1994/01/24  19:34:38  jcoop
 * per email
 *
 * Revision 1.19  1994/01/24  17:39:47  jcoop
 * 90.2 changes
 *
 * Revision 1.18  1994/01/17  21:56:15  jcoop
 * New font/color and INI file routines.  Sig file and en/decoding inits
 *
 * Revision 1.17  1994/01/12  19:25:35  mrr
 * mrr mods 4
 * 
 * Revision 1.16  1993/12/08  01:27:21  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.15  1993/08/25  18:55:19  mbretherton
 * MRB merge, mail & post logging
 *
 *
 * Revision 1.1x  1993/07/21 MBretherton 
 *                Include Posting and Mail log variables
 *
 * Revision 1.14  1993/08/05  20:06:07  jcoop
 * save multiple articles changes by jcoop@oakb2s01.apl.com (John S Cooper)
 *
 * Revision 1.13  1993/07/13  16:03:04  riordan
 * MRR mods
 *
 * Revision 1.12  1993/07/06  21:09:09  cnolan
 * win32 support
 *
 * Revision 1.11  1993/06/25  20:14:09  dumoulin
 * Cleaned up Compiler Warnings
 *
 * Revision 1.10  1993/06/24  17:13:14  riordan
 * Save window positions between sessions.
 *
 * Revision 1.9  1993/06/11  00:12:13  rushing
 * second merge from Matt Bretherton sources.
 *
 *
 * Revision 1.8  1993/05/28  17:18:00  rushing
 * Added Misc... option to enable threading
 *
 * Revision 1.7  1993/05/24  23:27:38  rushing
 * Create an Article Italics Font for Quotation lines (MRB)
 *
 * Revision 1.6  1993/05/18  22:10:45  rushing
 * smtp support
 *
 * Revision 1.5  1993/05/13  19:58:35  rushing
 * fancy 'from' in group window
 *
 * Revision 1.4  1993/05/13  16:18:00  rushing
 * article fetch limit (ArticleThreshold .ini param) support.
 *
 * Revision 1.3  1993/04/29  20:25:23  rushing
 * attempted support for WSAAsyncSelect
 *
 * Revision 1.2  1993/03/09  01:28:03  dumoulin
 * added support for italic fonts in printing articles
 *
 * Revision 1.1  1993/02/16  20:53:50  rushing
 * Initial revision
 *
 *
 */

/*  WinVn.c
 *
 *  This program is a visual Usenet news reader for Microsoft Windows.
 *  It bears a vague similarity to the Unix program "vn"; hence the
 *  name WINVN.
 *
 *  WinVN talks NNTP (Network News Transport (?) Protocol) to a news
 *  server, which must be running the NNTP program.  (The source for
 *  a unix implementation of NNTP is readily and apparently freely
 *  available.)
 *
 *  For more information, see WINVN.WRI and WVDOC.C.
 *
 *  Mark Riordan    September - October 1989    riordanmr@clvax1.cl.msu.edu
 *  1100 Parker  Lansing, MI  48912
 *  Now being maintained by Sam Rushing, rushing@titan.ksc.nasa.gov
 */

#define STRICT
#define WINMAIN
#include "windows.h"
#include "WVglob.h"
#include "WinVn.h"
#include "ctype.h"
#include "defaults.h"   // default profile settings
#include <stdlib.h>

char *CommStrtoID (char *, int *, int *, char *);

/*--- function WinMain -----------------------------------------------
 *
 *  Main program for WinVN.
 *  Initialize, then execute main loop.
 *
 *    Entry    hInstance      is a handle to this instance of execution
 *                            of this program.
 *             hPrevInstance  is a handle to a previous instance
 *                            of execution of this program (usually
 *                            0, i.e., none.  Few people would have
 *                            two copies of WinVN running simultaneously.)
 *             lpCmdLine      points to the command line--currently
 *                            not used.  (Not to useful for Windows programs.)
 *             nCmdShow       is a flag indicating that the main window
 *                            should be displayed.  (Fairly worthless.)
 */

int PASCAL
WinMain (hInstance, hPrevInstance, lpCmdLine, nCmdShow)
     HINSTANCE hInstance;
     HINSTANCE hPrevInstance;
     LPSTR lpCmdLine;
     int nCmdShow;
{        
  HWND hWnd;
  int x,y,width,height;
  char mybuf[MAXINTERNALLINE];


  if (!hPrevInstance)
    if (!WinVnInit (hInstance, lpCmdLine))
      return (0);

  hInst = hInstance;

  /* Initialize the document that will contain the list of          */
  /* newsgroups.  This will be the main window.                     */

  InitDoc (&NetDoc, (HWND) 0, (TypDoc *) NULL, DOCTYPE_NET);

  /* Create and display the main window.  At first, the window      */
  /* just has an initialization message in it.                      */

  /* Compute default window position and size. */
  x = 0;
  y = 0;
  width = (int) (xScreen * 1 / 2);
  height = (int) (yScreen * 9 / 10);

  /* If the window position and size have been saved, use that
   * info instead.
   */
  GetPrivateProfileString (szAppName, "UsenetWindowPos", "!",
    mybuf,MAXINTERNALLINE,szAppProFile);
  if(mybuf[0] != '!') {
    sscanf(mybuf,"%d,%d,%d,%d",&x,&y,&width,&height);
  }

  hWnd = CreateWindow ("WinVn",
             "WinVN -- Usenet News Reader",
             WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
             x,   /* Initial X position */
             y,   /* Initial Y position */
             width,  /* Initial X width */
             height,/* Initial Y height */
             NULL,
             NULL,
             hInstance,
             NULL);

  if (!hWnd)
    return (0);

  SetHandleBkBrush (hWnd, hListBackgroundBrush); 
  SetUserMenus (hWnd, DISABLE);  // menus disabled until connection made

  hWndConf = hWnd;
  NetDoc.hDocWnd = hWnd;   /* mrr */

  /* Make sure we have a host name (JSC) */
  while (NNTPHost[0] == '\0' || !strcmp (NNTPHost, DEF_NNTPHost))
     if (!DialogBox (hInst, (LPCSTR) "WinVnComm", hWndConf, (DLGPROC) lpfnWinVnCommDlg))
       break;


  /* Make sure we have a valid user name (JD) */
  while (UserName[0] == '\0' || !strcmp (UserName, DEF_UserName)
                             || !strcmp (UserName, "Unknown"))
     if (!DialogBox (hInst, (LPCSTR) "WinVnPersonal", hWndConf, (DLGPROC) lpfnWinVnPersonalInfoDlg))
       break;
  
  MoreInit ();

  ShowWindow (hWnd, nCmdShow);

  UpdateWindow (hWnd);
  SendMessage (hWnd, WM_SIZE, 0, 0L);

  /* Read the NEWSRC file and display its contents in the           */
  /* main window.                                                   */

  if (!ReadNewsrc ())
    {
      MessageBox (hWndConf, "Cannot open the NEWSRC file.", "Fatal error", MB_OK | MB_ICONHAND);
      PostQuitMessage (0);
    }

  Initializing = INIT_NOT_CONNECTED;

  if (ConnectAtStartup)
   Connect ();

  InvalidateRect (hWnd, NULL, TRUE);
  UpdateWindow (hWndConf);

  /* And now for the main loop, which appears in all Windows programs. */
  hAccel = LoadAccelerators (hInstance, "WinVNAccel");

  while (MainLoopPass ());
  return (MainMsg.wParam);
}

/* --- FUNCTION Connect ---------------------------------------------
 * Connect inits comm services and starts connection process
 */
void Connect ()
{
  if (Initializing != INIT_NOT_CONNECTED)
     return;
  
  /* disable the connect menu item during connect attempt */
  SetConnectMenuItem (hWndConf, DISABLE); // connect item has reverse attributes

  Initializing = INIT_ESTAB_CONN;
  InvalidateRect (hWndConf, NULL, TRUE);
  UpdateWindow (hWndConf);
   
  if (MRRInitComm ())   /* Set up communications */
  {
    Initializing = INIT_NOT_CONNECTED;
    SetConnectMenuItem (hWndConf, ENABLE);   // connect item has reverse attributes
    return;
  }
  /* Set up the timer which will wake us up every so often
   * so we can check to see if new characters have arrived from
   * the server.
   */
  idTimer = SetTimer (hWndConf, ID_TIMER , 1000, (TIMERPROC) NULL);

  if (!idTimer)
  {
    MessageBox (hWndConf, "Couldn't create timer!", "Fatal WinVN Error", MB_OK | MB_ICONASTERISK);
    Initializing = INIT_NOT_CONNECTED;
    SetConnectMenuItem (hWndConf, ENABLE);   // connect item has reverse attributes
    return;
  }
  CommDoc = &NetDoc;

  /* MRRInitComm causes CommState to be ST_ESTABLISH_COMM.  DoCommState 
   * begins processing when DoCommInput (started by the timer) sees data
   * from the server.  DoCommState will set Initializing back to 
   * INIT_NOT_CONNECTED if any error.
   * Initializing is INIT_DONE when we're connected and happy
   */
}

/* --- FUNCTION WinVnInit ---------------------------------------------
 *
 *    Initialize the program (first stage).
 *    This routine does some initialization needed before the
 *    creation of the main window.
 *    I put off additional initialization until after the main
 *    window is created.
 *
 *    Entry    hInstance   is a handle to the current instance of
 *                         execution.
 *
 *    Exit     Window classes have been registered, and a small
 *             amount of other window- and comm-related initialization
 *             is done.
 */

BOOL
WinVnInit (HINSTANCE hInstance, LPSTR lpCmdLine )
{
  HANDLE hMemory;
  PWNDCLASS pWndClass;
  HDC hDC;
//  char mesbuf[60];
//  char *errptr;
  BOOL bSuccess;

  Initializing = INIT_READING_NEWSRC;
  CommLineLWAp1 = CommLineIn + MAXCOMMLINE;
  LineHeight = 30;      /* kludge so Net window doesn't get divide-by-zero */

  szAppName = "WinVN";

  // locate newsrc and winvn.ini

  if (fnLocateFiles (hInstance, lpCmdLine))
   return (-1);

  ReadWinvnProfile();

  // obtain point-to-logical-units conversion constants for screen and printer
  hDC = GetDC (hWndConf);
  ScreenYPixels = GetDeviceCaps(hDC, LOGPIXELSY);
  ReleaseDC (hWndConf, hDC);

  LimitedColor = GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE;

  PrinterInit();
 
  // initialize fonts
  InitListFonts();
  InitArticleFonts();
//  InitPrintFonts();  
  InitStatusFonts();  
  
  /* Create pointers to the dialog box functions, needed   */
  /* for routine processing of dialog boxes.               */

  lpfnWinVnCommDlg =  MakeProcInstance ((FARPROC) WinVnCommDlg, hInstance);
  lpfnWinVnSaveArtDlg = MakeProcInstance ((FARPROC) WinVnSaveArtDlg, hInstance);
  lpfnWinVnSaveArtsDlg = MakeProcInstance ((FARPROC) WinVnSaveArtsDlg, hInstance);
  lpfnWinVnFindDlg = MakeProcInstance ((FARPROC) WinVnFindDlg, hInstance);
  lpfnWinVnGenericDlg = MakeProcInstance ((FARPROC) WinVnGenericDlg, hInstance);
  lpfnWinVnSubjectDlg = MakeProcInstance ((FARPROC) WinVnSubjectDlg, hInstance);
  lpfnWinVnAuthDlg = MakeProcInstance ((FARPROC) WinVnAuthDlg, hInstance);
  lpfnWinVnDoListDlg = MakeProcInstance ((FARPROC) WinVnDoListDlg, hInstance);
  lpfnWinVnPersonalInfoDlg = MakeProcInstance ((FARPROC) WinVnPersonalInfoDlg, hInstance);
  lpfnWinVnMiscDlg = MakeProcInstance ((FARPROC) WinVnMiscDlg, hInstance);
  lpfnWinVnAppearanceDlg = MakeProcInstance ((FARPROC)WinVnAppearanceDlg, hInstance);
  lpfnEditSubClass = MakeProcInstance((FARPROC) editIntercept, hInst);
  lpfnWinVnThresholdDlg = MakeProcInstance ((FARPROC)WinVnThresholdDlg, hInstance);
  lpfnWinVnLogOptDlg = MakeProcInstance ((FARPROC) WinVnLogOptDlg ,hInstance) ;
  lpfnWinVnDecodeArtsDlg = MakeProcInstance ((FARPROC) WinVnDecodeArtsDlg, hInstance);
  lpfnWinVnAttachDlg = MakeProcInstance ((FARPROC) WinVnAttachDlg, hInstance);
  lpfnWinVnEncodeDlg = MakeProcInstance ((FARPROC) WinVnEncodeDlg, hInstance);
  lpfnWinVnSigFileDlg = MakeProcInstance ((FARPROC) WinVnSigFileDlg, hInstance);
  lpfnWinVnExitDlg = MakeProcInstance ((FARPROC) WinVnExitDlg, hInstance);
  lpfnWinVnMailDlg = MakeProcInstance ((FARPROC) WinVnMailDlg, hInstance);

  xScreen = GetSystemMetrics (SM_CXSCREEN);
  yScreen = GetSystemMetrics (SM_CYSCREEN);
  CommDoc = &NetDoc;
  Authenticated = FALSE;

  hListBackgroundBrush = CreateSolidBrush (ListBackgroundColor);
  hArticleBackgroundBrush = CreateSolidBrush (ArticleBackgroundColor);
  hStatusBackgroundBrush = CreateSolidBrush (StatusBackgroundColor);

  hMemory = LocalAlloc (LPTR, sizeof (WNDCLASS));
  pWndClass = (PWNDCLASS) LocalLock (hMemory);
  pWndClass->hCursor = LoadCursor (NULL, IDC_ARROW);
  pWndClass->hIcon = LoadIcon (hInstance, (LPSTR) "winvn");
  pWndClass->lpszMenuName = (LPSTR) "ConfMenu";
  pWndClass->lpszClassName = (LPSTR) "WinVn";
  pWndClass->hbrBackground = GetStockObject (WHITE_BRUSH);
  pWndClass->hInstance = hInstance;
  pWndClass->style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
  pWndClass->lpfnWndProc = WinVnConfWndProc;

  bSuccess = RegisterClass (pWndClass);

  if (bSuccess)
    {
      pWndClass->hCursor = LoadCursor (NULL, IDC_ARROW);
      pWndClass->hIcon = LoadIcon (hInstance, (LPSTR) "wvgroup");
      pWndClass->lpszMenuName = (LPSTR) "ViewMenu";
      pWndClass->lpszClassName = (LPSTR) "WinVnView";
      pWndClass->hbrBackground = GetStockObject (WHITE_BRUSH);
      pWndClass->hInstance = hInstance;
      pWndClass->style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
      pWndClass->lpfnWndProc = WinVnViewWndProc;

      bSuccess = RegisterClass (pWndClass);
    }

  if (bSuccess)
    {
      pWndClass->hCursor = LoadCursor (NULL, IDC_ARROW);
      pWndClass->hIcon = LoadIcon (hInstance, (LPSTR) "wvart");
      pWndClass->lpszMenuName = (LPSTR) "ArtMenu";
      pWndClass->lpszClassName = (LPSTR) "WinVnArt";
      pWndClass->hbrBackground = GetStockObject (WHITE_BRUSH);
      pWndClass->hInstance = hInstance;
      pWndClass->style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
      pWndClass->lpfnWndProc = WinVnArtWndProc;

      bSuccess = RegisterClass (pWndClass);
    }

  if (bSuccess)
    {
      pWndClass->hCursor = LoadCursor (NULL, IDC_ARROW);
      pWndClass->hIcon = LoadIcon (hInstance, (LPSTR) "wvpost");
      pWndClass->lpszMenuName = (LPSTR) "PostMenu";
      pWndClass->lpszClassName = (LPSTR) "WinVnPost";
      pWndClass->hbrBackground = GetStockObject (WHITE_BRUSH);
      pWndClass->hInstance = hInstance;
      pWndClass->style = CS_HREDRAW | CS_VREDRAW;
      pWndClass->lpfnWndProc = WinVnPostWndProc;

      bSuccess = RegisterClass (pWndClass);
    }

  if (bSuccess)
    {
      pWndClass->hCursor = LoadCursor (NULL, IDC_ARROW);
      pWndClass->hIcon = LoadIcon (hInstance, (LPSTR) "wvmail");
      pWndClass->lpszMenuName = (LPSTR) "MailMenu";
      pWndClass->lpszClassName = (LPSTR) "WinVnMail";
      pWndClass->hbrBackground = GetStockObject (WHITE_BRUSH);
      pWndClass->hInstance = hInstance;
      pWndClass->style = CS_HREDRAW | CS_VREDRAW;
      pWndClass->lpfnWndProc = WinVnMailWndProc;

      bSuccess = RegisterClass (pWndClass);
    }
  if (bSuccess)
    {
      pWndClass->hCursor = LoadCursor (NULL, IDC_ARROW);
      pWndClass->hIcon = LoadIcon (hInstance, (LPSTR) "wvcstat2");
      pWndClass->lpszMenuName = (LPSTR) "StatMenu";
      pWndClass->lpszClassName = (LPSTR) "WinVnCoding";
      pWndClass->hbrBackground = GetStockObject (WHITE_BRUSH);
      pWndClass->hInstance = hInstance;
      pWndClass->style = CS_HREDRAW | CS_VREDRAW ;
      pWndClass->lpfnWndProc = WinVnCodingWndProc;

      bSuccess = RegisterClass (pWndClass);
    }
  if (bSuccess)
    {
      pWndClass->hCursor = LoadCursor (NULL, IDC_ARROW);
      pWndClass->hIcon = LoadIcon (hInstance, (LPSTR) "wvcstat1");
      pWndClass->lpszMenuName = NULL;
      pWndClass->lpszClassName = (LPSTR) "WinVnBlockCoding";
      pWndClass->hbrBackground = GetStockObject (WHITE_BRUSH);
      pWndClass->hInstance = hInstance;
      pWndClass->style = CS_HREDRAW | CS_VREDRAW ;
      pWndClass->lpfnWndProc = WinVnCodedBlockWndProc;

      bSuccess = RegisterClass (pWndClass);
    }

  LocalUnlock (hMemory);
  LocalFree (hMemory);
  return (bSuccess);
}

/* --- Function MoreInit -----------------------------------------------
 *
 *    Finish up initializing the program.
 *    I do as much initialization here as possible.  I'd rather
 *    have code run after the main window is created (in WinVNInit),
 *    so I have someplace to display error messages if necessary.
 *
 *    I read quite a few profile strings (from WIN.INI)
 *    here and act appropriately (usually means setting a global variable).
 *    I try to read as many as possible of the profile strings used by this
 *    program here.
 */
BOOL
MoreInit ()
{
  register int j;
  extern char uuTable[CODINGTABLESIZE], xxTable[CODINGTABLESIZE], base64Table[CODINGTABLESIZE];
//  CheckView (hWndConf);     /* modify menu as necessary           */

  /* Initialize some document and article related globals */

  for (j = 0; j < 16; j++)      // set custom colors to white
    CustomColors[j] = RGB(255, 255, 255);

  ActiveGroupDoc = (TypDoc *) NULL;
  ActiveArticleDoc = (TypDoc *) NULL;
  for (j = 0; j < MAXGROUPWNDS; j++)
    GroupDocs[j].InUse = FALSE;
  for (j = 0; j < MAXARTICLEWNDS; j++)
    ArticleDocs[j].InUse = FALSE;

  savingArtIndex = -1;     // reset to non-saving state
  SaveArtFileName[0] = '\0';

  CodingState = INACTIVE;
  CommDecoding = FALSE;

  CreateCodingMap(uuMap, uuTable);
  uuMap[' ']=0;      // decode both quote and space to 0 in UU
  CodingMap[CODE_UU] = uuMap;
  CreateCodingMap(xxMap, xxTable);
  CodingMap[CODE_XX] = xxMap;
  CreateCodingMap(base64Map, base64Table);
  CodingMap[CODE_BASE64] = base64Map;
  // the custom map is just a pointer.  The map is defined if/when a
  // custom table is read in during decoding
  CodingMap[CODE_CUSTOM] = customMap;

  AttachFileName[0]='\0';
  Attaching = FALSE;

  Signature = InitTextBlock (hWndConf);
  if (SigFileName[0] != '\0')
    if (ReadFileToTextBlock (hWndConf, Signature, SigFileName, ADD_PLAIN) == FAIL)
    {
   MessageBox (hWndConf, "Signatures disabled", "Signature File Failure", MB_OK);
   EnableSig = FALSE;
    }
     
/*MailList = InitTextBlock (hWndConf);
  if (MailFileName[0] != '\0')
    ReadFileToTextBlock (hWndConf, MailList, MailFileName, ADD_PLAIN);
*/
  hCodedBlockWnd = NULL;
  numStatusTexts = 0;
  for (j = 0; j < MAX_DECODE_THREADS; j++)
    codingStatusText[j] = NULL;

  return (TRUE);
}


/* --- InitFont Functions -----------------------------------------------
 *    Initialize global fonts and metrics
 * Be sure to delete any existing font objects before replacing them
 * Convert points representation to logical units for sizes
 * JSC 1/10/93
 */
void
InitListFonts ()
{
  TEXTMETRIC tmFontInfo;
  HDC hDC;

  /* Unless user specified the system font, create a font as per
   * the user's specifications.
   */

  if (hListFont)     
   DeleteObject (hListFont);

  if (ListFontSize)
  {
   hListFont = CreateFont 
      (-MulDiv(ListFontSize, ScreenYPixels, 72),
      0, /* width */
      0, /* escapement */
      0, /* orientation */
      strstr (ListFontStyle,"Bold") ? FW_BOLD : FW_MEDIUM,
      (BYTE) (strstr (ListFontStyle,"Italic") ? 1 : 0),
      0, /* no underline */
      0, /* no strikeout */
      0,
      OUT_DEFAULT_PRECIS,
      CLIP_DEFAULT_PRECIS,
      0, /* no PROOF_QUALITY */
      0,
      ListFontFace);
  }
  else
   hListFont = GetStockObject (OEM_FIXED_FONT);

  /*  Get information about the font.  LineHeight and CharWidth
   *  are pretty self-explanatory and the methods of determining these
   *  are standard.  TopSpace and SideSpace are the top and left margins
   *  in pixels; I compute them by a method I determined empirically.
   */

  hDC = GetDC (hWndConf);
  SelectObject (hDC, hListFont);
  GetTextMetrics (hDC, (LPTEXTMETRIC) & tmFontInfo);

  LineHeight = tmFontInfo.tmExternalLeading + tmFontInfo.tmHeight;
  CharWidth = tmFontInfo.tmAveCharWidth;
//TopSpace = tmFontInfo.tmExternalLeading;
  TopSpace = LineHeight / 4;
  StartPen = TopSpace;
  SideSpace = CharWidth / 2;
  ReleaseDC (hWndConf, hDC);
}

void
InitArticleFonts ()
{
  TEXTMETRIC tmFontInfo;
  HDC hDC;
  
  if (hFontArtNormal)      
  {
   DeleteObject (hFontArtNormal);
   DeleteObject (hFontArtQuote);
  }
  if (ArticleFontSize && !ArticleFixedFont)
   {
      hFontArtNormal = CreateFont (
      -MulDiv(ArticleFontSize, ScreenYPixels, 72),
      0, /* width */
      0, /* escapement */
      0, /* orientation */
      strstr (ArticleFontStyle,"Bold") ? FW_BOLD : FW_MEDIUM,
      (BYTE) (strstr (ArticleFontStyle,"Italic") ? 1 : 0),
      0, /* no underline */
      0, /* no strikeout */
      0,
      OUT_DEFAULT_PRECIS,
      CLIP_DEFAULT_PRECIS,
      0, /* no PROOF_QUALITY */
      0,
      ArticleFontFace);

     hFontArtQuote = CreateFont (
      -MulDiv(ArticleFontSize, ScreenYPixels, 72),
      0, /* width */
      0, /* escapement */
      0, /* orientation */
      strstr (ArticleFontStyle,"Bold") ? FW_BOLD : FW_MEDIUM,
      (BYTE) (strstr (ArticleFontStyle,"Italic") ? 0 : 1),  // opposite to above italics
      0, /* no underline */
      0, /* no strikeout */
      0,
      OUT_DEFAULT_PRECIS,
      CLIP_DEFAULT_PRECIS,
      0, /* no PROOF_QUALITY */
      0,
      ArticleFontFace);
   }
   else if (ArticleFixedFont)
   {
      hFontArtNormal  = GetStockObject (ANSI_FIXED_FONT);
      hFontArtQuote = GetStockObject (ANSI_FIXED_FONT);
   }
   else
   {
      hFontArtNormal = hListFont;
      hFontArtQuote = hListFont;
   }

  hDC = GetDC (hWndConf);
  SelectObject (hDC, hFontArtNormal);
  GetTextMetrics (hDC, (LPTEXTMETRIC) & tmFontInfo);
  
  ArtLineHeight = tmFontInfo.tmExternalLeading + tmFontInfo.tmHeight;
  ArtCharWidth = tmFontInfo.tmAveCharWidth;

  SelectObject (hDC, hFontArtQuote);
  GetTextMetrics (hDC, (LPTEXTMETRIC) & tmFontInfo);

  ArtLineHeight = max (ArtLineHeight, (unsigned int) (tmFontInfo.tmExternalLeading + tmFontInfo.tmHeight));
  ArtCharWidth = max (ArtCharWidth, (unsigned int) (tmFontInfo.tmAveCharWidth));

  ArtTopSpace = ArtLineHeight / 4;
  ArtSideSpace = ArtCharWidth / 2;

  ReleaseDC (hWndConf, hDC);
}

void
InitStatusFonts ()
{
  TEXTMETRIC tmFontInfo;
  HDC hDC;

  /* Unless user specified the system font, create a font as per
   * the user's specifications.
   */

  if (hStatusFont)      
   DeleteObject (hStatusFont);

  if (StatusFontSize)
   {
      hStatusFont = CreateFont 
      (-MulDiv(StatusFontSize, ScreenYPixels, 72),
      0, /* width */
      0, /* escapement */
      0, /* orientation */
      strstr (StatusFontStyle,"Bold") ? FW_BOLD : FW_MEDIUM,
      (BYTE) (strstr (StatusFontStyle,"Italic") ? 1 : 0),
      0, /* no underline */
      0, /* no strikeout */
      0,
      OUT_DEFAULT_PRECIS,
      CLIP_DEFAULT_PRECIS,
      0, /* no PROOF_QUALITY */
      0,
      StatusFontFace);
   }
   else
      hStatusFont = GetStockObject (OEM_FIXED_FONT);

  /*  Get information about the font.  LineHeight and CharWidth
   *  are pretty self-explanatory and the methods of determining these
   *  are standard.  TopSpace and SideSpace are the top and left margins
   *  in pixels; I compute them by a method I determined empirically.
   */

  hDC = GetDC (hWndConf);
  SelectObject (hDC, hStatusFont);
  GetTextMetrics (hDC, (LPTEXTMETRIC) & tmFontInfo);

  StatusLineHeight = tmFontInfo.tmExternalLeading + tmFontInfo.tmHeight;
  StatusCharWidth = tmFontInfo.tmAveCharWidth;
  ReleaseDC (hWndConf, hDC);
}

void
InitPrintFonts ()
 { 
  HDC hDC;
  if (hFontPrint)    
   {
     DeleteObject (hFontPrint);
     DeleteObject (hFontPrintB);
     DeleteObject (hFontPrintI);
     DeleteObject (hFontPrintS);
    } 

  hDC = (pd.hDC) ? pd.hDC : GetPrinterDC (NULL);  
  PrinterYPixels = GetDeviceCaps(hDC, LOGPIXELSY);
  if (!pd.hDC) DeletePrinterDC (hDC);
  
  if (PrintFontSize)
   {
    hFontPrint = CreateFont (
            -MulDiv(PrintFontSize, PrinterYPixels, 72),
              0,     /* width */
         0, /* escapement */
         0, /* orientation */
         FW_MEDIUM,
         0, /* no italics */
         0, /* no underline */
         0, /* no strikeout */
         0,
         OUT_DEFAULT_PRECIS,     
         CLIP_DEFAULT_PRECIS,
         0, /* no PROOF_QUALITY */
         0,
         PrintFontFace);

    hFontPrintB = CreateFont (
         -MulDiv(PrintFontSize, PrinterYPixels, 72),
         0,     /* width */
         0, /* escapement */
         0, /* orientation */
         FW_BOLD,
         0, /* no italics */
         0, /* no underline */
         0, /* no strikeout */
         0,
         OUT_DEFAULT_PRECIS,
         CLIP_DEFAULT_PRECIS,
         0, /* no PROOF_QUALITY */
         0,
         PrintFontFace);

    hFontPrintI = CreateFont (
         -MulDiv((int)(0.9*PrintFontSize), PrinterYPixels, 72),
         0,     /* width */
         0, /* escapement */
         0, /* orientation */
         FW_LIGHT,
         1, /* italics */
         0, /* no underline */
         0, /* no strikeout */
         0,
         OUT_DEFAULT_PRECIS,
         CLIP_DEFAULT_PRECIS,
         0, /* no PROOF_QUALITY */
         0,
         PrintFontFace);

      hFontPrintS = CreateFont (
         -MulDiv((int)(1.5*PrintFontSize), PrinterYPixels, 72),
         0,     /* width */
         0, /* escapement */
         0, /* orientation */
         FW_EXTRABOLD,     
         0, /* no italics */
         1, /* underline */
         0, /* no strikeout */
         0,
         OUT_DEFAULT_PRECIS,
         CLIP_DEFAULT_PRECIS,
         0, /* no PROOF_QUALITY */
         0,
         PrintFontFace);
   }
  else
   {
     hFontPrint = CreateFont (   
            -MulDiv(8, PrinterYPixels, 72),
   //    40,    /* default Font Size */
         0,     /* width */
         0, /* escapement */
         0, /* orientation */
         FW_MEDIUM,
         0, /* no italics */
         0, /* no underline */
         0, /* no strikeout */
         0,
         OUT_DEFAULT_PRECIS,
         CLIP_DEFAULT_PRECIS,
         0, /* no PROOF_QUALITY */
         0,
         0);    /* Default Font Face */

     hFontPrintB = CreateFont (
            -MulDiv(8, PrinterYPixels, 72),
   //    40,    /* default font size */
         0,     /* width */
         0, /* escapement */
         0, /* orientation */
         FW_BOLD,
         0, /* no italics */
         0, /* no underline */
         0, /* no strikeout */
         0,
         OUT_DEFAULT_PRECIS,
         CLIP_DEFAULT_PRECIS,
         0, /* no PROOF_QUALITY */
         0,
         0);    /* default font face */

   hFontPrintI = CreateFont (  
            -MulDiv((int)(0.9 * 8), PrinterYPixels, 72),
   //    36,    /* default font size */
         0,     /* width */
         0, /* escapement */
         0, /* orientation */
         FW_LIGHT,
         1, /* italics */
         0, /* no underline */
         0, /* no strikeout */
         0,
         OUT_DEFAULT_PRECIS,
         CLIP_DEFAULT_PRECIS,
         0, /* no PROOF_QUALITY */
         0,
         0);    /* default font face */

   hFontPrintS = CreateFont (
            -MulDiv(8, PrinterYPixels, 72),
   //    60,
         0,     /* width */
         0, /* escapement */
         0, /* orientation */
         FW_EXTRABOLD,
         0, /* no italics */
         1, /* underline */
         0, /* no strikeout */
         0,
         OUT_DEFAULT_PRECIS,
         CLIP_DEFAULT_PRECIS,
         0, /* no PROOF_QUALITY */
         0,
         "Arial");
   }
}

/*--- Function WinVnDoComm  -----------------------------------
 *
 *  Set communications parameters.
 *
 *  Entry    szComm         is a string equivalent to that on a MODE statement.
 *                          E.g.,  "COM1:2400,n,8"
 */
int
WinVnDoComm (szComm)
     char *szComm;
{
  int retcode;
  char mesbuf[60];

  if (retcode = BuildCommDCB (szComm, (DCB FAR *) & DCBComm))
    {
      sprintf (mesbuf, "BuildComm returned %d", retcode);
      MessageBox (hWndConf, mesbuf, "Error building COM1 DCB", MB_OK | MB_ICONEXCLAMATION);
    }
  else
    {
      //SetCommState ((DCB FAR *) & DCBComm);
    }
  return (retcode);
}

/*--- Function CommStrtoID --------------------------------------
 *
 *   Takes a communications string of the form given to the MODE command
 *   and breaks it down to its constituent parts.
 *
 *   Entry    CommStr    is the string; e.g., "COM1:2400,n,8"
 *
 *   Exit     *Port      is the port (an IDD_* variable)
 *            *Parity    is the parity/data bits infor (an IDD_* symbol)
 *            szSpeed    is the speed, in character form
 *            CommStr    has been converted to upper case.
 *            Function value NULL if no error, else a pointer
 *             to an error message.
 */
char *
CommStrtoID (CommStr, Port, Parity, szSpeed)
     char *CommStr;
     int *Port, *Parity;
     char *szSpeed;
{
  char *ptr, *Speedptr;

  strupr (CommStr);
  ptr = CommStr;

  while (*(ptr) == ' ')
    ptr++;

  /* Crack the "COMx" part of the string.                           */

  if (strncmp (ptr, "COM", 3) != 0)
    return ("Must be COM port");
  ptr += 3;

  if (*ptr == '1')
    {
      *Port = IDD_COM1;
    }
  else if (*ptr == '2')
    {
      *Port = IDD_COM2;
    }
  else
    {
      return ("COM port must be 1 or 2");
    }

  /* Crack the speed parameter.                                     */

  ptr += 2;
  while (*ptr == ' ')
    ptr++;
  for (Speedptr = szSpeed; isdigit (*ptr); *(Speedptr++) = *(ptr++));
  *Speedptr = '\0';

  if (szSpeed == Speedptr)
    {
      return ("Non-numeric COM speed");
    }

  while (isdigit (*ptr) || *ptr == ' ' || *ptr == ',')
    ptr++;

  /* Crack the parity parameter.                                    */

  if (*ptr == 'N')
    {
      *Parity = IDD_8NONE;
    }
  else if (*ptr == 'E')
    {
      *Parity = IDD_7EVEN;
    }
  else
    {
      return ("Bad parity");
    }

  return (NULL);
}

/*--- function MainLoopPass ---------------------------------------------
 *
 *    Do one pass of the main loop.
 *
 *    Entry
 *
 *    Exit     returns result of GetMessage()
 */
BOOL
MainLoopPass ()
{
  BOOL NoQuit;

  if (NoQuit = GetMessage (&MainMsg, NULL, NULL, NULL))
    {
      if (!TranslateAccelerator (MainMsg.hwnd, hAccel, &MainMsg))
      {
        TranslateMessage (&MainMsg);
        DispatchMessage (&MainMsg);
      }
    }
  return (NoQuit);
}


/*--- function ReadWinvnProfile -----------------------------------------
 *
 *    Read the profile values into global vars 
 *    (JSC 1/8/94)
 */
// section headings (globals)
char OLD[6]="winvn";
char COMM[15]="Communications";
char AUTH[14]="Authorization";
char INTERFACE[10]="Interface";
char PERSONAL[9]="Personal";
char PREF[12]="Preferences";
char CODING[7]="Coding";
char MAIL_ADDRESSES[15]="Mail Addresses";
char INTERNATIONAL[5]="intl";

void 
ReadWinvnProfile ()
{
  char buf[MAXINTERNALLINE];
  int j;

  // test for existence of Comm section in ini file
  // if doesn't exist, we need to upgrade this user's INI file
  if (GetPrivateProfileString (COMM, NULL, "", buf, MAXINTERNALLINE, szAppProFile) == 0)
  {
   UpdateWinvnProfile();
   return;
  }

  GetPrivateProfileString (PERSONAL, "UserName", DEF_UserName, UserName, MAILLEN, szAppProFile);
  GetPrivateProfileString (PERSONAL, "MailAddress", DEF_MailAddr, MailAddress, MAILLEN, szAppProFile);
  GetPrivateProfileString (PERSONAL, "Organization", DEF_Organization, Organization, MAILLEN, szAppProFile);
  GetPrivateProfileString (PERSONAL, "ReplyTo", DEF_ReplyTo, ReplyTo, MAILLEN, szAppProFile);

  UsingSocket =
     GetPrivateProfileInt (COMM, "UseSocket", DEF_UsingSocket, szAppProFile);
  GetPrivateProfileString (COMM, "NNTPHost", DEF_NNTPHost, NNTPHost, MAXNNTPSIZE, szAppProFile);
  GetPrivateProfileString (COMM, "SMTPHost", DEF_SMTPHost, SMTPHost, MAXNNTPSIZE, szAppProFile);
  GetPrivateProfileString (COMM, "NNTPService", DEF_NNTPService, NNTPService, MAXNNTPSIZE, szAppProFile);
  DebugComm = 
     GetPrivateProfileInt (COMM, "DebugComm", DEF_DebugComm, szAppProFile);
  
  MailDemandLogon = 
     GetPrivateProfileInt (COMM, "MailDemandLogon", DEF_MailDemandLogon, szAppProFile);
  MailForceType = 
     GetPrivateProfileInt (COMM, "MailForceType", DEF_MailForceType, szAppProFile);

  GetPrivateProfileString (AUTH, "NNTPUserName", DEF_NNTPUserName, NNTPUserName, MAXNNTPSIZE, szAppProFile);
  GetPrivateProfileString (AUTH, "NNTPPassword", DEF_NNTPPassword, NNTPPasswordEncrypted, 2*MAXNNTPSIZE, szAppProFile);
  AuthReqPost = GetPrivateProfileInt (AUTH, "AuthReqPost", DEF_AuthReq, szAppProFile);
  AuthReqMail = GetPrivateProfileInt (AUTH, "AuthReqMail", DEF_AuthReq, szAppProFile);

  GetPrivateProfileString (COMM, "GenSockDLL", DEF_GenSockDLL, GenSockDLL, MAXFILENAME+1,szAppProFile);

  force_xhdr =
    GetPrivateProfileInt (COMM, "ForceXhdr", DEF_ForceXhdr, szAppProFile);

  threadp =
    GetPrivateProfileInt (PREF, "EnableThreading", DEF_EnableThreading, szAppProFile);
  GetPrivateProfileString (PREF, "SignatureFile", DEF_SignatureFile, SigFileName, MAXFILENAME+1,szAppProFile);

  EnableSig =
     GetPrivateProfileInt (PREF, "EnableSignature", DEF_EnableSignature, szAppProFile);

//  GetPrivateProfileString (PREF, "MailListFile", DEF_MailFileName, MailFileName, MAXFILENAME+1,szAppProFile);

  MailLog = 
     GetPrivateProfileInt (PREF, "MailLog", DEF_MailLog, szAppProFile);
  GetPrivateProfileString (PREF, "MailLogFile" , DEF_MailLogFile, MailLogFile, MAXFILENAME, szAppProFile);
  PostLog = 
     GetPrivateProfileInt (PREF, "PostLog", DEF_PostLog, szAppProFile);
  GetPrivateProfileString (PREF, "PostLogFile" ,DEF_PostLogFile, PostLogFile, MAXFILENAME, szAppProFile);

  DoList = 
     GetPrivateProfileInt (PREF, "DoList", DEF_DoList,szAppProFile);
  ConnectAtStartup =
     GetPrivateProfileInt (PREF, "ConnectAtStartup", DEF_ConnectAtStartup, szAppProFile);
  FullNameFrom = 
     GetPrivateProfileInt (PREF, "FullNameFrom", DEF_FullNameFrom, szAppProFile);
  article_threshold = 
     GetPrivateProfileUInt (PREF, "ArticleThreshold", DEF_ArticleThreshold, szAppProFile);
  ShowUnsubscribed = 
     GetPrivateProfileInt (PREF, "ShowUnsubscribed",DEF_ShowUnsubscribed, szAppProFile); 
  ShowReadArticles = 
     GetPrivateProfileInt (PREF,"ShowReadArticles", DEF_ShowReadArticles, szAppProFile);
  ConfirmBatchOps = 
     GetPrivateProfileInt (PREF,"ConfirmBatchOps", DEF_ConfirmBatchOps, szAppProFile);
  ConfirmExit = 
     GetPrivateProfileInt (PREF,"ConfirmExit", DEF_ConfirmExit, szAppProFile);
  ConfirmReplyTo =
     GetPrivateProfileInt (PREF,"ConfirmReplyTo", DEF_ConfirmReplyTo, szAppProFile);
  ViewNew = 
     GetPrivateProfileInt (PREF, "NewWndGroup", DEF_NewWndGroup, szAppProFile);
  NewArticleWindow = 
     GetPrivateProfileInt (PREF, "NewWndArticle", DEF_NewWndArticle, szAppProFile);
  SaveArtAppend = 
     GetPrivateProfileInt (PREF, "SaveArtAppend", DEF_SaveArtAppend, szAppProFile);
  ShowUnreadOnly = 
     GetPrivateProfileInt (PREF, "ShowUnreadOnly",DEF_ShowUnreadOnly, szAppProFile); 

  ThreadFullSubject = 
    GetPrivateProfileInt (PREF, "ThreadFullSubject", DEF_ThreadFullSubject, szAppProFile);

  GroupMultiSelect = 
    GetPrivateProfileInt (PREF, "GroupMultiSelect", DEF_ThreadFullSubject, szAppProFile);

  GetPrivateProfileString (CODING, "DecodePath", DEF_DecodePath, DecodePathName, MAXFILENAME, szAppProFile);
  CodingStatusVerbose = 
     GetPrivateProfileInt (CODING, "CodingStatusVerbose", DEF_CodingStatusVerbose, szAppProFile);
  DumbDecode = 
     GetPrivateProfileInt (CODING, "DumbDecode", DEF_DumbDecode, szAppProFile);
  ReviewAttach = 
     GetPrivateProfileInt (CODING, "ReviewAttach", DEF_ReviewAttach, szAppProFile);
  ArticleSplitLength = 
      GetPrivateProfileUInt (CODING, "ArticleSplitLength", (unsigned int) DEF_ArticleSplitLength, szAppProFile);
  GetPrivateProfileString (CODING, "EncodingTable", DEF_EncodingTable, UserCodingTable, CODINGTABLESIZE+1, szAppProFile);
  GetPrivateProfileString (CODING, "SubjectTemplate", "%s  -  %f [%p/%t]", SubjectTemplate, CODINGTABLESIZE+1, szAppProFile);
  GenerateMIME = 
     GetPrivateProfileInt (CODING, "GenerateMIME", DEF_GenerateMIME, szAppProFile);
  GetPrivateProfileString (CODING, "MIMEBoundary",DEF_MIMEBoundary, MIMEBoundary, MAXINTERNALLINE, szAppProFile);
  BlankBeforeMIME =
     GetPrivateProfileInt (CODING, "BlankBeforeMIME", DEF_BlankBeforeMIME, szAppProFile);
  GetPrivateProfileString (CODING, "EncodingType", DEF_EncodingType, EncodingType, MAXINTERNALLINE, szAppProFile);
  EncodingTypeNum = EncodingTypeToNum (EncodingType);
  GetPrivateProfileString (CODING, "MIMEUUType", DEF_MIMEUUType, MIMEUUType, 80, szAppProFile);
  GetPrivateProfileString (CODING, "MIMEXXType", DEF_MIMEXXType, MIMEXXType, 80, szAppProFile);
  GetPrivateProfileString (CODING, "MIMECustomType", DEF_MIMECustomType, MIMECustomType, 80, szAppProFile);
  
  GetPrivateProfileString (CODING, "DefaultContentType",DEF_DefaultContentType, DefaultContentType, MAXINTERNALLINE, szAppProFile);
  AttachInNewArt =
     GetPrivateProfileInt (CODING, "AttachInNewArt", DEF_AttachInNewArt, szAppProFile);
  ExecuteDecodedFiles =
     GetPrivateProfileInt (CODING, "ExecuteDecodedFiles", DEF_ExecuteDecodedFiles, szAppProFile);

  GetPrivateProfileString (INTERFACE, "ListFontFace", DEF_ListFontFace, ListFontFace, 32,szAppProFile);
  ListFontSize = 
     GetPrivateProfileInt (INTERFACE, "ListFontSize", DEF_ListFontSize, szAppProFile);
  GetPrivateProfileString (INTERFACE, "ListFontStyle", DEF_ListFontStyle, ListFontStyle, 32, szAppProFile);
//  FontBold = 
//     GetPrivateProfileInt (INTERFACE, "FontBold", TRUE, szAppProFile);
  GetPrivateProfileString (INTERFACE, "ArticleFontFace", DEF_ArticleFontFace, ArticleFontFace, 32, szAppProFile);
  ArticleFontSize = 
     GetPrivateProfileInt (INTERFACE, "ArticleFontSize", DEF_ArticleFontSize, szAppProFile);
  GetPrivateProfileString (INTERFACE, "ArticleFontStyle", DEF_ArticleFontStyle, ArticleFontStyle, 32, szAppProFile);
  GetPrivateProfileString (INTERFACE, "StatusFontFace", DEF_StatusFontFace, StatusFontFace, 32, szAppProFile);
  StatusFontSize = 
     GetPrivateProfileInt (INTERFACE, "StatusFontSize", DEF_StatusFontSize, szAppProFile);
  GetPrivateProfileString (INTERFACE, "StatusFontStyle", DEF_StatusFontStyle, StatusFontStyle, 32, szAppProFile);
  GetPrivateProfileString (INTERFACE, "PrintFontFace", DEF_PrintFontFace, PrintFontFace, 32, szAppProFile);
  PrintFontSize = 
     GetPrivateProfileInt (INTERFACE, "PrintFontSize", DEF_PrintFontSize, szAppProFile);
//  ArticleFixedFont = 
//     GetPrivateProfileInt (INTERFACE, "ArticleFixedFont", FALSE, szAppProFile);
  ArticleFixedFont = 0;
  GetPrivateProfileString (INTERFACE, "NetUnSubscribedColor", DEF_NetUnSubscribedColor, buf, 32, szAppProFile);
  NetUnSubscribedColor = StrToRGB (buf);
  GetPrivateProfileString (INTERFACE, "NetSubscribedColor", DEF_NetSubscribedColor, buf, 32, szAppProFile);
  NetSubscribedColor = StrToRGB (buf);
  GetPrivateProfileString (INTERFACE, "ArticleUnSeenColor", DEF_ArticleUnSeenColor, buf, 32, szAppProFile);
  ArticleUnSeenColor = StrToRGB (buf);
  GetPrivateProfileString (INTERFACE, "ArticleSeenColor", DEF_ArticleSeenColor, buf, 32, szAppProFile);
  ArticleSeenColor = StrToRGB (buf);
  GetPrivateProfileString (INTERFACE, "ArticleTextColor", DEF_ArticleTextColor, buf, 32, szAppProFile);
  ArticleTextColor = StrToRGB (buf);
  GetPrivateProfileString (INTERFACE, "StatusTextColor", DEF_StatusTextColor, buf, 32, szAppProFile);
  StatusTextColor = StrToRGB (buf);
  GetPrivateProfileString (INTERFACE, "ArticleBackgroundColor", DEF_ArticleBackgroundColor, buf, 32, szAppProFile);
  ArticleBackgroundColor = StrToRGB (buf);
  GetPrivateProfileString (INTERFACE, "ListBackgroundColor", DEF_ListBackgroundColor, buf, 32, szAppProFile);
  ListBackgroundColor = StrToRGB (buf);
  GetPrivateProfileString (INTERFACE, "StatusBackgroundColor", DEF_StatusBackgroundColor, buf, 32, szAppProFile);
  StatusBackgroundColor = StrToRGB (buf);
  ThumbTrack = 
     GetPrivateProfileInt (INTERFACE, "ThumbTrack", DEF_ThumbTrack, szAppProFile);

  MailList = InitTextBlock (hWndConf);
  for (j = 1;; j++)
  {
    sprintf (str, "Address%d", j);
    GetPrivateProfileString (MAIL_ADDRESSES, str, "", buf, MAILLEN, szAppProFile);
    if (!strcmp(buf, ""))
      break;
    AddLineToTextBlock (MailList, buf);
  }
  MailListOrigNum = (int)MailList->numLines;
  
  // The following is in WIN.INI (??)
  GetProfileString (INTERNATIONAL, "sDate", "." ,fmtDateDelim ,2 );
//  fmtDateDelim[1]=0 ;
   fmtDaysB4Mth = (1 == GetProfileInt (INTERNATIONAL,"iDate", 0 /* US Date Format*/));

  
#if 0
// Disabled serial comm stuff
  AskComm = 
     GetPrivateProfileInt (COMM, "AskComm", ASK_COMM_INITIAL, szAppProFile);
  GetPrivateProfileString (COMM, "CommString", "COM1:9600,e,7", szCommString, MAXCOMMCHARS,szAppProFile);
  errptr = CommStrtoID (szCommString, &CommPortID, &CommParityID, pszCommSpeed);
  if (errptr)
    {
      strcpy (mesbuf, "Error parsing ");
      strcat (mesbuf, szCommString);
      MessageBox (hWndConf, errptr, mesbuf, MB_OK | MB_ICONEXCLAMATION);
    }
#endif
}

/*--- function WriteWinvnProfile -----------------------------------------
 *
 *    Write the values modifiable in WinVn menus to the profile 
 *    (JSC 1/8/94)
 */
void 
WriteWinvnProfile ()
{
  register int j;
  
  WritePrivateProfileString (PERSONAL, "UserName", UserName, szAppProFile);
  WritePrivateProfileString (PERSONAL, "MailAddress", MailAddress, szAppProFile);
  WritePrivateProfileString (PERSONAL, "Organization", Organization, szAppProFile);
  WritePrivateProfileString (PERSONAL, "ReplyTo", ReplyTo, szAppProFile);

//WritePrivateProfileInt    (COMM, "AskComm", AskComm, szAppProFile);
//WritePrivateProfileString (COMM, "CommString", szCommString, szAppProFile);
  WritePrivateProfileInt    (COMM, "UseSocket", UsingSocket, szAppProFile);
  WritePrivateProfileString (COMM, "NNTPHost", NNTPHost, szAppProFile);
  WritePrivateProfileString (COMM, "SMTPHost", SMTPHost, szAppProFile);
  WritePrivateProfileString (COMM, "NNTPService", NNTPService, szAppProFile);
  WritePrivateProfileInt    (COMM, "MailDemandLogon", MailDemandLogon, szAppProFile);
  WritePrivateProfileInt    (COMM, "MailForceType", MailForceType, szAppProFile);
  WritePrivateProfileInt    (COMM, "DebugComm", DebugComm, szAppProFile);
  WritePrivateProfileString (COMM, "GenSockDLL", GenSockDLL, szAppProFile);
  
  WritePrivateProfileString (AUTH, "NNTPUserName", NNTPUserName, szAppProFile);
  WritePrivateProfileString (AUTH, "NNTPPassword", NNTPPasswordEncrypted, szAppProFile);

  WritePrivateProfileInt    (PREF, "EnableThreading", threadp, szAppProFile);
  WritePrivateProfileString (PREF, "SignatureFile", SigFileName, szAppProFile);
  WritePrivateProfileInt    (PREF, "EnableSignature", EnableSig, szAppProFile);
//  WritePrivateProfileString (PREF, "MailListFile", MailFileName, szAppProFile);

  WritePrivateProfileInt    (PREF, "MailLog", MailLog, szAppProFile);
  WritePrivateProfileString (PREF, "MailLogFile", MailLogFile, szAppProFile);
  WritePrivateProfileInt    (PREF, "PostLog", PostLog, szAppProFile);
  WritePrivateProfileString (PREF, "PostLogFile", PostLogFile, szAppProFile);

  WritePrivateProfileInt    (PREF, "DoList", DoList, szAppProFile);
  WritePrivateProfileInt    (PREF, "ConnectAtStartup", ConnectAtStartup, szAppProFile);
  WritePrivateProfileInt    (PREF, "FullNameFrom", FullNameFrom, szAppProFile);
  WritePrivateProfileUInt   (PREF, "ArticleThreshold", article_threshold, szAppProFile);
  WritePrivateProfileInt    (PREF, "ShowUnsubscribed", ShowUnsubscribed, szAppProFile);
  WritePrivateProfileInt    (PREF, "ShowReadArticles", ShowReadArticles, szAppProFile);
  WritePrivateProfileInt    (PREF, "ConfirmBatchOps", ConfirmBatchOps, szAppProFile);
  WritePrivateProfileInt    (PREF, "ConfirmExit", ConfirmExit, szAppProFile);
  WritePrivateProfileInt    (PREF, "ConfirmReplyTo", ConfirmReplyTo, szAppProFile);
  WritePrivateProfileInt    (PREF, "NewWndGroup", ViewNew, szAppProFile);
  WritePrivateProfileInt    (PREF, "NewWndArticle", NewArticleWindow, szAppProFile);
  WritePrivateProfileInt    (PREF, "SaveArtAppend", SaveArtAppend, szAppProFile);
  WritePrivateProfileInt    (PREF, "ThreadFullSubject", ThreadFullSubject, szAppProFile);
  WritePrivateProfileInt    (PREF, "GroupMultiSelect", GroupMultiSelect, szAppProFile);
  WritePrivateProfileInt    (PREF, "ShowUnreadOnly", ShowUnreadOnly, szAppProFile);

  WritePrivateProfileString (CODING, "DecodePath", DecodePathName, szAppProFile);
  WritePrivateProfileInt    (CODING, "ExecuteDecodedFiles", ExecuteDecodedFiles, szAppProFile);
  WritePrivateProfileInt    (CODING, "CodingStatusVerbose", CodingStatusVerbose, szAppProFile);
  WritePrivateProfileInt    (CODING, "DumbDecode", DumbDecode, szAppProFile);
  WritePrivateProfileInt    (CODING, "ReviewAttach", ReviewAttach, szAppProFile);
  WritePrivateProfileInt    (CODING, "AttachInNewArt", AttachInNewArt, szAppProFile);
  WritePrivateProfileUInt   (CODING, "ArticleSplitLength", ArticleSplitLength, szAppProFile);
  WritePrivateProfileString (CODING, "EncodingType", EncodingType, szAppProFile);
  WritePrivateProfileString (CODING, "EncodingTable", UserCodingTable, szAppProFile);
  WritePrivateProfileString (CODING, "SubjectTemplate", SubjectTemplate, szAppProFile);
  WritePrivateProfileInt    (CODING, "GenerateMIME", GenerateMIME, szAppProFile);
  WritePrivateProfileString (CODING, "MIMEBoundary", MIMEBoundary, szAppProFile);
  WritePrivateProfileString (CODING, "MIMEUUType", MIMEUUType, szAppProFile);
  WritePrivateProfileString (CODING, "MIMEXXType", MIMEXXType, szAppProFile);
  WritePrivateProfileString (CODING, "MIMECustomType", MIMECustomType, szAppProFile);
  WritePrivateProfileInt    (CODING, "BlankBeforeMIME", BlankBeforeMIME, szAppProFile);
  WritePrivateProfileString (CODING, "DefaultContentType", DefaultContentType, szAppProFile);

  WritePrivateProfileString (INTERFACE, "ListFontFace", ListFontFace, szAppProFile);
  WritePrivateProfileInt    (INTERFACE, "ListFontSize", ListFontSize, szAppProFile);
  WritePrivateProfileString (INTERFACE, "ListFontStyle", ListFontStyle, szAppProFile);
  WritePrivateProfileString (INTERFACE, "ArticleFontFace", ArticleFontFace, szAppProFile);
  WritePrivateProfileInt    (INTERFACE, "ArticleFontSize", ArticleFontSize, szAppProFile);
  WritePrivateProfileString (INTERFACE, "ArticleFontStyle", ArticleFontStyle, szAppProFile);
  WritePrivateProfileString (INTERFACE, "StatusFontFace", StatusFontFace, szAppProFile);
  WritePrivateProfileInt    (INTERFACE, "StatusFontSize", StatusFontSize, szAppProFile);
  WritePrivateProfileString (INTERFACE, "StatusFontStyle", StatusFontStyle, szAppProFile);
  WritePrivateProfileString (INTERFACE, "PrintFontFace", PrintFontFace, szAppProFile);
  WritePrivateProfileInt    (INTERFACE, "PrintFontSize", PrintFontSize, szAppProFile);
//  WritePrivateProfileInt    (INTERFACE, "FontBold", FontBold, szAppProFile);
//  WritePrivateProfileInt    (INTERFACE, "ArticleFixedFont", ArticleFixedFont, szAppProFile);
  WritePrivateProfileString (INTERFACE, "NetUnSubscribedColor", RGBToStr(str, NetUnSubscribedColor), szAppProFile);
  WritePrivateProfileString (INTERFACE, "NetSubscribedColor", RGBToStr(str, NetSubscribedColor), szAppProFile);
  WritePrivateProfileString (INTERFACE, "ArticleUnSeenColor", RGBToStr(str, ArticleUnSeenColor), szAppProFile);
  WritePrivateProfileString (INTERFACE, "ArticleSeenColor", RGBToStr(str, ArticleSeenColor), szAppProFile);
  WritePrivateProfileString (INTERFACE, "ArticleTextColor", RGBToStr(str, ArticleTextColor), szAppProFile);
  WritePrivateProfileString (INTERFACE, "StatusTextColor", RGBToStr(str, StatusTextColor), szAppProFile);
  WritePrivateProfileString (INTERFACE, "ArticleBackgroundColor", RGBToStr(str, ArticleBackgroundColor), szAppProFile);
  WritePrivateProfileString (INTERFACE, "ListBackgroundColor", RGBToStr(str, ListBackgroundColor), szAppProFile);
  WritePrivateProfileString (INTERFACE, "StatusBackgroundColor", RGBToStr(str, StatusBackgroundColor), szAppProFile);
  WritePrivateProfileInt    (INTERFACE, "ThumbTrack", ThumbTrack, szAppProFile);

  if (MailList) {
    for (j = 0; j < max(MailListOrigNum, (int)MailList->numLines); j++)
    {
      sprintf (str, "Address%d", j+1);
      WritePrivateProfileString (MAIL_ADDRESSES, str, 
         (j >= (int)MailList->numLines)?"":TextBlockLine(MailList, j), szAppProFile);
    }
  }
}

/*--- function UpdateWinvnProfile -----------------------------------------
 *
 *    Upgrades a user's old style of WinVn.ini to the new style
 *    (JSC 1/8/94)
 */
void
UpdateWinvnProfile ()
{
  char buf[512];
  
 
  GetPrivateProfileString (OLD, "UserName", DEF_UserName, UserName, MAILLEN, szAppProFile);
  GetPrivateProfileString (OLD, "MailAddress", DEF_MailAddr, MailAddress, MAILLEN, szAppProFile);
  GetPrivateProfileString (OLD, "Organization", DEF_Organization, Organization, MAILLEN, szAppProFile);
  GetPrivateProfileString (OLD, "ReplyTo", DEF_ReplyTo, ReplyTo, MAILLEN, szAppProFile);

  UsingSocket =
     GetPrivateProfileInt (OLD, "UseSocket", DEF_UsingSocket, szAppProFile);
  GetPrivateProfileString (OLD, "NNTPHost", DEF_NNTPHost, NNTPHost, MAXNNTPSIZE, szAppProFile);
  GetPrivateProfileString (OLD, "SMTPHost", DEF_SMTPHost, SMTPHost, MAXNNTPSIZE, szAppProFile);
  GetPrivateProfileString (OLD, "NNTPService", DEF_NNTPService, NNTPService, MAXNNTPSIZE, szAppProFile);
  DebugComm = 
     GetPrivateProfileInt (OLD, "DebugComm", DEF_DebugComm, szAppProFile);
  MailDemandLogon = 
     GetPrivateProfileInt (OLD, "MailDemandLogon", DEF_MailDemandLogon, szAppProFile);
  MailForceType = 
     GetPrivateProfileInt (OLD, "MailForceType", DEF_MailForceType, szAppProFile);

  GetPrivateProfileString (OLD, "NNTPUserName", DEF_NNTPUserName, NNTPUserName, MAXNNTPSIZE, szAppProFile);
  GetPrivateProfileString (OLD, "NNTPPassword", DEF_NNTPPassword, NNTPPasswordEncrypted, 2*MAXNNTPSIZE, szAppProFile);

  threadp =
     GetPrivateProfileInt (OLD, "EnableThreading", DEF_EnableThreading, szAppProFile);
  GetPrivateProfileString (OLD, "SignatureFile", DEF_SignatureFile, SigFileName, MAXFILENAME+1,szAppProFile);
//  GetPrivateProfileString (OLD, "MailListFile", DEF_MailFileName, MailFileName, MAXFILENAME+1,szAppProFile);
  EnableSig =
     GetPrivateProfileInt (OLD, "EnableSignature", DEF_EnableSignature, szAppProFile);

  MailLog = 
     GetPrivateProfileInt (OLD, "MailLog", DEF_MailLog, szAppProFile);
  GetPrivateProfileString (OLD, "MailLogFile" , DEF_MailLogFile, MailLogFile, MAXFILENAME, szAppProFile);
  PostLog = 
     GetPrivateProfileInt (OLD, "PostLog", DEF_PostLog, szAppProFile);
  GetPrivateProfileString (OLD, "PostLogFile" ,DEF_PostLogFile, PostLogFile, MAXFILENAME, szAppProFile);

  ConnectAtStartup =
     GetPrivateProfileInt (OLD, "ConnectAtStartup", DEF_ConnectAtStartup,szAppProFile);
  FullNameFrom = 
     GetPrivateProfileInt (OLD, "FullNameFrom", DEF_FullNameFrom, szAppProFile);
  article_threshold = 
     GetPrivateProfileUInt (OLD, "ArticleThreshold", DEF_ArticleThreshold, szAppProFile);
  ShowUnsubscribed = 
     GetPrivateProfileInt (OLD, "ShowUnsubscribed",DEF_ShowUnsubscribed, szAppProFile); 
  ShowReadArticles = 
     GetPrivateProfileInt (OLD,"ShowReadArticles", DEF_ShowReadArticles, szAppProFile);
  ConfirmBatchOps = 
     GetPrivateProfileInt (OLD,"ConfirmBatchOps", DEF_ConfirmBatchOps, szAppProFile);
  ConfirmExit = 
     GetPrivateProfileInt (OLD,"ConfirmExit", DEF_ConfirmExit, szAppProFile);
  ConfirmReplyTo =
     GetPrivateProfileInt (OLD, "ConfirmReplyTo", DEF_ConfirmReplyTo, szAppProFile);
  ViewNew = 
     GetPrivateProfileInt (OLD, "NewWndGroup", DEF_NewWndGroup, szAppProFile);
  NewArticleWindow = 
     GetPrivateProfileInt (OLD, "NewWndArticle", DEF_NewWndArticle, szAppProFile);
  SaveArtAppend = 
     GetPrivateProfileInt (OLD, "SaveArtAppend", DEF_SaveArtAppend, szAppProFile);
  ShowUnreadOnly = 
     GetPrivateProfileInt (OLD, "ShowUnreadOnly",DEF_ShowUnreadOnly, szAppProFile); 

  GetPrivateProfileString (OLD, "DecodePath", DEF_DecodePath, DecodePathName, MAXFILENAME, szAppProFile);
  CodingStatusVerbose = 
     GetPrivateProfileInt (OLD, "CodingStatusVerbose", DEF_CodingStatusVerbose, szAppProFile);
  DumbDecode = 
     GetPrivateProfileInt (OLD, "DumbDecode", DEF_DumbDecode, szAppProFile);
  ReviewAttach = 
     GetPrivateProfileInt (OLD, "ReviewAttach", DEF_ReviewAttach, szAppProFile);
  GenerateMIME = 
     GetPrivateProfileInt (OLD, "GenerateMIME", DEF_GenerateMIME, szAppProFile);
  GetPrivateProfileString (OLD, "EncodingType", DEF_EncodingType, DefaultContentType, MAXINTERNALLINE, szAppProFile);
  EncodingTypeNum = EncodingTypeToNum (EncodingType);
  GetPrivateProfileString (OLD, "MIMEUUType", DEF_MIMEUUType, MIMEUUType, 80, szAppProFile);
  GetPrivateProfileString (OLD, "MIMEXXType", DEF_MIMEXXType, MIMEXXType, 80, szAppProFile);
  GetPrivateProfileString (OLD, "MIMECustomType", DEF_MIMECustomType, MIMEXXType, 80, szAppProFile);
  GetPrivateProfileString (OLD, "DefaultContentType",DEF_DefaultContentType, DefaultContentType, MAXINTERNALLINE, szAppProFile);
  AttachInNewArt =
     GetPrivateProfileInt (OLD, "AttachInNewArt", DEF_AttachInNewArt, szAppProFile);
  ExecuteDecodedFiles =
     GetPrivateProfileInt (OLD, "ExecuteDecodedFiles", DEF_ExecuteDecodedFiles, szAppProFile);

  ArticleSplitLength = 
      GetPrivateProfileUInt (OLD, "ArticleSplitLength", (unsigned int) DEF_ArticleSplitLength, szAppProFile);
  GetPrivateProfileString (OLD, "EncodingTable", DEF_EncodingTable, UserCodingTable, CODINGTABLESIZE+1, szAppProFile);
  GetPrivateProfileString (OLD, "SubjectTemplate", "%s  -  %f [%p/%t]", SubjectTemplate, CODINGTABLESIZE+1, szAppProFile);
  GetPrivateProfileString (OLD, "MIMEBoundary",DEF_MIMEBoundary, MIMEBoundary, MAXINTERNALLINE, szAppProFile);
  BlankBeforeMIME =
     GetPrivateProfileInt (OLD, "BlankBeforeMIME", DEF_BlankBeforeMIME, szAppProFile);

  GetPrivateProfileString (OLD, "ListFontFace", DEF_ListFontFace, ListFontFace, 32,szAppProFile);
  ListFontSize = 
     GetPrivateProfileInt (OLD, "ListFontSize", DEF_ListFontSize, szAppProFile);
  GetPrivateProfileString (OLD, "ListFontStyle", DEF_ListFontStyle, ListFontStyle, 32, szAppProFile);
//  FontBold = 
//     GetPrivateProfileInt (OLD, "FontBold", TRUE, szAppProFile);
  GetPrivateProfileString (OLD, "ArticleFontFace", DEF_ArticleFontFace, ArticleFontFace, 32, szAppProFile);
  ArticleFontSize = 
     GetPrivateProfileInt (OLD, "ArticleFontSize", DEF_ArticleFontSize, szAppProFile);
  GetPrivateProfileString (OLD, "ArticleFontStyle", DEF_ArticleFontStyle, ArticleFontStyle, 32, szAppProFile);
  GetPrivateProfileString (OLD, "StatusFontFace", DEF_StatusFontFace, StatusFontFace, 32, szAppProFile);
  StatusFontSize = 
     GetPrivateProfileInt (OLD, "StatusFontSize", DEF_StatusFontSize, szAppProFile);
  GetPrivateProfileString (OLD, "StatusFontStyle", DEF_StatusFontStyle, StatusFontStyle, 32, szAppProFile);
  GetPrivateProfileString (OLD, "PrintFontFace", DEF_PrintFontFace, PrintFontFace, 32, szAppProFile);
  PrintFontSize = 
     GetPrivateProfileInt (OLD, "PrintFontSize", DEF_PrintFontSize, szAppProFile);
//
// Previous versions of WINVN.INI stored fonts in Device dependent units
// instead of Device Independent font sizes.  Since we can't safely 
// convert, set font to something reasonable.   (JD 4/22/94)
//
  if (ArticleFontSize && (ArticleFontSize == 0 || ArticleFontSize > 16))
     ArticleFontSize = 10;
  if (ListFontSize && (ListFontSize == 0 || ListFontSize > 16))
     ListFontSize = 10;
  if (StatusFontSize && (StatusFontSize == 0 || StatusFontSize > 16))
     StatusFontSize = 11;
  if (PrintFontSize && (PrintFontSize == 0 || PrintFontSize > 16))
     PrintFontSize = 10;

  ArticleFixedFont = 0;
  GetPrivateProfileString (OLD, "NetUnSubscribedColor", DEF_NetUnSubscribedColor, buf, 32, szAppProFile);
  NetUnSubscribedColor = StrToRGB (buf);
  GetPrivateProfileString (OLD, "NetSubscribedColor", DEF_NetSubscribedColor, buf, 32, szAppProFile);
  NetSubscribedColor = StrToRGB (buf);
  GetPrivateProfileString (OLD, "ArticleUnSeenColor", DEF_ArticleUnSeenColor, buf, 32, szAppProFile);
  ArticleUnSeenColor = StrToRGB (buf);
  GetPrivateProfileString (OLD, "ArticleSeenColor", DEF_ArticleSeenColor, buf, 32, szAppProFile);
  ArticleSeenColor = StrToRGB (buf);
  GetPrivateProfileString (OLD, "ArticleTextColor", DEF_ArticleTextColor, buf, 32, szAppProFile);
  ArticleTextColor = StrToRGB (buf);
  GetPrivateProfileString (OLD, "StatusTextColor", DEF_StatusTextColor, buf, 32, szAppProFile);
  StatusTextColor = StrToRGB (buf);
  GetPrivateProfileString (OLD, "ArticleBackgroundColor", DEF_ArticleBackgroundColor, buf, 32, szAppProFile);
  ArticleBackgroundColor = StrToRGB (buf);
  GetPrivateProfileString (OLD, "ListBackgroundColor", DEF_ListBackgroundColor, buf, 32, szAppProFile);
  ListBackgroundColor = StrToRGB (buf);
  GetPrivateProfileString (OLD, "StatusBackgroundColor", DEF_StatusBackgroundColor, buf, 32, szAppProFile);
  StatusBackgroundColor = StrToRGB (buf);
  ThumbTrack = 
     GetPrivateProfileInt (OLD, "ThumbTrack", DEF_ThumbTrack, szAppProFile);

  GetPrivateProfileString (COMM, "GenSockDLL", DEF_GenSockDLL, GenSockDLL, MAXFILENAME+1,szAppProFile);
  if (!GenSockDLL)
    {
     WritePrivateProfileString (COMM, "GenSockDLL", "gensock.dll", szAppProFile);
     strcpy (GenSockDLL, "gensock.dll");
    }
  WriteWinvnProfile();
  WritePrivateProfileString (OLD, "Notice1", "**** The [winvn] section is no longer needed unless    ****", szAppProFile);
  WritePrivateProfileString (OLD, "Notice2", "**** you want compatibility with old versions of WinVn ****", szAppProFile);

  strlwr (szAppProFile);
  sprintf (buf, "WinVN has discovered your configuration file is from an older release."
                "  Your configuration file %s has been updated.\nIf something doesn't work"
                " as expected, please review the changes.", szAppProFile);
  MessageBox (hWndConf, buf, "WinVn Configuration", MB_OK);
}

#if 0
/*--- function WriteMailRc -----------------------------------------
 *
 *    Write mail address list to file
 *    (JSC 2/1/94)
 */
void
WriteMailrc ()
{
  HANDLE hRetCode;
  TypMRRFile *MRRFile;
  register long j;
  int len;
  char mybuf[MAXINTERNALLINE];
  
  // overwrite previous file
  hRetCode = MRROpenFile (MailFileName, OF_CREATE, &MRRFile);
  if ((int) hRetCode < 0)
    {
      sprintf (str, "MRROpenFile on %s returned %d", MailFileName, hRetCode);
      MessageBox (hWndConf, str, "Write to mailrc file failed", MB_OK);
    }
  for (j = 0; j < (int)MailList->numLines; j++)
  {
      strcpy (mybuf, TextBlockLine (MailList, j));
      len = strlen (mybuf);
      MRRWriteLine (MRRFile, mybuf, len);
  }
  MRRCloseFile (MRRFile);
}
#endif
