/*  pgpsendmail.h

    Various definitions for PGPsendmail.

    Copyright (C) 1994-1997  Richard Gooch

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Richard Gooch may be reached by email at  rgooch@atnf.csiro.au
    The postal address is:
      Richard Gooch, c/o ATNF, P. O. Box 76, Epping, N.S.W., 2121, Australia.
*/

#include <stdio.h>
#include <sys/types.h>  /* Need before  malloc.h  because BSDI 1.0 is broken */
/*  #include <malloc.h>    This doesn't exist on NEXTSTEP or BSDI 1.0 :-(
                           I hear <malloc.h> is a SysV-ism. I work with SunOS
			   and Linux mostly, which are a little schizoid.    */
#include <stdlib.h>     /* Needed for define of malloc()                     */
#include <karma.h>

#if !defined(SIGIO) && defined(SIGPOLL)
#  define SIGIO SIGPOLL
#endif

#define LINE_LENGTH 1024
#define BUF_LEN 4096
#define SECRING_SIZE_TYPE int

#define CHECK_CAN_ENCRYPT 0
#define CHECK_NO_KEY 1
#define CHECK_AMBIGUOUS_KEY 2
#define CHECK_NO_KEYLIST 3

#define INPUT_FD 0
#define OUTPUT_FD 1
#define ERROR_FD 2

#define m_alloc malloc
#define m_free free
#define m_abort(a,b) exit (1)
#define a_prog_bug(a) exit (1)

typedef struct
{
    flag secure;
    flag insecure;
    flag receipt;
    flag add_key;
    flag no_advertising;
    flag sign;
    flag global_done;
} option_type;

typedef struct
{
    flag scan_to;
    flag scan_cc;
    flag scan_bcc;
    flag scan_from;
    flag scan_sender;
    char *linebuf;
    unsigned int linebuf_size;
    unsigned int line_length;
    unsigned int next_pos;
    flag continuation_line;
} decode_type;

typedef struct
{
    char **strings;
    unsigned int num_set;
    unsigned int num_allocated;
} stringlist_type;


/*  File:  strings.c  */
EXTERN_FUNCTION (int st_nicmp, (CONST char *string1, CONST char *string2,
				int str_len) );
EXTERN_FUNCTION (char *st_nupr, (char *string, int str_len) );
EXTERN_FUNCTION (char *st_istr, (CONST char *string, CONST char *substring) );
EXTERN_FUNCTION (char *ex_str, (char *str, char **rest) );
EXTERN_FUNCTION (int st_icmp, (CONST char *string1, CONST char *string2) );
EXTERN_FUNCTION (char *st_upr, (char *string) );
EXTERN_FUNCTION (flag add_string,
		 (stringlist_type *list, CONST char *string, flag new_alloc) );


/*  File:  keylist.c  */
EXTERN_FUNCTION (flag freshen_keylist, (char *pgppath) );
EXTERN_FUNCTION (unsigned int check_recipients,
		 (char **recipients, char *pgppath, char **bad_recipient) );
EXTERN_FUNCTION (flag include_mykey,
		 (CONST char *pgppath, FILE *sendmail_fp) );
EXTERN_FUNCTION (char *find_field, (CONST char *string, int number) );


/*  File:  spawn.c  */
EXTERN_FUNCTION (int spawn_job, (char *path, char *argv[],
				 int *in_fd, int *out_fd, int *err_fd) );


/*  File:  options.c  */
EXTERN_FUNCTION (flag process_flags, (char *line, option_type *options) );
EXTERN_FUNCTION (flag translate_recipient,
		 (CONST char *recipient, FILE *config_fp,
		  option_type *options, FILE *debug_fp,
		  stringlist_type *recipient_list) );


/*  File:  conn_to_pgpd.c  */
EXTERN_FUNCTION (flag connect_to_pgpdaemon,
		 (CONST char *pgppath, int *to_fd, int *from_fd) );


/*  File:  memory.c  */
EXTERN_FUNCTION (void m_clear, (char *memory, unsigned int length) );
EXTERN_FUNCTION (void m_copy, (char *dest, CONST char *source,
			       unsigned int length) );


/*  File:  misc.c  */
EXTERN_FUNCTION (flag copy_data, (int out_fd, int in_fd,
				  flag return_on_terminator) );
EXTERN_FUNCTION (int set_env, (CONST char *env_name, CONST char *env_value) );
EXTERN_FUNCTION (flag read_line_fd, (int fd, char *buffer, unsigned int length,
				     flag discard_newline) );
EXTERN_FUNCTION (flag scrub_file, (char *filename, flag remove) );
EXTERN_FUNCTION (flag search_file,
		 (CONST char *filename, CONST char **strings, FILE *debug_fp,
		  flag *found) );
EXTERN_FUNCTION (flag read_fp_line,
		 (FILE *fp, char *buffer, unsigned int buf_len,
		  FILE *debug_fp, flag *eof) );


/*  File:  header.c  */
EXTERN_FUNCTION (char *get_next_address,
		 (decode_type *info, char error[STRING_LENGTH]) );
EXTERN_FUNCTION (flag read_line,
		 (int fd, decode_type *info, FILE *debug_fp, flag *eof) );


/*  File: expression.c  */
EXTERN_FUNCTION (flag st_expr_expand,
		 (char *output, unsigned int length, CONST char *input,
		  FILE *errfp) );
