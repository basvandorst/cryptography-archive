/* Interface to GENSOCK module */
/* $Id: wvsckgen.c 1.19 1994/09/22 00:16:59 jcooper Exp $ */

#include <windows.h>
#include <windowsx.h>
#include "wvglob.h"
#include "winvn.h"
#pragma hdrstop
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
int send_cc_article (char * destination, char * editstr);

HINSTANCE	gensock_lib = 0;

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
  if ((gensock_lib = LoadLibrary(GenSockDLL)) < HINSTANCE_ERROR) {
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
  char line [MAXOUTLINE], temp [MAXOUTLINE];

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

  /* find the subject line */
  TopOfDoc (Doc, &BlockPtr, &LinePtr);
  *line = '\0';
  while (ExtractTextLine (Doc, LinePtr, temp, MAXOUTLINE - 1))
    {
    if (!_strnicmp(temp, "subject:", 8))
      {
        _snprintf(line,
             MAXOUTLINE,
	     "Subject: %s (fwd)\r\n", 
	     &temp[8]);
        break;
      }
      if (!NextLine (&BlockPtr, &LinePtr))
       break;
    }
  if (*line && (retval =
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


int cc_smtp_message (char * destination, char * editstr)
{
  int retval;

  if ((retval = prepare_smtp_message (destination))) {
    complain ("prepare_smtp_message returned %d\r\n", retval);
    return (retval);
  }
  else if ((retval = send_cc_article (destination, editstr))) {
    complain ("send_cc_article returned %d\r\n", retval);
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

int send_cc_article (char * destination, char * editstr)
{
  char line [MAXOUTLINE];
  int retval;
  
  _snprintf(line,
	    MAXOUTLINE,
	    "To: %s\r\n",
	    destination);

  if ((retval =
      (*pgensock_put_data_buffered) (SMTPSock, line, lstrlen(line)))) {
    return (retval);
  }

  if ((retval =
      send_smtp_edit_data (editstr))) {
    return (retval);
  }

 return (0);
}

