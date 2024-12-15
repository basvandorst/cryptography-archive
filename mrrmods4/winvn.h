
/*
 *
 * $Id: winvn.h 1.22 1993/12/08 01:29:07 rushing Exp $
 * $Log: winvn.h $
 * Revision 1.22  1993/12/08  01:29:07  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.21  1993/08/25  18:53:17  mbretherton
 * MRB merge, mail & post logging
 *
 * Revision 1.20  1993/08/25  17:04:57  mbretherton
 * merge from first newsrc.zip
 *
 * Revision 1.2x  21/7/93 Matthew B.
 *                remove vRef (for Macintosh only)
 *
 * Revision 1.19  1993/08/18  21:49:21  rushing
 * more 16-bit article number fixes.
 *
 * Revision 1.18  1993/08/05  20:06:07  jcoop
 * save multiple articles changes by jcoop@oakb2s01.apl.com (John S Cooper)
 *
 * Revision 1.17  1993/07/13  16:03:04  riordan
 * MRR mods
 *
 * Revision 1.16  1993/07/06  21:09:09  cnolan
 * win32 support
 *
 * Revision 1.15  1993/06/28  17:51:39  rushing
 * fixed compiler warnings
 *
 * Revision 1.14  1993/06/25  20:26:07  dumoulin
 * Added StringDate to support POSIX standard numeric dates
 *
 * Revision 1.13  1993/06/24  17:13:14  riordan
 * Save window positions between sessions.
 *
 * Revision 1.12  1993/06/11  00:12:13  rushing
 * second merge from Matt Bretherton sources.
 *
 *
 * Revision 1.11  1993/06/10  18:27:50  rushing
 * added set_index_to_identity to prepare for writing article ranges
 *
 * Revision 1.10  1993/06/08  19:42:17  rushing
 * added support for Sort... menu
 *
 * Revision 1.9  1993/06/01  18:16:44  rushing
 * moved header funcs to headarry.c
 *
 * Revision 1.8  1993/05/29  03:32:30  rushing
 * threading support
 *
 * Revision 1.7  1993/05/28  18:23:23  rushing
 * fixed far/huge problem in find_art_by_subject
 *
 * Revision 1.6  1993/05/24  23:28:04  rushing
 * Header formatting refinements (MRB)
 *
 * Revision 1.5  1993/05/18  22:11:40  rushing
 * smtp support
 *
 * Revision 1.4  1993/05/13  16:17:12  rushing
 * article fetch limit support
 *
 * Revision 1.3  1993/05/01  01:36:56  rushing
 * added IDM_UPDATE
 *
 * Revision 1.2  1993/03/30  20:46:27  DUMOULIN
 * MAPI
 *
 * Revision 1.1  1993/02/16  20:54:42  rushing
 * Initial revision
 *
 *
 */

#ifdef WIN32         
  #define __far far
  #define huge far
  #define       __near near
#endif               

#define ID_ABOUT 100

int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);
BOOL WinVnInit(HANDLE hInstance, LPSTR lpCmdLine);

/* Functions define in Ini-News.c */
int fnLocateFiles (HINSTANCE hInstance, LPSTR lpCmdLine);

/* (SMR) Changed parameter lists to conform with 3.1 windows.h. */
long FAR PASCAL WinVnConfWndProc(HWND, UINT, WPARAM, LPARAM);
long FAR PASCAL WinVnViewWndProc(HWND, UINT, WPARAM, LPARAM);
long FAR PASCAL WinVnArtWndProc(HWND, UINT, WPARAM, LPARAM);

BOOL GetMemo(HWND, HWND, int);
BOOL FAR PASCAL About(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL WinVnCommDlg(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL WinVnSaveArtDlg(HWND hDlg,unsigned  iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnSaveArtsDlg(HWND hDlg,unsigned  iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnFindDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnGenericDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnAuthDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnDoListDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnPersonalInfoDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnMiscDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnAppearanceDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnThresholdDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnLogOptDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);

/* in file WVUSENET.C */
BOOL FAR PASCAL About(HWND, unsigned, WORD, LONG);
VOID CheckView(HWND);
BOOL CrackGroupLine(char *buf,TypLine *lineptr);
BOOL CursorToTextLine(int X,int Y,TypDoc *DocPtr,TypBlock far **BlockPtr,
  TypLine far **LinePtr);
int  ReadNewsrc(void);
void WriteNewsrc(void);
void SetNetDocTitle(void);
void SetLineFlag(TypDoc *Doc, TypBlock far **BlockPtr, TypLine far **LinePtr, int wFlag, int wValue);
void GroupAction(TypDoc *Doc, TypBlock far **BlockPtr, TypLine far **LinePtr, int wFlag, int wValue);
void MakeHelpPathName(char * szFileName, int maxchars);
int cursor_to_char_number (int X, int Y,TypDoc *DocPtr,TypBlock far **BlockPtr,TypLine far **LinePtr);
void SetGroupActiveLines(void);

BOOL WritePrivateProfileInt(char far *lpAppName,char far *lpKeyName,int intval, char far *lpProFile);



long FileLength(HANDLE);
BOOL MoreInit(void);
BOOL MainLoopPass(void);

void DoCommInput(void);
int WinVnDoComm(char *szComm);
char *CommStrtoID(char *CommStr,int *Port, int *Parity,char *szSpeed);
void MoveCursor();
void ViewArticle(TypDoc *Doc, long artindex ,BOOL Reuse, BOOL showArt);
void UnlinkArtsInGroup(TypDoc *GroupDoc);
void UpdateSeenArts(TypDoc *Doc);

int CommIDtoStr(int Port, int Parity,char *szSpeed,char *CommStr);

/* in file WVBLOCK.C */
int NewBlock(TypBlock far *CurBlockPtr, TypBlock far **NewBlockPtr);
void SetupEmptyBlock(TypBlock far *BlockPtr,HANDLE hCur, HANDLE hPrev,
  HANDLE hNext,TypDoc *DocPtr);
BOOL DeleteBlock(TypBlock far **CurBlockPtr, TypLine far **CurLinePtr);
int AddLine(TypLine *LineToAdd,TypBlock far **CurBlockPtr,TypLine far **CurAddPtr);
BOOL ReplaceLine(TypLine *LineToAdd,TypBlock far **CurBlockPtr,TypLine far **CurLinePtr);
BOOL DeleteLine(TypBlock far **CurBlockPtr, TypLine far **CurLinePtr);
int NextLine(TypBlock far **BlockPtr,TypLine far **LinePtr);
int PrevLine(TypBlock far **BlockPtr,TypLine far **LinePtr);
int FindString(BOOL StartAtTop);
int NumBlocksInDoc(TypDoc *Doc);
void ForAllLines(TypDoc *Doc,
   void lpfnFunc(TypDoc *Doc, TypBlock far **BlockPtr, TypLine far **LinePtr, int wFlag, int wValue),
   int wFlag, int wValue);
void ForAllBlocks (TypDoc *Doc, 
   void lpfnFunc(TypDoc *Doc, TypBlock far ** BlockPtr, int wFlag, int wValue),
   int wFlag, int wValue);
void SetForBlock(TypDoc *Doc, TypBlock far ** BlockPtr, int wFlag, int wValue);
BOOL FindLineOrd(TypDoc *Doc,unsigned int LineOrd,TypBlock far **BlockPtr,TypLine  far **LinePtr);
int SearchLine(char *Line,int LineLen,char *Target,int TargLen);
BOOL DoFind(BOOL StartAtTop);
void TopOfDoc(TypDoc *Doc,TypBlock far **BlockPtr,TypLine far **LinePtr);
BOOL ExtractTextLine(TypDoc *Doc,TypLine far *LinePtr,char *Buf,int  BufSize);
BOOL LockLine(HANDLE hBlock,unsigned int LineOff,TypLineID FindLineID,
  TypBlock far **BlockPtr,TypLine far **LinePtr);
void UnlockLine(TypBlock far *BlockPtr,TypLine far *LinePtr,
  HANDLE *hBlock,unsigned int *LineOff,TypLineID *TheLineID);
void PtrToOffset(TypBlock far *BlockPtr,TypLine far *LinePtr,
  HANDLE *hBlock,unsigned int *LineOff,TypLineID *TheLineID);
unsigned int WhatLine(TypBlock far *BlockPtr,TypLine far *LinePtr);
void MoveBytes(void far *Source, void far *Target,int NumBytes);
int InitDoc(TypDoc *Doc,HWND   hWnd,TypDoc *Parent,int DType);
void FreeDoc(TypDoc *Doc);
int mylstrncmp(char far *ptr1,char far *ptr2,int len);
char far *mylstrncpy(char far *ptr1,char far *ptr2, int len);

/* in file WVSCREEN.C */
void ScreenDown(int nLines,int LinesOnScreen,TypBlock far **BlockPtr,
 TypLine far **LinePtr,int *LinesAdvanced);
void ScreenUp(int nLines,TypBlock far **BlockPtr,TypLine far **LinePtr,
 int *LinesBackedUp);
void ScrollIt(TypDoc *Document,WPARAM wParam,LPARAM lParam);
void NewScrollIt(TypDoc *Document,WPARAM wParam,LPARAM lParam);
int LineOnScreen(TypDoc *Doc,HANDLE hTargBlock,unsigned int TargOffset,TypLineID TargLineID);
void NextWindow(TypDoc *Doc);
void AdjustTopSc(TypBlock far *BlockPtr,TypLine far *LinePtr);
void ScreenToTop(TypDoc *Doc);
BOOL AdvanceToActive(TypBlock far **BlockPtr,TypLine far **LinePtr);

HANDLE MRROpenFile(char *FileName,int Mode,TypMRRFile **MRRFile);
void MRRCloseFile(TypMRRFile *MRRFile);
int MRRReadLine(TypMRRFile *MRRFile,char *Linebuf,int Len);
BOOL MRRWriteLine(TypMRRFile *MRRFile,char far *Linebuf,int Len);
BOOL MRRWriteDocument(TypDoc *Document,int Offset,char *szFileName,BOOL Append);

BOOL GetNum(char **ptr,long int *num);
DWORD StrToRGB(char *cstring);
char * StringDate(char * s,time_t time);
void DoCommInput(void);
void DoCommState(void);
BOOL WasArtSeen(unsigned long ArtNum,TypGroup far *GroupPtr);
char chROT13(char achar) ;
void strROT13(char *cstring) ;
void strnROT13(char *cstring, int len) ;
BOOL isLineQuotation(char *textptr);
WndEdit * getWndEdit(WndEdit * WndEdits,HWND hWnd,int numEntries) ;

/* in file wvart.c */
long find_article_by_subject (TypHeader huge *headers,long artindex,long num_headers);
void view_article_by_message_id (TypDoc *Doc, char far *msg_id, long artindex);
void SetArticleRot13Mode(HANDLE hWnd,BOOL RotMode) ;
BOOL GetArticleRot13Mode(HANDLE hWnd);

/* in file wvheader.c */
void ParseAddress(char *headerline,char *addressout,long int addressoutlen,char *nameout,long int nameoutlen);
BOOL GetHeaderLine(TypDoc *Doc,char *Prefix,char *Buf,int  BufLen);
BOOL GetArtHeaderSubject(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL GetArtHeaderTo(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderSubject(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderMessageID(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderReferences(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderDate(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderFrom(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderTo(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderNewsgroups(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderOrganization(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderNewsreader(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderXmailer (TypDoc *Doc,char *Buf,long int BufLen);
BOOL MakeArtBody(TypDoc *Doc,char far **MesBuf,long int  *left, int DocType);
int  CompareStringNoCase(char *str1,char *str2,int slen);
void CopyBuf(char *instr,char **buf,long int *left);
BOOL NextToken(char **cptr);
void AppendTextToEditBuf(char *instr,char far **buf,long int *left);
HWND CreatePostingWnd(HWND hWnd, TypDoc *Doc,int DocType);
BOOL CreatePostingText(TypDoc *Doc,HWND hWndPost,HWND hWndEdit,int DocType);
HANDLE ExtractPostingText (TypDoc *Doc,int DocType);
BOOL FreePostingText ( HANDLE hTextBuffer) ;
BOOL IsLineBlank(char far *line);
BOOL FAR PASCAL editIntercept(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);
void  prepareEditMenu(HWND parWnd,HWND hWndEdit);
int DoEditCommands(HWND hWndEdit,WPARAM wParam,LPARAM lParam);
void DoEditClose(HWND hWnd,int dirty) ;
BOOL WriteEditLog(HWND hWnd, char *fName , char *mBuf);

/* In file WVPOST.C */
long FAR PASCAL WinVnPostWndProc(HWND, UINT, WPARAM, LPARAM);

BOOL StartPost(HWND hWnd);
BOOL PostText(WndEdit * WndPost);

/* In file WVPOST.C */
long FAR PASCAL WinVnMailWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL StartMail(HWND hWnd,int ih);

/* In file WVPRINT.C */      /* JD 930208 */
int FAR PASCAL AbortProc (HDC,WORD);
HDC FAR PASCAL GetPrinterDC(void);
BOOL FAR PASCAL PrintDlgProc(HWND, WORD, WORD, LPARAM);
void FAR PASCAL PrintFile(HWND);
void FAR PASCAL PrintArticle(HWND,TypDoc *Doc);

/* In file WVAUTH.C */
BOOL AuthenticatePosting(void);

int SetupSock(void);
void PutCommLine(char *line,unsigned int nchars);
int MRRReadComm(void);
void MRRCloseComm(void);
int MRRInitComm(void);
int send_smtp_message(char * destination, char * message);
int forward_smtp_message (char * destination, TypDoc * Doc);

/* In file WVLIST.C */
void StartList(void);
void ProcListLine(unsigned char *ListLine);
void AddGroupToTable(char far *GroupLine);
void InitGroupTable(void);
unsigned int HashGroup(unsigned char far *gname);
void HashNetGroups(TypDoc *Doc,TypLine far * far * HashTable);
void ProcEndList(void);
int GroupCompare(TypLine const far * far *g1,TypLine const far * far *g2);
BOOL FAR PASCAL WinVnGroupListDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
void PositionEndSubscribed(TypBlock far **BlockPtr, TypLine far **LinePtr);
void MergeGroups(int WhereSubscribed);
void CleanUpGroupTable(void);

/* In file WVMAIL.C  */
mlPOPSend (HWND hWnd, char *mBuf);
MailClose (HWND hWnd);
void SetMailType (int mailType);
void MailEnableLogout(BOOL enable);
void MailCrashExit(HWND hWnd) ;
int MailInit (HWND hWnd);

/* In file WVMAPI.C */
 mlMAPIInit (HWND hWnd);
 mlMAPILogout (HWND hWnd);
 mlMAPILogon (HWND hWnd);
 mlMAPISend (HWND hWnd,TypDoc *Doc, int DocType);
 mlMAPILogon (HWND hWnd);
 mlMAPIClose (HWND hWnd);

/* in file headarry.c */
header_p header_elt (header_p headers, long index);
header_p lock_headers (HANDLE header_handle,HANDLE thread_handle);
void unlock_headers (HANDLE header_handle, HANDLE thread_handle);
void free_headers (HANDLE header_handle, HANDLE thread_handle);
void sort_by_threads (HANDLE header_handle, HANDLE thread_handle, long length);
void initialize_header_array (HANDLE header_handle, HANDLE thread_handle, long length);
void set_index_to_identity (HANDLE header_handle, HANDLE thread_handle, long length);
void shell_sort_index_array (header_p headers, thread_array index, long nElements,
              int (*compare) (header_p headers, long elem1,long elem2));
int compare_subject (header_p headers, long elem1, long elem2);
int compare_date (header_p headers, long elem1, long elem2);
int compare_lines (header_p headers, long elem1, long elem2);
int compare_from (header_p headers, long elem1, long elem2);
int compare_artnum (header_p headers, long elem1, long elem2);

/* In file WVCRYPT.C */
void MRREncrypt(unsigned char *plainText,int len,char *cipherText);
int MRRDecrypt(char *cipherText,unsigned char *plainText, int plainMax);      

/* In file WVDEBUGC.C */
long FAR PASCAL 
WinVnDebugCommWndProc (HWND hWnd,unsigned message,WPARAM wParam,LPARAM lParam);

#if 0
/* In file FASTQSRT.C */
void __far __cdecl fastQSort(void far *base, size_t n, size_t size, 
   int (__far __cdecl *cmpFn)(const void far *, const void far *));
#endif

void __far ShellSort (void far *,
            size_t, size_t,
            int (*compare)
             (TypLine const far * far *, TypLine const far * far *));

/* void IncPtr(void **ptr,int incval); */

#define     IDM_UNDO             8
#define     IDM_CUT              9
#define     IDM_COPY             10
#define     IDM_PASTE            11
#define     IDM_CLEAR            12
#define     IDM_SELALL           13

#define     IDM_SHOW_SUBSCR        101
#define     IDM_SHOW_ALL_GROUP     102
#define     IDM_VIEW_SEL_GROUP     103
#define     IDM_SEL_SUBSCR         104
#define     IDM_UNSEL_ALL          105
#define     IDM_EXIT               106
#define     IDM_QUIT               107
#define     IDM_GROUP_TOP          108

#define     IDM_SELECTALL          IDM_SELALL
#define     IDM_CLEARALL           IDM_CLEAR

#define     IDM_SUBSCRIBE          130
#define     IDM_UNSUBSCRIBE        131

#define     IDM_CONFIG_LOG         109
#define     IDM_COMMOPTIONS        110
#define     IDM_CONFIG_PERSONAL    111
#define     IDM_CONFIG_APPEARANCE  112
#define     IDM_CONFIG_MISC        113

#define     IDM_NEW_WIN_GROUP      4
#define     IDM_NEW_WIN_ARTICLE    5

#define     IDM_EDIT               114
#define     IDM_EXCLUDE            115
#define     IDM_DELETE             116
#define     IDM_NEW                117

#define     IDM_VIEWNEW            122
#define     IDM_VIEWFULL           123

#define     IDM_SESSION            130
#define     IDM_RESET              140
#define     IDM_SAVE_WINDOW        141

#define     IDM_HELP               150

#define     IDV_APPEND             201
#define     IDV_EDIT               202
#define     IDV_FORGET             203

#define     IDV_NEXT               205

#define     IDV_EXIT               206

#define     IDM_NEXT_ARTICLE       401
#define     IDM_NEXT_SAME_SUBJECT  402
#define     IDM_PREV_ARTICLE       403

#define     IDM_POST             420

#define     IDM_SAVE             410
#define     IDM_SAVEAS           411
#define     IDM_PRINT            412

#define     IDM_FIND             450
#define     IDM_FIND_NEXT_SAME   451
#define     IDM_SEARCH           452
#define     IDM_SEARCH_NEXT      453

#define     IDM_SEND             470
#define     IDM_CANCEL           471

#define     IDM_MAIL             480
#define     IDM_LOGOUT           481
#define     IDM_REPLY            485

#define     IDM_ROT13            487
#define     IDM_UNROT            488

#define     IDM_UPDATE           490
#define     IDM_FORWARD          495

#define  IDM_SORT_DATE     460
#define  IDM_SORT_SUBJECT  461
#define  IDM_SORT_LINES    462
#define  IDM_SORT_THREADS  463
#define  IDM_SORT_ARTNUM      464
#define IDM_SORT_FROM      465

#define IDM_SAVE_SELECTED  466
#define IDM_DESELECT_ALL   467
#define IDM_SELECT_ALL     468
#define IDM_SELECT_MATCH   469
#define IDM_RETRIEVE_COMPLETE 468

#define     ID_TIMER            1000

/* Window word values for child windows */
#define GWW_HWNDEDIT    0
#define GWW_CHANGED     2
#define GWW_WORDWRAP    4
#define GWW_UNTITLED    6
#define CBWNDEXTRA      8

/*-- Dialog box constants -- */
#include "wvdlg.h"
