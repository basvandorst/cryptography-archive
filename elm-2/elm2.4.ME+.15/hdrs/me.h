#define PUBLIC 

#undef P_
#ifdef __STDC__
#define P_(x) x
#else
#define P_(x) ()
#endif

#ifdef MIME

/** Definitions for state operations **/

struct state;

/* can't be 'char' as argumnet because problems of default
 * promotion rules of arguments -- therefore 'int' is used
 * in functios: state_filter, state_putc
 *
 * Notice also that we assumen here that char agument (as int)
 * is 'unsigned char'
 *
 *  - K E H
 */

typedef int state_filter P_((int, struct state *));

typedef struct state {
  unsigned int displaying : 1;
  char *prefix;

  FILE *fpin;
  FILE *fpout;

  char *inbuf;
  char *inreadp;

  char *outbuf;
  char *outwritep;
  int outbufsize;

  state_filter * filter;
} state_t;

#define state_add_prefix(x) if((x)->prefix)state_puts((x)->prefix,x)

extern void state_clear P_((state_t *));
extern void state_buffer P_((char *,state_t *));
extern void state_destroy P_((state_t *));

extern int state_put P_((char *, int, state_t *));
extern int state_puts P_((char *, state_t *));
extern int state_putc P_((int, state_t *));

extern char *state_gets P_((char *, int, state_t *));
extern void state_buffer P_((char *s, state_t *));
extern int state_getc P_((state_t *s));

extern int NULL_filter P_((int, struct state *));

#endif /* MIME */

/* fileutil.c */

extern FILE *open_end_update P_((char *));

/* lib/strmcpy.c */

extern char *strmcpy P_((char *, char *));

/* lib/safeopen.c */

extern FILE *safeopen P_((char *));
extern FILE *safeopen_rdwr P_((char *));

/* lib/dispaddr.c */

int DisplayAddress  P_((struct header_rec *,char *,int));
void get_real_name P_((char *, char *, int));

/* lib/qstring.c */
char *qstrpbrk P_((char *, char *));

/* lib/strincmp.c */

int strincmp P_((char *, char *, int));

/* lib/istrcmp.c */

int istrcmp P_((char *, char *));

/* src/remailer.c */

#ifdef USE_REMAILER
#ifdef MIME
extern int remailer_copy_message_across P_((FILE *, FILE *, int, mime_send_t *));
#else
extern int remailer_copy_message_across P_((FILE *, FILE *, int));
#endif
extern int remailer_proc P_((void));
#endif

#ifdef USE_PGP
extern int pgp_decrypt_init		P_((FILE **, FILE **, int));
extern int pgp_encrypt			P_((char *, char *, int, int));
extern int pgp_menu			P_((char *));
extern int pgp_extract_public_key	P_((void));
#ifdef MIME
extern void pgp_decode  		P_((mime_t *, state_t *));
#endif
#endif

extern int metapager		P_((FILE *, struct header_rec *, int));
extern int builtinplusplus	P_((FILE *, long, int, char **, int));

/* mime.c */

extern int have_metamail        P_((void));

/* parse_util.c */
extern void rfc822_reap_comments        P_((char *, char *, int));
extern int read_header_line             P_((FILE *,char *,int,int));
extern int classify_header              P_((char *));


/* fileio.h */
extern void copy_message P_((FILE *, struct header_rec *,
			     char *, FILE *, int));

extern void copy_plain P_((char *,FILE *,int, struct header_rec *, FILE *));
#ifdef MIME
extern void copy_mime P_((char *,FILE *,int, struct header_rec *, FILE *));
#endif

typedef void copy_decoder P_((char *,FILE *,int, struct header_rec *, FILE *));
typedef copy_decoder *copy_decoder_t;
extern copy_decoder_t select_copy_decoder P_((struct header_rec *));

/* newmbox.c */

extern void header_zero P_((struct header_rec *));

/* syscall.c */
#define FDVEC_TO_PRG       1
#define FDVEC_DEFINE       2
#define FDVEC_STDIN        4
#define FDVEC_STDOUT       8
#define FDVEC_STDERR      16
#define FDVEC_FILE        32
#define FDVEC_END         -1

struct fdvec {
  int fd;
  char * name;
  int flag;

  /* used only internally: */
  int fd2;
};

extern int system_call    P_((char *, int));
#ifdef MAYBE_IN_NEXT_RELEASE
extern int start_pipes    P_((char *,struct fdvec [], char * [], int,int));
extern int end_pipes      P_((int,char *,struct fdvec [],int, char *));
extern void close_pipes   P_((struct fdvec []));
#endif

/* lib/errno.c */

extern char *error_description P_((int));

#ifdef MIME
extern int save_copy		P_((char *, char *, char *, char *, 
				       char *, int, mime_send_t *));
extern int append_copy_to_file	P_((char *, char *, char *, char *, 
				       char *, int, mime_send_t *));
extern FILE *write_header_info	P_((char *, char *, char *, char *, 
				       int, int, mime_send_t *));
extern int copy_message_across	P_((FILE *, FILE *, int, mime_send_t *));
extern int check_for_multipart	P_((FILE *, mime_send_t *));
extern int Include_Part		P_((FILE *, char *, int, mime_send_t *, int));

/* mime_decode.c */

extern void base64_decode		P_((state_t *, int, int));
extern void quoted_printable_decode	P_((state_t *, int, int));
extern void null_decode			P_((mime_t *, state_t *));
extern void mime_decode			P_((mime_t *, state_t *));
extern void rfc1522_decode P_((char *, int));
extern void rfc1522_decode_structured   P_((int, char *, int));
extern int is_rfc1522 P_((char *));

typedef void CT_decoder			P_((mime_t *, state_t *));
typedef CT_decoder *CT_decoder_t;
extern CT_decoder_t select_CT_decoder	P_((mime_t *));

/* mime_encode.c */

extern void attach_generate_message P_((mime_t *, FILE *, int, mime_send_t *));
extern void base64_encode	    P_((FILE *, FILE *, int, mime_send_t *));
extern void line_quoted_printable_encode P_((char *, FILE *, int, int,
					       int, mime_send_t *));
extern void quoted_printable_encode	P_((FILE *, FILE *, int, int,
						mime_send_t *));
extern int is_text_type			P_((char *, char *, int));
extern char *mime_generate_boundary	P_((char *));
extern void add_parameter		P_((char *,char *,char *,int, int));
extern void print_EOLN			P_((FILE *,int));
extern int update_encoding		P_((int *,int));
extern void write_encoded   P_((FILE *, FILE *, int, int, int, mime_send_t *));
extern void mime_write_header		P_((FILE *, mime_send_t *));
extern void rfc1522_encode_text         P_((char *,int,char *,int));
/* mime_parse.c */

extern mime_t *mime_t_alloc             P_((void));
extern void mime_t_zero                 P_((mime_t *));
extern void mime_destroy		P_((mime_t *));
extern void mime_get_disposition        P_((char *, mime_t *));
extern void mime_get_content		P_((char *, mime_t *));
extern int class_charset                P_((char *));
extern int mime_get_charset             P_((char *, char *, int));
extern char * mime_parse_content_opts	P_((char *));
extern int mime_get_param               P_((char *,char *,char *,int));
extern void mime_warnings		P_((struct header_rec *));
extern mime_t *rfc822_parse		P_((FILE *, int));
extern void attach_parse		P_((struct header_rec *,FILE *));
extern mime_t *mime_read_header         P_((FILE *, int));

/* attach_menu.c */

extern mime_t *attach_menu		P_((mime_t *, int));

#endif
