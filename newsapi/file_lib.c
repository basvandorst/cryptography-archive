#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "newsapi.h"
#include "file.h"
#include "util.h"

/*
 *
 *  news_add_server
 *  connect to a news server
 *
 */

int file_add_server(file_connect_type * connect_data, news_server_type ** server)
{
  buffer_type *	clone;
  news_server_type * serv;
  file_impdep_type * file_stuff;

  serv = (news_server_type *) news_malloc (sizeof(news_server_type));

  if (!serv) {
    return (ERR_CANT_MALLOC);
  }

  file_stuff = (file_impdep_type *) news_malloc (sizeof(file_impdep_type));
  
  if (!file_stuff) {
    news_free (serv);
    return (ERR_CANT_MALLOC);
  }

  clone = buffer_create (128);

  if (!clone) {
    news_free(serv);
    news_free(file_stuff);
    return (ERR_CANT_MALLOC);
  }
  else {
    clone_string (clone, connect_data->spool_root, 	&(file_stuff->spool_root));
    clone_string (clone, connect_data->lib_root, 	&(file_stuff->lib_root));
    clone_string (clone, connect_data->overview_root, 	&(file_stuff->overview_root));
    clone_string (clone, connect_data->inews_program, 	&(file_stuff->inews_program));
    clone_string (clone, connect_data->dir_sep, 	&(file_stuff->dir_sep));
  }

  file_stuff->connect_data = clone;

  *server = serv;

  serv->server_type	=	FILE_SERVER_TYPE;
  serv->impdep		=	file_stuff;
  serv->num_groups	= 	0;
  strcpy (serv->hostname, "localhost");

  /* connect to server - verify the basic files are readable */
  
  return(0);
}  
  

int file_list_groups(news_server_type *	server,
		     news_group_list_type ** group_list)
{
  char * active_filename;
  buffer_type * active_buffer;
  int retval;

  active_buffer = (buffer_type *) news_malloc (sizeof(buffer_type));
  if (!active_buffer) return (ERR_CANT_MALLOC);
  
  /* normally, "/usr/lib/news/active" */

  active_filename =
    string_concatenate (FILE_STUFF(server)->lib_root,
			FILE_STUFF(server)->dir_sep,
			"active", NULL);
  
  if ((retval = read_file_into_buffer (active_filename, &active_buffer))) {
    news_free (active_filename);
    return (retval);
  }
  
  news_free (active_filename);

  if ((retval = parse_active (server, active_buffer, group_list)))
    return (retval);

  return (0);

}

/* 
 * should this:
 * 1) scan the directory
 *	o accurate
 * 	o not as portable
 *	o slow
 *
 *  -*or*-
 *
 * 2) scan the active file
 *	o possibly inaccurate
 *	o portable
 *	o fast
 *
 * 3) count lines in the .overview file
 *
 * currently, going with #1
 *
 */

int file_group (news_server_type *	server,
		char *			group,
		news_group_type *	group_info)
{
  int retval;
  char * group_pathname; 

  if (!group_info)
    return (ERR_BAD_ARGS);

  /* this tells scan_group to count all articles */

  group_info->low = 0;
  group_info->high = 0;
  group_pathname = construct_group_pathname (server, group, 0);
  if ((retval = scan_group (group_pathname,
			    &(group_info->low),
			    &(group_info->high),
			    &(group_info->estnum),
			    NULL
			    ))) {
    news_free (group_pathname);
    return (retval);
  }
  news_free (group_pathname);
  
  return (0);
}

/* this should probably be used throughout */

unsigned long
get_artnum (char * index)
{
  char * tail;
  unsigned long val;
  
  val = strtoul (index, &tail, 10);

  if (tail == index)
    return (0);
  else
    return (val);
}
    
int
test_overview (unsigned long desired, char * index)
{
  char * tail;
  unsigned long maybe;

  maybe = strtoul (index, &tail, 10);

  /* valid entry ? */
  if (tail == index)
    return (-2);
  
  if (desired < maybe)
    return (1);
  if (desired > maybe)
    return (-1);
  
  return (0);
}

#define ARTNUM_TEMPLATE "0000000000"

typedef int (*compare_fun_type) (const void *, const void *);

int compare_artnum (const unsigned long *a, const unsigned long *b)
{
  if (*a < *b) return -1;
  if (*a > *b) return 1;
  return 0;
}

int file_get_overviews
	(news_server_type * server,
	 char * 		group,
	 unsigned long 		low,
	 unsigned long 		high,
	 news_overview_list_type ** overviews)
{
  int 			retval;
  char * 		group_pathname;
  char * 		nov_path;
  char * 		index;
  unsigned long 	i, estnum;
  news_overview_type *	header_array;
  news_overview_list_type * overview_list;
  file_impdep_type * 	impdep = FILE_STUFF(server);
  buffer_type * 	nov_buffer = NULL;
  buffer_type * line_buf = buffer_create((size_t) 8192); /* FIXME */
  unsigned long *	artnums;
  int			need_to_gen = 0;
  
  if (!line_buf) {
    return (ERR_CANT_MALLOC);
  }

  group_pathname = construct_group_pathname (server, group, 0);

  if (!group_pathname) {
    return (ERR_CANT_MALLOC);
  }

  /* find out how many articles are on file in the range [low, high] */
  if ((retval = scan_group (group_pathname, &low, &high, &estnum, NULL))) {
    return (retval);
  }
  
  /* create an array of overview objects */
  header_array = (news_overview_type *)
    news_malloc (estnum * sizeof(news_overview_type));
  
  if (!header_array) {
    return (ERR_CANT_MALLOC);
  }
  
  /* get the article numbers... */
  artnums = (unsigned long *) news_malloc (sizeof(unsigned long) * estnum);
  if (!artnums) {
    news_free(header_array);
  }

  if ((retval = scan_group (group_pathname, &low, &high, NULL, artnums))) {
    news_free (artnums);
    news_free (header_array);
    return (retval);
  }
  
  /* sort the article number array, since readdir() makes no guarantees */

  qsort (artnums,
	 (size_t) estnum,
	 sizeof (unsigned long),
	 (compare_fun_type) &compare_artnum);

  /* fill in the article numbers, init the overviews */

  for (i = 0; i < estnum; i++) {
    header_array[i].subject = NULL;
    header_array[i].date = (time_t) 0;
    header_array[i].xref = NULL;
    header_array[i].number = artnums[i];
  }

  news_free (artnums);

  /* create and initialize an overview_list object */

  overview_list = (news_overview_list_type *)
    news_malloc (sizeof (news_overview_list_type));
  
  overview_list->overviews = header_array;
  overview_list->length = estnum;
  *overviews = overview_list;

  if (!overview_list) {
    news_free (header_array);
    return (ERR_CANT_MALLOC);
  }

  nov_buffer = buffer_create (estnum * OVERVIEW_SIZE_ESTIMATE);
  
  if (!nov_buffer) {
    return (ERR_CANT_MALLOC);
  }

  /* if we support .overview files, slurp the
   * relevant lines into nov_buffer
   */

  if (impdep->overview_root) {
    FILE * fd; 
    nov_path = construct_overview_pathname (server, group);
    
    fd = fopen(nov_path, "r");
    if (fd) {
      if ((retval = slurp_overviews (fd, line_buf, nov_buffer,
				     low, high, overview_list))) {
	return (retval);
      }
      fclose(fd);
      news_free (nov_path);
    }
  }
    
  /* Now, we make a pass through the headers, and if any are not
   * flagged (we have an overview line already in nov_buffer), then
   * we must generate one
   */

  for (i = 0; i < estnum; i++)
    if (!header_array[i].date) need_to_gen++;
  
  if (need_to_gen) {
    buffer_type * temp_buf = buffer_create((size_t) 8192); /* FIXME */
    char * template, *num_p;
    
    if (!temp_buf) /* FIXME */ return (ERR_CANT_MALLOC);

    template = string_concatenate (group_pathname,
				   FILE_STUFF(server)->dir_sep,
				   ARTNUM_TEMPLATE,
				   NULL);
    
    if (!template) /* FIXME */ return (ERR_CANT_MALLOC);

    /* get a pointer to the start of the article number */
    num_p = template + strlen(template) - strlen (ARTNUM_TEMPLATE);

    /* now go on to fill in the missing pieces */
    for (i = 0; i < estnum; i++) {
      if (header_array[i].date == (time_t) 0) {
	FILE * fd;
	
	sprintf(num_p, "%-ld", header_array[i].number);
	fd = fopen(template, "r");
	if (!fd) /* FIXME */ return (ERR_NO_SUCH_ARTICLE);
	if ((retval = generate_overview (fd,
					 header_array[i].number,
					 line_buf,
					 temp_buf,
					 nov_buffer))) {
	  /* FIXME */
	  return (retval);
	}
	fclose (fd);
      }
    }
    buffer_free (line_buf);
  }
    
  /* FINALLY, we can parse the overview data, and fill in header_array */

  index = nov_buffer->data;

  for (i=0; i < estnum ; i++) {
    char * hop;
    int result;
    unsigned long which;
    news_overview_type * ov = &header_array[i];
    
    if ((which = get_artnum(index)) != ov->number) {
      ov = find_overview (overview_list, which);
    }
    
    if ((result = parse_overview(ov, index, &hop)))
      return (result);
    else
      index = hop;
  }

  overview_list->overview_buffer = nov_buffer;
  return (0);
	  
}

int file_article
	(news_server_type *	server,
	 char * 		group,
	 unsigned long 		art_num,
	 news_article_type **	article)
{
  char * article_pathname;
  news_article_type * art;
  buffer_type * buffer;
  int retval;

  art = (news_article_type *) news_malloc (sizeof(news_article_type));
  if (!art) return (ERR_CANT_MALLOC);

  article_pathname = construct_group_pathname (server, group, art_num);
  
  if (!article_pathname) return (ERR_CANT_MALLOC);    

  if ((retval = read_file_into_buffer(article_pathname, &buffer)))
    return (retval);

  art->data = buffer->data;
  art->lines = buffer->lines;
  news_free (buffer);

  *article = art;
  return (0);
}

int file_post
	(news_server_type * 	server,
	 char * 		group,
	 char * 		post_data)
{
  /* on unix, invoke 'inews' */
  /* this is very OS dependent */
  return (ERR_NOT_IMPLEMENTED);
}

int file_close_server (news_server_type * server)
{
  if (server) {
    file_impdep_type * impdep = FILE_STUFF(server);

    if (impdep) {
      buffer_free (impdep->connect_data);
    }
  }
  
  return (0);
}

