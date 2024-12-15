#ifndef _DEFS_DEFINED

/* Processor endianness.  Having to define this is unfortunate but is
   necessary to speed up the encryption code, especially the RSA code */

#if defined( __ARC__ ) || defined( __MSDOS__ ) || defined( __OS2__ ) || \
	defined( __VMS__ ) || defined( AIX370 ) || defined( AIX386 ) || \
	defined( ISC ) || defined( LINUX ) || defined( ULTRIX ) || \
	defined( ULTRIX_OLD )
  #define LITTLE_ENDIAN
#elif defined( AIX ) || defined( __AMIGA__ ) || defined( __ATARI__ ) || \
	  defined( IRIX ) || defined( __MAC__ ) || defined( MINT ) || \
	  defined( SUNOS )
  #define BIG_ENDIAN
#else
  #error Need to add processor endianness #define to start of defs.h
#endif /* Endianness defines */

/* Make sure none of the following have been defined elsewhere */

#ifdef BYTE
  #undef BYTE
#endif /* BYTE */
#ifdef WORD
  #undef WORD
#endif /* WORD */
#ifdef LONG
  #undef LONG
#endif /* LONG */
#ifdef QUAD
  #undef QUAD
#endif /* QUAD */
#ifdef LQUAD
  #undef LQUAD
#endif /* LQUAD */
#ifdef BOOLEAN
  #undef BOOLEAN
#endif /* BOOLEAN */

/* Various types */

typedef unsigned char		BYTE;
typedef unsigned short int	WORD;	/* 16-bit unsigned on most systems */
#ifndef _OS2EMX_H
typedef unsigned long int	LONG;	/* 32-bit unsigned on most systems */
#endif /* _OS2EMX.H */
typedef unsigned long int	QUAD;	/* 64-bit unsigned (fake as 32-bit) */
typedef unsigned long int	LQUAD;	/* 128-bit unsigned (fake as 32-bit) */

#ifdef __MSDOS__
typedef char	BOOLEAN;			/* If byte addressing is OK */
#else
typedef int		BOOLEAN;			/* If byte addressing is OK */
#endif /* __MSDOS__ */

/* Size of various data types */

#define BITS_PER_BYTE	8
#define BITS_PER_WORD	16
#define BITS_PER_LONG	32
#define BITS_PER_QUAD	64
#define BITS_PER_LQUAD	128

/* Boolean constants */

#define FALSE	0
#define TRUE	1

/* Exit status of functions. */

#define OK		0
#define ERROR	-1

/* 'inline' patch for compilers which can't handle this */

#if !( defined( __CPLUSPLUS__ ) || defined( __cplusplus ) ) || defined( __TSC__ )
  #define inline
#endif /* !( __CPLUSPLUS__ || __cplusplus ) */

/* #pragma patch for TopSpeed C */

#ifdef __TSC__
  #define warn
#endif /* __TSC__ */

/* Flag the fact we've been #included */

#define _DEFS_DEFINED

#endif /* _DEFS_DEFINED */
