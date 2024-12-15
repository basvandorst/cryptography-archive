#ifdef __STDC__
# define P(s) s
#else
# define P(s) ()
#endif

int main P((int argc , char *argv []));
char *CrackCmd P((int argc , char *argv []));
void procoutbuf P((void ));
BOOL PosFileLine P((FILE *stream , char *field ));
BOOL GetFileLine P((FILE *stream , char *field , char *value , int valuelen ));
char *DoEncipher P((void ));
char *DoEncRecipient P((int irecip,unsigned char *messagekey, int keysize, char *recipient[], unsigned char *msgkeycip[], int msgkeycipsize[]));
char *EncipherHandle P((int inhand , FILE *outstream , unsigned char *key , unsigned char *init_vec , TypCipherInfo *cipinfo ));
void WriteHeader P((FILE *stream , TypFileMode filemode, int nrecip, char *recip[] , char *username ,
  TypCipherInfo *msgcipinfo , unsigned char *init_vec ,
  unsigned char *msgkeycip[] , int msgkeycipsize[] , unsigned char *mypubkey ,
  int mypubkeysize , unsigned char *mic , int micsize ));
char *DoDecipher P((void ));
char *DecipherStream P((FILE *instream,int outhandle,TypCipherInfo cipinfo,
  unsigned char *msg_key,int msg_key_size,unsigned char *init_vec));
char *CrackHeader P((FILE *stream , TypFileMode *filemode, TypCipherInfo *cipinfo , unsigned char *init_vec , unsigned char *msg_key_bytes , int *num_key_bytes));
int CrackHeaderLine P((char *line , char *field_name , char **vals , int maxvals , int *more ));
int match P((char *str , char *pattern ));
char *tail P((char *str , int num ));
int WhiteSpace P((int ch ));

#undef P
