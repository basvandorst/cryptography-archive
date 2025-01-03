#ifndef _CRYPT_DEFINED

#define _CRYPT_DEFINED

/* Fixup for Windoze support.  We need to include windows.h for various types
   and prototypes needed for DLL's, and then fix up a type clash with a
   Windows predefined type - for some bizarre reason BYTE and WORD are
   unsigned, but LONG is signed */

#ifdef __WINDOWS__
  #include <windows.h>
  #undef LONG
#endif /* __WINDOWS__ */

/* Some useful types */

typedef int				BOOLEAN;
typedef unsigned char	BYTE;
#ifndef __WINDOWS__
  typedef unsigned int	WORD;
#endif /* __ WINDOWS__ */
typedef unsigned long	LONG;

/* Boolean consants */

#ifndef TRUE
  #define FALSE			0
  #define TRUE			!FALSE
#endif /* TRUE */

/* Machine-dependant types to allow use in special library types such as
   DLL's */

#ifdef __WINDOWS__
  #define CPTR	FAR *						/* DLL pointer */
  #define CRET	int FAR PASCAL _export		/* DLL return value */
#else
  #define CPTR	*							/* General pointer */
  #define CRET	int							/* General return value */
#endif /* __WINDOWS__ */

/* Determine the processor endianness.  Thanks to Shawn Clifford
   <sysop@robot.nuceng.ufl.edu> for this trick */

#if !defined( LITTLE_ENDIAN ) && !defined( BIG_ENDIAN )
  #ifdef _MSC_VER
	#define LITTLE_ENDIAN		/* MSC can't handle the following test */
  #else
	#if ( ( ( unsigned short ) ( 'AB' ) >> 8 ) == 'B' )
	  #define LITTLE_ENDIAN
	#elif ( ( ( unsigned short ) ( 'AB' ) >> 8 ) == 'A' )
	  #define BIG_ENDIAN
	#else
	  #error Cannot determine processor endianness - edit crypt.h and recompile
	#endif /* Endianness test */
  #endif /* Mickeysoft C check */
#endif /* !( LITTLE_ENDIAN || BIG_ENDIAN ) */

/****************************************************************************
*																			*
*						Encryption Algorithm Types/Modes					*
*																			*
****************************************************************************/

/* The encryption algorithms we can use */

typedef enum {
	CRYPT_ALGO_NONE,				/* No encryption */
	CRYPT_ALGO_MDCSHS,				/* MDC/SHS */
	CRYPT_ALGO_DES,					/* DES */
	CRYPT_ALGO_3DES,				/* Triple DES */
	CRYPT_ALGO_IDEA,				/* IDEA */

	CRYPT_ALGO_LAST					/* Last possible crypt algo value */
	} CRYPT_ALGO;

/* The encryption modes we can use */

typedef enum {
	/* No encryption */
	CRYPT_MODE_NONE,				/* No encryption */

	/* Stream cipher modes */
	CRYPT_MODE_STREAM,				/* Stream cipher */

	/* Block cipher modes */
	CRYPT_MODE_ECB,					/* ECB */
	CRYPT_MODE_CBC,					/* CBC */
	CRYPT_MODE_CFB,					/* CFB */
	CRYPT_MODE_OFB,					/* OFB */

	CRYPT_MODE_LAST					/* Last possible crypt algo value */
	} CRYPT_MODE;

/****************************************************************************
*																			*
*					Library-Wide Constants and Definitions					*
*																			*
****************************************************************************/

/* The maximum user key size - 2048 bits */

#define MAX_KEYSIZE			256

/* The maximum IV size - 256 bits */

#define MAX_IVSIZE			64

/* The maximum speed ratio for an encryption algorithm */

#define MAX_SPEED			1000

/* Macros to convert to and from the bit counts used for some encryption
   parameters */

#define bitsToBytes(bits)	( ( bits ) >> 3 )
#define bytesToBits(bytes)	( ( bytes ) << 3 )

/****************************************************************************
*																			*
*								Data Structures								*
*																			*
****************************************************************************/

/* The structure used to store internal information about the crypto library
   capabilities.  This information is used internally by the library and is
   not available to users */

struct CI;			/* Forward declaration for nested struct */

typedef struct CA {
	/* Basic identification information for the algorithm */
	CRYPT_ALGO cryptAlgo;			/* The encryption algorithm */
	CRYPT_MODE cryptMode;			/* The encryption mode */
	int blockSize;					/* The basic block size of the algorithm */
	char *name;						/* Algorithm name */
	int speed;						/* Speed relative to block copy */

	/* Keying information */
	int minKeySize;					/* Minimum key size in bytes */
	int keySize;					/* Recommended key size in bytes */
	int maxKeySize;					/* Maximum key size in bytes */

	/* IV information */
	int minIVsize;					/* Minimum IV size in bytes */
	int ivSize;						/* Recommended IV size in bytes */
	int maxIVsize;					/* Maximum IV size in bytes */

	/* The functions for implementing the algorithm */
	int ( *initFunction )( struct CI CPTR cryptInfo );
	int ( *initExFunction )( struct CI CPTR cryptInfo, void CPTR cryptInfoEx );
	int ( *endFunction )( struct CI CPTR cryptInfo );
	int ( *initKeyFunction )( struct CI CPTR cryptInfo );
	int ( *initIVFunction )( struct CI CPTR cryptInfo );
	int ( *encryptFunction )( struct CI CPTR cryptInfo, void CPTR buffer, int length );
	int ( *decryptFunction )( struct CI CPTR cryptInfo, void CPTR buffer, int length );

	/* The next record in the list */
	struct CA *next;				/* Pointer to the next record */
	} CAPABILITY_INFO;

/* An encryption context */

typedef struct CI {
	/* Basic information on the encryption we're using */
	CAPABILITY_INFO *capabilityInfo;/* The encryption capability data */

	/* User keying information.  The user key is the key as entered by the
	   user, the transformed user key is (for those algoriths which do
	   this) the user key transformed by whatever key preprocessing method
	   is used, stored in canonical form.  The IV is the initial IV stored
	   in canonical form */
	BYTE userKey[ MAX_KEYSIZE ];	/* User encryption key */
	int userKeyLength;				/* User encryption key length in bytes */
	BYTE transUserKey[ MAX_KEYSIZE ];	/* Transformed user key */
	BYTE iv[ MAX_IVSIZE ];			/* Initial IV */
	int ivLength;					/* IV length in bytes */
	BOOLEAN keySet;					/* Whether the key is set up */
	BOOLEAN ivSet;					/* Whether the IV is set up */

	/* Keying information.  The key is the raw encryption key stored in
	   whatever form is required by the algorithm.  This may be simply an
	   endianness-adjusted form of the transformed key (in the case of
	   algorithms like MDC/SHS) or a processed form of the user key (in the
	   case of algorithms like DES or IDEA).  The IV is the current working
	   IV stored in an endianness-adjusted form.  The ivCount is the number
	   of bytes of IV in use, and may be used for various chaining modes.
	   These fields may be unused if the algorithm is implemented in hardware */
	void *key;						/* Internal working key */
	int keyLength;					/* Internal key length in bytes */
	BYTE currentIV[ MAX_IVSIZE ];	/* Internal working IV */
	int ivCount;					/* Internal IV count for chaining modes */

	/* Private data needed by the algorithm */
	void *privateData;				/* For private use */
	} CRYPT_INFO;

/* Extra algorithm-specific information stored within a crypt context */

typedef struct {
	int keySetupIterations;		/* No.iterations for user key setup */
	} CRYPT_INFO_MDCSHS;

/* Results returned from the encryption capability query */

typedef struct {
	/* The algorithm, encryption mode, and algorithm name */
	CRYPT_ALGO cryptAlgo;			/* The encryption algorithm */
	CRYPT_MODE cryptMode;			/* The encryption mode */
	char *algoName;					/* The algorithm name */

	/* The algorithm parameters */
	int blockSize;					/* The basic block size of the algorithm */
	int minKeySize;					/* Minimum key size in bytes */
	int keySize;					/* Recommended key size in bytes */
	int maxKeySize;					/* Maximum key size in bytes */
	int minIVsize;					/* Minimum IV size in bytes */
	int ivSize;						/* Recommended IV size in bytes */
	int maxIVsize;					/* Maximum IV size in bytes */

	/* Various algorithm characteristics */
	int speed;						/* Speed relative to block copy */
	} CRYPT_QUERY_INFO;

/****************************************************************************
*																			*
*								Status Codes								*
*																			*
****************************************************************************/

/* No error in function call */

#define CRYPT_OK			0		/* No error */

/* Generic internal error */

#define CRYPT_ERROR			-1		/* Nonspecific error */

/* Error in parameters passed to function */

#define CRYPT_BADPARM		-2		/* Generic bad argument to function */
#define CRYPT_BADPARM1		-3		/* Bad argument, parameter 1 */
#define CRYPT_BADPARM2		-4		/* Bad argument, parameter 2 */
#define CRYPT_BADPARM3		-5		/* Bad argument, parameter 3 */
#define CRYPT_BADPARM4		-6		/* Bad argument, parameter 4 */
#define CRYPT_BADPARM5		-7		/* Bad argument, parameter 5 */
#define CRYPT_BADPARM6		-8		/* Bad argument, parameter 6 */
#define CRYPT_BADPARM7		-9		/* Bad argument, parameter 7 */
#define CRYPT_BADPARM8		-10		/* Bad argument, parameter 8 */
#define CRYPT_BADPARM9		-11		/* Bad argument, parameter 9 */
#define CRYPT_BADPARM10		-12		/* Bad argument, parameter 10 */
#define CRYPT_BADPARM11		-13		/* Bad argument, parameter 11 */
#define CRYPT_BADPARM12		-14		/* Bad argument, parameter 12 */
#define CRYPT_BADPARM13		-15		/* Bad argument, parameter 13 */
#define CRYPT_BADPARM14		-16		/* Bad argument, parameter 14 */
#define CRYPT_BADPARM15		-17		/* Bad argument, parameter 15 */

/* Errors due to insufficient resources */

#define CRYPT_NOMEM			-18		/* Out of memory */
#define CRYPT_NOTINITED		-19		/* Data has not been initialised */
#define CRYPT_INITED		-20		/* Data has already been initialised */
#define CRYPT_NOALGO		-21		/* Algorithm unavailable */
#define CRYPT_NOMODE		-22		/* Encryption mode unavailable */
#define CRYPT_NOKEY			-23		/* Key not initialised */
#define CRYPT_NOIV			-24		/* IV not initialised */

/* A macro to detect an error return value */

#define isStatusError(status)		( ( status ) < CRYPT_OK )
#define isStatusOK(status)			( ( status ) == CRYPT_OK )

/****************************************************************************
*																			*
*								Private Functions							*
*																			*
****************************************************************************/

/* Private functions for use only within the crypt library */

void longReverse( LONG *buffer, unsigned count );
void wordReverse( WORD *buffer, unsigned count );
void secureFree( void **pointer, int count );

/****************************************************************************
*																			*
*								Public Functions							*
*																			*
****************************************************************************/

/* Initialise and shut down the encryption library */

CRET initLibrary( void );
CRET endLibrary( void );

/* Query the capabilities of the encryption library */

CRET queryModeAvailability( CRYPT_ALGO cryptAlgo, CRYPT_MODE cryptMode );
CRET queryAlgoAvailability( CRYPT_ALGO cryptAlgo );
CRET queryAlgoModeInformation( CRYPT_ALGO cryptAlgo, CRYPT_MODE cryptMode, CRYPT_QUERY_INFO CPTR cryptQueryInfo );
CRET queryContextInformation( CRYPT_INFO CPTR cryptInfo, CRYPT_QUERY_INFO CPTR cryptQueryInfo );

/* Initialise and destroy an encryption context */

CRET initCryptContext( CRYPT_INFO CPTR cryptInfo, CRYPT_ALGO cryptAlgo, CRYPT_MODE cryptMode );
CRET initCryptContextEx( CRYPT_INFO CPTR cryptInfo, CRYPT_ALGO cryptAlgo, CRYPT_MODE cryptMode, void *cryptInfoEx );
CRET destroyCryptContext( CRYPT_INFO CPTR cryptInfo );

/* Load a user key into a crypt context */

CRET loadCryptContext( CRYPT_INFO CPTR cryptInfo, void CPTR key, int keyLength );

/* Load/retreive an IV into/from a crypt context */

CRET loadIV( CRYPT_INFO CPTR cryptInfo, void CPTR iv, int length );
CRET retrieveIV( CRYPT_INFO CPTR cryptInfo, void CPTR iv );

/* Encrypt/decrypt a block of memory */

CRET encryptBuffer( CRYPT_INFO CPTR cryptInfo, void CPTR buffer, int length );
CRET decryptBuffer( CRYPT_INFO CPTR cryptInfo, void CPTR buffer, int length );

#endif /* _CRYPT_DEFINED */
