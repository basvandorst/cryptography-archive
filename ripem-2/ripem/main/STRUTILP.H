#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif


/* strutil.c */
int R_match P((char *str , char *pattern ));
void R_trim P((char *line ));
int WhiteSpace P((int ch ));
BOOL LineIsWhiteSpace P((char *line ));
char *strcpyalloc P((char **target , char *source ));
char *strcatrealloc P((char **target , char *source ));
char *ExtractEmailAddr P((char *addr ));
char *BreakUpEmailAddr
  P((char *addr , char *userName , int lenUser , char *hostName ,
     int lenHost ));
int EmailHostnameComponents P((char *addr ));
int EmailAddrUpALevel P((char *addr ));
BOOL EmailMatch P((char *user , char *candidate ));
char *LowerCaseString P((char *str ));
void MakeHexDigest P((unsigned char *buf , unsigned int buflen , char *hex_digest ));

#undef P
