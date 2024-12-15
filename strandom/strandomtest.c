
/*
 *  strandomtest.c:  specifies a test program for the pseudo-random
 *                number generator "strandom".
 *
 *  Arguments for the test program:
 *
 *      -c        - test certification data
 *      -s        - test speed
 *      otherwise - show help menu
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

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "havalapp.h"          /* include this only when PASS is needed */
#include "strandom.h"

#define NUMBER_OF_BLOCKS 2000  /* number of test blocks */
#define BLOCK_SIZE       600   /* number of 32-bit words in a block */
#define CERT_LEN         16    /* number of words in a certification block */

static void strandom_speed (void);                /* test the speed of strandom */
static void strandom_cert (void);                 /* test data set */
static void strandom_print (unsigned int *, int); /* print a sequence */

int main (argc, argv)
int  argc;
char *argv[];
{
  int i;

  if (argc > 1) {
    for (i = 1; i < argc; i++) {
      if (strcmp (argv[i], "-c") == 0) {         /* print test data */
        strandom_cert ();
      } else if (strcmp (argv[i], "-s") == 0) {  /* test speed */
        strandom_speed ();
      } else {                                   /* show help menu */
        printf (" ? or -?    show help menu\n");
        printf (" -c         test certification data\n");
        printf (" -s         test speed\n");
      }
    }
  }
  return (0);
}

/* test the speed of strandom */
static void strandom_speed (void)
{
  time_t        start_time, end_time;
  double        elapsed_time;
  unsigned int  i;
  unsigned int  seed[] = {0x0, 0x76543210, 0xFEDCBA98, 0xFFFFFFFF};
  unsigned int  rand_num[BLOCK_SIZE];
			   

  printf ("Test the speed of strandom (PASS = %d).\n", PASS);
  printf ("Generating %d %d-word blocks ...\n", NUMBER_OF_BLOCKS, BLOCK_SIZE);

  time (&start_time);                            /* get start time */

  init_strandom (seed, 4);                       /* initialization */
  for (i = 0; i < NUMBER_OF_BLOCKS; i++) {
    strandom (rand_num, BLOCK_SIZE);             /* generate numbers */
  }

  time (&end_time);                              /* get end time */

  elapsed_time = difftime(end_time, start_time); /* get elapsed time */

  if (elapsed_time > 0.0) {                      /* print out speed */
    printf ("Elapsed Time = %3.1f seconds\n", elapsed_time);
    printf ("       Speed = %4.2f MBPS (megabits/second)\n",
    (NUMBER_OF_BLOCKS * BLOCK_SIZE * 32)/(1.0E6 * elapsed_time));
  } else {
    printf ("not enough blocks !\n");
  }
}

/* print a set of certification data.  */
static void strandom_cert (void)
{
  unsigned int  seed[32];
  unsigned int  rand_num[CERT_LEN];

  printf ("\n");
  printf ("\"strandom\" certification data (PASS=%d):", PASS);
  printf ("\n\n");

  seed[0] = 0x0;                               /* one word case */
  init_strandom(seed, 1);
  strandom(rand_num, CERT_LEN);
  printf ("strandom(%08X, %d) =", seed[0], CERT_LEN);
  printf ("\n");
  strandom_print (rand_num, CERT_LEN);
  printf ("\n");

  seed[0] = 0x76543210;                        /* two word case */
  seed[1] = 0xFEDCBA98;
  init_strandom(seed, 2);
  strandom(rand_num, CERT_LEN);
  printf ("strandom(%08X %08X, %d) =", seed[0], seed[1], CERT_LEN);
  printf ("\n");
  strandom_print (rand_num, CERT_LEN);
  printf ("\n");

  seed[0] = 0x76543210;                        /* four word case */
  seed[1] = 0xFEDCBA98;
  seed[2] = 0x89ABCDEF;
  seed[3] = 0x01234567;
  init_strandom(seed, 4);
  strandom(rand_num, CERT_LEN);
  printf ("strandom(%08X %08X %08X %08X, %d) =",
            seed[0], seed[1], seed[2], seed[3], CERT_LEN);
  printf ("\n");
  strandom_print (rand_num, CERT_LEN);
  printf ("\n");
}

/* print numbers in hexadecimal */
static void strandom_print (unsigned int *num, int len)
{
  int i;

  for (i = 0; i < len; i++) {
    printf (" %08X", num[i]);
    if ((i+1)%8 == 0) printf ("\n");
  }
}



