typedef struct {
    long totalLength;
    unsigned long h[5];
    unsigned long w[80];
} SHS_CTX;

unsigned char *qshs();
#ifdef SOLARIS2X
#define bzero(b, l)             memset(b, 0, l)
#define bcopy(s, d, l)          memcpy(d, s, l)
#define bcmp(s, d, l)           (memcmp(s, d, l)? 1 : 0)
#endif

