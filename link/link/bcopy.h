
#ifdef MEMCPY
#define bcopy(s,d,l)   memcpy(d,s,l)
#define bzero(d,l)     memset(d,0,l)
#endif
