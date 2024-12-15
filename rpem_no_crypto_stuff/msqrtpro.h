#ifdef __STDC__
# define P(s) s
#else
# define P(s) ()
#endif

void msqrt P((BIGINT *a , BIGINT *prime1 , BIGINT *prime2 , BIGINT sqrts []));

#undef P
