#include "newsapi.h"

int
rpc_add_server
	(int			server_type,
	 void * 		connect_data,
	 news_server_type **	server)
{
  return (ERR_NOT_IMPLEMENTED);
}

int
rpc_list_groups(news_server_type *	server,
		news_group_list_type ** group_list)
{
  return (ERR_NOT_IMPLEMENTED);
}


int
rpc_group (news_server_type *	server,
	   char *			group,
	   news_group_type *	news_group)
{
  return (ERR_NOT_IMPLEMENTED);
}

int
rpc_get_overviews (news_server_type * server,
		   char * 		group,
		   unsigned long 		low,
		   unsigned long 		high,
		   news_overview_list_type ** overviews)
{
  return (ERR_NOT_IMPLEMENTED);
}

int
rpc_article (news_server_type *	server,
	     char * 		group,
	     unsigned long 		art_num,
	     news_article_type **	article)
{
  return (ERR_NOT_IMPLEMENTED);
}

int
rpc_post (news_server_type * 	server,
	  char *		post_data,
	  size_t 		length)
{
  return (ERR_NOT_IMPLEMENTED);
}

int
rpc_close_server (news_server_type * server)
{
  return (ERR_NOT_IMPLEMENTED);
}

