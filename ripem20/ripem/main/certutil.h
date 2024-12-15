#ifdef __STDC__
# define  P(s) s
#else
# define P(s) ()
#endif

/* Forward declaration. */
struct CertificateStruct;

typedef struct RIPEM_CRLsFrame {
  BufferStream outStream;     /* This holds the value returned to the caller */
} RIPEM_CRLsFrame;

void CheckSelfSignedCert
  P((int *, struct CertificateStruct *, unsigned char *, unsigned int));
void GetDNSmartNameIndex P((unsigned int *, DistinguishedNameStruct *));
char *InsertCerts P((TypList *, RIPEMInfo *, RIPEMDatabase *));
char *InsertCRLs P((TypList *, RIPEMInfo *, RIPEMDatabase *));
void RIPEM_CRLsFrameConstructor P((RIPEM_CRLsFrame *));
void RIPEM_CRLsFrameDestructor P((RIPEM_CRLsFrame *));
void ComputeIssuerSerialAlias
  P((unsigned char *, DistinguishedNameStruct *, unsigned char *,
     unsigned int));
char *RIPEMLoadPreferences P((RIPEMInfo *, RIPEMDatabase *));
BOOL IsIssuerSerialAlias P((RIPEMInfo *, unsigned char *));
char *AddUserIssuerCerts P((RIPEMInfo *, TypList *, RIPEMDatabase *));
char *WriteOriginatorAndIssuerCerts P((RIPEMInfo *, BufferStream *, int));

#undef P
