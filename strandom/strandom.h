/*
 *  strandom.h:	interface to "strandom", a library for cryptographically
 *		strong pseudo-random number generation.
 *		the generator is based on the basic hashing function 
 *		employed by HAVAL.
 *
 *		to be used in conjunction with
 *                   haval.h
 *                   haval.c
 *                   havalapp.h (for defining PASS)
 *
 *  Descriptions:
 *      -  init_strandom: initialization with a seed of specified length
 *      -  strandom:      generate random sequence of specified length
 *
 *  Note:   
 *        Define PASS (=3, 4 or 5) in "havalapp.h"
 *
 *  Author:     Yuliang Zheng
 *              Department of Computer Science
 *              University of Wollongong
 *              Wollongong, NSW 2522, Australia
 *              Email: yuliang@cs.uow.edu.au
 *              Voice: +61 42 21 4331 (office)
 *
 *  Date:       July 1993
 *
 *      Copyright (C) 1993 by C^3SR. All rights reserved. 
 *      This program may not be sold or used as inducement to
 *      buy a product without the written permission of C^3SR.
 */

void init_strandom (unsigned int *, int); /* initialization */
void strandom (unsigned int *, int);      /* generate random numbers */

