head	 1.26;
branch   ;
access   ;
symbols  V80:1.9 V76d:1.1;
locks    ; strict;
comment  @ * @;


1.26
date	 94.09.16.00.42.53;  author jcooper;  state Exp;
branches ;
next	 1.25;

1.25
date	 94.08.24.18.40.15;  author mrr;  state Exp;
branches ;
next	 1.24;

1.24
date	 94.05.27.01.29.29;  author rushing;  state Exp;
branches ;
next	 1.23;

1.23
date	 94.02.24.21.31.49;  author jcoop;  state Exp;
branches ;
next	 1.22;

1.22
date	 94.02.09.18.01.08;  author cnolan;  state Exp;
branches ;
next	 1.21;

1.21
date	 94.01.18.09.51.34;  author jcoop;  state Exp;
branches ;
next	 1.20;

1.20
date	 94.01.15.20.31.16;  author jcoop;  state Exp;
branches ;
next	 1.19;

1.19
date	 93.12.08.01.27.21;  author rushing;  state Exp;
branches ;
next	 1.18;

1.18
date	 93.08.25.18.53.17;  author mbretherton;  state Exp;
branches ;
next	 1.17;

1.17
date	 93.08.17.21.53.06;  author UNKNOWN;  state Exp;
branches ;
next	 1.16;

1.16
date	 93.07.13.22.11.31;  author rushing;  state Exp;
branches ;
next	 1.15;

1.15
date	 93.07.06.21.09.50;  author cnolan;  state Exp;
branches ;
next	 1.14;

1.14
date	 93.06.29.20.06.07;  author rushing;  state Exp;
branches ;
next	 1.13;

1.13
date	 93.06.29.00.22.13;  author rushing;  state Exp;
branches ;
next	 1.12;

1.12
date	 93.06.28.17.51.21;  author rushing;  state Exp;
branches ;
next	 1.11;

1.11
date	 93.06.22.19.08.40;  author rushing;  state Exp;
branches ;
next	 1.10;

1.10
date	 93.06.22.16.46.33;  author bretherton;  state Exp;
branches ;
next	 1.9;

1.9
date	 93.06.11.01.05.17;  author rushing;  state Exp;
branches ;
next	 1.8;

1.8
date	 93.06.11.00.11.26;  author rushing;  state Exp;
branches ;
next	 1.7;

1.7
date	 93.05.24.23.34.12;  author rushing;  state Exp;
branches ;
next	 1.6;

1.6
date	 93.05.24.23.28.34;  author rushing;  state Exp;
branches ;
next	 1.5;

1.5
date	 93.05.18.22.10.45;  author rushing;  state Exp;
branches ;
next	 1.4;

1.4
date	 93.05.06.19.44.10;  author rushing;  state Exp;
branches ;
next	 1.3;

1.3
date	 93.04.30.21.17.01;  author rushing;  state Exp;
branches ;
next	 1.2;

1.2
date	 93.03.30.20.07.37;  author rushing;  state Exp;
branches ;
next	 1.1;

1.1
date	 93.02.16.20.54.22;  author rushing;  state Exp;
branches ;
next	 ;


desc
@winvn version 0.76 placed into RCS
@


1.26
log
@cc-by-mail feature and cleanup for 92.6
@
text
@/* --- WVHEADER.C ---------------------------------------------
 *
 *  This file contains the code necessary to create the initial skeleton
 *  version of an article, which will be edited by the user.
 *
 *  Mark Riordan   24-JAN-1990
 */


/*
 *
 * $Id: wvheader.c 1.25 1994/08/24 18:40:15 mrr Exp $
 * $Log: wvheader.c $
 * Revision 1.25  1994/08/24  18:40:15  mrr
 * authorization enables post/mail
 *
 * Revision 1.24  1994/05/27  01:29:29  rushing
 * unnecessary winundoc.h
 *
 * Revision 1.23  1994/02/24  21:31:49  jcoop
 * jcoop changes
 *
 * Revision 1.22  1994/02/09  18:01:08  cnolan
 * cnolan 90.2 changes
 *
 * Revision 1.21  1994/01/18  09:51:34  jcoop
 * Use new version number scheme on X-Mailer header line
 *
 * Revision 1.20  1994/01/15  20:31:16  jcoop
 * Font/color stuff and signature file handling
 * 
 * Revision 1.19  1993/12/08  01:27:21  rushing
 * new version box and cr lf consistency
 * 
 * Revision 1.18  1993/08/25  18:53:17  mbretherton
 * MRB merge, mail & post logging
 *
 *
 * remove underscore for BC++ compile   Bretherton
 *
 * Revision 1.17  1993/08/17  21:53:06  UNKNOWN
 * Updated version to Version 0.82 (JD)
 *
 * Revision 1.16  1993/07/13  22:11:31  rushing
 * strncpy crashing in parseaddress
 *
 * Revision 1.15  1993/07/06  21:09:50  cnolan
 * win32 support
 *
 * Revision 1.14  1993/06/29  20:06:07  rushing
 * use localtime for Date header
 *
 * Revision 1.13  1993/06/29  00:22:13  rushing
 * correct rfc822/rfc1036 MakeArtHeaderDate function
 *
 * Revision 1.12  1993/06/28  17:51:21  rushing
 * warnings
 *
 * Revision 1.11  1993/06/22  19:08:40  rushing
 * remove warnings
 *
 * Revision 1.10  1993/06/22  16:46:33  bretherton
 * robust parse_address
 *
 * Revision 1.9  1993/06/11  01:05:17  rushing
 * new version number
 *
 * Revision 1.8  1993/06/11  00:11:26  rushing
 * second merge from Matt Bretherton sources
 *
 *
 * Revision 1.7  1993/05/24  23:34:12  rushing
 * further refinements to header manipulation
 * extend reuse of header formatting capabilities
 *
 *
 * Revision 1.5  1993/05/18  22:10:45  rushing
 * smtp support
 *
 * Revision 1.4  1993/05/06  19:44:10  rushing
 * CompareStringNoCase was using _tolower rather than tolower.
 * according to c70 docs, _tolower is undefined for letters that
 * are not uppercase.  Yikes!
 *
 * Revision 1.3  1993/04/30  21:17:01  rushing
 * put the X-Newsreader: line back in.
 *
 * Revision 1.2  1993/03/30  20:07:37  rushing
 * MAPI
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
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#define MAXHEADERLINE 256
int NewDocType;

BOOL (*PostHeaderFuncs[])(TypDoc * Doc, char *Buf, long int BufLen) =
{
    MakeArtHeaderFrom,
    MakeArtHeaderNewsgroups,
    MakeArtHeaderSubject,
/* MakeArtHeaderMessageID, */
/* MakeArtHeaderDate,      */
    MakeArtHeaderReferences,
    MakeArtHeaderOrganization,
    MakeArtHeaderReplyTo,
    MakeArtHeaderNewsreader,
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
GetHeaderLine (Doc, Prefix, Buf, BufLen)
     TypDoc *Doc;
     char *Prefix;
     char *Buf;
     int BufLen;
{
  char *bufptr = Buf;
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
  unsigned int Offset;
  HANDLE hBlock;
  TypLineID MyLineID;
  int found = FALSE;

  TopOfDoc (Doc, &BlockPtr, &LinePtr);
  while (ExtractTextLine (Doc, LinePtr, Buf, BufLen))
    {

      /* Is this a blank line signifying the end of the header?      */

      if (IsLineBlank (Buf))
   break;

      if (CompareStringNoCase (Buf, Prefix, strlen (Prefix)) == 0)
   {
     found = TRUE;
     break;
   }
      if (!NextLine (&BlockPtr, &LinePtr))
   break;
    }
  UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
  return (found);
}

/*--- function CreatePostingWnd -----------------------------------------
 *
 *    Create the window for composing the text of a posting,
 *    if it's OK to post.
 *
 *    Entry    Hwnd     Handle to parnet window
 *             Doc      points to the document to which we are posting
 *                      a followup--NULL if it's a new posting.
 *
 *    Exit     Returns the handle of the newly-created window
 *              (zero if failure).
 */

HWND
CreatePostingWnd (hWnd,Doc, DocType)
     HWND hWnd ;
     TypDoc *Doc;
     int DocType;
{
  int ih;
  BOOL found = FALSE;        
  BOOL AuthReq;
  unsigned int width;
  char *TitlePtr, *WndName;      
  
  if (DocType == DOCTYPE_POSTING)
    {          
      AuthReq = AuthReqPost;
      WndName = "WinVnPost";
      PostDoc = Doc;
      if (Doc)
   {
     TitlePtr = "Composing Followup Article";
   }
      else
   {
     TitlePtr = "Composing New Article";
   }
      for (ih = 0; !found && ih < MAXPOSTWNDS; ih++)
   {
     if (!WndPosts[ih].hWnd)
       {
         found = TRUE;
         break;
       }
   }
    }
  else
    {         
      AuthReq = AuthReqMail;
      WndName = "WinVnMail";
      MailDoc = Doc;
      if (Doc)
   {
     TitlePtr = "Composing Reply Message";
   }
      else
   {
     TitlePtr = "Composing New Mail Message";
     strcpy(AddressString,"") ;
   }
      for (ih = 0; !found && ih < MAXMAILWNDS; ih++)
   {
     if (!WndMails[ih].hWnd)
       {
         found = TRUE;
         break;
       }
   }
    }
  if (found && AuthenticatePosting (AuthReq))
    {

      if (xScreen > 78 * ArtCharWidth)
   {
     width = 78 * ArtCharWidth;
   }
      else
   {
     width = xScreen - 2 * ArtCharWidth;
   }

      hWnd = CreateWindow (WndName,
            TitlePtr,
            WS_OVERLAPPEDWINDOW /* | WS_VSCROLL */ ,
            /* Initial X pos */
            ih * CharWidth,
            /* Initial Y pos */
            (int) (yScreen * 3 / 8) + (1 + ih) * LineHeight,
            /* Initial X Width */
            (int) width,
            /* Initial Y height */
            (int) (yScreen * 5 / 8) - (2 * LineHeight),
            NULL,
            NULL,
            hInst,
            NULL);

      if (hWnd)
   {
     SetHandleBkBrush (hWnd, hArticleBackgroundBrush);        
     ShowWindow (hWnd, SW_SHOWNORMAL);
     UpdateWindow (hWnd);
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
CreatePostingText (Doc, hWndPost, hWndEdit, DocType)
     TypDoc *Doc;
     HWND hWndPost;
     HWND hWndEdit;
     int DocType;
{
  char far *lpTextBuffer;
  HANDLE hTextBuffer;
  int nLines;
  long lParam;

  if (NULL == (hTextBuffer = ExtractPostingText(Doc,DocType)))
    {
      MessageBox (hWndPost, "Cannot allocate memory for text", "", MB_OK);
      return FALSE;
    }
  lpTextBuffer = GlobalLock(hTextBuffer) ;

  /*  Set the edit window text to this skeleton article.            */


  SetWindowText (hWndEdit, lpTextBuffer);
  FreePostingText (hTextBuffer);

  nLines = (int) SendMessage (hWndEdit, EM_GETLINECOUNT, 0, 0L);
  lParam = (long) ((long) nLines);

  return TRUE;
}

/*--------- Extract the Text for the Posting ---------------------------
 *
 *    Entry    Doc      points to the document to which we are posting
 *                      a followup--NULL if it's a new posting. *  
 *             DocType  either build skeletal mail or posting document
 *
 *    Exit      a pointer to the unlocked memory area for skeletal posting
 *
 *  Remember to call FreePosting Text to reclaim the memory allocated to
 *  the posting text 
 */

HANDLE
ExtractPostingText (Doc, DocType)
     TypDoc *Doc;
    int DocType;
{
  long int BytesLeft;
  char far *lpTextBuffer;
  BOOL (**HeaderFuncs) (TypDoc * Doc, char *Buf, long int BufLen);
  char far *lpCurPtr;
  HANDLE hTextBuffer;
  BOOL gotline;
  char line[MAXHEADERLINE];
  int ifunc;

  NewDocType = DocType;
  if (DocType == DOCTYPE_POSTING)
    {
      HeaderFuncs = PostHeaderFuncs;
    }
  else
    {
     HeaderFuncs = MailCtrl.MailHeaderFuncs;
    }

  /* Compute the number of bytes we need to hold a straight ASCII representation
   * of the initial text of the reply, and allocate a buffer of that size.
   */
  if (Doc)
    {
      BytesLeft = (2 + NumBlocksInDoc (Doc)) * Doc->BlockSize;
    }
  else
    {
      BytesLeft = 3000;
    }
 
  if (NULL == (hTextBuffer = GlobalAlloc (GHND, (DWORD) BytesLeft)))
    {
       return ((HANDLE) NULL) ;
    }

  lpCurPtr = lpTextBuffer = GlobalLock (hTextBuffer);
 
  for (ifunc = 0; HeaderFuncs[ifunc]; ifunc++)
    {
     gotline = (HeaderFuncs[ifunc]) (Doc, line, (long int) MAXHEADERLINE);
      if (gotline)
   {
         AppendTextToEditBuf (line, &lpCurPtr, &BytesLeft);
   }
    }
  MakeArtBody (Doc, &lpCurPtr, &BytesLeft, DocType);

  GlobalUnlock (hTextBuffer);
  return(hTextBuffer) ;
  }
/*---------- FreePostingText ---------------------------------------------
 *
 * Free the memory of the skeletal posting text
 *
 */
BOOL
FreePostingText (hTextBuffer)
  HANDLE hTextBuffer;
{
  GlobalUnlock (hTextBuffer);
  GlobalFree (hTextBuffer);
  
  strcpy(AddressString,"") ;
  strcpy(NameString,"") ;
  strcpy(DialogString,"") ;
 
  return(TRUE) ;
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
 *    Returned "Subject:" 
 *
 */

BOOL
MakeArtHeaderSubject (Doc, Buf, BufLen)
     TypDoc *Doc;
     char *Buf;
     long int BufLen;
{
  char *outptr = Buf;
  char from[MAXHEADERLINE];
  char *fromptr;
  
  CopyBuf ("Subject: ", &outptr, &BufLen);
  if (Doc)
    {
      GetArtHeaderSubject (Doc, NULL, NULL);
//    if (!DialogString[0])   // always allow reply-subject confirmation
     
      if (NewDocType == DOCTYPE_POSTING) {
        fromptr = from;	
        if (GetHeaderLine (Doc, "From:", from, MAXHEADERLINE-1))
          {
             NextToken (&fromptr);
             ParseAddress (fromptr, CcAddress, MAXDIALOGSTRING , str, MAXINTERNALLINE ) ;
          } else
              CcAddress[0] = (char) NULL;
      }
      DialogBoxParam (hInst, "WinVnSubject", Doc->hDocWnd, lpfnWinVnSubjectDlg, (LPARAM)(NewDocType == DOCTYPE_POSTING));
    }
  else
    {
      DialogString[0] = (char) NULL;
      CcAddress[0] = (char) NULL;

      DialogBoxParam (hInst, "WinVnSubject", hWndConf, lpfnWinVnSubjectDlg, (LPARAM)(NewDocType == DOCTYPE_POSTING));
    }

  CopyBuf (DialogString, &outptr, &BufLen);
  return (TRUE);
}

/*--- function GetArtHeaderSubject -------------------------------------
 *
 *    Fill the DialogString global variable if a subject can be extracted
 *    of the form  "Subject: Re: <previous subject>".
 *    If the article being replied to had a subject already starting
 *    with "Re:", the "Re:" is not repeated.
 *    returns FALSE as no line assembled
 *    if no previous subject is found SubjestString is ""
 */

GetArtHeaderSubject (Doc, NA, NALen)
     TypDoc *Doc;
     char *NA;
     long int NALen;
{
  char HeadLine[MAXHEADERLINE];
  char *outptr = DialogString ; 
  char *headptr = HeadLine;
  long int BufLen = MAXDIALOGSTRING ;

  *outptr='\0' ;
  if (Doc && ( GetHeaderLine (Doc, "Subject:", HeadLine, MAXHEADERLINE)))
     {
   NextToken (&headptr);
   if (CompareStringNoCase (headptr, "Re:", 3) != 0)
        {
        CopyBuf ("Re: ", &outptr, &BufLen);
         }
   CopyBuf (headptr, &outptr, &BufLen);
     }

   return(FALSE) ;
}
#if 0
/*--- function MakeArtHeaderMessageID ----------------------------------
 *
 *    Returned "Message-ID:" line is computed from the time and
 *    the ServerName (from Windows profile file).  This needs to
 *    be improved.
 */
BOOL
MakeArtHeaderMessageID (Doc, Buf, BufLen)
     TypDoc *Doc;
     char *Buf;
     long int BufLen;
{
#define MAXSERVERNAME 32
  char ServerName[MAXSERVERNAME];

  GetPrivateProfileString (szAppName, "ServerName", "titan.ksc.nasa.gov",
          ServerName, MAXSERVERNAME, szAppProFile);

  sprintf (Buf, "Message-ID: <%ld@@%s>", time ((time_t *) NULL), ServerName);
  return TRUE;
}

#endif

/*--- function MakeArtHeaderFrom ----------------------------------
 *
 *  Returned "From:" is taken from Windows profile entries.
 */
BOOL
MakeArtHeaderFrom (Doc, Buf, BufLen)
     TypDoc *Doc;
     char *Buf;
     long int BufLen;
{
  char *outptr = Buf;

  CopyBuf ("From: ", &outptr, &BufLen);
  if (MailAddress[0])
    {
      CopyBuf (MailAddress, &outptr, &BufLen);
    }
  else
    {
      CopyBuf ("<Unknown>", &outptr, &BufLen);
    }
  if (UserName[0])
    {
      CopyBuf (" (", &outptr, &BufLen);
      CopyBuf (UserName, &outptr, &BufLen);
      CopyBuf (")", &outptr, &BufLen);
    }

  return TRUE;
}

/*--- function MakeArtHeaderReplyTo -------------------------------
 *
 *  Returned "Reply-To:" is taken from Windows profile entries.
 */
BOOL
MakeArtHeaderReplyTo (Doc, Buf, BufLen)
     TypDoc *Doc;
     char *Buf;
     long int BufLen;
{
  char *outptr = Buf;

  if (ReplyTo[0] && strcmp(ReplyTo, MailAddress))
    {
      CopyBuf ("Reply-To: ", &outptr, &BufLen);
      CopyBuf (ReplyTo, &outptr, &BufLen);
      return TRUE;
    }
  return FALSE;
}

/*--- function MakeArtHeaderTo ----------------------------------
 *
 *  Returns a well formed "To:" line or if user selected cancel
 *  returns "To:" (blank) which if not edited will be rejected by
 *  the server.
 */
BOOL
MakeArtHeaderTo (Doc, Buf, BufLen)
     TypDoc *Doc;
     char *Buf;
     long int BufLen;
{
  char HeadLine[MAXHEADERLINE];
  char *outptr = Buf, *cptr = HeadLine;
  GetArtHeaderTo(Doc,NULL,NULL);  

  if (!strcmp(AddressString, "")) {

    DialogString[0]=(char)NULL;
    while (!strcmp(DialogString,""))
      if (!DialogBox (hInst, "WinVnMail", hWndConf, lpfnWinVnMailDlg))
        break;  
        
    sprintf(HeadLine,"To: %s", DialogString);
  }
  else
    sprintf(HeadLine,"To: %s", AddressString);

  CopyBuf (HeadLine, &outptr, &BufLen);
  return TRUE;
}

/*--- function GetArtHeaderTo ----------------------------------
 *
 *    Simply returns contents for a "To:"  Line
 *
 *    Global Variables
 *    NameString and AddressString are extracted from the Document
 *    returns FALSE as no line has been assembled
 */
BOOL
GetArtHeaderTo (Doc, NA, NALen)
     TypDoc *Doc;
     char *NA;
     long int NALen;
{
  char HeadLine[MAXHEADERLINE];
  char *cptr = HeadLine ;
  BOOL gotwho;
  char ReplyAddress[MAXDIALOGSTRING], ReplyName[MAXDIALOGSTRING];

  if (Doc)
    {
      if (GetHeaderLine (Doc, "From:", HeadLine, MAXHEADERLINE))
         {
        NextToken (&cptr);
        ParseAddress (cptr, AddressString,MAXDIALOGSTRING , NameString,MAXDIALOGSTRING ) ;
         }
      if (gotwho = GetHeaderLine (Doc, "Reply-To:", HeadLine, MAXHEADERLINE))
     {
        NextToken (&cptr);
        ParseAddress (cptr, ReplyAddress,MAXDIALOGSTRING , ReplyName,MAXDIALOGSTRING ) ;
     }
      if (gotwho && strcmp (AddressString, ReplyAddress))
         {
            if (AddressString[0] && ConfirmReplyTo)
            { 
               sprintf (str, "Use Reply-To: (%s) instead of\nFrom: (%s) in reply?", 
                        ReplyAddress, AddressString);
                      
               if (MessageBox (Doc->hDocWnd, str, "Confirm Reply-To", 
                               MB_YESNO|MB_ICONQUESTION) == IDNO)
                  gotwho = FALSE;
            }
            if (gotwho)
               strcpy (AddressString, ReplyAddress);
         }
      }

  return FALSE;
}
/*--- function ParseAddress --------------------------
 *   Scan the
 *       address field (null terminated ascii string)
 *  representing any email address and optional
 *       name
 *
 *   and extract
 *   addressout which is the email address
 *   nameout    which is the name (enclosed in parenthises)
 *
 *--------------------------------------------------------*/
void ParseAddress(headerline,addressout,addressoutlen,nameout,nameoutlen)
 char *headerline ;
 char *addressout ;
 long int addressoutlen ;
 char *nameout ;
 long int nameoutlen ;
{ char *ptra = headerline , *ptrb, *ptrc ;
  char *iptr , *optr;
  int  lastspace ;
  long int adlen = addressoutlen - 1 ,nmlen = nameoutlen -1 ;
  long int l;

   nameout[0]=addressout[0]=0 ;

// Note parsing not strictly correct as anything in quotes '"'
// overrides the meaning of lexical constructs '<' and '('

// Seems this works and is robust but is now in the main
// group list read and should be extremely efficient
// suggest a single pass 'stream' parse more appropriate
// but is more work

  if (ptrb=strchr(headerline,'<'))
  {  // address enclosed in brackets  
     l=min(ptrb-ptra-1,nmlen) ;
     if (l < 0) l = 0;     /* this was killing print??? */
     if (l>0)
     {                      // otherwise strncpy crashes
        nmlen-=l ;
        strncpy(nameout,ptra,(int)l) ;
        nameout[l]=0;
     }
     if(  ptrc=strchr(headerline,'>') )
     {
   if (nmlen >0)
        {
      strncpy(nameout+l,ptrc+1,(int)nmlen) ;
      nameout[nameoutlen-1]=0 ;
   }
   l=min(ptrc-ptrb-1,adlen) ;
     } else
     {
   l=adlen ;
     }
     if (l>0)
     {
     strncpy(addressout,ptrb+1,(int)l);
     addressout[l]=0;
     }

  } else
    if (ptrb=strchr(headerline,'('))
  {  // name enclosed in braces
     l=min(ptrb-ptra-1,adlen) ;
     if (l>0)
     {
        adlen-=l ;
   strncpy(addressout,ptra,(int)l) ;
        addressout[l+1]=0;
     }
     if( ptrc=strchr(headerline,')'))
     {
   if (adlen > 0)
        {
      strncpy(addressout+l,ptrc+1,(int)adlen) ;
      addressout[addressoutlen-1]=0 ;
        }
   l=min(ptrc-ptrb-1,nmlen) ;
     } else
     {
   l=nmlen ;
     }
     if (l>0)
     {
     strncpy(nameout,ptrb+1,(int)l);
     nameout[l]=0;
     }      

  } else
  { // just an address
     strncpy(addressout,ptra,(int)adlen) ;
     addressout[addressoutlen-1]=0 ;
  }

// Clean Name   - remove multiple space and quotes
  lastspace=TRUE ;
  for(iptr=optr=nameout ; *iptr ; iptr++)
  {  switch( *iptr)
     {
       case ' ':
    if (!lastspace) *optr++=*iptr ;
    lastspace=TRUE ;
         break;
       case '"':    
    break ;
       default :
    *optr++=*iptr ;
    lastspace=FALSE ;
      }
   }
   *optr=0 ;

// Clean Address
  for(iptr=optr=addressout ; *iptr ; iptr++)
  {  switch( *iptr)
     {
       case ' ':
         break;
       default :
    *optr++=*iptr ;
    lastspace=FALSE ;
      }
   }
   *optr=0 ;

   if (*nameout==0)
   {  // if no name default to address
      strncpy(nameout,addressout,(int)nmlen) ;
      addressout[nameoutlen-1]=0 ;
   }

 }

/*  original code for parsing address
  char *adptr = addressout, *nmptr=nameout, *cptr = headerline;
  char *tptr ;


  if ( *cptr == '(' )
  {                         //  skip Name in brackets 
      tptr =  strchr(headerline,')') ;
      if (tptr != NULL) cptr=tptr ;
      NextToken(&cptr);     // skip parenthesis and white space
  }     
  while (*cptr && *cptr != ' ' && adlen)
  {
      *(adptr++) = *(cptr++);
      adlen-- ;
  }
  *adptr = '\0';

  cptr = strchr(headerline,'(');
  if (cptr != NULL) 
  {
       cptr++;                                  // skip (    
       while (*cptr && *cptr != ')' && nmlen)   // copy to ) 
       {
       if (*cptr != '"')                   // drop quote characters 
            {
          *(nmptr++) = *(cptr);
          nmlen--;
       }
            cptr++ ;
       }
       *nmptr = '\0';
   }
   else
   {
   strncpy( nmptr,addressout,(int)nmlen)  ;
   nmptr[nmlen+1]= '\0' ;
  }

}

*/

/*--- function MakeArtHeaderOrganization ----------------------------------
 *
 *    Returned "Organization:" line is gotten from the Windows
 *    profile file.
 */
BOOL
MakeArtHeaderOrganization (Doc, Buf, BufLen)
     TypDoc *Doc;
     char *Buf;
     long int BufLen;
{
  char *outptr = Buf;

  if (Organization[0])
    {
      CopyBuf ("Organization: ", &outptr, &BufLen);
      CopyBuf (Organization, &outptr, &BufLen);
      return (TRUE);
    }

  return FALSE;
}

/*--- function MakeArtHeaderNewsreader ----------------------------------
 *
 *    Returned "Newsreader: " line is simply a constant.
 */
BOOL
MakeArtHeaderNewsreader (Doc, Buf, BufLen)
     TypDoc *Doc;
     char *Buf;
     long int BufLen;
{
  char *outptr = Buf;

  CopyBuf ("X-Newsreader: ", &outptr, &BufLen);
  CopyBuf (WINVN_VERSION, &outptr, &BufLen);
/* easier than examining this closely */
/*  CopyBuf ("\r", &outptr , &BufLen); */
  return (TRUE);

}

/*--- function MakeArtHeaderXmailer ----------------------------------
 *
 *    Returned "Mailer: " line is simply a constant.
 */
BOOL
MakeArtHeaderXmailer (TypDoc * Doc, char *Buf, long int BufLen)
{
  char *outptr = Buf;

  CopyBuf ("X-Mailer: ", &outptr, &BufLen);
//  CopyBuf ("WinVN version 0.82 SMTP", &outptr, &BufLen);
  CopyBuf (WINVN_VERSION, &outptr, &BufLen);
  return (TRUE);
}


/*--- function MakeArtHeaderDate ----------------------------------
 *
 *    Returned "Date:" line is computed from operating system time.
 *    Be sure to set the TZ environment variable correctly for
 *    "gmtime" to work properly.  Typical setting:  SET TZ=EST5
 */
BOOL
MakeArtHeaderDate (Doc, Buf, BufLen)
     TypDoc *Doc;
     char *Buf;
     long int BufLen;
{
  struct tm *timeptr;
  time_t curtime;
  static char * days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  static char * months[] = { "Jan","Feb","Mar","Apr","May","Jun",
                "Jul","Aug","Sep","Oct","Nov","Dec"};


  if (!getenv("TZ")) {
    MessageBox (hWndConf,
      "Timezone environment variable 'TZ' not set.\n"
      "You must set this variable for your messages\n"
      "to be timestamped correctly.\n"
      "For example, you could add the following to your\n"
      "'autoexec.bat' file:\n"
      "set TZ=EST5EDT\n",
      "Warning:",
      MB_OK|MB_ICONHAND);
  }

  curtime = time ((time_t *) NULL);
  timeptr = localtime (&curtime);
  
  /* rfc1036&rfc822 acceptable format */
  /* Mon, 29 Jun 93 02:15:23 GMT */
  sprintf (Buf, "Date: %s, %.2d %s %.2d %.2d:%.2d:%.2d %s",
      days[timeptr->tm_wday],
      timeptr->tm_mday,
      months[timeptr->tm_mon],
      timeptr->tm_year,
      timeptr->tm_hour,
      timeptr->tm_min,
      timeptr->tm_sec,
      tzname[0]);
  return TRUE;
}


/*--- function MakeArtHeaderReferences -----------------------------------
 *
 *  Returned "References:" line contains all the previous article's
 *  References (if any), plus its Message-ID (if any).
 */
BOOL
MakeArtHeaderReferences (Doc, Buf, BufLen)
     TypDoc *Doc;
     char *Buf;
     long int BufLen;
{
  char HeadLine[MAXHEADERLINE];
  BOOL gotrefs;
  BOOL gotmesID;
  char *outptr = Buf;
  char *headptr = HeadLine;


  if (Doc)
    {
      gotrefs = GetHeaderLine (Doc, "References:", HeadLine, MAXHEADERLINE);
      if (gotrefs)
   {
     CopyBuf (HeadLine, &outptr, &BufLen);
     CopyBuf (" ", &outptr, &BufLen);
   }
      gotmesID = GetHeaderLine (Doc, "Message-ID:", HeadLine, MAXHEADERLINE);
      if (gotmesID)
   {
     if (!gotrefs)
       {
         CopyBuf ("References: ", &outptr, &BufLen);
       }
     NextToken (&headptr);
     CopyBuf (headptr, &outptr, &BufLen);
   }
      if (gotrefs || gotmesID)
   return (TRUE);
    }
  return FALSE;
}

/*--- function MakeArtHeaderNewsgroups -----------------------------------
 *
 *  Returned "Newsgroups:" line is simply a copy of previous article's.
 */
BOOL
MakeArtHeaderNewsgroups (Doc, Buf, BufLen)
     TypDoc *Doc;
     char *Buf;
     long int BufLen;
{
  char HeadLine[MAXHEADERLINE];
  BOOL gotnews = FALSE;
  char *outptr = Buf;

  if (Doc)
    {
      gotnews = GetHeaderLine (Doc, "Newsgroups:", HeadLine, MAXHEADERLINE);
      if (gotnews)
   {
     CopyBuf (HeadLine, &outptr, &BufLen);
   }
    }
  if (!gotnews)
    {
      CopyBuf ("Newsgroups: ", &outptr, &BufLen);
      CopyBuf (NewsgroupsPtr, &outptr, &BufLen);
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
BOOL 
MakeArtBody (Doc, MesBuf, left, DocType)
     TypDoc *Doc;
     char far **MesBuf;
     long int *left;
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
  BOOL gotmesID, gotfrom;
  char *outptr = line;
  char *fromptr = from;
  char *mesptr = mesID;
  register unsigned long i;
  
  AppendTextToEditBuf ("", MesBuf, left); /* blank line after header */

  /* Create the "In message xxx, yyy says:" line.                   */

  if (Doc)
    {
      gotmesID = GetHeaderLine (Doc, "Message-ID:", mesID, MAXHEADERLINE);
      if (gotmesID)
   {
     NextToken (&mesptr);
   }
      else
   {
     mesptr = "<Unknown>";
   }
      switch (DocType) {
      case DOCTYPE_POSTING:
   gotfrom = GetHeaderLine (Doc, "From:", from, MAXHEADERLINE);
   if (gotfrom)
       NextToken (&fromptr);
   else
         fromptr = "<Unknown>";
   sprintf (line, "In article %s, %s says:", mesptr, fromptr);
   break;

      case DOCTYPE_MAIL:
   sprintf (line, "In article %s, you say:", mesptr);
   break;
      }

      AppendTextToEditBuf (line, MesBuf, left);

      /* Skip past header of reply document.                            */

      TopOfDoc (Doc, &BlockPtr, &LinePtr);
      while (ExtractTextLine (Doc, LinePtr, line, MAXHEADERLINE) &&
        !IsLineBlank (line))
   {
     if (!NextLine (&BlockPtr, &LinePtr))
       break;
   }

      /* Copy body of reply document into the body of this article,  */
      /* prepending a ">" to each line */

      /* following line commented after a merge of MRB code (SMR)*/
      /* line[0] = '>'; */

      line[0] = QuoteLineInd ;

      while (ExtractTextLine (Doc, LinePtr, line + 1, MAXHEADERLINE - 1))
   {
     AppendTextToEditBuf (line, MesBuf, left);
     if (!NextLine (&BlockPtr, &LinePtr))
       break;
   }
      UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
      if (EnableSig && Signature->numLines > 0)
        AppendTextToEditBuf ("", MesBuf, left); /* blank line before sig */
    }

  if (EnableSig)
    {
   for (i = 0; i < Signature->numLines; i++)
     AppendTextToEditBuf (TextBlockLine(Signature, i), MesBuf, left);
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
CompareStringNoCase (str1, str2, slen)
     char *str1;
     char *str2;
     int slen;
{
/* bug when either string is zero length mrb */
  if (strlen(str1) < (unsigned) slen)
     {
   return(-1)  ;
      }
  if (strlen(str2) < (unsigned) slen)
     {
       return(+1) ;
     }

  while (slen-- && *str1 && *str2)
    {
      if (tolower (*str1) < tolower (*str2))
   {
     return (-1);
   }
      else if (tolower (*str1) > tolower (*str2))
   {
     return (1);
   }
      str1++;
      str2++;
    }
  return (0);
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
CopyBuf (instr, buf, left)
     char *instr;
     char **buf;
     long int *left;
{
  while (--(*left) > 1 && *instr)
    {
      *((*buf)++) = *(instr++);
    }
  if (*left > 0)
    **buf = '\0';
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
NextToken (cptr)
     char **cptr;
{
  /* Skip to end of current token, if any.              */

  while (**cptr != ' ' && **cptr != '\t' && **cptr)
    (*cptr)++;

  /* Skip past white space.                             */

  while (**cptr && (**cptr == ' ' || **cptr == '\t'))
    (*cptr)++;
  if (**cptr)
    {
      return (TRUE);
    }
  else
    {
      return (FALSE);
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
AppendTextToEditBuf (instr, buf, left)
     char *instr;
     char far **buf;
     long int *left;
{
  while (--(*left) > 0 && *instr)
    {
      *((*buf)++) = *(instr++);
    }
  if (--(*left) > 0)
    *((*buf)++) = '\r';

  if (--(*left) > 0)
    *((*buf)++) = '\n';

  if ((*left) > 0)
    **buf = '\0';
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
IsLineBlank (line)
     char far *line;
{
  while (*line == ' ' || *line == '\t' || *line == '\n')
    line++;
  return (!(*line));
}

/*------------ editIntercept ---------------------------------- 
 *
 *   the subclassed WindowProc for the edit component
 *
 *   Capture key messages and pass them to parent
 * as may contain accelerator information
 *
 *--------------------------------------------------------------*/

BOOL FAR PASCAL  editIntercept(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{ 
  HWND  parWnd ;
  BOOL res ;

  if (msg == WM_KEYDOWN || msg == WM_KEYUP)
    /* if it receives a keyboard event, pass it to your parent */
    {
     parWnd = GetParent(hwnd);
      res =  PostMessage(parWnd,msg, wParam,lParam );
    }
  /* and always let the edit control do its thing  */
  return (BOOL) CallWindowProc((WNDPROC)GetClassLong(hwnd, GCL_WNDPROC),
                hwnd, msg, wParam , lParam);
}

/* --------------- prepareEditMenu -----------------
 *
 *  Common (wvpost and wvmail) preparartion
 *  of edit menus 
 *
 * ------------------------------------------------- 
 */

void  prepareEditMenu(HWND parWnd,HWND hWndEdit) 

{  DWORD dwResult ;
   UINT action;
   HMENU hMenu = GetMenu(parWnd);

   dwResult = SendMessage (hWndEdit, EM_GETSEL, 0, 0L);

   if ( HIWORD (dwResult) == LOWORD (dwResult) )
   {   /* No selection to ROT,Copy or Cut */
       action=MF_GRAYED ;
   } else
   {
       action=MF_ENABLED ;
   }
   EnableMenuItem(hMenu,IDM_ROT13,action) ;
   EnableMenuItem(hMenu,IDM_COPY,action) ;
   EnableMenuItem(hMenu,IDM_CUT,action) ;
   EnableMenuItem(hMenu,IDM_CLEAR,action) ;

   if (IsClipboardFormatAvailable(CF_TEXT))
   { 
      action=MF_ENABLED ;
   } else
   {   /* No Clip board contents to paste */
      action=MF_GRAYED ;
   }
   EnableMenuItem(hMenu,IDM_PASTE,action) ;


  if (SendMessage (hWndEdit, EM_CANUNDO, 0, 0L))
  { 
     action=MF_ENABLED ;
  } else
  {  /* Undo Buffer is empty so cannot Undo changes */
     action=MF_GRAYED ;
  }
  EnableMenuItem(hMenu,IDM_UNDO,action) ;

}

/* ---------------- function DoEditCommands -------------
 *
 *  Command edit commands between wvpost and wvmail
 *
 * ------------------------------------------------------*/
DoEditCommands(HWND hWndEdit,WPARAM wParam,LPARAM lParam)
{
      switch (LOWORD(wParam))
   {
   case IDM_UNDO:
     SendMessage (hWndEdit, WM_UNDO, 0, 0L);
     break;

   case IDM_CUT:
     SendMessage (hWndEdit, WM_CUT, 0, 0L);
     break;

   case IDM_COPY:
     SendMessage (hWndEdit, WM_COPY, 0, 0L);
     break;

   case IDM_PASTE:
     SendMessage (hWndEdit, WM_PASTE, 0, 0L);
     break;

   case IDM_CLEAR:
     SendMessage (hWndEdit, WM_CLEAR, 0, 0L);
     break;

   case IDM_SELALL:
     SendMessage (hWndEdit, EM_SETSEL, 0,
             MAKELONG (0, 32767));
     break;

   case IDM_ROT13:
     { DWORD dwResult ;
       WORD  wStart, wEnd ;
            char *editptr;

       dwResult = SendMessage (hWndEdit, EM_GETSEL, 0, 0L);
       wStart = LOWORD (dwResult) ;
       wEnd   = HIWORD (dwResult) ;

       editptr = GetEditText (hWndEdit);
       strnROT13(editptr + wStart, wEnd - wStart) ;
            SetEditText (hWndEdit, editptr);
            GlobalFreePtr (editptr);
       InvalidateRect (hWndEdit, NULL, TRUE);
     }
     break;

        default :
          return(0) ;   // signnify no action 
   }
   return(1) ;          // signify actions completed 
 }

/*
 *  function DoEditClose
 *  close down the edit window only if clean to close 
 *  or user agrees to lose information 
 *
 */

 void DoEditClose(HWND hWnd,int dirty)
 {
   if ( dirty == DT_DIRTY)
   {
     if (MessageBox(hWnd, "Are you sure you want to exit?", "Unsaved Work",
    MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
    DestroyWindow(hWnd);
   }
   else
      DestroyWindow(hWnd);
 }
@


1.25
log
@authorization enables post/mail
@
text
@d12 1
a12 1
 * $Id: wvheader.c 1.24 1994/05/27 01:29:29 rushing Exp $
d14 3
d97 2
a98 2
#include "windows.h"
#include <windowsx.h>   // for GlobalFreePtr
d101 1
a105 1

d107 1
a108 1

d349 1
d443 3
a445 1

d451 11
a461 1
          DialogBox (hInst, "WinVnSubject", Doc->hDocWnd, lpfnWinVnSubjectDlg);
d466 3
a468 1
      DialogBox (hInst, "WinVnSubject", hWndConf, lpfnWinVnSubjectDlg);
d605 1
a605 1
      if (!DialogBoxParam (hInst, "WinVnMail", hWndConf, lpfnWinVnMailDlg, (LPARAM) NULL))
d1311 1
a1311 1
  return (BOOL) CallWindowProc((FARPROC)GetClassLong(hwnd, GCL_WNDPROC),
@


1.24
log
@unnecessary winundoc.h
@
text
@d12 1
a12 1
 * $Id: wvheader.c 1.23 1994/02/24 21:31:49 jcoop Exp rushing $
d14 3
d95 1
a95 1
#include <windowsx.h>	// for GlobalFreePtr
d161 1
a161 1
	break;
d164 4
a167 4
	{
	  found = TRUE;
	  break;
	}
d169 1
a169 1
	break;
d195 2
a196 1
  BOOL found = FALSE;
d201 2
a202 1
    {
d206 3
a208 3
	{
	  TitlePtr = "Composing Followup Article";
	}
d210 3
a212 3
	{
	  TitlePtr = "Composing New Article";
	}
d214 7
a220 7
	{
	  if (!WndPosts[ih].hWnd)
	    {
	      found = TRUE;
	      break;
	    }
	}
d223 2
a224 1
    {
d228 3
a230 3
	{
	  TitlePtr = "Composing Reply Message";
	}
d232 4
a235 4
	{
	  TitlePtr = "Composing New Mail Message";
	  strcpy(AddressString,"") ;
	}
d237 7
a243 7
	{
	  if (!WndMails[ih].hWnd)
	    {
	      found = TRUE;
	      break;
	    }
	}
d245 1
a245 1
  if (found && AuthenticatePosting ())
d249 3
a251 3
	{
	  width = 78 * ArtCharWidth;
	}
d253 3
a255 3
	{
	  width = xScreen - 2 * ArtCharWidth;
	}
d258 14
a271 14
			   TitlePtr,
			   WS_OVERLAPPEDWINDOW /* | WS_VSCROLL */ ,
			   /* Initial X pos */
			   ih * CharWidth,
			   /* Initial Y pos */
			   (int) (yScreen * 3 / 8) + (1 + ih) * LineHeight,
			   /* Initial X Width */
			   (int) width,
			   /* Initial Y height */
			   (int) (yScreen * 5 / 8) - (2 * LineHeight),
			   NULL,
			   NULL,
			   hInst,
			   NULL);
d274 5
a278 5
	{
	  SetHandleBkBrush (hWnd, hArticleBackgroundBrush);		  
	  ShowWindow (hWnd, SW_SHOWNORMAL);
	  UpdateWindow (hWnd);
	}
d335 1
a335 1
	 int DocType;
d352 1
a352 1
	  HeaderFuncs = MailCtrl.MailHeaderFuncs;
d368 3
a370 3
	 {
	    return ((HANDLE) NULL) ;
	 }
d376 1
a376 1
	  gotline = (HeaderFuncs[ifunc]) (Doc, line, (long int) MAXHEADERLINE);
d378 3
a380 3
	{
			AppendTextToEditBuf (line, &lpCurPtr, &BytesLeft);
	}
d444 1
a444 1
//    if (!DialogString[0])	// always allow reply-subject confirmation
d480 6
a485 6
	NextToken (&headptr);
	if (CompareStringNoCase (headptr, "Re:", 3) != 0)
	     {
		  CopyBuf ("Re: ", &outptr, &BufLen);
	      }
	CopyBuf (headptr, &outptr, &BufLen);
d507 1
a507 1
		    ServerName, MAXSERVERNAME, szAppProFile);
d622 2
a623 2
	     NextToken (&cptr);
	     ParseAddress (cptr, AddressString,MAXDIALOGSTRING , NameString,MAXDIALOGSTRING ) ;
d626 4
a629 4
	  {
	     NextToken (&cptr);
	     ParseAddress (cptr, ReplyAddress,MAXDIALOGSTRING , ReplyName,MAXDIALOGSTRING ) ;
	  }
d651 1
a651 1
 *	 representing any email address and optional
d684 1
a684 1
     if (l < 0) l = 0;		/* this was killing print??? */
d693 1
a693 1
	if (nmlen >0)
d695 4
a698 4
	   strncpy(nameout+l,ptrc+1,(int)nmlen) ;
	   nameout[nameoutlen-1]=0 ;
	}
	l=min(ptrc-ptrb-1,adlen) ;
d701 1
a701 1
	l=adlen ;
d716 1
a716 1
	strncpy(addressout,ptra,(int)l) ;
d721 1
a721 1
	if (adlen > 0)
d723 2
a724 2
	   strncpy(addressout+l,ptrc+1,(int)adlen) ;
	   addressout[addressoutlen-1]=0 ;
d726 1
a726 1
	l=min(ptrc-ptrb-1,nmlen) ;
d729 1
a729 1
	l=nmlen ;
d749 2
a750 2
	 if (!lastspace) *optr++=*iptr ;
	 lastspace=TRUE ;
d752 2
a753 2
       case '"':	  
	 break ;
d755 2
a756 2
	 *optr++=*iptr ;
	 lastspace=FALSE ;
d768 2
a769 2
	 *optr++=*iptr ;
	 lastspace=FALSE ;
d806 1
a806 1
	    if (*cptr != '"')                   // drop quote characters 
d808 3
a810 3
	       *(nmptr++) = *(cptr);
	       nmlen--;
	    }
d817 2
a818 2
	strncpy( nmptr,addressout,(int)nmlen)  ;
	nmptr[nmlen+1]= '\0' ;
d900 1
a900 1
			       "Jul","Aug","Sep","Oct","Nov","Dec"};
d905 8
a912 8
		"Timezone environment variable 'TZ' not set.\n"
		"You must set this variable for your messages\n"
		"to be timestamped correctly.\n"
		"For example, you could add the following to your\n"
		"'autoexec.bat' file:\n"
		"set TZ=EST5EDT\n",
		"Warning:",
		MB_OK|MB_ICONHAND);
d921 8
a928 8
	   days[timeptr->tm_wday],
	   timeptr->tm_mday,
	   months[timeptr->tm_mon],
	   timeptr->tm_year,
	   timeptr->tm_hour,
	   timeptr->tm_min,
	   timeptr->tm_sec,
	   tzname[0]);
d955 4
a958 4
	{
	  CopyBuf (HeadLine, &outptr, &BufLen);
	  CopyBuf (" ", &outptr, &BufLen);
	}
d961 8
a968 8
	{
	  if (!gotrefs)
	    {
	      CopyBuf ("References: ", &outptr, &BufLen);
	    }
	  NextToken (&headptr);
	  CopyBuf (headptr, &outptr, &BufLen);
	}
d970 1
a970 1
	return (TRUE);
d993 3
a995 3
	{
	  CopyBuf (HeadLine, &outptr, &BufLen);
	}
d1044 1
a1044 1
  AppendTextToEditBuf ("", MesBuf, left);	/* blank line after header */
d1052 3
a1054 3
	{
	  NextToken (&mesptr);
	}
d1056 3
a1058 3
	{
	  mesptr = "<Unknown>";
	}
d1061 7
a1067 7
	gotfrom = GetHeaderLine (Doc, "From:", from, MAXHEADERLINE);
	if (gotfrom)
	    NextToken (&fromptr);
	else
	      fromptr = "<Unknown>";
	sprintf (line, "In article %s, %s says:", mesptr, fromptr);
	break;
d1070 2
a1071 2
	sprintf (line, "In article %s, you say:", mesptr);
	break;
d1080 5
a1084 5
	     !IsLineBlank (line))
	{
	  if (!NextLine (&BlockPtr, &LinePtr))
	    break;
	}
d1095 5
a1099 5
	{
	  AppendTextToEditBuf (line, MesBuf, left);
	  if (!NextLine (&BlockPtr, &LinePtr))
	    break;
	}
d1102 1
a1102 1
        AppendTextToEditBuf ("", MesBuf, left);	/* blank line before sig */
d1107 2
a1108 2
  	for (i = 0; i < Signature->numLines; i++)
	  AppendTextToEditBuf (TextBlockLine(Signature, i), MesBuf, left);
d1134 1
a1134 1
	return(-1)  ;
d1137 3
a1139 3
	  {
		 return(+1) ;
	  }
d1144 3
a1146 3
	{
	  return (-1);
	}
d1148 3
a1150 3
	{
	  return (1);
	}
d1289 1
a1289 1
	  parWnd = GetParent(hwnd);
d1294 1
a1294 1
			       hwnd, msg, wParam , lParam);
d1354 29
a1382 29
	{
	case IDM_UNDO:
	  SendMessage (hWndEdit, WM_UNDO, 0, 0L);
	  break;

	case IDM_CUT:
	  SendMessage (hWndEdit, WM_CUT, 0, 0L);
	  break;

	case IDM_COPY:
	  SendMessage (hWndEdit, WM_COPY, 0, 0L);
	  break;

	case IDM_PASTE:
	  SendMessage (hWndEdit, WM_PASTE, 0, 0L);
	  break;

	case IDM_CLEAR:
	  SendMessage (hWndEdit, WM_CLEAR, 0, 0L);
	  break;

	case IDM_SELALL:
	  SendMessage (hWndEdit, EM_SETSEL, 0,
		       MAKELONG (0, 32767));
	  break;

	case IDM_ROT13:
	  { DWORD dwResult ;
	    WORD  wStart, wEnd ;
d1385 3
a1387 3
	    dwResult = SendMessage (hWndEdit, EM_GETSEL, 0, 0L);
	    wStart = LOWORD (dwResult) ;
	    wEnd   = HIWORD (dwResult) ;
d1389 2
a1390 2
	    editptr = GetEditText (hWndEdit);
	    strnROT13(editptr + wStart, wEnd - wStart) ;
d1393 3
a1395 3
	    InvalidateRect (hWndEdit, NULL, TRUE);
	  }
	  break;
d1399 1
a1399 1
	}
d1415 2
a1416 2
	 MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
	 DestroyWindow(hWnd);
@


1.23
log
@jcoop changes
@
text
@d12 1
a12 1
 * $Id: wvheader.c 1.22 1994/02/09 18:01:08 cnolan Exp $
d14 3
a94 1
#include "winundoc.h"
@


1.22
log
@cnolan 90.2 changes
@
text
@d12 1
a12 1
 * $Id: wvheader.c 1.21 1994/01/18 09:51:34 jcoop Exp $
d14 3
d103 1
a103 1
  MakeArtHeaderFrom,
d110 1
a429 2


d436 2
a437 5
      if (!strcmp(DialogString,""))
	{
	  DialogBoxParam (hInst, "WinVnGeneric", Doc->hDocWnd, lpfnWinVnGenericDlg,
			  (LPARAM) (char far *) "Enter a Subject line for your post");
	}
d442 1
a442 2
      DialogBoxParam (hInst, "WinVnGeneric", hWndConf, lpfnWinVnGenericDlg,
		      (LPARAM) (char far *) "Enter a Subject line for your post");
d538 21
d561 3
a563 1
 *  Simply returns a blank "To:" line.
d579 3
a581 3
      DialogBoxParam (hInst, "WinVnGeneric", hWndConf, lpfnWinVnGenericDlg,
		      (LPARAM) (char far *) "Enter mail recipient:");
  
d608 1
d612 6
a617 2
      gotwho = GetHeaderLine (Doc, "Reply-To:", HeadLine, MAXHEADERLINE);
      if (!gotwho)
a618 4
	     gotwho = GetHeaderLine (Doc, "From:", HeadLine, MAXHEADERLINE);
	  }
      if (gotwho)
	 {
d620 1
a620 1
	     ParseAddress (cptr, AddressString,MAXDIALOGSTRING , NameString,MAXDIALOGSTRING ) ;
d622 14
@


1.21
log
@Use new version number scheme on X-Mailer header line
@
text
@d12 1
a12 1
 * $Id: wvheader.c 1.20 1994/01/15 20:31:16 jcoop Exp $
d14 3
d263 1
a263 1
	  SetClassWord (hWnd, GCW_HBRBACKGROUND, hArticleBackgroundBrush);
@


1.20
log
@Font/color stuff and signature file handling
@
text
@d12 1
a12 1
 * $Id: wvheader.c 1.19 1993/12/08 01:27:21 rushing Exp $
d14 3
d823 1
a823 1
 *    Returned "Newsreader: " line is simply a constant.
d831 2
a832 1
  CopyBuf ("WinVN version 0.82 SMTP", &outptr, &BufLen);
@


1.19
log
@new version box and cr lf consistency
@
text
@d12 1
a12 1
 * $Id: wvheader.c 1.18 1993/08/25 18:53:17 mbretherton Exp rushing $
d14 3
d80 1
d181 1
a181 3
  unsigned int width, AveCharWidth;
  HDC hDC;
  TEXTMETRIC tmFontInfo;
d183 1
a183 1

d230 1
a230 6
      hDC = GetDC (hWndConf);
      GetTextMetrics (hDC, (LPTEXTMETRIC) & tmFontInfo);
      AveCharWidth = tmFontInfo.tmAveCharWidth;
      ReleaseDC (hWndConf, hDC);

      if (xScreen > 78 * AveCharWidth)
d232 1
a232 1
	  width = 78 * AveCharWidth;
d236 1
a236 1
	  width = xScreen - 2 * AveCharWidth;
d257 1
a257 1

d991 2
a992 1

d1050 2
d1053 6
a1332 1
	    HANDLE hEditBuffer;
d1338 1
a1338 3
	    hEditBuffer = (HANDLE) SendMessage (hWndEdit, EM_GETHANDLE, 0, 0L);
            editptr = LocalLock (hEditBuffer);

d1340 2
a1341 2

	    LocalUnlock (hEditBuffer);
@


1.18
log
@MRB merge, mail & post logging
@
text
@d9 1
d12 1
a12 1
 * $Id: wvheader.c 1.17 1993/08/17 21:53:06 UNKNOWN Exp $
d14 3
d814 1
a814 1
  CopyBuf ("WinVN version 0.82", &outptr, &BufLen);
@


1.17
log
@Updated version to Version 0.82 (JD)
@
text
@d11 1
a11 1
 * $Id: wvheader.c 1.16 1993/07/13 22:11:31 rushing Exp dumoulin $
d13 6
d192 1
a192 1
	  if (!hWndPosts[ih])
d210 1
d214 1
a214 1
	  if (!hWndMails[ih])
d876 1
a876 1
	   _tzname[0]);
d1343 19
@


1.16
log
@strncpy crashing in parseaddress
@
text
@d1 8
d11 1
a11 1
 * $Id: wvheader.c 1.15 1993/07/06 21:09:50 cnolan Exp rushing $
d13 3
a65 9
/* --- WVHEADER.C ---------------------------------------------
 *
 *  This file contains the code necessary to create the initial skeleton
 *  version of an article, which will be edited by the user.
 *
 *  Mark Riordan   24-JAN-1990
 */


d803 1
a803 1
  CopyBuf ("WinVN version 0.81", &outptr, &BufLen);
d820 1
a820 1
  CopyBuf ("WinVN version 0.81 SMTP", &outptr, &BufLen);
@


1.15
log
@win32 support
@
text
@d3 1
a3 1
 * $Id: wvheader.c 1.14 1993/06/29 20:06:07 rushing Exp $
d5 3
d624 1
@


1.14
log
@use localtime for Date header
@
text
@d3 1
a3 1
 * $Id: wvheader.c 1.13 1993/06/29 00:22:13 rushing Exp rushing $
d5 3
d1215 1
a1215 1
      parWnd =  GetWindowWord(hwnd,GWW_HWNDPARENT);
d1279 1
a1279 1
      switch (wParam)
@


1.13
log
@correct rfc822/rfc1036 MakeArtHeaderDate function
@
text
@d3 1
a3 1
 * $Id: wvheader.c 1.12 1993/06/28 17:51:21 rushing Exp rushing $
d5 3
d64 1
a64 1

d820 1
a820 1
 *    "grtime" to work properly.  Typical setting:  SET TZ=EST5
a829 1

d834 13
d848 1
a848 1
  timeptr = gmtime (&curtime);
d852 1
a852 1
  sprintf (Buf, "Date: %s, %.2d %s %.2d %.2d:%.2d:%.2d GMT",
d859 2
a860 1
	   timeptr->tm_sec);
@


1.12
log
@warnings
@
text
@d3 1
a3 1
 * $Id: wvheader.c 1.11 1993/06/22 19:08:40 rushing Exp rushing $
d5 3
d791 1
a791 1
  CopyBuf ("WinVN version 0.80", &outptr, &BufLen);
d808 1
a808 1
  CopyBuf ("WinVN version 0.80 SMTP", &outptr, &BufLen);
a812 1
#if 0
d828 4
d834 11
a844 2
  sprintf (Buf, "Date: %.4d-%.2d-%.2d %.2d:%.2d GMT", timeptr->tm_year + 1900,
  timeptr->tm_mon + 1, timeptr->tm_mday, timeptr->tm_hour, timeptr->tm_min);
a847 1
#endif
@


1.11
log
@remove warnings
@
text
@d3 1
a3 1
 * $Id: wvheader.c 1.10 1993/06/22 16:46:33 bretherton Exp rushing $
d5 3
d153 1
a153 1
  int width, AveCharWidth;
d409 1
a409 1
			  (LPARAM)"Enter a Subject line for your post");
d416 1
a416 1
		      (LPARAM) "Enter a Subject line for your post");
d531 1
a531 1
		      (LPARAM) "Enter mail recipient:");
d1025 1
a1025 1
  if (strlen(str1) < slen)
d1029 1
a1029 1
  if (strlen(str2) < slen)
d1174 1
a1174 1
BOOL FAR PASCAL  editIntercept(HWND hwnd, unsigned msg, WORD wParam, LONG lParam)
d1176 2
a1177 2
HWND  parWnd ;
BOOL res ;
d1179 9
a1187 10
if (msg == WM_KEYDOWN || msg == WM_KEYUP)
  { //if it receives a keyboard event, pass it to your parent

    parWnd =  GetWindowWord(hwnd,GWW_HWNDPARENT);
    res =  PostMessage(parWnd,msg, wParam,lParam );

  }
// and always let the edit control do its thing 
return CallWindowProc((FARPROC)GetClassLong(hwnd, GCL_WNDPROC), hwnd, msg,
                                  wParam, lParam);
@


1.10
log
@robust parse_address
@
text
@d3 1
a3 1
 * $Id: wvheader.c 1.9 1993/06/11 01:05:17 rushing Exp $
d5 3
a256 1
  BOOL found;
d406 1
a406 1
			  (char far *) "Enter a Subject line for your post");
d413 1
a413 1
		      (char far *) "Enter a Subject line for your post");
a520 1
  BOOL gotwho;
d528 1
a528 1
		      (char far *) "Enter mail recipient:");
d612 1
a612 1
        strncpy(nameout,ptra,l) ;
d619 1
a619 1
	   strncpy(nameout+l,ptrc+1,nmlen) ;
d629 1
a629 1
     strncpy(addressout,ptrb+1,l);
d640 1
a640 1
	strncpy(addressout,ptra,l) ;
d647 1
a647 1
	   strncpy(addressout+l,ptrc+1,adlen) ;
d657 1
a657 1
     strncpy(nameout,ptrb+1,l);
d663 1
a663 1
     strncpy(addressout,ptra,adlen) ;
d700 1
a700 1
      strncpy(nameout,addressout,nmlen) ;
d741 1
a741 1
	strncpy( nmptr,addressout,nmlen)  ;
@


1.9
log
@new version number
@
text
@d3 1
a3 1
 * $Id: wvheader.c 1.8 1993/06/11 00:11:26 rushing Exp rushing $
d5 3
d589 117
a705 1
{
a707 1
  long int adlen = addressoutlen - 1 ,nmlen = nameoutlen -1 ;
d711 1
a711 1
  {                         /* skip Name in brackets */
d714 1
a714 1
      NextToken(&cptr);     /* skip parenthesis and white space */
d726 2
a727 2
       cptr++;                                  /* skip (    */
       while (*cptr && *cptr != ')' && nmlen)   /* copy to ) */
d729 6
a734 2
	    *(nmptr++) = *(cptr++);
	    nmlen--;
d746 1
@


1.8
log
@second merge from Matt Bretherton sources
@
text
@d3 1
a3 1
 * $Id: wvheader.c 1.7 1993/05/24 23:34:12 rushing Exp $
d5 2
d8 1
d661 1
a661 1
  CopyBuf ("WinVN version 0.77", &outptr, &BufLen);
d678 1
a678 1
  CopyBuf ("WinVN version 0.77 SMTP", &outptr, &BufLen);
d1171 1
a1171 1
 }@


1.7
log
@further refinements to header manipulation
extend reuse of header formatting capabilities
@
text
@d3 1
a3 1
 * $Id: wvheader.c 1.5 1993/05/18 22:10:45 rushing Exp $
d6 5
d1035 118
d1154 15
@


1.6
log
@Further refinements to header manipulation
extend reuse of header formatting capabilities
@
text
@@


1.5
log
@smtp support
@
text
@d3 1
a3 1
 * $Id: wvheader.c 1.4 1993/05/06 19:44:10 rushing Exp rushing $
d5 4
d542 1
a542 1
  char *adptr = AddressString, *nmptr=NameString, *cptr = HeadLine;
a544 3
   *adptr = '\0';
   *nmptr = '\0';
  
d554 3
a556 22
	      NextToken (&cptr);
	      while (*cptr && *cptr != ' ')
		 {
		    *(adptr++) = *(cptr++);
		 }
	     *adptr = '\0';

	     cptr = strchr(cptr,'(');
	    if (cptr != NULL) 
	       {
		   cptr++;                             /* skip (    */
		   while (*cptr && *cptr != ')')       /* copy to ) */
		      {
			 *(nmptr++) = *(cptr++);
		      }
		   *nmptr = '\0';
	       }
	    else
	       {
		   strcpy(nmptr,AddressString) ;
	       }
	   }
d561 54
d855 4
a858 1
      line[0] = '>';
@


1.4
log
@CompareStringNoCase was using _tolower rather than tolower.
according to c70 docs, _tolower is undefined for letters that
are not uppercase.  Yikes!
@
text
@d3 1
a3 1
 * $Id: wvheader.c 1.3 1993/04/30 21:17:01 rushing Exp rushing $
d5 5
d199 8
a206 4
			   ih * CharWidth,	/* Initial X pos */
			   (int) (yScreen * 3 / 8) + (1 + ih) * LineHeight,	/* Initial Y pos */
			   (int) width,	/* Initial X Width */
			   (int) (yScreen * 5 / 8) - (2 * LineHeight),	/* Initial Y height */
d341 1
a341 1
  strcpy(SubjectString,"") ;
d386 1
a386 1
	  if (!strcmp(SubjectString,""))
d388 2
a389 1
	  DialogBox (hInst, "WinVnSubject", Doc->hDocWnd, lpfnWinVnSubjectDlg);
d392 1
a392 1
    else
d394 3
a396 1
      DialogBox (hInst, "WinVnSubject", hWndConf, lpfnWinVnSubjectDlg);
d399 1
a399 1
  CopyBuf (SubjectString, &outptr, &BufLen);
d405 1
a405 1
 *    Fill the SubjectString global variable if a subject can be extracted
d419 1
a419 1
  char *outptr = SubjectString ; 
d421 1
a421 1
  long int BufLen = MAXSUBJECTSTRING ;
d505 1
d507 11
a517 3
  CopyBuf ("To: ", &outptr, &BufLen);
  GetArtHeaderTo(Doc,NULL,NULL);  
  CopyBuf(AddressString,&outptr, &BufLen);
d519 1
d624 15
d789 6
a794 9
      if (DocType == DOCTYPE_POSTING)
	{
	  gotfrom = GetHeaderLine (Doc, "From:", from, MAXHEADERLINE);
	  if (gotfrom)
	    {
	      NextToken (&fromptr);
	    }
	  else
	    {
d796 8
a803 7
	    }
	  sprintf (line, "In article %s, %s says:", mesptr, fromptr);
	}
      else
	{
	  sprintf (line, "In article %s, you say:", mesptr);
	}
d817 1
a817 1
      /* prepending a ">" to each line.                              */
d820 1
@


1.3
log
@put the X-Newsreader: line back in.
@
text
@d3 1
a3 1
 * $Id: wvheader.c 1.2 1993/03/30 20:07:37 rushing Exp rushing $
d5 3
d826 1
a826 1
      if (_tolower (*str1) < _tolower (*str2))
d830 1
a830 1
      else if (_tolower (*str1) > _tolower (*str2))
@


1.2
log
@MAPI
@
text
@d3 5
a7 2
 * $Id: wvheader.c%v 1.1 1993/02/16 20:54:22 rushing Exp $
 * $Log: wvheader.c%v $
d44 1
a44 1
/*    MakeArtHeaderNewsreader, */
d591 2
a592 3
  CopyBuf ("Newsreader: ", &outptr, &BufLen);
  CopyBuf ("MS Windows ", &outptr, &BufLen);
  CopyBuf (szAppName, &outptr, &BufLen);
d594 1
a594 1
  CopyBuf ("\r", &outptr , &BufLen);
@


1.1
log
@Initial revision
@
text
@d3 4
a6 2
 * $Id$
 * $Log$
d8 1
a44 8
BOOL (*MailHeaderFuncs[])(TypDoc * Doc, char *Buf, long int BufLen) =
{
  MakeArtHeaderTo,
    MakeArtHeaderFrom,
    MakeArtHeaderSubject,
    MakeArtHeaderOrganization,
    NULL
};
d105 2
a106 1
 *    Entry    Doc      points to the document to which we are posting
d114 2
a115 1
CreatePostingWnd (Doc, DocType)
a120 1
  HWND hWnd = 0;
d124 1
a124 1
  char *TitlePtr, *WndName;
d220 42
d264 1
a269 4
  int nLines;
  long lParam;
  BOOL found;
  BOOL (**HeaderFuncs) (TypDoc * Doc, char *Buf, long int BufLen);
d277 1
a277 1
      HeaderFuncs = MailHeaderFuncs;
a282 1
  found = 0;
d291 1
d293 3
a295 4
    {
      MessageBox (hWndPost, "Cannot allocate memory for text", "", MB_OK);
      return FALSE;
    }
d298 1
a298 1

d301 1
a301 1
      gotline = (HeaderFuncs[ifunc]) (Doc, line, (long int) MAXHEADERLINE);
d304 1
a304 1
	  AppendTextToEditBuf (line, &lpCurPtr, &BytesLeft);
d309 12
a320 4
  /*  Set the edit window text to this skeleton article.            */


  SetWindowText (hWndEdit, lpTextBuffer);
d323 6
a328 5

  nLines = (int) SendMessage (hWndEdit, EM_GETLINECOUNT, 0, 0L);
  lParam = (long) ((long) nLines);

  return TRUE;
d353 2
a354 4
 *    Returned "Subject:" line is blank if no previous Subject: line
 *    could be found.  Otherwise, it is "Subject: Re: <previous subject>".
 *    If the article being replied to had a subject already starting
 *    with "Re:", the "Re:" is not repeated.
d363 2
a364 2
  char HeadLine[MAXHEADERLINE];
  BOOL gotline;
a365 1
  char *headptr = HeadLine;
d370 2
a371 11
      gotline = GetHeaderLine (Doc, "Subject:", HeadLine, MAXHEADERLINE);
      if (gotline)
	{
	  NextToken (&headptr);
	  if (CompareStringNoCase (headptr, "Re:", 3) != 0)
	    {
	      CopyBuf ("Re: ", &outptr, &BufLen);
	    }
	  CopyBuf (headptr, &outptr, &BufLen);
	}
	else
a373 1
	  CopyBuf (SubjectString, &outptr, &BufLen);
a378 1
      CopyBuf (SubjectString, &outptr, &BufLen);
d380 2
d385 33
d489 27
d520 3
a522 3
	{
	  gotwho = GetHeaderLine (Doc, "From:", HeadLine, MAXHEADERLINE);
	}
d524 24
a547 9
	{
	  NextToken (&cptr);
	  while (*cptr && *cptr != ' ')
	    {
	      *(outptr++) = *(cptr++);
	    }
	  *outptr = '\0';
	}
    }
d549 1
a549 1
  return TRUE;
d552 1
d809 10
@
