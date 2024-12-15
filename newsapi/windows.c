#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "newsapi.h"
#include "nntp.h"
#include "util.h"

/* under construction */

/* 
 * This file contains the Windows-specific routines necessary
 * to:
 * 1) implement the library as a self-contained DLL
 * 2) implement the socket interface via WINSOCK.DLL
 *
 */

/*
 * socket interface:
 *
 * connect_to_server
 * get_server_char  (need to do blocks)
 * put_server_data
 * close_server
 *
 * Their async socket operations are:
 * connect_to_server
 *	getservbyname() WSAAsyncGetServByName
 *	gethostbyname() WSAAsyncGetHostByName
 *	connect()	[non-blocking]
 *	WSAAsyncSelect()
 *
 * get_server_char
 *	recv()		[non-blocking]
 *
 * put_server_data
 *	send()		[non-blocking]
 *
 * close_server
 *	closesocket()	[non-blocking]
 *	need to think about LINGER stuff.
 */

#define SOCKET_BUFFER_SIZE	8192
#define SERVER_LINE_SIZE	8192
#define PORT_NNTP		119
#define WM_NET_ACTIVITY		(WM_USER + 1)
#define NET_CLASS		"NAPIClass"

/*
 * Need to understand how these variables work in
 * a threaded environment 
 */

int		sockets_opened = 0;
HWND		hwnd_net;
HANDLE		dll_module_handle;

/*
 *  per-socket state information.
 *  currently a linked list.
 */

typedef enum _winsock_state {
  ss_getting_service,
  ss_getting_hostname,
  ss_getting_connected,
  ss_waiting_to_write,
  ss_waiting_to_read,
  ss_successful,
  ss_idle,
  ss_closing,
  ss_closed,
  ss_hosed,
  ss_confused
} winsock_state;

/* linked list holds mapping from socket -> news_server */

typedef struct _socket_entry {
  SOCKET 			socket;
  news_server_type * 		server;
  winsock_state			state;
  struct _socket_entry * next;
} socket_entry;

socket_entry * socket_table = NULL;

/* push a new socket onto the list */
int
add_to_socket_tab (socket_entry ** ptable,
		   SOCKET sock,
		   news_server_type * server)
{
  socket_entry * new_entry;

  new_entry = (socket_entry *) news_malloc (sizeof (socket_entry));

  if (!new_entry)
    return (ERR_CANT_MALLOC);

  new_entry->socket = sock;
  new_entry->state = ss_idle;
  new_entry->server = server;
  new_entry->next = *ptable;
  *ptable = new_entry;

  return(0);

}

socket_entry *
remove_socket_helper (socket_entry * table, SOCKET sock)
{
  socket_entry * temp;
  if (table == NULL)
    return (NULL);

  if (table->socket == sock){
    temp = table->next;
    news_free (table);
    return (temp);
  }
  else {
    table->next = remove_socket_helper (table->next, sock);
    return (table);
  }
}

/* remove a socket from the list */
void
remove_from_socket_tab (socket_entry ** ptable,
			SOCKET sock)
{
  *ptable = remove_socket_helper (*ptable, sock);
}

socket_entry *
lookup_socket (socket_entry *table, SOCKET sock)
{
  while (table != NULL) {
    if (table->socket == sock)
      return (table);
    else
      table = table->next;
  }
  return (NULL);
}

void complain (char * message)
{
#ifdef CONSOLE
  fprintf(stderr, "news.dll complaint: %s", message);
#else
  MessageBox (NULL, message, "NEWS.DLL complaint", MB_OK | MB_ICONEXCLAMATION);
#endif
}  

#ifdef DEBUG_OUTPUT
void DEBUG_OUT(char FAR *lps, ...)
{
   static char buf[80];
   static char FAR *args;

   args = (char FAR *) &lps + sizeof(lps);
   wvsprintf(buf, lps, args);
   OutputDebugString(buf);
}
#else
#define DEBUG_OUT(stuff)
#endif

/* this network process function catches events that belong to sockets.
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
  socket_entry * socktab;

  if (msg == WM_NET_ACTIVITY) {

    if (wparam != 0)
      socktab = lookup_socket (socket_table, (SOCKET)wparam);
    else
      socktab = NULL;

    if (socktab) {
      switch (WSAGETSELECTEVENT(lparam)) {
	/* we've connected */
      case FD_CONNECT:
	if (socktab->state == ss_getting_connected) {
	  socktab->state = ss_successful;
	}
	break;

      case FD_READ:
	if (socktab->state == ss_waiting_to_read) {
	  socktab->state = ss_successful;
	}
	break;

      case FD_WRITE:
	if (socktab->state == ss_waiting_to_write) {
	  socktab->state = ss_successful;
	}
	break;

      case FD_CLOSE:
	if ((NNTP_STUFF(socktab->server)->nntp_state == st_closed_comm) ||
	    (NNTP_STUFF(socktab->server)->nntp_state == st_closing) &&
	    (socktab->state == ss_closing)) {
	  socktab->state = ss_closed;

	}
	else {
	  complain ("net_proc: unexpected FD_CLOSE");
	}
	break;

      default:
	complain ("net_proc: strange event notice");
	/* FIXME catch this */
	break;
      }
    }
  }
  return(DefWindowProc(hwnd, msg, wparam, lparam));
}

int
init_net_window_class(void)
{
  WNDCLASS wndclass;

  wndclass.style         = 0;
  wndclass.lpfnWndProc   = (WNDPROC) net_proc;
  wndclass.cbClsExtra    = 0;
  wndclass.cbWndExtra    = 0;
  wndclass.hInstance     = dll_module_handle;
  wndclass.hIcon         = 0;
  wndclass.hCursor       = 0;
  wndclass.hbrBackground = 0;
  wndclass.lpszMenuName  = NULL;
  wndclass.lpszClassName = (LPSTR) NET_CLASS;
  if (!RegisterClass(&wndclass)) {
    complain ("init_net_window_class: RegisterClass() failed");
    return (1);
  }
  else
    return (0);
}


/*
 *
 * When a process attaches to the DLL (via LoadLibrary)
 * o	we register the window class
 *
 * When the first NNTP connection is made
 * o	call WSAStartup()
 * o	create a window to handle socket messages
 *
 * When subsequent connections are made
 * o	add them to the fd_set associated with the net window
 *
 * When a socket is closed,
 * o	remove it from the fd_set
 *
 * When all sockets have been closed
 * o	call WSACleanup
 * o	destroy the window
 *
 * When the process detaches from the DLL
 * o	unregister the window class
 *
 */

INT APIENTRY
LibMain (HANDLE 	hInst,
	 ULONG		reason_called,
	 LPVOID		reserved)
{

  switch (reason_called) {
  case DLL_PROCESS_ATTACH:
    /* init */
    dll_module_handle = hInst;
    if (init_net_window_class())
      return (0);		/* bool reversal */
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

int
init_net_window()
{
  hwnd_net = CreateWindow ((LPSTR) NET_CLASS,
			   (char *) NULL,
			   0,
			   0,0,0,0,
			   NULL,
			   NULL,
			   dll_module_handle,
			   NULL);

  if (!hwnd_net)
    {
      char message[200];
      sprintf (message, "error code was %ld", GetLastError());
      complain (message);
      UnregisterClass((LPSTR)NET_CLASS, dll_module_handle);
      return(ERR_INITIALIZING);
    }
  return(0);
}

int
init_winsock()
{
  int retval;
  WSADATA winsock_data;
  WORD version_required = 0x0101; /* Version 1.1 */

  if ((retval = init_net_window()))
    return (retval);

  retval = WSAStartup (version_required, &winsock_data);

  /* need to actually _check_ the version number ? */

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

/*
 * net_wait_loop lets us give time back to Windows while
 * we wait for a particular network operation to finish
 */

/*
 * Might want to modify this to have a builtin 'hard' timeout -
 * say, if nothing happens for one full minute, then return.
 * 
 */

void
net_wait_loop(winsock_state * state, winsock_state desired)
{
  MSG msg;

  DEBUG_OUT ("entering net_wait_loop\n");

  for (;;) {
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) &&
	   (*state != desired)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    if (*state == desired) {
      DEBUG_OUT ("returning from net_wait_loop\n");
      return;
    }
    else {
      DEBUG_OUT ("calling WaitMessage\n");
      WaitMessage();
      return;			/* FIXME: this is a kludge. */
    }
  }
}

/* handle any outstanding messages */
void
check_messages (void)
{
  MSG msg;

  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

int shutdown_winsock()
{
  DestroyWindow(hwnd_net);
  WSACleanup();
  return (0);
}

/*
 * get the port for NNTP
 * resolve the hostname
 * connect, waiting if necessary
 */

int connect_to_server (char *hostname, news_server_type * server)
{
  struct hostent FAR *	hostentry;
  int 			retval;
  struct sockaddr_in 	sa_in;
  unsigned long 	ip_address;
  struct servent far *	serventry;
  char * 		sockbuf;
  char * 		server_line;
  socket_entry * 	our_socket;
  SOCKET 		nntp_sock;
  char			message[100];

  int nntp_port = htons(PORT_NNTP);
  nntp_impdep_type * impdep = NNTP_STUFF(server);
  int not = 0;

  if (sockets_opened == 0) {
    if ((retval = init_winsock()))
      return (retval);
  }

  serventry = getservbyname ("nntp", "tcp");
  /*
   * If we find the SMTP port number from the database, use it.
   * Otherwise, use the value that was hardcoded above.
   */

  if (serventry)
    nntp_port = serventry->s_port;

  sockbuf = (char *) news_malloc (SOCKET_BUFFER_SIZE);
  server_line = (char *) news_malloc (SERVER_LINE_SIZE);

  if (!sockbuf || !server_line)
    return (ERR_CANT_MALLOC);

  impdep->sockbuf = sockbuf;
  impdep->server_line = server_line;
  impdep->sockbuf_index = 0;
  impdep->chars_in_buf = 0;

  /* default timeout is thirty seconds */
  (impdep->timeout).tv_sec = 30;
  (impdep->timeout).tv_usec = 0;

  if ((ip_address=inet_addr (hostname)) != INADDR_NONE) {
    sa_in.sin_addr.s_addr = ip_address;
  }
  else {
    if ((hostentry = gethostbyname(hostname)) == NULL) {
      return (ERR_CANT_RESOLVE_HOSTNAME);
    }
    sa_in.sin_addr.s_addr = *(long far *)hostentry->h_addr;
  }

  if ((nntp_sock = socket( AF_INET, SOCK_STREAM, 0 )) == INVALID_SOCKET) {
    return (ERR_CANT_GET_SOCKET);
  }

  impdep->socket = nntp_sock;

#if 0
  sprintf (message, "we got socket # %d", nntp_sock);
  complain (message);
#endif

  sa_in.sin_family = AF_INET;
  sa_in.sin_port = nntp_port;

  /* add this socket to the socket->server table */

  if ((retval = add_to_socket_tab (&socket_table, nntp_sock, server))) {
    news_free (sockbuf);
    news_free (server_line);
    return (retval);
  }

  retval = setsockopt(nntp_sock,
		      SOL_SOCKET,
		      SO_DONTLINGER,
		      (char FAR *) &not, sizeof(not));

  our_socket = lookup_socket(socket_table, nntp_sock);

  if (!our_socket) {
    complain ("connect_to_server: socket table hosed");
    return (ERR_CANT_GET_SOCKET);
  }

  /* request notification of network events to window hwnd_net  */
  if (WSAAsyncSelect (nntp_sock, hwnd_net,
		      WM_NET_ACTIVITY,
		      FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE)
      == SOCKET_ERROR) {
    complain ("connect_to_server: WSAAsyncSelect() failed");
    news_free (sockbuf);
    news_free (server_line);
    remove_from_socket_tab (&socket_table, nntp_sock);
    return (retval);
  }

  sockets_opened++;

  if ((retval = connect (nntp_sock,
			 (struct sockaddr *)&sa_in,
			 sizeof(struct sockaddr_in))==SOCKET_ERROR)) {
    if (retval == SOCKET_ERROR) {
      if (WSAGetLastError() != WSAEWOULDBLOCK)
	complain ("error other than EWOULDBLOCK");
      else {
	/* twiddle our thumbs until we're connected */
	our_socket->state = ss_getting_connected;
	net_wait_loop(&(our_socket->state), ss_successful);
	our_socket->state = ss_idle;
      }
    }
  }

  return (0);
}


int
get_server_char (news_server_type * server, char * ch)
{
  int bytes_read = 0;
  int retval;
  nntp_impdep_type * impdep = NNTP_STUFF(server);
  
  /* if our buffer's empty, grab another block from the server */
  if (impdep->sockbuf_index >= impdep->chars_in_buf) {
    unsigned long ready_to_read = 0;
    socket_entry * our_socket =
      lookup_socket (socket_table, impdep->socket);

    /* will this increase reponse ? */
    /* possibly not necessary */

    check_messages();

    DEBUG_OUT ("gonna recv()\n");

    if (!our_socket) {
      complain ("get_server_char(): socket not in table!");
      return (ERR_READING_SOCKET);
    }
    
    /* this may be impossible */
    if (our_socket->state != ss_idle)
      return (ERR_NNTP_BUSY);

    while (bytes_read < 1) {
      char what_error[100];

      /* is there data waiting for us? */
      retval = ioctlsocket (impdep->socket, FIONREAD, &ready_to_read);
      if (retval == SOCKET_ERROR) {
	sprintf (what_error,
		 "get_server_char: ioctlsocket hurls: %d %d",
		 impdep->socket,
		 WSAGetLastError());
	complain (what_error);
      }
      
      if (!ready_to_read) {
	/* nothing yet, must wait */
	DEBUG_OUT ("waiting to read... \n");
	our_socket->state = ss_waiting_to_read;
	net_wait_loop (&(our_socket->state), ss_successful);
      }
      else
	DEBUG_OUT ("%ld bytes ready to read... \n",ready_to_read);

      DEBUG_OUT ("calling recv...\n");

      bytes_read = recv(impdep->socket,
			impdep->sockbuf,
			SOCKET_BUFFER_SIZE, 0);

      DEBUG_OUT ("bytes_read = %d\n", bytes_read);

      if (bytes_read == SOCKET_ERROR) {
	switch ((retval = WSAGetLastError())) {
	case WSAEWOULDBLOCK:
	  /* this shouldn't happen */
	  break;

	case WSAEINPROGRESS:
	  /* this shouldn't happen */
	  complain ("get_server_char(): WSAEINPROGRESS?");
	  break;

	default:
	  sprintf (what_error,
		   "get_server_char(): Unexpected error: %d",
		   retval);
	  complain (what_error);
	  return (ERR_READING_SOCKET);
	  break;
	}
      }
      /* we're not waiting on this socket any more */
      our_socket->state = ss_idle;
    }
      
    /* reflect the new data in the buffer */
    impdep->chars_in_buf = bytes_read;
    impdep->sockbuf_index = 0; /* reset the index */
  }
  
  *ch = (impdep->sockbuf)[(impdep->sockbuf_index)++];
  return (0);
}

int
put_server_data (news_server_type * server, char * outbuf, size_t length)
{
  int num_sent;
  int bytes_sent = 0;
  nntp_impdep_type * impdep = NNTP_STUFF(server);
  socket_entry * our_socket =
    lookup_socket (socket_table, impdep->socket);
    
  if (!our_socket) {
    complain ("put_server_data(): socket not in table!");
    return (ERR_SENDING_LINE);
  }

  if (our_socket->state != ss_idle)
    return (ERR_NNTP_BUSY);

  while (length > 0) {

    num_sent = send (NNTP_STUFF(server)->socket, outbuf, length, 0);

    if (num_sent == SOCKET_ERROR) {
      switch (WSAGetLastError()) {
      case WSAEWOULDBLOCK:
	/* wait till we can write */

	our_socket->state = ss_waiting_to_write;
	net_wait_loop (&(our_socket->state), ss_successful);
	break;

      case WSAEINPROGRESS:
	complain ("put_server_data(): WSAEINPROGRESS?");
	break;

      default:
	return (ERR_SENDING_LINE);
	break;
      }
    }
    else {
      our_socket->state = ss_idle;
      length -= num_sent;
      outbuf += num_sent;
    }
  }
  return (0);
}

/* 
 * close this server.  If there are no other sockets opened,
 * shut down this winsock session.
 * 
 * This should only be called after a "QUIT" command has been issued,
 * because we are actually waiting for the other side to disconnect.
 */


int close_server (news_server_type * server)
{
  int retval;
  nntp_impdep_type * impdep = NNTP_STUFF(server);
  socket_entry * our_socket =
    lookup_socket (socket_table, impdep->socket);

  our_socket->state = ss_closing;

  net_wait_loop (&(our_socket->state), ss_closed);

  /* This is a DONTLINGER graceful disconnect */
  closesocket (impdep->socket);

  /* remove this entry from the socket/server table */
  remove_from_socket_tab (&socket_table, our_socket->socket);

  sockets_opened--;
  
  if (sockets_opened == 0)
    if ((retval = shutdown_winsock()))
      return (retval);
  
  return (0);
}


