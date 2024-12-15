#ifdef WIN32
#define UINT4 unsigned long
#define PLATFORM_ID 1
#endif

#ifdef LINUX
#define UINT4 unsigned long
#define PLATFORM_ID 2
#endif

#ifdef MKLINUX
#define UINT4 unsigned long
#define PLATFORM_ID 14
#define SWAP_BYTES
#endif

#ifdef ALPHA
#define UINT4 unsigned int
#define BIT64
#define PLATFORM_ID 3
#endif

#ifdef HPUX
#define UINT4 unsigned long
#define NO_USLEEP
#define SWAP_BYTES
#define PLATFORM_ID 13
#endif

#ifdef AIX
#define UINT4 unsigned long
#define SWAP_BYTES
#define PLATFORM_ID 15
#endif

#if defined(SUN4M) || defined(SUN4C)
#define UINT4 unsigned long
#undef LITTLE_ENDIAN
#define PLATFORM_ID 4
#endif

#ifdef SUN4U
#define UINT4 unsigned int
#undef LITTLE_ENDIAN
#define SWAP_BYTES
#define PLATFORM_ID 5
#endif

#ifdef IRIX
#define UINT4 unsigned int
#define SWAP_BYTES
#undef LITTLE_ENDIAN
#define PLATFORM_ID 6
#define NO_USLEEP
#define NO_TIMEB
#endif

#ifdef NEXTSTEP
xxx umm
#define PLATFORM_ID 7
#endif

#ifdef ULTRIX
#define UINT4 unsigned long
#undef LITTLE_ENDIAN
#define PLATFORM_ID 8
#define NO_USLEEP
#endif

#ifdef ALPHAVMS
#define UINT4 unsigned int
#undef LITTLE_ENDIAN
#define PLATFORM_ID 9
#endif

#ifdef FREEBSD
#define UINT4 unsigned long
#define PLATFORM_ID 10
#endif

#ifdef ALPHALINUX
#define UINT4 unsigned int
#undef LITTLE_ENDIAN
#define PLATFORM_ID 11
#endif

#ifdef BSDI
#define UINT4 unsigned long
#define LITTLE_ENDIAN
#define PLATFORM_ID 12
#define USE_ASM
#endif

#ifndef PLATFORM_ID
Hey You have to define a platform
#endif
