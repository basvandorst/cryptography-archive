
/*
 *
 * $Id: wvutil.c 1.33 1994/01/05 22:36:05 rushing Exp $
 * $Log: wvutil.c $
 * Revision 1.33  1994/01/05  22:36:05  rushing
 * off by one bug in PutCommLine ("XOVER\r", 5) fixed
 * l.
 *
 * Revision 1.32  1993/12/08  01:28:01  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.31  1993/12/01  23:24:06  rushing
 * off by one on date problem shows up dec. 1.
 *
 * Revision 1.30  1993/11/24  23:07:20  rushing
 * let the user know about access trouble
 *
 * Revision 1.29  1993/11/22  20:30:50  rushing
 * handle mixed-case dates
 *
 * Revision 1.28  1993/08/25  18:53:17  mbretherton
 * MRB merge, mail & post logging
 *
 * Revision 1.27  1993/08/25  17:05:17  mbretherton
 * merge from first newsrc.zip
 *
 * Revision 1.26  1993/08/18  21:49:21  rushing
 * more 16-bit article number fixes.
 *
 * Revision 1.25  1993/08/05  20:06:07  jcoop
 * save multiple articles changes by jcoop@oakb2s01.apl.com (John S Cooper)
 *
 * Revision 1.24  1993/07/13  16:03:04  riordan
 * MRR mods
 *
 * Revision 1.23  1993/06/28  17:52:17  rushing
 * fixed compiler warnings
 *
 * Revision 1.22  1993/06/25  20:18:51  dumoulin
 * Changed Date fields from Strings to Posix standard time format
 *
 * Revision 1.21  1993/06/22  16:43:03  bretherton
 * auto close on posting window
 *
 *
 * Revision 1.20  1993/06/22  14:04:05  dumoulin
 * Fixed problems with article numbers larger than 32K
 *
 * Revision 1.19  1993/06/19  18:40:17  dumoulin
 * Added error routines to handle unsyncd XHDRS from screwed up servers
 *
 * Revision 1.18  1993/06/15  21:16:59  rushing
 * headers-> to header-> in two lines.
 *
 * Revision 1.17  1993/06/11  00:10:35  rushing
 * second merge from Matt Bretherton sources
 *
 * Revision 1.16  1993/06/10  18:25:17  rushing
 * XOVER support.
 *
 * Revision 1.15  1993/06/08  19:42:38  rushing
 * always do the thread_index array so we can sort on other fields.
 *
 * Revision 1.14  1993/06/05  03:18:25  rushing
 * primitive functional threading.
 *
 * Revision 1.13  1993/06/01  18:22:40  rushing
 * moved header funcs to headarry.c
 *
 * Revision 1.12  1993/05/26  23:59:40  rushing
 * reimplemented positioning group window near last read article
 *
 * Revision 1.11  1993/05/24  23:56:51  rushing
 * date formatting for group list window
 * and save header size for article repaing (MRB)
 *
 * Revision 1.10  1993/05/13  19:58:09  SOMEONE
 * fancy 'from' in group window
 *
 * Revision 1.9  1993/05/13  16:16:06  rushing
 * article fetch limit support
 *
 * Revision 1.8  1993/05/08  17:11:05  rushing
 * correct handling of 4XX response codes, and
 * of NNTP data lines beginning with '.'
 *
 * Revision 1.7  1993/04/29  21:42:11  rushing
 * checked for null from parse_usenet_date on bad date.
 *
 * Revision 1.6  1993/04/29  20:24:59  rushing
 * attempted support for WSAAsyncSelect
 *
 * Revision 1.5  1993/04/27  21:03:20  rushing
 * clicking on a group that doesn't exist is correctly handled.
 * the prematurely created group window is destroyed correctly
 * after telling the user it doesn't exist.
 *
 * Revision 1.4  1993/04/23  22:10:48  rushing
 * added function check_server_code and used it
 * to watch for server and other errors, rather
 * than just ignoring and crashing 8^)
 *
 * Revision 1.3  1993/02/18  20:34:16  rushing
 * set CommState to ST_NONE after closing connection.
 *
 * Revision 1.2  1993/02/16  21:20:28  rushing
 * Added a function (initialize_header_array) to ensure
 * valid data in the header array in case a WM_PAINT gets
 * sent to the group window before the retrieval is finished.
 *
 * Revision 1.1  1993/02/16  20:54:22  rushing
 * Initial revision
 *
 *
 */

/*-- WVUTIL.C -- File containing utility routines.
 */

#include "windows.h"
#include "wvglob.h"
#include "winvn.h"
#include "winundoc.h"
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

char far *mylstrcpy(char_p ptr1, char far *ptr2);
char * get_xhdr_line (char * line);
time_t parse_usenet_date (char * date);
void finish_header_retrieval ();

// please update this if you modify XHDR retrieval
// This will now be either 4 or 6, depending on whether threading
// is enabled via 'threadp'.
unsigned int total_xhdrs = 4;

/*--- function GetNum --------------------------------------------
 *
 *  Cracks off a positive integer number from a string.
 *
 *  Entry    *ptr  is the character position to start scanning
 *                 for an integer
 *
 *  Exit     *ptr  is the character position at which we stopped
 *                 scanning (because of a non-digit).
 *           *num  is the cracked off number.
 *           Returns TRUE iff we got a number.
 */
BOOL
GetNum(ptr,num)
char **ptr;
long int *num;
{
   BOOL gotit = FALSE;

   /* Skip initial spaces                                            */

   while((**ptr) && **ptr == ' ') (*ptr)++;

   *num = 0;
   while(**ptr && isdigit(**ptr)) {
      *num = 10*(*num) + (**ptr - '0');
      gotit = TRUE;
      (*ptr)++;
   }
   return(gotit);
}

char * get_xhdr_line (char *line)
{
  char * cptr;
/* skip past the art # and space */
  for(cptr=line; isdigit(*cptr); cptr++);
  for(;*cptr == ' ';cptr++);
  return (cptr);
}

#if 0  
MRB already did this
void make_neat_from (char far *in, char far *out)
{
  char far * left, far *right;
  
  /* this is controlled from .ini */
  if (FullNameFrom) {
    left = strchr (in, '(');
    right = strrchr (in, ')');

    if ((left && right) && (left < right)) {
      strncpy (out, left + 1, (size_t) (right - left - 1));
      out[(right - left - 1)] = (char) 0;
    } 
    else                        /* No name in parens */
      strcpy (out, in);
  }
  else                          /* !FullNameFrom */
    strcpy (out,in);
}
#endif

/*-- function StrToRGB -------------------------------------------------
 *
 *  Takes an ASCII string of the form "r,g,b" where r, g, and b are
 *  decimal ASCII numbers, and converts it to an RGB color number.
 */
DWORD
StrToRGB(cstring)
char *cstring;
{
   BYTE red, green, blue;
   long int lred, lgreen, lblue;

   GetNum(&cstring,&lred);
   cstring++;
   GetNum(&cstring,&lgreen);
   cstring++;
   GetNum(&cstring,&lblue);
   red = (BYTE) lred; green = (BYTE) lgreen; blue = (BYTE) lblue;

   return(RGB(red,green,blue));
}

/* This was lifted from ANU news. */

time_t
parse_usenet_date(char * s)
{
  struct tm datetime;
  char *cp, mon[80];
  int dom = 0, yr = 0, hr = 0, mn = 0, sc = 0, mth=0 ;
  static char fmtMonthTable[37] = "janfebmaraprmayjunjulaugsepoctnovdec";

  if (!s || !*s) return(0);
  if (cp = strchr(s,',')) s = ++cp;
  while (isspace(*s)) s++;
  *mon = '\0';
  if (isdigit(*s)) {
    sscanf(s,"%d %s %d %d:%d:%d",&dom,mon,&yr,&hr,&mn,&sc);
    if (yr < 100 ) yr += 1900;
    }
  else sscanf(s,"%*s %s %d %d:%d:%d %d",mon,&dom,&hr,&mn,&sc,&yr);
  
  if (!dom || !yr || !*(cp = mon)) return(0);
  if ((dom <= 0) || (dom >= 32)) return(0);
  if ((yr < 1980) || (yr > 2020)) return(0);
  if (strlen(mon) > 10) return(0);
  if ((hr < 0) || (hr > 23)) return(0);
  if ((mn < 0) || (mn > 59)) return(0);
  if ((sc < 0) || (sc > 59)) return(0);  
  
  for (cp=mon; *cp; cp++)
    *cp=tolower(*cp);

  if (cp=strstr(fmtMonthTable,mon)) mth=(cp-fmtMonthTable)/3;

/*  Setup a Posix time structure and calculate time in absolute
    time (seconds since midnight, Jan 1, 1970    JD 06/25/93 */

    datetime.tm_year = yr - 1900;
    datetime.tm_mon = mth;
    datetime.tm_mday = dom;
    datetime.tm_hour = hr;
    datetime.tm_min = mn; 
    datetime.tm_sec = sc;

    return (mktime(&datetime));
}

/*-- function StringDate ----------------*/
char *
StringDate(char * s,time_t time)

{
  struct tm *datetime;
  if (time != 0)
    {
     datetime = localtime(&time);

     if (fmtDaysB4Mth)
      {
       sprintf(s,"%02d%s%02d",datetime->tm_mday,fmtDateDelim,datetime->tm_mon+1);
      } else
       { 
   sprintf(s,"%02d%s%02d",datetime->tm_mon+1,fmtDateDelim,datetime->tm_mday);
       }
    return(s);
    }
  else
    return("-----");
}

/*-- function DoCommInput ---------------------------------------
 *
 *
 */
void
DoCommInput()
{
   int ch;

   while((CommState != ST_CLOSED_COMM) && (ch = MRRReadComm()) >= 0) {
      if(ch == IgnoreCommCh) {
      } else if(ch == EOLCommCh) {
    *CommLinePtr = '\0';
    DoCommState();
    CommLinePtr = CommLineIn;
      } else {
    *(CommLinePtr++) = (char) ch;
    if(CommLinePtr == CommLineLWAp1) CommLinePtr--;
      }
   }
}

void
update_window_title (HWND hwnd,
          char * group_name,
          unsigned long line_number,
          unsigned long total_lines)
{
  char title[200];

  line_number *= 100;
  
  if ((line_number%UPDATE_TITLE_FREQ) == 0) {
    sprintf (title, "Retrieving headers for '%s' : %d%%", group_name,
        (int) (line_number / total_lines));
    SetWindowText(hwnd,title);
  }
}


int check_server_code (int retcode)
{
  int class = retcode / 100;
  switch (class) {
  case 5:
    MessageBox (hWndConf, "News Server Error", "WinVN", MB_OK | MB_ICONHAND);
    CommBusy = FALSE;
    CommState = ST_NONE;
    return (1);
    break;
  case 4:
    MessageBox (hWndConf, CommLineIn, "WinVN", MB_OK | MB_ICONHAND);
    switch (class) {
    case 400:
      /* service discontinued */
      MRRCloseComm ();
      PostQuitMessage (0);
      return (1);
      break;
    default:
      return (1);
      break;
    }
    break;
  }
  return (0);
}

/*  Function sync_artnum

  Normally XREF returns lists of the same length for each header type
  but some servers have errors that could cause these lists to get
  out of sync. This function tries to find the proper location in the
  headers array and returns that location.  If the article number isn't
  found, it returns -1.  JD 6/19/93 */

long sync_artnum (unsigned long artnum,
        unsigned long activenum,
        header_p headers,TypGroup far * GroupDoc)
{ 
  long headerloc = CommDoc->ActiveLines;
  if (artnum == activenum) return(headerloc);
   else
     if (artnum < activenum)
       {
   while((artnum != activenum) && (headerloc > 0))
   {
    headerloc--;
    if ((header_elt(headers,headerloc))->number == artnum)
      return(headerloc);
   }
   return(-1);
       }
     else
       {
   while ((artnum != activenum) && (headerloc <= GroupDoc->total_headers))
   {
    headerloc++;
    if ((header_elt(headers,headerloc))->number == artnum)
      return(headerloc);
   }
   return(-1);
       }
 }

char *
get_best_reference (char * refer)
{
  char * bracket1, * bracket2;
  char * end = refer + strlen(refer) - 1; /* points to NULL */
  int bracket_len;

  bracket1 = strrchr (refer, '<');
  if (bracket1) {
    bracket_len = (int) (end - bracket1) + 2;
    if ((bracket_len < 30) && (!strrchr(bracket1,'>'))) {
      *bracket1 = (char) NULL;
      bracket1 = strrchr (refer, '<');
      if (!bracket1)
   bracket_len = 0;
      else {
   bracket2 = strrchr (refer, '>');
   if (bracket2)
     bracket_len = (int) (bracket2 - bracket1) + 1;
      }
    }
  }
  else
    bracket_len = 0;
  
  if (!bracket_len)
    return (NULL);
  else if (bracket_len > 29)
    bracket_len = 29;
  
  if ((bracket1+bracket_len) < end)
    *(bracket1+bracket_len) = (char) NULL;

  return (bracket1);
}

/*-- function DoCommState ----------------------------------------------
 *
 *  Function to implement an FSA to process incoming lines from
 *  the server.
 *  This function is called once for each line from the server.
 *
 *    Entry    CommLineIn  is a zero-terminated line received from
 *                         the server.
 *             CommState   is the current state of the FSA.
 */
void
DoCommState()
{
  static BOOL dialog_active=FALSE;
  TypLine far *LinePtr;
  TypBlock far *BlockPtr;
  HANDLE hBlock;
  WndEdit * WndPost;
  unsigned int Offset;
  TypLineID MyLineID;
  int retcode;
  int ih, found;
  unsigned long estnum;
  long int first,last,syncnum;
  unsigned long artnum;
  int mylen;
  int col;
  int mbcode;
  BOOL done=FALSE;
  BOOL dolist, do_authinfo;
  static char group[MAXINTERNALLINE];
  char mybuf[MAXINTERNALLINE];
  char mybuf2[MAXINTERNALLINE];
  char artline[MAXINTERNALLINE];
  char *cptr, *cdest;
  char far *lpsz;
  HANDLE header_handle;
  HANDLE thread_handle;
  header_p headers;
  header_p header;
  TypGroup far * GroupDoc;

  if(CommDoc) {

    switch(CommState) {
    case ST_NONE:
      break;

    case ST_ESTABLISH_COMM:
      if(!sscanf(CommLineIn,"%u",&retcode)) break;
      if(retcode == 200 || retcode == 201) { /* was 500 from induced error */
        CommBusy = TRUE;
        do_authinfo = FALSE;
        if(strlen(NNTPUserName)) {
          /* We have the AUTHINFO username.  Do we have the password?*/
          if(!strlen(NNTPPasswordEncrypted)) {
            /* Get the news server user password from the user */
            if (DialogBox (hInst, "WinVnComm", hWndConf, lpfnWinVnCommDlg)
               && strlen(NNTPPasswordEncrypted)) {
              do_authinfo = TRUE;
            }
          } else {
            do_authinfo = TRUE;
          }
        }
        if(do_authinfo) {
          sprintf(mybuf,"AUTHINFO user %s\r",NNTPUserName);
          CommState = ST_CHECK_AUTHINFO_USERNAME;
          PutCommLine(mybuf,strlen(mybuf));
        } else {
          goto End_Authinfo;
        }

      } else {
        MessageBox (hWndConf, CommLineIn, "Access Problem", MB_OK);
/*      MRRCloseComm (); */
        PostQuitMessage (0);
      }

      break;

    case ST_CHECK_AUTHINFO_USERNAME:
      retcode = 0;
      sscanf(CommLineIn,"%u",&retcode);
      if(!retcode) break;
      if(retcode >= 500) {
        MessageBox (hWndConf, "Error authorizing your username with the News Server.", "WinVN", MB_OK | MB_ICONHAND);
        goto End_Authinfo;
      }
      MRRDecrypt(NNTPPasswordEncrypted,(unsigned char *)mybuf2,MAXINTERNALLINE);
      sprintf(mybuf,"AUTHINFO pass %s\r",mybuf2);
      CommState = ST_CHECK_AUTHINFO_PASSWORD;
      PutCommLine(mybuf,strlen(mybuf));
      break;

   case ST_CHECK_AUTHINFO_PASSWORD:
      if(dialog_active)break;
      retcode = 0;
      if(sscanf(CommLineIn,"%u",&retcode) <= 0) break;
      if(retcode < 200 || retcode > 299) {
        dialog_active = TRUE;
        sprintf(mybuf,"Error authorizing your password with the News Server:\n%s.",CommLineIn);
        MessageBox (hWndConf,mybuf , "WinVN", MB_OK | MB_ICONHAND);
        dialog_active = FALSE;
      }
      goto End_Authinfo;


    case ST_END_AUTHINFO:
  End_Authinfo:;
   /* now check for the XOVER command */
      CommState = ST_XOVER_CHECK;
      PutCommLine ("XOVER\r", 6);
      break;

    case ST_XOVER_CHECK:
      retcode = 0;
      sscanf(CommLineIn, "%u", &retcode);
      if (retcode == 412)       /* 412 == 'not in a newsgroup' */
   xoverp = 1;
      else                      /* 500 == 'command not understood' */
   xoverp = 0;

      dolist = DoList;
      if(dolist == ID_DOLIST_ASK-ID_DOLIST_BASE) {
   dolist = DialogBox(hInst,"WinVnDoList",hWndConf,lpfnWinVnDoListDlg);
      }
      if(dolist) {
   StartList();
   did_list = 1;
      } else {
   did_list = 0;
   CommState = ST_NONE;
   CommBusy = FALSE;
   Initializing = INIT_DONE;
      }

      InvalidateRect(hWndConf,NULL,FALSE);

      break;


    case ST_LIST_RESP:
      retcode = 0;
      sscanf(CommLineIn,"%u",&retcode);
      if(retcode != 215) {
   check_server_code (retcode);
   break;
      }

      CommState = ST_LIST_GROUPLINE;
      RcvLineCount = 0;
      break;

    case ST_LIST_GROUPLINE:
      if(strcmp(CommLineIn,".") == 0) {
   CommState = ST_NONE;
   CommBusy = FALSE;
   Initializing = INIT_DONE;
   InvalidateRect(hWndConf,NULL,FALSE);

   ProcEndList();
      } else {
   ProcListLine((unsigned char *)CommLineIn);
      }
      break;

    case ST_GROUP_RESP:
      retcode = 0;
      sscanf(CommLineIn,"%u",&retcode);
      switch (retcode) {
      case 411:
   MessageBox (hWndConf, "No Such Newsgroup", "Error", MB_OK | MB_ICONHAND);
   /* abort the fledgling group window */
   DestroyWindow (CommDoc->hDocWnd);
   CommBusy = FALSE;
   CommState = ST_NONE;
   return;
   break;
      case 502:
   MessageBox (hWndConf, "Restricted Access", "WinVN", MB_OK | MB_ICONHAND);
   /* abort the fledgling group window */
   DestroyWindow (CommDoc->hDocWnd);
   CommBusy = FALSE;
   CommState = ST_NONE;
   return;
   break;
      default:
   if (check_server_code(retcode)) return;
   break;
      }

      sscanf(CommLineIn,"%u %lu %ld %ld %s",
        &retcode,&estnum,&first,&last,group);

      /* we don't want to grab *that* many! */
      if (estnum >= article_threshold) {
   DialogBox (hInst, "THRESHOLD",
         CommDoc->hDocWnd, lpfnWinVnThresholdDlg);
   if ((arts_to_retrieve > 0) && ((last - arts_to_retrieve) > first)) {
     first = (last - arts_to_retrieve) + 1;
   }
   else
     if (arts_to_retrieve == -1) /* they clicked 'all of them' */
       arts_to_retrieve = estnum;
      }
      else if (estnum > 0)
   arts_to_retrieve = estnum;
      else {
   MessageBox (hWndConf, "Empty Newsgroup", "WinVN", MB_OK | MB_ICONHAND);
   /* abort the fledgling group window */
   DestroyWindow (CommDoc->hDocWnd);
   CommBusy = FALSE;
   CommState = ST_NONE;
   return;
      }

      CommDoc->TotalLines = arts_to_retrieve;

      LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,
          CommDoc->ParentLineID,&BlockPtr,&LinePtr);

      if (arts_to_retrieve > 0)
   {
     header_handle =
       GlobalAlloc (GMEM_MOVEABLE, (long)
          ((sizeof (TypHeader)) *
           (long)(arts_to_retrieve)) + sizeof (thread_array *));
 
     /* allocate space for the header_array index table */
     thread_handle =
       GlobalAlloc (GMEM_MOVEABLE,
          (long) ((sizeof (long)) * (long) (arts_to_retrieve)));
     
     GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof(TypLine)));
     GroupDoc->header_handle = header_handle;
     GroupDoc->thread_handle = thread_handle;
     
   }

      /* stick nulls and 0's, etc.. in case display code get mis-threaded */
      initialize_header_array (header_handle, thread_handle, arts_to_retrieve);

      GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof(TypLine));
      GroupDoc->ServerEstNum = estnum;
      GroupDoc->ServerFirst = first;
      GroupDoc->ServerLast = last;

      GlobalUnlock(BlockPtr->hCurBlock);

      if (xoverp) {
   mylen = sprintf (mybuf, "XOVER %ld-%ld\r", first, last);
   CommState = ST_XOVER_START;
   PutCommLine(mybuf,mylen);
      }
      else {
   mylen = sprintf(mybuf,"XHDR from %ld-%ld\r",first,last);
   CommState = ST_XHDR_FROM_START;
   PutCommLine(mybuf,mylen);
      }

      break;

      /* The next few cases handle retrieval of XHDR information for display */
      /* in the group window.  If you change the number of XHDR's retrieved */
      /* (such as adding 'XHDR References' back into the state machine), you */
      /* need to reflect that change in the variable total_xhdrs. */

      /* the current flow is FROM -> DATE -> LINES -> SUBJECT */
      /* (threadp) FROM -> DATE -> LINES -> REF -> ID -> SUBJECT */

      /* this will now be done dynamically, depending on the state of */
      /* the 'threadp' variable */ 

    case ST_XOVER_START:
      retcode = 0;
      sscanf(CommLineIn,"%d",&retcode);
      if(retcode < 100) break;
      CommState = ST_XOVER_DATA;
      CommDoc->ActiveLines = 0;
      break;

    case ST_XOVER_DATA:
      if(strcmp(CommLineIn,".") == 0) {
   /* this is a yuck way to do this */
   CommState = ST_IN_GROUP;
   CommBusy = FALSE;
   finish_header_retrieval (CommDoc);
   InvalidateRect(CommDoc->hDocWnd,NULL,FALSE);
      }
      else {
   char * this_hop, * next_hop;
   char * reference;
   
   LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,
       CommDoc->ParentLineID,&BlockPtr,&LinePtr);
   GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof(TypLine)));
   header_handle = GroupDoc->header_handle;
   thread_handle = GroupDoc->thread_handle;

   GlobalUnlock(BlockPtr->hCurBlock);

   /* Lock the header data */
   headers = lock_headers (header_handle, thread_handle);
   header = header_elt (headers, CommDoc->ActiveLines);

   this_hop=CommLineIn;

   /* article number */
   next_hop = strchr (this_hop, '\t');
   *(next_hop++) = (char) NULL;

   header->number = atol (this_hop);
   
   /* subject */
   this_hop = next_hop;
   next_hop = strchr (this_hop, '\t');
   *(next_hop++) = (char) NULL;
   
   mylstrncpy (header->subject, this_hop, HEADER_SUBJECT_LENGTH);

   /* author */
   this_hop = next_hop;
   next_hop = strchr (this_hop, '\t');
   *(next_hop++) = (char) NULL;

   ParseAddress (this_hop,
            AddressString,MAXDIALOGSTRING,
            NameString,MAXDIALOGSTRING ) ;

   if (FullNameFrom)
     mylstrncpy (header->from, NameString, HEADER_FROM_LENGTH);
   else
     mylstrncpy (header->from, this_hop, HEADER_FROM_LENGTH);

   /* date */
   this_hop = next_hop;
   next_hop = strchr (this_hop, '\t');
   *(next_hop++) = (char) NULL;

   header->date = parse_usenet_date (this_hop);

   /* message-id */
   this_hop = next_hop;
   next_hop = strchr (this_hop, '\t');
   *(next_hop++) = (char) NULL;

   mylstrncpy (header->message_id, this_hop, HEADER_MESSAGE_ID_LENGTH);

   /* references */
   this_hop = next_hop;
   next_hop = strchr (this_hop, '\t');
   *(next_hop++) = (char) NULL;
   
   reference = get_best_reference (this_hop);
   if (reference) mylstrncpy (header->references, reference, HEADER_REFERENCES_LENGTH);

   /* bytes (ignored) */
   this_hop = next_hop;
   next_hop = strchr (this_hop, '\t');
   *(next_hop++) = (char) NULL;
   
   /* lines (last one doesn't have to have the tab */
   this_hop = next_hop;
   header->lines = atoi (this_hop);

   /* set other header fields */
   header->Selected= FALSE ;
   header->ArtDoc  = (TypDoc *) NULL;
   header->Seen = WasArtSeen (header->number,GroupDoc);

   unlock_headers (header_handle, thread_handle);

   CommDoc->ActiveLines++;

   update_window_title (CommDoc->hDocWnd, group,
              RcvLineCount++,
              CommDoc->TotalLines);
      }

      break;


      /* The next few cases handle retrieval of XHDR information for display */
      /* in the group window.  If you change the number of XHDR's retrieved */
      /* (such as adding 'XHDR References' back into the state machine), you */
      /* need to reflect that change in the variable total_xhdrs. */

      /* the current flow is FROM -> DATE -> LINES -> SUBJECT */
      /* (threadp) FROM -> DATE -> LINES -> REF -> ID -> SUBJECT */
             
      /* this will now be done dynamically, depending on the state of */
      /* the 'threadp' variable */ 

    case ST_XHDR_FROM_START:
      retcode = 0;
      sscanf(CommLineIn,"%d",&retcode);
      total_xhdrs = threadp ? 6 : 4; /* we do this here to allow */
                 /* mid-session change-of-mind  */
      if(retcode < 100) break;
      CommState = ST_XHDR_FROM_DATA;
      CommDoc->ActiveLines = 0;
      break;

    case ST_XHDR_FROM_DATA:
      if(strcmp(CommLineIn,".") == 0) {
   LockLine(CommDoc->hParentBlock, CommDoc->ParentOffset,
       CommDoc->ParentLineID, &BlockPtr,&LinePtr);

   GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof(TypLine)) );
   GroupDoc->total_headers = CommDoc->ActiveLines;

   first = GroupDoc->ServerFirst;
   last = GroupDoc->ServerLast;

   GlobalUnlock(BlockPtr->hCurBlock);
   CommDoc->ActiveLines = 0;

   /* Now ask for the date lines */
   mylen = sprintf(mybuf,"XHDR date %ld-%ld\r",first,last);
   CommState = ST_XHDR_DATE_START;
   PutCommLine(mybuf,mylen);
      }
      else {
//      char neat_from [80];
   /* Access the Group struct, get HANDLE for header data */
   LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,
       CommDoc->ParentLineID,&BlockPtr,&LinePtr);
   
   GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof(TypLine)) );
   header_handle = GroupDoc->header_handle;
   thread_handle = GroupDoc->thread_handle;

   GlobalUnlock(BlockPtr->hCurBlock);

   /* Lock the header data */
   headers = lock_headers (header_handle, thread_handle);

   sscanf (CommLineIn, "%ld", &artnum);
   header = header_elt (headers, CommDoc->ActiveLines);
   header->number = artnum;

   /* now use some of our nice formatting of email addresses */
   ParseAddress (get_xhdr_line (CommLineIn),
            AddressString,MAXDIALOGSTRING,
            NameString,MAXDIALOGSTRING ) ;

   /* copy that into headers[].from */
   mylstrncpy (header->from, NameString,HEADER_FROM_LENGTH);

   unlock_headers (header_handle, thread_handle);

   CommDoc->ActiveLines++;

   update_window_title (CommDoc->hDocWnd, group,
              RcvLineCount++,
              CommDoc->TotalLines * total_xhdrs);
      }

      break;

    case ST_XHDR_DATE_START:
      retcode = 0;
      sscanf(CommLineIn,"%d",&retcode);
      if (check_server_code(retcode)) break;
      CommState = ST_XHDR_DATE_DATA;
      CommDoc->ActiveLines = 0;
      break;

    case ST_XHDR_DATE_DATA:
      if(strcmp(CommLineIn,".") == 0) {
   LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,
       CommDoc->ParentLineID,&BlockPtr,&LinePtr);

   GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof(TypLine)) );
   GroupDoc->total_headers = CommDoc->ActiveLines;

   first = GroupDoc->ServerFirst;
   last = GroupDoc->ServerLast;

   GlobalUnlock(BlockPtr->hCurBlock);
   CommDoc->ActiveLines = 0;

   /* Now ask for the #of lines */
   mylen = sprintf(mybuf,"XHDR lines %ld-%ld\r",first,last); 
   CommState = ST_XHDR_LINES_START; 
   PutCommLine(mybuf,mylen);
      }
      else {
   
   /* Access the Group struct, get HANDLE for header data */
   LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,
       CommDoc->ParentLineID,&BlockPtr,&LinePtr);
   GroupDoc = ((TypGroup far *)((char far *)LinePtr+sizeof(TypLine)));
   header_handle = GroupDoc->header_handle;
   thread_handle = GroupDoc->thread_handle;

   GlobalUnlock(BlockPtr->hCurBlock);

   /* Lock the header data */
   headers = lock_headers (header_handle, thread_handle); 
   syncnum = sync_artnum(atol (CommLineIn),(header_elt(headers,CommDoc->ActiveLines))->number,headers,GroupDoc);
   if (syncnum >= 0)
     (header_elt(headers,syncnum))->date = parse_usenet_date (get_xhdr_line (CommLineIn));

   unlock_headers (header_handle, thread_handle);

   CommDoc->ActiveLines++;
   update_window_title (CommDoc->hDocWnd, group,
              RcvLineCount++,
              CommDoc->TotalLines * total_xhdrs);
      }

      break;

    case ST_XHDR_LINES_START:
      retcode = 0;
      sscanf(CommLineIn,"%d",&retcode);
      if (check_server_code(retcode)) break;
      CommState = ST_XHDR_LINES_DATA;
      CommDoc->ActiveLines = 0;
      break;

    case ST_XHDR_LINES_DATA:
      if(strcmp(CommLineIn,".") == 0) {
   LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,
       CommDoc->ParentLineID,&BlockPtr,&LinePtr);

   GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof(TypLine)) );
   GroupDoc->total_headers = CommDoc->ActiveLines;

   first = GroupDoc->ServerFirst;
   last = GroupDoc->ServerLast;

   GlobalUnlock(BlockPtr->hCurBlock);
   CommDoc->ActiveLines = 0;

   /* Check for threading option, if enabled, go to REF & ID */
   /* states first */

   if (threadp) {
     CommState = ST_XHDR_REF_START;          
     mylen = sprintf(mybuf,"XHDR references %ld-%ld\r",first,last); 
     PutCommLine(mybuf,mylen);
   }
   else {
     CommState = ST_XHDR_SUBJECT_START; 
     mylen = sprintf(mybuf,"XHDR subject %ld-%ld\r",first,last); 
     PutCommLine(mybuf,mylen);
   }
      }

      else {

   /* Access the Group struct, get HANDLE for header data */
   LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,
       CommDoc->ParentLineID,&BlockPtr,&LinePtr);
   GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof(TypLine)));

   header_handle = GroupDoc->header_handle;
   thread_handle = GroupDoc->thread_handle;

   GlobalUnlock(BlockPtr->hCurBlock);

   /* Lock the header data */
   headers = lock_headers (header_handle, thread_handle);

   syncnum = sync_artnum(atol (CommLineIn),(header_elt(headers,CommDoc->ActiveLines))->number,headers,GroupDoc);
   if (syncnum >= 0)
      sscanf (CommLineIn, "%ld %Fd", &artnum,&((header_elt(headers,syncnum))->lines));

   unlock_headers (header_handle, thread_handle);
   CommDoc->ActiveLines++;
   update_window_title (CommDoc->hDocWnd, group,
              RcvLineCount++,
              CommDoc->TotalLines * total_xhdrs);
      }

      break;

    case ST_XHDR_REF_START:
      retcode = 0;
      sscanf(CommLineIn,"%d",&retcode);
      if (check_server_code(retcode)) break;
      CommState = ST_XHDR_REF_DATA;
      CommDoc->ActiveLines = 0;
      break;

    case ST_XHDR_REF_DATA:
      if(strcmp(CommLineIn,".") == 0) {
   LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,
       CommDoc->ParentLineID,&BlockPtr,&LinePtr);

   GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof(TypLine));
   GroupDoc->total_headers = CommDoc->ActiveLines;

   first = GroupDoc->ServerFirst;
   last = GroupDoc->ServerLast;

   GlobalUnlock(BlockPtr->hCurBlock);
   CommDoc->ActiveLines = 0;

   /* Now ask for the message-id lines */
   mylen = sprintf(mybuf,"XHDR message-id %ld-%ld\r",first,last);
   CommState = ST_XHDR_MID_START;
   PutCommLine(mybuf,mylen);
      }
      else {
   char far * refer;    // , far * end,far * bracket1,far *bracket2;
//      int bracket_len; 

   /* Access the Group struct, get HANDLE for header data */
   LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,
       CommDoc->ParentLineID,&BlockPtr,&LinePtr);
   GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof(TypLine)));

   header_handle = GroupDoc->header_handle;
   thread_handle = GroupDoc->thread_handle;

   GlobalUnlock(BlockPtr->hCurBlock);

   /* Lock the header data */
   headers = lock_headers (header_handle, thread_handle);

   /* for now, we only pay attention to first (whole) referral */
   refer = get_xhdr_line(CommLineIn);

   refer = get_best_reference (refer);

   if (refer)
     {
      /* Patch to check for bad info from server JD 6/19/93 */
     syncnum = sync_artnum(atol (CommLineIn),
            (header_elt(headers,
                   CommDoc->ActiveLines))->number,
            headers,GroupDoc);
     if (syncnum >= 0)
            mylstrncpy ((header_elt (headers,syncnum))->references,
         refer,HEADER_REFERENCES_LENGTH);
     }

   unlock_headers (header_handle, thread_handle);

   CommDoc->ActiveLines++;
   update_window_title (CommDoc->hDocWnd, group,
              RcvLineCount++,
              CommDoc->TotalLines * total_xhdrs);

      }

      break;


    case ST_XHDR_MID_START:
      retcode = 0;
      sscanf(CommLineIn,"%d",&retcode);
      if (check_server_code(retcode)) break;
      CommState = ST_XHDR_MID_DATA;
      CommDoc->ActiveLines = 0;
      break;

    case ST_XHDR_MID_DATA:
      if(strcmp(CommLineIn,".") == 0) {
   LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,
       CommDoc->ParentLineID,&BlockPtr,&LinePtr);
   
   GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof(TypLine));

   GroupDoc->total_headers = CommDoc->ActiveLines;
   
   first = GroupDoc->ServerFirst;
   last = GroupDoc->ServerLast;

   GlobalUnlock(BlockPtr->hCurBlock);
   CommDoc->ActiveLines = 0;

   /* Now ask for the subject lines */
   mylen = sprintf(mybuf,"XHDR subject %ld-%ld\r",first,last);
   CommState = ST_XHDR_SUBJECT_START;
   PutCommLine(mybuf,mylen);
      }
      else {
   /* Access the Group struct, get HANDLE for header data */
   LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,
       CommDoc->ParentLineID,&BlockPtr,&LinePtr);

   GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof(TypLine));

   header_handle = GroupDoc->header_handle;
   thread_handle = GroupDoc->thread_handle;

   GlobalUnlock(BlockPtr->hCurBlock);

   /* Lock the header data */
   headers = lock_headers (header_handle, thread_handle);

   syncnum = sync_artnum(atol (CommLineIn),(header_elt(headers,CommDoc->ActiveLines))->number,headers,GroupDoc);
   if (syncnum >= 0)
     mylstrncpy((header_elt (headers,syncnum))->message_id,
           (char far *) (get_xhdr_line (CommLineIn)),
           HEADER_MESSAGE_ID_LENGTH);         /* bad, hardcoded. */

   unlock_headers (header_handle, thread_handle);

   CommDoc->ActiveLines++;
   update_window_title (CommDoc->hDocWnd, group,
              RcvLineCount++,
              CommDoc->TotalLines * total_xhdrs);

      }

      break;


    case ST_XHDR_SUBJECT_START:
      retcode = 0;
      sscanf(CommLineIn,"%d",&retcode);
      if (check_server_code(retcode)) break;
      CommState = ST_XHDR_SUBJECT_DATA;
      break;

    case ST_XHDR_SUBJECT_DATA:

      if(strcmp(CommLineIn,".") == 0) {
   CommState = ST_IN_GROUP;
   CommBusy = FALSE;
   finish_header_retrieval (CommDoc);
   InvalidateRect(CommDoc->hDocWnd,NULL,FALSE);
      }
      else {

   artnum = 0;
   sscanf(CommLineIn,"%ld",&artnum);
   if(artnum) {
     LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,
         CommDoc->ParentLineID,&BlockPtr,&LinePtr);

     GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof(TypLine)));
     header_handle = GroupDoc->header_handle;
     thread_handle = GroupDoc->thread_handle;        
     headers = lock_headers (header_handle, thread_handle);

     /* update the seen thing. */
     syncnum = sync_artnum(atol (CommLineIn),(header_elt(headers,CommDoc->ActiveLines))->number,headers,GroupDoc);
     if (syncnum >= 0) 
        header = header_elt (headers, syncnum);
     else
        header = header_elt (headers, CommDoc->ActiveLines);

     header->Selected= FALSE ;
     header->ArtDoc  = (TypDoc *) NULL;
     header->Seen =
       WasArtSeen (artnum,(TypGroup far *)( ((char far *)LinePtr) +
                  sizeof(TypLine) ) );

     UnlockLine(BlockPtr,LinePtr,&(CommDoc->hParentBlock),
           &(CommDoc->ParentOffset),&(CommDoc->ParentLineID));

     mylstrncpy(header->subject,
           get_xhdr_line (CommLineIn), HEADER_SUBJECT_LENGTH);


     unlock_headers (header_handle, thread_handle);
     CommDoc->ActiveLines++;
     update_window_title (CommDoc->hDocWnd, group,
                RcvLineCount++,
                CommDoc->TotalLines * total_xhdrs);

   }
      }

      break;

    case ST_IN_GROUP:
      break;

    case ST_ARTICLE_RESP:
      retcode = 0;
      sscanf(CommLineIn,"%d",&retcode);
      if (check_server_code(retcode)) break;
      CommState = ST_REC_ARTICLE;
      break;

    case ST_REC_ARTICLE:
      if(strcmp(CommLineIn,".") == 0) {
   CommState = ST_IN_GROUP;
   CommBusy = FALSE;

   LockLine (CommDoc->ParentDoc->hParentBlock,
        CommDoc->ParentDoc->ParentOffset,
        CommDoc->ParentDoc->ParentLineID,
        &BlockPtr, &LinePtr);

   GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));

   header_handle = GroupDoc->header_handle;
   thread_handle = GroupDoc->thread_handle;

   headers = lock_headers (header_handle, thread_handle);
   lpsz = (char far *) ((header_elt (headers, CommDoc->LastSeenLineID))->subject);
   unlock_headers (header_handle, thread_handle);

   mylstrncpy(group,lpsz,MAXGROUPNAME);
   sprintf(mybuf,"%s (%u lines)",group,CommDoc->TotalLines);
   SetWindowText(CommDoc->hDocWnd,mybuf);
   InvalidateRect(CommDoc->hDocWnd,NULL,FALSE);
   GlobalUnlock(BlockPtr->hCurBlock);

   SendMessage(CommDoc->ParentDoc->hDocWnd, IDM_RETRIEVE_COMPLETE, 0, 0);

   /* Skip to the first line of the text of the article
    * and make sure it's visible on the screen.  This is
    * so that the user doesn't have to have the first
    * screen filled with a lengthy, worthless header.
    *
    * and save number of header lines (on display)
    * for later (Bretherton)
    */
   TopOfDoc(CommDoc,&BlockPtr,&LinePtr);
   found = FALSE;
   do {
     lpsz = ((char far *)LinePtr + sizeof(TypLine) + sizeof(TypText));
     if(IsLineBlank(lpsz)) {
       found = TRUE;
       CommDoc->HeaderLines=WhatLine (BlockPtr, LinePtr);
       break;
     }
     if(!NextLine(&BlockPtr,&LinePtr)) break;
   } while(!found);
   NextLine(&BlockPtr,&LinePtr);

   if(CommDoc->TotalLines > CommDoc->ScYLines
      && !CommDoc->TopScLineID) {
     TopOfDoc(CommDoc,&BlockPtr,&LinePtr);
     found = FALSE;
     do {
       lpsz = ((char far *)LinePtr + sizeof(TypLine) + sizeof(TypText));
       if(IsLineBlank(lpsz)) {
         found = TRUE;
         break;
       }
       if(!NextLine(&BlockPtr,&LinePtr)) break;
     } while(!found);
     NextLine(&BlockPtr,&LinePtr);

     /* If the line is in the last screen's worth of lines, back
      * up the pointer so it points to the first line of the last
      * screen.
      */
     if(found) {
       AdjustTopSc(BlockPtr,LinePtr);
     } else {
       UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
     }
   }

      } else {
   /* Copy this line into an image of a textblock line,
    * expanding tabs.
    */
   cdest = artline+sizeof(TypLine)+sizeof(TypText);

   /* special case for lines starting with '..' */
   if (strncmp (CommLineIn, "..", 2))
     cptr = CommLineIn;
   else
     cptr = CommLineIn + 1;

   for(col=0;
       *cptr && col<(MAXINTERNALLINE-3*sizeof(TypLine)-sizeof(TypText));
       cptr++) {
     if(*cptr == '\t') {
       do {
         *(cdest++) = ' ';
       } while (++col & 7);
     } else {
       *(cdest++) = *cptr;
       col++;
     }
   }
   *(cdest++) = '\0';

   mylen = (cdest-artline) + sizeof(int);
   mylen += mylen%2;
   ((TypText *)(artline+sizeof(TypLine)))->NameLen =
     (cdest-1) - (artline+sizeof(TypLine)+sizeof(TypText));
   ((TypLine *)artline)->length = mylen;
   ((TypLine *)artline)->LineID = NextLineID++;
   ((TypLine *)artline)->active = TRUE;
   *( (int *) (artline+mylen-sizeof(int)) ) = mylen;
   LockLine(CommDoc->hCurAddBlock,CommDoc->AddOffset,CommDoc->AddLineID,&BlockPtr,&LinePtr);
   AddLine((TypLine *)artline,&BlockPtr,&LinePtr);
   UnlockLine(BlockPtr,LinePtr,&(CommDoc->hCurAddBlock),
         &(CommDoc->AddOffset),&(CommDoc->AddLineID));
   if((CommDoc->TotalLines % UPDATE_ART_FREQ) == 0) {
     InvalidateRect(CommDoc->hDocWnd,NULL,FALSE);  
   }
      }

      break;

    case ST_POST_WAIT_PERMISSION:

      WndPost = getWndEdit(WndPosts,CommWnd,MAXPOSTWNDS) ;
      found = (WndPost != NULL) ;

      retcode = 0;
      sscanf(CommLineIn,"%u",&retcode);

      if(retcode == 340) {
   PostText(WndPost);
      } else {
   check_server_code (retcode);
   MessageBox(WndPost->hWnd,CommLineIn+4,"Cannot Post Article",
         MB_OK|MB_ICONEXCLAMATION);
   CommBusy = FALSE;
   CommState = ST_NONE;
      }

      break;

    case ST_POST_WAIT_END:

      WndPost = getWndEdit(WndPosts,CommWnd,MAXPOSTWNDS) ;
      found = (WndPost != NULL) ;

 /* no check for failure to find posting documents */
      retcode = 0;
      sscanf(CommLineIn,"%d",&retcode);
      if(retcode == 441 || retcode == 440) {
   cptr = "Posting Failed";
   mbcode = MB_OK|MB_ICONEXCLAMATION;
   MessageBox(WndPost->hWnd,CommLineIn+4,cptr,mbcode);
   done = TRUE;
      } else if(retcode == 240) {
   cptr = "Article Posted OK";
   mbcode = MB_OKCANCEL;
//      if (MessageBox(hWndPost,CommLineIn+4,cptr,mbcode)
//           == ID_OK)
   if (PostLog)
   {  HANDLE hEditBuffer ;
      char *mbuf ;
      hEditBuffer = (HANDLE) SendMessage (WndPost->hWndEdit, EM_GETHANDLE, 0, 0L);
      mbuf = LocalLock (hEditBuffer);
      WriteEditLog(WndPost->hWnd,PostLogFile,mbuf) ;
      LocalUnlock (hEditBuffer);
   }
   WndPost->dirty=DT_CLEAN ;
   DestroyWindow(WndPost->hWnd) ;
   done = TRUE;
      } else
   if (check_server_code (retcode)) break;

      if(done) {
   CommBusy = FALSE;
   CommState = ST_NONE;
      }
      break;

      /* the following code is for an MRR-hacked nntp server */

/* Code not used anywhere
         M Bretherton 14 June 1993
#if 0

    case ST_MAIL_WAIT_PERMISSION:
      for(ih=0,found=FALSE; !found && ih<MAXMAILWNDS; ih++) {
   if( WndMails[ih].hWnd) == CommWnd) {

     hWndPost = WndMails[ih];

     found = TRUE;
     break;
   }
      }
      retcode = 0;
      sscanf(CommLineIn,"%d",&retcode);
      if(retcode <= 0) {
   break;
      } else if(retcode == 350) {
   PostText(ih,DOCTYPE_MAIL);
      } else {
   MessageBox(WndPost->hWnd,CommLineIn+4,"Cannot Mail Message",
         MB_OK|MB_ICONEXCLAMATION);
   CommBusy = FALSE;
   CommState = ST_NONE;
      }

      break;

    case ST_POST_WAIT_END:
      for(ih=0,found=FALSE; !found && ih<MAXPOSTWNDS; ih++) {
   if(WbdPosts[ih].hWnd) == CommWnd) {

     WndPost = WndPosts[ih];

     found = TRUE;
     break;
   }
      }
      retcode = 0;
      sscanf(CommLineIn,"%d",&retcode);
      if(retcode == 441 || retcode == 440) {
   cptr = "Posting Failed";
   mbcode = MB_OK|MB_ICONEXCLAMATION;
   MessageBox(WndPost->hWnd,CommLineIn+4,cptr,mbcode);
   done = TRUE;
      } else if(retcode == 240) {
   cptr = "Article Posted OK";
   mbcode = MB_OKCANCEL;
//      if (MessageBox(WndPost->hWnd,CommLineIn+4,cptr,mbcode)
//           == ID_OK)
   DestroyWindow(WndPost->hWnd) ;
   done = TRUE;
      } else
   if (check_server_code (retcode)) break;

      if(done) {
   CommBusy = FALSE;
   CommState = ST_NONE;
      }
      break;

#endif
*/
    case ST_GROUP_REJOIN:
      CommState = ST_ARTICLE_RESP;
      break;
    }
  }
}

BOOL
isLineQuotation(char *textptr)
{
   char *loc ;
   loc = memchr(textptr,QuoteLineInd,2);
   if (!loc) loc = memchr(textptr,'|',2);
   return(loc!=NULL) ;
}


/*-- function WasArtSeen ---------------------------------------------
 *
 *  Determines whether (according to the information in a TypGroup entry)
 *  a given article number was seen.
 *
 *  Returns TRUE iff the article has been seen.
 */
BOOL
WasArtSeen(ArtNum,GroupPtr)
unsigned long ArtNum;
TypGroup far *GroupPtr;
{
   TypRange far *RangePtr = (TypRange far *) ((char far *)
      GroupPtr + RangeOffset(GroupPtr->NameLen));
   unsigned int nr;

   for(nr=0; nr < GroupPtr->nRanges; nr++) {
      if(ArtNum >= RangePtr->First && ArtNum <= RangePtr->Last) {
    return(TRUE);
      } else {
    RangePtr++;
      }
   }
   return(FALSE);
}


/*--- function mylstrncmp -----------------------------------------------
 *
 *   Just like strncmp, except takes long pointers.
 */
int
mylstrncmp(ptr1,ptr2,len)
char far *ptr1;
char far *ptr2;
int len;
{
   for(;len--;ptr1++,ptr2++) {
      if(*ptr1 > *ptr2) {
    return(1);
      } else if(*ptr1 < *ptr2) {
    return(-1);
      }
   }
   return(0);
}

/*--- function mylstrncpy -----------------------------------------------
 *
 *   Just like strncpy, except takes long pointers.
 */
char far *
mylstrncpy(ptr1,ptr2,len)
char far *ptr1;
char far *ptr2;
int len;
{
   char far *targ = ptr1;

   for(; --len && *ptr2; ptr1++,ptr2++) {
      *ptr1 = *ptr2;
   }
   *ptr1 = '\0';
   return(targ);
}

/* this is a temporary test... */
char far *
mylstrcpy (ptr1, ptr2)
char_p ptr1;
char far *ptr2;
{
  char far *targ = ptr1;
  for (; *ptr2; ptr1++, ptr2++) {
     *ptr1 = *ptr2;
  }
  *ptr1 = '\0';
  return (targ);
}

#if 0
/*--- function lstrcmpnoblank ------------------------------------------
 *
 *   Like strcmp, except takes long pointers and also stops at
 *   the first blank.
 */
int
lstrcmpnoblank(str1,str2)
char far **str1;
char far **str2;
{
   register char far *s1=*str1, far *s2=*str2;

   for(;*s1 && *s2 && *s1!=' ' && *s2!=' '; s1++,s2++) {
      if(*s1 > *s2) {
         return (1);
      } else if(*s1 < *s2) {
         return (-1);
      }
   }
   if(*s1 == *s2) {
      return(0);
   } else if(*s1) {
      return(1);
   } else {
      return(-1);
   }
}
#endif

void
finish_header_retrieval ()
{
  TypLine far *LinePtr;
  TypBlock far *BlockPtr;
  TypGroup far * GroupDoc;
  HANDLE header_handle, thread_handle;
//  HANDLE hBlock;
  char far * lpsz;
  char group[MAXGROUPNAME];
  char mybuf[MAXINTERNALLINE];
  header_p headers;

  /* release the mouse that is captured to the usenet window */
  ReleaseCapture ();

  CommDoc->TotalLines = CommDoc->ActiveLines;
  /* Disabled by MRR so that ActiveLines is the number of lines
   * we should display in the Group window.  Eventually, will
   * change it so that ActiveLines will count only unread articles
   * if the user desires. 
   */
  /* CommDoc->ActiveLines = 0;*/
  /* Fetch this group's line in NetDoc so we can get the
   * group's name for the window's title bar.
   */
  LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,
      CommDoc->ParentLineID,&BlockPtr,&LinePtr);

  GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof(TypLine)));
  header_handle = GroupDoc->header_handle;
  thread_handle = GroupDoc->thread_handle;        
  lock_headers (header_handle, thread_handle);

  if (threadp) {
    SetWindowText (CommDoc->hDocWnd, "sorting headers...");
    sort_by_threads (header_handle, thread_handle, CommDoc->TotalLines);
  }

  unlock_headers (header_handle, thread_handle);
   
  GroupDoc->total_headers = CommDoc->TotalLines;

  lpsz = (char far *) ( ((char far *)LinePtr) +
             sizeof(TypLine)+ sizeof(TypGroup) ) ;

  mylstrncpy(group,lpsz,MAXGROUPNAME);
  sprintf(mybuf,"%s (%u articles)",group,CommDoc->TotalLines);
  SetWindowText(CommDoc->hDocWnd,mybuf);

  /* If we have information from NEWSRC on the highest-
   * numbered article previously seen, position the window
   * so the new articles can be seen without scrolling.
   */
  {
    unsigned int i;

    LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,
        CommDoc->ParentLineID,&BlockPtr,&LinePtr);

    /* inside the lock, can access the GroupStruct */
    GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof(TypLine)));
    header_handle = GroupDoc->header_handle;
    thread_handle = GroupDoc->thread_handle;      
    headers = lock_headers (header_handle, thread_handle);
     
    if (CommDoc->TotalLines >0)
      for (i = CommDoc->TotalLines - 1;
       ((i > 0) && ((header_elt(headers,i))->Seen == FALSE));
       i--) ;
    CommDoc->TopLineOrd = (i > 5) ? i - 4 : 0;

    unlock_headers (header_handle, thread_handle);
  }

  InvalidateRect(CommDoc->hDocWnd,NULL,FALSE);
}

/*
 * Look through the MAIL or Post edits and return the edit with
 * matching window handle Consider - centralising initial window
 * location in wvmail and wndpost using a single array (save passing
 * structure and size into this module)
 *
 */

WndEdit * getWndEdit(WndEdit * WndEdits,HWND hWnd,int numEntries)
{ 
  int ih ;

  for(ih=0; ih<numEntries; ih++)
    {
      if(WndEdits[ih].hWnd == hWnd)
   {
     return & WndEdits[ih] ;
   }
    }

  //MessageBox(0,"getWndEditFound Nothing","mrb debug", MB_OK | MB_ICONHAND);

  return(WndEdit *) NULL ;
}






