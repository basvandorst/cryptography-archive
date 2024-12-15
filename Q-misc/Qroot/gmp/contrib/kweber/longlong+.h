/*	longlong+.h:  add a few more facilities to longlong.h

	Ken Weber (kweber@mcs.kent.edu)		06/08/93
*/

#include "longlong.h"

#if defined (__ns32000__)
#define count_trailing_zeros(count,x) \
    do						\
    {   unsigned long int _ = 0;		\
	__asm__ 				\
	("  ## inlined count_trailing_zeros
	    ffsd 	%2,%0
	    ## end inlined count_trailing_zeros"\
	    :   "=r" ((unsigned long int)_)	\
	    :   "0" ((unsigned long int)_),	\
		"r" ((unsigned long int)(x))	\
	);					\
	(count) = _;				\
    } while (0)
#endif

#if !defined (count_trailing_zeros)
#define count_trailing_zeros(count, x) \
do					\
{   mp_limb _ = (x);			\
    unsigned long int __;		\
    count_leading_zeros(__,_ & -_);	\
    (count) = BITS_PER_MP_LIMB - 1 - __;\
}   while (0)
#endif

