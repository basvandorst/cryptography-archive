#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif


/* derkey.c */
int DERToPubKey P((unsigned char *, R_RSA_PUBLIC_KEY *));
int DERToPrivKey P((unsigned char *, R_RSA_PRIVATE_KEY *));
int DERToEncryptedPrivKey
  P((unsigned char *, unsigned int, int *, unsigned char *, unsigned int *,
     unsigned char *, unsigned int *));
int gettaglen P((UINT2 *, unsigned int *, unsigned char **));

#undef P
