/**********************************************************************
tran.c -- The original code was written by setzer@math.ncsu.edu (William
Setzer) and has been modified by Carl Ellison.

This software is not claimed to offer cryptographic security.  Rather, it
is offered as an illustration of a general cryptographic technique which
has been known and analyzed for years -- transposition of an input source.
See H. F. Gaines "Cryptanalysis", for example, for a discussion of
transposition ciphers.

The particular transposition used here is a self-inverse.  In any block (up
to BLOCKSIZE in length), pairs of input bytes are swapped.  That makes this
one less general than the ones discussed in the literature.

Choice of pairs is made by a PRNG (Pseudo-random number generator).  This
is slightly different from the usage found in textbooks.  Typically, the
transposition key used in those texts is a moderate length word.  The key
here is generated by the PRNG and applies to the whole block.  This then
becomes a columnar transposition with one row and with up to BLOCKSIZE
columns.  Each block of the transposition uses a new key, generated by the
PRNG.  Otherwise, this is the same as the systems described in standard
texts.  The difference is due primarily to the computer.  Prior mechanisms
had to use smaller sizes in order to keep the paperwork down for the human
cipher clerk.  The use of a self-inverse was not necessary but is a
convenience so that the user does not need to specify encryption or
decryption when invoking tran.  However, it should be noted that this
feature reduces security of the algorithm.

The use of the histogram of the first block (possible the entire file, if
it is less than BLOCKSIZE long) to augment the typed password in forming
the PRNG key is intended to prevent the kind of cryptanalysis detailed in
Gaines: solution in depth.  That is, a common attack on transposition
ciphers is to take two messages enciphered with the same key and lay them
side-by-side, anagramming them in sets.  [For more detail, please see
Gaines.]  By modifying the key with information from the file being
transposed, using a function which is invariant under transposition, this
program attempts to make it unlikely that two messages would be found which
have the same key, even though there might easily be two messages
transposed using the same password.

Usage:  as written, tran takes 0 to 3 arguments:

      tran <password-string>  <input file>  <output file>

If either of the files is not specified, stdin or stdout is used
respectively.  If the password string is not specified, the histogram of
the first block becomes the key.

There was posted to sci.crypt a version of tran which accepts no password
and is therefore not capable of hiding information.  That program was
written to illustrate cryptographic techniques but to remain free of U.S.
Government export limitations.  If no password is given to this program, it
should interoperate with the publicly posted version.

**********************************************************************/

#include <stdio.h>

extern void set_rnd_seed();	/* init the PRNG from key[] */
extern unsigned long rnd();	/* get one random integer */

#define BLOCKSIZE (8192)	/* use something larger than 256 */

long perm[BLOCKSIZE];		/* array of byte indices yet to swap */
char buf[BLOCKSIZE];		/* array of bytes in one block */

FILE *my_fopen(file, type)	/* open a file for R or W and exit if fail */
char *file, *type;		/* file: name; type: r or w */
{
  FILE *fp;			/* resulting file */

  if (fp = fopen(file, type))	/* do the open */
    return fp;			/* return if success */
  (void) fprintf(stderr, "Can't open '%s'\n", file); /* print error O'E */
  exit(1);			/* and exit */
} /* end of my_fopen */

main(argc, argv)		/* get params and do the transposition */
long argc;
char **argv;
{
  register long i, len;
  register FILE *infp, *outfp;
  char *password, *pwp ;
  long savlen, pos ;
  long key[256] ;
  char tmp;


  password   = (argc > 1) ? argv[1] : 0 ; /* typed password */
  infp  = (argc > 2) ? my_fopen(argv[2], "r") : stdin ;	/* input file name */
  outfp = (argc > 3) ? my_fopen(argv[3], "w") : stdout ; /* output file name */

  len = fread(buf, 1, BLOCKSIZE, infp);	/* get first block and its length */

  pwp = password ;		/* point to the typed password */
  if (password == 0)		/* if there was none, ignore it */
    for (i=0;i<256;i++) key[i] = 0 ; /* init the histogram array */
  else				/* there was a password typed */
    for (i=0;i<256;i++)		/* init the array with the password */
      { char nxt = *(pwp++) ;	/* get the next password char and advance */
	if (nxt == 0)		/* if off the end of the typed password, */
	  pwp = password ;	/*  ... cycle to the start */
	key[i] = nxt ;		/* use the current char to init this loc */
      } /* for -- using the command line password */

  for (i=0;i<len;i++) key[ buf[i] ]++ ;	/* gather the histogram */

  set_rnd_seed(key);		/* init the ranno generator with the hist */

  do {				/* for each block of the input file */
    savlen = len;		/* remember how long it was */

    for (i = 0; i < len; i++)	/* initialize a permutation array */
      perm[i] = i;		/* of the same length as the block */
    
#define swap(A,B)  tmp = A; A = B; B = tmp;

    while (len > 1)		/* swap byte pairs as long as there are any */
      {
	pos = 1 + rnd() % (len - 1); /* pick a non-0 position */
	swap( buf[perm[0]], buf[perm[pos]] ); /* swap two bytes */
				/* replace the two positions swapped */
				/* with ones from the end */
	perm[0]   = perm[(pos == len - 2) ? len - 1 : len - 2];
	perm[pos] = perm[len - 1];
	len -= 2;		/* and reduce the length to be done */
      }
    fwrite(buf, 1, savlen, outfp); /* write the transposed block */
  } while (len = fread(buf, 1, BLOCKSIZE, infp)); /* and read the next one */
} /* end of main() */
