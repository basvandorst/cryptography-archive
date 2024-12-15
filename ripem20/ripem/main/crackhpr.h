typedef struct TypMsgInfo {
  enum enum_ids proc_type;    /* Processing type (ENCRYPTED, MIC-ONLY, etc.) */
  unsigned char iv[8];                         /* DES Initialization Vector. */
  int da;                                   /* Digest algorithm (DA_ symbol) */
  unsigned char *mic;                                       /* Encrypted MIC */
  unsigned int mic_len;                              /* # of bytes in above. */
  unsigned char *msg_key;                           /* Encrypted message key */
  unsigned int msg_key_len;                          /* # of bytes in above. */
  char *orig_name;                /* Originator's name, or NULL if not found */
  R_RSA_PUBLIC_KEY orig_pub_key;                     /* Originator's pub key */
  BOOL  got_orig_pub_key;                 /* TRUE if header has Orig pub key */
  int ea;
  TypList certs;                      /* First is the Originator-Certificate */
  TypList crls;

  /* These variables keep track of the state between calls to
       DoHeaderLine. */
  BOOL foundBeginBoundary, doingHeader, inEmailHeaders, thisUser;
  BufferStream extendedLine;
} TypMsgInfo;

#ifdef __STDC__
# define  P(s) s
#else
# define P(s) ()
#endif

void TypMsgInfoConstructor P((TypMsgInfo *));
void TypMsgInfoDestructor P((TypMsgInfo *));
char *ProcessHeaderLine
  P((RIPEMInfo *, TypMsgInfo *, BOOL *, char *, BOOL, TypList *));

#undef P
