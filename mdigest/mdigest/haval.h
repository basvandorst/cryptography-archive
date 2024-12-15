
/*  -- derived from original havalapp.h and haval.h
 *  -- mgream: endian definitions not needed, they are computed run time
 *  -- mgream: changed prototypes from ansi (for compat)
 *
 *  havalapp.h:  specifies the following three constants needed to
 *               compile the HAVAL hashing library:
 *                     LITTLE_ENDIAN, PASS and FPTLEN
 *
 *  Descriptions:
 *
 *   LITTLE_ENDIAN  define this only if your machine is little-endian
 *                  (such as 80X86 family). 
 *
 *         Note:
 *            1. In general, HAVAL is faster on a little endian
 *               machine than on a big endian one.
 *
 *            2. The test program "havaltest.c" provides an option
 *               for testing the endianity of your machine.
 *
 *            3. The speed of HAVAL is even more remarkable on a
 *               machine that has a large number of internal registers.
 *
 *   PASS     define the number of passes        (3, 4, or 5)
 *   FPTLEN   define the length of a fingerprint (128, 160, 192, 224 or 256)
 */

/*#define LITTLE_ENDIAN*/

#ifndef PASS
#define PASS       3        /* 3, 4, or 5 */
#endif

#ifndef FPTLEN  
#define FPTLEN     128      /* 128, 160, 192, 224 or 256 */
#endif

/*
 *  haval.h:  specifies the interface to the HAVAL (V.1) hashing library.
 *
 *      HAVAL is a one-way hashing algorithm with the following
 *      collision-resistant property:
 *             It is computationally infeasible to find two or more
 *             messages that are hashed into the same fingerprint.
 *
 *  Reference:
 *       Y. Zheng, J. Pieprzyk and J. Seberry:
 *       ``HAVAL --- a one-way hashing algorithm with variable
 *       length of output'', Advances in Cryptology --- AUSCRYPT'92,
 *       Lecture Notes in Computer Science, Springer-Verlag, 1993.
 *
 *      This library provides routines to hash
 *        -  a string,
 *        -  a file,
 *        -  input from the standard input device,
 *        -  a 32-word block, and
 *        -  a string of specified length.
 *
 *  Author:     Yuliang Zheng
 *              Department of Computer Science
 *              University of Wollongong
 *              Wollongong, NSW 2522, Australia
 *              Email: yuliang@cs.uow.edu.au
 *              Voice: +61 42 21 4331 (office)
 *
 *  Date:       June 1993
 *
 *      Copyright (C) 1993 by C^3SR. All rights reserved. 
 *      This program may not be sold or used as inducement to
 *      buy a product without the written permission of C^3SR.
 */

typedef unsigned long int haval_word; /* a HAVAL word = 32 bits */

typedef struct {
  haval_word    count[2];                /* number of bits in a message */
  haval_word    fingerprint[8];          /* current state of fingerprint */    
  haval_word    block[32];               /* buffer for a 32-word block */ 
  unsigned char remainder[32*4];         /* unhashed chars (No.<128) */   
} haval_state;

void haval_start ();
void haval_hash ();
void haval_end ();

