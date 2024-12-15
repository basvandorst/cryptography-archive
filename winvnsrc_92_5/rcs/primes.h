head	1.1;
access;
symbols;
locks; strict;
comment	@ * @;


1.1
date	94.08.23.22.56.19;	author DUMOULIN;	state Exp;
branches;
next	;


desc
@list of primes for the hash table mechanism.
@


1.1
log
@Initial revision
@
text
@/* $Id: $ */

/*
 * This is a small set of primes taken from a 'primes.h' written
 * by Landon Curt Noll.
 */

unsigned long prime[] = {
2203,4909,7757,10771,13879,17053,20333,23593,26903,30341,33679,
37139,40709,44119,47717,51287,54829,58313,61991,65537
};
@
