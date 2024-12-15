#include "newsapi.h"

/* is there a cleaner way to do this? */

int nntp_add_server(nntp_connect_type * connect_data, news_server_type ** server);
int file_add_server(file_connect_type * connect_data, news_server_type ** server);
int rpc_add_server(rpc_connect_type * connect_data, news_server_type ** server);

typedef int
LIST_GROUPS_FUN
(news_server_type * 	server,
 news_group_list_type ** group_list);

typedef int
GROUP_FUN
(news_server_type * 	server,
 char * 		group,
 news_group_type * 	news_group);

typedef int
GET_OVERVIEWS_FUN
(news_server_type * server,
 char * 		group,
 unsigned long 		low,
 unsigned long 		high,
 news_overview_list_type ** overviews);

typedef int
ARTICLE_FUN
(news_server_type *	server,
 char * 		group,
 unsigned long 		art_num,
 news_article_type **	article);

typedef int
POST_FUN
(news_server_type * 	server,
 char * 		post_data,
 size_t			length);

typedef int
CLOSE_FUN
(news_server_type *	server);

GROUP_FUN nntp_group;
GROUP_FUN file_group;
GROUP_FUN rpc_group;
LIST_GROUPS_FUN nntp_list_groups;
LIST_GROUPS_FUN file_list_groups;
LIST_GROUPS_FUN rpc_list_groups;
GET_OVERVIEWS_FUN nntp_get_overviews;
GET_OVERVIEWS_FUN file_get_overviews;
GET_OVERVIEWS_FUN rpc_get_overviews;
ARTICLE_FUN nntp_article;
ARTICLE_FUN file_article;
ARTICLE_FUN rpc_article;
POST_FUN nntp_post;
POST_FUN file_post;
POST_FUN rpc_post;
CLOSE_FUN nntp_close_server;
CLOSE_FUN file_close_server;
CLOSE_FUN rpc_close_server;

int news_add_server (int server_type,
		     void * connect_data,
		     news_server_type ** server)
{

  switch (server_type) {
  case NNTP_SERVER_TYPE:
    return (nntp_add_server((nntp_connect_type *) connect_data, server));
    break;
  case FILE_SERVER_TYPE:
    return (file_add_server((file_connect_type *) connect_data, server));
    break;
  case RPC_SERVER_TYPE:
    return (rpc_add_server((rpc_connect_type *) connect_data, server));
    break;
  }
  return (ERR_UNKNOWN_SERVER_TYPE);
}

int news_list_groups(news_server_type *	server,
		     news_group_list_type ** group_list)
{
  switch(server->server_type) {
  case NNTP_SERVER_TYPE:
    return (nntp_list_groups(server,group_list)); break;
  case FILE_SERVER_TYPE:
    return (file_list_groups(server,group_list)); break;
  case RPC_SERVER_TYPE:
    return (rpc_list_groups(server,group_list)); break;
  default:
    return(ERR_UNKNOWN_SERVER_TYPE);
  }
}

int news_group (news_server_type * server,
		char * group,
		news_group_type * news_group)
{
  switch(server->server_type) {
  case NNTP_SERVER_TYPE:
    return (nntp_group(server,group,news_group)); break;
  case FILE_SERVER_TYPE:
    return (file_group(server,group,news_group)); break;
  case RPC_SERVER_TYPE:
    return (rpc_group(server,group,news_group)); break;
  default:
    return (ERR_UNKNOWN_SERVER_TYPE);    
  }
}



int news_get_overviews
	(news_server_type * server,
	 char * 		group,
	 unsigned long 		low,
	 unsigned long 		high,
	 news_overview_list_type ** overviews)
{
  switch(server->server_type) {
  case NNTP_SERVER_TYPE:
    return(nntp_get_overviews(server,group,low,high,overviews)); break;
  case FILE_SERVER_TYPE:
    return(file_get_overviews(server,group,low,high,overviews)); break;
  case RPC_SERVER_TYPE:
    return(rpc_get_overviews(server,group,low,high,overviews)); break;
  default:
    return (ERR_UNKNOWN_SERVER_TYPE);
  }
}


int news_article
	(news_server_type *	server,
	 char * 		group,
	 unsigned long 		art_num,
	 news_article_type **	article)
{
  switch(server->server_type) {
  case NNTP_SERVER_TYPE:
    return (nntp_article(server,group,art_num,article)); break;
  case FILE_SERVER_TYPE:
    return (file_article(server,group,art_num,article)); break;
  case RPC_SERVER_TYPE:
    return (rpc_article(server,group,art_num,article)); break;
  default:
    return (ERR_UNKNOWN_SERVER_TYPE);
  }
}

int news_post (news_server_type * 	server,
	       char * 		post_data,
	       size_t		length)
{
  switch(server->server_type) {
  case NNTP_SERVER_TYPE:
    return(nntp_post(server,post_data,length)); break;
  case FILE_SERVER_TYPE:
    return(file_post(server,post_data,length)); break;
  case RPC_SERVER_TYPE:
    return(rpc_post(server,post_data,length)); break;
  default:
    return (ERR_UNKNOWN_SERVER_TYPE);
  }
}

int news_close_server (news_server_type * server)
{
  switch(server->server_type) {
  case NNTP_SERVER_TYPE:
    return(nntp_close_server(server)); break;
  case FILE_SERVER_TYPE:
    return(file_close_server(server)); break;
  case RPC_SERVER_TYPE:
    return(rpc_close_server(server)); break;
  default:
    return (ERR_UNKNOWN_SERVER_TYPE);
  }
}
