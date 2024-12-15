/* modified from the original posting, by Carl Ellison, to remove */
/* the password and therefore keep this from being an encryption program */
/* The original code was written by setzer@math.ncsu.edu (William Setzer) */
/* Modified to init the PRNG from a histogram of the input bytes. */

#include <stdio.h>

extern void set_rnd_seed();
extern long rnd();

#define BLOCKSIZE 8192

long  perm[BLOCKSIZE];
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

  len = fread(buf, 1, BLOCKSIZE, infp);

  for (i=0;i<256;i++) key[i] = 0 ; /* init the histogram array */
  for (i=0;i<len;i++) key[ buf[i] ]++ ;	/* gather the histogram */
  set_rnd_seed(key);		/* init the ranno generator with the hist */

  do {
    savlen = len;

    for (i = 0; i < len; i++)
      perm[i] = i;
    
#define swap(A,B)  tmp = A; A = B; B = tmp;

    while (len > 1)
      {
	pos = 1 + rnd() % (len - 1);
	swap( buf[perm[0]], buf[perm[pos]] );

	perm[0]   = perm[(pos == len - 2) ? len - 1 : len - 2];
	perm[pos] = perm[len - 1];
	len -= 2;
      }
    fwrite(buf, 1, savlen, outfp);
  } while (len = fread(buf, 1, BLOCKSIZE, infp));
}
