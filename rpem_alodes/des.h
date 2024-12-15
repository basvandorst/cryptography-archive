#ifndef	_DES_
# define	_DES_	1
#include "alodes.h"
typedef struct des {
#if 0
	int		desmode;
	unsigned char	kn[16][8];
#else
	int		mode;
	C_Block		key;
	Key_schedule	sched;
#endif
} DES;
#endif /* _DES_ */
