
/*	gmp-impl+.h:	add a few more facilities to gmp-impl.h

	Ken Weber (kweber@mcs.kent.edu)		06/08/93
*/

#include "gmp-impl.h"

#if 0
/*  Remove  */
#define MPN_NORMALIZE_SIZE(u,ul) do if ((u)[--(ul)]) {(ul)++; break;} while (1)
#endif

/*  Swap (mp_ptr, mp_size_t) (U, UL) with (V, VL)  */
#define MPN_SWAP(u,l,v,m)\
    do\
    {   {mp_ptr _; _ = (u), (u) = (v), (v) = _;}\
	{mp_size_t _; _ = (l), (l) = (m), (m) = _;}\
    }   while (0)\

/*  Return true iff the limb X has less bits than the limb Y.  */
#define MPN_LESS_BITS_LIMB(x,y) ((x) < (y) && (x) < ((x) ^ (y)))

/*  Return true iff (mp_ptr, mp_size_t) (U, UL) has less bits than (V, VL).  */
#define MPN_LESS_BITS(u,l,v,m) \
    (   (l) < (m) \
	|| ((l)==(m) && (l)!=0 && MPN_LESS_BITS_LIMB((u)[(l-1)],(v)[(l)-1]))\
    )

/*  Return true iff (mp_ptr, mp_size_t) (U, UL) has more bits than (V, VL).  */
#define MPN_MORE_BITS(u,l,v,m) MPN_LESS_BITS(v,m,u,l)

/*  Perform twos complement on (mp_ptr, mp_size_t) (U, UL), 
    putting result at (v, VL).
    Precondition: U[0] != 0.
*/
#define MPN_COMPL_INCR(u, v, l)	\
    do				\
    {   mp_size_t _ = 0;	\
	(u)[0] = -(v)[_];	\
	while (_++ < (l)) 	\
	    (u)[_] = ~(v)[_];	\
    }   while (0)

#define MPN_COMPL MPN_COMPL_INCR
