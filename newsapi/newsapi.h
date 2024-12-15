
/* Usenet News API */

#ifndef _NEWSAPI_H
#define _NEWSAPI_H

#define NEWS_MAX_HOST_NAME 256

#include <time.h>

/* buffer_type is used to collect an unknown quantity of NNTP output
 * into a single area of memory, using realloc.
 */

typedef struct _buffer_type {
  char *	data;		/* data */
  char *	index;		/* current pointer into buffer */
  char *	limit;		/* past this point we must realloc */
  size_t	size;		/* current buffer size */
  unsigned long	lines;		/* # of lines in the buffer */
} buffer_type;

/*
 * -------------------------------------------------------------------------
 * The 'group' object.
 * -------------------------------------------------------------------------
 */

typedef struct _news_group_type {
  char *	group_name;
  unsigned long	low;
  unsigned long	high;  
  unsigned long	estnum;
  char		flags;
} news_group_type;

/*
 * -------------------------------------------------------------------------
 * The 'group_list' object.
 * -------------------------------------------------------------------------
 */

typedef struct _news_group_list_type {
  unsigned long			length;
  news_group_type *		groups;
  struct _news_server_type *	server;
  buffer_type *			list_buffer;
} news_group_list_type;

/*
 * -------------------------------------------------------------------------
 * The 'overview' object.
 * -------------------------------------------------------------------------
 */

typedef struct _news_overview_type {
  char *	subject;
  char *	from;
  char *	message_id;
  char *	references;
  char *	xref;
  time_t	date;
  unsigned long	number;
  unsigned long	lines;
  size_t	bytes;		/* was unsigned long 8^) */
} news_overview_type;

/*
 * -------------------------------------------------------------------------
 * The 'overview_list' object.
 * -------------------------------------------------------------------------
 * to be freed: .overview or XOVER output buffer.
 */

typedef struct _news_overview_list_type {
  unsigned long			length;
  news_overview_type *		overviews;
  news_group_type *		group;
  struct _news_server_type *	server;
  buffer_type *			overview_buffer;
} news_overview_list_type;

/*
 * -------------------------------------------------------------------------
 * The 'article' object.
 * -------------------------------------------------------------------------
 * bytes added to help support binary articles
 */

typedef struct _news_article_type {
  char *		data;
  unsigned int		lines;
  size_t		bytes;
} news_article_type;

/* 
 * -------------------------------------------------------------------------
 * The 'server' object. 
 * -------------------------------------------------------------------------
 * to be freed: group list object, impdep stuff
 */

#define	NNTP_SERVER_TYPE	0 
#define	FILE_SERVER_TYPE	1 
#define	RPC_SERVER_TYPE		2 

typedef struct _news_server_type {
  char 			hostname[NEWS_MAX_HOST_NAME]; 	/* server's hostname */  
  unsigned 		long num_groups;
  int 			server_type;	/* nntp, local, rpc... */
  int 			posting_allowed;
  void *		impdep; 	/* implementation dependent data */
} news_server_type;

/*
 * -------------------------------------------------------------------------   
 *   connect_data types
 * -------------------------------------------------------------------------   
 */

typedef struct _nntp_connect_type {
  char *	hostname;
  char *	username;
  char *	password;
} nntp_connect_type;

/* For a file-accessible server.
 * where are the spool, the .overview files, etc... 
 * FIXME: assumption of files named 'active, .overview' is bad
 */ 

typedef struct _file_connect_type {
  char *	spool_root;
  char *	lib_root;
  char *	overview_root;
  char *	inews_program;
  char *	dir_sep;
} file_connect_type;

/* Don't know what an RPC server would need yet */

typedef struct _rpc_connect_type {
  char *	idunno;
} rpc_connect_type;

/*
 * -------------------------------------------------------------------------   
 * News Library Prototypes
 * -------------------------------------------------------------------------   
 */

#ifdef __cplusplus
extern "C" {
#endif

/* connect to another server */

/* 
 * news_add_server
 * will create a server object, and place a pointer to it in 'server'.
 * to free, call news_close_server.
 */
   
int
news_add_server (int			server_type,
		 void * 		connect_data,
		 news_server_type **	server);

/* retrieve list of groups on this server. */

/* 
 * news_list_groups
 * will create or update the current list of groups for this server
 * for nntp, this means [re]sending a "LIST" command.
 * for file, this means rescanning the 'active' file.
 */
int
news_list_groups (news_server_type *	server,
		  news_group_list_type ** group_list);

/* update information about this group on this server */

/*
 * news_group
 * get information about this news group.
 */

int
news_group (news_server_type *	server,
	    char *			group,
	    news_group_type *	news_group);

/* get header information for this group from this server */
/*
 * news_get_overviews
 * retrieves header (overview) information for a range of
 * articles, returning to you a pointer ('overviews') to an
 * overview object.  When you are finished with the overview
 * object, you should call free_overview().
 */

int
news_get_overviews (news_server_type * server,
		    char * 		group,
		    unsigned long 		low,
		    unsigned long 		high,
		    news_overview_list_type ** overviews);

/* get article from group on server */
/* news_article
 * returns an article object containing the text of the article
 * to you.  When finished with the article, call free_article().
 */

int
news_article (news_server_type *	server,
	      char * 		group,
	      unsigned long 		art_num,
	      news_article_type **	article);

/* post to this group on server */
int
news_post (news_server_type * 	server,
	   char * 		post_data,
	   size_t			length);

/* close a server */

/* news_close_server 
 * close any connections to this server, freeing any associated
 * storage.
 */

int news_close_server (news_server_type * server);
int news_free_article (news_article_type * article);
int news_free_overviews (news_overview_list_type * overviews);
int news_free_group_list (news_group_list_type * group_list);

#ifdef __cplusplus
}
#endif

/*
 * -------------------------------------------------------------------------      
 * Return Codes
 * -------------------------------------------------------------------------   
 */

/* NEWSAPI error classes:
 *
 *   7XXX fatal library error
 *   6XXX non-fatal library error
 *   5XXX fatal server error
 *   4XXX non-fatal server error
 */


#define ERR_CANT_MALLOC			6001
#define ERR_NOT_IMPLEMENTED		6002
#define ERR_UNKNOWN_SERVER_TYPE		6003
#define ERR_BAD_ARGS			6004

/* nntp related */
#define ERR_CANT_CONNECT		5001
#define ERR_SERVICE_DISCONTINUED	5002
#define ERR_UNKNOWN_STATE		5003
#define ERR_UNKNOWN_RESPONSE		5004
#define ERR_CANT_PARSE			5005
#define ERR_CANT_GET_SOCKET		5006
#define ERR_SENDING_LINE		5007		 
#define ERR_CANT_RESOLVE_HOSTNAME	5008
#define ERR_SOCKET_ERROR		5009

#define ERR_READING_SOCKET		5011

/* file server related */
#define ERR_CANT_OPEN_FILE		5021
#define ERR_CANT_READ_FILE		5022

/* winsock related */
#define ERR_SYS_NOT_READY		5050
#define ERR_EINVAL			5051
#define ERR_VER_NOT_SUPPORTED		5052
#define ERR_ERROR_CLOSING_SOCKET	5053
#define ERR_INITIALIZING		5054

#define ERR_NO_SUCH_ARTICLE		4001
#define ERR_NO_SUCH_GROUP		4002
#define ERR_UNEXPECTED_RESPONSE		4003
#define ERR_NNTP_BUSY			4004 
#define ERR_OPENING_GROUP		4005
#define ERR_BOGUS_XHDR_DATA		4006
#define ERR_POSTING_NOT_ALLOWED		4007
#define ERR_POSTING_FAILED		4008
#define ERR_LINE_TOO_LONG		4009

#endif /* _NEWSAPI_H */
