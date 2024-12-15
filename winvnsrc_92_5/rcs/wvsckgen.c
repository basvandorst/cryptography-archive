head	1.17;
access;
symbols;
locks; strict;
comment	@ * @;


1.17
date	94.08.11.20.16.41;	author rushing;	state Exp;
branches;
next	1.16;

1.16
date	94.08.10.21.27.53;	author rushing;	state Exp;
branches;
next	1.15;

1.15
date	94.08.04.17.59.36;	author rushing;	state Exp;
branches;
next	1.14;

1.14
date	94.08.03.22.16.25;	author dumoulin;	state Exp;
branches;
next	1.13;

1.13
date	94.07.12.22.19.04;	author rushing;	state Exp;
branches;
next	1.12;

1.12
date	94.07.12.21.42.48;	author rushing;	state Exp;
branches;
next	1.11;

1.11
date	94.06.10.01.10.21;	author rushing;	state Exp;
branches;
next	1.10;

1.10
date	94.06.09.21.47.08;	author rushing;	state Exp;
branches;
next	1.9;

1.9
date	94.06.09.18.39.34;	author rushing;	state Exp;
branches;
next	1.8;

1.8
date	94.06.07.01.33.36;	author rushing;	state Exp;
branches;
next	1.7;

1.7
date	94.06.02.00.11.48;	author rushing;	state Exp;
branches;
next	1.6;

1.6
date	94.05.27.01.18.12;	author rushing;	state Exp;
branches;
next	1.5;

1.5
date	94.05.23.19.51.51;	author rushing;	state Exp;
branches;
next	1.4;

1.4
date	94.05.19.02.03.05;	author rushing;	state Exp;
branches;
next	1.3;

1.3
date	94.03.22.22.44.55;	author rushing;	state Exp;
branches;
next	1.2;

1.2
date	94.03.22.20.43.06;	author rushing;	state Exp;
branches;
next	1.1;

1.1
date	94.02.04.02.17.15;	author rushing;	state Exp;
branches;
next	;


desc
@generic socket DLL interface
@


1.17
log
@better handling of nntp disconnect
@
text
@/* Interface to GENSOCK module */
/* $Id: wvsckgen.c 1.16 1994/08/10 21:27:53 rushing Exp rushing $ */

#include <windows.h>
#include "wvglob.h"
#include "winvn.h"
#include <stdlib.h>
#include <stdio.h> /* for sprintf */
#include <stdarg.h> /* for varargs stuff */

#define MAXSENDLEN 32767
#define COMMBUFSIZE	1600

/* generic socket DLL support */
#include "gensock.h"

char	CommBuff[COMMBUFSIZE];
int	CommBuffIdx = COMMBUFSIZE + 1;
int	CharsInCommBuff = 0;

socktag	NNTPSock;
socktag SMTPSock;
char my_hostname[1024];

/* functions the rest of winvn expects to see */
void PutCommLine(char *line);
void PutCommData(char *data, unsigned int length);

int MRRReadComm(void);
void MRRCloseComm(void);
int MRRInitComm(void);
int send_smtp_message(char * destination, char * message);
int forward_smtp_message (char * destination, TypDoc * Doc);

/* internal to this file */
int close_smtp_socket(void);
int open_smtp_socket(void);
void smtp_error (char * message);
int send_smtp_article (char * destination, TypDoc *Doc);
int send_smtp_edit_data (char * message);
int prepare_smtp_message (char * destination);
int finish_smtp_message (void);
int get_smtp_line (void);
int put_smtp_line (socktag sock, char far * line, unsigned int nchars);

HANDLE	gensock_lib = 0;

int (FAR PASCAL *pgensock_connect) (char FAR * hostname, char FAR * service, socktag FAR * pst);
int (FAR PASCAL *pgensock_getchar) (socktag st, int wait, char FAR * ch);
int (FAR PASCAL *pgensock_put_data) (socktag st, char FAR * data, unsigned long length);
int (FAR PASCAL *pgensock_close) (socktag st);
int (FAR PASCAL *pgensock_gethostname) (char FAR * name, int namelen);
int (FAR PASCAL *pgensock_put_data_buffered) (socktag st, char FAR * data, unsigned long length);
int (FAR PASCAL *pgensock_put_data_flush) (socktag st);


/*-- function MRRReadComm ---------------------------------------
 *
 *  Reads characters from the NNTP Socket
 */

#define ERROR_BUFFER_SIZE 1024

void
complain (char *fmt, ... )
{
  va_list marker;
  char    message[ERROR_BUFFER_SIZE];
  int done = 0;

  /* Write text to a string and output the string. */
  va_start(marker, fmt);
  _vsnprintf(message, ERROR_BUFFER_SIZE, fmt, marker);
  va_end(marker);
  MessageBox (hWndConf, message, "WinVN GenSock Library Error", MB_OK);
}

void
gensock_error (char * function, int retval)
{
  char error_string[512];
  int len;

  len = LoadString (hInst, retval, error_string, sizeof(error_string));
  complain ("error %d in function '%s', '%s'",
	    retval,
	    function,
	    len == 0 ? "[couldn't get error string]" : error_string);
}

int MRRReadComm()
{
  int retval;
  unsigned char ch;

  /* we set wait = 1, sortof non-blocking */
  if ((retval = (*pgensock_getchar) (NNTPSock, 1, &ch))) {
    switch (retval) {
    case WAIT_A_BIT:
    case ERR_BUSY:
      break;
    case ERR_NOT_CONNECTED:
      Initializing = INIT_NOT_CONNECTED;
      SendMessage (hWndConf, (UINT) WM_COMMAND,
    		   (WPARAM) IDM_DISCONNECT, (LPARAM) 0);
      complain ("NNTP server has closed the connection");
      return (-1);
      break;
    default:
      gensock_error ("gensock_getchar", retval);
    }
    return (-1);
  }
  else
    return ((int)ch);
}

int
putline_internal (socktag sock, char * line, unsigned int nchars)
{
  int retval;

  if ((retval =
       (*pgensock_put_data) (sock,
			    (char FAR *) line,
			    (unsigned long) nchars))) {
    switch (retval) {
    case ERR_NOT_CONNECTED:
      Initializing = INIT_NOT_CONNECTED;
      SendMessage (hWndConf, (UINT) WM_COMMAND,
    		   (WPARAM) IDM_DISCONNECT, (LPARAM) 0);
      complain ("NNTP server has closed the connection");
      return (-1);
      break;

    default:
      gensock_error ("gensock_put_data", retval);
    }
    return (-1);
  }
  return (0);
}

/* PutCommLine takes a string, sends it, followed by
 * \r\n.  All one-line sends in winvn should use this.
 */

void
PutCommLine (char * line)
{
  putline_internal (NNTPSock, line, lstrlen(line));
  putline_internal (NNTPSock, "\r\n", 2);
}

/* Use this function for sending larger chunks of data.
 * the text data from an Edit class window can be sent
 * using this function
 */

void
PutCommData (char * data, unsigned int length)
{
  putline_internal (NNTPSock, data, length);
}

/*-- function MRRCloseComm -----------------------------------------
 *
 *  Close the communications port, serial or TCP.
 */
void MRRCloseComm ()
{
  (*pgensock_close)(NNTPSock);
  NNTPSock = (socktag) 0;
  FreeLibrary(gensock_lib);
  gensock_lib = 0;
}

int
load_gensock()
{
  if ((gensock_lib = LoadLibrary(GenSockDLL)) < 32) {
    char message[256];
    wsprintf (message, "Couldn't load '%s'", GenSockDLL);
    complain (message);
    gensock_lib = 0;
    return (-1);
  }

  if (((FARPROC)pgensock_connect =
       GetProcAddress(gensock_lib, "gensock_connect")) == NULL) {
    complain ("couldn't getprocaddress for gensock_connect");
    return (-1);
  }

  if (((FARPROC)pgensock_getchar =
       GetProcAddress(gensock_lib, "gensock_getchar")) == NULL) {
    complain ("couldn't getprocaddress for gensock_getchar");
    return (-1);
  }

  if (((FARPROC)pgensock_put_data =
       GetProcAddress(gensock_lib, "gensock_put_data")) == NULL) {
    complain ("couldn't getprocaddress for gensock_put_data");
    return (-1);
  }

  if (((FARPROC)pgensock_close =
       GetProcAddress(gensock_lib, "gensock_close")) == NULL) {
    complain ("couldn't getprocaddress for gensock_close");
    return (-1);
  }

  if (((FARPROC)pgensock_gethostname =
       GetProcAddress(gensock_lib, "gensock_gethostname")) == NULL) {
    complain ("couldn't getprocaddress for gensock_gethostname");
    return (-1);
  }

  if (((FARPROC)pgensock_put_data_buffered =
       GetProcAddress(gensock_lib, "gensock_put_data_buffered")) == NULL) {
    complain ("couldn't getprocaddress for gensock_put_data_buffered");
    return (-1);
  }

  if (((FARPROC)pgensock_put_data_flush =
       GetProcAddress(gensock_lib, "gensock_put_data_flush")) == NULL) {
    complain ("couldn't getprocaddress for gensock_put_data_flush");
    return (-1);
  }

  return (0);
}


int
MRRInitComm()
{
  int retval;

  /* load the library if it's not loaded */
  if (!gensock_lib) {
    if ((retval = load_gensock())) {
      return (retval);
    }
  }

  /* to keep our sanity, we will quietly try '119' in case it fails */
  if ((retval = (*pgensock_connect) ((LPSTR)NNTPHost,
				     (LPSTR)NNTPService,
				     &NNTPSock))) {
    if (retval == ERR_CANT_RESOLVE_SERVICE) {
      if ((retval = (*pgensock_connect) ((LPSTR)NNTPHost,
					 (LPSTR)"119",
					 &NNTPSock))) {
	gensock_error ("gensock_connect", retval);
	return (-1);
      }
    }
    /* error other than can't resolve service */
    else {
      gensock_error ("gensock_connect", retval);
      return (-1);
    }
  }

  IgnoreCommCh = '\r';
  EOLCommCh = '\n';

  CommLinePtr = CommLineIn;
  CommState = ST_ESTABLISH_COMM;
  CommBusy = FALSE;
  return (0);
}

/* ---------------------------------------------------------------------------
 * transform_and_send_edit_data
 */

int transform_and_send_edit_data (socktag sock, char * editptr)
{
  char *index;
  char *header_end;
  char previous_char = 'x';
  unsigned int send_len;
  int retval;
  BOOL done = 0;

  send_len = lstrlen(editptr);
  index = editptr;

  header_end = strstr (editptr, "\r\n\r\n");

  while (!done) {
    /* room for extra char for double dot on end case */
    while ((unsigned int) (index - editptr) < send_len) {
      switch (*index) {
      case '.':
	if (previous_char == '\n') {
	  /* send _two_ dots... */
	  if ((retval = (*pgensock_put_data_buffered) (sock, index, 1))) {
	    return (retval);
	  }
	}
	if ((retval = (*pgensock_put_data_buffered) (sock, index, 1))) {
	  return (retval);
	}
	break;
      case '\r':
	/* watch for soft-breaks in the header, and ignore them */

	if (index < header_end && (strncmp (index, "\r\r\n", 3) == 0)) {
	  index = index + 2;
	  break;
	}
	else {
	  if (previous_char != '\r') {
	    if ((retval = (*pgensock_put_data_buffered) (sock, index, 1))) {
	      return (retval);
	    }
	  }
	  /* soft line-break (see EM_FMTLINES), skip extra CR */
	  break;
	}

      default:
	if ((retval = (*pgensock_put_data_buffered) (sock, index, 1))) {
	  return (retval);
	}
      }
    previous_char = *index;
    index++;
    }
    if ( (unsigned int) (index - editptr) == send_len) {
      done = 1;
    }
  }

  /* this handles the case where the user doesn't end the last
   * line with a <return>
   */

  if (editptr[send_len-1] != '\n') {
    if ((retval = (*pgensock_put_data_buffered) (sock, "\r\n.\r\n", 5))) {
      return (retval);
    }
  }
  else {
    if ((retval = (*pgensock_put_data_buffered) (sock, ".\r\n", 3))) {
      return (retval);
    }
  }

  /* now make sure it's all sent... */
  if ((retval = (*pgensock_put_data_flush)(sock))) {
    return (retval);
  }
  return (TRUE);
}

int
post_edit_data (char * edit_data)
{
  return (transform_and_send_edit_data (NNTPSock, edit_data));
}

int open_smtp_socket ()
{
  int retval;


  /* load the library if it's not loaded */
  if (!gensock_lib) {
    if ((retval = load_gensock())) {
      return (retval);
    }
  }

  if ((retval = (*pgensock_connect) ((LPSTR)SMTPHost,
				     (LPSTR)"smtp",
				     &SMTPSock))) {
    if (retval == ERR_CANT_RESOLVE_SERVICE) {
      if ((retval = (*pgensock_connect) ((LPSTR)SMTPHost,
					 (LPSTR)"25",
					 &SMTPSock))) {
	gensock_error ("gensock_connect", retval);
	return (-1);
      }
    }
    /* error other than can't resolve service */
    else {
      gensock_error ("gensock_connect", retval);
      return (-1);
    }
  }

  /* we wait to do this until here because WINSOCK is
   * guaranteed to be already initialized at this point.
   */

  /* get the local hostname (needed by SMTP) */
  if ((retval = (*pgensock_gethostname) (my_hostname, sizeof(my_hostname)))) {
    gensock_error ("gensock_gethostname", retval);
    return (-1);
  }

}

int close_smtp_socket()
{
  int retval;

  if ((retval = (*pgensock_close) (SMTPSock))) {
    gensock_error ("gensock_close", retval);
    return (-1);
  }

  /* in case we're _not_ connected to a NNTP server, and
   * we just used SMTP mail only, then we probably want to
   * close the library...
   */

  if (!NNTPSock) {
    FreeLibrary(gensock_lib);
    gensock_lib = 0;
  }

  return (0);
}

/* 'destination' is the address the message is to be sent to */
/* 'message' is a pointer to a null-terminated 'string' containing the */
/* entire text of the message. */

int prepare_smtp_message(char * destination)
{
  char out_data[MAXOUTLINE];
  char *ptr;
  int len, startLen;

  if (open_smtp_socket())
    return (-1);

  if (get_smtp_line() != 220) {
    smtp_error ("SMTP server error");
    return(-1);
  }

  sprintf (out_data, "HELO %s\r\n", my_hostname);
  put_smtp_line (SMTPSock, out_data, strlen (out_data));

  if (get_smtp_line() != 250) {
    smtp_error ("SMTP server error");
    return (-1);
  }

  sprintf (out_data, "MAIL From:<%s>\r\n", MailAddress);
  put_smtp_line (SMTPSock, out_data, strlen (out_data));

  if (get_smtp_line() != 250) {
    smtp_error ("The mail server doesn't like the sender name, "
		"have you set your mail address correctly?");
    return (-1);
  }

  // do a series of RCPT lines for each name in address line
  for (ptr = destination; *ptr; ptr += len + 1)
  {
    // if there's only one token left, then len will = startLen,
    // and we'll iterate once only
    startLen = strlen (ptr);
    if ((len = strcspn (ptr, " ,\n\t\r")) != startLen)
    {
      ptr[len] = '\0';			// replace delim with NULL char
      while (strchr (" ,\n\t\r", ptr[len+1]))	// eat white space
        ptr[len++] = '\0';
    }

    sprintf (out_data, "RCPT To: <%s>\r\n", ptr);
    putline_internal (SMTPSock, out_data, strlen (out_data));

    if (get_smtp_line() != 250)
    {
      sprintf (str, "The mail server doesn't like the name %s.  "
                    "Have you set the 'To: ' field correctly?", ptr);
      smtp_error (str);
      return (-1);
    }

    if (len == startLen)	// last token, we're done
      break;
  }

  sprintf (out_data, "DATA\r\n");
  put_smtp_line (SMTPSock, out_data, strlen (out_data));

  if (get_smtp_line() != 354) {
    smtp_error ("Mail server error accepting message data");
    return (-1);
  }

  return(0);

}

int send_smtp_edit_data (char * message)
{
  transform_and_send_edit_data (SMTPSock, message);

  if (get_smtp_line() != 250) {
    smtp_error ("Message not accepted by server");
    return (-1);
  }
  return(0);
}

int
finish_smtp_message () {
  return (put_smtp_line (SMTPSock, "QUIT\r\n", 6));
}

int get_smtp_line ()
{
  char ch = '.';
  char in_data [MAXOUTLINE];
  char * index;
  int retval = 0;

  index = in_data;

  while (ch != '\n') {
    if ((retval = (*pgensock_getchar) (SMTPSock, 0, &ch))) {
      gensock_error ("gensock_getchar", retval);
      return (-1);
    }
    else {
      *index = ch;
      index++;
    }
  }

  /* this is to support multi-line responses, common with */
  /* servers that speak ESMTP */

  /* I know, I know, it's a hack 8^) */
  if (in_data[3] == '-')
    return (get_smtp_line());
  else
    return (atoi(in_data));
}

int put_smtp_line (socktag sock, char far * line, unsigned int nchars)
{
  int retval;

  if ((retval = (*pgensock_put_data) (sock, line, (unsigned long) nchars))) {
    gensock_error ("gensock_put_data", retval);
    return (-1);
  }
  return (0);
}

void smtp_error (char * message)
{
  MessageBox (hWndConf, message, "WinVN", MB_OK);
  put_smtp_line (SMTPSock, "QUIT\r\n", 6);
  close_smtp_socket();
}


int send_smtp_message (char * destination, char * message)
{
  if (!prepare_smtp_message (destination))
    if (!send_smtp_edit_data (message))
      if (!finish_smtp_message ()) {
	close_smtp_socket();
	return (0);
      }

  close_smtp_socket();
  return (1);
}

int forward_smtp_message (char * destination, TypDoc * Doc)
{
  int retval;

  if ((retval = prepare_smtp_message (destination))) {
    complain ("prepare_smtp_message returned %d\r\n", retval);
    return (retval);
  }
  else if ((retval = send_smtp_article (destination, Doc))) {
    complain ("send_smtp_article returned %d\r\n", retval);
    return (retval);
  }
  else if ((retval = finish_smtp_message())) {
    complain ("finish_smtp_message() returned %d\r\n", retval);
    close_smtp_socket();
    return (retval);
  }
  close_smtp_socket();
  return (1);
}

// testing new buffered stuff...

int send_smtp_article (char * destination, TypDoc * Doc)
{
  HANDLE hBlock;
  unsigned int Offset;
  TypLineID MyLineID;
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
  int retval;
  char line [MAXOUTLINE];

  _snprintf(line,
	    MAXOUTLINE,
	    "From: %s (%s)\r\n",
	    MailAddress,
	    UserName);


  if ((retval =
       (*pgensock_put_data_buffered) (SMTPSock, line, lstrlen(line)))) {
    return (retval);
  }

  _snprintf(line,
	    MAXOUTLINE,
	    "To: %s\r\n",
	    destination);


  if ((retval =
       (*pgensock_put_data_buffered) (SMTPSock, line, lstrlen(line)))) {
    return (retval);
  }

  _snprintf(line,
	    MAXOUTLINE,
	    "Subject: forwarded Usenet article\r\n",
	    destination);

  if ((retval =
       (*pgensock_put_data_buffered) (SMTPSock, line, lstrlen(line)))) {
    return (retval);
  }

  _snprintf(line,
	    MAXOUTLINE,
	    "\r\n[article forwarded from %s (%s)]\r\n\r\n",
	    UserName,
	    MailAddress);

  if ((retval =
       (*pgensock_put_data_buffered) (SMTPSock, line, lstrlen(line)))) {
    return (retval);
  }

  TopOfDoc (Doc, &BlockPtr, &LinePtr);
  while (ExtractTextLine (Doc, LinePtr, line, MAXOUTLINE - 1))
    {
      /* double a leading dot */
      if (line[0] == '.') {
	if ((retval = (*pgensock_put_data_buffered) (SMTPSock, ".", 1))) {
	  return (retval);
	}
      }
      strcat (line, "\r\n");

      if ((retval =
	   (*pgensock_put_data_buffered) (SMTPSock, line, strlen(line)))) {
	return (retval);
      }

      if (!NextLine (&BlockPtr, &LinePtr))
	break;
    }
  UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);

  if ((retval =
       (*pgensock_put_data_buffered) (SMTPSock, "\r\n.\r\n", 5))) {
    return (retval);
  }
  if ((retval =
       (*pgensock_put_data_flush) (SMTPSock))) {
    return (retval);
  }

  if ((retval = get_smtp_line()) != 250) {
    complain("Send Failed. get_smtp_line() returned %d\r\n", retval);
  }

 return (0);
}

@


1.16
log
@don't ingore NNTPService
@
text
@d2 1
a2 1
/* $Id: wvsckgen.c 1.15 1994/08/04 17:59:36 rushing Exp $ */
a102 1
      MRRCloseComm();
d104 2
a105 2
      CommState = ST_CLOSED_COMM;
      InvalidateRect (hWndConf, NULL, TRUE);
a128 1
      MRRCloseComm();
d130 2
a131 2
      CommState = ST_CLOSED_COMM;
      InvalidateRect (hWndConf, NULL, TRUE);
d133 1
@


1.15
log
@removed debug thing that got left in
@
text
@d2 1
a2 1
/* $Id: wvsckgen.c 1.14 1994/08/03 22:16:25 dumoulin Exp rushing $ */
d248 1
d250 1
a250 1
				     (LPSTR)"nntp",
@


1.14
log
@Fixed compiler warning messages
@
text
@d2 1
a2 1
/* $Id: wvsckgen.c 1.13 1994/07/12 22:19:04 rushing Exp dumoulin $ */
d690 3
a692 1
  complain("get_smtp_line returned %d\r\n", get_smtp_line());
@


1.13
log
@ignore soft breaks in headers when sending...
@
text
@d2 1
a2 1
/* $Id: wvsckgen.c 1.12 1994/07/12 21:42:48 rushing Exp $ */
d295 1
a295 1
    while ((index - editptr) < send_len) {
d333 1
a333 1
    if ((index - editptr) == send_len) {
@


1.12
log
@changed transform_... to use new buffered gensock put_data
and other gensock cleanup.
@
text
@d2 1
a2 1
/* $Id: wvsckgen.c 1.10 1994/06/09 21:47:08 rushing Exp $ */
a141 1
  
a278 2
/* this needs to be rewritten to use put_data_buffered... */

d282 1
d291 2
d309 11
a319 3
	if (previous_char != '\r') {
	  if ((retval = (*pgensock_put_data_buffered) (sock, index, 1))) {
	    return (retval);
d321 2
d324 1
a324 2
	/* soft line-break (see EM_FMTLINES), skip extra CR */
	break;
d596 1
a596 1
    complain ("finish_smtp_message() returned %d\r\n", retval);    
@


1.11
log
@bad return value in send_smtp_article
@
text
@d2 1
a2 1
/* $Id: wvsckgen.c 1.10 1994/06/09 21:47:08 rushing Exp rushing $ */
d39 1
a39 1
int send_smtp_article (TypDoc *Doc);
d53 2
d56 1
d107 1
d128 12
a139 1
    gensock_error ("gensock_put_data", retval);
d142 2
a143 3
  else
    return (0);

a182 2
  int retval;

d220 13
a233 1

d280 1
a280 1
#define MAGIC_BUFFER_SIZE	512
d287 1
a288 2
  unsigned int magic_index = 0;
  char magic_buffer[MAGIC_BUFFER_SIZE];
a293 1
    magic_index = 0;
d295 1
a295 1
    while (((index - editptr) < send_len) && (magic_index < MAGIC_BUFFER_SIZE - 1))  {
d300 3
a302 1
	  magic_buffer[magic_index++] = '.';
d304 3
a306 1
	magic_buffer[magic_index++] = '.';
d310 3
a312 1
	  magic_buffer[magic_index++] = '\r';
d317 3
a319 1
	magic_buffer[magic_index++] = *index;
a323 1
    putline_internal (sock, magic_buffer, magic_index);
d334 3
a336 1
    putline_internal (sock, "\r\n.\r\n", 5);
d339 3
a341 1
    putline_internal (sock, ".\r\n", 3);
d344 4
d417 1
a417 1
  
a426 1

a429 1
  int retval;
d576 15
a590 6
  if (!prepare_smtp_message (destination))
    if (!send_smtp_article (Doc))
      if (!finish_smtp_message ()) {
	close_smtp_socket();
	return (0);
      }
d595 3
a597 1
int send_smtp_article (TypDoc * Doc)
d604 1
d607 38
a644 1
  wsprintf(line, "\r\n[article forwarded from %s (%s)]\r\n", UserName, MailAddress);
d646 4
a649 2
  /* blank line to separate the header */
  put_smtp_line (SMTPSock, line, lstrlen(line));
d655 5
a659 2
      if (line[0] == '.')
	put_smtp_line (SMTPSock, ".", 1);
d661 6
a666 1
      put_smtp_line (SMTPSock, line, strlen (line));
d672 10
a681 1
  put_smtp_line (SMTPSock, "\r\n.\r\n", 5);
d683 1
a683 1
  return (0);
@


1.10
log
@handle unexpected disconnect, and smtp mail while disconnected from nntp
@
text
@d2 1
a2 1
/* $Id: wvsckgen.c 1.9 1994/06/09 18:39:34 rushing Exp $ */
d580 1
a580 1
  return (1);
@


1.9
log
@word-wrap, dot-doubling, and default port numbers
@
text
@d2 1
a2 1
/* $Id: wvsckgen.c 1.8 1994/06/07 01:33:36 rushing Exp rushing $ */
d46 1
a46 1
HANDLE	gensock_lib;
d59 1
a59 1
#define ERROR_BUFFER_SIZE 1204
d80 1
a80 1
  
d99 7
d161 1
d163 1
d167 1
a167 1
MRRInitComm()
d175 1
d179 2
a180 1
  if (((FARPROC)pgensock_connect = GetProcAddress(gensock_lib, "gensock_connect")) == NULL) {
d185 2
a186 1
  if (((FARPROC)pgensock_getchar = GetProcAddress(gensock_lib, "gensock_getchar")) == NULL) {
d191 2
a192 1
  if (((FARPROC)pgensock_put_data = GetProcAddress(gensock_lib, "gensock_put_data")) == NULL) {
d197 2
a198 1
  if (((FARPROC)pgensock_close = GetProcAddress(gensock_lib, "gensock_close")) == NULL) {
d203 2
a204 1
  if (((FARPROC)pgensock_gethostname = GetProcAddress(gensock_lib, "gensock_gethostname")) == NULL) {
d208 4
d213 4
d218 7
a241 3
  else { /* no error originally */
    return (0);
  }
d299 1
a299 1
  
d324 8
d349 9
a357 2
  else { /* no error originally */
    return (0);
d359 1
d370 11
d395 1
a395 7
  
  /* get the local hostname (needed by SMTP) */
  if ((retval = (*pgensock_gethostname) (my_hostname, sizeof(my_hostname)))) {
    gensock_error ("gensock_gethostname", retval);
    return (-1);
  }
  
d413 1
a413 1
  put_smtp_line (SMTPSock, out_data, strlen (out_data));  
d423 2
a424 2
  {                                         
    // if there's only one token left, then len will = startLen, 
d428 1
a428 1
    { 
d450 2
a451 2
  put_smtp_line (SMTPSock, out_data, strlen (out_data));  
  
d464 1
a464 1
  
d483 1
a483 1
    
d496 1
a496 1
  
d503 1
a503 1
  else 
d510 1
a510 1
  
d528 1
a528 1
  if (!prepare_smtp_message (destination)) 
d538 1
a538 1
  
d559 1
a559 1
  
d579 1
a579 1
  
@


1.8
log
@use new gethostname feature of gensock,
fixed smtp mail with it
@
text
@d2 1
a2 1
/* $Id: wvsckgen.c 1.7 1994/06/02 00:11:48 rushing Exp rushing $ */
d194 20
a213 5
  if ((retval = (*pgensock_connect)((LPSTR)NNTPHost,
				    (LPSTR)NNTPService,
				    &NNTPSock))) { 
    gensock_error ("gensock_connect", retval);
    return (-1);
d215 1
a215 1
  
d225 68
d298 18
a315 4
				    (LPSTR)"smtp",
				    &SMTPSock))) {
    gensock_error ("gensock_connect", retval);
    return (-1);
a316 1
  return (0);
d415 2
a416 16
  int maillen = strlen(message);
  int finished = 0;

  /* as per the comments in wvpost.c, the Edit controls already
   * have their data in netascii format.
   */

  putline_internal(SMTPSock, message, maillen);

  if (message[maillen] != '\n') {
    put_smtp_line (SMTPSock, "\r\n.\r\n", 5);
  }
  else {
    put_smtp_line (SMTPSock, ".\r\n", 3);
  }

@


1.7
log
@better error messages
@
text
@d2 1
a2 1
/* $Id: wvsckgen.c 1.6 1994/05/27 01:18:12 rushing Exp rushing $ */
d23 1
d52 1
d189 5
d244 7
d260 1
a260 1
  sprintf (out_data, "HELO\r\n");
d319 1
a319 1

d326 8
a333 1
  putline_internal(SMTPSock, message, strlen(message));
a334 1
  put_smtp_line (SMTPSock, "\r\n.\r\n", 5);
@


1.6
log
@cleaned up smtp some
@
text
@d2 1
a2 1
/* $Id: wvsckgen.c 1.5 1994/05/23 19:51:51 rushing Exp rushing $ */
d8 2
a9 1
#include <stdio.h> // for sprintf
d57 2
d60 1
a60 1
complain(char * message)
d62 10
d73 11
a83 2
/*  OutputDebugString (message); */
  MessageBox (hWndConf, message, "WinVN socket error", MB_OK);
a93 1
      char message[200];
d98 1
a98 2
      wsprintf (message, "gensock_getchar returns %d\n", retval);
      complain(message);
d110 1
a110 1
  
d115 1
a115 1
    complain ("Error in gensock_put_data()");
d188 3
a190 5
				   (LPSTR)NNTPService,
				   &NNTPSock))) { 
	char message[512];
	wsprintf (message, "gensock_connect() failed for %s with error code %d\n", NNTPHost, retval);
    complain (message);
d206 1
a206 2
  char error_message[MAXOUTLINE];
  
d210 1
a210 3
	wsprintf(error_message, "connect() failed for host '%s', error %d\n",
				SMTPHost, retval);
    complain (error_message);
a212 1

d221 1
a221 1
    complain ("gensock_close failed() on SMTPSock");
d338 1
a338 1
      complain ("error in gensock_getchar on SMTPSock");
d362 1
a362 1
    complain ("error in gensock_put_data on SMTPSock");
@


1.5
log
@NNTPPort becomes NNTPService, and it's a string.
@
text
@d2 1
a2 1
/* $Id: $ */
d169 4
a172 2
				   &NNTPSock))) {
    complain ("gensock_connect() failed for NNTPHost");
d188 1
d193 3
a195 1
    complain ("gensock_connect() failed for SMTPHost");
d199 1
a199 3
  get_smtp_line();

  return (1);
a201 2

// old boolean ways of mine
d208 1
a208 1
    return (0);
d210 1
a210 1
  return (1);
d224 7
a230 2
  if (!open_smtp_socket())
    return (0);
d237 1
a237 1
    return (0);
d246 1
a246 1
    return (0);
d270 1
a270 1
      return (0);
d282 1
a282 1
    return (0);
d285 1
a285 1
  return(1);
a292 1
  char out_data[MAXOUTLINE];
a297 31
#if 0
  /* this code is a slight rework of that from wvpost.c */
  while (!finished)
    {
      int linelen;
      char * outptr;

      for (linelen = 0, outptr = out_data;
	   *message && *message != '\r' && linelen < MAXOUTLINE;
	   linelen++)
	{			/* special case of a line starting with '.' */
	  if ((outptr == out_data) && (*message == '.'))
	    *(outptr++) = '.';
	  else
	    *(outptr++) = *(message++);
	}
      
      *(outptr++) = '\r'; *(outptr++) = '\n';
      linelen += 2;
      
      put_smtp_line (SMTPSock, out_data, linelen);

      if (*message == '\r')
	message++;
      if (*message == '\n')
	message++;
      if (!*message)
	  finished = TRUE;
    }
#endif

d303 1
a303 1
    return (0);
d305 1
a305 1
  return(1);
d308 1
d310 1
a310 4

  put_smtp_line (SMTPSock, "QUIT\r\n", 6);
  close_smtp_socket();
  return (1);
a312 1

d325 1
d333 8
a340 1
  return (atoi(in_data));
d351 1
a351 2

  return (nchars);
d364 6
d371 1
a371 9
  if (!prepare_smtp_message (destination))
    return (0);

  if (!send_smtp_edit_data (message))
    return (0);

  if (!finish_smtp_message ())
    return (0);

a376 1

d378 6
a383 8
    return (0);

  if (!send_smtp_article (Doc))
    return (0);

  if (!finish_smtp_message ())
    return (0);

a384 1
  
d396 2
d399 1
a399 1
  put_smtp_line (SMTPSock, "\r\n", 2);
@


1.4
log
@changes for gensock & version 0.91
@
text
@d1 3
d168 1
a168 1
				   (LPSTR)"nntp",
@


1.3
log
@multiple recipient stuff, remove gethostname, clean up MAIL FROM: <xxx>
@
text
@d21 3
a23 1
void PutCommLine(char *line,unsigned int nchars);
d64 1
a64 1
  char ch;
d100 4
d105 1
a105 1
PutCommLine (char * line, unsigned int nchars)
d107 2
a108 2
  putline_internal (NNTPSock, line, nchars);
  putline_internal (NNTPSock, "\n", 1);
d111 10
d286 5
d319 4
@


1.2
log
@catch both wait_a_bit and err_busy
@
text
@d201 2
a202 1
  /* char my_name[128]; */
a203 4
  /* get our hostname */
  /* leaving this out for now. yuck */
  /*  gethostname (my_name, 128); */

a206 1
  /* sprintf (out_data, "HELO %s\r\n", my_name); */
d215 1
a215 1
  sprintf (out_data, "MAIL From: <%s (%s)>\r\n", MailAddress, UserName);
d224 23
a246 2
  sprintf (out_data, "RCPT To: <%s>\r\n", destination);
  put_smtp_line (SMTPSock, out_data, strlen (out_data));    
d248 2
a249 4
  if (get_smtp_line() != 250) {
    smtp_error ("The mail server doesn't like the recipient's name, "
		"have you set the 'To: ' field correctly?");
    return (0);
@


1.1
log
@Initial revision
@
text
@d66 1
a66 1
    if (retval != WAIT_A_BIT) {
d68 4
@
