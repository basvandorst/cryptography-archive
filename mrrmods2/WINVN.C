/*
 *
 * $Id: winvn.c 1.12 1993/07/06 21:09:09 cnolan Exp $
 * $Log: winvn.c $
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

#define WINMAIN
#include "windows.h"
#include "WVglob.h"
#include "WinVn.h"
#include "winundoc.h"
#include "ctype.h"

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
     HANDLE hInstance;
     HANDLE hPrevInstance;
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
  /* just has an initialization message in it.							  */

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
		       WS_OVERLAPPEDWINDOW | WS_VSCROLL,
				 x,	/* Initial X position */
				 y,	/* Initial Y position */
				 width,	/* Initial X width */
				 height,/* Initial Y height */
		       NULL,
		       NULL,
		       hInstance,
		       NULL);

  if (!hWnd)
    return (0);

  hWndConf = hWnd;
  NetDoc.hDocWnd = hWnd;	/* mrr */

  /* Set up communications--serial or IP.                           */

  if (MRRInitComm ())
      PostQuitMessage(0);

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
  Initializing = INIT_ESTAB_CONN;
  InvalidateRect (hWnd, NULL, FALSE);

  /* And now for the main loop, which appears in all Windows programs. */


  hAccel = LoadAccelerators (hInstance, "WinVNAccel");

  while (MainLoopPass ());
  return (MainMsg.wParam);

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
WinVnInit (HANDLE hInstance, LPSTR lpCmdLine )
{
  HANDLE hMemory;
  PWNDCLASS pWndClass;
//  char mesbuf[60];
  char buf[60];
//  char *errptr;
  BOOL bSuccess;
  char intl_section[5] = "intl" ;

  Initializing = INIT_READING_NEWSRC;
  SaveNewsrc = TRUE;
  CommLineLWAp1 = CommLineIn + MAXCOMMLINE;
  LineHeight = 30;		/* kludge so Net window doesn't get divide-by-zero */


  szAppName = "WinVN";

  // locate newsrc and winvn.ini

  if (fnLocateFiles (hInstance, lpCmdLine))
   return (-1);

  /* Read profile strings.   */

  AskComm = GetPrivateProfileInt (szAppName, "AskComm", ASK_COMM_INITIAL, szAppProFile);
  UsingSocket = GetPrivateProfileInt (szAppName, "UseSocket", 1, szAppProFile);
  threadp = GetPrivateProfileInt (szAppName, "EnableThreading", 0, szAppProFile);
  GetPrivateProfileString (szAppName, "NNTPHost", "news_host_here", NNTPHost, MAXNNTPSIZE,szAppProFile);
  GetPrivateProfileString (szAppName, "SMTPHost", "", SMTPHost, MAXNNTPSIZE,szAppProFile);
  NNTPPort = GetPrivateProfileInt (szAppName, "NNTPPort", 119 ,szAppProFile);
  GetPrivateProfileString (szAppName, "CommString", "COM1:9600,e,7", szCommString, MAXCOMMCHARS,szAppProFile);
  GetPrivateProfileString (szAppName, "NNTPUserName", "", NNTPUserName, MAXNNTPSIZE,szAppProFile);
  GetPrivateProfileString (szAppName, "NNTPPassword", "", NNTPPasswordEncrypted, 2*MAXNNTPSIZE,szAppProFile);

#if 0
  errptr = CommStrtoID (szCommString, &CommPortID, &CommParityID, pszCommSpeed);
  if (errptr)
    {
      strcpy (mesbuf, "Error parsing ");
      strcat (mesbuf, szCommString);
      MessageBox (hWndConf, errptr, mesbuf, MB_OK | MB_ICONEXCLAMATION);
    }
#endif
  DoList =
    GetPrivateProfileInt (szAppName, "DoList", ID_DOLIST_ASK,szAppProFile);
  FullNameFrom =
    GetPrivateProfileInt (szAppName, "FullNameFrom", 1, szAppProFile);
  article_threshold =
    GetPrivateProfileInt (szAppName, "ArticleThreshold",300, szAppProFile);

  GetPrivateProfileString (szAppName, "UserName", "", UserName, MAILLEN,szAppProFile);
  GetPrivateProfileString (szAppName, "MailAddress", "", MailAddress, MAILLEN,szAppProFile);
  GetPrivateProfileString (szAppName, "Organization", "", Organization, MAILLEN,szAppProFile);

  FontSize = GetPrivateProfileInt (szAppName, "FontSize", 0, szAppProFile);
  PrintFontSize = GetPrivateProfileInt (szAppName, "PrintFontSize", 0, szAppProFile);
  GetPrivateProfileString (szAppName, "PrintFontFace", "Helv", PrintFontFace, 32,szAppProFile);
  GetPrivateProfileString (szAppName, "FontFace", "Helv", FontFace, 32,szAppProFile);
  ArticleFontSize = GetPrivateProfileInt (szAppName, "ArticleFontSize", 0,szAppProFile);
  GetPrivateProfileString (szAppName, "ArticleFontFace", "Helv", ArticleFontFace, 32, szAppProFile);
  FontBold = GetPrivateProfileInt (szAppName, "FontBold", TRUE, szAppProFile);
  ArticleFixedFont = GetPrivateProfileInt (szAppName, "ArticleFixedFont", FALSE, szAppProFile);
  GetPrivateProfileString (szAppName, "NetUnSubscribedColor", "255,33,33" /*"200,60,150"*/ , buf, 32, szAppProFile);
  NetUnSubscribedColor = StrToRGB (buf);
  GetPrivateProfileString (szAppName, "GroupSeenColor", "80,100,235" /*"100,120,180"*/ , buf, 32, szAppProFile);
  GroupSeenColor = StrToRGB (buf);
  ViewNew = GetPrivateProfileInt (szAppName, "NewWndGroup", FALSE, szAppProFile);
  NewArticleWindow = GetPrivateProfileInt (szAppName, "NewWndArticle", FALSE, szAppProFile);
  SaveArtAppend = GetPrivateProfileInt (szAppName, "SaveArtAppend", TRUE, szAppProFile);
  ThumbTrack = GetPrivateProfileInt (szAppName, "ThumbTrack", TRUE, szAppProFile);
  MailDemandLogon = GetPrivateProfileInt (szAppName, "MailDemandLogon", FALSE, szAppProFile);
  MailForceType = GetPrivateProfileInt (szAppName, "MailForceType", -1, szAppProFile);

  /* internationalise winvn */

  GetProfileString (intl_section , "sDate", "." ,fmtDateDelim ,2 );
//  fmtDateDelim[1]=0 ;
  fmtDaysB4Mth = (1 == GetProfileInt (intl_section,"iDate", 0 /* US Date Format*/));

  /* Create pointers to the dialog box functions, needed   */
  /* for routine processing of dialog boxes.               */

  lpfnWinVnCommDlg = MakeProcInstance (WinVnCommDlg, hInstance);
  lpfnWinVnSaveArtDlg = MakeProcInstance (WinVnSaveArtDlg, hInstance);
  lpfnWinVnFindDlg = MakeProcInstance (WinVnFindDlg, hInstance);
  lpfnWinVnGenericDlg = MakeProcInstance (WinVnGenericDlg, hInstance);
  lpfnWinVnAuthDlg = MakeProcInstance (WinVnAuthDlg, hInstance);
  lpfnWinVnDoListDlg = MakeProcInstance (WinVnDoListDlg, hInstance);
  lpfnWinVnPersonalInfoDlg = MakeProcInstance (WinVnPersonalInfoDlg, hInstance);
  lpfnWinVnMiscDlg = MakeProcInstance (WinVnMiscDlg, hInstance);
  lpfnWinVnAppearanceDlg = MakeProcInstance (WinVnAppearanceDlg, hInstance);
  lpfnEditSubClass = MakeProcInstance((FARPROC) editIntercept, hInst);
  lpfnWinVnThresholdDlg = MakeProcInstance (WinVnThresholdDlg, hInstance);

  xScreen = GetSystemMetrics (SM_CXSCREEN);
  yScreen = GetSystemMetrics (SM_CYSCREEN);
  CommDoc = &NetDoc;
  Authorized = FALSE;


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
 *    In particular, I do a bunch of font stuff here.
 */

BOOL
MoreInit ()
{
  int retcode, j;
  TEXTMETRIC tmFontInfo;

  HDC hDC;

  retcode = TRUE;
  CheckView (hWndConf);		/* modify menu as necessary           */

  hDC = GetDC (hWndConf);
  /* Unless user specified the system font, create a font as per
   * the user's specifications.
   */

  if (FontSize)
	{
		hFont = CreateFont (FontSize,
		0,	/* width */
		0,	/* escapement */
		0,	/* orientation */
		FontBold ? FW_BOLD : FW_MEDIUM,
		0,	/* no italics */
		0,	/* no underline */
		0,	/* no strikeout */
		0,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		0,	/* no PROOF_QUALITY */
		0,
		FontFace);
	}
	else
	{
		hFont = GetStockObject (OEM_FIXED_FONT);
	}

  if (ArticleFontSize && !ArticleFixedFont)
	{
		hFontArtNormal = CreateFont (ArticleFontSize,
		0,	/* width */
		0,	/* escapement */
		0,	/* orientation */
		FW_MEDIUM,    /* default weight */
		0,	/* no italics */
		0,	/* no underline */
		0,	/* no strikeout */
		0,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		0,	/* no PROOF_QUALITY */
		0,
		ArticleFontFace);
	  hFontArtQuote = CreateFont (ArticleFontSize,
		0,	/* width */
		0,	/* escapement */
		0,	/* orientation */
		FW_MEDIUM,    /* default weight */
		1,	/* comments are in italics */
		0,	/* no underline */
		0,	/* no strikeout */
		0,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		0,	/* no PROOF_QUALITY */
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
		hFontArtNormal = hFont;
		hFontArtQuote = hFont;
	}

  if (PrintFontSize)
	{
	  hFontPrint = CreateFont (PrintFontSize,
		                         0,     /* width */
		                         0,	/* escapement */
		                         0,	/* orientation */
		                         FW_MEDIUM,
		                         0,	/* no italics */
		                         0,	/* no underline */
		                         0,	/* no strikeout */
	                                 0,
		                         OUT_DEFAULT_PRECIS,
		                         CLIP_DEFAULT_PRECIS,
		                         0,	/* no PROOF_QUALITY */
		                         0,
		                         PrintFontFace);

	  hFontPrintB = CreateFont (PrintFontSize,
		                         0,     /* width */
		                         0,	/* escapement */
		                         0,	/* orientation */
		                         FW_BOLD,
		                         0,	/* no italics */
		                         0,	/* no underline */
		                         0,	/* no strikeout */
	                                 0,
		                         OUT_DEFAULT_PRECIS,
		                         CLIP_DEFAULT_PRECIS,
		                         0,	/* no PROOF_QUALITY */
		                         0,
		                         PrintFontFace);

           hFontPrintI = CreateFont ((int)(PrintFontSize * .90),
		                         0,     /* width */
		                         0,	/* escapement */
		                         0,	/* orientation */
		                         FW_LIGHT,
		                         1,	/* italics */
		                         0,	/* no underline */
		                         0,	/* no strikeout */
	                                 0,
		                         OUT_DEFAULT_PRECIS,
		                         CLIP_DEFAULT_PRECIS,
		                         0,	/* no PROOF_QUALITY */
		                         0,
		                         PrintFontFace);

           hFontPrintS = CreateFont ((int)(PrintFontSize * 1.5),
		                         0,     /* width */
		                         0,	/* escapement */
		                         0,	/* orientation */
		                         FW_EXTRABOLD,
		                         0,	/* no italics */
		                         1,	/* underline */
		                         0,	/* no strikeout */
	                                 0,
		                         OUT_DEFAULT_PRECIS,
		                         CLIP_DEFAULT_PRECIS,
		                         0,	/* no PROOF_QUALITY */
		                         0,
		                         "Helv");
	}
	else
	{
                hFontPrint = CreateFont (40,    /* default Font Size */
		                         0,     /* width */
		                         0,	/* escapement */
		                         0,	/* orientation */
		                         FW_MEDIUM,
		                         0,	/* no italics */
		                         0,	/* no underline */
		                         0,	/* no strikeout */
	                                 0,
		                         OUT_DEFAULT_PRECIS,
		                         CLIP_DEFAULT_PRECIS,
		                         0,	/* no PROOF_QUALITY */
		                         0,
		                         0);    /* Default Font Face */

		hFontPrintB = CreateFont (40,   /* default font size */
		                         0,     /* width */
		                         0,	/* escapement */
		                         0,	/* orientation */
		                         FW_BOLD,
		                         0,	/* no italics */
		                         0,	/* no underline */
		                         0,	/* no strikeout */
	                                 0,
		                         OUT_DEFAULT_PRECIS,
		                         CLIP_DEFAULT_PRECIS,
		                         0,	/* no PROOF_QUALITY */
		                         0,
		                         0);    /* default font face */

              hFontPrintI = CreateFont (36,   /* default font size */
		                         0,     /* width */
		                         0,	/* escapement */
		                         0,	/* orientation */
		                         FW_LIGHT,
		                         1,	/* italics */
		                         0,	/* no underline */
		                         0,	/* no strikeout */
	                                 0,
		                         OUT_DEFAULT_PRECIS,
		                         CLIP_DEFAULT_PRECIS,
		                         0,	/* no PROOF_QUALITY */
		                         0,
		                         0);    /* default font face */

              hFontPrintS = CreateFont (60,
		                         0,     /* width */
		                         0,	/* escapement */
		                         0,	/* orientation */
		                         FW_EXTRABOLD,
		                         0,	/* no italics */
		                         1,	/* underline */
		                         0,	/* no strikeout */
	                                 0,
		                         OUT_DEFAULT_PRECIS,
		                         CLIP_DEFAULT_PRECIS,
		                         0,	/* no PROOF_QUALITY */
		                         0,
		                         "Helv");
	}

  ReleaseDC (hWndConf, hDC);

  /*  Get information about the font.  LineHeight and CharWidth
   *  are pretty self-explanatory and the methods of determining these
   *  are standard.  TopSpace and SideSpace are the top and left margins
   *  in pixels; I compute them by a method I determined empirically.
   */

  hDC = GetDC (hWndConf);
  SelectObject (hDC, hFont);
  GetTextMetrics (hDC, (LPTEXTMETRIC) & tmFontInfo);

  LineHeight = tmFontInfo.tmExternalLeading + tmFontInfo.tmHeight;
  CharWidth = tmFontInfo.tmAveCharWidth;
  TopSpace = tmFontInfo.tmExternalLeading;
  TopSpace = LineHeight / 4;
  StartPen = TopSpace;
  SideSpace = tmFontInfo.tmAveCharWidth / 2;
  ReleaseDC (hWndConf, hDC);

  hDC = GetDC (hWndConf);
  SelectObject (hDC, hFontArtNormal);
  GetTextMetrics (hDC, (LPTEXTMETRIC) & tmFontInfo);

  ArtLineHeight = tmFontInfo.tmExternalLeading + tmFontInfo.tmHeight;
  ArtCharWidth = tmFontInfo.tmAveCharWidth;
  ReleaseDC (hWndConf, hDC);

  strcpy (SaveArtFileName, "");

  /* Initialize some document-related stuff.                        */

  ActiveGroupDoc = (TypDoc *) NULL;
  ActiveArticleDoc = (TypDoc *) NULL;
  for (j = 0; j < MAXGROUPWNDS; j++)
    GroupDocs[j].InUse = FALSE;
  for (j = 0; j < MAXARTICLEWNDS; j++)
    ArticleDocs[j].InUse = FALSE;

  return retcode;
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
