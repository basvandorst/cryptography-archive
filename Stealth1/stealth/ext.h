
/* define TRUE and FALSE */

#ifndef TRUE
#define	TRUE	1
#define FALSE	0
#endif

typedef	unsigned char	byte;
typedef	short		int16;
typedef long		int32;
typedef unsigned short	word;
typedef unsigned long	lword;

#ifdef DOS
#define random()	rand()
#define srandom(s)	srand(s)
#endif

#ifdef SYSV
#define random()	lrand48()
#define srandom(s)	srand48(s)
#endif

