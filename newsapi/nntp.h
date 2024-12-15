#include "machine.h"

#ifdef WINSOCK
#include <winsock.h>
#endif

/*
 * -------------------------------------------------------------------------
 * Implementation Dependent Data for NNTP servers.
 * server->impdep will point to this.
 * -------------------------------------------------------------------------
 */

typedef struct _nntp_impdep_type {
  char 	* 		server_line;
  nntp_state		nntp_state;	/* current network state */
  buffer_type *		buffer;		/* buffer for NNTP output, holds
					   LIST, ARTICLE, XHDR, and XOVER commands */
  int 			response;	/* latest reponse code from server */
  news_group_type *	current_group;  /* info about the current group */
  char			current_group_name[512]; /* the current group's name (* note below) */
  int 			xover_supported;/* does this server support XOVER? */
  buffer_type *		connect_data; /* copy of connect data held here */
  char *		hostname;	/* hostname (redundant) */
  char *		username;	/* username for AUTHINFO */
  char *		password;	/* password for AUTHINFO */

  /* these are sockets-specific variables, dependent on the socket implementation */
  /* FIXME not safe to use 'int' as an index */
  /* I worry about uninit'd vars here */
#ifdef UNIX_SOCKET
  int 			socket;		/* fd for this server */
  char *		sockbuf;	/* buffer for this socket */
  int 			sockbuf_index;	/* current index into buffer */
  int 			chars_in_buf;	/* current number of chars in the socket buffer */
#else
#	ifdef WINSOCK
  SOCKET		socket;
  char *		sockbuf;	/* buffer for this socket */
  int 			sockbuf_index;	/* current index into buffer */
  int 			chars_in_buf;	/* current number of chars in the socket buffer */
  struct timeval	timeout; 	/* timeout value for select() */
#	endif
#endif

} nntp_impdep_type;

/* (*) We can make this assumption (group names < 512 chars) because an nntp
 * command line cannot exceed 512 characters, and you need to be able to send
 * the group command.
 */

/* use this macro to get to the nntp-specific data */ 
#define NNTP_STUFF(s1)	((nntp_impdep_type *)((s1)->impdep))

/* prototypes needed for NNTP */

/* These functions are needed for an NNTP implementation */

int connect_to_server	(char *hostname, news_server_type * server);
int get_server_char	(news_server_type * server, char * ch);
int put_server_data	(news_server_type * server, char * outbuf, size_t length);
int close_server	(news_server_type * server);

/* from nntp_util.c, using get_server_char and put_server_data */
int get_server_line	(news_server_type * server);
int put_server_line	(news_server_type * server, char * outbuf);
int get_server_data	(news_server_type * server);

int create_header_array_from_overview (news_overview_list_type ** overviews,
				       buffer_type * buffer);

int create_header_array_from_xhdr (news_server_type * server,
				   news_overview_list_type ** overviews,
				   buffer_type * buffer,
				   unsigned long num_arts);
