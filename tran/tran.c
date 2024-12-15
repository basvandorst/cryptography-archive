/* modified from the original posting, by Carl Ellison, to remove */
/* the password and therefore keep this from being an encryption program */
/* The original code was written by setzer@math.ncsu.edu (William Setzer) */

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
  long savlen, pos, key;
  char tmp;

  infp  = (argc > 1) ? my_fopen(argv[1], "r") : stdin;
  outfp = (argc > 2) ? my_fopen(argv[2], "w") : stdout;

  len = fread(buf, 1, BLOCKSIZE, infp);
  key = lstr2int(buf,len);
  set_rnd_seed(key);

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

/* Make an integer out of a string.  Do a poor job of it.
 * Note that since this function is called on a block of transposed
 * text and used to construct an "rng key", it mustn't be sensitive
 * to the position of characters  in `str'.
 */
long lstr2int(str,len)
char *str;
long len;
{
  long sum = 0;
  long i ;

  for ( i = 0 ; i < len ; i++ )
    sum += *str++;

  return sum;
}

