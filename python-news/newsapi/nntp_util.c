#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "newsapi.h"
#include "util.h"
#include "nntp.h"



int
create_header_array_from_overview (news_overview_list_type ** overviews,
				   buffer_type * buffer)

{
  unsigned long i;
  char * index;
  news_overview_type * header_array;
  news_overview_list_type * overview_list;
  unsigned long num_headers = 0;
  
  header_array = (news_overview_type *)
    news_malloc (buffer->lines * sizeof(news_overview_type));
  
  if (!header_array) {
    return (ERR_CANT_MALLOC);
  }

  overview_list = (news_overview_list_type *)
    news_malloc (sizeof (news_overview_list_type));
  
  if (!overview_list) {
    news_free (header_array);
    return (ERR_CANT_MALLOC);
  }
  
  /* store the user's pointer */

  *overviews = overview_list;
  overview_list->overviews = header_array;
  num_headers = buffer->lines;
  overview_list->length = num_headers;
  overview_list->overview_buffer = buffer;
  index = buffer->data;

  for (i=0; i<num_headers ; i++) {
    char * hop;
    int result;

    /* FIXME xref should probably be removed from the structure ? */
    header_array[i].xref = NULL;

    if ((result = parse_overview(&(header_array[i]), index, &hop)))
      return (result);
    else
      index = hop;
  }
  
  return (0);
}

/* 
 * need to defend against bogus XHDR data.
 * As far as I know, only ANU news does this.
 * Since it keeps a separate database of *some* headers,
 * xhdr calls are quick for these headers, but require looking 
 * in the article file for others.  If the article file is not
 * there, then xhdr returns for these articles will be missing.
 */

int
create_header_array_from_xhdr (news_server_type * server,
			       news_overview_list_type ** overviews,
			       buffer_type * buffer,
			       unsigned long num_arts)
{

  news_overview_type * header_array;
  news_overview_list_type * overview_list;
  int i;
  int inconsistentp = 0;
  char * data = buffer->data;
  
  overview_list = (news_overview_list_type *)
    news_malloc (sizeof (news_overview_list_type));
  
  if (!overview_list) {
    return (ERR_CANT_MALLOC);
  }

  /* impdep->current group should exist here */
  /* It _does_ _not_, ouch! 940420 */
  /* num_arts = NNTP_STUFF(server)->current_group->estnum; */
  /* now being passed in FIXME (clean this up) */

  header_array = (news_overview_type *)
    news_malloc (num_arts * sizeof(news_overview_type));
  
  if (!header_array) {
    news_free (overview_list);
    buffer_free (buffer);
    NNTP_STUFF(server)->buffer = NULL;
    return (ERR_CANT_MALLOC);
  }
  
  /* this gets confusing, perhaps need to rename some things */ 

  *overviews = overview_list;
  overview_list->overviews = header_array;
  overview_list->length = num_arts;
  overview_list->overview_buffer = buffer;

  /* subject date from lines message_id references */

  data = buffer->data;

  /* for each header type ... */
  for (i = 0; i < 6; i++) {
    unsigned long index = 0;
    for (;;) {
      unsigned long artnum;
      news_overview_type * hi = &header_array[index];
      char * tail;
    
      /* parse the article number */
      artnum = strtoul (data, &tail, 10);
      if (tail == data) {
	buffer_free (buffer);
	news_free (header_array);
	return (ERR_CANT_PARSE);
      }
      
      /*
       * check for sentinel, and skip to the next
       * header type if we find it.
       */
      if (artnum == 0) {
	if (strncmp (data, "0 SENTINEL\r\n", 12)) {
	  buffer_free(buffer);
	  news_free (header_array);
	  return(ERR_CANT_PARSE);
	}

	/* the first round is 'canonical' */
	if (i == 0) {
	  overview_list->length = index;
	  num_arts = index;
	}

	/* skip to next line */
	data += 12;	/* danger! depends on sentinel string */
	
	break;
      }

      /* setting the initial artnum */
      if (i == 0)
	hi->number = artnum;
      else
	if (artnum != hi->number) { /* mismatch! */
	  inconsistentp = 1;
	  /* the first (canonical) xhdr sequence did
	   * not contain this article number, so we just
	   * ignore this one.
	   */
	  if (artnum < hi->number) {
	    data = strchr (data, '\n');
	    if (!data) {
	      buffer_free (buffer);
	      news_free (header_array);
	      return (ERR_BOGUS_XHDR_DATA);
	    }
	    break;
	  }
	  else {
	    unsigned long bogus_index = index + 1;
	    /* skip to the correct artnum */
	    while ((bogus_index < num_arts) &&
		   ((header_array[bogus_index]).number != artnum)) {
	      bogus_index++;
	    }
	    if (artnum == (header_array[bogus_index]).number) {
	      hi = &header_array[bogus_index];
	    }
	  }
	}

      data = tail+1;		/* skip the space after artnum */
      
      /* subject date from lines message_id references */

      switch (i) {
      case 0:			/* subject */
	hi->subject = data;
	break;
      case 1:
	/* need to terminate the string early */
	tail = strchr (data, '\r');
	if (!tail) {
	  buffer_free (buffer);
	  news_free (header_array);
	  return (ERR_CANT_PARSE);
	}
	*tail = (char) NULL;

	/* If the date is bad, be silent. We *could* use an informative
	 * error return
	 */

	hi->date = parsedate (data, (TIMEINFO *) NULL);
	/* what a fun little game */
	*tail = '\n';
	break;
      case 2:
	hi->from = data;
	break;
      case 3:
	hi->lines = strtoul (data, &tail, 10);
	if (tail == data) {
	  buffer_free(buffer);
	  news_free (header_array);
	  return (ERR_CANT_PARSE);
	}
	break;
      case 4:
	hi->message_id = data;
	break;
      case 5:
	hi->references = data;
	break;
      default:
	/* this shouldn't happen 8^) */
	break;
      }
      /* skip to next line */
      tail = strchr (data, '\r');
      if (!tail) {
	buffer_free (buffer);
	news_free (header_array);
	return (ERR_CANT_PARSE);
      }
      *tail = (char) NULL;
      data = tail+2;
      index++;
    }
  }
  return (0);
}

/*
 * socket utils... these have been moved from {unix,windows}.c to here.
 */

/* use this for line-oriented responses */

/* sneaky bug.  'ch' managed to be uninitialized, gcc didn't catch it */
/* because -Wuninitialized doesn't work on auto vars that you take the */
/* address of */

int get_server_line(news_server_type * server)
{
  int retval;
  char ch = '\000';
  int index = 0;

  while (ch != '\r' && ch != '\n') {
    if ((retval = get_server_char (server, &ch)))
      return (retval);
    else
      (NNTP_STUFF(server)->server_line)[index++] = ch;
  }
  (NNTP_STUFF(server)->server_line)[index-1]= '\0';

  /* read the '\n' that should follow a '\r'. */
  /* Is this a bad assumption? */

  if (ch == '\r')
    if ((retval = get_server_char (server, &ch)))
      return (retval);

  return (0);
}

/* Thanks to Rhys Weatherley for tips */

/* use this for multi-line response */

/*
 * This is a little state machine to correctly process data from the
 * NNTP server.  This would be responses from commands such as ARTICLE,
 * HEAD, BODY, XOVER, and any as yet unknown multi-line-response extensions.
 * It's been done this way to ease the transition to binary articles,
 * which may come about in the next incarnation of Usenet.  This code
 * will correctly receive binary articles of arbitrary line length.
 */

/* this is broken */

int get_server_data (news_server_type * server)
{
  char ch;
  unsigned char	done = 0;
  unsigned int lines = 0;
  int num_chars = 0;
  char skip = 0;
  int retval;
  nntp_impdep_type * impdep = NNTP_STUFF(server);
  char lf[4] = "xxxx";	/* 'last four', a fifo */  
  /* convert "cr lf ." to "cr lf", and stop when you hit "cr lf . cr lf" */
  
  while (!done) {
    if ((retval = get_server_char (server,&ch)))
      return (retval);
    
    switch (ch) {
    case '\n':
      if (lf[0]=='\r' && lf[1]=='\n' && lf[2]=='.'  && lf[3]=='\r')
	done = 1;
      else
	lines++;
      break;
    case '.':
      if (lf[2]=='\r' && lf[3]=='\n')
	skip = 5;
	break;
    default:
      break;
    }
    /* shift */
    lf[0]=lf[1];lf[1]=lf[2];lf[2]=lf[3];lf[3]=ch;

    if (skip && (skip-- == 1))
      skip = 0;
    else if (num_chars++ > 2)
      if ((retval = buffer_add_data (impdep->buffer,&lf[0],1)))
	return (retval);
  }
  impdep->buffer->lines = lines;
  /* append a NULL char to the buffer (skip is re(ab)used)*/
  skip = 0;
  retval = buffer_add_data (impdep->buffer, &skip, 1);
  return (0);
}

/* a convenience function */
int put_server_line (news_server_type * server, char * outbuf)
{
  int length = strlen (outbuf);

  if (length <= 512)
    return (put_server_data(server, outbuf, length));
  else
    return (ERR_LINE_TOO_LONG);
}

