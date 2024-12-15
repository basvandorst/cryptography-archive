/* This program, taken from PHOENIX of the American Cryptogram Association,
** has been rewritten into C.  The BASIC version was printed in the
** March/April 1994 issue of The Cryptogram.
**
** It's purpose is to apply a Kasiski analysis on a cipher to find the 
** period.
*/

#include <stdio.h>
#include <string.h>
#define TRUE     1
#define FALSE    0

main(int argc, char **argv){
  char ct[1024], sg1[1024], sg2[1024];
  char c;
  int i, j, k, m, ks[20];
  int ctlen, verbose=FALSE;

  --argc, ++argv;
  while(argc){
    if(**argv != '-'){
      printf("Bad argument: %s\n", *argv);
      ++argv, --argc;
    }
    else switch(*++*argv){
      case 'v':
	++argv, --argc;
	verbose = TRUE;
	break;
    }
  }

  /* Initialize the Kasiski array
  */

  for(i = 0; i < 20; i++)
    ks[i] = 0;

  /* Read in the cryptogram
  */

  i = 0;
  while( (c = getchar()) != EOF){
    if(c != ' ' && c != '\t' && c != '\n')
      ct[i++] = c;
  }
  ct[i] = '\0';
  ctlen = i;

  /* Look for repeats of strings up to 5 characters long
  */

  for(i = 8; i >= 1; i--){
    if(verbose && i > 1)
      printf("\nLength = %d\n", i);

    /* Get a string of length i
    */

    for(j = 0; j <= ctlen - i+1; j++){
      strncpy(sg1, ct+j, i);
      sg1[i] = '\0';

      /* Search for a repeat of the string just obtained.
      */
      for(k = j+1; k <= ctlen - i; k++){

	if(strncmp(sg1, ct+k, i) == 0){
          if(i > 1)
	    if(verbose)
	      printf("%s occurred at positions %d and %d\n", sg1, j, k);

	  for(m = 3; m <= 15; m++){
	    if( (k - j)%m == 0)
	      ks[m] += 1;
	  }
	}
      }
    }
  }

  printf("\nKasiski Factors\n");

  for(i = 3; i <= 15; i++)
    printf("%5d ", i);

  printf("\n");
  for(i = 3; i <= 15; i++)
    printf("%5d ", ks[i]);

  printf("\n");
  for(i = 3; i <= 15; i++)
    printf("%5d ", ks[i] * i);

  printf("\n");
}
