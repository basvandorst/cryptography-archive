#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "newsapi.h"
#include "nntp.h"
#include "util.h"

int
expecting_response (nntp_state state)
{
  switch (state)
    {
      /* these we don't expect or care about the response code */
    case st_closed_comm:
    case st_confused:
    case st_fatal_error:
    case st_initializing:
    case st_list_data:
    case st_post_ok_to_send:
    case st_none:
    case st_rec_article:
    case st_xhdr_request_data:
    case st_xover_data:
      return 0;
      break;

      /* these we need to know */
    case st_article_resp:
    case st_closing:
    case st_establish_comm:
    case st_group_resp:
    case st_list_start:
    case st_post_wait_permission:
    case st_post_wait_end:
    case st_xhdr_request_start:
    case st_xover_start:
    case st_xover_check:
      return 1;
      break;
    }
  /* this shouldn't happen */
  return 0;
}


/*
 * Handle NNTP response code from server, checking for error
 * conditions.  400 is the only one that really spells trouble.
 *
 * FIXME: This should be rethought.  Sometimes NNTP will spit out
 * an unasked-for status line. (timeout in post, service discontinued, etc.)
 *
 * Maybe what we want is to let the main part of the machine look
 * at the response code first, and punt, via 'default:' to this routine.
 *
 */

int handle_server_response(news_server_type * server)
{
  int class = NNTP_STUFF(server)->response / 100;

  switch(class) {
  case 5:
    /* These are the 5XX class errors from rfc0977.txt
     *   500 command not recognized
     *   501 command syntax error
     *   502 access restriction or permission denied
     *   503 program fault - command not performed
     */
    if (NNTP_STUFF(server)->nntp_state != st_xover_check) {
      NNTP_STUFF(server)->nntp_state = st_none;
      return(ERR_UNEXPECTED_RESPONSE);
    }
    break;
  case 4:
    switch(NNTP_STUFF(server)->response) {
    case 400:
      /* service discontinued */
      NNTP_STUFF(server)->nntp_state = st_closed_comm;
      return(ERR_SERVICE_DISCONTINUED);
      break;
    default:
      /* hopefully news_machine() will handle it */
      break;
    }
  }
  return(0);
}


/* 
 * a state machine for handling an NNTP connection
 *
 */


int news_machine(news_server_type * server)
{
  int retval;
  nntp_impdep_type * impdep = NNTP_STUFF(server);

  /* states where we don't expect a line */
  switch (impdep->nntp_state) {
  default:
    break;

  case st_closed_comm:
    break;

  case st_confused:
    return (ERR_UNKNOWN_STATE);
    break;
  }

  if ((retval = get_server_line(server)))
    return (retval);
    
  /* if we're expecting a response code, handle and store it */
  if (expecting_response(impdep->nntp_state)) {
    impdep->response = atoi(impdep->server_line);
    if ((retval = handle_server_response(server))) return (retval);
  }
    
  /* states where we do expect a line */
  switch (impdep->nntp_state) {
      
  case st_none:
    break;

  case st_establish_comm:

    /*
     * here we verify a connection and issue a 'mode reader' command
     * if we're talking to the InterNetNews (INN) server daemon (innd),
     * rather than the reader daemon (nnrpd).
     */

    switch (impdep->response) {
    case 200:
      server->posting_allowed = 1;
      break;
    case 201:
      server->posting_allowed = 0;
      break;
    default:
      return (ERR_UNKNOWN_RESPONSE);
      break;
    }
    if (strstr(impdep->server_line, "InterNetNews") &&
	!strstr(impdep->server_line, "NNRP")) {
      put_server_line (server, "mode reader\r\n");
      impdep->nntp_state = st_establish_comm; /* deja vu */
    }
    else {
      impdep->nntp_state = st_none;
    }

    break;
    
  case st_group_resp:

    /* handle the response from the 'GROUP' command */
    
    switch (impdep->response) {
    default:
      impdep->nntp_state = st_none;
      impdep->current_group = NULL;
      return (ERR_UNEXPECTED_RESPONSE);
      break;

    case 411:
      impdep->nntp_state = st_none;
      impdep->current_group = NULL;
      return (ERR_NO_SUCH_GROUP);
      break;
      
    case 211: 
      /* GNU's sscanf can allocate buffers, would be safer */
      if (impdep->current_group) {
	if (
	    sscanf (impdep->server_line,
		    /* the exact format of this sscanf depends on defs in newsapi.h */
		    "%*u %lu %lu %lu %*s",
		    &impdep->current_group->estnum,
		    &impdep->current_group->low,
		    &impdep->current_group->high)
	    != 3) {
	  return(ERR_CANT_PARSE);
	}
      }
      impdep->nntp_state = st_none;
      break;
    }
    
    break;

  case st_article_resp:
    switch (impdep->response) {
    case 220:
      /* FIXME we might have (or can get) knowledge of the article's size */
      impdep->buffer = buffer_create (ARTICLE_INITIAL_SIZE);
      if (!impdep->buffer)
	return (ERR_CANT_MALLOC);

      impdep->nntp_state = st_rec_article;
      if ((retval = get_server_data (server)))
	return (retval);
      else
	impdep->nntp_state = st_none;
      break;
      
    case 423:
    case 430:
      impdep->nntp_state = st_none;
      return (ERR_NO_SUCH_ARTICLE);
      break;

    case 412:
      /* this shouldn't happen */
      impdep->nntp_state = st_none;      
      return (ERR_UNEXPECTED_RESPONSE);

    default:
      return (ERR_UNKNOWN_RESPONSE);
    }
    
    break;

  case st_rec_article:
    /* obsolete */
    break;
    
  case st_post_wait_permission:
    switch (impdep->response) {
    case 340:
      impdep->nntp_state = st_post_ok_to_send;
      break;
    case 400:
      impdep->nntp_state = st_none;
      return (ERR_POSTING_NOT_ALLOWED);
      break;
    default:
      impdep->nntp_state = st_none;
      return (ERR_UNKNOWN_RESPONSE);
    }
    break;

  case st_post_ok_to_send:
    /* shouldn't happen */
    break;

  case st_post_wait_end:
    switch (impdep->response) {
    case 240:
      /* article posted ok */
      impdep->nntp_state = st_none;
      break;
    case 441:
      /* posting failed */
      impdep->nntp_state = st_none;
      return (ERR_POSTING_FAILED);
      break;
    default:
      impdep->nntp_state = st_none;
      return (ERR_UNKNOWN_RESPONSE);
    }
    
    break;

  case st_list_start:
    if (impdep->response != 215) {
      impdep->nntp_state = st_confused;
      return (ERR_UNKNOWN_RESPONSE);
    }
    else {
      impdep->nntp_state = st_list_data;
      if ((retval = get_server_data (server)))
	return (retval);
      else
	impdep->nntp_state = st_none;
    }
    break;

  case st_list_data:
    /* obsolete */
    break;

  case st_xhdr_request_start:
    if (impdep->response != 221) {
      impdep->nntp_state = st_none;
      return (ERR_UNEXPECTED_RESPONSE);
    }
    else {
      impdep->nntp_state = st_xhdr_request_data;
      if ((retval = get_server_data (server)))
	return (retval);
      else
	impdep->nntp_state = st_none;
    }
    break;

  case st_xhdr_request_data:
    /* obsolete */
    break;

  case st_xover_start:

    if (impdep->response != 224) {
      impdep->nntp_state = st_none;
    }
    else {
      impdep->nntp_state = st_xover_data;
      if ((retval = get_server_data (server)))
	return (retval);
      else
	impdep->nntp_state = st_none;
    }
    break;

  case st_xover_data:
    /* obsolete */
    break;

  case st_xover_check:
    switch (impdep->response) {
      /* 500 command not recognized */
    case 500:
      impdep->xover_supported = 0;
      break;
      /* 412 Not in a newsgroup */
    case 412:
      impdep->xover_supported = 1;
      break;
    }
    impdep->nntp_state = st_none;
    
    break;

  case st_closing:
    if (impdep->response != 205) {
      return (ERR_UNKNOWN_RESPONSE);
    }
    impdep->nntp_state = st_closed_comm;
    break; 
    
  case st_initializing:
    break;

  default: 
    break;

  }

  /* everything's ok */
  return (0);

}
