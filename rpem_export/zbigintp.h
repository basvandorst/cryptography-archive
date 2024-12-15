#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif

void zstart P((void ));
void zsubmul P((MYINT r , MYINT *a , MYINT *b ));
void zcopy P((MYINT *a , MYINT *b ));
void zintoz P((MYINT d , MYINT a []));
MYINT ztoint P((MYINT *a ));
int zcompare P((MYINT *a , MYINT *b ));
void zsadd P((MYINT a [], MYINT d , MYINT b []));
void zaddls P((MYINT *a , MYINT *b , MYINT c []));
void zadd P((MYINT *a , MYINT *b , MYINT c []));
void zsub P((MYINT *a , MYINT *b , MYINT c []));
void zsmul P((MYINT *a , MYINT d , MYINT b []));
void zmul P((MYINT *a , MYINT *b , MYINT c []));
MYINT zsdiv P((MYINT *a , MYINT d , MYINT *b ));
void zread P((MYINT a []));
void fzread P((FILE *fil , MYINT a []));
MYINT zwrite P((MYINT a []));
MYINT fzwrite P((FILE *fil , MYINT a [], MYINT lll , char *eol , char *bol ));
void zdiv P((MYINT a [], MYINT b [], MYINT q [], MYINT r []));
void zmod P((MYINT a [], MYINT b [], MYINT r []));
void zaddmod P((MYINT a [], MYINT b [], MYINT n [], MYINT c []));
void zsubmod P((MYINT a [], MYINT b [], MYINT n [], MYINT c []));
void zsmulmod P((MYINT a [], MYINT d , MYINT n [], MYINT c []));
void zmulmod P((MYINT a [], MYINT b [], MYINT n [], MYINT c []));
void zmstart P((MYINT n []));
void ztom P((MYINT a [], MYINT n [], MYINT b []));
void zmtoz P((MYINT a [], MYINT n [], MYINT b []));
void zmont P((MYINT *a , MYINT *b , MYINT *n , MYINT c []));
MYINT zinv P((MYINT ain [], MYINT nin [], MYINT inv []));
void zrstart P((MYINT s ));
MYINT zrandomint P((MYINT b ));
MYINT zrandombits P((void ));
MYINT zprime P((MYINT m [], MYINT t ));
void zpstart P((void ));
void zpshift P((void ));
MYINT zpnext P((void ));
MYINT isqrt P((MYINT n ));
void zsqrt P((MYINT n [], MYINT r []));
void zexp P((MYINT a [], MYINT e [], MYINT n [], MYINT b []));
void zhalt P((char *msg ));

#undef P
