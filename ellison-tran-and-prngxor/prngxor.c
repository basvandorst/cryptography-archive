/* prngxor.c  */
/* modified from tran.c, by Carl Ellison, 21 Aug 93 */

/* This program uses a fixed key and is therefore useless as an */
/* encryption program.  It is offered as an example of how such a */
/* program might be written. */

/* prngxor uses XOR to mask the bytes of the input file. */
/*  prngxor | prngxor is a no-op  */
/* The original tran.c was written by setzer@math.ncsu.edu (William Setzer) */

#include <stdio.h>

extern void set_rnd_seed();
extern long rnd();

#define BLOCKSIZE 1024

char buf[BLOCKSIZE];

FILE *my_fopen(file, type)
char *file, *type;
{
  FILE *fp;

  if (fp = fopen(file, type))
    return fp;
  (void) fprintf(stderr, "Can't open '%s'\n", file);
  exit(1);
}

main(argc, argv)
long argc;
char **argv;
{
  register long i, len;
  register FILE *infp, *outfp;
  long savlen, pos ;
  long key[256] ;
  char tmp;

  infp  = (argc > 1) ? my_fopen(argv[1], "r") : stdin;
  outfp = (argc > 2) ? my_fopen(argv[2], "w") : stdout;

  for (i=0;i<256;i++) key[i] = i ; /* init the ranno seed key */
  set_rnd_seed(key);		/* init the ranno generator with key */

  while (len = fread(buf, 1, BLOCKSIZE, infp)) 
    {
      for ( i = 0; i < len; i++) buf[i] ^= (rnd() & 0xff) ;
      fwrite(buf, 1, len, outfp);
    }
}
