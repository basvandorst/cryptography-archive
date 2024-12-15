#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif


/* getsys.c */
int GetRandomBytes P((unsigned char *bytes , int maxbytes ));
void CopyRandomBytes P((void *thisBuf , int thisSize , unsigned char *userBuf , int *numBytes , int *maxBytes , char *message ));
void ReportCPUTime P((char *msg));
void GetUserInput P((unsigned char userbytes [], int *num_userbytes , unsigned char timebytes [], int *num_timebytes , int echo ));
int  GetUserName P((char **name ));
unsigned int GetPasswordFromUser P((char *prompt , int verify , unsigned char *password , unsigned int maxchars ));
void GetUserAddress P((char **address ));
void GetUserHome P((char **home ));
#undef P
