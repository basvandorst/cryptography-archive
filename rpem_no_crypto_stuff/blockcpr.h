#ifdef __STDC__
# define P(s) s
#else
# define P(s) ()
#endif

void BlockInit P((TypCipherInfo *cipptr ));
void BlockSetKey P((unsigned char *key , int nbytes , int encipher , TypCipherInfo *cipptr ));
void BlockEncipherBlock P((unsigned char *buf , TypCipherInfo *cipptr ));
void BlockDecipherBlock P((unsigned char *buf , TypCipherInfo *cipptr ));
int BlockEncipherBuf P((unsigned char *buf , int nbytes , TypCipherInfo *cipptr ));
int BlockDecipherBuf P((unsigned char *buf , int nbytes , TypCipherInfo *cipptr ));
void BlockFinish P((TypCipherInfo *cipptr ));

#undef P
