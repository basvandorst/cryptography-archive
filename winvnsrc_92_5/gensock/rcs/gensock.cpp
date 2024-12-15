head	1.13;
access;
symbols;
locks; strict;
comment	@// @;


1.13
date	94.08.25.22.44.23;	author rushing;	state Exp;
branches;
next	1.12;

1.12
date	94.08.10.21.28.17;	author rushing;	state Exp;
branches;
next	1.11;

1.11
date	94.08.03.21.05.54;	author rushing;	state Exp;
branches;
next	1.10;

1.10
date	94.07.26.23.49.13;	author rushing;	state Exp;
branches;
next	1.9;

1.9
date	94.07.25.23.30.01;	author rushing;	state Exp;
branches;
next	1.8;

1.8
date	94.06.09.21.44.13;	author rushing;	state Exp;
branches;
next	1.7;

1.7
date	94.06.07.01.33.06;	author rushing;	state Exp;
branches;
next	1.6;

1.6
date	94.05.26.19.54.07;	author rushing;	state Exp;
branches;
next	1.5;

1.5
date	94.05.26.19.28.42;	author rushing;	state Exp;
branches;
next	1.4;

1.4
date	94.05.24.05.18.57;	author rushing;	state Exp;
branches;
next	1.3;

1.3
date	94.05.24.03.46.10;	author rushing;	state Exp;
branches;
next	1.2;

1.2
date	94.02.07.21.14.21;	author rushing;	state Exp;
branches;
next	1.1;

1.1
date	94.02.04.03.26.20;	author rushing;	state Exp;
branches;
next	;


desc
@generic socket DLL, winsock version.  (deja vu)
@


1.13
log
@catch more disconnect-type errors
@
text
@// -*- C++ -*-
// generic socket DLL, winsock version
// disclaimer:  a C programmer wrote this.

// $Id: gensock.cpp 1.12 1994/08/10 21:28:17 rushing Exp $

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern "C" {
#include <winsock.h>
#include "gensock.h"
}

#define SOCKET_BUFFER_SIZE	512

/* This is for NT */
#ifdef WIN32
HANDLE	dll_module_handle;
#define	GET_CURRENT_TASK	dll_module_handle
#define	TASK_HANDLE_TYPE	HANDLE
#define GENSOCK_EXPORT

/* This is for WIN16 */
#else
HINSTANCE dll_module_handle;
#define	GET_CURRENT_TASK	GetCurrentTask()
#define	TASK_HANDLE_TYPE	HTASK
#define GENSOCK_EXPORT		_export
#endif

int  init_winsock (void);
void deinit_winsock (void);

//
//
//

#ifdef _DEBUG
void complain (char * message)
{
  OutputDebugString (message);
}
#else
void complain (char * message)
{
  MessageBox (NULL, message, "GENSOCK.DLL Error", MB_OK|MB_ICONHAND);
}
#endif

//
// ---------------------------------------------------------------------------
// container for a buffered SOCK_STREAM.

class connection
{
 private:
  SOCKET	the_socket;
  char *	in_buffer;
  char *	out_buffer;
  unsigned int	in_index;
  unsigned int	out_index;
  unsigned int	in_buffer_total;
  unsigned int	out_buffer_total;
  unsigned int	last_winsock_error;
  TASK_HANDLE_TYPE		owner_task;
  fd_set	fds;
  struct timeval	timeout;

 public:

  connection (void);
  ~connection (void);

  int 		get_connected (char * hostname, char * service);
  SOCKET 	get_socket(void) { return (the_socket); }
  TASK_HANDLE_TYPE		get_owner_task(void) { return (owner_task); }
  int		get_buffer(int wait);
  int		close (void);
  int		getchar (int wait, char * ch);
  int		put_data (char * data, unsigned long length);
  int		put_data_buffered (char * data, unsigned long length);
  int		put_data_flush (void);
};

connection::connection (void)
{
  the_socket = 0;
  in_index = 0;
  out_index = 0;
  in_buffer_total = 0;
  out_buffer_total = 0;
  in_buffer = 0;

  in_buffer = new char[SOCKET_BUFFER_SIZE];
  out_buffer = new char[SOCKET_BUFFER_SIZE];

  last_winsock_error = 0;
}

connection::~connection (void)
{
  delete [] in_buffer;
}

int
gensock_is_a_number (char * string)
{
  while (*string) {
    if (!isdigit (*string)) {
      return (0);
    }
    string++;
  }
  return (1);
}
    
//
// ---------------------------------------------------------------------------
//

int
connection::get_connected (char FAR * hostname, char FAR * service)
{
  struct hostent FAR *	hostentry;
  struct servent FAR *	serventry;
  unsigned long 	ip_address;
  struct sockaddr_in	sa_in;
  int			our_port;
  int			not = 0;
  int			retval, err_code;
  unsigned long		ioctl_blocking = 1;
  char			message[512];

  // if the ctor couldn't get a buffer
  if (!in_buffer || !out_buffer)
    return (ERR_CANT_MALLOC);

  // --------------------------------------------------
  // resolve the service name
  //

  // If they've specified a number, just use it.
  if (gensock_is_a_number (service)) {
    char * tail;
    our_port = (int) strtol (service, &tail, 10);
    if (tail == service) {
      return (ERR_CANT_RESOLVE_SERVICE);
    } else {
      our_port = htons (our_port);
    }
  } else {
    // we have a name, we must resolve it.
    serventry = getservbyname (service, (LPSTR)"tcp");
    
    if (serventry)
      our_port = serventry->s_port;
    else {
      retval = WSAGetLastError();
      // Chicago beta is throwing a WSANO_RECOVERY here...
      if ((retval == WSANO_DATA) || (retval == WSANO_RECOVERY)) {
	return (ERR_CANT_RESOLVE_SERVICE);
      } else {
	return (retval - 5000);
      }
    }
  }

  // --------------------------------------------------
  // resolve the hostname/ipaddress
  //

  if ((ip_address = inet_addr (hostname)) != INADDR_NONE) {
    sa_in.sin_addr.s_addr = ip_address;
  }
  else {
    if ((hostentry = gethostbyname(hostname)) == NULL) {
      return (ERR_CANT_RESOLVE_HOSTNAME);
    }
    sa_in.sin_addr.s_addr = *(long far *)hostentry->h_addr;
  }


  // --------------------------------------------------
  // get a socket
  //

  if ((the_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
    return (ERR_CANT_GET_SOCKET);
  }

  sa_in.sin_family = AF_INET;
  sa_in.sin_port = our_port;

  // set socket options.  DONTLINGER will give us a more graceful disconnect

  setsockopt(the_socket,
	     SOL_SOCKET,
	     SO_DONTLINGER,
	     (char *) &not, sizeof(not));

  // get a connection

  if ((retval = connect (the_socket,
			 (struct sockaddr *)&sa_in,
			 sizeof(struct sockaddr_in))==SOCKET_ERROR)) {
    switch ((err_code = WSAGetLastError())) {
      /* twiddle our thumbs until the connect succeeds */
    case WSAEWOULDBLOCK:
      break;
    case WSAECONNREFUSED:
      return (ERR_CONNECTION_REFUSED);
      break;
    default:
      wsprintf(message, "unexpected error %d from winsock\n", err_code);
      complain(message);
      return (ERR_CANT_CONNECT);
      break;
    }
  }

  owner_task = GET_CURRENT_TASK;

  // Make this a non-blocking socket
  ioctlsocket (the_socket, FIONBIO, &ioctl_blocking);

  // make the FD_SET and timeout structures for later operations...

  FD_ZERO (&fds);
  FD_SET  (the_socket, &fds);

  // normal timeout, can be changed by the wait option.
  timeout.tv_sec = 30;
  timeout.tv_usec = 0;

  return (0);
}


//
//---------------------------------------------------------------------------
//
// The 'wait' parameter, if set, says to return WAIT_A_BIT
// if there's no data waiting to be read.

int
connection::get_buffer(int wait)
{
  int retval;
  int bytes_read = 0;
  unsigned long ready_to_read = 0;

  // Use select to see if data is waiting...

  FD_ZERO (&fds);
  FD_SET  (the_socket, &fds);

  // if wait is set, we are polling, return immediately
  if (wait) {
    timeout.tv_sec = 0;
  }
  else {
    timeout.tv_sec = 30;
  }

  if ((retval = select (0, &fds, NULL, NULL, &timeout))
      == SOCKET_ERROR) {
    char what_error[256];
    int error_code = WSAGetLastError();

    if (error_code == WSAEINPROGRESS && wait) {
      return (WAIT_A_BIT);
    }

    wsprintf (what_error,
	      "connection::get_buffer() unexpected error from select: %d",
	      error_code);
    complain (what_error);
  }

  // if we don't want to wait
  if (!retval && wait) {
    return (WAIT_A_BIT);
  }

  // we have data waiting...
  bytes_read = recv (the_socket,
		     in_buffer,
		     SOCKET_BUFFER_SIZE,
		     0);

  // just in case.

  if (bytes_read == 0) {
    // connection terminated (semi-) gracefully by the other side
    return (ERR_NOT_CONNECTED);
  }

  if (bytes_read == SOCKET_ERROR) {
    char what_error[256];
    int ws_error = WSAGetLastError();
    switch (ws_error) {
      // all these indicate loss of connection (are there more?)
    case WSAENOTCONN:
    case WSAENETDOWN:
    case WSAENETUNREACH:
    case WSAENETRESET:
    case WSAECONNABORTED:
    case WSAECONNRESET:
      return (ERR_NOT_CONNECTED);
      break;

    default:
      wsprintf (what_error,
		"connection::get_buffer() unexpected error: %d",
		ws_error);
      complain (what_error);
    }
  }

  // reset buffer indices.
  in_buffer_total = bytes_read;
  in_index = 0;
  return (0);

}

//
//---------------------------------------------------------------------------
// get a character from this connection.
//

int
connection::getchar(int wait, char FAR * ch)
{
  int retval;

  if (in_index >= in_buffer_total) {
    if ((retval = get_buffer(wait)))
      return (retval);
  }
  *ch = in_buffer[in_index++];
  return (0);
}


//
//---------------------------------------------------------------------------
// FIXME: should try to handle the fact that send can only take
// an int, not an unsigned long.

int
connection::put_data (char * data, unsigned long length)
{
  int num_sent;
  int retval;

  FD_ZERO (&fds);
  FD_SET  (the_socket, &fds);

  timeout.tv_sec = 30;

  while (length > 0) {
    if ((retval = select (0, NULL, &fds, NULL, &timeout)) == SOCKET_ERROR) {
      char what_error[256];
      int error_code = WSAGetLastError();
      
      wsprintf (what_error,
		"connection::put_data() unexpected error from select: %d",
		error_code);
      complain (what_error);
    }

    num_sent = send (the_socket,
		     data,
		     length > 1024 ? 1024 : (int)length,
		     0);

    if (num_sent == SOCKET_ERROR) {
      char what_error[256];
      int ws_error = WSAGetLastError();
      switch (ws_error) {
	// this is the only error we really expect to see.
      case WSAENOTCONN:
	return (ERR_NOT_CONNECTED);
	break;

	// seems that we can still get a block
      case WSAEWOULDBLOCK:
	break;

      default:
	wsprintf (what_error,
		  "connection::put_data() unexpected error from send(): %d",
		  ws_error);
	complain (what_error);
	return (ERR_SENDING_DATA);
      }
    }
    else {
      length -= num_sent;
      data += num_sent;
    }
  }

  return (0);
}

//
//
// buffered output
//

int
connection::put_data_buffered (char * data, unsigned long length)
{
  unsigned int sorta_sent = 0;
  int retval;

  while (length) {
    if ((out_index + length) < SOCKET_BUFFER_SIZE) {
      // we won't overflow, simply copy into the buffer
      memcpy (out_buffer + out_index, data, length);
      out_index += length;
      length = 0;
    }
    else {
      unsigned int orphaned_chunk = SOCKET_BUFFER_SIZE - out_index;
      // we will overflow, handle it
      memcpy (out_buffer + out_index, data, orphaned_chunk);
      // send this buffer...
      if ((retval = put_data (out_buffer, SOCKET_BUFFER_SIZE))) {
	return (retval);
      }
      length -= orphaned_chunk;
      out_index = 0;
      data += orphaned_chunk;
    }
  }

  return (0);
}

int
connection::put_data_flush (void)
{
  int retval;

  if ((retval = put_data (out_buffer, out_index)))
    return (retval);
  else
    out_index = 0;

  return(0);
}

//
//---------------------------------------------------------------------------
//

int
connection::close (void)
{
  if (closesocket(the_socket) == SOCKET_ERROR)
    return (ERR_CLOSING);
  else
    return (0);
}


//
//---------------------------------------------------------------------------
// we keep lists of connections in this class

class connection_list
{
private:
  connection * 		data;
  connection_list * 	next;

public:
  connection_list 	(void);
  ~connection_list	(void);
  void push 		(connection & conn);

  // should really use pointer-to-memberfun for these
  connection * find	(SOCKET sock);
  int how_many_are_mine	(void);

  void remove		(socktag sock);
};

connection_list::connection_list (void)
{
  next = 0;
}

connection_list::~connection_list(void)
{
  delete data;
}

// add a new connection to the list

void
connection_list::push (connection & conn)
{
  connection_list * new_conn;

  new_conn = new connection_list();

  new_conn->data = data;
  new_conn->next = next;

  data = &conn;
  next = new_conn;

}

int
connection_list::how_many_are_mine(void)
{
  TASK_HANDLE_TYPE	current_task = GET_CURRENT_TASK;
  connection_list * iter = this;
  int num = 0;

  while (iter->data) {
    if (iter->data->get_owner_task() == current_task)
      num++;
    iter = iter->next;
  }
  return (num);
}

// find a particular socket's connection object.

connection *
connection_list::find (SOCKET sock)
{
  connection_list * iter = this;

  while (iter->data) {
    if (iter->data->get_socket() == sock)
      return (iter->data);
    iter = iter->next;
  }
  return (0);
}

void
connection_list::remove (socktag sock)
{
  // at the end
  if (!data)
    return;

  // we can assume next is valid because
  // the last node is always {0,0}
  if (data == sock) {
    delete data;
    data = next->data;
    next = next->next;	// 8^)
    return;
  }

  // recurse
  next->remove(sock);
}

//
// ---------------------------------------------------------------------------
// global variables (shared by all DLL users)

connection_list global_socket_list;
int	network_initialized;

//
//---------------------------------------------------------------------------
//

#ifndef WIN32

// the DLL entry routine
int FAR PASCAL LibMain (HINSTANCE hinstance,
			WPARAM data_seg,
			LPARAM heap_size,
			LPSTR command_line)
{
  network_initialized = 0;
  dll_module_handle = hinstance;
  return (1);
}

#else

extern "C" {
  INT APIENTRY
    LibMain (HANDLE 	hInst,
	     ULONG		reason_called,
	     LPVOID		reserved)
      {

	switch (reason_called) {
	case DLL_PROCESS_ATTACH:
	  /* init */
	  dll_module_handle = hInst;
	  break;
	case DLL_THREAD_ATTACH:
	  break;
	case DLL_THREAD_DETACH:
	  break;
	case DLL_PROCESS_DETACH:
	  break;

	default:
	  break;
	}
	return (1);
      }

  /*
   * This wrapper is the actual entry point for the DLL.  It ensures
   * that the C RTL is correctly [de]initialized.
   */

BOOL WINAPI _CRT_INIT (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

BOOL WINAPI
dll_entry_point (HINSTANCE hinstDLL,
		 DWORD fdwReason,
		 LPVOID lpReserved)
{
  /* Init the C run-time before calling any of your code */

  switch (fdwReason) {
  case DLL_PROCESS_ATTACH:
  case DLL_THREAD_ATTACH:
    if (!_CRT_INIT (hinstDLL, fdwReason, lpReserved))
      return (FALSE);
    else
      LibMain (hinstDLL, fdwReason, lpReserved);
    break;

  case DLL_PROCESS_DETACH:
  case DLL_THREAD_DETACH:
    if (!_CRT_INIT(hinstDLL, fdwReason, lpReserved))
      return(FALSE);
    break;
  }
  return (TRUE);
}

}
#endif

// ---------------------------------------------------------------------------
// C/DLL interface
//

int FAR PASCAL GENSOCK_EXPORT
gensock_connect (char FAR * hostname,
		 char FAR * service,
		 socktag FAR * pst)
{
  int retval;
  connection * conn = new connection;

  if (!conn)
    return (ERR_INITIALIZING);

  // if this task hasn't opened any sockets yet, then
  // call WSAStartup()

  if (global_socket_list.how_many_are_mine() < 1)
    init_winsock();

  global_socket_list.push(*conn);

  if ((retval = conn->get_connected (hostname, service))) {
    gensock_close(conn);
    *pst = 0;
    return (retval);
  }
  *pst = (void FAR *) conn;

  return (0);
}

//
//
//

int FAR PASCAL GENSOCK_EXPORT
gensock_getchar (socktag st, int wait, char FAR * ch)
{
  connection * conn;
  int retval = 0;

  conn = (connection *) st;
  if (!conn)
    return (ERR_NOT_A_SOCKET);

  if ((retval = conn->getchar(wait, ch)))
    return (retval);
  else
    return (0);
}


//---------------------------------------------------------------------------
//
//

int FAR PASCAL GENSOCK_EXPORT
gensock_put_data (socktag st, char FAR * data, unsigned long length)
{
  connection * conn;
  int retval = 0;

  conn = (connection *) st;

  if (!conn)
    return (ERR_NOT_A_SOCKET);

  if ((retval = conn->put_data(data, length)))
    return (retval);

  return (0);
}

//---------------------------------------------------------------------------
//
//

int FAR PASCAL GENSOCK_EXPORT
gensock_put_data_buffered (socktag st, char FAR * data, unsigned long length)
{
  connection * conn;
  int retval = 0;

  conn = (connection *) st;

  if (!conn)
    return (ERR_NOT_A_SOCKET);

  if ((retval = conn->put_data_buffered (data, length)))
    return (retval);

  return (0);
}

//---------------------------------------------------------------------------
//
//

int FAR PASCAL GENSOCK_EXPORT
gensock_put_data_flush (socktag st)
{
  connection * conn;
  int retval = 0;

  conn = (connection *) st;

  if (!conn)
    return (ERR_NOT_A_SOCKET);

  if ((retval = conn->put_data_flush() ))
    return (retval);

  return (0);
}

//---------------------------------------------------------------------------
//
//
int FAR PASCAL GENSOCK_EXPORT
gensock_gethostname (char FAR * name, int namelen)
{
  int retval;
  if ((retval = gethostname(name, namelen))) {
    return (retval - 5000);
  }
  else return (0);
}

//---------------------------------------------------------------------------
//
//

int FAR PASCAL GENSOCK_EXPORT
gensock_close (socktag st)
{
  connection * conn;
  int retval;

  conn = (connection *) st;

  if (!conn)
    return (ERR_NOT_A_SOCKET);

  if ((retval = conn->close()))
    return (retval);

  global_socket_list.remove((connection *)st);

  if (global_socket_list.how_many_are_mine() < 1) {
    deinit_winsock();
  }

  return (0);
}

//---------------------------------------------------------------------------
//
//

int
init_winsock(void)
{
  int retval;
  WSADATA winsock_data;
  WORD version_required = 0x0101; /* Version 1.1 */

  retval = WSAStartup (version_required, &winsock_data);

  switch (retval) {
  case 0:
    /* successful */
    break;
  case WSASYSNOTREADY:
    return (ERR_SYS_NOT_READY);
    break;
  case WSAEINVAL:
    return (ERR_EINVAL);
    break;
  case WSAVERNOTSUPPORTED:
    return (ERR_VER_NOT_SUPPORTED);
    break;
  }
  network_initialized = 1;
  return (0);
}

void
deinit_winsock(void)
{
  network_initialized = 0;
  WSACleanup();
}
@


1.12
log
@watch specifically for a numeric service
@
text
@d5 1
a5 1
// $Id: gensock.cpp 1.11 1994/08/03 21:05:54 rushing Exp $
d305 1
a305 1
      // this is the only error we really expect to see.
d307 5
@


1.11
log
@fix for chicago beta
@
text
@d5 1
a5 1
// $Id: gensock.cpp 1.10 1994/07/26 23:49:13 rushing Exp rushing $
d10 1
d108 12
a143 1
  serventry = getservbyname (service, (LPSTR)"tcp");
d145 19
a163 9
  if (serventry)
    our_port = serventry->s_port;
  else {
    retval = WSAGetLastError();
    // Chicago beta is throwing a WSANO_RECOVERY here...
    if ((retval == WSANO_DATA) || (retval == WSANO_RECOVERY)) {
      char * tail;
      our_port = (int) strtol (service, &tail, 10);
      if (tail == service) {
d165 2
a167 5
      else
	our_port = htons (our_port);
    }
    else {
      return (ERR_CANT_RESOLVE_SERVICE);
d778 1
a778 1
    return (retval - 6000);
@


1.10
log
@WAIT_A_BIT on select() fail (shows up with multiple sockets open)
@
text
@d5 1
a5 1
// $Id: gensock.cpp 1.9 1994/07/25 23:30:01 rushing Exp rushing $
d137 2
a138 1
    if (retval == WSANO_DATA) {
@


1.9
log
@buffered put_data
@
text
@d5 1
a5 1
// $Id: gensock.cpp 1.8 1994/06/09 21:44:13 rushing Exp $
d253 4
d259 1
a259 1
	      retval);
d342 1
a342 1
    char what_error[256];
d344 1
a344 1

d347 1
a347 1
		retval);
@


1.8
log
@handle close events...
@
text
@d5 1
a5 1
// $Id: gensock.cpp 1.7 1994/06/07 01:33:06 rushing Exp $
d9 1
a15 3
#define WM_NET_ACTIVITY		(WM_USER + 1)

char NET_CLASS[] = "GenSockClass";
a32 15
typedef enum _winsock_state {
  ss_getting_connected,
  ss_waiting_to_write,
  ss_waiting_to_read,
  ss_successful,
  ss_idle,
  ss_closing,
  ss_closed,
  ss_error,
  ss_refused,
  ss_recv,
  ss_confused
} winsock_state;

void net_wait_loop(winsock_state * state, winsock_state desired);
d56 1
a56 1
class connection 
d60 6
a65 4
  char *	buffer;
  unsigned int	index;
  unsigned int	chars_in_buffer;
  winsock_state	state;
d68 3
a70 1
  
a72 3
  friend LRESULT CALLBACK
    net_proc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  
d80 5
a84 5
  int		close(void);
  int		getchar(int wait, char * ch);
  int		put_data(char * data, unsigned long length);
  void 		handle_winsock_message (LPARAM lparam);
  int		ready() { return (state == ss_idle ? 1 : 0); }
d90 8
a97 5
  index = 0;
  chars_in_buffer = 0;
  buffer = 0;
  
  buffer = new char[SOCKET_BUFFER_SIZE];
a98 1
  state = ss_getting_connected;
d104 1
a104 47
  delete [] buffer;
}

//---------------------------------------------------------------------------
// here's where we should catch an unexpected close.
//

void
connection::handle_winsock_message (LPARAM lparam)
{
  int winsock_error = WSAGETSELECTERROR(lparam);
  if (winsock_error) {
    state = ss_error;
    last_winsock_error = winsock_error;
  }
  else {
    switch (WSAGETSELECTEVENT(lparam)) {
    case FD_CONNECT:
      if (state == ss_getting_connected)
	state = ss_successful;
      break;
    
    case FD_READ:
      if (state == ss_waiting_to_read)
	state = ss_successful;
      break;
    
    case FD_WRITE:
      if (state == ss_waiting_to_write)
	state = ss_successful;
      break;
    
    case FD_CLOSE:
      if (state == ss_closing)
	state = ss_successful;
      else {
	// this is a kludge, for some reason, WSAENOTCONN is
	// not getting thrown.
	state = ss_closed;
      }
      break;
    
    default:
      complain ("unexpected WSAAsyncSelect event message");
      break;
    }
  }
a107 6
// This is the handle for the window that catches WINSOCK messages
//

HWND network_window;

//
d121 1
d123 1
a123 1
  
d125 1
a125 1
  if (!buffer)
d127 1
a127 1
  
d132 1
a132 1
  
d146 3
d150 1
a150 1
  
d164 1
a164 1
  
d173 1
a173 1
  
d176 1
a176 1
  
d178 1
a178 1
  
a182 11
  

  // request notification of network events for this socket to network_window

  if (WSAAsyncSelect (the_socket, network_window,
		      WM_NET_ACTIVITY,
		      FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE)
      == SOCKET_ERROR) {
    complain ("connection::connect() WSAAsyncSelect call fails");
    return (ERR_CANT_GET_SOCKET);
  }
a185 1

a191 14
      state = ss_getting_connected;
      net_wait_loop (&state, ss_successful);
      if (state == ss_error) {
      switch (last_winsock_error) {
      case WSAECONNREFUSED:
        return(ERR_CONNECTION_REFUSED);
        break;
      default:
		return (last_winsock_error - 6000);
      }
      }
      else {
	state = ss_idle;
      }
d203 1
a203 3
  else {
    state = ss_idle;
  }
d205 13
d225 2
d234 40
a273 3
    
  switch (state) {
  case ss_closed:
a274 5
    break;
  case ss_idle:
    break;
  default:
    return (ERR_BUSY);
d277 8
a284 7
  while (bytes_read < 1) {
    state = ss_waiting_to_read;
    // this should succeed, but...
    bytes_read = recv (the_socket,
		       buffer,
		       SOCKET_BUFFER_SIZE,
		       0);
d286 5
a290 26
    // just in case.

    if (bytes_read == SOCKET_ERROR) {
      char what_error[256];
      switch ((retval = WSAGetLastError())) {
      case WSAEWOULDBLOCK:
      case WSAEINPROGRESS:
	if (wait) {
	  state = ss_idle;
	  return (WAIT_A_BIT);
	}
	else {
	  net_wait_loop (&state, ss_successful);
	}
	break;
      default:
	wsprintf (what_error,
		  "connection::get_buffer() unexpected error: %d",
		  retval);
	complain (what_error);
	break;
      }
    }
    else {
      // go to the idle state.
      state = ss_idle;
d295 2
a296 2
  chars_in_buffer = bytes_read;
  index = 0;
d298 1
d304 1
a304 1
// 
d311 1
a311 1
  if (index >= chars_in_buffer) {
d315 1
a315 1
  *ch = buffer[index++];
d329 4
d334 2
a335 10
  switch (state) {
  case ss_closed:
    return (ERR_NOT_CONNECTED);
    break;
  case ss_idle:
    break;
  default:
    return (ERR_BUSY);
  }
  
d337 15
a351 2
    num_sent = send (the_socket, data, length > 1024 ? 1024 : (int)length, 0);
    
d353 9
a361 1
      switch (WSAGetLastError()) {
a362 2
	state = ss_waiting_to_write;
	net_wait_loop (&state, ss_successful);
d364 1
a364 3
      case WSAEINPROGRESS:
	complain ("connection::put_data() unexpected WSAEINPROGRESS");
	break;
d366 4
a370 1
	break;
a373 1
      state = ss_idle;
d378 36
d416 14
a429 1
      
d450 1
a450 1
 private:
d454 1
a454 1
 public:
d522 1
a522 1
      
d529 1
a529 1
  
d535 1
a535 1
    next = next->next; // 8^)
d542 1
a542 1
  
a553 61
// this registers the network window class, and creates an iconic window
// for capturing network events.
//

int
init_network(void)
{
  static WNDCLASS wndclass;

  wndclass.style         = 0;
  wndclass.lpfnWndProc   = (WNDPROC) net_proc;
  wndclass.cbClsExtra    = 0;
  wndclass.cbWndExtra    = 0;
  wndclass.hInstance     = dll_module_handle;
  wndclass.hIcon         = LoadIcon (dll_module_handle, (LPSTR) "gensock");
  wndclass.hCursor       = 0;
  wndclass.hbrBackground = 0;
  wndclass.lpszMenuName  = NULL;
  wndclass.lpszClassName = (LPSTR) NET_CLASS;

  if (!RegisterClass(&wndclass)) {
    complain ("init_network: RegisterClass() failed");
    return (1);
  }
  
  network_window =
    CreateWindow ((LPSTR) NET_CLASS,
		  (char *) "WINSOCK",
		  WS_MINIMIZE,
		  0,0,0,0,
		  NULL,
		  NULL,
		  dll_module_handle,
		  NULL);

  if (!network_window)
    {
      complain ("couldn't create network window");
      UnregisterClass((LPSTR)NET_CLASS, dll_module_handle);
      return(ERR_INITIALIZING);
    }

  ShowWindow(network_window, SW_SHOWMINIMIZED);
  
  return(0);

}

// this registers the network window class, and creates an iconic window
// for capturing network events.
//

int
deinit_network(void)
{
  DestroyWindow (network_window);
  UnregisterClass((LPSTR)NET_CLASS, dll_module_handle);
  network_initialized = 0;
  return(0);
}

d561 1
a561 1
{ 
d566 1
a566 1
			
d570 17
d588 5
a592 33
INT APIENTRY
LibMain (HANDLE 	hInst,
	 ULONG		reason_called,
	 LPVOID		reserved)
{

  switch (reason_called) {
  case DLL_PROCESS_ATTACH:
    /* init */
    dll_module_handle = hInst;
    if (init_network())
      return (0);		/* bool reversal */
    network_initialized++;
    
    break;

  case DLL_THREAD_ATTACH:
    break;

  case DLL_THREAD_DETACH:
    break;

  case DLL_PROCESS_DETACH:
    /* deinit */

    UnregisterClass((LPSTR)NET_CLASS, hInst);
    break;

  default:
    break;
  }
  return (1);
}
d594 4
a597 4
/*
 * This wrapper is the actual entry point for the DLL.  It ensures
 * that the C RTL is correctly [de]initialized.
 */
a600 2
/* Need to understand process/thread stuff */

a620 1

a621 1
    
a622 1

a628 60
//
// ---------------------------------------------------------------------------
//

/*
 * this network process function catches events that belong to sockets.
 * we need to:
 * 	map the socket to a server object.
 * 	decode the message type
 * 	make sure we're getting the expected message (by examining socktab->state)
 * 	perform the necessary WINSOCK operation
 * 	set the state variable to 'successful'
 *
 */

LRESULT CALLBACK
net_proc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  if (msg == WM_NET_ACTIVITY) {
    connection * conn;

    conn = global_socket_list.find ((SOCKET)wparam);

    if (conn)
      conn->handle_winsock_message(lparam);
    
  }
  return(DefWindowProc(hwnd, msg, wparam, lparam));
}

/*
 * Might want to modify this to have a builtin 'hard' timeout -
 * say, if nothing happens for one full minute, then return.
 * 
 */

void
net_wait_loop(winsock_state * state, winsock_state desired)
{
  MSG msg;

  for (;;) {
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) &&
	   (*state != desired)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (*state == ss_error) {
      	return;
      }
      if (*state == desired) {
       /* what we wanted */
       return;
      }
   }
   /* nothing happend, wait for another message */
   WaitMessage();
  }
}


d640 1
a640 1
  
a643 7
  if (!network_initialized) {
    if (init_network())
      return (ERR_INITIALIZING);
    else
      network_initialized++;
  }

a656 2

  // *pst = conn->get_socket();
d671 1
a671 2
  
  // conn = global_socket_list.find((SOCKET)st);
a692 1
  // conn = global_socket_list.find((SOCKET)st);
d694 1
a694 1
  
d707 42
d768 1
a768 2
  
  // conn = global_socket_list.find((SOCKET)st);
d773 1
a773 1
  
a776 1
  // global_socket_list.remove((SOCKET)st);
d778 1
a778 1
  
a780 1
    deinit_network();
d813 1
d820 1
@


1.7
log
@added 'gethostname' function to dll signature
@
text
@d5 1
a5 1
// $Id: gensock.cpp 1.6 1994/05/26 19:54:07 rushing Exp rushing $
d153 5
d309 10
a319 3
  if (!ready())
    return (ERR_BUSY);
    
a320 23

#if 0
    // is there anything waiting to be read? 

    retval = ioctlsocket(the_socket, FIONREAD, &ready_to_read);
    if (retval == SOCKET_ERROR) {
      char message[100];
      wsprintf (message, 
                "connection::get_buffer() error #%d from ioctlsocket()",
                (int)WSAGetLastError());
      complain(message);
    }

    if (!ready_to_read && wait)
      return (WAIT_A_BIT);

    // if not, sit around for a while...
    if (!ready_to_read) {
      state = ss_waiting_to_read;
      net_wait_loop (&state, ss_successful);
    }
  
#endif
a334 4
#if 0
	// shouldn't happen
	complain ("connection::get_buffer() gets WSAEWOULDBLOCK");
#endif
d392 7
a398 1
  if (state != ss_idle)
d400 1
@


1.6
log
@version info
@
text
@d5 1
a5 1
// $Id: $
d345 1
a357 4
      case WSAEINPROGRESS:
	// shouldn't happen
	complain ("connection::get_buffer() gets WSAEINPROGRESS");
	break;
d856 13
@


1.5
log
@change socktag from 'unsigned long' to 'void FAR *'
@
text
@d5 2
@


1.4
log
@better handling of connection failure
@
text
@d470 1
a470 1
  void remove		(SOCKET sock);
d531 1
a531 1
connection_list::remove (SOCKET sock)
d539 1
a539 1
  if (data->get_socket() == sock) {
d609 13
d804 1
a804 1
    gensock_close(conn->get_socket());
d809 2
a810 1
  *pst = conn->get_socket();
d825 2
a826 1
  conn = global_socket_list.find((SOCKET)st);
d847 2
a848 1
  conn = global_socket_list.find((SOCKET)st);
d869 3
a871 1
  conn = global_socket_list.find((SOCKET)st);
d878 2
a879 1
  global_socket_list.remove((SOCKET)st);
d881 1
a881 1
  if (global_socket_list.how_many_are_mine() < 1)
d883 2
@


1.3
log
@handle CONNREFUSED condition, start of better error propagation.
@
text
@d749 7
a755 7
    }
    if (*state == desired) {
      /* what we wanted */
      return;
    }
     /* nothing happend, wait for another message */
    WaitMessage();
d791 1
a791 1
    global_socket_list.remove(conn->get_socket());
@


1.2
log
@use our_port instead of serventry->s_port
@
text
@d16 1
a16 1
#define SOCKET_BUFFER_SIZE	8192
d18 15
d41 2
a42 1
  ss_hosed,
a46 1

d79 2
a80 1
  HTASK		owner_task;
d92 1
a92 1
  HTASK		get_owner_task(void) { return (owner_task); }
d111 1
d126 4
a129 4
  if (WSAGETSELECTERROR(lparam)) {
    char message[200];
    wsprintf (message, "error %d in connection::handle_winsock_message", WSAGetLastError());
    complain (message);
d131 6
d138 4
a141 5
  switch (WSAGETSELECTEVENT(lparam)) {
  case FD_CONNECT:
    if (state == ss_getting_connected)
      state = ss_successful;
    break;
d143 4
a146 4
  case FD_READ:
    if (state == ss_waiting_to_read)
      state = ss_successful;
    break;
d148 4
a151 4
  case FD_WRITE:
    if (state == ss_waiting_to_write)
      state = ss_successful;
    break;
d153 4
a156 8
  case FD_CLOSE:
    if (state == ss_closing)
      state = ss_successful;
    break;
    
  default:
    complain ("unexpected WSAAsyncSelect event message");
    break;
d179 2
a180 1
  int			retval;
d239 1
d241 1
a241 1
  
d249 4
a252 3
  
  // start the connection.  may not happen immediately.
  
d256 3
a258 4
    if (WSAGetLastError() != WSAEWOULDBLOCK)
      complain ("error other than EWOULDBLOCK");
    else {
      /* twiddle our thumbs until we're connected */
d260 22
a281 2
      net_wait_loop(&state, ss_successful);
      state = ss_idle;
d284 1
a284 1
  else
d286 2
a287 2

  owner_task = GetCurrentTask();
d412 1
a412 1
    num_sent = send (the_socket, data, (int)length, 0);
d503 1
a503 1
  HTASK	current_task = GetCurrentTask();
a554 1
HINSTANCE dll_module_handle;
d609 2
d622 46
d669 30
d720 1
a720 1
    
d746 3
d751 1
d754 2
a755 4
    else {
      WaitMessage();
      return;			/* FIXME: this is a kludge. */
    }
d764 1
a764 1
int FAR PASCAL _export
d788 2
d791 1
a791 1
    delete conn;
a795 2
  global_socket_list.push(*conn);

d805 1
a805 1
int FAR PASCAL _export
d826 1
a826 1
int FAR PASCAL _export
d847 1
a847 1
int FAR PASCAL _export
@


1.1
log
@Initial revision
@
text
@d211 1
a211 1
  sa_in.sin_port = serventry->s_port;
@
