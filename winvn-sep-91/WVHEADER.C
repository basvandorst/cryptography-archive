/* --- WVHEADER.C ---------------------------------------------
 *
 *  This file contains the code necessary to create the initial skeleton
 *  version of an article, which will be edited by the user.
 *
 *  Mark Riordan   24-JAN-1990
 */


#include "windows.h"
#include "wvglob.h"
#include "winvn.h"
#ifndef MAC
#include "winundoc.h"
#include <ctype.h>
#include <time.h>
#endif


#define MAXHEADERLINE 256


BOOL (*PostHeaderFuncs[])(TypDoc *Doc,char   *Buf,long int    BufLen) = {
   MakeArtHeaderFrom,
   MakeArtHeaderNewsgroups,
   MakeArtHeaderSubject,
/* MakeArtHeaderMessageID, */
/* MakeArtHeaderDate,      */
   MakeArtHeaderReferences,
   MakeArtHeaderOrganization,
   MakeArtHeaderNewsreader,
   NULL
};

BOOL (*MailHeaderFuncs[])(TypDoc *Doc,char   *Buf,long int    BufLen) = {
   MakeArtHeaderTo,
   MakeArtHeaderFrom,
   MakeArtHeaderSubject,
   MakeArtHeaderOrganization,
   NULL
};

/*--- function GetHeaderLine -------------------------------------------
 *
 *  Given a document, get a line from the header portion of that document
 *  whose prefix matches a given prefix.  Prefix = first word in line.
 *  For instance, you might call this routine to say "Get the 'Subject:'"
 *  line from this document.
 *
 *    Entry    Doc      points to the document whose header we are scanning.
 *                      The header is all the lines up to the first blank line.
 *             Prefix   is the character string which will identify the
 *                      line we are seeking.  It is the first word
 *                      (blank-delimited) in a line in the header.
 *             BufLen   is the number of bytes left in the buffer Buf.
 *
 *    Exit     Returns TRUE iff we returned a line.
 *             Buf      if line was returned, contains that line, zero-
 *                      terminated.
 */
BOOL
GetHeaderLine(Doc,Prefix,Buf,BufLen)
TypDoc *Doc;
char *Prefix;
char *Buf;
int  BufLen;
{
   char *bufptr=Buf;
   TypBlock far *BlockPtr;
   TypLine far *LinePtr;
   unsigned int Offset;
   HANDLE hBlock;
   TypLineID MyLineID;
   int found = FALSE;

   TopOfDoc(Doc,&BlockPtr,&LinePtr);
   while(ExtractTextLine(Doc,LinePtr,Buf,BufLen)) {

      /* Is this a blank line signifying the end of the header?      */

      if(IsLineBlank(Buf)) break;

      if(CompareStringNoCase(Buf,Prefix,strlen(Prefix)) == 0) {
         found = TRUE;
         break;
      }
      if(!NextLine(&BlockPtr,&LinePtr)) break;
   }
   UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
   return (found);
}

/*--- function CreatePostingWnd -----------------------------------------
 *
 *    Create the window for composing the text of a posting,
 *    if it's OK to post.
 *
 *    Entry    Doc      points to the document to which we are posting
 *                      a followup--NULL if it's a new posting.
 *
 *    Exit     Returns the handle of the newly-created window
 *              (zero if failure).
 */

HWND
CreatePostingWnd(Doc,DocType)
TypDoc *Doc;
int DocType;
{
   int ih;
   BOOL found = FALSE;
   HWND hWnd = 0;
   int width, AveCharWidth;
   HDC hDC;
#ifndef MAC
   TEXTMETRIC  tmFontInfo;
#endif
   char *TitlePtr, *WndName;

   if(DocType == DOCTYPE_POSTING) {
     WndName = "WinVnPost";
     PostDoc = Doc;
     if(Doc) {
        TitlePtr = "Composing Followup Article";
     } else {
        TitlePtr = "Composing New Article";
     }
     for(ih=0; !found && ih<MAXPOSTWNDS; ih++) {
        if(!hWndPosts[ih]) {
           found = TRUE;
           break;
         }
     }
   } else {
     WndName = "WinVnMail";
     MailDoc = Doc;
     if(Doc) {
        TitlePtr = "Composing Reply Message";
     } else {
        TitlePtr = "Composing New Mail Message";
     }
     for(ih=0; !found && ih<MAXMAILWNDS; ih++) {
        if(!hWndMails[ih]) {
           found = TRUE;
           break;
         }
     }
   }
   if(found && AuthenticatePosting()) {
#ifndef MAC
      hDC = GetDC(hWndConf);
      GetTextMetrics(hDC,(LPTEXTMETRIC) &tmFontInfo);
      AveCharWidth  = tmFontInfo.tmAveCharWidth;
      ReleaseDC(hWndConf,hDC);

      if(xScreen > 78*AveCharWidth) {
         width = 78*AveCharWidth;
      } else {
         width = xScreen - 2*AveCharWidth;
      }
#else
      width = xScreen - 20;
#endif

      hWnd = CreateWindow(WndName,
      TitlePtr,
      WS_OVERLAPPEDWINDOW /* | WS_VSCROLL */ ,
      ih*CharWidth,     /* Initial X pos */
      (int) (yScreen*3/8) + (1+ih)*LineHeight, /* Initial Y pos */
      (int) width,                     /* Initial X Width */
      (int) (yScreen*5/8) - (2*LineHeight),  /* Initial Y height */
      NULL,
      NULL,
      hInst,
      NULL);

      if (hWnd) {
#ifndef MAC
         ShowWindow(hWnd, SW_SHOWNORMAL);
#else
         MyShowWindow(hWnd, SW_SHOWNORMAL);
#endif
         UpdateWindow(hWnd);
      }
   }
   return (hWnd);
}


/*--- function CreatePostingText ------------------------------------------
 *
 *    Create the text of the skeleton article to be edited by
 *    the user before posting.   Display that text in an edit window.
 */
BOOL
CreatePostingText(Doc,hWndPost,hWndEdit,DocType)
TypDoc *Doc;
HWND hWndPost;
HWND hWndEdit;
int DocType;
{
   long int BytesLeft;
   char far *lpTextBuffer;
   char far *lpCurPtr;
   HANDLE hTextBuffer;
   BOOL gotline;
   char line[MAXHEADERLINE];
   int ifunc;
   int nLines;
   long lParam;
   int RefCount;
   BOOL found;
   BOOL (**HeaderFuncs)(TypDoc *Doc,char   *Buf,long int    BufLen);
#ifdef MAC
   long int textLength;
   TEHandle TEH;
   int ih;
   Rect viewRect,destRect;
#endif

   if(DocType == DOCTYPE_POSTING) {
      HeaderFuncs = PostHeaderFuncs;
   } else {
      HeaderFuncs = MailHeaderFuncs;
   }

   /* Compute the number of bytes we need to hold a straight ASCII representation
    * of the initial text of the reply, and allocate a buffer of that size.
    */
   found = 0;
   if(Doc) {
      BytesLeft = (2 + NumBlocksInDoc(Doc)) * Doc->BlockSize;
   } else {
      BytesLeft = 3000;
   }
   if(NULL == (hTextBuffer = GlobalAlloc(GHND, (DWORD)BytesLeft))) {
      MessageBox(hWndPost, "Cannot allocate memory for text","",MB_OK);
      return FALSE;
   }

   lpCurPtr = lpTextBuffer = GlobalLock(hTextBuffer);

   for(ifunc=0; HeaderFuncs[ifunc]; ifunc++) {
      gotline = (HeaderFuncs[ifunc])(Doc,line,(long int)MAXHEADERLINE);
      if(gotline) {
         AppendTextToEditBuf(line,&lpCurPtr,&BytesLeft);
      }
   }
   MakeArtBody(Doc,&lpCurPtr,&BytesLeft,DocType);

   /*  Set the edit window text to this skeleton article.            */

#ifndef MAC
   SetWindowText(hWndEdit,lpTextBuffer);
#else
   viewRect = hWndPost->portRect;
   viewRect.right -= SBarWidth;
   viewRect.bottom -= SBarWidth;
   destRect = viewRect;
   destRect.left = 3;

   SetPort(hWndPost);
   TEH = TENew(&destRect,&viewRect);
   TEHCurrent = TEH;
   (*TEH)->crOnly = -1;
   textLength = lstrlen(lpTextBuffer);
   TESetText(lpTextBuffer,textLength,TEH);
   SetView(hWndPost);
   ShowSelect(hWndPost);
   TEUpdate(&destRect,TEH);

   for(ih=0,found=FALSE; !found && ih<MAXPOSTWNDS; ih++) {
      if(hWndPosts[ih] == hWndPost) {
         TEHPosts[ih] = TEH;
         found = TRUE;
         break;
      }
    }
  /* mrr add here */
#endif
   GlobalUnlock(hTextBuffer);
   GlobalFree(hTextBuffer);
#if 0
            strcpy(mesbuf,"Winc - Viewing ");
            strncat(mesbuf,NewMemos[imemo],MAXMEMONAME);
            SetWindowText(hWnd, mesbuf);
#endif
#ifndef MAC
   nLines = (int) SendMessage(hWndEdit,EM_GETLINECOUNT,0,0L);
   lParam = (long) ((long)nLines);
/* while(nLines--) SendMessage(hWndEdit,WM_KEYDOWN,VK_DOWN,0L); */
#endif
   return TRUE;
}

/*--- functions MakeArtHeaderXXXXX ---------------------------------------
 *
 *   Functions with names of the form MakeArtHeaderXxxxxx are all called
 *   the same way and have similar purposes.  They are called to
 *   create and return a line of text that will become a line in
 *   the header of an article to be posted.  The source of
 *   this information varies; it may be taken from an article
 *   being replied to, it may be taken from a configuration file,
 *   read from the system clock, etc.
 *
 *    Entry    Doc      points to a document containing an article
 *                      being replied to; NULL if we're not replying
 *                      to an article.
 *             BufLen   is the length of the output buffer.
 *
 *    Exit     Returns TRUE iff we returned a line.
 *             Buf      contains the line, if any, terminated by a
 *                      zero byte.
 */

/*--- function MakeArtHeaderSubject -------------------------------------
 *
 *    Returned "Subject:" line is blank if no previous Subject: line
 *    could be found.  Otherwise, it is "Subject: Re: <previous subject>".
 *    If the article being replied to had a subject already starting
 *    with "Re:", the "Re:" is not repeated.
 */

BOOL
MakeArtHeaderSubject(Doc,Buf,BufLen)
TypDoc *Doc;
char   *Buf;
long int    BufLen;
{
   char HeadLine[MAXHEADERLINE];
   BOOL gotline;
   char *outptr = Buf;
   char *headptr = HeadLine;

   CopyBuf("Subject: ",&outptr,&BufLen);
   if(Doc) {
      gotline = GetHeaderLine(Doc,"Subject:",HeadLine,MAXHEADERLINE);
      if(gotline) {
         NextToken(&headptr);
         if(CompareStringNoCase(headptr,"Re:",3) != 0) {
            CopyBuf("Re: ",&outptr,&BufLen);
         }
         CopyBuf(headptr,&outptr,&BufLen);
      }
   }
   return(TRUE);
}

#if 0
/*--- function MakeArtHeaderMessageID ----------------------------------
 *
 *    Returned "Message-ID:" line is computed from the time and
 *    the ServerName (from Windows profile file).  This needs to
 *    be improved.
 */
BOOL
MakeArtHeaderMessageID(Doc,Buf,BufLen)
TypDoc *Doc;
char   *Buf;
long int    BufLen;
{
#define MAXSERVERNAME 32
   char ServerName[MAXSERVERNAME];

   GetProfileString(szAppName,"ServerName","msuinfo.cl.msu.edu",
      ServerName,MAXSERVERNAME);

   sprintf(Buf,"Message-ID: <%ld@%s>",time((time_t *)NULL),ServerName);
   return TRUE;
}
#endif

/*--- function MakeArtHeaderFrom ----------------------------------
 *
 *  Returned "From:" is taken from Windows profile entries.
 */
BOOL
MakeArtHeaderFrom(Doc,Buf,BufLen)
TypDoc *Doc;
char   *Buf;
long int    BufLen;
{
   char *outptr = Buf;

   CopyBuf("From: ",&outptr,&BufLen);
   if(MailAddress[0]) {
      CopyBuf(MailAddress,&outptr,&BufLen);
   } else {
      CopyBuf("<Unknown>",&outptr,&BufLen);
   }
   if(UserName[0]) {
      CopyBuf(" (",&outptr,&BufLen);
      CopyBuf(UserName,&outptr,&BufLen);
      CopyBuf(")",&outptr,&BufLen);
   }

   return TRUE;
}

/*--- function MakeArtHeaderTo ----------------------------------
 *
 *  Simply returns a blank "To:" line.
 */
BOOL
MakeArtHeaderTo(Doc,Buf,BufLen)
TypDoc *Doc;
char   *Buf;
long int    BufLen;
{
   char HeadLine[MAXHEADERLINE];
   char *outptr = Buf, *cptr=HeadLine;
   BOOL gotwho;

   CopyBuf("To: ",&outptr,&BufLen);
   if(Doc) {
      gotwho = GetHeaderLine(Doc,"Reply-To:",HeadLine,MAXHEADERLINE);
      if(!gotwho) {
         gotwho = GetHeaderLine(Doc,"From:",HeadLine,MAXHEADERLINE);
      }
      if(gotwho) {
         NextToken(&cptr);
         while(*cptr && *cptr!=' ') {
            *(outptr++) = *(cptr++);
         }
         *outptr = '\0';
      }
   }

   return TRUE;
}

/*--- function MakeArtHeaderOrganization ----------------------------------
 *
 *    Returned "Organization:" line is gotten from the Windows
 *    profile file.
 */
BOOL
MakeArtHeaderOrganization(Doc,Buf,BufLen)
TypDoc *Doc;
char   *Buf;
long int    BufLen;
{
   char *outptr = Buf;

   if(Organization[0]) {
      CopyBuf("Organization: ",&outptr,&BufLen);
      CopyBuf(Organization,&outptr,&BufLen);
      return (TRUE);
   }

   return FALSE;
}

/*--- function MakeArtHeaderNewsreader ----------------------------------
 *
 *    Returned "Newsreader: " line is simply a constant.
 */
BOOL
MakeArtHeaderNewsreader(Doc,Buf,BufLen)
TypDoc *Doc;
char   *Buf;
long int    BufLen;
{
   char *outptr = Buf;

   CopyBuf("Newsreader: ",&outptr,&BufLen);
   CopyBuf("MS Windows ",&outptr,&BufLen);
   CopyBuf(szAppName,&outptr,&BufLen);
   return (TRUE);

}

#if 0
/*--- function MakeArtHeaderDate ----------------------------------
 *
 *    Returned "Date:" line is computed from operating system time.
 *    Be sure to set the TZ environment variable correctly for
 *    "grtime" to work properly.  Typical setting:  SET TZ=EST5
 */
BOOL
MakeArtHeaderDate(Doc,Buf,BufLen)
TypDoc *Doc;
char   *Buf;
long int    BufLen;
{
   struct tm *timeptr;
   time_t curtime;

   curtime = time((time_t *) NULL);
   timeptr = gmtime(&curtime);
   sprintf(Buf,"Date: %.4d-%.2d-%.2d %.2d:%.2d GMT",timeptr->tm_year+1900,
     timeptr->tm_mon+1,timeptr->tm_mday,timeptr->tm_hour,timeptr->tm_min);
   return TRUE;
}
#endif

/*--- function MakeArtHeaderReferences -----------------------------------
 *
 *  Returned "References:" line contains all the previous article's
 *  References (if any), plus its Message-ID (if any).
 */
BOOL
MakeArtHeaderReferences(Doc,Buf,BufLen)
TypDoc *Doc;
char   *Buf;
long int    BufLen;
{
   char HeadLine[MAXHEADERLINE];
   BOOL gotrefs;
   BOOL gotmesID;
   char *outptr = Buf;
   char *headptr = HeadLine;


   if(Doc) {
      gotrefs = GetHeaderLine(Doc,"References:",HeadLine,MAXHEADERLINE);
      if(gotrefs) {
         CopyBuf(HeadLine,&outptr,&BufLen);
         CopyBuf(" ",&outptr,&BufLen);
      }
      gotmesID = GetHeaderLine(Doc,"Message-ID:",HeadLine,MAXHEADERLINE);
      if(gotmesID) {
         if(!gotrefs) {
            CopyBuf("References: ",&outptr,&BufLen);
         }
         NextToken(&headptr);
         CopyBuf(headptr,&outptr,&BufLen);
      }
      if(gotrefs || gotmesID) return(TRUE);
   }
   return FALSE;
}

/*--- function MakeArtHeaderNewsgroups -----------------------------------
 *
 *  Returned "Newsgroups:" line is simply a copy of previous article's.
 */
BOOL
MakeArtHeaderNewsgroups(Doc,Buf,BufLen)
TypDoc *Doc;
char   *Buf;
long int    BufLen;
{
   char HeadLine[MAXHEADERLINE];
   BOOL gotnews = FALSE;
   char *outptr = Buf;

   if(Doc) {
      gotnews = GetHeaderLine(Doc,"Newsgroups:",HeadLine,MAXHEADERLINE);
      if(gotnews) {
         CopyBuf(HeadLine,&outptr,&BufLen);
      }
   }
   if(!gotnews) {
      CopyBuf("Newsgroups: ",&outptr,&BufLen);
      CopyBuf(NewsgroupsPtr,&outptr,&BufLen);
   }
   return TRUE;
}

/*--- function MakeArtBody ----------------------------------------------
 *
 *    Make the body of the article.  This is null if there's no article
 *    to reply to, else it's text of the form:
 *    In article <Message-ID>, <user> says:
 *    >line 1
 *    >line 2
 *    > .....
 *
 *    Entry    Doc      points to the article being replied to, else
 *                      NULL if none.
 *             left     is the number of bytes left in MesBuf.
 *
 *    Exit     MesBuf   contains the added lines, and has been updated
 *                      to point to just after the last added byte.
 *             left     has been decremented appropriately.
 *             Return value is not used, for now.
 */
BOOL MakeArtBody(Doc,MesBuf,left,DocType)
TypDoc *Doc;
char far **MesBuf;
long int  *left;
int DocType;
{
   HANDLE hBlock;
   unsigned int Offset;
   TypLineID MyLineID;
   TypBlock far *BlockPtr;
   TypLine far *LinePtr;
   char mesID[MAXHEADERLINE];
   char from[MAXHEADERLINE];
   char line[MAXHEADERLINE];
   BOOL gotmesID,gotfrom;
   char *outptr = line;
   char *fromptr=from;
   char *mesptr=mesID;

   AppendTextToEditBuf("",MesBuf,left);  /* blank line after header */

   /* Create the "In message xxx, yyy says:" line.                   */

   if(Doc) {
      gotmesID = GetHeaderLine(Doc,"Message-ID:",mesID,MAXHEADERLINE);
      if(gotmesID) {
         NextToken(&mesptr);
      } else {
         mesptr = "<Unknown>";
      }
      if(DocType == DOCTYPE_POSTING) {
         gotfrom = GetHeaderLine(Doc,"From:",from,MAXHEADERLINE);
         if(gotfrom) {
            NextToken(&fromptr);
         } else {
            fromptr = "<Unknown>";
         }
         sprintf(line,"In article %s, %s says:",mesptr,fromptr);
      } else {
         sprintf(line,"In article %s, you say:",mesptr);
      }
      AppendTextToEditBuf(line,MesBuf,left);

      /* Skip past header of reply document.                            */

      TopOfDoc(Doc,&BlockPtr,&LinePtr);
      while(ExtractTextLine(Doc,LinePtr,line,MAXHEADERLINE) &&
        !IsLineBlank(line) ) {
         if(!NextLine(&BlockPtr,&LinePtr)) break;
      }

      /* Copy body of reply document into the body of this article,  */
      /* prepending a ">" to each line.                              */

      line[0] = '>';
      while(ExtractTextLine(Doc,LinePtr,line+1,MAXHEADERLINE-1)) {
         AppendTextToEditBuf(line,MesBuf,left);
         if(!NextLine(&BlockPtr,&LinePtr)) break;
      }
      UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
   }
   return TRUE;
}


/*--- function CompareStringNoCase -------------------------------------
 *
 *   Compare two strings, case insensitive.
 *
 *    Entry    str1, str2  are two strings to compare.
 *             slen        is the number of characters to compare
 *                         (but stop at a zero byte).
 *
 *    Exit     returns -1 if str1 is earlier in collating sequence,
 *              0 if strings are equal, 1 otherwise
 */
int
CompareStringNoCase(str1,str2,slen)
char *str1;
char *str2;
int slen;
{
   while (slen-- && *str1 && *str2) {
      if(_tolower(*str1) < _tolower(*str2)) {
         return(-1);
      } else if(_tolower(*str1) > _tolower(*str2)) {
         return(1);
      }
      str1++;  str2++;
   }
   return(0);
}

/*--- function CopyBuf -----------------------------------------------
 *
 *    Copy a string into a buffer, being sure not to overrun the
 *    buffer.
 *
 *    Entry    instr    points to the string to copy into the buffer,
 *                      zero-terminated.
 *             buf      is the buffer
 *             left     is the number of bytes left in the buffer
 *    Exit     buf      contains the LWA+1 of the bytes copied into
 *                      the output buffer.  A Zero byte is stored
 *                      into this location.
 *             left     is the number of bytes left in the buffer.
 */

void
CopyBuf(instr,buf,left)
char *instr;
char **buf;
long int      *left;
{
   while (--(*left) > 1 && *instr) {
      *((*buf)++) = *(instr++);
   }
   if(*left > 0) **buf = '\0';
}

/*--- function NextToken -----------------------------------------------
 *
 *  Position a pointer to the next token in a string.
 *  Delimiters are space and tab.
 *
 *    Entry    cptr     points to a position in a zero-terminated string.
 *
 *    Exit     Returns TRUE iff a next token was found.
 *             cptr     points to the next token, if found--else
 *                      it is unchanged.
 */
BOOL
NextToken(cptr)
char **cptr;
{
   /* Skip to end of current token, if any.              */

   while(**cptr!=' ' && **cptr!='\t' && **cptr) (*cptr)++;

   /* Skip past white space.                             */

   while(**cptr && (**cptr==' ' || **cptr=='\t')) (*cptr)++;
   if(**cptr) {
      return(TRUE);
   } else {
      return(FALSE);
   }
}

/*--- function AppendTextToEditBuf ----------------------------------------
 *
 *    Appends a zero-terminated text line to a buffer to be given
 *    to an Edit window.  Used in building messages to be displayed
 *    and edited by an Edit window.
 *
 *    Entry    instr    points to a text line to add.  It is terminated
 *                      by a zero byte and does not end in CR or LF.
 *             left     is the number of characters left in buf.
 *
 *    Exit     buf      contains the line, terminated by CR and LF.
 *                      buf now points to the next available byte.
 *             left     has been decremented as appropriate.
 */
void
AppendTextToEditBuf(instr,buf,left)
char *instr;
char far **buf;
long int      *left;
{
   while (--(*left) > 0 && *instr) {
      *((*buf)++) = *(instr++);
   }
   if(--(*left) > 0) *((*buf)++) = '\r';
#ifndef MAC
   if(--(*left) > 0) *((*buf)++) = '\n';
#endif
   if((*left) > 0)   **buf = '\0';
}

/*--- function IsLineBlank ------------------------------------------------
 *
 *    Determine whether a zero-terminated line is blank.
 *    "Blank" means it contains nothing but spaces and tabs.
 *
 *    Entry    line     points to the line.
 *
 *    Exit     returns TRUE iff the line is blank.
 */
BOOL
IsLineBlank(line)
char far *line;
{
   while(*line==' ' || *line=='\t' || *line=='\n') line++;
   return (!(*line));
}

