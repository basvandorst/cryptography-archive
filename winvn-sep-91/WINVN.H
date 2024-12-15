#define ID_ABOUT 100

int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);
BOOL WinVnInit(HANDLE);
long FAR PASCAL WinVnConfWndProc(HWND, unsigned, WORD, LONG);
long FAR PASCAL WinVnViewWndProc(HWND, unsigned, WORD, LONG);
long FAR PASCAL WinVnArtWndProc(HWND, unsigned, WORD, LONG);
BOOL GetMemo(HWND, HWND, int);
BOOL FAR PASCAL About(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL WinVnCommDlg(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL WinVnSaveArtDlg(HWND hDlg,unsigned  iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnFindDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnAuthDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnDoListDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnPersonalInfoDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnMiscDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnAppearanceDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL WriteProfileInt(char far *lpAppName,char far *lpKeyName,int intval);

void ForAllLines(TypDoc *Doc,
   void lpfnFunc(TypDoc *Doc, TypBlock far **BlockPtr, TypLine far **LinePtr, int wFlag, int wValue),
   int wFlag, int wValue);
void SetLineFlag(TypDoc *Doc, TypBlock far **BlockPtr, TypLine far **LinePtr, int wFlag, int wValue);
void GroupAction(TypDoc *Doc, TypBlock far **BlockPtr, TypLine far **LinePtr, int wFlag, int wValue);


VOID CheckView(HWND);
long FileLength(HANDLE);
BOOL MoreInit(void);
BOOL MainLoopPass(void);
void MakeHelpPathName(char * szFileName, int maxchars);

void DoCommInput(void);
int WinVnDoComm(char *szComm);
char *CommStrtoID(char *CommStr,int *Port, int *Parity,char *szSpeed);
int  ReadNewsrc(void);
void WriteNewsrc(void);
void SetNetDocTitle(void);
void MoveCursor();
BOOL CursorToTextLine(int X,int Y,TypDoc *DocPtr,TypBlock far **BlockPtr,
  TypLine far **LinePtr);
void ViewArticle(TypDoc *Doc,TypBlock far *ArtBlockPtr,TypLine far  *ArtLinePtr,BOOL Reuse);
void UnlinkArtsInGroup(TypDoc *GroupDoc);
void UpdateSeenArts(TypDoc *Doc);

int CommIDtoStr(int Port, int Parity,char *szSpeed,char *CommStr);

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

BOOL CrackGroupLine(char *buf,TypLine *lineptr);

void ScreenDown(int nLines,int LinesOnScreen,TypBlock far **BlockPtr,
 TypLine far **LinePtr,int *LinesAdvanced);
void ScreenUp(int nLines,TypBlock far **BlockPtr,TypLine far **LinePtr,
 int *LinesBackedUp);
void ScrollIt(TypDoc *Document,WORD wParam,DWORD lParam);
int LineOnScreen(TypDoc *Doc,HANDLE hTargBlock,unsigned int TargOffset,TypLineID TargLineID);
void NextWindow(TypDoc *Doc);
void AdjustTopSc(TypBlock far *BlockPtr,TypLine far *LinePtr);
void ScreenToTop(TypDoc *Doc);

HANDLE MRROpenFile(char *FileName,int vRef,int Mode,TypMRRFile **MRRFile);
void MRRCloseFile(TypMRRFile *MRRFile);
int MRRReadLine(TypMRRFile *MRRFile,char *Linebuf,int Len);
BOOL MRRWriteLine(TypMRRFile *MRRFile,char far *Linebuf,int Len);
BOOL MRRWriteDocument(TypDoc *Document,int Offset,char *szFileName,int vRef,BOOL Append);

BOOL GetNum(char **ptr,long int *num);
DWORD StrToRGB(char *cstring);
void DoCommInput(void);
void DoCommState(void);
BOOL WasArtSeen(long int ArtNum,TypGroup far *GroupPtr);

/* in file wvheader.c */
BOOL GetHeaderLine(TypDoc *Doc,char *Prefix,char *Buf,int  BufLen);
BOOL MakeArtHeaderSubject(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderMessageID(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderReferences(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderDate(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderFrom(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderTo(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderNewsgroups(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderOrganization(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtHeaderNewsreader(TypDoc *Doc,char   *Buf,long int    BufLen);
BOOL MakeArtBody(TypDoc *Doc,char far **MesBuf,long int  *left, int DocType);
int  CompareStringNoCase(char *str1,char *str2,int slen);
void CopyBuf(char *instr,char **buf,long int *left);
BOOL NextToken(char **cptr);
void AppendTextToEditBuf(char *instr,char far **buf,long int *left);
HWND CreatePostingWnd(TypDoc *Doc,int DocType);
BOOL CreatePostingText(TypDoc *Doc,HWND hWndPost,HWND hWndEdit,int DocType);
BOOL IsLineBlank(char far *line);

/* In file WVPOST.C */
long FAR PASCAL WinVnPostWndProc(HWND, unsigned, WORD, LONG);
BOOL StartPost(int ih);
BOOL PostText(int ih,int DocType);

/* In file WVPOST.C */
long FAR PASCAL WinVnMailWndProc(HWND, unsigned, WORD, LONG);
BOOL StartMail(int ih);

/* In file WVAUTH.C */
BOOL AuthenticatePosting(void);

int SetupSock(void);
void PutCommLine(char *line,unsigned int nchars);
int MRRReadComm(void);
void MRRCloseComm(void);
int MRRInitComm(void);

/* In file WVLIST.C */
void StartList(void);
void ProcListLine(unsigned char *ListLine);
void AddGroupToTable(char far *GroupLine);
void InitGroupTable(void);
unsigned int HashGroup(unsigned char far *gname);
void HashNetGroups(TypDoc *Doc,TypLine far * far * HashTable);
void ProcEndList(void);
int GroupCompare(TypLine far * far *g1,TypLine far * far *g2);
BOOL FAR PASCAL WinVnGroupListDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
void PositionEndSubscribed(TypBlock far **BlockPtr, TypLine far **LinePtr);
void MergeGroups(int WhereSubscribed);
void CleanUpGroupTable(void);


/* In file FASTQSRT.C */
void _FAR_ _cdecl fastQSort(void far *base, size_t n, size_t size,
   int (_FAR_ _cdecl *cmpFn)(const void far *, const void far *));
/* In file SHELLSOR.C */
void _FAR_ _cdecl ShellSort(void far *base, size_t n, size_t size,
   int (_FAR_ _cdecl *cmpFn)(const void far *, const void far *));


/* void IncPtr(void **ptr,int incval); */

#ifdef MAC
extern TEHandle WndToTEH();
#endif

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

#define     IDM_HELP               150

#define     IDV_APPEND             201
#define     IDV_EDIT               202
#define     IDV_FORGET             203

#define     IDV_CREATE             204
#define     IDV_NEXT               205

#define     IDV_EXIT               206

#define     IDM_NEXT_ARTICLE       401
#define     IDM_NEXT_SAME_SUBJECT  402

#define     IDM_FOLLOWUP         420

#define     IDM_SAVE             410
#define     IDM_SAVEAS           411

#define     IDM_FIND             450
#define     IDM_FIND_NEXT_SAME   451

#define     IDM_POST             470
#define     IDM_CANCEL           471

#define     IDM_MAIL             480
#define     IDM_REPLY            485

/*-- Dialog box constants -- */
#include "wvdlg.h"
