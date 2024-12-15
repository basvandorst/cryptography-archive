#ifndef _SHS_DEFINED

#define _SHS_DEFINED

/* Define the following to compile the SHS test code */

/*#define TEST_SHS			/**/

/* Define the following to use the updated SHS implementation */

/*#define NEW_SHS			/**/

#if 0

/* Determine the endianness if need be.  Thanks to Shawn Clifford
   <sysop@robot.nuceng.ufl.edu> for this trick */

#if !defined( LITTLE_ENDIAN ) && !defined( BIG_ENDIAN )
	#if ( ( ( unsigned short ) ( 'AB' ) >> 8 ) == 'B' )
		#define LITTLE_ENDIAN
	#elif ( ( ( unsigned short ) ( 'AB' ) >> 8 ) == 'A' )
		#define BIG_ENDIAN
	#else
		#error Cannot determine processor endianness - edit shs.h and recompile
	#endif /* Endianness test */
#endif /* !( LITTLE_ENDIAN || BIG_ENDIAN ) */
#endif /* 0 */

/* The SHS block size and message digest sizes, in bytes */

#define SHS_DATASIZE	64
#define SHS_DIGESTSIZE	20

/* The structure for storing SHS info */

typedef struct {
			   LONG digest[ 5 ];			/* Message digest */
			   LONG countLo, countHi;		/* 64-bit bit count */
			   LONG data[ 16 ];				/* SHS data buffer */
			   } SHS_INFO;

/* Message digest functions */

void shsInit( SHS_INFO *shsInfo );
void shsUpdate( SHS_INFO *shsInfo, BYTE *buffer, int count );
void shsFinal( SHS_INFO *shsInfo );

#endif /* _SHS_DEFINED */
