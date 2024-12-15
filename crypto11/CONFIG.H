#ifndef CONFIG_H
#define CONFIG_H

// define this if you have a copy of RSAREF and wants to compile the RSAREF
// class wrappers

// #define USE_RSAREF

// define this if you want to be able to initialize RSAREF structures with bignums

// #define USE_BIGNUM_WITH_RSAREF

#define GZIP_OS_CODE 0

// how much memory to use for deflation
// note Borland C++ doesn't work with BIG_MEM (pointer wraps around segment boundary)

// #define SMALL_MEM
#define MEDIUM_MEM
// #define BIG_MEM

// select which MPILIB mult-mod algorithm to use

// #define PEASANT
// #define MERRITT
#define SMITH
// #define UPTON

#define UNIT32

// define this if you want the library to throw exceptions when things go wrong

#define THROW_EXCEPTIONS

// switch between different secure memory allocation mechnisms, this is the only
// one available right now

#define SECALLOC_DEFAULT

// Define this to 0 if your system doesn't differentiate between
// text mode and binary mode files.

// #define BINARY_MODE 0
#define BINARY_MODE ios::binary
// #define BINARY_MODE ios::bin

// Define the following to use the updated SHS implementation

// #define NEW_SHS

// Define the following if running on a little-endian CPU

#define LITTLE_ENDIAN

// Define this if you're running on a CPU with 32 bit registers

// #define IDEA32

// Define this if, for the linear congruential RNG, you want to use
// the original constants as specified in S.K. Park and K.W. Miller's
// CACM paper.

// #define LCRNG_ORIGINAL_NUMBERS

// Make sure these typedefs are correct for your computer

typedef unsigned char byte;
typedef unsigned short word16;
#ifdef __alpha
typedef unsigned int word32;
#else
typedef unsigned long word32;
#endif

// workaround for Solaris's boolean typename conflict
#ifdef SOLARIS
#define boolean __bite_me
#include <sys/types.h>
#undef boolean
#endif

typedef unsigned char boolean;

#if (!defined(FALSE) && !defined(TRUE))
enum {FALSE, TRUE};
#endif

#endif

