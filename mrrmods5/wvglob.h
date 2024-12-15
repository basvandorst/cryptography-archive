/*---- Module WVGLOB.H  -------------------------------------------- */
/* WVGLOB.H contains all the global types and variables for WinVN.   */
/* Stylistically, this should probably be separated into several     */
/* header files, but this is what you've got for now.                */
/* cf WVDOC.C -- it describes some of these structures.              */
/*
 *
 * $Id: wvglob.h 1.39 1994/05/23 19:51:51 rushing Exp $
 * $Log: wvglob.h $
 * Revision 1.39  1994/05/23  19:51:51  rushing
 * NNTPPort becomes NNTPService, and it's a string.
 *
 * Revision 1.38  1994/05/23  18:37:00  jcooper
 * new attach code, session [dis]connect
 *
 * Revision 1.37  1994/05/02  20:32:55  rushing
 * changes from jody glasser
 *
 * Revision 1.36  1994/03/01  19:10:19  rushing
 * ThreadFullSubject option added
 *
 * Revision 1.35  1994/02/24  21:28:49  jcoop
 * jcoop changes
 *
 * Revision 1.34  1994/02/16  20:59:17  rushing
 * Added .ini param to force use of XHDR over XOVER
 *
 * Revision 1.33  1994/02/04  02:19:12  rushing
 * added GenSockDLL .ini param
 *
 * Revision 1.32  1994/01/24  17:39:47  jcoop
 * 90.2 changes
 *
 * Revision 1.31  1994/01/17  21:54:58  jcoop
 * New entries for colors/fonts, sig files, attachments, decoding
 *
 * Revision 1.30  1994/01/12  19:25:59  mrr
 * mrr mods 4
 * 
 * Revision 1.29  1994/01/05  22:36:43  mrr
 * BLOCK_SIZE=4096
 *
 * Revision 1.28  1993/12/08  20:21:15  dumoulin
 * Added support for using Shift-Mouse-Left inplace of Mouse-Middle and
 * support for dragging mouse to set article Seen flag
 *
 * Revision 1.27  1993/12/08  01:29:07  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.26  1993/08/25  18:55:19  mbretherton
 * MRB merge, mail & post logging
 *
 *
 * remove vRef (for Macintosh only)
 * include Mail and Post Log variables
 *
 * Revision 1.25  1993/08/18  21:49:21  rushing
 * more 16-bit article number fixes.
 *
 * Revision 1.24  1993/08/17  21:48:44  dumoulin
 * Added variable for Mouse Dragging
 *
 * Revision 1.23  1993/08/05  20:06:07  jcoop
 * save multiple articles changes by jcoop@oakb2s01.apl.com (John S Cooper)
 *
 * Revision 1.22  1993/07/13  16:03:04  riordan
 * MRR mods
 *
 * Revision 1.21  1993/07/09  19:13:36  rushing
 * alignment problem.  Sigh.
 *
 * Revision 1.20  1993/07/06  21:09:09  cnolan
 * win32 support
 *
 * Revision 1.19  1993/06/28  17:52:17  rushing
 * fixed compiler warnings
 *
 * Revision 1.18  1993/06/25  20:27:10  dumoulin
 * Added StringDate to support POSIX standard numeric dates
 *
 * Revision 1.17  1993/06/22  19:08:56  rushing
 * mail force type selection via dialog
 *
 * Revision 1.16  1993/06/22  16:43:58  bretherton
 * remove mail type MT_POP add enable forward menus seperate from mail
 *
 * Revision 1.15  1993/06/22  16:14:21  rushing
 * unsigned long for article number
 *
 * Revision 1.14  1993/06/14  17:05:16  rushing
 * made the MAILLEN define larger.  48 -> 255
 *
 * Revision 1.13  1993/06/11  00:11:26  rushing
 * second merge from Matt Bretherton sources
 *
 * Revision 1.12  1993/06/10  18:25:40  rushing
 * XOVER support and did_list variable (newsrc&dolist problem fixed)
 *
 * Revision 1.11  1993/06/01  18:23:17  rushing
 * moved header funcs to headarry.c
 *
 * Revision 1.10  1993/05/29  03:32:30  rushing
 * threading support
 *
 * Revision 1.9  1993/05/28  17:32:11  rushing
 * made the #defines for ST_XHDR_SUBJ jive with the other new ones.
 *
 * Revision 1.8  1993/05/28  17:18:00  rushing
 * Added Misc... option to enable threading
 *
 * Revision 1.7  1993/05/24  23:57:22  rushing
 * rot13 merge (MRB)
 *
 * Revision 1.6  1993/05/18  22:10:45  rushing
 * smtp support
 *
 * Revision 1.5  1993/05/13  20:00:38  rushing
 * fancy 'from' in group window
 *
 * Revision 1.4  1993/05/13  16:17:45  rushing
 * article fetch limit support
 *
 * Revision 1.3  1993/03/30  20:49:30  DUMOULIN
 * MAPI
 *
 * Revision 1.2  1993/03/30  17:08:12  rushing
 * Added global handle for printer font.
 *
 * Revision 1.1  1993/02/16  20:54:42  rushing
 * Initial revision
 *
 *
 */

#ifdef WIN32         
  #define huge far   
#endif               

#define BUFSIZE   1024
#define TEMPBUFSIZE   1240
#define MAXHEADERLINE 256
#define MAXFINDSTRING  80
#define MAXDIALOGSTRING 255

#define MAXFILENAME 60
#define MAXCHARS 80
#define MAXLINES 24
/* this has been increased for XOVER - some lines are *very* long */
#define MAXCOMMLINE      1536
#define MAXINTERNALLINE   180
#define MAXOUTLINE    255
#define MAXPOSTLENGTH  60000	// max length for a posting/mail edit window
#define MAXBOUNDARYLEN 70    	// max length for MIME boundary

#include <time.h>

typedef long TypLineID;

typedef struct structdoc {
   HANDLE hFirstBlock;        /* handle to first textblock in doc.        */
   HANDLE hLastBlock;         /* unused.                                  */
   unsigned int TotalLines;   /* total # of lines in doc.                 */
   unsigned int HeaderLines;  /* total number of lines in header of doc
                                        (incl. blank line) */
   unsigned int ActiveLines;  /* # of selected or otherwise active lines  */
   unsigned int CountedLines; /* # of counted lines, as in subscr. groups */
   unsigned int BlockSize;    /* # of bytes in a textblock                */
   unsigned int SplitSize;    /* textblock split point in bytes           */
   HANDLE hCurAddBlock;       /* hBlock to point at which to add lines.   */
   unsigned int AddOffset;    /* offset in bytes for point to add lines   */
   TypLineID  AddLineID;      /* LineID of place to add lines.            */
   HANDLE hCurTopScBlock;     /* hBlock of current top line of window     */
   unsigned int TopScOffset;
   TypLineID  TopScLineID;
   unsigned int TopLineOrd;   /* ordinal in doc of top line in window     */
   HANDLE hLastSeenBlock;
   unsigned int LastSeenOffset;
   TypLineID    LastSeenLineID;
   HANDLE hFindBlock;         /* location of place to start next search   */
   unsigned int FindOffset;   /* for "SearchStr"                          */
   TypLineID    FindLineID;
   unsigned int FindTextOffset;
   char SearchStr[MAXFINDSTRING];  /* Search string for current search    */
   unsigned int ScXChars;     /* # of chars/line for current window size  */
   unsigned int ScYLines;     /* # of lines that fit in current window    */
   unsigned int ScXWidth;     /* current size of client area of window    */
   unsigned int ScYHeight;
   HWND hDocWnd;
   struct structdoc *ParentDoc;  /* pointer to parent document            */
   HANDLE hParentBlock;          /* points to line in parent doc          */
   unsigned int ParentOffset;    /* that corresponds to/describes this    */
   TypLineID    ParentLineID;    /* document                              */
   unsigned int OffsetToText;
   BOOL          InUse;          /* TRUE if this TypDoc in use.           */
   int          DocType;         /* DOCTYPE_xxx                           */
} TypDoc;

typedef struct structblock {
   HANDLE hPrevBlock;     /* handle of previous block, or 0               */
   HANDLE hNextBlock;     /* handle of next block in document, or 0       */
   HANDLE hCurBlock;      /* handle of this block                         */
   int    LWAp1;          /* # of used data bytes in block, inc header    */
   int    NumLines;       /* # of lines in this block                     */
   int    NumActiveLines; /* # of active lines in this block.             */
   TypDoc *OwnerDoc;      /* pointer to document this block is in.        */
   int    eob;            /* end-of-block; must be just before 1st line.  */
   /* Text lines */
   /* Another EOB marker */
} TypBlock;

typedef struct structline {
   int length;              /* Total # of bytes in line, all-inclusive    */
   TypLineID LineID;        /* Unique identifier for this line.           */
   int active;              /* =1 if line should be displayed, else 0     */
   /* Bytes of text */
   /* Another copy of length */
} TypLine;

typedef struct structtext {
   int NameLen;            /* # of bytes of text                          */
} TypText;

typedef struct structgroup {
  char      Subscribed; /* =TRUE if subscribed to this group.   */
  char      Selected;   /* =TRUE if selected                    */
  int    NameLen; /* # of bytes in group name             */
  TypDoc *SubjDoc;   /* points to doc containing subjs       */
  unsigned long   ServerEstNum;  /* est # of arts in server              */
  unsigned long   ServerFirst;   /* # of first art that server has.      */
  unsigned long   ServerLast; /* # of last art that server has.       */
  unsigned long   HighestPrevSeen; /* # of highest art server had in last  */
  /* session, from "s<num>" newsrc field  */
  HANDLE header_handle; /* handle to global block of header array mem */
  HANDLE thread_handle; /* handle to global block of thread ind. pointers */
  long int  total_headers; /* total # of headers in array */
  unsigned int  nRanges;   /* # of TypRanges describing seen arts  */
  /* Name of group */            /* name of group, zero-terminated       */
  /* Ranges of articles seen */  /* array of TypRanges, of seen articles */
} TypGroup;

typedef struct structarticle {
  unsigned long   Number;     /* Server's number for this article    */
  char      Seen;    /* =TRUE if article seen               */
  char      Selected;   /* =TRUE if article selected           */
  TypDoc *ArtDoc; /* points to doc with actual article   */
  int    NameLen; /* # of bytes in subject line */
  /* Subject line of article */
} TypArticle;

/* the size of this struct *must* be a power of two for win16.  'huge' */
/* arrays require this. */  
/* (+ 1 1 1 4 2 4 105 74 30 30 4) => 256 */

#define HEADER_SUBJECT_LENGTH    105
#define HEADER_FROM_LENGTH    74
#define HEADER_MESSAGE_ID_LENGTH 30
#define HEADER_REFERENCES_LENGTH 30

typedef struct {
   char Seen;
   char Selected;
   char thread_depth;
   char subject[HEADER_SUBJECT_LENGTH];
   unsigned long  number;
   unsigned int  lines;
//   char date[9];     JD 6/25/93
   time_t date;
   char from[HEADER_FROM_LENGTH];
   char message_id[HEADER_MESSAGE_ID_LENGTH];
   char references[HEADER_REFERENCES_LENGTH];
   TypDoc * ArtDoc;
} TypHeader;

typedef TypHeader huge * header_p;

typedef long huge * thread_array;

typedef thread_array huge * thread_array_p;

typedef char huge * char_p;

typedef struct structrange {
   long int   First;
   long int   Last;
} TypRange;

/* TypMRRFile is used to describe a file; I use it because Windows  */
/* doesn't provide much in the way of disk I/O support.                   */

typedef struct structMRRfile {
   HANDLE   hFile;           /* handle to file                            */
   OFSTRUCT of;
   char     buf[BUFSIZE];    /* my I/O buffer.                            */
   HANDLE   hthis;           /* handle to this structure                  */
   int      bufidx;          /* Index to next place in buf                */
   int      bytesread;       /* for reads, # of bytes read in last read   */
   int      mode;            /* mode in which to open file.               */
   int      eofflag;         /* whether we have reached EOF (read)        */
} TypMRRFile;

#define CtoX(c) (c*CharWidth + SideSpace)
#define LtoY(l) (l*LineHeight + TopSpace)
#define EDITID   1      /* ID of edit box, to identify for return values */
#define MAXMEMONAME 15  /* Number of chars in memo name */
#define MEMONAMECHARS (MAXMEMONAME+2)
#define MAXVIEWS 10
#define END_OF_BLOCK (-1)
#define BLOCK_SIZE  4096
/*  RangeOffset gives the number of bytes from the beginning of
 *  a Group structure to the first Range field, given the length
 *  of the name entry.
 */
#define RangeOffset(nlen) (((nlen+2)/2)*2 + sizeof(TypGroup))

#ifdef WINMAIN
#define DEF
#else
#define DEF extern
#endif

DEF TypDoc NetDoc;
DEF TypDoc DebugCommDoc;

#define MRR_SCROLL_LINEUP    0
#define MRR_SCROLL_LINEDOWN  1
#define MRR_SCROLL_PAGEUP    2
#define MRR_SCROLL_PAGEDOWN  3

/* This structure maps between keystrokes and mouse events.
 * From an idea on p. 137 of Charles Petzold's book.
 * If you change the definition, be sure to update NUMKEYS.
 */
#define NUMKEYS 4
#ifndef MAC
DEF struct {
   WORD wVirtKey;
   int CtlState;
   int iMessage;
   WORD wRequest;
} key2scroll[NUMKEYS]
#ifdef WINMAIN
=
{VK_PRIOR, 0, WM_VSCROLL,SB_PAGEUP,
 VK_NEXT,  0, WM_VSCROLL,SB_PAGEDOWN,
 VK_UP,    1, WM_VSCROLL,SB_LINEUP,
 VK_DOWN,  1, WM_VSCROLL,SB_LINEDOWN
}
#endif
;
#endif

DEF BOOL Initializing;
#define INIT_DONE                0
#define INIT_READING_NEWSRC      1
#define INIT_ESTAB_CONN          2
#define INIT_SCANNING_NETDOC     3
#define INIT_GETTING_LIST        4
#define INIT_NOT_CONNECTED       5
#define INIT_READY               6

#define MAXGROUPWNDS   4
#define MAXARTICLEWNDS 4
#define MAXPOSTWNDS    20 
#define MAXMAILWNDS    20

DEF TypDoc GroupDocs[MAXGROUPWNDS];
DEF TypDoc ArticleDocs[MAXARTICLEWNDS];

typedef struct WndEditStruct
{HWND hWnd ;
 HWND hWndEdit ;
 int dirty ;
 TypDoc * Doc ;
} WndEdit;

#define DT_CLEAN 0
#define DT_INITIALISING -1
#define DT_DIRTY 1

DEF WndEdit  WndPosts[MAXPOSTWNDS];
DEF WndEdit  WndMails[MAXMAILWNDS];

#define DOCTYPE_UNKNOWN  0
#define DOCTYPE_NET      1
#define DOCTYPE_GROUP    2
#define DOCTYPE_ARTICLE  4
#define DOCTYPE_POSTING  8
#define DOCTYPE_MAIL     16

/* Variables and constants for handling the FSA used to deal with    */
/* talking to the NNTP server.                                       */

DEF int CommState;        /* current state in comm FSA                        */
DEF BOOL CommBusy;        /* =TRUE if comm line busy interacting w/ server    */
DEF BOOL CommDecoding;    /* =TRUE if comm line busy decoding lines from server*/
DEF WndEdit *PostEdit, *MailEdit;  /* set to post/mail window during send */
DEF TypDoc *CommDoc;      /* Document currently receiving lines from server   */
DEF HWND CommWnd  ;       /* Window containing details of document currently talking to server */
DEF char CommLineIn[MAXCOMMLINE];  /* current input line being built by FSA   */
DEF char *CommLinePtr;    /* pointer to next place to put char in CommLineIn  */
DEF char *CommLineLWAp1;  /* if we get this far, we're at end of buffer       */
DEF char IgnoreCommCh;    /* char to ignore when reading from server          */
DEF char EOLCommCh;       /* char that indicates end of line upon input       */
DEF TypDoc *ActiveGroupDoc;
DEF TypDoc *ActiveArticleDoc;

#define MAXGROUPNAME 80
DEF char CurrentGroup[MAXGROUPNAME]; /* name of group currently selected on server */

DEF BOOL UsingSocket;     /* =TRUE if using PC/TCP rather than serial I/O     */

DEF BOOL threadp;
DEF BOOL xoverp;
DEF BOOL force_xhdr;

#define MAXNNTPSIZE 40
DEF char NNTPHost[MAXNNTPSIZE];
DEF char NNTPService[MAXNNTPSIZE];
DEF char SMTPHost[MAXNNTPSIZE];
DEF char NNTPUserName[MAXNNTPSIZE];
DEF char NNTPPasswordEncrypted[2*MAXNNTPSIZE];

#define MAILLEN 256
DEF char MailAddress[MAILLEN];
DEF char ReplyTo[MAILLEN];
DEF char UserName[MAILLEN];
DEF char Organization[MAILLEN];

DEF char GenSockDLL[MAXFILENAME];
DEF int  ConnectAtStartup;

/* States in the FSA that cracks input lines from the server.             */

#define ST_NONE                    0
#define ST_ESTABLISH_COMM          1
#define ST_GROUP_RESP              2
#define ST_XHDR_SUBJECT_START      3
#define ST_XHDR_SUBJECT_DATA       4
#define ST_IN_GROUP                5
#define ST_ARTICLE_RESP            6
#define ST_REC_ARTICLE_HEADER      65
#define ST_REC_ARTICLE             7
#define ST_POST_WAIT_PERMISSION    8
#define ST_POST_WAIT_END           9
#define ST_GROUP_REJOIN           10
#define ST_LIST_RESP              11
#define ST_LIST_GROUPLINE         12
#define ST_MAIL_WAIT_PERMISSION   13
#define ST_MAIL_WAIT_END          14
#define ST_CHECK_AUTHINFO_USERNAME 20
#define ST_CHECK_AUTHINFO_PASSWORD 21
#define ST_END_AUTHINFO            22
#define ST_XHDR_FROM_START   50
#define ST_XHDR_FROM_DATA    51
#define ST_XHDR_SUBJ_START   52
#define ST_XHDR_SUBJ_DATA    53
#define ST_XHDR_REF_START    54
#define ST_XHDR_REF_DATA     55
#define ST_XHDR_MID_START    56
#define ST_XHDR_MID_DATA     57
#define ST_XHDR_DATE_START   58
#define ST_XHDR_DATE_DATA    59
#define ST_XHDR_LINES_START     60
#define ST_XHDR_LINES_DATA   61
#define ST_XOVER_START       62
#define ST_XOVER_DATA        63
#define ST_XOVER_CHECK       64
#define ST_CLOSED_COMM            99

#define LFN_HELP "WINVN.HLP"

/* variables used in an ad hoc attempt to optimize updating of windows by
 * the input cracking FSA.
 */

#define UPDATE_TITLE_FREQ    10
#define UPDATE_ART_FREQ    12
#define MAX_IMMEDIATE_UPDATE  2
#define STATUS_UPDATE_FREQ 20

DEF unsigned int RcvLineCount;
DEF unsigned int TimesWndUpdated;

DEF TypLineID NextLineID  /* LineID to assign to next created line            */
#ifdef WINMAIN
= 10664L
#endif
;

DEF HANDLE hInst;
DEF HWND   hWndConf;   /* handle to NetDoc window                             */
DEF HWND   hWndSk;
DEF HWND   hDosWnd;
DEF HWND   hWndDebugComm;  /*handle to Debug Comm window */

DEF HANDLE hAccel;     /* handle to main accelerator table */
DEF MSG MainMsg;       /* message returned from GetMessage */

DEF int CommDevice;    /* comm device ID, if using serial I/O                 */

DEF char str[255];                     /* general-purpose string buffer */


DEF HCURSOR hSaveCursor;               /* handle to current cursor      */
DEF HCURSOR hHourGlass;                /* handle to hourglass cursor    */


DEF int X, Y;                          /* last cursor position          */
DEF int DragMouseAction;               /* mouse action for dragging      */

#define DRAG_NONE                  0
#define DRAG_SELECT                1
#define DRAG_DESELECT              2
#define SEEN_SELECT                3
#define SEEN_DESELECT              4

DEF POINT ptCursor;                    /* x and y coordinates of cursor */

DEF DCB DCBComm;
#define MAXCOMMCHARS 40
DEF char szCommString[MAXCOMMCHARS];    /* string describing serial comm port */


DEF int xPos, yPos;

DEF int ViewNew;
DEF int NewArticleWindow;
DEF int SelectAll;
DEF int NViews;
DEF int DoList;
DEF int did_list;
DEF unsigned int article_threshold;
DEF int arts_to_retrieve;
DEF int savingArtIndex;
DEF int numArtsSaved;  
DEF int nextBatchIndex;		// for batch post/mails
DEF int ConfirmBatchOps;
DEF int ConfirmExit;
DEF int ConfirmReplyTo;
DEF int ShowUnsubscribed;
DEF int ShowReadArticles;  
DEF int DebugComm;

#define	FAIL	-1 
#define SUCCESS	0
#define THREAD_COMPLETE 	1
#define THREAD_INCOMPLETE 	2

/* AskComm says whether to put up the communications dialog box
 * upon starting up.  Options are never, always, and yes, because
 * this is the first time the program has been run.
 */
#define ASK_COMM_NEVER    0
#define ASK_COMM_ALWAYS   1
#define ASK_COMM_INITIAL  2
DEF int AskComm;
DEF BOOL ArticleFixedFont;
DEF BOOL FullNameFrom;

DEF unsigned int xScreen, yScreen;
DEF HANDLE hListFont;     /* handle to font used in group/art lists */
DEF HANDLE hFontArtNormal;/* handle to font used in Article window - normal text mode. */
DEF HANDLE hFontArtQuote; /* handle to font used in Article window - comments == Italics. */
DEF HANDLE hFontPrint;    /* handle to font used for printing. */
DEF HANDLE hFontPrintB;   /* handle to font used for bold printing. */
DEF HANDLE hFontPrintS;   /* handle to font used for printing Subject line. */
DEF HANDLE hFontPrintI;   /* handle to font used for printing Quoted lines. */
DEF HANDLE hStatusFont;   /* handle to font used in status windows */

DEF int ListFontSize;
DEF int ArticleFontSize;
DEF int PrintFontSize;
DEF int StatusFontSize;
//DEF int FontBold;
DEF char ListFontFace[32];
DEF char ListFontStyle[32];
DEF char ArticleFontFace[32];
DEF char ArticleFontStyle[32];
DEF char PrintFontFace[32];
DEF char StatusFontFace[32];
DEF char StatusFontStyle[32];

DEF unsigned int LineHeight,CharWidth;  /* # of window units for list font */
DEF unsigned int ArtLineHeight, ArtCharWidth; /* # of windows units for article font */
DEF unsigned int StatusLineHeight,StatusCharWidth;  /* # of window units for status font */
DEF int TopSpace, SideSpace;   /* # of window units to leave at top and side  */
                           /* of window (for aesthetic purposes)          */
DEF int ArtTopSpace, ArtSideSpace;   /* # of window units to leave at top and side  */
DEF int StartPen;      /* Similar to TopSpace; where to start pen 1st row */
DEF int ScreenYPixels;		    /* used in converting points to logical units */
DEF int PrinterYPixels;		    /* used in converting points to logical units */
DEF COLORREF OldTextColor, OldBkColor;
DEF COLORREF NetUnSubscribedColor;  /* color to use for unsubscribed groups  */
DEF COLORREF NetSubscribedColor;    /* color to use for subscribed groups  */
DEF COLORREF ArticleSeenColor;      /* color to use for articles that have been seen */
DEF COLORREF ArticleUnSeenColor;    /* color to use for articles that have not been seen */
DEF COLORREF ArticleTextColor;      /* color to use for article/post text */
DEF COLORREF StatusTextColor;       /* Color to use for status text */
DEF COLORREF ListBackgroundColor;   /* background color for group/article listings */
DEF COLORREF ArticleBackgroundColor;/* background color for article/post windows */
DEF COLORREF StatusBackgroundColor; /* background color for status windows */
DEF COLORREF CustomColors[16];      /* Custom colors definable by user in ChooseColor */
DEF HBRUSH   hListBackgroundBrush;
DEF HBRUSH   hArticleBackgroundBrush;
DEF HBRUSH   hStatusBackgroundBrush;
DEF BOOL LimitedColor;

DEF int ReverseVideo;
DEF BOOL ThumbTrack;

DEF BOOL  MailDemandLogon;      /* FALSE = permanent logon
                                   TRUE  = logon on send mail */
DEF int  MailForceType;        /*  Force Mail Type to valid value where
               Less then zero means
                                   default negotiation */
DEF int MailLog ;             /* True means append to log files */
DEF int PostLog ;             /* where XXXXLogFile is file name */ 


DEF char MailLogFile[MAXFILENAME] ;
DEF char PostLogFile[MAXFILENAME] ;

DEF int  fmtDaysB4Mth      ;
DEF char fmtDateDelim[2]   ;


#define QuoteLineInd  '>'

DEF FARPROC lpfnWinVnSaveArtDlg;
DEF FARPROC lpfnWinVnSaveArtsDlg;
DEF FARPROC lpfnWinVnFindDlg;
DEF FARPROC lpfnWinVnGenericDlg;
DEF FARPROC lpfnWinVnSubjectDlg;
DEF FARPROC lpfnWinVnCommDlg;
DEF FARPROC lpfnWinVnAuthDlg;
DEF FARPROC lpfnWinVnDoListDlg;
DEF FARPROC lpfnWinVnPersonalInfoDlg;
DEF FARPROC lpfnWinVnMiscDlg;
DEF FARPROC lpfnWinVnAppearanceDlg;
DEF FARPROC lpfnEditSubClass ;
DEF FARPROC lpfnWinVnThresholdDlg;
DEF FARPROC lpfnWinVnLogOptDlg;
DEF FARPROC lpfnWinVnDecodeArtsDlg;
DEF FARPROC lpfnWinVnAttachDlg;
DEF FARPROC lpfnWinVnEncodeDlg;
DEF FARPROC lpfnWinVnSigFileDlg;
DEF FARPROC lpfnWinVnExitDlg;
DEF FARPROC lpfnWinVnMailDlg;

DEF char *szAppName;
DEF char szAppProFile[128];
DEF char szNewsSrc[128];

DEF BOOL Authorized;

DEF char SaveArtFileName[MAXFILENAME];
DEF char DecodePathName[MAXFILENAME];
DEF char DialogString[MAXDIALOGSTRING];
DEF char SubjectString[MAXDIALOGSTRING];
DEF char AddressString[MAXDIALOGSTRING];
DEF char NameString[MAXDIALOGSTRING];
DEF int  SaveArtAppend;
DEF int  ThreadFullSubject;
DEF BOOL GroupMultiSelect;

DEF TypDoc *PostDoc;   /* Used to pass pointer to article being replied to. */
DEF TypDoc *MailDoc;
DEF TypDoc *FindDoc;
DEF char *NewsgroupsPtr;  /* Used to pass pointer to newsgroup of new posting. */

/* For each new group detected, we create an entry in the following
 * array, NewGroupTable.  Each entry contains a far pointer to a
 * dynamically-allocated global data structure containing:
 *  -- Handle to this structure (so we can deallocate later).
 *  -- A line containing information on this group, in the exact
 *     same format as used in the NetDoc.
 */
DEF void far * far * NewGroupTable;  /* array of pointers to new group lines */
DEF HANDLE hNewGroupTable;     /* Handle to the above array */
DEF int nNewGroups;

#define ADD_SUBSCRIBED_END_OF_SUB 0
#define ADD_SUBSCRIBED_TOP_OF_DOC        1

#define LINE_FLAG_SET           0

#define GROUP_ACTION_SUBSCRIBE    0
#define GROUP_ACTION_UNSUBSCRIBE  1
#define GROUP_ACTION_CHECK_ACTIVE 2  

#define ARTICLE_ACTION_SAVE	0
#define COMPARE			1
#define NO_COMPARE		0
#define SHOW			1
#define NO_SHOW			0
#define NO_ID			NULL	
#define REUSE			1
#define NO_REUSE		0
#define SELECT			1
#define DESELECT		0
#define ENABLE			1
#define DISABLE			0
#define BLOCK_ACTION_SET_ACTIVE   0

DEF int il,ic;
DEF int irow;
DEF int imemo;
DEF int ih;

#define MAXCOMMSPEEDCHARS 7

DEF int CommPortID;     /* IDD_COM1 or IDD_COM2 */
DEF int CommParityID;   /* IDD_7EVEN or IDD_8NONE  */
DEF char pszCommSpeed[MAXCOMMSPEEDCHARS];  /* character version of comm speed */

DEF int idTimer;                                          /* timer ID            */

#define IncPtr(ptr,byint) (char far *)ptr += byint

/* the next global variable is scoped globally after the main windows is
   created and is only modified at creation during MailInit in wvmail.c */

DEF struct MailCtrlStruct 
{
    int MailType   ;
    int enableMail ;
    int enableLogout ;
    int enableForward ;
    int (*fnMlInit)(HWND hWnd),
   (*fnMlLogout)(HWND hWnd),
   (*fnMlClose)(HWND hWnd),
        (*fnMlWinCreate)(HWND hWnd,TypDoc *Doc,int DocType),
        (*fnMlSend)(HWND hWnd, char *mBuf),
        (*fnMlForward)(HWND hWnd, TypDoc * Doc);
    BOOL (**MailHeaderFuncs)(TypDoc * Doc, char *Buf, long int BufLen) ;

} MailCtrl ;

#define MT_NONE 0
#define MT_MAPI 1
#define MT_SMTP 2

#define INACTIVE		0       // values for CodingState
#define DECODE_ACTIVE		1	// values for Decoding must be
#define DECODE_SKIPPING		2	// a contiguous range
#define DECODE_PROCESSING       3
#define DECODE_GET_TABLE        4
#define ATTACH_PROCESSING	10	// Encodes must be > decodes
#define ATTACH_POSTING		11
#define ATTACH_READFILE		12

#define CODINGTABLESIZE 	64

#define BASE_BLOCK_SIZE		4096	/* base size for the huge data */
typedef struct structcoded {
  char	name[MAXFILENAME];	/* original file name from begin line */
  char	ident[MAXINTERNALLINE];	/* based on article subject line + author */
  BOOL	beginFlag;		/* =TRUE this block is begin of the complete binary */
  BOOL	endFlag;		/* =TRUE this block is end of the complete binary */
  int	sequence;		/* =-1 if unknown, or # in mult-part file */
  int	seqConfidence;		/* confidence of sequence info */
  unsigned long numLines;	/* num lines read so far (for status window */
  unsigned long	numBytes;       /* # of bytes used in this coded data block */
  unsigned long	maxBytes;       /* max # of bytes in this coded data block */
  char huge *data;		/* the huge data array */
  HWND	hParentWnd;		/* handle to group wnd that started this */
} TypCoded;

typedef struct textBlockStruct {
   HWND hTextWnd;
   unsigned long numLines;
   unsigned long maxLines;
   unsigned long numBytes;
   unsigned long maxBytes;
   unsigned int maxLineLen;
   char huge *text;		/* big text of all lines */
   unsigned long huge *offset;	/* array of offsets into text array */
} TypTextBlock;

#define MAX_DECODE_BLOCKS	256	/* max # of parts for a decode file */
#define MAX_DECODE_THREADS	75	/* max # of files in one batch decode */
typedef struct decodeThreadStruct {
  char	name[MAXFILENAME];	/* original file name from begin line */
  char	ident[MAXINTERNALLINE];	/* thread ID based on article subject line */
  char	dosFileName[MAXFILENAME];/* workable DOS file name */
  char	contentType[80];	/* MIME content type */
  char	contentDesc[80];	/* MIME content description */
  char	customTable[100];	/* custom coding table (if CODE_CUSTOM) */
  int	contentEncoding;	/* MIME content-transfer-encoding */
  int	numBlocksWritten;	/* number of blocks written to disk, so far */
  int	mode;			/* 2nd field of begin header, don't use in DOS */
  int	numBlocks;		/* number of coded blocks used in list */
  int	expectedNumBlocks;	/* 0 if unknown, or a # if found in headers */
  unsigned long totalBytes;	/* total # of bytes for entire file */
  TypCoded *codedBlockList[MAX_DECODE_BLOCKS]; /* list of ptrs to decode objects */
  TypTextBlock *statusText;	/* buffers for thread status text window */
} TypDecodeThread;

#define CODE_BASE64   1
#define CODE_UU       2
#define CODE_XX       3
#define CODE_CUSTOM   4
#define CODE_QP       5
#define CODE_UNKNOWN  6
#define CODE_NONE     0

// these correspond to the # members in the static arrays of strings defined in wvattach.c
#define NUM_ENCODING_TYPES 5
#define NUM_CONTENT_TYPES 10

// coding and attachment globals
DEF char xxMap[128], uuMap[128], base64Map[128], customMap[128];
DEF char *CodingMap[7];	// N+1 for N coding types
DEF int numFilesDecoded;
DEF int CodingState;
DEF TypCoded *currentCoded;
DEF int	numStatusTexts;
DEF TypTextBlock *codingStatusText[MAX_DECODE_THREADS];
DEF BOOL CodingStatusVerbose;
DEF BOOL DumbDecode;
DEF HWND hCodedBlockWnd;
DEF char UserCodingTable[CODINGTABLESIZE+1];
#define STATUSWIDTH (85*StatusCharWidth)
#define STATUSHEIGHT (8*StatusLineHeight)

DEF char  EncodingType[MAXINTERNALLINE];
DEF int  EncodingTypeNum;
DEF int  AttachInNewArt;
DEF char AttachFileName[MAXFILENAME];
DEF char ContentType[MAXINTERNALLINE];
DEF char DefaultContentType[MAXINTERNALLINE];
DEF char SubjectTemplate[MAXINTERNALLINE];
DEF unsigned int ArticleSplitLength;
DEF BOOL ReviewAttach;
DEF BOOL BlankBeforeMIME;
DEF BOOL Attaching;	// == TRUE if currently making attachment
DEF char MIMEBoundary[MAXINTERNALLINE];

#define MIME_VERSION "1.0"
#define ADD_TO_EDIT 1   // used for AddEndedLineToBlock
#define ADD_TO_POST 2
#define ADD_TO_FILE 3
#define ADD_PLAIN   0

// Signature File globals
DEF int EnableSig;
DEF char SigFileName[MAXFILENAME];
DEF TypTextBlock *Signature;
#define BIG_SIG_FILE	40	// warning if sig file > this many lines

// Mail List globals
DEF char MailFileName[MAXFILENAME];
DEF TypTextBlock *MailList;
DEF int MailListOrigNum;
#define MAX_MAIL_NAMES 50000;	// warning if mail file > this many lines

// macros added by JSC
#define uitoa(x,s,b) ultoa((unsigned long)(x),(s),(b)) // why is there no uitoa!?
#define atoui(s) (unsigned int)atol((s))
#define Decoding (CodingState >= DECODE_ACTIVE && CodingState <= DECODE_GET_TABLE)
#define TextBlockLine(b, n) &(b->text[b->offset[n]])

#include <string.h>
#include <stdio.h>  // sprintf, etc...

/*--- Last line of WVGLOB.H -------------------------------------------- */
