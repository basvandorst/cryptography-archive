#ifdef MIME

/** Definitions for state operations **/
typedef struct state {
  FILE *fpin;
  FILE *fpout;
  char *prefix;
  unsigned int displaying : 1;
} state_t;

#define state_put(x,y,z) fwrite(x,1,y,z->fpout)
#define state_putc(c,s) fputc(c,s->fpout)
#define state_puts(x,y) fputs(x,y->fpout)
#define state_add_prefix(x) if(x->prefix)state_puts(x->prefix,x)

#endif

#undef PARAM
#ifdef __STDC__
#define PARAM(x) x
#else
#define PARAM(x) ()
#endif

/* Function prototypes */

/* lib/strmcpy.c */
extern char *strmcpy PARAM((char *, char *));

#ifdef USE_REMAILER
#ifdef MIME
extern int remailer_copy_message_across PARAM((FILE *, FILE *, int, mime_send_t *));
#else
extern int remailer_copy_message_across PARAM((FILE *, FILE *, int));
#endif
extern void remailer_proc PARAM((void));
#endif

#ifdef USE_PGP
extern int pgp_decrypt_init		PARAM((FILE **, FILE **, int));
extern int pgp_encrypt			PARAM((char *, char *, int));
extern int pgp_menu			PARAM((char *));
extern int pgp_extract_public_key	PARAM((void));
#ifdef MIME
extern void mime_pgp_decode		PARAM((mime_t *, state_t *));
#endif
#endif

extern int metapager		PARAM((FILE *, struct header_rec *, int));
extern int builtinplusplus	PARAM((FILE *, long, int, char **, int));

#ifdef MIME
extern int save_copy		PARAM((char *, char *, char *, char *, 
				       char *, int, mime_send_t *));
extern int append_copy_to_file	PARAM((char *, char *, char *, char *, 
				       char *, int, mime_send_t *));
extern FILE *write_header_info	PARAM((char *, char *, char *, char *, 
				       int, int, mime_send_t *));
extern int copy_message_across	PARAM((FILE *, FILE *, int, mime_send_t *));
extern int check_for_multipart	PARAM((FILE *, mime_send_t *));
extern int Include_Part		PARAM((FILE *, char *, int, mime_send_t *, 
				       int));

/* mime_decode.c */
extern void Xbit_decode			PARAM((state_t *, int));
extern void base64_decode		PARAM((state_t *, int, int));
extern void quoted_printable_decode	PARAM((state_t *, int, int));
extern void multipart_decode		PARAM((mime_t *, state_t *));
extern void multipart_0_decode          PARAM((mime_t *, state_t *));
extern void rfc822_decode		PARAM((mime_t *, state_t *));
extern void text_decode			PARAM((mime_t *, state_t *));
extern void text_unsupported_decode	PARAM((mime_t *, state_t *));
extern void null_decode			PARAM((mime_t *, state_t *));
extern void mime_decode			PARAM((mime_t *, state_t *));

typedef void CT_decoder			PARAM((mime_t *, state_t *));
typedef CT_decoder *CT_decoder_t;
extern CT_decoder_t select_CT_decoder	PARAM((mime_t *));

/* mime_encode.c */
extern void attach_generate_message		PARAM((mime_t *, FILE *, int,
						       mime_send_t *));
extern void base64_encode	PARAM((FILE *, FILE *, int, mime_send_t *));
extern void line_quoted_printable_encode	PARAM((char *, FILE *, int, int,
						       int, mime_send_t *));
extern void quoted_printable_encode		PARAM((FILE *, FILE *, int,
						       int, mime_send_t *));
extern int is_text_type				PARAM((char *, char *, int));
extern char *mime_generate_boundary		PARAM((char *));
extern void add_parameter		PARAM((char *,char *,char *,int, int));
extern void print_EOLN				PARAM((FILE *,int));
extern int update_encoding			PARAM((int *,int));
extern void write_encoded PARAM((FILE *, FILE *, int, int, int,mime_send_t *));
extern void mime_write_header			PARAM((FILE *, mime_send_t *));

/* mime_parse.c */
extern mime_t *mime_t_alloc             PARAM((void));
extern void mime_t_zero                 PARAM((mime_t *));
extern void mime_destroy		PARAM((mime_t *));
extern void mime_get_disposition        PARAM((char *, mime_t *));
extern void mime_get_content		PARAM((char *, mime_t *));
extern int mime_get_charset             PARAM((char *, char *, int));
extern char * mime_parse_content_opts	PARAM((char *));
extern void rfc822_reap_comments        PARAM((char *, char *, int));
extern int read_header_line             PARAM((FILE *,char *,int));
extern int mime_get_param               PARAM((char *,char *,char *,int));
extern void mime_warnings		PARAM((struct header_rec *));
extern mime_t *rfc822_parse		PARAM((FILE *, int));
extern void attach_parse		PARAM((struct header_rec *,FILE *));

/* attach_menu.c */
extern mime_t *attach_menu		PARAM((mime_t *, int));
#endif
