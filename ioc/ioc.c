/* This program will calculate the index of coincidence of a cipher.
** It reads from standard input.
**
** Written by wart@ugcs.caltech.edu  (Mike Thomas)
**
*/

#include <stdio.h>
#include <stdlib.h>
#define TRUE     1
#define FALSE    0

main(int argc, char **argv){
  double *ic, tic = 0.;
  char c;
  int period = 1, i, j, count = 0;
  int **hist;
  int vflag = FALSE;

  --argc, ++argv;
  while(argc){
  if(**argv != '-'){
    (void) printf("Bad option: %s\n", *argv);
    --argc, ++argv;
  }
  else switch(*++*argv){
    case 'p':
      --argc, ++argv;
      if(sscanf(*argv, "%d", &period) != 1){
	(void) (void) printf("Bad period.\n");
	period = 0;
      }
      --argc, ++argv;
      break;
    case 'v':
      --argc, ++argv;
      vflag = TRUE;
      break;
    }
  }

  if(period < 1){
    (void) printf("Bad period.  Exitting.\n");
    exit(0);
  }

  hist = (int **)malloc(sizeof(int *)*period);
  for(i = 0; i < period; i++)
    hist[i] = (int *)malloc(sizeof(int)*26);
  ic = (double *)malloc(sizeof(double)*period);

  for(j = 0; j < period; j++){
    for(i = 0; i < 26; i++)
      hist[j][i] = 0;
    ic[j] = 0.;
  }

  count = 0;
  while((c = getchar()) != EOF){
    if(isalpha(c)){
      c |= ' ';
      hist[count%period][c - 'a']++;
      count++;
    }
  }

  /* If we are in verbose mode then print the histograms.
  */
  if(vflag){
    (void) printf("Histogram%c:\n", (period > 1)?'s':'\0');
    for(j = 0; j < period; j++){
      if(period > 1)
	(void) printf("%d:", j%10);

      for(i = 0; i < 26; i++)
	(void) printf("%2d ", hist[j][i]);
      (void) putchar('\n');
    }
    if(period > 1)
      printf("  ");

    for(i = 0; i < 26; i++)
      (void) printf("%2c ", i+'a');
    (void) putchar('\n');
    (void) putchar('\n');
  }

  /* Now we have our histogram.  Calculate the IC.
  */

  for(j = 0; j < period; j++){
    count = 0;
    for(i = 0; i < 26; i++){
      ic[j] += (double) hist[j][i] * ((double) hist[j][i] - 1);
      count += hist[j][i];
    }
    ic[j] /= (double) count * (count - 1);
    tic += ic[j];
  }
  tic /= period;

  if(period == 1)
    (void) printf("I.C. for this cipher is %g\n", ic[0]);
  else{ 
    for(j = 0; j < period; j++)
      (void) printf("I.C. for column %d is %g\n", j, ic[j]);
    printf("Mean I.C. for this cipher: %g\n", tic);
  }
}
