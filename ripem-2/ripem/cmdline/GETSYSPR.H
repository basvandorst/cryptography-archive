/* "Cannot allocate memory" error string to be used throughout command-line
   applications. */
extern char *ERROR_MALLOC;

#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif


/* getsys.c */
char *EstablishRIPEMHomeDir P((char **, RIPEMInfo *));
void WritePrintableName P((FILE *, DistinguishedNameStruct *));
char *StrCopyAlloc P((char **, char *));
char *StrConcatRealloc P((char **, char *));
int matchn P((char *str , char *pattern , int nchars ));
int GetRandomBytes P((unsigned char *, int, RIPEMInfo *));
void CopyRandomBytes
  P((void *, int, unsigned char *, int *, int *, char *, RIPEMInfo *));
void ReportCPUTime P((char *, RIPEMInfo *));
void GetUserInput
  P((unsigned char userbytes [], int *num_userbytes ,
     unsigned char timebytes [], int *num_timebytes , int echo));
int  GetUserName P((char **name ));
unsigned int GetPasswordFromUser
  P((char *prompt , BOOL verify , unsigned char *password ,
     unsigned int maxchars ));
void GetUserAddress P((char **address ));
void GetUserHome P((char **home ));
void ExpandFilename P((char **fileName));
#ifndef MACTC	/* rwo */
BOOL GetEnvFileName P((char *envName , char *defName , char **fileName ));
BOOL GetEnvAlloc P((char *envName , char **target ));
#else
BOOL GetEnvFileName P((short envName , char *defName , char **fileName ));
BOOL GetEnvAlloc P((short envName , char **target ));
#endif
#undef P
