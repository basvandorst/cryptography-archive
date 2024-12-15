#include <stdio.h>
#include "../include/sca.h"
main()
{
printf("sizeof = %d\n",sizeof(struct SCA_ErrList));

#if defined(_BSD42_) || defined(_SYSVxx_)
	fprintf(stderr,"BSD or SYSV\n");
#else
#ifdef _POSIX_
	fprintf(stderr,"POSIX\n");
#endif
#endif

#ifdef _BSD42_
	fprintf(stderr,"BSD\n");
#elif _SYSVxx_
	fprintf(stderr,"SYSV\n");

#elif _POSIX_
	fprintf(stderr,"POSIX\n");

#elif _HP_BSDxx_		/* KLAEREN */
	fprintf(stderr,"HP_BSD\n");

#endif
}
