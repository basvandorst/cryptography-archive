/* beat on a server, no mercy */

/* usage:
 * ./abuse nntp-server-name
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "newsapi.h"

void
hurl (int val) {fprintf(stderr, "Hurled %d\n", val); exit(0); }

main(int argc, char * argv[])
{
  int retval = 0;
  nntp_connect_type connect_data;
  news_server_type *server;
  news_group_list_type * gl;
  
  connect_data.hostname = argv[1];
  connect_data.username = NULL;
  connect_data.password = NULL;

  retval = news_add_server(NNTP_SERVER_TYPE, &connect_data, &server);
  if(retval) hurl(retval);
  
  retval = news_list_groups(server, &gl);
  if(retval) hurl(retval);
  
  while (1) {
    news_overview_list_type *ov_list;
    int choose = rand() % gl->length;
    news_group_type ng = (gl->groups)[choose];
    
    fprintf (stdout, "group: %s\n", ng.group_name);

    retval = news_group(server, ng.group_name, &ng);
    if(retval) hurl(retval);
    
    if (ng.estnum > 1) {
      int i;
      retval = news_get_overviews(server, ng.group_name, ng.low, ng.high, &ov_list);
      if(retval) hurl(retval);
      
      for (i = 0; i < 10; i++) {
	news_article_type * art;
	int art_choose = rand() % ov_list->length;
	news_overview_type ov = ov_list->overviews[art_choose];
	
	fprintf (stdout, "  article: %ld\n", ov.number);

	retval = news_article (server, ng.group_name, ov.number, &art);
	if(retval) hurl(retval);
	
	/* a little delay */
	/* sleep(1); */

	news_free_article(art);
      }
      news_free_overviews(ov_list);
    }
  }
}

