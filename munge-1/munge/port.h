#ifdef solaris2
#define bzero(a,n) memset(a,0,n)
#define bcopy(a,b,n) memcpy(b,a,n)
#define index(a,b) strchr(a,b)
#define rindex(a,b) strrchr(a,b)
#endif
