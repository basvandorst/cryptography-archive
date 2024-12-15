#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sys/stat.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include "newsapi.h"
#include "file.h"
#include "util.h"

#ifdef HAVE_READDIR
#include <dirent.h>
#include <ctype.h>

/* originally based on GRPScandir from inn1.4 */

/*
 * this is an absolute mess.  I want an 'iterator', but I'm not sure
 * it can be done cleanly & portably.
 * Currently:
 *
 * if low or high are NULL, then all articles will
 * be considered.
 *
 * if artnums is non-NULL, then the array will be filled
 * in with article numbers.  In this case, low & high
 * should be set.
 * 
 */

int scan_group (char * dirname,
		unsigned long * low,
		unsigned long * high,
		unsigned long * count,
		unsigned long artnums[])
{
  struct dirent	*ep;
  DIR		*dp;
  unsigned long	i;
  char *	p;
  unsigned long t_low = ULONG_MAX;
  unsigned long t_high = 0;
  unsigned long index = 0;
  int all	= 1;
  
  if (count)
    *count = 0;

  if (low && high && (*low || *high))
    all = 0;

  dp = opendir(dirname);
  if (dp == NULL) {
    return (ERR_OPENING_GROUP);
  }

  while ((ep = readdir(dp)) != NULL) {
    /* Get the numeric value of the filename, if it's all digits. */
    for (p = ep->d_name, i = 0; *p; p++) {
      if (!isdigit(*p))
	break;
      i = i * 10 + *p - '0';
    }
    if (*p || i == 0)
      continue;
    if (i < t_low) t_low = i;
    if (i > t_high) t_high = i;

    if (artnums) {
      /* set the article number */
      if (!all) {
	if (i >= *low && i <= *high)
	  artnums[index++] = i;
      }
    }
    else {
      if (!all) {
	if (i >= *low && i <= *high)
	  (*count)++;
      }
      else
	(*count)++;
    }
  }
  closedir(dp);
  if (!artnums) {
    if (!*low && !*high) {
      *low = t_low;
      *high = t_high;
    }
  }
  return (0);
}
#else
int scan_group (char * dirname,
		unsigned long * low,
		unsigned long * high,
		unsigned long * count,
		unsigned long artnums[])
{
  return (ERR_NOT_IMPLEMENTED);
}
#endif

char * construct_group_pathname (news_server_type * server,
				 char * group,
				 unsigned long artnum)
{
  char * path;
  char * group_name;
  char * scan;
  char artnum_string[10];
  file_impdep_type * impdep = FILE_STUFF(server);

  /* copy the group string */
  group_name = string_concatenate (group, NULL);

  if (!group_name) return NULL;

  /* replace the dots with dir_sep */
  for (scan=group_name; *scan ; scan++)
    if (*scan=='.') *scan=*(impdep->dir_sep);
  
  /* if we're looking for an article pathname */
  if (artnum) {
    sprintf (artnum_string, "%lu", artnum);
    path = string_concatenate (impdep->spool_root, 	/* /usr/spool/news */
			       impdep->dir_sep, 	/* / */
			       group_name, 		/* comp/os/linux */
			       impdep->dir_sep, 	/* / */
			       artnum_string, 		/* 123 */
			       NULL);
  }
  else
    path = string_concatenate (impdep->spool_root, 	/* /usr/spool/news */
			       impdep->dir_sep, 	/* / */
			       group_name, 		/* comp/os/linux */
			       NULL);

  news_free (group_name);
  if (!path) return (NULL);
  
  return (path);
}

char * construct_overview_pathname (news_server_type * server,
				    char * group)
{
  char * path;
  char * group_name;
  char * scan;
  file_impdep_type * impdep = FILE_STUFF(server);

  /* copy the group string */
  group_name = string_concatenate (group, NULL);

  if (!group_name) return NULL;

  /* replace the dots with dir_sep */
  for (scan=group_name; *scan ; scan++)
    if (*scan=='.') *scan=*(impdep->dir_sep);
  
  path = string_concatenate (impdep->overview_root, 	/* /usr/spool/news */
			     impdep->dir_sep, 	/* / */
			     group_name, 	/* comp/os/linux */
			     impdep->dir_sep, 	/* / */
			     ".overview",	/* bad, hardcoded */
			     NULL);

  news_free (group_name);
  if (!path) return (NULL);
  
  return (path);
}


#define CHUNK_SIZE	1024
char chunk[CHUNK_SIZE];
char * chunk_index = chunk;

/* slurp a new line from fd into line_buf, safe for obnoxiously
 * long lines
 */

int
slurp_a_line (FILE * fd, buffer_type * line_buf)
{
  int done = 0;
  while (!done) {
    /* if we already have data in the buffer */
    if (chunk_index > chunk) {
      char * nl;
      /* is there a newline in there? */
      nl = (char *) memchr(chunk, '\n', (size_t) (chunk_index - chunk));
    
      if (nl) {
	*nl = '\000';		/* terminate the string */
	
	/* add the contents of the line to the buffer */
	buffer_add_data (line_buf, chunk, (size_t) (nl - chunk + 1) );
	
	nl++;
	
	/* move the leftover data to the front */
	memmove (chunk, nl, (size_t) (chunk_index - nl));
	chunk_index = chunk + (size_t) (chunk_index - nl);
	return (0);
      }
      else {
	/* add the part of the line to the buffer... */
	buffer_add_data(line_buf, chunk, (size_t) (chunk_index - chunk));
	chunk_index = chunk;
	/* and try again */
      }
    }
    /* we need to grab some data ... */
    else {
      size_t num_read = fread(chunk_index, (size_t)1, (size_t)CHUNK_SIZE, fd);
      if (num_read)
	chunk_index = chunk_index + num_read;
      else
	return (-1);
    }
  }
  /* shouldn't get here */
  return (-1);
}  

typedef ptrdiff_t overview_temp[5];

char * overview_headers[] = {   "Subject: ",
				"From: ",
				"Date: ",
				"Message-ID: ",
				"References: " };

int
generate_overview (FILE * fd,
		   unsigned long artnum,
		   buffer_type * line_buf,
		   buffer_type * temp_buf,
		   buffer_type * nov_buf)
{
  overview_temp offsets = {-1,-1,-1,-1,-1};
  int overview_lens[5] = {9,6,6,12,12};
  char num_template[] = "0000000000\t";
  int i;
  int last_head = -1;
  unsigned int lines = 0;	/* FIXME (fill me in!) */
  unsigned int bytes = 0;	/* FIXME (fill me in!) */
  
  buffer_reset (temp_buf);
  buffer_reset (line_buf);

  while (slurp_a_line(fd, line_buf) != -1) {
    /* blank line? (end of headers?) */
    if (!lines && line_buf->data[0] != '\000') {
      /* is this a continuation line? */

      if (isspace(line_buf->data[0])) {
	/* are we expecting a continuation? */
	if (last_head >= 0) {
	  char * p = line_buf->data;
	  /* skip past the whitespace */
	  while (isspace(*p))
	    p++;
	  /* back up to ignore the NULL */
	  temp_buf->index--;
	  buffer_add_data (temp_buf, p, strlen(p) + 1);
	  /* add a space */
	  buffer_add_data (temp_buf, " \000", 2);
	}
      }

      /* a new header */

      else {
	/* do we want this header ? */
	for (i = 0; i < 5; i++) {
	  if (strncmp(overview_headers[i], line_buf->data, overview_lens[i]) == 0) {
	    char * skip = line_buf->data + overview_lens[i];
	    /* store the offset of this string */
	    offsets[i] = temp_buf->index - temp_buf->data;
	    buffer_add_data (temp_buf,
			     skip,
			     strlen(skip) + 1);
	    /* keep track of which header in case of continuation lines */
	    last_head = i;
	    break;
	  }
	  /* need to reset this, else we'll get continuations tacked on
           * that don't belong 
	   */
	  else {
	    last_head = -1;
	  }
	}
      }
    }
    else {
      lines++;
    }
    buffer_reset (line_buf);
  }
    
  /* put together a .overview line */
  sprintf (num_template, "%ld\t", artnum);
  buffer_add_data (nov_buf, num_template, strlen(num_template));

  /* using the offsets, collect the line into nov_buf */
  for (i = 0; i < 5; i++) {
    char * stuff;
      
    if (offsets[i] != (ptrdiff_t) -1) {
      char * p;
      stuff = temp_buf->data + offsets[i];
      /* replace tabs with spaces */
      for (p = stuff; *p; p++)
	if (*p == '\t') *p = ' ';
      
      buffer_add_data (nov_buf, temp_buf->data + offsets[i], strlen(stuff));
    }
    buffer_add_data (nov_buf, "\t", 1);
  }
  /* now add the computed bytes and lines, and end the line */
  sprintf(num_template, "%ld\t", (unsigned long) bytes);
  buffer_add_data (nov_buf, num_template, strlen(num_template));
  sprintf(num_template, "%d\r\n", lines);
  buffer_add_data (nov_buf, num_template, strlen(num_template));

  buffer_reset (temp_buf);
  buffer_reset (line_buf);
  return (0);
}

/* do a binary search for an overview by article number */

news_overview_type *
find_overview (news_overview_list_type * nov_list, unsigned long artnum)
{
  unsigned long low = 0;
  unsigned long high = nov_list->length;
  unsigned long p;
  news_overview_type * ovs = nov_list->overviews;

  while ((high - low) > 1) {
    p = (high + low) / 2;
    if (artnum <= ovs[p-1].number)
      high = p;
    else
      low = p;
  }
  if (artnum == ovs[high-1].number)
    return (&ovs[high-1]);
  else
    return NULL;
}

/*
 * Grab all the overview lines from 'fd' that are in the
 * range [low, high], and store them into nov_buffer.
 *
 * Also, flag the relevant entries in nov_list, so we know
 * which ones we need to use generate_overview() on
 */

int
slurp_overviews (FILE *fd,
		 buffer_type * line_buffer,
		 buffer_type * nov_buffer,
		 unsigned long low,
		 unsigned long high,
		 news_overview_list_type * nov_list)
{
  unsigned long left = nov_list->length;
  
  while (left) {
    buffer_reset (line_buffer);
    if (slurp_a_line (fd, line_buffer) != -1) {
      unsigned long artnum;
      char * index = line_buffer->data;
      char * tail;
      
      artnum = strtoul (index, &tail, 10);
      
      /* does it start with a number ? */
      if (tail != index) {
	/* is the number in range ? */
	if (artnum >= low && artnum <= high) {
	  news_overview_type * ov;
	  /* copy this line to nov_buffer */
	  buffer_add_line (nov_buffer, line_buffer->data);
	  buffer_add_data (nov_buffer, "\r\n", 2);

	  /* in this situation, ov != NULL is guaranteed */ 
	  ov = find_overview (nov_list, artnum);
	  /* so we know we have an overview for this artnum */
	  ov->date = (time_t) 1;
	  left--;
	}
      }
    }
    else {
      /* EOF, no more lines */
      left = 0;
    }
  }
  return (0);
}

	  
