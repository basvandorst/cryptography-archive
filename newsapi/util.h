int parse_active(news_server_type * server,
		 buffer_type * buffer,
		 news_group_list_type ** group_list);

int parse_overview (news_overview_type * ov, char * index, char ** new_index);


char * string_concatenate (const char *first_string, ...);
void clone_string (buffer_type * to_buffer, char * from, char ** stored);

#define STR_EQ(s1,s2)	(!strcmp ((s1),(s2)))

buffer_type * buffer_create (size_t initial_size);
int buffer_add_line (buffer_type * buffer, char * line);
int buffer_add_data (buffer_type * buffer, char * data, size_t length);
void buffer_free (buffer_type * buffer);
int buffer_reset (buffer_type * buffer);

/* this is used by parsedate.y */

typedef struct _TIMEINFO {
  time_t	time;
  long		usec;
  long		tzone;
} TIMEINFO;

time_t parsedate(char * p, TIMEINFO * now);

news_group_type *
news_find_group (news_group_list_type * group_list,
		 char * group_name);

/* memory wrapper routines */
void * news_malloc (size_t size);
void news_free (void * ptr);
void * news_realloc (void * ptr, size_t newsize);

void * news_memmem (const void *needle,
		    size_t needle_len,
		    const void *haystack,
		    size_t haystack_len);

#define OVERVIEW_SIZE_ESTIMATE 200
