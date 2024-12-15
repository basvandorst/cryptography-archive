/*
 * bnsize00.h - pick the correct machine word size to use.
 *
 * Written by Colin Plumb.
 *
 * $Id: bnsize00.h,v 1.1 1997/12/14 11:30:39 wprice Exp $
 */
#include "bni.h"	/* Get basic information */

#if !BNSIZE64 && !BNSIZE32 && !BNSIZE16 && defined(BNWORD64)
# if defined(BNWORD128) || (defined(bniMulAdd1_64) && defined(bniMulSub1_64))
#  define BNSIZE64 1
# elif defined(mul64_ppmm) || defined(mul64_ppmma) || defined(mul64_ppmmaa)
#  define BNSIZE64 1
# endif
#endif

#if !BNSIZE64 && !BNSIZE32 && !BNSIZE16 && defined(BNWORD32)
# if defined(BNWORD64) || (defined(bniMulAdd1_32) && defined(bniMulSub1_32))
#  define BNSIZE32 1
# elif defined(mul32_ppmm) || defined(mul32_ppmma) || defined(mul32_ppmmaa)
#  define BNSIZE32 1
# endif
#endif

#if !BNSIZE64 && !BNSIZE32 && !BNSIZE16 && defined(BNWORD16)
# if defined(BNWORD32) || (defined(bniMulAdd1_16) && defined(bniMulSub1_16))
#  define BNSIZE16 1
# elif defined(mul16_ppmm) || defined(mul16_ppmma) || defined(mul16_ppmmaa)
#  define BNSIZE16 1
# endif
#endif

#if !BNSIZE64 && !BNSIZE32 && !BNSIZE16
#error Unable to find a viable word size to compile bignum library.
#endif
