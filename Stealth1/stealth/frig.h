
/* 
 * Note : If you change anything here, you should also change the defs
 * in frig.inc
 */

#define ROUNDS 32
#define BLOCK_SIZE_BITS	512	/* Must be power of two, multiple of four */
#define BLOCK_SIZE	(BLOCK_SIZE_BITS/8)
#define KEY_SIZE_BITS	128
#define KEY_SIZE	(KEY_SIZE_BITS/8)
#define CONV_ARRAYS	64

