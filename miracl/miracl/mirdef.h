/* 
 *   MIRACL definitions - mirdef.h
 *   This version suitable for use with most 16-bit computers
 *   e.g. IBM PC
 */
#define MIRACL 1
#define small int
                            /* smalls are usually ints, but see  *
                             * bnmuldv.any                       */

#define MAXNUM  1073741824L 
                            /* =2^[L-2] where L is the number    *
                             * of bits in a 'long' data type     */
 
#define MAXBASE 16384  
                            /* = 2^[W-2] where W is the number   *
                             * of bits in a small and L>=2W OR   *
                             * if double-small sized integer is  *
                             * available at machine code level   *
                             * and muldiv is written in assembly *
                             * language to take advantage of it. *
                             * Otherwise = 2^[(W/2)-1]           */

#define TOOBIG  16384
                            /* = 2^[I-2] where I is the number   *
                             * of bits in an 'int'               */

#define MSK     0xFF      
                            /* 2^(W/2) - 1                       */
#define BTS     8         
                            /* (W/2)                             */
#define WPERD   3
                            /* (Bits in double mantissa)/(W-2)   */
#define BIGGEST 1.0E38
                            /* biggest representable double      */
#define IBSIZ   132
                            /* Input buffer size                 */
#define OBSIZ   256        
                            /* Output buffer size                */

               /* some useful definitions */

#define bool int
#define forever for(;;)   
#define FALSE 0
#define TRUE 1
#define OFF 0
#define ON 1
#define PLUS 1
#define MINUS (-1)

