// -*- C++ -*-
// generic socket DLL, winsock version
// disclaimer:  a C programmer wrote this.

// $Id: gensock.cpp 1.6 1994/05/26 19:54:07 rushing Exp $

#include <windows.h>
#include <stdlib.h>

extern "C" {
#include <winsock.h>
#include "gensock.h"
}

#define WM_NET_ACTIVITY		(WM_USER + 1)

char NET_CLASS[] = "GenSockClass";
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
  char *	buffer;
  unsigned int	index;
  unsigned int	chars_in_buffer;
  winsock_state	state;
  unsigned int	last_winsock_error;
  TASK_HANDLE_TYPE		owner_task;
  
 public:

  friend LRESULT CALLBACK
    net_proc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  
  connection (void);
  ~connection (void);

  int 		get_connected (char * hostname, char * service);
  SOCKET 	get_socket(void) { return (the_socket); }
  TASK_HANDLE_TYPE		get_owner_task(void) { return (owner_task); }
  int		get_buffer(int wait);
  int		close(void);
  int		getchar(int wait, char * ch);
  int		put_data(char * data, unsigned long length);
  void 		handle_winsock_message (LPARAM lparam);
  int		ready() { return (state == ss_idle ? 1 : 0); }
};

connection::connection (void)
{
  the_socket = 0;
  index = 0;
  chars_in_buffer = 0;
  buffer = 0;
  
  buffer = new char[SOCKET_BUFFER_SIZE];

  state = ss_getting_connected;
  last_winsock_error = 0;
}

connection::~connection (void)
{
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
      break;
    
    default:
      complain ("unexpected WSAAsyncSelect event message");
      break;
    }
  }
}

//
// This is the handle for the window that catches WINSOCK messages
//

HWND network_window;

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
  char			message[512];
  
  // if the ctor couldn't get a buffer
  if (!buffer)
    return (ERR_CANT_MALLOC);
  
  // --------------------------------------------------
  // resolve the service name
  //
  serventry = getservbyname (service, (LPSTR)"tcp");
  
  if (serventry)
    our_port = serventry->s_port;
  else {
    retval = WSAGetLastError();
    if (retval == WSANO_DATA) {
      char * tail;
      our_port = (int) strtol (service, &tail, 10);
      if (tail == service) {
	return (ERR_CANT_RESOLVE_SERVICE);
      }
      else
	our_port = htons (our_port);
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
  

  // request notification of network events for this socket to network_window

  if (WSAAsyncSelect (the_socket, network_window,
		      WM_NET_ACTIVITY,
		      FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE)
      == SOCKET_ERROR) {
    complain ("connection::connect() WSAAsyncSelect call fails");
    return (ERR_CANT_GET_SOCKET);
  }

  // get a connection


  if ((retval = connect (the_socket,
			 (struct sockaddr *)&sa_in,
			 sizeof(struct sockaddr_in))==SOCKET_ERROR)) {
    switch ((err_code = WSAGetLastError())) {
      /* twiddle our thumbs until the connect succeeds */
    case WSAEWOULDBLOCK:
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
  else {
    state = ss_idle;
  }
  owner_task = GET_CURRENT_TASK;
  return (0);
}


//
//---------------------------------------------------------------------------
//

int
connection::get_buffer(int wait)
{
  int retval;
  int bytes_read = 0;
  unsigned long ready_to_read = 0;

  if (!ready())
    return (ERR_BUSY);
    
  while (bytes_read < 1) {

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
    state = ss_waiting_to_read;
    // this should succeed, but...
    bytes_read = recv (the_socket,
		       buffer,
		       SOCKET_BUFFER_SIZE,
		       0);

    // just in case.

    if (bytes_read == SOCKET_ERROR) {
      char what_error[256];
      switch ((retval = WSAGetLastError())) {
      case WSAEWOULDBLOCK:
#if 0
	// shouldn't happen
	complain ("connection::get_buffer() gets WSAEWOULDBLOCK");
#endif
	if (wait) {
	  state = ss_idle;
	  return (WAIT_A_BIT);
	}
	else {
	  net_wait_loop (&state, ss_successful);
	}
	break;
      case WSAEINPROGRESS:
	// shouldn't happen
	complain ("connection::get_buffer() gets WSAEINPROGRESS");
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
    }
  }

  // reset buffer indices.
  chars_in_buffer = bytes_read;
  index = 0;
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

  if (index >= chars_in_buffer) {
    if ((retval = get_buffer(wait)))
      return (retval);
  }
  *ch = buffer[index++];
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

  if (state != ss_idle)
    return (ERR_BUSY);
  
  while (length > 0) {
    num_sent = send (the_socket, data, length > 1024 ? 1024 : (int)length, 0);
    
    if (num_sent == SOCKET_ERROR) {
      switch (WSAGetLastError()) {
      case WSAEWOULDBLOCK:
	state = ss_waiting_to_write;
	net_wait_loop (&state, ss_successful);
	break;
      case WSAEINPROGRESS:
	complain ("connection::put_data() unexpected WSAEINPROGRESS");
	break;
      default:
	return (ERR_SENDING_DATA);
	break;
      }
    }
    else {
      state = ss_idle;
      length -= num_sent;
      data += num_sent;
    }
  }
  return (0);
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
    next = next->next; // 8^)
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

/*
 * This wrapper is the actual entry point for the DLL.  It ensures
 * that the C RTL is correctly [de]initialized.
 */

BOOL WINAPI _CRT_INIT (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

/* Need to understand process/thread stuff */

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

  if (!network_initialized) {
    if (init_network())
      return (ERR_INITIALIZING);
    else
      network_initialized++;
  }

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

  // *pst = conn->get_socket();
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
  
  // conn = global_socket_list.find((SOCKET)st);
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

  // conn = global_socket_list.find((SOCKET)st);
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
gensock_close (socktag st)
{
  connection * conn;
  int retval;
  
  // conn = global_socket_list.find((SOCKET)st);
  conn = (connection *) st;

  if (!conn)
    return (ERR_NOT_A_SOCKET);
  
  if ((retval = conn->close()))
    return (retval);

  // global_socket_list.remove((SOCKET)st);
  global_socket_list.remove((connection *)st);
  
  if (global_socket_list.how_many_are_mine() < 1) {
    deinit_winsock();
    deinit_network();
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
  return (0);
}

void
deinit_winsock(void)
{
  WSACleanup();
}
