/*
 *
 * $Id: wvsckstd.c 1.10 1993/07/08 19:40:49 rushing Exp $
 * $Log: wvsckstd.c $
 * Revision 1.10  1993/07/08  19:40:49  rushing
 * added winsock error code stuff
 *
 * Revision 1.9  1993/06/29  20:05:47  rushing
 * removed ioctlsocket call
 *
 * Revision 1.8  1993/06/28  17:51:39  rushing
 * fixed compiler warnings
 *
 * Revision 1.7  1993/06/15  21:17:43  rushing
 * catch WSAECONNABORTED
 *
 * Revision 1.6  1993/06/11  03:23:14  rushing
 * add a separating line between our header and the original
 *
 * Revision 1.5  1993/05/18  22:10:45  rushing
 * smtp support
 *
 * Revision 1.4  1993/04/29  20:23:41  rushing
 * attempted support for WSAAsyncSelect
 *
 * Revision 1.3  1993/02/23  23:41:06  rushing
 * more robust handling of unconnected state
 *
 * Revision 1.2  1993/02/18  20:31:56  rushing
 * set CommState to ST_NONE after closing connection.
 *
 * Revision 1.1  1993/02/16  20:54:22  rushing
 * Initial revision
 *
 *
 */

/*--  First line of WVSock.c ------------------------------------ */
/* wvsock for WINDOWS SOCKETS */


#include <winsock.h>
#include "wvglob.h"
#include "winvn.h"
//#include <errno.h>
#include <stdlib.h>
#include <stdio.h> // for sprintf

#include <memory.h>
#include <dos.h>

#define MAXSENDLEN 32767
#define COMMBUFSIZE	1600

#define PORT_SMTP 25

char CommBuff[COMMBUFSIZE];
int CommBuffIdx = COMMBUFSIZE + 1;
int CharsInCommBuff = 0;

SOCKET NNTPSock;
SOCKET SMTPSock;

WSADATA winsock_data;

int DebugSock = 0;
char mymesbuf[160];

unsigned long taddr_n, taddr_h;
short tport_n, tport_h;

int PASCAL FAR WSAsperror (int errorcode, char far * buf, int len);

int close_smtp_socket(void);
int open_smtp_socket(void);
void smtp_error (char * message);
int send_smtp_message (char * destination, char * message);
int send_smtp_article (TypDoc *Doc);
int send_smtp_edit_data (char * message);
int prepare_smtp_message (char * destination);
int finish_smtp_message (void);
int get_smtp_line (void);
int put_smtp_line (SOCKET sock, char far * line, unsigned int nchars);
char *SocketErrStr(void);

/* this is from the winsock site */
int PASCAL FAR WSAsperror (int errorcode, char far * buf, int len)
{
        if (errorcode == 0)
                errorcode = WSABASEERR;
        if (errorcode < WSABASEERR)
                return 0;
        return LoadString(hInst,errorcode,buf,len);
}

/*-- function SetupSock --------------------------------------------
 *
 *   Setups up BSD-style socket to the NNTP server.
 *
 *   Returns 1 upon success, else 0.
 */
int SetupSock ()
{
  struct hostent far *hostentry;
  struct sockaddr_in sa_in;
  unsigned long ip_address;
  unsigned long ioctl_blocking = 1; // non-zero indicates non-blocking

  if ((ip_address=inet_addr (NNTPHost)) != INADDR_NONE) {
    sa_in.sin_addr.s_addr = ip_address;
  }
  else {
    if ((hostentry = gethostbyname( NNTPHost )) == NULL) {
      MessageBox (hWndConf, "Can't resolve hostname.", "WinVn", MB_OK);
      exit (1);
    }
    sa_in.sin_addr.s_addr = *(long far *)hostentry->h_addr;
  }

  if ((NNTPSock = socket( AF_INET, SOCK_STREAM, 0 )) == INVALID_SOCKET) {
    MessageBox (hWndConf, "Have you loaded the network?", "socket() failed", MB_OK);
    exit (1);
  }

  sa_in.sin_family = AF_INET;
  sa_in.sin_port = htons (NNTPPort);

  if (connect( NNTPSock, (struct sockaddr *)&sa_in,
	       sizeof(struct sockaddr_in))==SOCKET_ERROR) {
    MessageBox( hWndConf, "Could not connect to News Server", "WinVn", MB_OK);
    closesocket( NNTPSock );
    exit (0);
  }

  // Make this a non-blocking socket
  ioctlsocket (NNTPSock, FIONBIO, &ioctl_blocking);

  // I've tried this with FTP Inc and NetManage.
  // With FTP, Windows freezes.  With NetManage, there is no
  // noticeable speed increase.  Doing this correctly would
  // mean re-writing DoCommInput,DoCommState, etc to be event-driven
  // rather than polled (via the timer).

  // request asynchronous notification when data is waiting.
  //  WSAAsyncSelect (NNTPSock,hWndConf,WM_TIMER,FD_READ);

  return (1);
}

/* MRRReadComm PutCommLine MRRInitComm MRRCloseComm */

/*-- function MRRReadComm ---------------------------------------
 *
 *  Reads characters from the NNTP Socket
 */

int MRRReadComm()
{
  int bytesread;
  int myerr;
/*   u_long lIoctlarg; */
  char error_code[256];

  myerr = 0;

  if (CommState != ST_CLOSED_COMM) {
    if (CommBuffIdx >= CharsInCommBuff) {
      bytesread = recv( NNTPSock, CommBuff, COMMBUFSIZE, 0 );

      if (bytesread == SOCKET_ERROR) {
	myerr = WSAGetLastError();
	switch (myerr) {
	case WSAEWOULDBLOCK:
	case WSAEINPROGRESS:
	  return (-1);
	  break;
	default:
	  WSAsperror (myerr, error_code, 255);
	  MessageBox (hWndConf, error_code, "Error in recv", MB_OK);
	  CommState = ST_CLOSED_COMM;
	  return (-1);
	  break;
	}
      }

      if (!bytesread)
	return (-1);
      else if (bytesread > 0) {
	CharsInCommBuff = bytesread;
	CommBuffIdx = 0;
      }

      else {
	sprintf (mymesbuf, "NNTPSock=%d  bytesread=%d  neterrno=%d", NNTPSock, bytesread, WSAGetLastError());
	MessageBox (hWndConf, mymesbuf, "Unexpected error in MRRReadComm", MB_OK);
	return (-1);
      }
    }
    return (0xff & CommBuff[CommBuffIdx++]);
  }
  else
    return (-1);

}


/*-- function PutCommLine ----------------------------------------------
 *
 *  Entry   line   points to a buffer of characters.
 *          nchars is the number of characters to output.
 */
int
putline_internal (SOCKET sock, char * line, unsigned int nchars)
{
  int num_sent;
  char error_code[256];
  int myerr;

  while (nchars > 0) {
    if (CommState != ST_CLOSED_COMM) {
      num_sent = send (sock, line, nchars, 0);
	if (num_sent == SOCKET_ERROR) {
	  myerr = WSAGetLastError();
	  switch (myerr) {
	  case WSAEWOULDBLOCK:
	  case WSAEINPROGRESS:
	    return (-1);
	    break;
	  default:
	    CommState = ST_CLOSED_COMM;
	    WSAsperror (myerr, error_code, 255);
	    MessageBox (hWndConf, error_code, "Error in recv", MB_OK);
	    return (-1);
	    break;
	  }
	}
	else {
	  nchars -= num_sent;
	  line += num_sent;
	}
    }
    else {
      MessageBox (hWndConf, "Not Connected", "WinVN", MB_OK || MB_ICONHAND);
      return (-1);
    }
  }
}


void
PutCommLine (line, nchars)
     char *line;
     unsigned int nchars;
{
  putline_internal (NNTPSock, line, nchars);
  putline_internal (NNTPSock, "\n", 1);
}


/*-- function MRRCloseComm -----------------------------------------
 *
 *  Close the communications port, serial or TCP.
 */
void MRRCloseComm ()
{
  CommState = ST_CLOSED_COMM;
  closesocket( NNTPSock );
  WSACleanup();
}

MRRInitComm()
{
  int retcode;
  char message[120];
  WORD version_required = 0x0101;  // Version 1.1

  retcode = WSAStartup (version_required, &winsock_data);

  switch (retcode) {
   case 0:
	// successful
	break;
   case WSASYSNOTREADY:
	MessageBox (hWndConf, "Network Not Initialized", "WinVN", MB_ICONHAND || MB_OK);
	return (-1);
	break;
   case WSAEINVAL:
	sprintf (message, "WINSOCK Version Invalid.\nHighVersion = %d.%d",
		   (winsock_data.wHighVersion & 0xff), (winsock_data.wHighVersion & 0xff00)>>8);
	MessageBox (hWndConf, message, "WinVN", MB_ICONHAND || MB_OK);
	return (-1);
	break;
   case WSAVERNOTSUPPORTED:
	sprintf (message, "WINSOCK Version not supported.\nHighVersion = %d.%d",
		   (winsock_data.wHighVersion & 0xff), (winsock_data.wHighVersion & 0xff00)>>8);
	MessageBox (hWndConf, message, "WinVN", MB_ICONHAND || MB_OK);
	return (-1);
	break;
  }

  retcode = SetupSock();

  if (!retcode) {
    MessageBox (hWndConf, "Can't set up socket", "Initialization error", MB_OK | MB_ICONHAND);
  }

  IgnoreCommCh = '\r';
  EOLCommCh = '\n';

  CommLinePtr = CommLineIn;
  CommState = ST_ESTABLISH_COMM;
  CommBusy = FALSE;
  return (0);
}

int open_smtp_socket ()
{
  struct sockaddr_in sa_in;
  struct hostent far *hostentry;
  struct servent far *serventry;
  int smtp_port = htons(PORT_SMTP);

  unsigned long ip_address;
  unsigned long ioctl_blocking = 1; // non-zero indicates non-blocking

  if ((ip_address=inet_addr (SMTPHost)) != INADDR_NONE) {
    sa_in.sin_addr.s_addr = ip_address;
  }
  else {
    if ((hostentry = gethostbyname(SMTPHost )) == NULL) {
      MessageBox (hWndConf, "Can't resolve hostname.", "WinVn", MB_OK);
      return (0);
    }
    sa_in.sin_addr.s_addr = *(long far *)hostentry->h_addr;
  }

  if ((SMTPSock = socket( AF_INET, SOCK_STREAM, 0 )) == INVALID_SOCKET) {
    MessageBox (hWndConf, "Have you loaded the network?", "socket() failed", MB_OK);
    return (0);
  }

  serventry = getservbyname ("smtp", "tcp");
  /* If we find the SMTP port number from the database, use it.
	* Otherwise, use the value that was hardcoded above.
	*/
  if (serventry) {
	 smtp_port = serventry->s_port;
  }

  sa_in.sin_family = AF_INET;
  /* winsock says this is already in network byte order, no need for htons */
  sa_in.sin_port = smtp_port;

  if (connect( SMTPSock,
	      (struct sockaddr *)&sa_in, sizeof(struct sockaddr_in))
		== SOCKET_ERROR) {
	 sprintf(mymesbuf,"Could not connect to Mail Server:\n%s",
		SocketErrStr());
	 MessageBox( hWndConf,mymesbuf, "WinVn", MB_OK);
    closesocket( SMTPSock );
    return (0);
  }

  ioctlsocket (SMTPSock, FIONBIO, &ioctl_blocking);
  
  /* grab intro line */
  get_smtp_line();

  return (1);
}


int close_smtp_socket()
{
  if (closesocket(SMTPSock)) {
    MessageBox (hWndConf, "Error closing SMTP socket", "WinVN", MB_OK);
    return (0);
  }
  else
    return (1);
}


/* 'destination' is the address the message is to be sent to */
/* 'message' is a pointer to a null-terminated 'string' containing the */
/* entire text of the message. */

int prepare_smtp_message(char * destination)
{

  char out_data[MAXOUTLINE];
  char my_name[128];
  
  /* get our hostname */
  gethostname (my_name, 128);

  if (!open_smtp_socket())
    return (0);

  sprintf (out_data, "HELO %s\r\n", my_name);
  put_smtp_line (SMTPSock, out_data, strlen (out_data));

  if (get_smtp_line() != 250) {
    smtp_error ("SMTP server error");
    return (0);
  }

  sprintf (out_data, "MAIL From: <%s (%s)>\r\n", MailAddress, UserName);
  put_smtp_line (SMTPSock, out_data, strlen (out_data));  

  if (get_smtp_line() != 250) {
    smtp_error ("The mail server doesn't like the sender name, "
		"have you set your mail address correctly?");
    return (0);
  }

  sprintf (out_data, "RCPT To: <%s>\r\n", destination);
  put_smtp_line (SMTPSock, out_data, strlen (out_data));    

  if (get_smtp_line() != 250) {
    smtp_error ("The mail server doesn't like the recipient's name, "
		"have you set the 'To: ' field correctly?");
    return (0);
  }

  sprintf (out_data, "DATA\r\n");
  put_smtp_line (SMTPSock, out_data, strlen (out_data));  
  
  if (get_smtp_line() != 354) {
    smtp_error ("Mail server error accepting message data");
    return (0);
  }

}

int send_smtp_edit_data (char * message)
{

  int finished = 0;
  char out_data[MAXOUTLINE];

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
  put_smtp_line (SMTPSock, "\r\n.\r\n", 5);
  if (get_smtp_line() != 250) {
    smtp_error ("Message not accepted by server");
    return (0);
  }
}

finish_smtp_message () {

  put_smtp_line (SMTPSock, "QUIT\r\n", 6);
  close_smtp_socket();
  return (1);
}


int get_smtp_line ()
{
  char in_data [MAXOUTLINE];
  int bytesread,errcode;
    
  do {
    bytesread = recv (SMTPSock, in_data, MAXOUTLINE, 0);
    if (bytesread == 0)		/* connection closed */
      return (0);
    if (bytesread == SOCKET_ERROR) {
      errcode = WSAGetLastError();
      if (errcode != WSAEWOULDBLOCK) {
	sprintf (in_data, "winsock error %d", errcode);
	MessageBox (hWndConf, in_data, "WinVN", MB_OK|MB_ICONHAND);
      }
    }
  }
  while (bytesread < 0);
/*  MessageBox (hWndConf, in_data, "get_smtp_line", MB_OK); */
  return (atoi(in_data));
}

int put_smtp_line (SOCKET sock, char far * line, unsigned int nchars)
{
  int num_sent;

  while (nchars > 0) {
    num_sent = send (sock, line, nchars, 0);
    if (num_sent == SOCKET_ERROR) {
      if (WSAGetLastError() == WSAENOTCONN)
	MessageBox (hWndConf, "Not connected to mail server",
		    "WinVN", MB_OK || MB_ICONHAND);
      else
	MessageBox (hWndConf, "Error in put_smtp_line", "WinVN", MB_OK);
    }
    else {
      nchars -= num_sent;
      line += num_sent;
    }
    
  }
  return (num_sent);
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
    return (0);

  if (!send_smtp_edit_data (message))
    return (0);

  if (!finish_smtp_message ())
    return (0);

  return (1);
}
  
int forward_smtp_message (char * destination, TypDoc * Doc)
{

  if (!prepare_smtp_message (destination))
    return (0);

  if (!send_smtp_article (Doc))
    return (0);

  if (!finish_smtp_message ())
    return (0);

  return (1);
  
}

int send_smtp_article (TypDoc * Doc)
{
  HANDLE hBlock;
  unsigned int Offset;
  TypLineID MyLineID;
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
  char line [MAXOUTLINE];
  
  /* blank line to separate the header */
  put_smtp_line (SMTPSock, "\r\n", 2);

  TopOfDoc (Doc, &BlockPtr, &LinePtr);
  while (ExtractTextLine (Doc, LinePtr, line, MAXOUTLINE - 1))
    {
      /* double a leading dot */
      if (line[0] == '.')
	put_smtp_line (SMTPSock, ".", 1);
      strcat (line, "\r\n");
      put_smtp_line (SMTPSock, line, strlen (line));
      if (!NextLine (&BlockPtr, &LinePtr))
	break;
    }
  UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);

  put_smtp_line (SMTPSock, "\r\n.\r\n", 5);
  
  return (1);
}


/*--- function SocketErrStr ----------------------------------------
 *
 *  Returns a string describing the most recent socket error.
 *
 *  Entry:	The last winsock function called returned a result
 * 			of SOCKET_ERROR.
 *
 *  Exit:	Returns a pointer to a string describing the error.
 */

char *
SocketErrStr(void)
{
#define MAXERRORSTR 60
	static char errstr[MAXERRORSTR];
	int wsa_error = WSAGetLastError();

	WSAsperror(wsa_error,errstr,MAXERRORSTR);
	return errstr;

}
