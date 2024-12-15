#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif


/* getreta.c */
char *GetEmailAddress P((FILE *instream, FILE *outstream ));

#undef P
