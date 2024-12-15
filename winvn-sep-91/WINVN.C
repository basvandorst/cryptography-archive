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
 *  WINVN currently only reads articles; it does not allow posting
 *  or mailing.  Posting will be implemented when I get around to it;
 *  the delay is due to the need to first implement an authentication
 *  scheme so we know the poster is accurately identified.
 *
 *  For more information, see WINVN.WRI and WVDOC.C.
 *
 *  Mark Riordan    September - October 1989    riordanmr@clvax1.cl.msu.edu
 *  1100 Parker  Lansing, MI  48912
 */


#define WINMAIN
#include "windows.h"
#include "WVglob.h"
#include "WinVn.h"
#ifndef MAC
#include "winundoc.h"
#endif
#include "ctype.h"

char *CommStrtoID(char *, int *, int *, char *);

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

int PASCAL WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow)
HANDLE hInstance;
HANDLE hPrevInstance;
LPSTR lpCmdLine;
int nCmdShow;
{
   HWND hWnd;
   HDC hDC;

   if (!hPrevInstance)
      if (!WinVnInit(hInstance))
         return (0);
#ifndef MAC
   hInst = hInstance;
#endif

   /* Set up communications--serial or IP.                           */

   MRRInitComm();

   /* Initialize the document that will contain the list of          */
   /* newsgroups.  This will be the main window.                     */

   InitDoc(&NetDoc,(HWND) 0,(TypDoc *) NULL,DOCTYPE_NET);

   /* Create and display the main window.  At first, the window      */
   /* just has an initialization message in it.                      */

   hWnd = CreateWindow("WinVn",
        "WinVN -- Usenet News Reader",
        WS_OVERLAPPEDWINDOW | WS_VSCROLL,
        0,    /* Initial X position */
        0,    /* Initial Y position */
        (int) (xScreen * 7 / 20),  /* Initial X width */
        (int) (yScreen * 3 / 8),  /* Initial Y height */
        NULL,
        NULL,
        hInstance,
        NULL);

   if (!hWnd)  return (0);

   hWndConf = hWnd;
      NetDoc.hDocWnd = hWnd; /* mrr */

   MoreInit();

#ifndef MAC
   ShowWindow(hWnd, nCmdShow);
#endif
   UpdateWindow(hWnd);
   SendMessage(hWnd,WM_SIZE,0,0L);

   /* Read the NEWSRC file and display its contents in the           */
   /* main window.                                                   */

   if(!ReadNewsrc()) {
      MessageBox(hWndConf,"Cannot open the NEWSRC file.","Fatal error",MB_OK|MB_ICONHAND);
      PostQuitMessage(0);
   }
   Initializing = INIT_ESTAB_CONN;
   InvalidateRect(hWnd,NULL,FALSE);

   /* And now for the main loop, which appears in all Windows programs. */

#ifndef MAC
   hAccel = LoadAccelerators(hInstance, "WinVNAccel");
#endif
#ifndef MAC
   while(MainLoopPass());
   return (MainMsg.wParam);
#endif
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

BOOL WinVnInit(hInstance)
HANDLE hInstance;
{
   HANDLE hMemory;
#ifndef MAC
   PWNDCLASS pWndClass;
#endif
   char mesbuf[60], mesbuf2[60];
   char buf[60];
   char *errptr;
   BOOL bSuccess;

   Initializing = INIT_READING_NEWSRC;
   SaveNewsrc = TRUE;
   CommLineLWAp1 = CommLineIn + MAXCOMMLINE;
   LineHeight = 30;  /* kludge so Net window doesn't get divide-by-zero */

#ifndef MAC
   szAppName = "WinVN";

   /* Read profile strings.   */

   UsingSocket = GetProfileInt(szAppName,"UseSocket",1);
   GetProfileString(szAppName,"NNTPIPAddr","35.8.2.20",NNTPIPAddr,MAXNNTPSIZE);
   NNTPPort = GetProfileInt(szAppName,"NNTPPort",119 /*1521*/ /*normally 119*/);
   GetProfileString(szAppName,"CommString","COM1:9600,e,7",szCommString,MAXCOMMCHARS);
   errptr = CommStrtoID(szCommString,&CommPortID,&CommParityID,pszCommSpeed);
   if(errptr) {
      strcpy(mesbuf,"Error parsing ");
      strcat(mesbuf,szCommString);
      MessageBox(hWndConf,errptr,mesbuf,MB_OK|MB_ICONEXCLAMATION);
   }

   DoList = GetProfileInt(szAppName,"DoList",ID_DOLIST_ASK);

   GetProfileString(szAppName,"UserName","",UserName,MAILLEN);
   GetProfileString(szAppName,"MailAddress","",MailAddress,MAILLEN);
   GetProfileString(szAppName,"Organization","",Organization,MAILLEN);


   FontSize = GetProfileInt(szAppName,"FontSize",0);
   GetProfileString(szAppName,"FontFace","Helv",FontFace,32);
   FontBold = GetProfileInt(szAppName,"FontBold",TRUE);
   GetProfileString(szAppName,"NetUnSubscribedColor","200,60,150",buf,32);
   NetUnSubscribedColor = StrToRGB(buf);
   GetProfileString(szAppName,"GroupSeenColor","100,120,180",buf,32);
   GroupSeenColor = StrToRGB(buf);
   ViewNew = GetProfileInt(szAppName,"NewWndGroup",FALSE);
   NewArticleWindow = GetProfileInt(szAppName,"NewWndArticle",FALSE);
   SaveArtAppend = GetProfileInt(szAppName,"SaveArtAppend",TRUE);
   ThumbTrack = GetProfileInt(szAppName,"ThumbTrack",TRUE);

   /* Create pointers to the dialog box functions, needed   */
   /* for routine processing of dialog boxes.               */

   lpfnWinVnCommDlg =    MakeProcInstance(WinVnCommDlg, hInstance);
   lpfnWinVnSaveArtDlg = MakeProcInstance(WinVnSaveArtDlg,hInstance);
   lpfnWinVnFindDlg =    MakeProcInstance(WinVnFindDlg,hInstance);
   lpfnWinVnAuthDlg =    MakeProcInstance(WinVnAuthDlg,hInstance);
   lpfnWinVnDoListDlg =  MakeProcInstance(WinVnDoListDlg,hInstance);
   lpfnWinVnPersonalInfoDlg = MakeProcInstance(WinVnPersonalInfoDlg,hInstance);
   lpfnWinVnMiscDlg =    MakeProcInstance(WinVnMiscDlg,hInstance);
   lpfnWinVnAppearanceDlg   = MakeProcInstance(WinVnAppearanceDlg,hInstance);

   xScreen = GetSystemMetrics(SM_CXSCREEN);
   yScreen = GetSystemMetrics(SM_CYSCREEN);
#else
   xScreen = screenBits.bounds.right;
   yScreen = screenBits.bounds.bottom - 20;
   TEHCurrent = 0;
#endif
   CommDoc = &NetDoc;
   Authorized = FALSE;

#ifndef MAC
   hMemory = LocalAlloc(LPTR, sizeof(WNDCLASS));
   pWndClass = (PWNDCLASS) LocalLock(hMemory);
   pWndClass->hCursor = LoadCursor(NULL, IDC_ARROW);
   pWndClass->hIcon = LoadIcon(hInstance, (LPSTR) "winvn");
   pWndClass->lpszMenuName = (LPSTR) "ConfMenu";
   pWndClass->lpszClassName = (LPSTR) "WinVn";
   pWndClass->hbrBackground = GetStockObject(WHITE_BRUSH);
   pWndClass->hInstance = hInstance;
   pWndClass->style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
   pWndClass->lpfnWndProc = WinVnConfWndProc;

   bSuccess = RegisterClass(pWndClass);

   if(bSuccess) {
      pWndClass->hCursor = LoadCursor(NULL, IDC_ARROW);
      pWndClass->hIcon = LoadIcon(hInstance, (LPSTR) "wvgroup");
      pWndClass->lpszMenuName = (LPSTR) "ViewMenu";
      pWndClass->lpszClassName = (LPSTR) "WinVnView";
      pWndClass->hbrBackground = GetStockObject(WHITE_BRUSH);
      pWndClass->hInstance = hInstance;
      pWndClass->style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
      pWndClass->lpfnWndProc = WinVnViewWndProc;

      bSuccess = RegisterClass(pWndClass);
    }

   if(bSuccess) {
      pWndClass->hCursor = LoadCursor(NULL, IDC_ARROW);
      pWndClass->hIcon = LoadIcon(hInstance, (LPSTR) "wvart");
      pWndClass->lpszMenuName = (LPSTR) "ArtMenu";
      pWndClass->lpszClassName = (LPSTR) "WinVnArt";
      pWndClass->hbrBackground = GetStockObject(WHITE_BRUSH);
      pWndClass->hInstance = hInstance;
      pWndClass->style = CS_HREDRAW | CS_VREDRAW;
      pWndClass->lpfnWndProc = WinVnArtWndProc;

      bSuccess = RegisterClass(pWndClass);
    }

   if(bSuccess) {
      pWndClass->hCursor = LoadCursor(NULL, IDC_ARROW);
      pWndClass->hIcon = LoadIcon(hInstance, (LPSTR) "wvpost");
      pWndClass->lpszMenuName = (LPSTR) "PostMenu";
      pWndClass->lpszClassName = (LPSTR) "WinVnPost";
      pWndClass->hbrBackground = GetStockObject(WHITE_BRUSH);
      pWndClass->hInstance = hInstance;
      pWndClass->style = CS_HREDRAW | CS_VREDRAW;
      pWndClass->lpfnWndProc = WinVnPostWndProc;

      bSuccess = RegisterClass(pWndClass);
    }

   if(bSuccess) {
      pWndClass->hCursor = LoadCursor(NULL, IDC_ARROW);
      pWndClass->hIcon = LoadIcon(hInstance, (LPSTR) "wvmail");
      pWndClass->lpszMenuName = (LPSTR) "MailMenu";
      pWndClass->lpszClassName = (LPSTR) "WinVnMail";
      pWndClass->hbrBackground = GetStockObject(WHITE_BRUSH);
      pWndClass->hInstance = hInstance;
      pWndClass->style = CS_HREDRAW | CS_VREDRAW;
      pWndClass->lpfnWndProc = WinVnMailWndProc;

      bSuccess = RegisterClass(pWndClass);
    }

    LocalUnlock(hMemory);
    LocalFree(hMemory);
#else
   bSuccess = TRUE;
#endif
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
MoreInit()
{
   char szComName[10];
   int retcode, j;
#ifndef MAC
   TEXTMETRIC  tmFontInfo;

   HDC hDC;


   CheckView(hWndConf);        /* modify menu as necessary           */

   hDC = GetDC(hWndConf);
   /* Unless user specified the system font, create a font as per
    * the user's specifications.
    */
   if(FontSize) {
      hFont = CreateFont(FontSize,
         0,  /* width */
         0,  /* escapement */
         0,  /* orientation */
         FontBold ? FW_BOLD : FW_MEDIUM,
         0,  /* no italics */
         0,  /* no underline */
         0,  /* no strikeout */
         0,
         OUT_DEFAULT_PRECIS,
         CLIP_DEFAULT_PRECIS,
         0,  /* no PROOF_QUALITY */
         0,
         FontFace);
   } else {
      hFont = GetStockObject(SYSTEM_FONT);
   }
   ReleaseDC(hWndConf,hDC);

    /*  Get information about the font.  LineHeight and CharWidth
     *  are pretty self-explanatory and the methods of determining these
     *  are standard.  TopSpace and SideSpace are the top and left margins
     *  in pixels; I compute them by a method I determined empirically.
     */

   hDC = GetDC(hWndConf);
   SelectObject(hDC,hFont);
   GetTextMetrics(hDC,(LPTEXTMETRIC) &tmFontInfo);

   LineHeight = tmFontInfo.tmExternalLeading + tmFontInfo.tmHeight;
   CharWidth  = tmFontInfo.tmAveCharWidth;
   TopSpace = tmFontInfo.tmExternalLeading;
   TopSpace = LineHeight / 4;
   StartPen = TopSpace;
   SideSpace = tmFontInfo.tmAveCharWidth/2;
   ReleaseDC(hWndConf,hDC);

#else
   BigClipRgn = NewRgn();
   SetRectRgn(BigClipRgn,0,0,2000,2000);
   SetWindowVars();
#endif

   strcpy(SaveArtFileName,"");

   /* Initialize some document-related stuff.                        */

   ActiveGroupDoc = (TypDoc *)NULL;
   ActiveArticleDoc = (TypDoc *)NULL;
   for(j=0; j<MAXGROUPWNDS; j++) GroupDocs[j].InUse = FALSE;
   for(j=0; j<MAXARTICLEWNDS; j++) ArticleDocs[j].InUse = FALSE;

   return retcode;
}

#ifndef MAC

/*--- Function WinVnDoComm  -----------------------------------
 *
 *  Set communications parameters.
 *
 *  Entry    szComm         is a string equivalent to that on a MODE statement.
 *                          E.g.,  "COM1:2400,n,8"
 */
int
WinVnDoComm(szComm)
char *szComm ;
{
    int retcode;
    char mesbuf[60];

    if(retcode = BuildCommDCB(szComm,(DCB FAR *) &DCBComm)) {
        sprintf(mesbuf,"BuildComm returned %d",retcode);
        MessageBox(hWndConf,mesbuf,"Error building COM1 DCB",MB_OK|MB_ICONEXCLAMATION);
    } else {
        SetCommState((DCB FAR *) &DCBComm);
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
CommStrtoID(CommStr,Port,Parity,szSpeed)
char *CommStr;
int *Port, *Parity;
char *szSpeed;
{
   char *ptr, *Speedptr;
   int myPort;

   strupr(CommStr);
   ptr = CommStr;

   while(*(ptr) == ' ') ptr++;

   /* Crack the "COMx" part of the string.                           */

   if(strncmp(ptr,"COM",3) != 0) return("Must be COM port");
   ptr += 3;

   if(*ptr == '1') {
      *Port = IDD_COM1;
   } else if (*ptr == '2') {
      *Port = IDD_COM2;
   } else {
      return("COM port must be 1 or 2");
   }

   /* Crack the speed parameter.                                     */

   ptr += 2;
   while(*ptr == ' ') ptr++;
   for(Speedptr = szSpeed; isdigit(*ptr); *(Speedptr++) = *(ptr++));
   *Speedptr = '\0';

   if(szSpeed == Speedptr) {
      return("Non-numeric COM speed");
   }

   while (isdigit(*ptr) || *ptr==' ' || *ptr==',') ptr++;

   /* Crack the parity parameter.                                    */

   if(*ptr == 'N') {
      *Parity = IDD_8NONE;
   } else if(*ptr == 'E') {
      *Parity = IDD_7EVEN;
   } else {
      return("Bad parity");
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
MainLoopPass()
{
   BOOL NoQuit;

   if(NoQuit = GetMessage(&MainMsg, NULL, NULL, NULL)) {
      if(!TranslateAccelerator(MainMsg.hwnd,hAccel,&MainMsg)) {
         TranslateMessage(&MainMsg);
         DispatchMessage(&MainMsg);
      }
   }
   return(NoQuit);
}
#endif
