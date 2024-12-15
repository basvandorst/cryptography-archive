/*
 * $Id: wvutil.c 1.69 1994/09/18 22:35:49 jcooper Exp $
 *
 */

/*-- WVUTIL.C -- File containing utility routines.
 */

#include <windows.h>
#include <windowsx.h>      // for GlobalFreePtr (JSC)
#include "wvglob.h"
#include "winvn.h"
#pragma hdrstop
#include <commdlg.h>    // for GetOpenFileName dialog (JSC)
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

char far *mylstrcpy (char_p ptr1, char far * ptr2);
char *get_xhdr_line (char *line);
time_t parse_usenet_date (char *date);
void finish_header_retrieval ();
void GenerateFileFilters (HWND hParentWnd, char *filters);

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
GetNum (ptr, num)
     char **ptr;
     long int *num;
{
  BOOL gotit = FALSE;

  /* Skip initial spaces                                            */

  while ((**ptr) && **ptr == ' ')
    (*ptr)++;

  *num = 0;
  while (**ptr && isdigit (**ptr)) {
    *num = 10 * (*num) + (**ptr - '0');
    gotit = TRUE;
    (*ptr)++;
  }
  return (gotit);
}

char *
get_xhdr_line (char *line)
{
  char *cptr;
/* skip past the art # and space */
  for (cptr = line; isdigit (*cptr); cptr++);
  for (; *cptr == ' '; cptr++);
  return (cptr);
}

#if 0
MRB already did this
void
make_neat_from (char far * in, char far * out)
{
  char far *left, far * right;

  /* this is controlled from .ini */
  if (FullNameFrom) {
    left = strchr (in, '(');
    right = strrchr (in, ')');

    if ((left && right) && (left < right)) {
      strncpy (out, left + 1, (size_t) (right - left - 1));
      out[(right - left - 1)] = (char) 0;
    }
    else       /* No name in parens */
      strcpy (out, in);
  }
  else            /* !FullNameFrom */
    strcpy (out, in);
}
#endif

/*-- function StrToRGB -------------------------------------------------
 *
 *  Takes an ASCII string of the form "r,g,b" where r, g, and b are
 *  decimal ASCII numbers, and converts it to an RGB color number.
 */
COLORREF
StrToRGB (cstring)
     char *cstring;
{
  BYTE red, green, blue;
  long int lred, lgreen, lblue;

  GetNum (&cstring, &lred);
  cstring++;
  GetNum (&cstring, &lgreen);
  cstring++;
  GetNum (&cstring, &lblue);
  red = (BYTE) lred;
  green = (BYTE) lgreen;
  blue = (BYTE) lblue;

  return (RGB (red, green, blue));
}

/*-- function RGBToStr -------------------------------------------------
 *
 *  Takes an RGB color ref and converts to a string of the form "r,g,b"
 *  result is placed in buf
 *  (JSC)
 */
char *
RGBToStr (char *buf, DWORD rgbVal)
{
  sprintf (buf, "%u,%u,%u", GetRValue (rgbVal),
      GetGValue (rgbVal),
      GetBValue (rgbVal));
  return (buf);
}

/* This was lifted from ANU news. */

time_t
parse_usenet_date (char *s)
{
  struct tm datetime;
  char *cp, mon[80];
  int dom = 0, yr = 0, hr = 0, mn = 0, sc = 0, mth = 0;
  static char fmtMonthTable[37] = "janfebmaraprmayjunjulaugsepoctnovdec";

  if (!s || !*s)
    return (0);
  if (cp = strchr (s, ','))
    s = ++cp;
  while (isspace (*s))
    s++;
  *mon = '\0';
  if (isdigit (*s)) {
    sscanf (s, "%d %s %d %d:%d:%d", &dom, mon, &yr, &hr, &mn, &sc);
    if (yr < 100)
      yr += 1900;
  }
  else
    sscanf (s, "%*s %s %d %d:%d:%d %d", mon, &dom, &hr, &mn, &sc, &yr);

  if (!dom || !yr || !*(cp = mon))
    return (0);
  if ((dom <= 0) || (dom >= 32))
    return (0);
  if ((yr < 1980) || (yr > 2020))
    return (0);
  if (strlen (mon) > 10)
    return (0);
  if ((hr < 0) || (hr > 23))
    return (0);
  if ((mn < 0) || (mn > 59))
    return (0);
  if ((sc < 0) || (sc > 59))
    return (0);

  for (cp = mon; *cp; cp++)
    *cp = tolower (*cp);

  if (cp = strstr (fmtMonthTable, mon))
    mth = (cp - fmtMonthTable) / 3;

/*  Setup a Posix time structure and calculate time in absolute
   time (seconds since midnight, Jan 1, 1970    JD 06/25/93 */

  datetime.tm_year = yr - 1900;
  datetime.tm_mon = mth;
  datetime.tm_mday = dom;
  datetime.tm_hour = hr;
  datetime.tm_min = mn;
  datetime.tm_sec = sc;

  return (mktime (&datetime));
}

/*-- function StringDate ----------------*/
char *
StringDate (char *s, time_t time)
{
  struct tm *datetime;
  if (time != 0) {
    datetime = localtime (&time);

    if (fmtDaysB4Mth) {
      sprintf (s, "%02d%s%02d", datetime->tm_mday, fmtDateDelim, datetime->tm_mon + 1);
    }
    else {
      sprintf (s, "%02d%s%02d", datetime->tm_mon + 1, fmtDateDelim, datetime->tm_mday);
    }
    return (s);
  }
  else
    return ("-----");
}

/*-- function DoCommInput ---------------------------------------
 *
 *
 */
void
DoCommInput ()
{
  int ch;

  while ((CommState != ST_CLOSED_COMM) && ((ch = MRRReadComm ()) >= 0)) {
    if (ch == IgnoreCommCh) {
    }
    else if (ch == EOLCommCh) {
      *CommLinePtr = '\0';
      DoCommState ();
      CommLinePtr = CommLineIn;
    }
    else {
      *(CommLinePtr++) = (char) ch;
      if (CommLinePtr == CommLineLWAp1)
   CommLinePtr--;
    }
  }
}

void
update_window_title (HWND hwnd,
           char *group_name,
           unsigned long line_number,
           unsigned long total_lines)
{
  char title[200];
  static int prevPercent, newPercent;
  // to avoid flicker, update percent only if it has changed more than 1%

  line_number *= 100;
  if (newPercent < prevPercent)
    prevPercent = 0;

  if ((line_number % UPDATE_TITLE_FREQ) == 0) {
    newPercent = (int) (line_number / total_lines);
    if (newPercent != prevPercent && newPercent - prevPercent > 1) {
      sprintf (title, "Retrieving headers for '%s' : %d%%", group_name, newPercent);
      SetWindowText (hwnd, title);
      prevPercent = newPercent;
    }
  }
}

int
check_server_code (int retcode)
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
      break;
    default:
      break;
    }
    CommBusy = FALSE;
    CommState = ST_NONE;
    return (1);
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

long
sync_artnum (unsigned long artnum,
        unsigned long activenum,
        header_p headers, TypGroup far * GroupDoc)
{
  long headerloc = CommDoc->ActiveLines;
  if (artnum == activenum)
    return (headerloc);
  else if (artnum < activenum) {
    while ((artnum != activenum) && (headerloc > 0)) {
      headerloc--;
      if ((header_elt (headers, headerloc))->number == artnum)
   return (headerloc);
    }
    return (-1);
  }
  else {
    while ((artnum != activenum) && (headerloc <= GroupDoc->total_headers)) {
      headerloc++;
      if ((header_elt (headers, headerloc))->number == artnum)
   return (headerloc);
    }
    return (-1);
  }
}

char *
get_best_reference (char *refer)
{
  char *bracket1, *bracket2;
  char *end = refer + strlen (refer) - 1; /* points to NULL */
  int bracket_len;

  bracket1 = strrchr (refer, '<');
  if (bracket1) {
    bracket_len = (int) (end - bracket1) + 2;
    if ((bracket_len < 30) && (!strrchr (bracket1, '>'))) {
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

  if ((bracket1 + bracket_len) < end)
    *(bracket1 + bracket_len) = (char) NULL;

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
DoCommState ()
{
  static BOOL dialog_active = FALSE;
  TypLine far *LinePtr;
  TypBlock far *BlockPtr;
  HANDLE hBlock;
  unsigned int Offset;
  TypLineID MyLineID;
  int retcode;
  int found;
  unsigned long estnum;
  long int first, last, syncnum;
  unsigned long artnum;
  int mylen;
  BOOL done = FALSE;
  BOOL DlgStatus = FALSE;
  BOOL dolist, do_authinfo;
  static char group[MAXINTERNALLINE];
  char mybuf[MAXINTERNALLINE];
  char mybuf2[MAXINTERNALLINE];
  char far *lpsz;
  HANDLE header_handle;
  HANDLE thread_handle;
  header_p headers;
  header_p header;
  TypGroup far *GroupDoc;
  TypRange *RangePtr;

  /* CommDoc is !NULL if retrieving group list, article headers or articles */
  /* CommDecoding is true if retrieving article in decode mode (not to a doc) */
  /* PostEdit !NULL if we are posting (this is from an edit, no doc involved) */
  if (CommDoc || CommDecoding || PostEdit) {

    switch (CommState) {
    case ST_NONE:
      break;

    case ST_ESTABLISH_COMM:
      if (!sscanf (CommLineIn, "%u", &retcode))
   break;
      if (retcode == 200 || retcode == 201) {   /* was 500 from induced error */
   CommBusy = TRUE;
   do_authinfo = FALSE;
   Authenticated = FALSE;
   if (strlen (NNTPUserName)) {
     /* We have the AUTHINFO username.  Do we have the password? */
     if (!strlen (NNTPPasswordEncrypted)) {
       /* Get the news server user password from the user */
       if (DialogBox (hInst, (LPCSTR) "WinVnComm", hWndConf, (DLGPROC) lpfnWinVnCommDlg)
      && strlen (NNTPPasswordEncrypted)) {
         do_authinfo = TRUE;
       }
     }
     else {
       do_authinfo = TRUE;
     }
   }
   if (do_authinfo) {
     sprintf (mybuf, "AUTHINFO user %s", NNTPUserName);
     CommState = ST_CHECK_AUTHINFO_USERNAME;
     PutCommLine (mybuf);
   }
   else {
     goto End_Authinfo;
   }

      }
      else {
   MessageBox (hWndConf, CommLineIn, "Access Problem", MB_OK);
   /*      MRRCloseComm (); */
   /*        PostQuitMessage (0); */
   Initializing = INIT_NOT_CONNECTED;
   InvalidateRect (hWndConf, NULL, TRUE);
      }

      break;

    case ST_CHECK_AUTHINFO_USERNAME:
      retcode = 0;
      sscanf (CommLineIn, "%u", &retcode);
      if (!retcode)
   break;
      if (retcode >= 500) {
   MessageBox (hWndConf, "Error authorizing your username with the News Server.", "WinVN", MB_OK | MB_ICONHAND);
   goto End_Authinfo;
      }
      MRRDecrypt (NNTPPasswordEncrypted, (unsigned char *) mybuf2, MAXINTERNALLINE);
      sprintf (mybuf, "AUTHINFO pass %s", mybuf2);
      CommState = ST_CHECK_AUTHINFO_PASSWORD;
      PutCommLine (mybuf);
      break;

    case ST_CHECK_AUTHINFO_PASSWORD:
      if (dialog_active)
   break;
      retcode = 0;
      if (sscanf (CommLineIn, "%u", &retcode) <= 0)
   break;
      if (retcode < 200 || retcode > 299) {
   dialog_active = TRUE;
   sprintf (mybuf, "Error authorizing your password with the News Server:\n%s.", CommLineIn);
   MessageBox (hWndConf, mybuf, "WinVN", MB_OK | MB_ICONHAND);
   dialog_active = FALSE;
      } else {                                               
         /* Authentication was successful.  Store this fact, and the name under
          * which the user was authenticated.
          */
         Authenticated = TRUE;
         strncpy(AuthenticatedName,NNTPUserName,MAXNNTPSIZE);
      }
      goto End_Authinfo;


    case ST_END_AUTHINFO:
    End_Authinfo:;
      /* now check for the XOVER command */
      CommState = ST_XOVER_CHECK;
      PutCommLine ("XOVER");
      break;

    case ST_XOVER_CHECK:
      retcode = 0;
      sscanf (CommLineIn, "%u", &retcode);
      if (retcode == 412 && !force_xhdr)  /* 412 == 'not in a newsgroup' */
   xoverp = 1;
      else        /* 500 == 'command not understood' */
   xoverp = 0;

      dolist = DoList;
      if (dolist == ID_DOLIST_ASK - ID_DOLIST_BASE)
   if (MessageBox (hWndConf, "Request the latest group list from server?\n(This can be time consuming)",
         "Request LIST from server?", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDNO)
     dolist = 0;

      if (dolist) {
   StartList ();
   did_list = 1;
      }
      else {
   did_list = 0;
   CommState = ST_NONE;
   CommBusy = FALSE;
   Initializing = INIT_READY;
      }
      InvalidateRect (hWndConf, NULL, FALSE);
      break;

    case ST_LIST_RESP:
      retcode = 0;
      sscanf (CommLineIn, "%u", &retcode);
      if (retcode != 215) {
   check_server_code (retcode);
   break;
      }

      CommState = ST_LIST_GROUPLINE;
      RcvLineCount = 0;
      break;

    case ST_LIST_GROUPLINE:
      if (strcmp (CommLineIn, ".") == 0) {
   CommState = ST_NONE;
   CommBusy = FALSE;
   Initializing = INIT_READY;
   InvalidateRect (hWndConf, NULL, FALSE);

   ProcEndList ();
      }
      else {
   ProcListLine ((unsigned char *) CommLineIn);
      }
      break;

    case ST_GROUP_RESP:
      retcode = 0;
      sscanf (CommLineIn, "%u", &retcode);
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
   if (check_server_code (retcode))
     return;
   break;
      }

      sscanf (CommLineIn, "%u %lu %ld %ld %s", &retcode, &estnum, &first, &last, group);

      LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset,
      CommDoc->ParentLineID, &BlockPtr, &LinePtr);
      GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof (TypLine)));
      RangePtr = (TypRange far *) ((char far *) GroupDoc + RangeOffset (GroupDoc->NameLen));
      GroupDoc->Determined = TRUE;

      /* we don't want to grab *that* many! */
      if (estnum >= article_threshold) {
   if (!ShowUnreadOnly)
     { 
     arts_to_retrieve = (int) estnum;
     DlgStatus = DialogBox (hInst, (LPCSTR) "THRESHOLD", CommDoc->hDocWnd, (DLGPROC) lpfnWinVnThresholdDlg);
          if (DlgStatus == FALSE)
             {             
              DestroyWindow (CommDoc->hDocWnd);
         CommBusy = FALSE;
         CommState = ST_NONE;
         GroupDoc->ServerFirst = GroupDoc->ServerLast;
         GroupDoc->ServerEstNum = estnum;
              return;
              }
          }
   if ((arts_to_retrieve > 0)
       && ((last - arts_to_retrieve) > first)) {
     first = (last - arts_to_retrieve) + 1;
   }
   else if (arts_to_retrieve == -1) /* they clicked 'all of them' */
     arts_to_retrieve = (int) estnum;
   /* added by jlg */
   else if ((arts_to_retrieve == -2)   /* they clicked 'unread' */
       ||(ShowUnreadOnly)) {
     if (GroupDoc->nRanges) {
       first = RangePtr[0].Last + 1;
       arts_to_retrieve = (int) ((last - first) + 1);
       if (arts_to_retrieve < 50) {
         arts_to_retrieve = 50;
         first = last - 49;
       }
     }
     else
       arts_to_retrieve = (int) estnum;
   }
      }
      else {
   if (estnum > 0)
     arts_to_retrieve = (int) estnum;
   else {
     MessageBox (hWndConf, "Empty Newsgroup", "WinVN", MB_OK | MB_ICONHAND);
     /* abort the fledgling group window */
     DestroyWindow (CommDoc->hDocWnd);
     InvalidateRect(hWndConf, NULL, FALSE);
     CommBusy = FALSE;
     CommState = ST_NONE;
     GroupDoc->ServerFirst = GroupDoc->ServerLast;
     GroupDoc->ServerEstNum = 0;
     return;
   }
      }

      CommDoc->TotalLines = arts_to_retrieve;

      if (arts_to_retrieve > 0) {
   header_handle =
     GlobalAlloc (GMEM_MOVEABLE, (long)
             ((sizeof (TypHeader)) *
            (long) (arts_to_retrieve)) + sizeof (thread_array *));

   /* allocate space for the header_array index table */
   thread_handle =
     GlobalAlloc (GMEM_MOVEABLE,
             (long) ((sizeof (long)) * (long) (arts_to_retrieve)));

   GroupDoc->header_handle = header_handle;
   GroupDoc->thread_handle = thread_handle;

      }

      /* stick nulls and 0's, etc.. in case display code get mis-threaded */
      initialize_header_array (header_handle, thread_handle, arts_to_retrieve);

      GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));
      GroupDoc->ServerEstNum = estnum;
      GroupDoc->ServerFirst = first;
      GroupDoc->ServerLast = last;

      GlobalUnlock (BlockPtr->hCurBlock);

      if (xoverp) {
   mylen = sprintf (mybuf, "XOVER %ld-%ld", first, last);
   CommState = ST_XOVER_START;
   PutCommLine (mybuf);
      }
      else {
   mylen = sprintf (mybuf, "XHDR from %ld-%ld", first, last);
   CommState = ST_XHDR_FROM_START;
   PutCommLine (mybuf);
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
      sscanf (CommLineIn, "%d", &retcode);
      if (retcode == 224) {
   CommState = ST_XOVER_DATA;
   CommDoc->ActiveLines = 0;
      }
      else {
   mylen = sprintf (mybuf, "XHDR from %ld-%ld", first, last);
   CommState = ST_XHDR_FROM_START;
   PutCommLine (mybuf);
      }
      break;

    case ST_XOVER_DATA:
      if (strcmp (CommLineIn, ".") == 0) {
   /* this is a yuck way to do this */
   CommState = ST_IN_GROUP;
   CommBusy = FALSE;
   finish_header_retrieval (CommDoc);
   InvalidateRect (CommDoc->hDocWnd, NULL, FALSE);
      }
      else {
   char *this_hop, *next_hop;
   char *reference;

   LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset,
        CommDoc->ParentLineID, &BlockPtr, &LinePtr);
   GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof (TypLine)));
   header_handle = GroupDoc->header_handle;
   thread_handle = GroupDoc->thread_handle;

   GlobalUnlock (BlockPtr->hCurBlock);

   /* Lock the header data */
   headers = lock_headers (header_handle, thread_handle);
   header = header_elt (headers, CommDoc->ActiveLines);

   this_hop = CommLineIn;

   /* article number */
   next_hop = strchr (this_hop, '\t');
   *(next_hop++) = (char) NULL;

   header->number = atol (this_hop);

   /* subject */
   this_hop = next_hop;
   next_hop = strchr (this_hop, '\t');
   *(next_hop++) = (char) NULL;

   mylstrncpy (header->subject, this_hop, HEADER_SUBJECT_LENGTH);
   CommDoc->LongestLine = max (CommDoc->LongestLine,
                ARTICLE_SUBJECT_OFFSET +
                (unsigned) lstrlen (header->subject));
   /* author */
   this_hop = next_hop;
   next_hop = strchr (this_hop, '\t');
   *(next_hop++) = (char) NULL;

   ParseAddress (this_hop,
            AddressString, MAXDIALOGSTRING,
            NameString, MAXDIALOGSTRING);

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
   if (reference)
     mylstrncpy (header->references, reference, HEADER_REFERENCES_LENGTH);

   /* bytes (ignored) */
   this_hop = next_hop;
   next_hop = strchr (this_hop, '\t');
   *(next_hop++) = (char) NULL;

   /* lines (last one doesn't have to have the tab */
   this_hop = next_hop;
   header->lines = atoi (this_hop);

   /* set other header fields */
   header->Selected = FALSE;
   header->ArtDoc = (TypDoc *) NULL;
   header->Seen = WasArtSeen (header->number, GroupDoc);

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
      sscanf (CommLineIn, "%d", &retcode);
      total_xhdrs = threadp ? 6 : 4;   /* we do this here to allow */
      /* mid-session change-of-mind  */
      if (retcode < 100)
   break;
      CommState = ST_XHDR_FROM_DATA;
      CommDoc->ActiveLines = 0;
      break;

    case ST_XHDR_FROM_DATA:
      if (strcmp (CommLineIn, ".") == 0) {
   LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset,
        CommDoc->ParentLineID, &BlockPtr, &LinePtr);

   GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof (TypLine)));
   GroupDoc->total_headers = CommDoc->ActiveLines;

   first = GroupDoc->ServerFirst;
   last = GroupDoc->ServerLast;

   GlobalUnlock (BlockPtr->hCurBlock);
   CommDoc->ActiveLines = 0;

   /* Now ask for the date lines */
   mylen = sprintf (mybuf, "XHDR date %ld-%ld", first, last);
   CommState = ST_XHDR_DATE_START;
   PutCommLine (mybuf);
      }
      else {
   /*      char neat_from [80]; */
   /* Access the Group struct, get HANDLE for header data */
   LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset,
        CommDoc->ParentLineID, &BlockPtr, &LinePtr);

   GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof (TypLine)));
   header_handle = GroupDoc->header_handle;
   thread_handle = GroupDoc->thread_handle;

   GlobalUnlock (BlockPtr->hCurBlock);

   /* Lock the header data */
   headers = lock_headers (header_handle, thread_handle);

   sscanf (CommLineIn, "%ld", &artnum);
   header = header_elt (headers, CommDoc->ActiveLines);
   header->number = artnum;

   /* now use some of our nice formatting of email addresses */
   ParseAddress (get_xhdr_line (CommLineIn),
            AddressString, MAXDIALOGSTRING,
            NameString, MAXDIALOGSTRING);

   /* copy that into headers[].from */
   if (FullNameFrom)
     mylstrncpy (header->from, NameString, HEADER_FROM_LENGTH);
   else
     mylstrncpy (header->from, AddressString, HEADER_FROM_LENGTH);

   unlock_headers (header_handle, thread_handle);

   CommDoc->ActiveLines++;

   update_window_title (CommDoc->hDocWnd, group,
              RcvLineCount++,
              CommDoc->TotalLines * total_xhdrs);
      }

      break;

    case ST_XHDR_DATE_START:
      retcode = 0;
      sscanf (CommLineIn, "%d", &retcode);
      if (check_server_code (retcode))
   break;
      CommState = ST_XHDR_DATE_DATA;
      CommDoc->ActiveLines = 0;
      break;

    case ST_XHDR_DATE_DATA:
      if (strcmp (CommLineIn, ".") == 0) {
   LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset,
        CommDoc->ParentLineID, &BlockPtr, &LinePtr);

   GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof (TypLine)));
   GroupDoc->total_headers = CommDoc->ActiveLines;

   first = GroupDoc->ServerFirst;
   last = GroupDoc->ServerLast;

   GlobalUnlock (BlockPtr->hCurBlock);
   CommDoc->ActiveLines = 0;

   /* Now ask for the #of lines */
   mylen = sprintf (mybuf, "XHDR lines %ld-%ld", first, last);
   CommState = ST_XHDR_LINES_START;
   PutCommLine (mybuf);
      }
      else {

   /* Access the Group struct, get HANDLE for header data */
   LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset,
        CommDoc->ParentLineID, &BlockPtr, &LinePtr);
   GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof (TypLine)));
   header_handle = GroupDoc->header_handle;
   thread_handle = GroupDoc->thread_handle;

   GlobalUnlock (BlockPtr->hCurBlock);

   /* Lock the header data */
   headers = lock_headers (header_handle, thread_handle);
   syncnum = sync_artnum (atol (CommLineIn),
             (header_elt (headers, CommDoc->ActiveLines))->number,
                headers,
                GroupDoc);
   if (syncnum >= 0)
     (header_elt (headers, syncnum))->date
       = parse_usenet_date (get_xhdr_line (CommLineIn));

   unlock_headers (header_handle, thread_handle);

   CommDoc->ActiveLines++;
   update_window_title (CommDoc->hDocWnd, group,
              RcvLineCount++,
              CommDoc->TotalLines * total_xhdrs);
      }

      break;

    case ST_XHDR_LINES_START:
      retcode = 0;
      sscanf (CommLineIn, "%d", &retcode);
      if (check_server_code (retcode))
   break;
      CommState = ST_XHDR_LINES_DATA;
      CommDoc->ActiveLines = 0;
      break;

    case ST_XHDR_LINES_DATA:
      if (strcmp (CommLineIn, ".") == 0) {
   LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset,
        CommDoc->ParentLineID, &BlockPtr, &LinePtr);

   GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof (TypLine)));
   GroupDoc->total_headers = CommDoc->ActiveLines;

   first = GroupDoc->ServerFirst;
   last = GroupDoc->ServerLast;

   GlobalUnlock (BlockPtr->hCurBlock);
   CommDoc->ActiveLines = 0;

   /* Check for threading option, if enabled, go to REF & ID */
   /* states first */

   if (threadp) {
     CommState = ST_XHDR_REF_START;
     mylen = sprintf (mybuf, "XHDR references %ld-%ld", first, last);
     PutCommLine (mybuf);
   }
   else {
     CommState = ST_XHDR_SUBJECT_START;
     mylen = sprintf (mybuf, "XHDR subject %ld-%ld", first, last);
     PutCommLine (mybuf);
   }
      }

      else {

   /* Access the Group struct, get HANDLE for header data */
   LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset,
        CommDoc->ParentLineID, &BlockPtr, &LinePtr);
   GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof (TypLine)));

   header_handle = GroupDoc->header_handle;
   thread_handle = GroupDoc->thread_handle;

   GlobalUnlock (BlockPtr->hCurBlock);

   /* Lock the header data */
   headers = lock_headers (header_handle, thread_handle);

   syncnum = sync_artnum (atol (CommLineIn),
             (header_elt (headers, CommDoc->ActiveLines))->number,
                headers,
                GroupDoc);
   if (syncnum >= 0)
     sscanf (CommLineIn, "%ld %Fd", &artnum, &((header_elt (headers, syncnum))->lines));

   unlock_headers (header_handle, thread_handle);
   CommDoc->ActiveLines++;
   update_window_title (CommDoc->hDocWnd, group,
              RcvLineCount++,
              CommDoc->TotalLines * total_xhdrs);
      }

      break;

    case ST_XHDR_REF_START:
      retcode = 0;
      sscanf (CommLineIn, "%d", &retcode);
      if (check_server_code (retcode))
   break;
      CommState = ST_XHDR_REF_DATA;
      CommDoc->ActiveLines = 0;
      break;

    case ST_XHDR_REF_DATA:
      if (strcmp (CommLineIn, ".") == 0) {
   LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset,
        CommDoc->ParentLineID, &BlockPtr, &LinePtr);

   GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));
   GroupDoc->total_headers = CommDoc->ActiveLines;

   first = GroupDoc->ServerFirst;
   last = GroupDoc->ServerLast;

   GlobalUnlock (BlockPtr->hCurBlock);
   CommDoc->ActiveLines = 0;

   /* Now ask for the message-id lines */
   mylen = sprintf (mybuf, "XHDR message-id %ld-%ld", first, last);
   CommState = ST_XHDR_MID_START;
   PutCommLine (mybuf);
      }
      else {
   char far *refer;  /* , far * end,far * bracket1,far *bracket2; */
   /*      int bracket_len; */

   /* Access the Group struct, get HANDLE for header data */
   LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset,
        CommDoc->ParentLineID, &BlockPtr, &LinePtr);
   GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof (TypLine)));

   header_handle = GroupDoc->header_handle;
   thread_handle = GroupDoc->thread_handle;

   GlobalUnlock (BlockPtr->hCurBlock);

   /* Lock the header data */
   headers = lock_headers (header_handle, thread_handle);

   /* for now, we only pay attention to first (whole) referral */
   refer = get_xhdr_line (CommLineIn);

   refer = get_best_reference (refer);

   if (refer) {
     /* Patch to check for bad info from server JD 6/19/93 */
     syncnum = sync_artnum (atol (CommLineIn),
             (header_elt (headers,
                     CommDoc->ActiveLines))->number,
             headers, GroupDoc);
     if (syncnum >= 0)
       mylstrncpy ((header_elt (headers, syncnum))->references,
         refer, HEADER_REFERENCES_LENGTH);
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
      sscanf (CommLineIn, "%d", &retcode);
      if (check_server_code (retcode))
   break;
      CommState = ST_XHDR_MID_DATA;
      CommDoc->ActiveLines = 0;
      break;

    case ST_XHDR_MID_DATA:
      if (strcmp (CommLineIn, ".") == 0) {
   LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset,
        CommDoc->ParentLineID, &BlockPtr, &LinePtr);

   GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));

   GroupDoc->total_headers = CommDoc->ActiveLines;

   first = GroupDoc->ServerFirst;
   last = GroupDoc->ServerLast;

   GlobalUnlock (BlockPtr->hCurBlock);
   CommDoc->ActiveLines = 0;

   /* Now ask for the subject lines */
   mylen = sprintf (mybuf, "XHDR subject %ld-%ld", first, last);
   CommState = ST_XHDR_SUBJECT_START;
   PutCommLine (mybuf);
      }
      else {
   /* Access the Group struct, get HANDLE for header data */
   LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset,
        CommDoc->ParentLineID, &BlockPtr, &LinePtr);

   GroupDoc = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));

   header_handle = GroupDoc->header_handle;
   thread_handle = GroupDoc->thread_handle;

   GlobalUnlock (BlockPtr->hCurBlock);

   /* Lock the header data */
   headers = lock_headers (header_handle, thread_handle);

   syncnum = sync_artnum (atol (CommLineIn),
             (header_elt (headers, CommDoc->ActiveLines))->number,
                headers,
                GroupDoc);
   if (syncnum >= 0)
     mylstrncpy ((header_elt (headers, syncnum))->message_id,
            (char far *) (get_xhdr_line (CommLineIn)),
            HEADER_MESSAGE_ID_LENGTH); /* bad, hardcoded. */

   unlock_headers (header_handle, thread_handle);

   CommDoc->ActiveLines++;
   update_window_title (CommDoc->hDocWnd, group,
              RcvLineCount++,
              CommDoc->TotalLines * total_xhdrs);

      }

      break;


    case ST_XHDR_SUBJECT_START:
      retcode = 0;
      sscanf (CommLineIn, "%d", &retcode);
      if (check_server_code (retcode))
   break;
      CommState = ST_XHDR_SUBJECT_DATA;
      break;

    case ST_XHDR_SUBJECT_DATA:

      if (strcmp (CommLineIn, ".") == 0) {
   CommState = ST_IN_GROUP;
   CommBusy = FALSE;
   finish_header_retrieval (CommDoc);
   InvalidateRect (CommDoc->hDocWnd, NULL, FALSE);
      }
      else {

   artnum = 0;
   sscanf (CommLineIn, "%ld", &artnum);
   if (artnum) {
     LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset,
          CommDoc->ParentLineID, &BlockPtr, &LinePtr);

     GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof (TypLine)));
     header_handle = GroupDoc->header_handle;
     thread_handle = GroupDoc->thread_handle;
     headers = lock_headers (header_handle, thread_handle);

     /* update the seen thing. */
     syncnum = sync_artnum (atol (CommLineIn),
             (header_elt (headers, CommDoc->ActiveLines))->number,
             headers,
             GroupDoc);
     if (syncnum >= 0)
       header = header_elt (headers, syncnum);
     else
       header = header_elt (headers, CommDoc->ActiveLines);

     header->Selected = FALSE;
     header->ArtDoc = (TypDoc *) NULL;
     header->Seen =
       WasArtSeen (artnum, (TypGroup far *) (((char far *) LinePtr) +
                    sizeof (TypLine)));

     UnlockLine (BlockPtr, LinePtr, &(CommDoc->hParentBlock),
            &(CommDoc->ParentOffset), &(CommDoc->ParentLineID));

     mylstrncpy (header->subject,
            get_xhdr_line (CommLineIn), HEADER_SUBJECT_LENGTH);

     CommDoc->LongestLine = max (CommDoc->LongestLine,
                  ARTICLE_SUBJECT_OFFSET +
                  (unsigned) lstrlen (header->subject));

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
      sscanf (CommLineIn, "%d", &retcode);
      if (check_server_code (retcode)) {
   if (CommDoc->hDocWnd)
     DestroyWindow (CommDoc->hDocWnd);
   break;
      }
      CommState = ST_REC_ARTICLE_HEADER;
      /*      Bossanova = FALSE; */
      break;

    case ST_REC_ARTICLE_HEADER:
      if (strcmp (CommLineIn, ".") == 0) {
   ;        /* error: empty article (end in middle of header) */

      }
      if (IsBlankStr (CommLineIn))  /* headers end in blank line */
   CommState = ST_REC_ARTICLE;
      AddCommLineToDoc (CommLineIn);
      break;

    case ST_REC_ARTICLE:
      if (strcmp (CommLineIn, ".") == 0) {   /* article receive complete */

   CommState = ST_IN_GROUP;
   CommBusy = FALSE;

   if (CommDecoding) {
     SendMessage (currentCoded->hParentWnd, IDM_ARTICLE_RETRIEVE_COMPLETE, 0, 0);
     break;
   }
   else
   {
          SendMessage (CommDoc->ParentDoc->hDocWnd, IDM_ARTICLE_RETRIEVE_COMPLETE, 0, 0);
          SendMessage(CommDoc->hDocWnd, IDM_ARTICLE_RETRIEVE_COMPLETE, 0, 0);
        }

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

   mylstrncpy (group, lpsz, MAXGROUPNAME);
   sprintf (mybuf, "%s (%u lines)", group, CommDoc->TotalLines);
   SetWindowText (CommDoc->hDocWnd, mybuf);
   InvalidateRect (CommDoc->hDocWnd, NULL, FALSE);
   GlobalUnlock (BlockPtr->hCurBlock);

   /* Skip to the first line of the text of the article
    * and make sure it's visible on the screen.  This is
    * so that the user doesn't have to have the first
    * screen filled with a lengthy, worthless header.
    *
    * and save number of header lines (on display)
    * for later (Bretherton)
    */
   TopOfDoc (CommDoc, &BlockPtr, &LinePtr);
   found = FALSE;
   do {
     lpsz = ((char far *) LinePtr + sizeof (TypLine) + sizeof (TypText));
     if (IsLineBlank (lpsz)) {
       found = TRUE;
       CommDoc->HeaderLines = WhatLine (BlockPtr, LinePtr);
       break;
     }
     if (!NextLine (&BlockPtr, &LinePtr))
       break;
   }
   while (!found);
   NextLine (&BlockPtr, &LinePtr);

   /* If the line is in the last screen's worth of lines, back
    * up the pointer so it points to the first line of the last
    * screen.
    */
   if (found && CommDoc->TotalLines > CommDoc->ScYLines &&
       !CommDoc->TopScLineID)
     AdjustTopSc (BlockPtr, LinePtr);

   UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
      }
      else        /* not finished, continue article receive */
   AddCommLineToDoc (CommLineIn);

      break;

    case ST_POST_WAIT_PERMISSION:

      /*      WndPost = getWndEdit(WndPosts,CommWnd,MAXPOSTWNDS) ; */
      /*      found = (WndPost != NULL) ; */

      /* lock out changes */
      SendMessage (PostEdit->hWndEdit, EM_SETREADONLY, TRUE, 0L);

      retcode = 0;
      sscanf (CommLineIn, "%u", &retcode);

      if (retcode == 340) {
   if (Attaching && !ReviewAttach)
     ProcessAttach (CONTINUE);
   else
     PostText (PostEdit);
      }
      else {
   check_server_code (retcode);
   MessageBox (PostEdit->hWnd, CommLineIn + 4, "Cannot Post Article",
          MB_OK | MB_ICONEXCLAMATION);
   CommBusy = FALSE;
   CommState = ST_NONE;

   if (Attaching && !ReviewAttach)
     ProcessAttach (ABORT);   /* cancel attachment */

   else
     /* unlock to allow user modification */
     SendMessage (PostEdit->hWndEdit, EM_SETREADONLY, FALSE, 0L);
      }
      break;

    case ST_POST_WAIT_END:

      /*      WndPost = getWndEdit(WndPosts,CommWnd,MAXPOSTWNDS) ; */
      /*      found = (WndPost != NULL) ; */

      /* no check for failure to find posting documents */
      retcode = 0;
      sscanf (CommLineIn, "%d", &retcode);
      if (retcode == 441 || retcode == 440) {
   CompletePost(PostEdit, FAIL);
   done = TRUE;
   nextBatchIndex = 0;  /* cancel any batch */

   if (Attaching && !ReviewAttach)
     ProcessAttach (ABORT);/* cancel attachment */

      }
      else if (retcode == 240) {
   CompletePost(PostEdit, SUCCESS);
   done = TRUE;
      }
      else if (check_server_code (retcode))
   break;

      if (done) {
   CommBusy = FALSE;
   CommState = ST_NONE;
   PostEdit = (WndEdit *) NULL;

   if (nextBatchIndex)  /* if we're sending a batch, send the next */
     BatchSend (DOCTYPE_POSTING);
      }
      if (Attaching && !ReviewAttach)
   ProcessAttach (CONTINUE);
      break;

      /* the following code is for an MRR-hacked nntp server */

    case ST_GROUP_REJOIN:
      CommState = ST_ARTICLE_RESP;
      break;
    }
  }
}

BOOL
isLineQuotation (char *textptr)
{
  char *loc;
  loc = memchr (textptr, QuoteLineInd, 2);
  if (!loc)
    loc = memchr (textptr, '|', 2);
  if (!loc)
    loc = memchr (textptr, ':', 2);
  return (loc != NULL);
}


/*-- function AddCommLineToDoc ---------------------------------------
 *
 *  Adds the given line to the comm doc
 */
void
AddCommLineToDoc (char *line)
{
  TypLine far *LinePtr;
  TypBlock far *BlockPtr;
  char *cptr, *cdest;
  int col, mylen;
  char artline[MAXINTERNALLINE];

  /* special case for lines starting with '..' */
  if (strncmp (CommLineIn, "..", 2))
    cptr = CommLineIn;
  else
    cptr = CommLineIn + 1;

  if (CommDecoding) {
    DecodeLine (currentCoded, cptr);
    return;
  }
  /* Copy this line into an image of a textblock line,
   * expanding tabs.
   */
  cdest = artline + sizeof (TypLine) + sizeof (TypText);
  for (col = 0;
  *cptr && col < (MAXINTERNALLINE - 3 * sizeof (TypLine) - sizeof (TypText));
       cptr++) {
    if (*cptr == '\t') {
      do {
   *(cdest++) = ' ';
      }
      while (++col & 7);
    }
    else {
      *(cdest++) = *cptr;
      col++;
    }
  }
  *(cdest++) = '\0';

  ((TypLine *) artline)->LineID = NextLineID++;
  LockLine (CommDoc->hCurAddBlock, CommDoc->AddOffset, CommDoc->AddLineID, &BlockPtr, &LinePtr);
  mylen = (cdest - artline) + sizeof (int);
  mylen += mylen % 2;
  ((TypText *) (artline + sizeof (TypLine)))->NameLen =
    (cdest - 1) - (artline + sizeof (TypLine) + sizeof (TypText));
  ((TypLine *) artline)->length = mylen;
  ((TypLine *) artline)->active = TRUE;
  *((int *) (artline + mylen - sizeof (int))) = mylen;
  AddLine ((TypLine *) artline, &BlockPtr, &LinePtr);
  CommDoc->LongestLine = max (CommDoc->LongestLine, (unsigned) mylen);
  CommDoc->ActiveLines++;
  UnlockLine (BlockPtr, LinePtr, &(CommDoc->hCurAddBlock),
         &(CommDoc->AddOffset), &(CommDoc->AddLineID));

  if ((CommDoc->TotalLines % UPDATE_ART_FREQ) == 0)
    InvalidateRect (CommDoc->hDocWnd, NULL, FALSE);
}

/*-- function WasArtSeen ---------------------------------------------
 *
 *  Determines whether (according to the information in a TypGroup entry)
 *  a given article number was seen.
 *
 *  Returns TRUE iff the article has been seen.
 */
BOOL
WasArtSeen (ArtNum, GroupPtr)
     unsigned long ArtNum;
     TypGroup far *GroupPtr;
{
  TypRange far *RangePtr = (TypRange far *) ((char far *)
            GroupPtr + RangeOffset (GroupPtr->NameLen));
  unsigned int nr;

  for (nr = 0; nr < GroupPtr->nRanges; nr++) {
    if (ArtNum >= (unsigned long) RangePtr->First &&
   ArtNum <= (unsigned long) RangePtr->Last) {
      return (TRUE);
    }
    else {
      RangePtr++;
    }
  }
  return (FALSE);
}


/*--- function mylstrncmp -----------------------------------------------
 *
 *   Just like strncmp, except takes long pointers.
 */
int
mylstrncmp (ptr1, ptr2, len)
     char far *ptr1;
     char far *ptr2;
     int len;
{
  for (; len--; ptr1++, ptr2++) {
    if (*ptr1 > *ptr2) {
      return (1);
    }
    else if (*ptr1 < *ptr2) {
      return (-1);
    }
  }
  return (0);
}

/*--- function mylstrncpy -----------------------------------------------
 *
 *   Just like strncpy, except takes long pointers.
 */
char far *
mylstrncpy (ptr1, ptr2, len)
     char far *ptr1;
     char far *ptr2;
     int len;
{
  char far *targ = ptr1;

  for (; --len && *ptr2; ptr1++, ptr2++) {
    *ptr1 = *ptr2;
  }
  *ptr1 = '\0';
  return (targ);
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
lstrcmpnoblank (str1, str2)
     char far **str1;
     char far **str2;
{
  register char far *s1 = *str1, far * s2 = *str2;

  for (; *s1 && *s2 && *s1 != ' ' && *s2 != ' '; s1++, s2++) {
    if (*s1 > *s2) {
      return (1);
    }
    else if (*s1 < *s2) {
      return (-1);
    }
  }
  if (*s1 == *s2) {
    return (0);
  }
  else if (*s1) {
    return (1);
  }
  else {
    return (-1);
  }
}
#endif

void
finish_header_retrieval ()
{
  TypLine far *LinePtr;
  TypBlock far *BlockPtr;
  TypGroup far *GroupDoc;
  HANDLE header_handle, thread_handle;
/*  HANDLE hBlock; */
  char far *lpsz;
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
  /* CommDoc->ActiveLines = 0; */
  /* Fetch this group's line in NetDoc so we can get the
   * group's name for the window's title bar.
   */
  LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset,
       CommDoc->ParentLineID, &BlockPtr, &LinePtr);

  GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof (TypLine)));
  header_handle = GroupDoc->header_handle;
  thread_handle = GroupDoc->thread_handle;
  lock_headers (header_handle, thread_handle);

  if (threadp) {
    SetWindowText (CommDoc->hDocWnd, "sorting headers...");
    sort_by_threads (header_handle, thread_handle, CommDoc->TotalLines);
  }

  unlock_headers (header_handle, thread_handle);

  GroupDoc->total_headers = CommDoc->TotalLines;

  lpsz = (char far *) (((char far *) LinePtr) +
             sizeof (TypLine) + sizeof (TypGroup));

  mylstrncpy (group, lpsz, MAXGROUPNAME);
  sprintf (mybuf, "%s (%u articles)", group, CommDoc->TotalLines);
  SetWindowText (CommDoc->hDocWnd, mybuf);

  /* If we have information from NEWSRC on the highest-
   * numbered article previously seen, position the window
   * so the new articles can be seen without scrolling.
   */
  {
    unsigned int i;

    LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset,
         CommDoc->ParentLineID, &BlockPtr, &LinePtr);

    /* inside the lock, can access the GroupStruct */
    GroupDoc = ((TypGroup far *) ((char far *) LinePtr + sizeof (TypLine)));
    header_handle = GroupDoc->header_handle;
    thread_handle = GroupDoc->thread_handle;
    headers = lock_headers (header_handle, thread_handle);

    if (CommDoc->TotalLines > 0)
      for (i = CommDoc->TotalLines - 1;
      ((i > 0) && ((header_elt (headers, i))->Seen == FALSE));
      i--);
    CommDoc->TopLineOrd = (i > 5) ? i - 4 : 0;

    unlock_headers (header_handle, thread_handle);
  }

  SendMessage (CommDoc->hDocWnd, IDM_RETRIEVE_COMPLETE, 0, 0);
  InvalidateRect (CommDoc->hDocWnd, NULL, FALSE);
}

/*
 * Look through the MAIL or Post edits and return the edit with
 * matching window handle Consider - centralising initial window
 * location in wvmail and wndpost using a single array (save passing
 * structure and size into this module)
 *
 */

WndEdit *
getWndEdit (WndEdit * WndEdits, HWND hWnd, int numEntries)
{
  int ih;

  for (ih = 0; ih < numEntries; ih++) {
    if (WndEdits[ih].hWnd == hWnd) {
      return &WndEdits[ih];
    }
  }

  /*MessageBox(0,"getWndEditFound Nothing","mrb debug", MB_OK | MB_ICONHAND); */

  return (WndEdit *) NULL;
}



/* ------------------------------------------------------------------------
 * Replace any white space at end of string with NULL's
 * JSC 11/1/93
 */
void
RemoveTrailingWhiteSpace (char *str)
{
  register int i;

  for (i = strlen (str) - 1; i > 0 && isspace (str[i]); i--)
    str[i] = '\0';
}

/*------------------------------------------------------------------------------
 * IsBlankStr
 * Returns true if the string is entirely whitespace, else false
 * JSC 12/6/93
 */
BOOL
IsBlankStr (char *temp)
{
  register char *ptr;

  for (ptr = temp; *ptr; ptr++)
    if (!isspace (*ptr))
      return (FALSE);
  return (TRUE);
}

/*------------------------------------------------------------------------------
 * isnumber
 * Returns true if the string is a all digits
 * JSC 12/6/93
 */
BOOL
isnumber (char *str)
{
  char *ptr;

  for (ptr = str; *ptr != '\0'; ptr++)
    if (!isdigit (*ptr))
      return (FALSE);
  return (TRUE);
}

/* ------------------------------------------------------------------------
 *    Open the common font dialog
 *      Place resulting selection name and size in face,style and size
 *      Note: to select a printer font, send style as "Printer"
 *      printer font selection ignores any chosen style
 *      (JSC 1/9/94)
 */
BOOL
AskForFont (HWND hParentWnd, char *face, int *size, char *style)
{
  LOGFONT lf;
  CHOOSEFONT cf;
  HDC hDC;

  memset (&lf, 0, sizeof (LOGFONT));
  strcpy (lf.lfFaceName, face);
  /* convert points to logical units (1 pt = 1/72 inch) */
  /* For printer fonts, use ScreenYPixels here anyway - the choosefont */
  /* dialog appears to require the lfHeight to be in screen units */
  /* we will convert point size to PrinterUnits in InitPrinterFonts() */
  lf.lfHeight = -MulDiv (*size, ScreenYPixels, 72);

  memset (&cf, 0, sizeof (CHOOSEFONT));
  cf.lStructSize = sizeof (CHOOSEFONT);
  cf.hwndOwner = hParentWnd;
  cf.lpLogFont = &lf;
  if (!stricmp (style, "Printer")) {
    cf.nFontType = PRINTER_FONTTYPE;
    hDC = GetPrinterDC (hParentWnd);
    cf.hDC = hDC;
    cf.Flags = CF_PRINTERFONTS | CF_INITTOLOGFONTSTRUCT | CF_FORCEFONTEXIST;
  }
  else {
    cf.nFontType = SCREEN_FONTTYPE;
    cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_USESTYLE | CF_FORCEFONTEXIST;
    cf.lpszStyle = style;
  }
  if (!ChooseFont (&cf))
    return (FAIL);

/*      if (!stricmp (style, "Printer"))      // commented out JD 6/17/94 */
  /*         ReleaseDC (hWndConf, hDC);  */

/*      if (!stricmp (style, "Printer")) */
  /*         DeletePrinterDC (hDC); */

  *size = cf.iPointSize / 10; /* iPointSize is in tenths of a point */

  strcpy (face, lf.lfFaceName);
  return (SUCCESS);
}

/* ------------------------------------------------------------------------
 *    Open the common color dialog
 *      (JSC 1/9/94)
 */
BOOL
AskForColor (HWND hParentWnd, COLORREF * color)
{
  CHOOSECOLOR cc;
  COLORREF nearC;
  HDC hDC;

  memset (&cc, 0, sizeof (CHOOSECOLOR));
  cc.lStructSize = sizeof (CHOOSECOLOR);
  cc.hwndOwner = hParentWnd;
  cc.rgbResult = *color;
  cc.lpCustColors = CustomColors;
  cc.Flags = CC_RGBINIT;

  if (!ChooseColor (&cc))
    return (FAIL);

  /* until we figure out how to deal with dithered colors, force */
  /* the color to the nearest physical color */
  hDC = GetDC (hParentWnd);
  nearC = GetNearestColor (hDC, cc.rgbResult);
  if (cc.rgbResult != nearC)
    MessageBox (hParentWnd, "WinVn does not currently support dithered (non-solid) colors.\nThe nearest physical solid color has been selected.",
      "Sorry", MB_OK | MB_ICONINFORMATION);
  *color = nearC;
  ReleaseDC (hParentWnd, hDC);
  return (SUCCESS);
}

/* ------------------------------------------------------------------------
 * This should be used instead of EM_GETHANDLE on global edit buf
 * Returns a string containing the contents of an edit wnd
 * It is the reponsibility of the caller to GlobalFreePtr the string
 * (JSC)
 */
char *
GetEditText (HWND hWndEdit)
{
  unsigned int size;
  char *newText;

  /* SendMessage (hWndEdit, EM_FMTLINES, TRUE, 0); */

  size = (unsigned int) SendMessage (hWndEdit, WM_GETTEXTLENGTH, 0, 0L) + 1;

  if ((newText = (char *) GlobalAllocPtr (GMEM_MOVEABLE, size * sizeof (char))) == NULL) {
    MessageBox (hWndEdit, "Memory allocation failure", "Edit Text", MB_OK);
    return (NULL);
  }
  if (SendMessage (hWndEdit, WM_GETTEXT, size, (LPARAM) ((LPCSTR) newText)) != (long) (size - 1)) {
    MessageBox (hWndEdit, "Failed to get text", "Edit Text", MB_OK);
    return (NULL);
  }

  return (newText);
}
LRESULT
SetEditText (HWND hWndEdit, char *editMem)
{
  return (SendMessage (hWndEdit, WM_SETTEXT, 0, (LPARAM) (LPCSTR) editMem));
}

/* ------------------------------------------------------------------------
 * To maximize amount of data allowable in posting, allocate a new
 * data segment for this posting from the Global heap, and set the
 * window hInstance to this segment
 * Don't forget you can't use EM_GET/SETHANDLE on an edit buf with
 * a global mem segment
 */
BOOL
CreateEditWnd (WndEdit * NewWnd)
{
#ifndef _WIN32
  GLOBALHANDLE editDS;
  LPVOID lpPtr;
  if ((editDS = GlobalAlloc (GMEM_DDESHARE | GMEM_MOVEABLE | GMEM_ZEROINIT, 1024L)) == NULL) {
    MessageBox (NewWnd->hWnd, "Memory allocation failure", "Edit Buffer", MB_OK);
    editDS = hInst;     /* use local heap instead */

  }
  else {
    lpPtr = GlobalLock (editDS);
    LocalInit (HIWORD ((LONG) lpPtr), 0, (WORD) (GlobalSize (editDS) - 16));
    UnlockSegment (HIWORD ((LONG) lpPtr));   /* we still have a global lock */

  }
  NewWnd->hWndEdit = CreateWindow ((LPCSTR) "edit", (LPCSTR) NULL,
          WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_BORDER |
         ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
               0, 0, 0, 0,
          NewWnd->hWnd, (HMENU) EDITID, (HINSTANCE) HIWORD ((LONG) lpPtr), NULL);
#else
  NewWnd->hWndEdit = CreateWindow ("edit", (char *) NULL,
          WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_BORDER |
         ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
               0, 0, 0, 0,
               NewWnd->hWnd, (HMENU) EDITID, hInst, NULL);  /* No problems with Win32 */
#endif
  if (!NewWnd->hWndEdit) {
    MessageBox (NewWnd->hWnd, "Window creation failure", "Edit Buffer", MB_OK);
    return (FAIL);
  }

  SendMessage (NewWnd->hWndEdit, EM_LIMITTEXT, 0, 0L);
  SetHandleBkBrush (NewWnd->hWndEdit, hArticleBackgroundBrush);
  NewWnd->dirty = DT_CLEAN;

  return (SUCCESS);
}
/* ------------------------------------------------------------------------
 *    Write an integer to the private profile
 */
BOOL
WritePrivateProfileInt (lpAppName, lpKeyName, intval, lpProFile)
     char far *lpAppName;
     char far *lpKeyName;
     char far *lpProFile;
     int intval;
{
  char msg[20];

  itoa (intval, msg, 10);
  return (WritePrivateProfileString (lpAppName, lpKeyName, msg, lpProFile));
}

/* ------------------------------------------------------------------------
 *    Write an unsigned integer to the private profile
 *      (JSC 1/8/94)
 */
BOOL
WritePrivateProfileUInt (lpAppName, lpKeyName, intval, lpProFile)
     char far *lpAppName;
     char far *lpKeyName;
     char far *lpProFile;
     unsigned int intval;
{
  char msg[20];

  uitoa (intval, msg, 10);
  return (WritePrivateProfileString (lpAppName, lpKeyName, msg, lpProFile));
}

/* ------------------------------------------------------------------------
 *    Get an unsigned integer to the private profile
 *      (JSC 1/8/94)
 */
unsigned int
GetPrivateProfileUInt (lpAppName, lpKeyName, intval, lpProFile)
     char far *lpAppName;
     char far *lpKeyName;
     char far *lpProFile;
     unsigned int intval;
{
  char buf[20];

  GetPrivateProfileString (lpAppName, lpKeyName, "", buf, 20, lpProFile);

  if (buf[0] == '\0')
    return (intval);
  else
    return (atoui (buf));
}
/* ------------------------------------------------------------------------
 *    Refresh Window functions
 *      Called after a font/color selection has changed to affect all
 *      windows of a certain type (group/article/status)
 *      (JSC 1/9/94)
 */
void
RefreshGroupWnds ()
{
  register int i;
  for (i = 0; i < MAXGROUPWNDS; i++)
    if (GroupDocs[i].InUse && GroupDocs[i].hDocWnd) {
      SetHandleBkBrush (GroupDocs[i].hDocWnd, hListBackgroundBrush);
      SendMessage (GroupDocs[i].hDocWnd, WM_SIZE, 0, 0L);
      InvalidateRect (GroupDocs[i].hDocWnd, NULL, TRUE);
    }
}

void
RefreshArticleWnds ()
{
  register int i;

  for (i = 0; i < MAXARTICLEWNDS; i++)
    if (ArticleDocs[i].InUse && ArticleDocs[i].hDocWnd) {
      SetHandleBkBrush (ArticleDocs[i].hDocWnd, hArticleBackgroundBrush);
      SendMessage (ArticleDocs[i].hDocWnd, WM_SIZE, 0, 0L);
      InvalidateRect (ArticleDocs[i].hDocWnd, NULL, TRUE);
    }

  for (i = 0; i < MAXPOSTWNDS; i++)
    if (WndPosts[i].hWnd) {
      SendMessage (WndPosts[i].hWndEdit, WM_SETFONT, (WPARAM) hFontArtNormal, TRUE);
      SetHandleBkBrush (WndPosts[i].hWndEdit, hArticleBackgroundBrush);
      InvalidateRect (WndPosts[i].hWndEdit, NULL, TRUE);
    }

  for (i = 0; i < MAXMAILWNDS; i++)
    if (WndMails[i].hWnd) {
      SendMessage (WndMails[i].hWndEdit, WM_SETFONT, (WPARAM) hFontArtNormal, TRUE);
      SetHandleBkBrush (WndMails[i].hWndEdit, hArticleBackgroundBrush);
      InvalidateRect (WndMails[i].hWndEdit, NULL, TRUE);
    }

}
void
RefreshStatusWnds ()
{
  register int i;

  for (i = 0; i < NumStatusTexts; i++)
    if (CodingStatusText[i]->hTextWnd) {
      SetHandleBkBrush (CodingStatusText[i]->hTextWnd, hStatusBackgroundBrush);
      SendMessage (CodingStatusText[i]->hTextWnd, WM_SIZE, 0, 0L);
      InvalidateRect (CodingStatusText[i]->hTextWnd, NULL, TRUE);
    }
  if (hCodedBlockWnd) {
    SetHandleBkBrush (hCodedBlockWnd, hStatusBackgroundBrush);
    SendMessage (hCodedBlockWnd, WM_SIZE, 0, 0L);
    InvalidateRect (hCodedBlockWnd, NULL, TRUE);
  }

}
/* ------------------------------------------------------------------------
 *    Close Window functions
 *      Batch operation, close all windows of a certain type
 *      (group/article/status)
 *      (JSC 1/18/94)
 */
void
CloseGroupWnds ()
{
  register int i;
  for (i = 0; i < MAXGROUPWNDS; i++)
    if (GroupDocs[i].InUse && GroupDocs[i].hDocWnd && (!CommBusy || &GroupDocs[i] != CommDoc))
      SendMessage (GroupDocs[i].hDocWnd, WM_CLOSE, 0, 0L);
}

void
CloseArticleWnds ()
{
  register int i;

  for (i = 0; i < MAXARTICLEWNDS; i++)
    if (ArticleDocs[i].InUse && ArticleDocs[i].hDocWnd && (!CommBusy || &ArticleDocs[i] != CommDoc))
      SendMessage (ArticleDocs[i].hDocWnd, WM_CLOSE, 0, 0L);
}
void
CloseStatusWnds ()
{
  /* destroying a coding status text is like popping from a stack */
  /* so we just loop while the top of the stack still exists */
  int numSkipped = 0;
  while (numSkipped < NumStatusTexts && CodingStatusText[numSkipped]->hTextWnd)
    if (!CodingStatusText[numSkipped]->IsBusy)
      SendMessage (CodingStatusText[numSkipped]->hTextWnd, WM_CLOSE, 0, 0L);
    else
      numSkipped++;
#if 0
    if (CodingState) {
      MessageBox (CodingStatusText[0]->hTextWnd,
        "Please wait until en/decoding is complete",
        "Cannot close status window", MB_OK | MB_ICONSTOP);
      break;
    }
    else
      SendMessage (CodingStatusText[0]->hTextWnd, WM_CLOSE, 0, 0L);
#endif
}

/* ------------------------------------------------------------------------
 *    CascadeWindows (and helper CascadeWnd)
 *      cascade em
 *      jsc 9/18/94
 */
HWND
CascadeWnd(HWND hWnd, HWND prevWnd, int nthWnd, int width, int height, int maxX, int maxY)
{
  short x, y;
  
//  if (IsMaximized(hWnd))
//     ShowWindow(hWnd, SW_SHOWNORMAL);

  x = (nthWnd * 12) % maxX;
  y = (nthWnd * (CaptionHeight+2)) % maxY;
  SetWindowPos(hWnd, prevWnd, x, y, width, height, SWP_DRAWFRAME);
  
  return hWnd;
}

void
CascadeWindows()
{
  register int i;
  int nthWnd, width, height, maxX, maxY;
  HWND prevWnd;

  width = (int)(xScreen>>1);
  height = (int)(yScreen>>1);
  maxX = 3 * (width>>1);	/* 3/4 screen width  */
  maxY = 3 * (height>>1);	/* 3/4 screen height */
  
  prevWnd = CascadeWnd(hWndConf, (HWND)NULL, 1, width, height, maxX, maxY);
  nthWnd = 2;
  for (i = 0; i < MAXGROUPWNDS; i++)
    if (GroupDocs[i].InUse && GroupDocs[i].hDocWnd && !IsMinimized(GroupDocs[i].hDocWnd)) {
		prevWnd = CascadeWnd(GroupDocs[i].hDocWnd, prevWnd, nthWnd, width, height, maxX, maxY);
        nthWnd++;
	}
    
  for (i = 0; i < MAXARTICLEWNDS; i++)
    if (ArticleDocs[i].InUse && ArticleDocs[i].hDocWnd && !IsMinimized(ArticleDocs[i].hDocWnd)) {
		prevWnd = CascadeWnd(ArticleDocs[i].hDocWnd, prevWnd, nthWnd, width, height, maxX, maxY);
        nthWnd++;
	}

  for (i = 0; i < MAXPOSTWNDS; i++)
    if (WndPosts[i].hWnd && !IsMinimized(WndPosts[i].hWnd)) {
		prevWnd = CascadeWnd(WndPosts[i].hWnd, prevWnd, nthWnd, width, height, maxX, maxY);
        nthWnd++;
	}

  for (i = 0; i < MAXMAILWNDS; i++)
    if (WndMails[i].hWnd && !IsMinimized(WndMails[i].hWnd)) {
		prevWnd = CascadeWnd(WndMails[i].hWnd, prevWnd, nthWnd, width, height, maxX, maxY);
        nthWnd++;
	}

  for (i = 0; i < NumStatusTexts; i++)
    if (CodingStatusText[i]->hTextWnd && !IsMinimized(CodingStatusText[i]->hTextWnd)) {
		prevWnd = CascadeWnd(CodingStatusText[i]->hTextWnd, prevWnd, nthWnd, width, height, maxX, maxY);
        nthWnd++;
	}

  /* move coded block status window to top center */
  if (hCodedBlockWnd)
    SetWindowPos(hCodedBlockWnd, (HWND)NULL, (xScreen-STATUSWIDTH)>>1, 1, STATUSWIDTH, STATUSHEIGHT, SWP_DRAWFRAME);
  
}
/* ------------------------------------------------------------------------
 *    BatchSend
 *      type is DOCTYPE_MAIL or _POST
 *      Increments nextBatchIndex and initiates mail/post
 *      Note: on entry nextBatchIndex is the index we will use for this send
 *      on exit, nextBatchIndex is either 0 (no more to send) or the index
 *      of the next mail/post to send
 *      (JSC 1/18/94)
 */
void
BatchSend (int type)
{
  int thisSend;
  int maxWnds;
  WndEdit *WndEdits;

  if (type == DOCTYPE_POSTING) {
    WndEdits = WndPosts;
    maxWnds = MAXPOSTWNDS;
  }
  else {
    WndEdits = WndMails;
    maxWnds = MAXMAILWNDS;
  }

  thisSend = nextBatchIndex;
  if (thisSend == 0) {     /* find first in batch (if any) */
    while (thisSend < maxWnds)
      if (WndEdits[thisSend].hWnd)
   break;
      else
   thisSend++;

    if (thisSend == maxWnds)
      return;        /* no open windows.  cancel */

    nextBatchIndex = thisSend;
  }

  /* find next in batch (if any) */
  while (++nextBatchIndex < maxWnds)
    if (WndEdits[nextBatchIndex].hWnd)
      break;

  if (nextBatchIndex == maxWnds)
    nextBatchIndex = 0;    /* no more */

  if (type == DOCTYPE_POSTING)
    StartPost (&WndEdits[thisSend]);
  else
    StartMail (&WndEdits[thisSend]);
}

/* ------------------------------------------------------------------------
 *    Test busy functions
 *      Called to test if a comm or decoding is busy
 *      Returns true if busy, false if not busy
 *      (JSC 1/9/94)
 */
BOOL
TestCommBusy (HWND hParentWnd, char *msg)
{
  if (CommBusy) {
    MessageBox (hParentWnd,
      "Sorry, I am already busy communicating with the server.\n"
      "Try again in a little while.", msg,
      MB_OK | MB_ICONASTERISK);
    return (TRUE);
  }
  else
    return (FALSE);
}

BOOL
TestDecodeBusy (HWND hParentWnd, char *msg)
{
  if (Decoding || CommDecoding) {
    MessageBox (hParentWnd,
        "Sorry, I can only handle one en/decoding session at a time.\n"
      "Try again in a little while.", msg,
      MB_OK | MB_ICONASTERISK);
    return (TRUE);
  }
  else
    return (FALSE);
}

/* ------------------------------------------------------------------------
 *    	Update the mail menus -- called on mail transport change
 *		jsc 9/9/94
 */
void
UpdateAllMailMenus()
{
	register int i;
	
	SetMainMailMenu(hWndConf);
    for (i = 0; i < MAXARTICLEWNDS; i++)
	    if (ArticleDocs[i].hDocWnd) 
	    	SetArticleMailMenu(ArticleDocs[i].hDocWnd);

	for (i = 0; i < MAXGROUPWNDS; i++)
		if (GroupDocs[i].hDocWnd)
	    	SetGroupMailMenu(GroupDocs[i].hDocWnd);
}   	
