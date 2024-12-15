#ifdef __STDC__
# define P(s) s
#else
# define P(s) ()
#endif

void megcd P((BIGINT *p , BIGINT *q , BIGINT *m , int *msign , BIGINT *n , int *nsign , BIGINT *gcd ));

#undef P
