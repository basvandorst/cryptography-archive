/* types.h - some common typedefs
 *	Copyright (C) 1998 Free Software Foundation, Inc.
 *
 * This file is part of GNUPG.
 *
 * GNUPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNUPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef G10_TYPES_H
#define G10_TYPES_H


/* The AC_CHECK_SIZEOF() in configure fails for some machines.
 * we provide some fallback values here */
#if !SIZEOF_UNSIGNED_SHORT
  #undef SIZEOF_UNSIGNED_SHORT
  #define SIZEOF_UNSIGNED_SHORT 2
#endif
#if !SIZEOF_UNSIGNED_INT
  #undef SIZEOF_UNSIGNED_INT
  #define SIZEOF_UNSIGNED_INT 4
#endif
#if !SIZEOF_UNSIGNED_LONG
  #undef SIZEOF_UNSIGNED_LONG
  #define SIZEOF_UNSIGNED_LONG 4
#endif


#include <sys/types.h>


#ifndef HAVE_BYTE_TYPEDEF
  #undef byte	    /* maybe there is a macro with this name */
  typedef unsigned char byte;
  #define HAVE_BYTE_TYPEDEF
#endif

#ifndef HAVE_USHORT_TYPEDEF
  #undef ushort     /* maybe there is a macro with this name */
  typedef unsigned short ushort;
  #define HAVE_USHORT_TYPEDEF
#endif

#ifndef HAVE_ULONG_TYPEDEF
  #undef ulong	    /* maybe there is a macro with this name */
  typedef unsigned long ulong;
  #define HAVE_ULONG_TYPEDEF
#endif

#ifndef HAVE_U16_TYPEDEF
  #undef u16	    /* maybe there is a macro with this name */
  #if SIZEOF_UNSIGNED_INT == 2
    typedef unsigned int   u16;
  #elif SIZEOF_UNSIGNED_SHORT == 2
    typedef unsigned short u16;
  #else
    #error no typedef for u16
  #endif
  #define HAVE_U16_TYPEDEF
#endif

#ifndef HAVE_U32_TYPEDEF
  #undef u32	    /* maybe there is a macro with this name */
  #if SIZEOF_UNSIGNED_INT == 4
    typedef unsigned int u32;
  #elif SIZEOF_UNSIGNED_LONG == 4
    typedef unsigned long u32;
  #else
    #error no typedef for u32
  #endif
  #define HAVE_U32_TYPEDEF
#endif

#ifndef HAVE_U64_TYPEDEF
  #undef u64	    /* maybe there is a macro with this name */
  #if SIZEOF_UNSIGNED_INT == 8
    typedef unsigned int u64;
    #define HAVE_U64_TYPEDEF
  #elif SIZEOF_UNSIGNED_LONG == 8
    typedef unsigned long u64;
    #define HAVE_U64_TYPEDEF
  #elif __GNUC__ >= 2 || defined(__SUNPRO_C)
    typedef unsigned long long u64;
    #define HAVE_U64_TYPEDEF
  #endif
#endif

typedef union {
    int a;
    short b;
    char c[1];
    long d;
  #ifdef HAVE_U64_TYPEDEF
    u64 e;
  #endif
    float f;
    double g;
} PROPERLY_ALIGNED_TYPE;

typedef struct string_list {
    struct string_list *next;
    unsigned int flags;
    char d[1];
} *STRLIST;


#endif /*G10_TYPES_H*/
