/*
 * -------------------------------------------------------------------------
 * Implementation Dependent Data for FILE servers.
 * server->impdep will point to this.
 * -------------------------------------------------------------------------
 */

typedef struct _file_impdep_type {
  char *		spool_root;
  char *		lib_root;
  char *		overview_root;
  char *		inews_program;
  char *		dir_sep;
  buffer_type *		connect_data; /* copy of connect data held here */
} file_impdep_type;


#define FILE_STUFF(s1)	((file_impdep_type *)((s1)->impdep))

int
read_file_into_buffer (char * filename, buffer_type ** bufferp);

int
scan_group (char * dirname,
	    unsigned long * low,
	    unsigned long * high,
	    unsigned long * count,
	    unsigned long * artnums);

char *
construct_group_pathname (news_server_type * server,
			  char * group,
			  unsigned long artnum);

char *
construct_overview_pathname (news_server_type * server,
				    char * group);


news_overview_type *
find_overview (news_overview_list_type * nov_list, unsigned long artnum);

int
slurp_overviews (FILE *fd,
		 buffer_type * line_buffer,
		 buffer_type * nov_buffer,
		 unsigned long low,
		 unsigned long high,
		 news_overview_list_type * nov_list);

int
generate_overview (FILE * fd,
		   unsigned long artnum,
		   buffer_type * line_buf,
		   buffer_type * temp_buf,
		   buffer_type * nov_buf);
