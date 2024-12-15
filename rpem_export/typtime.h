#ifdef MSDOS
#define TYPTIME clock_t
#endif

#ifdef USEBSD
#include <sys/time.h>
#include <sys/resource.h>
#define TYPTIME struct rusage
#endif
