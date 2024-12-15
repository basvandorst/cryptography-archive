#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "newsapi.h"
#include "util.h"

/*
 * string_concatenate allocates and fills a temporary buffer with the
 *  concatentation of all its arguments, returning the buffer. It is the
 *  user's responsibilty to free the buffer when done with it.
 *
 * returns NULL if it can't allocate memory.
 *
 */

char * string_concatenate (const char *first_string, ...)
{
  va_list strings;
  const char * next = first_string;
  char * temp, * index;
  unsigned int total_size = 0;
  
  va_start (strings, first_string);

  while (next != NULL) {
    total_size += strlen (next);
    next = va_arg (strings, char *);
  }
  va_end (strings);

  total_size++; /* for the NULL */

  temp = (char *) news_malloc (total_size);
  if (!temp)
    return (NULL);
  
  *temp = '\000';

  va_start (strings, first_string);
  
  next = first_string;
  index = temp;
  while (next != NULL) {
    strcat (temp, next);
    next = va_arg (strings, char *);
  }

  return (temp);
}

/* clone a null-terminated string into a buffer */
void
clone_string (buffer_type * to_buffer, char * from, char ** stored)
{
  char a_null = '\000';
  if (from) {
    *stored = to_buffer->index;
    buffer_add_line (to_buffer, from);
    buffer_add_data (to_buffer, &a_null, 1);
  }
  else {
    *stored = NULL;
  }
}

#ifdef WIN32
/* This is here because the strchr on NT seems to be doing
 * a strlen (or something like it), slowing parse_{active,overview}
 * to a snail's pace
 */ 

char *
strchr (const char * string, int looking_for)
{
  for (;*string;string++)
    if (*string == (char) looking_for)
      return (string);
  return (NULL);
}
#endif

/*
 * parse the 'active' file or output of the nntp LIST command.
 */

int parse_active(news_server_type * server,
		 buffer_type * buffer,
		 news_group_list_type ** group_list)
{
  unsigned long i;
  char * index,* hop;
  news_group_type * group_array;
  news_group_list_type * group_list_object;
  
  /* create an array of news_group_type */
  group_array = (news_group_type *)
    news_malloc (buffer->lines * sizeof(news_group_type));

  if (!group_array) {
    return (ERR_CANT_MALLOC);
  }
  
  group_list_object = (news_group_list_type *)
    news_malloc (sizeof(news_group_list_type));

  if (!group_list_object) {
    news_free (group_array);
    return (ERR_CANT_MALLOC);
  }

  *group_list = group_list_object;
  group_list_object->groups = group_array;
  server->num_groups = buffer->lines;

  index = buffer->data;

  /* iterate through the buffer's lines, filling in the news_group object
   * with parsed LIST output.
   * 'group.name high low flags [\r]\n'
   */

  group_list_object->length = buffer->lines;
  group_list_object->list_buffer = buffer;

  for (i=0; i<group_list_object->length; i++) {
    news_group_type * gi = &group_array[i];
    char * tail;

    gi->group_name = index;
    hop = strchr (index, ' ');
    if (!hop)
      return (ERR_CANT_PARSE);
    *hop = '\000';
    hop++;
    gi->high = strtoul (hop, &tail, 10);
    if (tail == hop)
      return (ERR_CANT_PARSE);
    hop = tail;

    gi->low = strtoul (hop, &tail, 10);
    if (tail == hop)
      return (ERR_CANT_PARSE);
    hop = tail;

    gi->flags = *(hop+1);
    
    index = strchr (hop, '\n');
    if (!index)
      return (ERR_CANT_PARSE);
    else
      /* skip the newline */
      index++;
  }

  return(0);
}

/*
 *
 * 	memory buffer management
 *
 */


/* create a new auto-reallocing buffer */

buffer_type *
buffer_create (size_t initial_size) 
{
  buffer_type * bi;

  bi = (buffer_type *) news_malloc (sizeof(buffer_type));

  if (!bi)
    return (NULL);

  bi->data = (char *) news_malloc (initial_size);

  if (!(bi->data)) {
    news_free (bi);
    return (NULL);
  }    

  bi->index = bi->data;
  bi->limit = bi->data + initial_size;
  bi->size = initial_size;
  bi->lines = 0;
  
  return (bi);

}

int
buffer_reset (buffer_type * buffer)
{
  buffer->index = buffer->data;
  buffer->limit = buffer->data + buffer->size;
  buffer->lines = 0;
  return (0);
}

/*
 * buffer_add_line/data will realloc if there is no more room in
 * the buffer.
 */

int
buffer_add_line (buffer_type * buffer, char * line) 
{
  return (buffer_add_data (buffer, line, strlen(line)));
  buffer->lines++;
}  

int
buffer_add_data (buffer_type * buffer, char * data, size_t length)
{
  if (length + buffer->index > buffer->limit) {

    /* like tin, we increase buffer size by 50% */
    size_t new_size = buffer->size + buffer->size/2;
    char * new_buffer =
      (char *) news_realloc ((void *) buffer->data, new_size);
    
    if (!new_buffer) {
      return (ERR_CANT_MALLOC);
    }
    
    /* setup possibly new index */
    buffer->size = new_size;
    buffer->limit = new_buffer + new_size;
    buffer->index = new_buffer + (buffer->index - buffer->data);
    buffer->data = new_buffer;
  }
  
  /* optimized for the one-char adds that get_server_data does */
  if (length == 1)
    *(buffer->index) = *data;
  else
    memcpy (buffer->index, data, length);
  buffer->index += length;
  return(0);
}

/* free-object routines */

void
buffer_free (buffer_type * buffer)
{
  news_free (buffer->data);
  news_free (buffer);
}
   
int
news_free_article (news_article_type * article)
{
  news_free (article->data);
  news_free (article);
  return(0);
}

int
news_free_group_list (news_group_list_type * group_list)
{
  buffer_free (group_list->list_buffer);
  news_free (group_list->groups);
  news_free (group_list);
  return (0);
}
    
int
news_free_overviews (news_overview_list_type * overview_list)
{
  buffer_free (overview_list->overview_buffer);
  news_free (overview_list->overviews);
  news_free (overview_list);
  return (0);
}

/* convenient. */

unsigned long count_newlines (char * data)
{
  unsigned long count = 0;
  while (*data) {
    if (*data == '\n')
      count++;
    data++;
  }
  return (count);
}

/* void * memchr (const void *BLOCK, int C, size_t SIZE) */
/* int memcmp (const void *A1, const void *A2, size_t SIZE) */

/* clone of GNU's memmem */
void * news_memmem (const void *	needle,
		    size_t 		needle_len,
		    const void *	haystack,
		    size_t 		haystack_len)
{
  const char * index = (const char *) haystack;
  const char * end = index+haystack_len - needle_len;
  
  for (;;) {
    if ((index = memchr (index, ((const char *)needle)[0],
			 haystack_len - (index - (const char *)haystack)))) {
      if (index <= end) {
	if (memcmp (index, needle, needle_len)) {
	  index++;
	}
	else
	  return ((void *)index);
      }
      else
	return (NULL);
    }
    else
      return (NULL);
  }
}


/*
 * read_file_into_buffer
 * for now, no gymnastics.  assume POSIX.1 fstat().
 */

/*
 * FIXME
 * this needs to be redone, so that it can translate the local
 * end-of-line convention to 'cr lf'
 *
 */

int read_file_into_buffer (char * filename, buffer_type ** bufferp)
{
  
  FILE * handle;
  unsigned int file_size, num_read;
  struct stat sb;
  buffer_type * buffer;
  
  if (stat (filename, &sb) < 0) {
    return (ERR_CANT_OPEN_FILE);
  }

  handle = fopen (filename, "r");
  
  if (!handle)
    return (ERR_CANT_OPEN_FILE);
    
  file_size = sb.st_size;
  
  buffer = buffer_create (file_size + 1); /* file & NULL */

  if (!buffer) {
    fclose (handle);
    return (ERR_CANT_MALLOC);
  }

  /* *gulp!* (as opposed to slurp) */
  
  num_read = fread (buffer->data, 1, file_size, handle);

  if (num_read != file_size) {
    fclose (handle);
    buffer_free (buffer);
    return (ERR_CANT_READ_FILE);
  }

  *bufferp = buffer;
  buffer->index = buffer->data + num_read;
  /* add a NULL */
  *buffer->index = (char) 0;
  (buffer->index)++;
  buffer->lines = count_newlines (buffer->data);

  return (0);
}



/*
 *
 *  Construct the header array from the data in buffer
 *  
 */

/* FIXME belongs in util.c */

int
parse_overview (news_overview_type * ov, char * index, char ** new_index)
{
  char * tail;
  char * hop;
  
  /*
   * article number 
   */

  ov->number = strtoul (index, &tail, 10);
  if (tail == index) {
    return (ERR_CANT_PARSE);
  }

  /* can there be whitespace after the article number ? */
  if (*tail != '\t') {
    return (ERR_CANT_PARSE);
  }

  /*
   * subject 
   */

  ov->subject = ++tail;
  hop = strchr (tail, '\t');
  if (!hop) {
    return (ERR_CANT_PARSE);
  }
    
  *hop++ = '\000';

  /*
   * from 
   */

  ov->from = hop;
  hop = strchr (hop, '\t');
  if (!hop) {
    return (ERR_CANT_PARSE);
  }

  *hop++ = '\000';

  /*
   * date 
   */

  tail = strchr (hop, '\t'); 
  if (!tail) {
    return (ERR_CANT_PARSE);
  }

  /* the date string needs to be terminated */ 
  *tail = '\000';

  ov->date = parsedate (hop, (TIMEINFO *) NULL);

  if (ov->date == -1) {
    return (ERR_CANT_PARSE);
  }

  hop = tail+1;

  /*
   * message_id 
   */

  ov->message_id = hop;

  hop = strchr (hop, '\t');
  if (!hop) {
    return (ERR_CANT_PARSE);
  }

  *hop++ = '\000';
      
  /*
   * references (optional) 
   */

  ov->references = hop;
    
  hop = strchr (hop, '\t');
  if (!hop) {
    return (ERR_CANT_PARSE);
  }

  *hop++ = '\000';

  /*
   * byte count 
   */

  ov->bytes = strtoul (hop, &tail, 10);

  if (tail == hop) {
    return (ERR_CANT_PARSE);
  }
    
  hop = tail;

  /*
   * lines (optional)
   */
    
  if (*(++hop) == '\t')
    /* lines field not there */
    ov->lines = 0;
  else {
    ov->lines = strtoul (hop, &tail, 10);
    if (tail == hop) {
      return (ERR_CANT_PARSE);
    }
    else
      hop = tail;
  }

  /*
   * xref (optional), possibly mixed with other headers
   * don't think we're going to worry about Xref, 'cause if
   * the API user wants it, they can get it out of the article
   *
   */

  /* search for the end of the overview line */

  /* FIXME this should really be done outside of this function */

  tail = strchr (hop, '\n');
  if (!tail) {
    return (ERR_CANT_PARSE);
  }
    
  /* set up for the next line */
  *new_index = tail+1;
      
  return (0);

}
