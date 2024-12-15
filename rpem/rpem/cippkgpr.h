#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif

void ChooseNumber P((MYINT nbits , unsigned char seed_bytes [], int nbytes_seed , BIGINT *p ));
void ChoosePrime P((MYINT nbits , unsigned char seed_bytes [], int nbytes_seed , BIGINT *p ));
MYINT BitsInBignum P((BIGINT *a ));
int BignumToBytes P((BIGINT *a , int minbytes , unsigned char bytes []));
void BytesToBignum P((unsigned char bytes [], int nbytes , BIGINT *a ));
int BytesToBignumRedun P((unsigned char bytes [], int nbytes , int totalbytes , BIGINT *a ));
int WhetherGoodMsg P((unsigned char *bytes , int nbytes ));
void RabinEncipherBignum P((BIGINT *zplain , BIGINT *modulus , BIGINT *zcipher ));
int RabinEncipherBuf P((unsigned char *bytesplain , int nbytes , BIGINT *modulus , unsigned char *bytescip ));
void RabinDecipherBignum P((BIGINT *cipnum , BIGINT *prime1 , BIGINT *prime2 , BIGINT *planum ));
int RabinDecipherBuf P((unsigned char *bytescip , int nbytes , BIGINT *prime1 , BIGINT *prime2 , unsigned char *bytesplain ));
int HexToBin P((char *hex , int maxbytes , unsigned char *bin ));
void BinToHex P((unsigned char *bin , int nbytes , char *hex ));
void ExpandBytes P((unsigned char *source , int nsour , unsigned char *target , int ntarg ));
void RandomizeBytes P((unsigned char *source , int nsour , unsigned char *target , int ntarg ));
int ConcatArrays P((unsigned char **arrays , int lengths [], int narray , unsigned char *targ , int targlimit ));
int GetEncodedBytes P((FILE *stream , unsigned char *bytes , int maxbytes ));
void PutEncodedBytes P((FILE *stream , unsigned char *bytes , int nbytes ));
void WilliamsE P((BIGINT *m , BIGINT *r , BIGINT *e ));
void WilliamsD P((BIGINT *L , BIGINT *r , BIGINT *d ));
void myzrstart P((MYINT s , BIGINT *biggest ));
void myzrandom P((BIGINT *bound , BIGINT *zran ));
#if 0
void display_time P((double begtime , double endtime , char *msg ));
double my_get_time P((void ));
#endif

#undef P
