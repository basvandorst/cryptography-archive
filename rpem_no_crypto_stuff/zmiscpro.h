#ifdef __STDC__
# define P(s) s
#else
# define P(s) ()
#endif

int ztest P((BIGINT *x ));
int zparity P((BIGINT *x ));
void zmult3 P((BIGINT *x , BIGINT *y , BIGINT *z ));
void zmultmod3 P((BIGINT *x , BIGINT *y , BIGINT *m , BIGINT *z ));
MYINT zlog P((BIGINT *x ));
void zshiftr P((BIGINT *x , MYINT n ));
void zshiftl P((BIGINT *x , MYINT n ));
void zdec2z P((char *str , BIGINT *a ));
void zset P((MYINT c , BIGINT *x ));
MYINT zlowbits P((BIGINT *x ));
void zsignadd P((BIGINT *a , int *asign , BIGINT *b , int *bsign , BIGINT *c , int *csign ));
void zsignsub P((BIGINT *a , int *asign , BIGINT *b , int *bsign , BIGINT *c , int *csign ));
void zsignmult P((BIGINT *a , int *asign , BIGINT *b , int *bsign , BIGINT *c , int *csign ));
void zsigndiv P((BIGINT *a , int *asign , BIGINT *b , int *bsign , BIGINT *c , int *csign , BIGINT *r , int *rsign ));
int zjacobi P((BIGINT *a , BIGINT *n ));

#undef P
