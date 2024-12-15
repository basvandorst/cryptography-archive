#include <stdio.h>
#include <stdlib.h>
#include "newsapi.h"
#include <time.h>

#define STR_EQ(s1,s2)  (!strcmp ((s1),(s2)))

/* this is not a demo 8^) */

/* the list of connected servers */
typedef struct server_list {
  news_server_type * server;
  struct server_list * next;
} server_list_type;


char post_text[] = 
"Newsgroups: nasa.ksc.test\r\n"
"Subject: robo-post from newsapi\r\n"
"Distribution: local\r\n"
"Date: %s\r\n"
"From: rushing@titan.ksc.nasa.gov\r\n"
"Binary-Header: \377\r\n"
"\r\n"
".\r\n"
"...testing1.\r\n"
"...testing2.\r\n";

server_list_type * server_list = NULL;

static char * mstr[] =
{"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

char *
current_date_string(void)
{
  static char stuff[30];
  time_t current_time;
  struct tm * tm;

  time (&current_time);
  tm = localtime(&current_time);
  
  sprintf (stuff, "%d %.3s %d %02d:%02d:%02d",
	   tm->tm_mday, mstr[tm->tm_mon],
	   tm->tm_year, tm->tm_hour,
	   tm->tm_min, tm->tm_sec);
  return (stuff);
}

void dump_list_array(news_server_type * server)
{
  news_group_list_type * group_list = server->group_list;

  fprintf (stdout, "%ld groups.\n", group_list->length);

#if 0
  for (i=0; i < group_list->length ; i++) {
    news_group_type * gi = &(group_list->groups)[i];
    fprintf (stdout,"low: %.5d high: %.5d flags: %c %s\n", 
	     gi->low, gi->high, gi->flags, gi->group_name);
  }
#endif

}

/*
 *
 *  news_quit
 *
 *  close all servers and relenquish all memory
 *
 */

int news_quit(server_list_type * server_list)
{
  server_list_type * numbered_days;
  while (server_list) {
    numbered_days = server_list->next;
    news_close_server(server_list->server);
    free (server_list);
    server_list = numbered_days;
  }
  return(0);
}

int main(int argc, char * argv[])
{
  int i;
  news_server_type * my_server;
  server_list_type * a_server;
  news_group_type * my_group;
  news_article_type * my_article;
/*  char group_name[] = "comp.os.linux.announce";*/
  char group_name[] = "talk.politics.crypto";
/*  char group_name[] = "local.test"; */
  news_overview_list_type * my_headers;
  int retval;

  nntp_connect_type nntp_connect_data;
  file_connect_type file_connect_data;

/*  temp_test(); */
/*  exit(0); */

  /* connect to all the servers listed on the command line */
  for (i=1; i < argc; i++) {
    server_list_type * a_server =
      (server_list_type *) malloc (sizeof (server_list_type));
    nntp_connect_data.hostname = argv[i];
    retval = news_add_server (NNTP_SERVER_TYPE, &nntp_connect_data, &my_server);

    if (retval)
      fprintf (stdout, "Error in news_add_server: %d\n", retval);
    
    a_server->next = server_list;
    a_server->server = my_server;
    server_list = a_server;
  }

  /* connect to the local file server, if we can */

#ifdef LOCAL_TEST
  a_server = (server_list_type *) malloc (sizeof (server_list_type));
  file_connect_data.spool_root 		= "/usr/spool/news";
  file_connect_data.lib_root 		= "/usr/lib/news";
  file_connect_data.overview_root 	= "/usr/spool/news";
  file_connect_data.inews_program	= "/bin/inews";
  file_connect_data.dir_sep 		= "/";  

  if (!(news_add_server (FILE_SERVER_TYPE, &file_connect_data, &my_server))) {
    a_server->next = server_list;
    a_server->server = my_server;
    server_list = a_server;
  }
#endif
  /* look for c.o.l.a on each server */
  for (a_server = server_list; a_server != NULL; a_server = a_server->next) {
    news_server_type * server = a_server->server;

    /* just use a user-defined group object */
    my_group = (news_group_type *) malloc (sizeof (news_group_type));
    my_group->group_name = group_name;

    if (server->posting_allowed) {
      char test_post[1024];
	
      sprintf (test_post, post_text, current_date_string());
	
      retval = news_post (server, test_post, strlen(test_post));

      if (retval)
	fprintf (stdout, "error in news_post: %d\n", retval);
    }

    /* call the news 'GROUP' command */

    retval = news_group(server, my_group->group_name, my_group);

    if (retval) {
      fprintf (stdout, "error in news_group: %d\n", retval);
      if (retval == ERR_NO_SUCH_GROUP)
	my_group = NULL;
    }

    retval = news_list_groups (server);


    if (retval)
      fprintf (stdout, "error in news_list_groups: %d\n", retval);

    dump_list_array (server);
    
    if (my_group) {
      int i;

      fprintf (stdout,
	       "Group %s on server '%s' low: %ld high: %ld estnum: %ld\n",
	       my_group->group_name,
	       server->hostname,
	       my_group->low,
	       my_group->high,
	       my_group->estnum);

      retval = news_article (server, my_group->group_name, my_group->high, &my_article);


      if (retval)
	fprintf (stdout, "error in news_article: %d\n", retval);
      
      if (my_article) {
	fputs (my_article->data, stdout);
      }

      retval = 
	news_get_overviews (server,
			    my_group->group_name,
			    my_group->low,
			    my_group->high,
			    &my_headers); 


      if (retval)
	fprintf (stdout, "error in news_get_overviews: %d\n", retval);


      for (i = 0; i < my_headers->length; i++) {

	fprintf (stdout, "%-5.5ld %-5.5ld %-20.20s %-5.5ld %-20.20s %s\n",
		 (my_headers->overviews)[i].number,
		 (my_headers->overviews)[i].date,
		 (my_headers->overviews)[i].from,
		 (my_headers->overviews)[i].lines,
		 (my_headers->overviews)[i].references,
		 (my_headers->overviews)[i].subject);

      }
      
    } else {

      fprintf (stdout,
	       "Group not present on server '%s'\n",
	       server->hostname);

    }
    
  }
  
  /* close all the servers */
  news_quit(server_list);

  return (0);
}


