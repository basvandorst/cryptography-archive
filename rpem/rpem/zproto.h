#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

void zstart P((void ));
void zsubmul P((MYINT r , BIGINT *a , BIGINT *b ));
void zcopy P((BIGINT *a , BIGINT *b ));
void zintoz P((MYINT d , BIGINT *a));
MYINT ztoint P((BIGINT *a ));
int zcompare P((BIGINT *a , BIGINT *b ));
void zsadd P((BIGINT *a, MYINT d , BIGINT *b ));
void zaddls P((BIGINT *a , BIGINT *b , BIGINT *c));
void zadd P((BIGINT *a , BIGINT *b , BIGINT *c));
void zsub P((BIGINT *a , BIGINT *b , BIGINT *c));
void zsmul P((BIGINT *a , MYINT d , BIGINT *b));
void zmul P((BIGINT *a , BIGINT *b , BIGINT *c));
MYINT zsdiv P((BIGINT *a , MYINT d , BIGINT *b ));
void zread P((BIGINT *a));
void fzread P((FILE *fil , BIGINT *a));
MYINT zwrite P((BIGINT *a));
MYINT fzwrite P((FILE *fil , BIGINT *a, MYINT lll , char *eol , char *bol ));
void zdiv P((BIGINT *a, BIGINT *b, BIGINT *q, BIGINT *r));
void zmod P((BIGINT *a, BIGINT *b, BIGINT *r));
void zaddmod P((BIGINT *a, BIGINT *b, BIGINT *n, BIGINT *c));
void zsubmod P((BIGINT *a, BIGINT *b, BIGINT *n, BIGINT *c));
void zsmulmod P((BIGINT *a [], MYINT d , BIGINT *n, BIGINT *c));
void zmulmod P((BIGINT *a, BIGINT *b, BIGINT *n, BIGINT *c));
void zmstart P((BIGINT *n));
void ztom P((BIGINT *a, BIGINT *n, BIGINT *b));
void zmtoz P((BIGINT *a, BIGINT *n, BIGINT *b));
void zmont P((BIGINT *a, BIGINT *b, BIGINT *n, BIGINT *c));
MYINT zinv P((BIGINT *ain, BIGINT *nin, BIGINT *inv));
void zrstart P((MYINT s ));
MYINT zrandomint P((MYINT b ));
MYINT zrandombits P((void ));
MYINT zprime P((BIGINT *m, MYINT t ));
void zpstart P((void ));
void zpshift P((void ));
MYINT zpnext P((void ));
MYINT isqrt P((MYINT n ));
void zsqrt P((BIGINT *n, BIGINT *r));
void zexp P((BIGINT *a, BIGINT *e, BIGINT *n, BIGINT *b));
void zhalt P((char *msg ));
int ztest P((BIGINT *x ));
int zparity P((BIGINT *x ));
void zmult3 P((BIGINT *x , BIGINT *y , BIGINT *z ));
void zmultmod3 P((BIGINT *x , BIGINT *y , BIGINT *m , BIGINT *z ));
MYINT zlog P((BIGINT *x ));
void zshiftr P((BIGINT *x , MYINT n ));
void zshiftl P((BIGINT *x , MYINT n ));
void zset P((MYINT c , BIGINT *x ));
MYINT zlowbits P((BIGINT *x));

#undef P
