#ifndef CONFIG_H
#define CONFIG_H

// define this if running on a little-endian CPU

#define LITTLE_ENDIAN

// define this if you want the library to throw exceptions when things go wrong

#define THROW_EXCEPTIONS

// define this if your compiler doesn't support namespaces

#ifdef __GNUC__
#define NO_NAMESPACE
#endif

// define this if your compiler only has the old version of iostream library

#ifdef __GNUC__
#define OLD_IOSTREAM
#endif

// switch between different secure memory allocation mechnisms, this is the only
// one available right now

#define SECALLOC_DEFAULT

// Define this to 0 if your system doesn't differentiate between
// text mode and binary mode files.

// #define BINARY_MODE 0
#define BINARY_MODE ios::binary
// #define BINARY_MODE ios::bin

// Define this to 0 if your compiler doesn't have ios::nocreate.
// This is needed because some compilers automaticly create a new file
// when you try to open one for reading if it doesn't already exist.

#define FILE_NO_CREATE 0
// #define FILE_NO_CREATE ios::nocreate

// define this if you have a copy of RSAREF and want to compile the RSAREF
// class wrappers

// #define USE_RSAREF

// define this if you want to be able to initialize RSAREF structures with bignums

// #define USE_BIGNUM_WITH_RSAREF

#define GZIP_OS_CODE 0

// how much memory to use for deflation (compression)
// note 16 bit compilers don't work with BIG_MEM (pointer wraps around segment boundary)

// #define SMALL_MEM
#define MEDIUM_MEM
// #define BIG_MEM

// Define the following to use the updated standard SHA-1 instead of SHA

#define NEW_SHA

// Try this if your CPU has 256K internal cache or a slow multiply instruction
// and you want a (possibly) faster IDEA implementation using log tables

// #define IDEA_LARGECACHE

// Try this if you have a large cache or your CPU is slow manipulating 
// individual bytes.  

// #define DIAMOND_USE_PERMTABLE

// Define this if, for the linear congruential RNG, you want to use
// the original constants as specified in S.K. Park and K.W. Miller's
// CACM paper.

// #define LCRNG_ORIGINAL_NUMBERS

// Make sure these typedefs are correct for your computer

typedef unsigned char byte;
typedef unsigned short word16;
#if defined(__alpha) && !defined(_MSC_VER)
typedef unsigned int word32;
#else
typedef unsigned long word32;
#endif

#ifdef __GNUC__
#define WORD64_AVAILABLE
typedef unsigned long long word64;
#define W64LIT(x) x##LL
#endif

#ifdef _MSC_VER
#define WORD64_AVAILABLE
typedef unsigned __int64 word64;
#define W64LIT(x) x##i64
#endif

// word should have the same size as your CPU registers
// dword should be twice as big as word

#if defined(__GNUC__) && !defined(__alpha)
typedef unsigned long word;
typedef unsigned long long dword;
#elif defined(_MSC_VER)
typedef unsigned __int32 word;
typedef unsigned __int64 dword;
#else
typedef unsigned int word;
typedef unsigned long dword;
#endif

const unsigned int WORD_SIZE = sizeof(word);
const unsigned int WORD_BITS = WORD_SIZE * 8;

#define LOW_WORD(x) (word)(x)

#ifdef LITTLE_ENDIAN
#define HIGH_WORD(x) (*(((word *)&(x))+1))
#else
#define HIGH_WORD(x) (*((word *)&(x)))
#endif

// if the above HIGH_WORD macro doesn't work (if you are not sure, compile it
// and run the validation tests), try this:
// #define HIGH_WORD(x) (word)((x)>>WORD_BITS)

#ifdef _MSC_VER
// 4250: dominance
// 4660: explicitly instantiating a class that's already implicitly instantiated
#pragma warning(disable: 4250 4660)
#endif

#ifdef NO_NAMESPACE
#define std
#define USING_NAMESPACE(x)
#define NAMESPACE_BEGIN(x)
#define NAMESPACE_END
#define ANONYMOUS_NAMESPACE_BEGIN
#else
#define USING_NAMESPACE(x) using namespace x;
#define NAMESPACE_BEGIN(x) namespace x {
#define ANONYMOUS_NAMESPACE_BEGIN namespace {
#define NAMESPACE_END }
#endif

#ifdef _MSC_VER
// VC50 workaround
#define STDMIN std::_cpp_min
#define STDMAX std::_cpp_max
#else
#define STDMIN std::min
#define STDMAX std::max
#endif

#endif
