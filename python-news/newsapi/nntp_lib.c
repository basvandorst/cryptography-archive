#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "newsapi.h"
#include "nntp.h"
#include "util.h"

/*
 *
 *  news_add_server
 *  connect to a news server
 *
 */

int nntp_add_server(nntp_connect_type * connect_data, news_server_type ** server)
{
  buffer_type * clone;
  int retval;
  news_server_type * serv;
  nntp_impdep_type * nntp_stuff;

  serv = (news_server_type *) news_malloc (sizeof(news_server_type));

  if (!serv) {
    return (ERR_CANT_MALLOC);
  }

  nntp_stuff = (nntp_impdep_type *) news_malloc (sizeof(nntp_impdep_type));

  if (!nntp_stuff) {
    news_free (serv);
    return (ERR_CANT_MALLOC);
  }

  clone = buffer_create (512);	/* arbitrary */

  if (!clone) {
    news_free (serv);    
    news_free (nntp_stuff);
    return (ERR_CANT_MALLOC);
  }
  else {
    clone_string(clone, connect_data->hostname,	&nntp_stuff->hostname);
    clone_string(clone, connect_data->username,	&nntp_stuff->username);
    clone_string(clone, connect_data->password,	&nntp_stuff->password);
  }

  
  /* don't forget to initialize impdepimpdep stuff */

  *server = serv;
  serv->impdep = nntp_stuff;
  serv->server_type		=	NNTP_SERVER_TYPE;
  serv->posting_allowed		=	0;
  serv->num_groups		= 	0;
  serv->impdep			=	nntp_stuff;
  
  /* initialize impdep stuff */
  nntp_stuff->response		=	0;
  nntp_stuff->buffer 		=	NULL;
  nntp_stuff->current_group 	= 	NULL;
  nntp_stuff->current_group_name[0]	= (char) 0;
  nntp_stuff->xover_supported	=	0;
  nntp_stuff->connect_data	=	clone;

  strncpy (serv->hostname, connect_data->hostname, NEWS_MAX_HOST_NAME);

  if ((retval = connect_to_server (connect_data->hostname, serv)))
    return (retval);
  
  /* get initial reponse code from server */
  NNTP_STUFF(serv)->nntp_state = st_establish_comm;

  /* there should only be one line from the server */
  while (NNTP_STUFF(serv)->nntp_state != st_none) {
    if ((retval = (news_machine(serv))))
      return (retval);
  }

  /* test for the XOVER command */

  if ((retval = put_server_line (serv, "XOVER\r\n")))
    return (retval);

  NNTP_STUFF(serv)->nntp_state = st_xover_check;
  while (NNTP_STUFF(serv)->nntp_state != st_none) {
    if ((retval = (news_machine(serv))))
      return (retval);
  }
  
  return(0);
}  
  


/*
 *
 *  news_list_groups
 *
 *  retrieve the list of groups from this server,
 *   constructing an array of news_group_type objects
 *   via the function parse_active 
 *
 */

int nntp_list_groups(news_server_type * server,
		     news_group_list_type ** group_list)
{
  int retval;

  if (NNTP_STUFF(server)->nntp_state != st_none) {
    return (ERR_NNTP_BUSY);
  }

  NNTP_STUFF(server)->buffer = buffer_create (ACTIVE_INITIAL_SIZE);

  if (!NNTP_STUFF(server)->buffer)
    return (ERR_CANT_MALLOC);

  if (put_server_line(server, "LIST\r\n"))
    return (ERR_SENDING_LINE);
  else {
    NNTP_STUFF(server)->nntp_state = st_list_start;

    while (NNTP_STUFF(server)->nntp_state != st_none) {
      if ((retval = news_machine(server)))
	return (retval);
    }
  }
  if ((retval = parse_active(server,
			     NNTP_STUFF(server)->buffer,
			     group_list)))
    return (retval);
  
  return(0);
}



/* I've removed the group_list member from news_server type,
 * it was just more trouble than help, not to mention inconsistent
 * with the behavior of the rest of the API.
 * 940425 (SMR)
 */

/* if group_info is NULL, we ignore it, and do this only for the
 * NNTP side effect. (make this the current group)
 */

int nntp_group (news_server_type * server,
		char * group,
		news_group_type * group_info)
{
  char * tstring;
  nntp_impdep_type * impdep = NNTP_STUFF(server);
  int retval;

  if (impdep->nntp_state != st_none) {
    return (ERR_NNTP_BUSY);
  }
  
  /* construct the 'GROUP' command to send to the server */
  tstring = string_concatenate ("GROUP ", group, " \r\n", NULL);

  if (!tstring)
    return (ERR_CANT_MALLOC);

  /* let nntp_machine know where to put the info */
  if (group_info)
    impdep->current_group = group_info;
  else
    impdep->current_group = NULL;

  if (put_server_line(server, tstring))
    return (ERR_SENDING_LINE);
  else {
    impdep->nntp_state = st_group_resp;
    while (impdep->nntp_state != st_none) {
      if ((retval = news_machine(server))) {
	news_free(tstring);
	return(retval);
      }
    }
  }

  /* set current group name */
  strcpy (impdep->current_group_name, group);

  /* free the temporary string */
  news_free (tstring);
  
  if (impdep->response == 411)
    return (ERR_NO_SUCH_GROUP);
  
  return(0);
}




/*
 * use the NNTP XOVER command to get header information
 *
 */

int nntp_get_overviews_via_xover (news_server_type * server,
				  char * group,
				  long low,
				  long high,
				  news_overview_list_type ** overviews)

{
  /* this leaves enough room for "xover " + ulong + '-' + ulong */
  char command_string[28];
  int retval;

  sprintf (command_string, "XOVER %ld-%ld\r\n", low, high);

  NNTP_STUFF(server)->buffer =
    buffer_create ((high - low) * OVERVIEW_SIZE_ESTIMATE);
  
  if (!NNTP_STUFF(server)->buffer)
    return (ERR_CANT_MALLOC);

  if (put_server_line(server, command_string)) {
    return (ERR_SENDING_LINE);
  }
  else {
    NNTP_STUFF(server)->nntp_state = st_xover_start;
    while (NNTP_STUFF(server)->nntp_state != st_none) {
      if ((retval =  news_machine(server))) {
	return (retval);
      }
    }
  }

  /* construct the header array, pointing into the header buffer */

  if ((retval =
       create_header_array_from_overview (overviews,
					  NNTP_STUFF(server)->buffer)))
    return (retval);
  else {
    /* the buffer's now owned by the overview_list object */
    NNTP_STUFF(server)->buffer = NULL;
    return (0);
  }
}


/*
 * use the NNTP XHDR command to get header information
 * 
 */

int xhdr_helper (news_server_type * server,
		      char * 	command_string)
{
  int retval;
  nntp_impdep_type * impdep = NNTP_STUFF(server);

  if (put_server_line(server, command_string)) {
    return (ERR_SENDING_LINE);
  }
  else {
    impdep->nntp_state = st_xhdr_request_start;
    while (impdep->nntp_state != st_none) {
      if ((retval =  news_machine(server))) {
	return (retval);
      }
    }
  }
  /* add a sentinel */
  buffer_add_line(impdep->buffer, "0 SENTINEL\r\n");
  return(0);
}  

/* FIXME should low & high be unsigned? Need an 'artnum' type */

int nntp_get_overviews_via_xhdr (news_server_type * server,
				 char * group,
				 long low,
				 long high,
				 news_overview_list_type ** overviews)
{
  /* xhdr references 0000000000-0000000000\r\n */
  char command_string[40];
  int retval;

  /* need about the same space as a total XOVER, plus some overhead
   * for the article numbers FIXME get a better estimate */
  
  NNTP_STUFF(server)->buffer =
    buffer_create ((high - low) * 256);

  if (!NNTP_STUFF(server)->buffer)
    return (ERR_CANT_MALLOC);

  /* subject date from lines message_id references */
  /* create_header_array_from_xhdr depends on this order */

  sprintf (command_string, "XHDR subject  %ld-%ld\r\n", low, high);
  if ((retval = xhdr_helper (server, command_string)))
    return (retval);

  sprintf (command_string, "XHDR date %ld-%ld\r\n", low, high);
  if ((retval = xhdr_helper (server, command_string)))
    return (retval);

  sprintf (command_string, "XHDR from %ld-%ld\r\n", low, high);
  if ((retval = xhdr_helper (server, command_string)))
    return (retval);

  sprintf (command_string, "XHDR lines %ld-%ld\r\n", low, high);
  if ((retval = xhdr_helper (server, command_string)))
    return (retval);

  sprintf (command_string, "XHDR message-id  %ld-%ld\r\n", low, high);
  if ((retval = xhdr_helper (server, command_string)))
    return (retval);

  sprintf (command_string, "XHDR references %ld-%ld\r\n", low, high);
  if ((retval = xhdr_helper (server, command_string)))
    return (retval);

  /* construct the header array, pointing into the header buffer */

  if ((retval =
       create_header_array_from_xhdr (server,
				      overviews,
				      NNTP_STUFF(server)->buffer,
				      high - low + 1)))
    return (retval);
  else
    return (0);
}

/*
 *
 *  news_get_overviews
 *
 *  retrieve information about the headers for this range of
 *   articles, constructing an array of news_overview_type.
 *  this nntp version will use either XHDR or XOVER.
 *   
 */

int nntp_get_overviews (news_server_type * server,
			char * group,
			unsigned long low,
			unsigned long high,
			news_overview_list_type ** overviews)
{
  int retval;

  if (high <= low)
    return (ERR_BAD_ARGS);

  if (!(STR_EQ(group, NNTP_STUFF(server)->current_group_name)))
    if ((retval = news_group (server, group, NULL))) {
      return retval;
    }
  
  if (NNTP_STUFF(server)->xover_supported)
    return (nntp_get_overviews_via_xover
	    (server,group,low,high,overviews));
  else
    return (nntp_get_overviews_via_xhdr
	    (server,group,low,high,overviews));    
}




/*
 *
 *  news_article
 *
 *  retrieve an article from the server
 *
 * 
 */

int nntp_article(news_server_type * server,
		 char * group,
		 unsigned long art_num,
		 news_article_type ** article)
{

  int retval;
  news_article_type *	art;
  /* enough room here for 'ARTICLE + ulong' */
  nntp_impdep_type * impdep = NNTP_STUFF(server);
  char command[20];

  art = (news_article_type *) news_malloc(sizeof(news_article_type));
  if (!art)
    return(ERR_CANT_MALLOC);

  *article = art;

  /* if this isn't the current group, make it so */
  if (!(STR_EQ(group, impdep->current_group_name)))
    if ((retval = news_group (server, group, NULL))) {
      news_free (art);
      return retval;
    }
  
  sprintf (command, "ARTICLE %ld\r\n", art_num);
  
  /* Need to replace with get_server_data call */

  if (put_server_line(server, command)) {
    return (ERR_SENDING_LINE);
  }
  else {
    impdep->nntp_state = st_article_resp;
    while (impdep->nntp_state != st_none) {
      if ((retval = news_machine(server)))
	return (retval);
    }
  }

  if (impdep->response != 220) {
    impdep->nntp_state = st_none;
    art->data = NULL;
    return (ERR_UNEXPECTED_RESPONSE);
  }
  
  /* give them a pointer to the article's text and destroy
     the buffer object */

  art->data = impdep->buffer->data;
  news_free (impdep->buffer);
  impdep->buffer = NULL;
  
  return(0);
}



/*
 *
 *  news_post
 *
 *  post an article to the server
 *  lines must end with 'cr lf' to get correct behaviour.
 * 
 */

int nntp_post (news_server_type * server,
	       char * post_data,
	       size_t length)
{
  int retval;
  char command[] = "POST\r\n";
  char * ddot;
  nntp_impdep_type * impdep = NNTP_STUFF(server);

  if (put_server_line(server, command)) {
    return (ERR_SENDING_LINE);
  }
  else {
    impdep->nntp_state = st_post_wait_permission;
    while (impdep->nntp_state != st_post_ok_to_send) {
      if ((retval = news_machine(server)))
	return (retval);
    }
  }

  /* do NNTP dot-doubling magic */
  /* should this key off of '\n.' ?? */
  for (;;) {
    if ((ddot = news_memmem ("\r\n.", 3, post_data, length))) {
      if (put_server_data (server, post_data,
			   (size_t) (ddot - post_data))) {
	return (ERR_SENDING_LINE);
      }
      /* double the dot */
      if (put_server_data (server, "\r\n..", 4)) {
	return (ERR_SENDING_LINE);
      }
      /* skip over the offending text */
      ddot += 3; 
      length = length - (ddot - post_data);
      post_data = ddot;
    }
    else {
      if (put_server_data (server, post_data, length)) {
	return (ERR_SENDING_LINE);
      }
      break;
    }
  }
		 
  /* need to be smart here */

  if (put_server_data (server, ".\r\n", 3)) {
    return (ERR_SENDING_LINE);
  }
  
  impdep->nntp_state = st_post_wait_end;
  while (impdep->nntp_state != st_none) {
    if ((retval = news_machine(server)))
      return (retval);
  }

  return(0);
  
}



/*
 *
 *  news_close_server
 *
 *  close this server and reclaim any memory we've allocated for it
 *   (active buffer, list array, etc...)
 * 
 */

int nntp_close_server (news_server_type * server)
{
  if (server) {
    nntp_impdep_type * impdep = NNTP_STUFF(server);
    /* send the QUIT command */

    put_server_line (server, "quit\r\n");
    impdep->nntp_state = st_closing;
    while (impdep->nntp_state != st_closed_comm)
      news_machine(server);
    
    /* give back memory */

    if (impdep) {
      
      buffer_free (impdep->connect_data);

      if (impdep->buffer)
	buffer_free (impdep->buffer);
      
      if (impdep->sockbuf)
	news_free(impdep->sockbuf);

      if (impdep->server_line)
	news_free(impdep->server_line);
      
      news_free(impdep);
    }
    /* sever the connection */
    close_server(server);
  
    news_free (server);
  }
  return(0);
}

