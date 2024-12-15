#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif

int GetRandomBytes P((unsigned char *bytes , int maxbytes ));
void GetUserInput P((unsigned char userbytes [], int *num_userbytes , unsigned char timebytes [], int *num_timebytes , int echo ));
void GetUserName P((char *name , int namelen ));
void GetUserAddress P((char *address , int addrlen ));
void GetUserHome P((char *home , int homelen ));

#undef P
