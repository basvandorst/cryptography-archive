/******    bigint.h   *****/
/****************************************************************
*                                                               *
*       defines and structures for crypto section.              *
*               Jan. 21, 1995                                   *
*                                                               *
****************************************************************/

#define WORDSIZE        32
/*#define       MAXBITS         1024*/
#define MAXBITS         256
#define MAXLONG         (MAXBITS/WORDSIZE)
#define SUBMASK         0x80000000
#define LONGPOS         (MAXLONG-1)
#define MAXSHIFT        (WORDSIZE-1)
/*#define field_prime   1019
#define log2_fp         9
/*  that's the most significant power of 2, or floor(log2(field_prime)) */
#define field_prime   149
#define log2_fp         7
/*#define field_prime     13
#define log2_fp         3 */
#define NUMBITS         (field_prime-1)
#define NUMWORD         (NUMBITS/WORDSIZE)
#define UPRSHIFT        (NUMBITS%WORDSIZE)
#define UPRBIT          (1L<<(UPRSHIFT-1))
#define STRTPOS         (LONGPOS-NUMWORD)
#define UPRMASK         (~(-1L<<UPRSHIFT))

typedef short int       INDEX;
typedef unsigned long   ELEMENT;
typedef struct  {
        ELEMENT b[MAXLONG];
} BIGINT;
