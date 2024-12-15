#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

/* keyman.c */
char *GetPrivateKey
  P((char *, TypKeySource *, R_RSA_PRIVATE_KEY *, unsigned char *,
     unsigned int, RIPEMInfo *));
char *GetNextUserRecordFromFile P((FILE *, unsigned int, char *, BOOL *));
char *WriteCert P((unsigned char *, RIPEMDatabase *));
char *WriteCRL P((unsigned char *, unsigned char *, RIPEMDatabase *));
int pbeWithMDAndDESWithCBC
  P((int, int, unsigned char *, unsigned int, unsigned char *, unsigned int,
     unsigned char *, unsigned int, unsigned int *));
void DESWithCBC
  P((int, unsigned char *, unsigned int, unsigned char *, unsigned char *));
void DumpPubKey P((R_RSA_PUBLIC_KEY *, FILE *));
void DumpPrivKey P((R_RSA_PRIVATE_KEY *, FILE *));
void DumpBigNum P((unsigned char *, int, FILE *));
char *GetLatestCRL
  P((RIPEMDatabase *, unsigned char **, unsigned char *, UINT4));
char *GetPreferencesByDigest
  P((RIPEMDatabase *, unsigned char **, unsigned char *, RIPEMInfo *));
char *WriteRIPEMPreferences
  P((unsigned char *, unsigned int, unsigned char *, RIPEMDatabase *));

#undef P
