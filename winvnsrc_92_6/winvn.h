/*
 * $Id: winvn.h 1.51 1994/09/18 22:50:26 jcooper Exp $
 */

#ifdef WIN32         
  #define __far far
  #define huge far
  #define       __near near
#endif               

#define ID_ABOUT 100

int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
BOOL WinVnInit(HINSTANCE hInstance, LPSTR lpCmdLine);

/* Functions define in Ini-News.c */
int fnLocateFiles (HINSTANCE hInstance, LPSTR lpCmdLine);

/* (SMR) Changed parameter lists to conform with 3.1 windows.h. */
long WINAPI WinVnConfWndProc(HWND, UINT, WPARAM, LPARAM);
long FAR PASCAL WinVnViewWndProc(HWND, UINT, WPARAM, LPARAM);
long WINAPI WinVnArtWndProc(HWND, UINT, WPARAM, LPARAM);

BOOL GetMemo(HWND, HWND, int);
BOOL WINAPI About(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL WinVnCommDlg(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL WinVnSaveArtDlg(HWND hDlg,unsigned  iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnSaveArtsDlg(HWND hDlg,unsigned  iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnFindDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnGenericDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnSubjectDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnAuthDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnDoListDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnPersonalInfoDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnMiscDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnAppearanceDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnThresholdDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnLogOptDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnDecodeArtsDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnAttachDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnEncodeDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnSigFileDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnExitDlg(HWND hDlg,unsigned iMessage,WORD wParam,LONG lParam);
BOOL FAR PASCAL WinVnMailDlg(HWND hDlg,unsigned iMessage,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK VerListDlgProc(HWND hDlg,UINT iMessage,WPARAM wParam,LPARAM lParam);
BOOL FAR PASCAL WinVnSmartFilerDlg(HWND hDlg,unsigned iMessage,WPARAM wParam,LPARAM lParam);
BOOL FAR PASCAL WinVnSelectPathDlg(HWND hDlg,unsigned iMessage,WPARAM wParam,LPARAM lParam);

/* in file WVUSENET.C */
BOOL FAR PASCAL About(HWND, unsigned, WORD, LONG);
//VOID CheckView(HWND);
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
void SetUserMenus (HWND hWnd, int enable);
void SetConnectMenuItem (HWND hWnd, int enable);

long FileLength(HFILE);
BOOL MoreInit(void);
void InitListFonts ();
void InitArticleFonts ();
void InitPrintFonts ();
void InitStatusFonts ();
void DestroyFonts();
void InitBitmaps();
void DestroyBitmaps();
BOOL MainLoopPass(void);
void Connect ();

void MakeHelpPathName(char * szFileName, int maxchars);
void ReadWinvnProfile();
void WriteWinvnProfile();
void UpdateWinvnProfile();

void DoCommInput(void);
int WinVnDoComm(char *szComm);
char *CommStrtoID(char *CommStr,int *Port, int *Parity,char *szSpeed);
void MoveCursor();
BOOL CursorToTextLine(int X,int Y,TypDoc *DocPtr,TypBlock far **BlockPtr,
  TypLine far **LinePtr);
void ViewArticle(TypDoc *Doc, long artindex ,BOOL Reuse, BOOL showArt, char far *articleID);
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
void HScrollIt(TypDoc *Document,WPARAM wParam,LPARAM lParam);
int LineOnScreen(TypDoc *Doc,HANDLE hTargBlock,unsigned int TargOffset,TypLineID TargLineID);
void NextWindow(TypDoc *Doc);
void AdjustTopSc(TypBlock far *BlockPtr,TypLine far *LinePtr);
void AdjustTopScByDoc (TypDoc *aDoc, unsigned int index);
void ScreenToTop(TypDoc *Doc);
BOOL AdvanceToActive(TypBlock far **BlockPtr,TypLine far **LinePtr);

HFILE MRROpenFile(char *FileName,int Mode,TypMRRFile **MRRFile);
void MRRCloseFile(TypMRRFile *MRRFile);
int MRRReadLine(TypMRRFile *MRRFile,char *Linebuf,int Len);
BOOL MRRWriteLine(TypMRRFile *MRRFile,char far *Linebuf,unsigned int Len);
BOOL MRRWriteDocument(TypDoc *Document,int Offset,char *szFileName,BOOL Append);

BOOL GetNum(char **ptr,long int *num);
COLORREF StrToRGB(char *cstring);
char *RGBToStr(char *buf, COLORREF rgbVal);
char * StringDate(char * s,time_t time);
void DoCommInput(void);
void DoCommState(void);
BOOL WasArtSeen(unsigned long ArtNum,TypGroup far *GroupPtr);
char chROT13(char achar) ;
void strROT13(char *cstring) ;
void strnROT13(char *cstring, int len) ;
BOOL isLineQuotation(char *textptr);
void SetHandleBkBrush(HWND hwnd, HBRUSH handle);

/* more in file wvutil.c */
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
BOOL MakeArtHeaderReplyTo(TypDoc *Doc,char   *Buf,long int    BufLen);
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
BOOL WriteEditLog(HWND hWnd, char *fName , char *mBuf, unsigned int size);

/* In file WVPOST.C */
long FAR PASCAL WinVnPostWndProc(HWND, UINT, WPARAM, LPARAM);

BOOL StartPost(WndEdit * WndPost);
void CompletePost(WndEdit * WndPost, int result);
void FailPost(WndEdit * PostEdit);
BOOL PostText(WndEdit * WndPost);

/* In file WVPOST.C */
long FAR PASCAL WinVnMailWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL StartMail(WndEdit * WndPost);

/* In file WVPRINT.C */      /* JD 930208 */ 
BOOL WINAPI PrinterInit(void);
BOOL WINAPI FreePrinterMemory(void);
BOOL WINAPI DeletePrinterDC(HDC);
HDC WINAPI GetPrinterDC(HWND);
BOOL CALLBACK AbortProc (HDC,int);
DWORD WINAPI PrinterSetup(HWND,DWORD);
//BOOL WINAPI PrintDlgProc(HWND, WORD, WORD, LPARAM); 
LRESULT CALLBACK PrintDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL WINAPI PrintHeaderP(char *str, int limit);
void WINAPI PrintFile(HWND);
void WINAPI PrintArticle(HWND,TypDoc *Doc);
void WINAPI ReportPrintError(int, HWND);

/* In file WVAUTH.C */
BOOL AuthenticatePosting(BOOL AuthReq);
void AddAuthHeader(HWND hWndEdit);


/* in file wvsckgen.c */
int SetupSock(void);
void PutCommLine(char *line);
void PutCommData(char *data, unsigned int length);
int MRRReadComm(void);
void MRRCloseComm(void);
int MRRInitComm(void);
int send_smtp_message(char * destination, char * message);
int forward_smtp_message (char * destination, TypDoc * Doc);
int cc_smtp_message (char * destination, char * editstr);
int post_edit_data (char * edit_data);

/* In file WVLIST.C */
void StartList(void);
void ProcListLine(unsigned char *ListLine);
void AddGroupToTable(char far *GroupLine);
void InitHashTable(unsigned int);
void InitGroupTable(void);
unsigned int HashGroup(unsigned char huge *gname);
void HashNetGroups(TypDoc *Doc,TypLine far * far * HashTable);
void BuildPtrList(void);
void BuildHashTable(void);
void ProcEndList(void);
int GroupCompare(TypLine const huge * huge *g1,TypLine const huge * huge *g2);
BOOL FAR PASCAL WinVnGroupListDlg(HWND hDlg,unsigned iMessage,WPARAM wParam,LPARAM lParam);
void PositionEndSubscribed(TypBlock far **BlockPtr, TypLine far **LinePtr);
void MergeGroups(int WhereSubscribed);
void CleanUpGroupTable(void);
unsigned long FindNextPrime(unsigned long);

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

/* JSC additions to WVUTIL.C */

void RemoveTrailingWhiteSpace (char *str);
BOOL IsBlankStr (char *temp);
BOOL isnumber (char *str);
BOOL AskForFont (HWND hParentWnd, char *face, int *size, char *style);
BOOL AskForColor (HWND hParentWnd, COLORREF *color);
BOOL AskForExistingFileName (HWND hParentWnd, char *fileName, char *title);
char *GetEditText (HWND hWndEdit);
LRESULT SetEditText (HWND hWndEdit, char *editMem);
BOOL CreateEditWnd (WndEdit *NewWnd);
BOOL WritePrivateProfileInt(char far *lpAppName,char far *lpKeyName,int intval, char far *lpProFile);
BOOL WritePrivateProfileUInt (char far *lpAppName,char far *lpKeyName, unsigned int intval, char far *lpProFile);
unsigned int GetPrivateProfileUInt (char far *lpAppName,char far *lpKeyName, unsigned int intval, char far *lpProFile);
void RefreshGroupWnds();
void RefreshArticleWnds();
void RefreshStatusWnds();
void BatchSend(int DocType);
void CloseGroupWnds();
void CloseArticleWnds();
void CloseStatusWnds();
void CascadeWindows();
void AddCommLineToDoc (char *line);

BOOL TestCommBusy (HWND hParentWnd, char *msg);
BOOL TestDecodeBusy (HWND hParentWnd, char *msg);

void SetMainMailMenu(HWND);
void SetArticleMailMenu(HWND);
void SetGroupMailMenu(HWND hWnd);
void UpdateAllMailMenus();

/* in file wvfiler.c (jsc) */
char *GetFileExtension (char *tempName, char *fileName);
char *NameWithoutPath (char *tempName, char *fileName);
char *PathWithoutName (char *tempName, char *fileName);
BOOL AskForNewFileName (HWND hParentWnd, char *fileName, char *startDir, BOOL appendOk);
BOOL AskForFilePath (HWND hParentWnd, char *pathName, char *title);
BOOL VerifyFileName (char *fileName);
BOOL SmartFile (HWND hParentWnd, char *fileName);

/* In file WVCODING.C */
void  DecodeInit ();
void  DecodeDone ();
TypCoded *InitCoded ();
BOOL	DecodeLine (TypCoded *thisDecode, char *line); 
void	DecodeFile (HWND hParentWnd);
void	DecodeDoc (HWND hParentWnd, TypDoc *Document);
int 	Encode (TypTextBlock *textBlock, char *fileName, int mode);
void	EncodeToFile (HWND hParentWnd, char *inFile);
BOOL	CompleteThisDecode ();
int	CreateCodingMap (char *map, char *table);
void	CreateStatusArea (HWND hParentWnd);
void	DestroyStatusArea ();
void	UpdateBlockStatus ();

/* In file WVCODEWN.C */
long FAR PASCAL WinVnCodedBlockWndProc (HWND, UINT, WPARAM, LPARAM);
long FAR PASCAL WinVnCodingWndProc  (HWND, UINT, WPARAM, LPARAM);

/* In file WVTXTBLK.C */
TypTextBlock *InitTextBlock (HWND hParentWnd);
void FreeTextBlock (TypTextBlock *textBlock);
BOOL AddLineToTextBlock (TypTextBlock *textBlock, char *line);
BOOL ReplaceLineInTextBlock (TypTextBlock *textBlock, unsigned long num, char *line);
BOOL AddEndedLineToTextBlock (TypTextBlock *textBlock, char *line, int mode);
void ResetTextBlock ();
BOOL ReadFileToTextBlock (HWND hParentWnd, TypTextBlock *block, char *fileName, int mode);

/* in file WVATTACH.C */
void Attach (WndEdit *WndPost, char *fileName, int DocType);
void ProcessAttach (int cancel);
void FinishAttachment (int abort);
int EncodingTypeToNum (char *str);

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

void
ShellSort (void huge * huge * base,
	   size_t nElements,
	   size_t width,
	   int (*compare) (void const huge *elem1, void const huge * elem2));

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
#define     IDM_CONFIG_SIGFILE     120
#define     IDM_CONFIG_SMART_FILER 121

#define     IDM_NEW_WIN_GROUP      4
#define     IDM_NEW_WIN_ARTICLE    5

#define     IDM_EDIT               114
#define     IDM_EXCLUDE            115
#define     IDM_DELETE             116
#define     IDM_NEW                117
#define      IDM_DECODE_FILE     118
#define      IDM_ENCODE_FILE     119

#define     IDM_VIEWNEW            122
#define     IDM_VIEWFULL           123

#define     IDM_SESSION            130
#define     IDM_RESET              140
#define     IDM_CONNECT            143
#define     IDM_DISCONNECT         144
#define     IDM_SAVE_WINDOW        141
#define     IDM_SAVE_CONFIG        142

#define     IDM_HELP               150

#define     IDV_APPEND             201
#define     IDV_EDIT               202
#define     IDV_FORGET             203

#define     IDV_NEXT               205

#define     IDV_EXIT               206

#define     IDM_NEXT_ARTICLE       401
#define     IDM_NEXT_SAME_SUBJECT  402
#define     IDM_PREV_ARTICLE       403
#define     IDM_DECODE_ARTICLE     404
#define     IDM_ALWAYSONTOP        405

#define     IDM_SAVE             410
#define     IDM_SAVEAS           411
#define     IDM_PRINT            412
#define     IDM_PRINT_SETUP      413

#define     IDM_POST             420
#define     IDM_ATTACH           421

#define     IDM_FIND             450
#define     IDM_FIND_NEXT_SAME   451
#define     IDM_SEARCH           452
#define     IDM_SEARCH_NEXT      453
#define     IDM_FIND_NEXT_UNSEEN 454

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
#define IDM_DECODE_SELECTED   467
#define IDM_DESELECT_ALL   468
#define IDM_SELECT_ALL     469
#define IDM_SELECT_MATCH   470
#define IDM_RETRIEVE_COMPLETE    471
#define IDM_ARTICLE_RETRIEVE_COMPLETE  472

#define IDM_FONT_GROUPLIST      501
#define IDM_FONT_ARTICLE_LIST   502
#define IDM_FONT_ARTICLE_TEXT   503
#define IDM_FONT_PRINT_TEXT     504
#define IDM_FONT_STATUS_TEXT    505

#define IDM_COLOR_UNSUBSCRIBED  510
#define IDM_COLOR_SUBSCRIBED    511
#define IDM_COLOR_UNSEEN        512
#define IDM_COLOR_SEEN          513
#define IDM_COLOR_LIST_BACKGROUND    514
#define IDM_COLOR_ARTICLE_BACKGROUND 515
#define IDM_COLOR_STATUS_BACKGROUND  516
#define IDM_COLOR_ARTICLE_TEXT       517
#define IDM_COLOR_STATUS_TEXT       518

#define IDM_WINDOW_CASCADE      530
#define IDM_CLOSE_ALL_GROUP     531
#define IDM_CLOSE_ALL_ARTICLE   532
#define IDM_CLOSE_ALL_STATUS    533
#define IDM_CLOSE_ALL           534

#define IDM_SEND_ALL_POST       535
#define IDM_SEND_ALL_MAIL       536

#define IDM_MARK_ALL       600

#define     ID_TIMER            1000

/* Window word values for child windows */
#define GWW_HWNDEDIT    0
#define GWW_CHANGED     2
#define GWW_WORDWRAP    4
#define GWW_UNTITLED    6
#define CBWNDEXTRA      8

/* String Table constants */
#define IDS_FILTERSTRING   0

/* path selection bitmap ids, etc*/
#ifndef WIN32
// Private drive types not covered in Win3.x GetDriveType but in Win32
#define DRIVE_CDROM                 5
#define DRIVE_RAM                   6

// We also need this export from Kernel, normally in windows.inc
void FAR PASCAL DOS3Call(void);
#endif

#define IDB_DRIVEMIN                DRIVE_REMOVABLE
#define IDB_DRIVEFLOPPY             DRIVE_REMOVABLE
#define IDB_DRIVEHARD               DRIVE_FIXED
#define IDB_DRIVENETWORK            DRIVE_REMOTE
#define IDB_DRIVECDROM              DRIVE_CDROM
#define IDB_DRIVERAM                DRIVE_RAM
#define IDB_DRIVEMAX                DRIVE_RAM

#define IDB_FOLDERMIN               10
#define IDB_FOLDERCLOSED            10
#define IDB_FOLDEROPEN              11
#define IDB_FOLDEROPENSELECT        12
#define IDB_FOLDERMAX               12


/*-- Dialog box constants -- */
#include "wvdlg.h" 

