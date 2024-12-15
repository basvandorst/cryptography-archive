/* normidpr.h -- Function prototypes for normid.c */

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

void NormalizeString P((char *pw));
void ToLower P((char *str));
void RemoveNonprinting P((char *str));


