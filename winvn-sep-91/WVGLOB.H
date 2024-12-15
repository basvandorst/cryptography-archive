/*---  This is WVGLOB.H  ------------------------------------------- */
/* WVGLOB.H contains all the global types and variables for WinVN.   */
/* Stylistically, this should probably be separated into several     */
/* header files, but this is what you've got for now.                */
/* cf WVDOC.C -- it describes some of these structures.              */

#define BUFSIZE   1024
#define MAXFINDSTRING  80

typedef long TypLineID;

typedef struct structdoc {
   HANDLE hFirstBlock;        /* handle to first textblock in doc.        */
   HANDLE hLastBlock;         /* unused.                                  */
   unsigned int TotalLines;   /* total # of lines in doc.                 */
   unsigned int ActiveLines;  /* # of selected or otherwise active lines  */
   unsigned int BlockSize;    /* # of bytes in a textblock                */
   unsigned int SplitSize;    /* textblock split point in bytes           */
   HANDLE hCurAddBlock;       /* hBlock to point at which to add lines.   */
   unsigned int AddOffset;    /* offset in bytes for point to add lines   */
   TypLineID  AddLineID;      /* LineID of point to add lines.            */
   HANDLE hCurTopScBlock;     /* hBlock of current top line of window     */
   unsigned int TopScOffset;
   TypLineID  TopScLineID;
   unsigned int TopLineOrd;   /* ordinal in doc of line line in window    */
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
#ifdef MAC
   RECT  DocClipRect;      /* Current window clip rectangle          */
#endif
} TypDoc;

typedef struct structblock {
   HANDLE hPrevBlock;     /* handle of previous block, or 0               */
   HANDLE hNextBlock;     /* handle of next block in document, or 0       */
   HANDLE hCurBlock;      /* handle of this block                         */
   int    LWAp1;          /* # of used data bytes in block, inc header    */
   int    NumLines;       /* # of lines in this block                     */
   TypDoc *OwnerDoc;      /* pointer to document this block is in.        */
   int    eob;            /* end-of-block; must be just before 1st line.  */
   /* Text lines */
   /* Another EOB marker */
} TypBlock;

typedef struct structline {
   int length;              /* Total # of bytes in line, all-inclusive    */
   TypLineID LineID;        /* Unique identifier for this line.           */
   /* Bytes of text */
   /* Another copy of length */
} TypLine;

typedef struct structtext {
   int NameLen;            /* # of bytes of text                          */
} TypText;

typedef struct structgroup {
   char          Subscribed;      /* =TRUE if subscribed to this group.   */
   char          Selected;        /* =TRUE if selected                    */
   int           NameLen;         /* # of bytes in group name             */
   TypDoc        *SubjDoc;        /* points to doc containing subjs       */
   unsigned int  ServerEstNum;    /* est # of arts in server              */
   long int  ServerFirst;     /* # of first art that server has.      */
   long int  ServerLast;      /* # of last art that server has.       */
   long int  HighestPrevSeen;     /* # of highest art server had in last  */
                                  /* session, from "s<num>" newsrc field  */
   int  nRanges;                  /* # of TypRanges describing seen arts  */
   /* Name of group */            /* name of group, zero-terminated       */
   /* Ranges of articles seen */  /* array of TypRanges, of seen articles */
} TypGroup;

typedef struct structarticle {
   long int Number;            /* Server's number for this article    */
   char         Seen;              /* =TRUE if article seen               */
   char         Selected;          /* =TRUE if article selected           */
   TypDoc      *ArtDoc;            /* points to doc with actual article   */
   int         NameLen;       /* # of bytes in subject line */
   /* Subject line of article */
} TypArticle;

typedef struct structrange {
   long int   First;
   long int   Last;
} TypRange;

/* TypMRRFile is used to describe a file; I use it because Windows  */
/* doesn't provide much in the way of disk I/O support.                   */

typedef struct structMRRfile {
   HANDLE   hFile;           /* handle to file                            */
#ifdef MAC
   int      vRef;
#else
   OFSTRUCT of;
#endif
   char     buf[BUFSIZE];    /* my I/O buffer.                            */
   HANDLE   hthis;           /* handle to this structure                  */
   int      bufidx;          /* Index to next place in buf                */
   int      bytesread;       /* for reads, # of bytes read in last read   */
   int      mode;            /* mode in which to open file.               */
   int      eofflag;         /* whether we have reached EOF (read)        */
} TypMRRFile;

#define MAXCHARS 80
#define MAXLINES 24
#define MAXCOMMLINE      140
#define MAXINTERNALLINE   180
#define MAXOUTLINE    255
#define CtoX(c) (c*CharWidth + SideSpace)
#define LtoY(l) (l*LineHeight + TopSpace)

#define EDITID   1      /* ID of edit box, to identify for return values */
#define MAXMEMONAME 15  /* Number of chars in memo name */
#define MEMONAMECHARS (MAXMEMONAME+2)
#define MAXVIEWS 10
#define END_OF_BLOCK (-1)
#define BLOCK_SIZE  1024
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
struct {
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

#define MAXGROUPWNDS   4
#define MAXARTICLEWNDS 4
#define MAXPOSTWNDS    4
#define MAXMAILWNDS    4

DEF TypDoc GroupDocs[MAXGROUPWNDS];
DEF TypDoc ArticleDocs[MAXARTICLEWNDS];
DEF TypDoc PostingDocs[MAXPOSTWNDS];
DEF TypDoc MailDocs[MAXMAILWNDS];
DEF HWND  hWndPosts[MAXPOSTWNDS];
DEF HWND  hWndMails[MAXMAILWNDS];
#ifndef MAC
DEF HWND  hWndPostEdits[MAXPOSTWNDS];
DEF HWND  hWndMailEdits[MAXMAILWNDS];
#else
DEF TEHandle TEHPosts[MAXPOSTWNDS];
DEF TEHandle TEHCurrent;
DEF ControlHandle    vScroll;
DEF int           linesInFolder;
#endif

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
DEF TypDoc *CommDoc;      /* Document currently receiving lines from server   */
DEF char CommLineIn[MAXCOMMLINE];  /* current input line being built by FSA   */
DEF char *CommLinePtr;    /* pointer to next place to put char in CommLineIn  */
DEF char *CommLineLWAp1;  /* if we get this far, we're at end of buffer       */
DEF char IgnoreCommCh;    /* char to ignore when reading from server          */
DEF char EOLCommCh;       /* char that indicates end of line upon input       */
DEF TypDoc *ActiveGroupDoc;
DEF TypDoc *ActiveArticleDoc;
DEF BOOL SaveNewsrc;      /* =TRUE iff we write NEWSRC upon exit */

#define MAXGROUPNAME 80
DEF char CurrentGroup[MAXGROUPNAME]; /* name of group currently selected on server */

DEF BOOL UsingSocket;     /* =TRUE if using PC/TCP rather than serial I/O     */
#define MAXNNTPSIZE 40
DEF char NNTPIPAddr[MAXNNTPSIZE];
DEF int NNTPPort;

#define MAILLEN 48
DEF char MailAddress[MAILLEN];
DEF char UserName[MAILLEN];
DEF char Organization[MAILLEN];


/* States in the FSA that cracks input lines from the server.             */

#define ST_NONE                    0
#define ST_ESTABLISH_COMM          1
#define ST_GROUP_RESP              2
#define ST_XHDR_RESP               3
#define ST_XHDR_SUBJ               4
#define ST_IN_GROUP                5
#define ST_ARTICLE_RESP            6
#define ST_REC_ARTICLE             7
#define ST_POST_WAIT_PERMISSION    8
#define ST_POST_WAIT_END           9
#define ST_GROUP_REJOIN           10
#define ST_LIST_RESP              11
#define ST_LIST_GROUPLINE         12
#define ST_MAIL_WAIT_PERMISSION   13
#define ST_MAIL_WAIT_END          14

#define LFN_HELP "WINVN.HLP"

/* variables used in an ad hoc attempt to optimize updating of windows by
 * the input cracking FSA.
 */

#define UPDATE_TITLE_FREQ    10
#define UPDATE_ART_FREQ    12
#define MAX_IMMEDIATE_UPDATE  2
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

#ifndef MAC
HANDLE hAccel;     /* handle to main accelerator table */
MSG MainMsg;       /* message returned from GetMessage */

int CommDevice;    /* comm device ID, if using serial I/O                 */
#endif


DEF char str[255];                              /* general-purpose string buffer */

#ifndef MAC
HCURSOR hSaveCursor;                        /* handle to current cursor      */
HCURSOR hHourGlass;                         /* handle to hourglass cursor    */
#endif

DEF int X, Y;                                   /* last cursor position          */

#ifndef MAC
POINT ptCursor;                             /* x and y coordinates of cursor */

DCB DCBComm;
#define MAXCOMMCHARS 40
char szCommString[MAXCOMMCHARS];    /* string describing serial comm port */
#endif

DEF int xPos, yPos;

DEF int ViewNew;
DEF int NewArticleWindow;
DEF int SelectAll;
DEF int NViews;
DEF int DoList;

DEF unsigned int xScreen, yScreen;
DEF HANDLE hFont;   /* handle to font used in all windows                     */
DEF int FontSize;
DEF int FontBold;
DEF char FontFace[32];
DEF int LineHeight,CharWidth;  /* # of window units for current font          */
DEF int TopSpace, SideSpace;   /* # of window units to leave at top and side  */
                           /* of window (for aesthetic purposes)          */
DEF int StartPen;      /* Similar to TopSpace; where to start pen 1st row */

DEF DWORD OldTextColor, OldBkColor;
DEF DWORD NetUnSubscribedColor;  /* color to use for unsubscribed groups  */
DEF DWORD GroupSeenColor;        /* color to use for articles that have been seen */
DEF int ReverseVideo;
DEF BOOL ThumbTrack;
#ifdef MAC
DEF RgnHandle BigClipRgn;
DEF   Rect  myClipRect;

DEF Handle netMenuBar,groupMenuBar,articleMenuBar,postMenuBar;
DEF MenuHandle gAppleMenu;
DEF MenuHandle gOptionsMenu;
#endif

#ifndef MAC
FARPROC lpfnWinVnSaveArtDlg;
FARPROC lpfnWinVnFindDlg;
FARPROC lpfnWinVnCommDlg;
FARPROC lpfnWinVnAuthDlg;
FARPROC lpfnWinVnDoListDlg;
FARPROC lpfnWinVnPersonalInfoDlg;
FARPROC lpfnWinVnMiscDlg;
FARPROC lpfnWinVnAppearanceDlg;
char *szAppName;
#endif

DEF BOOL Authorized;

#define MAXFILENAME 60
DEF char SaveArtFileName[MAXFILENAME];
DEF int  SaveArtvRef;   /* volume ref num used only by Mac */
DEF int  SaveArtAppend;

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

#define LINE_FLAG_SET        0

#define GROUP_ACTION_SUBSCRIBE    0
#define GROUP_ACTION_UNSUBSCRIBE  1

DEF int il,ic;
DEF int irow;
DEF int imemo;
DEF int ih;

#define MAXCOMMSPEEDCHARS 7

DEF int CommPortID;     /* IDD_COM1 or IDD_COM2 */
DEF int CommParityID;   /* IDD_7EVEN or IDD_8NONE  */
DEF char pszCommSpeed[MAXCOMMSPEEDCHARS];  /* character version of comm speed */

DEF int idTimer;                                          /* timer ID            */

#ifndef MAC
#define IncPtr(ptr,byint) (char far *)ptr += byint
#endif

#include <string.h>
#ifndef MAC
int _FAR_ _cdecl sprintf(char _FAR_ *, const char _FAR_ *, ...);
int _FAR_ _cdecl sscanf(const char _FAR_ *, const char _FAR_ *, ...);
#endif
/*--- Last line of WVGLOB.H -------------------------------------------- */
