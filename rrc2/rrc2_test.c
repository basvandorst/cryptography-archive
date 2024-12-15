/****************************************************************************
*                                                                            *
*              Very Simple Test Program for my Implementation of             *
*                      RRC.2 - Ron Rivest Cipher Nr. 2                       *
*                                                                            *
*       Implementation done on 15.02.96 by Daniel Vogelheim.                 *
*                                                                            *
*       This document is placed into the public domain.                      *
*       I'd appriciate if my name was mentioned in derivative works.         *
*       (I'd be extremely suprised if anybody wanted to derive anything      *
*       from this program, though.)                                          *
*                                                                            *
 ****************************************************************************/

#include "rrc2.h"
#include <stdio.h>

int main(void)
{
	printf("Testing RRC.2 specification conformance ...");
	rrc2_test();
	printf(" passed.\n");
	return 0;
}

/* FINI: rrc2_test.c */
