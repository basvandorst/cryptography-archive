// -*- C++ -*-
// generic socket DLL, pathworks version
// disclaimer:  a C programmer wrote this.

#include <windows.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
/* you must define WSOCKETS_DLL before including socket.h */
#define  WSOCKETS_DLL
#include <sys\socket.h>
#include <netinet\in.h>
#include <netdb.h>
#include <sock_err.h>
#include "gensock.h"
}

#define SOCKET_BUFFER_SIZE	8192

//
typedef int SOCKET;

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
  HTASK		owner_task;
  
 public:

  connection (void);
  ~connection (void);

  int 		get_connected (char * hostname, char * service);
  SOCKET 	get_socket(void) { return (the_socket); }
  HTASK		get_owner_task(void) { return (owner_task); }
  int		get_buffer(int wait);
  int		close(void);
  int		getchar(int wait, char * ch);
  int		put_data(char * data, unsigned long length);
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
}

connection::~connection (void)
{
  delete [] in_buffer;
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
  long 			ioctl_blocking = 1;
  int			retval;
  
  // if the ctor couldn't get a buffer
  if (!in_buffer || !out_buffer)
    return (ERR_CANT_MALLOC);
  
  // --------------------------------------------------
  // resolve the service name
  //
  serventry = getservbyname (service, (LPSTR)"tcp");
  
  if (serventry)
    our_port = serventry->s_port;
  else {
    char * tail;
    our_port = (int) strtol (service, &tail, 10);
    if (tail == service) {
      return (ERR_CANT_RESOLVE_SERVICE);
    }
    else
      our_port = htons (our_port);
  }

  // --------------------------------------------------
  // resolve the hostname/ipaddress
  //

  if ((ip_address = inet_addr (hostname)) != -1) {
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

  if ((the_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    return (ERR_CANT_GET_SOCKET);
  }
  
  sa_in.sin_family = AF_INET;
  sa_in.sin_port = our_port;
  
  // set socket options.  DONTLINGER will give us a more graceful disconnect
  
  setsockopt(the_socket,
	     SOL_SOCKET,
	     SO_DONTLINGER,
	     (char *) &not, sizeof(not));
  
  if ((retval = connect (the_socket,
			 (struct sockaddr *)&sa_in,
			 sizeof(struct sockaddr_in)))) {
    return (ERR_CANT_CONNECT);
  }
  
  // Make this a non-blocking socket
  ioctl (the_socket, FIONBIO, (char far *)(&ioctl_blocking)); 

  owner_task = GetCurrentTask();
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

  while (bytes_read < 1) {
    MSG msg;

    // is there anything waiting to be read? 

    retval = ioctl(the_socket, FIONREAD, (char far *)(&ready_to_read));
    if (retval == -1) {
      char socket_error[256];
      char message[512];
      sock_strerror (errno, socket_error); 
      wsprintf (message, "connection::get_buffer() error from ioctl() '%s'", socket_error);
      complain(message);
    }

    if (!ready_to_read && wait)
      return (WAIT_A_BIT);
    
    bytes_read = recv (the_socket,
		       in_buffer,
		       SOCKET_BUFFER_SIZE,
		       0);

    // do something in the meanwhile...
    if (!bytes_read) {
      for (;;) {
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
	  TranslateMessage(&msg);
	  DispatchMessage(&msg);
	}
      }
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
  char error_string[256];
  int num_sent;

  while (length > 0) {
    num_sent = send (the_socket, data, length > 512 ? 512 : (int)length, 0);
    
    if (num_sent < 0 ) {
      switch (errno) {
      case ENOTCONN:
	return (ERR_NOT_CONNECTED);
	break;
      case ENOBUFS:
	complain ("ENOBUFS, waiting...");
//	stupid_pathworks_delay (1,0); 
	break;
      default:
	sock_strerror (errno, error_string);
	complain (error_string);
	break;
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
      memcpy (out_buffer + out_index, data, (size_t) length);
      out_index += (unsigned int) length;
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
  if (close_socket(the_socket) == -1)
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

  void remove		(socktag st);
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
  HTASK	current_task = GetCurrentTask();
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
connection_list::remove (socktag st)
{
  // at the end
  if (!data)
    return;
  
  // we can assume next is valid because
  // the last node is always {0,0}
  if (data == st) {
    delete data;
    data = next->data;
    next = next->next; // 8^)
    return;
  }

  // recurse
  next->remove(st);
}
  
//
// ---------------------------------------------------------------------------
// global variables (shared by all DLL users)

connection_list global_socket_list;
HINSTANCE dll_module_handle;

// the DLL entry routine
int FAR PASCAL LibMain (HINSTANCE hinstance,
			WPARAM data_seg,
			LPARAM heap_size,
			LPSTR command_line)
{ 
  dll_module_handle = hinstance;
  return (1);
}
			
// ---------------------------------------------------------------------------
// C/DLL interface
//

int FAR PASCAL _export
gensock_connect (char FAR * hostname,
		 char FAR * service,
		 socktag FAR * pst)
{
  int retval;
  connection * conn = new connection;
  
  if (!conn)
    return (ERR_INITIALIZING);

  if ((retval = conn->get_connected (hostname, service))) {
    delete conn;
    *pst = 0;
    return (retval);
  }

  global_socket_list.push(*conn);

  // *pst = conn->get_socket();
  *pst = (socktag) conn;
  
  return (0);
}

//
//
//

int FAR PASCAL _export
gensock_getchar (socktag st, int wait, char FAR * ch)
{
  connection * conn;
  int retval = 0;
  
  conn = (connection *) st;
  // conn = global_socket_list.find((SOCKET)st);
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

int FAR PASCAL _export
gensock_put_data (socktag st, char FAR * data, unsigned long length)
{
  connection * conn;
  int retval = 0;

  conn = (connection *) st;
  // conn = global_socket_list.find((SOCKET)st);
  
  if (!conn)
    return (ERR_NOT_A_SOCKET);

  if ((retval = conn->put_data(data, length)))
    return (retval);

  return (0);
}


//---------------------------------------------------------------------------
//
//

int FAR PASCAL _export
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

int FAR PASCAL _export
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

int FAR PASCAL _export
gensock_close (socktag st)
{
  connection * conn;
  int retval;
  
  conn = (connection *) st;
  // conn = global_socket_list.find((SOCKET)st);
  if (!conn)
    return (ERR_NOT_A_SOCKET);
  
  if ((retval = conn->close()))
    return (retval);

  global_socket_list.remove(st);
  
  return (0);
}

//---------------------------------------------------------------------------
//
//

int FAR PASCAL _export
gensock_gethostname (char FAR * name, int namelen)
{
  return (gethostname (name, namelen));
}

#if 0 
void
stupid_pathworks_delay(int secs, int usecs)
{
  struct timeval to;
  int retval;

  to.tv_sec = secs;
  to.tv_usec = usecs;

  /* Now we use select to delay */
  retval = select (1, NULL, NULL, NULL, &to);
  switch (retval) {
  case -1:
    complain ("error in select");
    break;
  case 1:
    complain("Delaying select returned true on NULL socket set");
    break;
  case 0:
    break;
  }
}

#endif
