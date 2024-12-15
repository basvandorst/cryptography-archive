/* This short program prints out a histogram for the letter occurence
** of the standard input.
** It will also find frequencies for numeric lists with the -n option.
**
** A contact chart can be made by using the -c option.
** A digram chart can be made using the -d option.
**
** Written by wart@ugcs.caltech.edu  (Mike Thomas)
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef TRUE
# define TRUE   1
#endif

#ifndef FALSE
# define FALSE  0
#endif

#define BLANK  ' '

typedef struct letter{
  struct letter **precede;
  int unique_prec;
  struct letter **postcede;
  int unique_post;
  char ct;
  int uniques;
  int count;
} LETTER;

int numflag;

int get_index(char letter);
int compare(LETTER *, LETTER *);
void sort(LETTER **, int);

int
main(int argc, char **argv){
  register int i, j;
  int digram[26][26], max_prec;
  int show_digrams = FALSE, show_contacts = FALSE, verbose = FALSE;
  LETTER hist[26];
  register char c, d;
  int alphlen = 26;

  numflag = FALSE;

  --argc, ++argv;
  while(argc){
    if(**argv != '-'){
      printf("Bad argument: %s\n", *argv);
      ++argv, --argc;
    }
    else{
      switch(*++*argv){
	case 'd':
	  show_digrams = TRUE;
	  ++argv, --argc;
	  break;
	case 'c':
	  show_contacts = TRUE;
	  ++argv, --argc;
	  break;
	case 'v':
	  verbose = TRUE;
	  ++argv, --argc;
	  break;
	case 'n':
	  numflag = TRUE;
	  alphlen = 10;
	  ++argv, --argc;
	  break;
      }
    }
  }

  /* Initialize the letter array
  */

  for(i = 0; i < alphlen; i++){
    hist[i].count = 0;
    if(numflag)
      hist[i].ct = i + '0';
    else
      hist[i].ct = i + 'a';
    hist[i].uniques = 0;
    hist[i].unique_prec = 0;
    hist[i].unique_post = 0;
    hist[i].precede = (LETTER **)malloc(sizeof(LETTER *)*alphlen);
    hist[i].postcede = (LETTER **)malloc(sizeof(LETTER *)*alphlen);
    for(j = 0; j < alphlen; j++){
      hist[i].precede[j] = (LETTER *)malloc(sizeof(LETTER));
      hist[i].postcede[j] = (LETTER *)malloc(sizeof(LETTER));
      if(numflag)
	hist[i].precede[j]->ct  = j + '0';
      else
	hist[i].precede[j]->ct  = j + 'a';
      hist[i].precede[j]->count = 0;
      hist[i].precede[j]->uniques = 0;
      if(numflag)
	hist[i].postcede[j]->ct = j + '0';
      else
	hist[i].postcede[j]->ct = j + 'a';
      hist[i].postcede[j]->count = 0;
      hist[i].postcede[j]->uniques = 0;
    }
  }

  d = BLANK;
  while( (c = getchar()) != EOF){
    if(verbose)
      putchar(c);
    if(isalnum(c)){
      if( (numflag && isdigit(c)) || (!numflag && isalpha(c)) ){
	if(numflag && isdigit(c))
	  c -= (int)'0';
	else if(!numflag && isalpha(c)){
	  c |= ' ';
	  c -= 'a';
	}
	hist[(int) c].count++;
	if(d != BLANK){
	  if(hist[(int)d].postcede[(int)c]->count == 0 && hist[(int)d].precede[(int)c]->count == 0){
	    hist[(int)d].uniques++;
	  }
	  if(hist[(int)c].postcede[(int)d]->count == 0 && hist[(int)c].precede[(int)d]->count == 0){
	    hist[(int)c].uniques++;
	  }
	  if(hist[(int)c].precede[(int)d]->count == 0)
	    hist[(int)c].unique_prec++;
	  hist[(int)c].precede[(int)d]->count++;
	  if(hist[(int)d].postcede[(int)c]->count == 0)
	    hist[(int)d].unique_post++;
	  hist[(int)d].postcede[(int)c]->count++;
	  digram[(int)c][(int)d]++;
	}
	d = c;
      }
    }
  }
  if(verbose)
    printf("\n\n");

  qsort(hist, alphlen, sizeof(LETTER), (void *)compare);
  for(i = 0; i < alphlen; i++){
    sort(hist[i].precede, alphlen);
    sort(hist[i].postcede, alphlen);
  }

  /* If we want a chart of digrams then show it
  */

  if(show_digrams){
    printf("\n  ");
    for(i = 0; i < alphlen; i++){ 
      if(hist[i].count)
	printf("%2c ", hist[i].ct);
    }
    putchar('\n');
    for(i = 0; i < alphlen; i++){
      if(hist[i].count){
	printf("%c ", hist[i].ct);
	for(j = 0; j < alphlen; j++){
	  if(hist[j].count){
	    if(digram[get_index(hist[i].ct)][get_index(hist[j].ct)])
	      printf("%2d ", digram[get_index(hist[i].ct)][get_index(hist[j].ct)]);
	    else if(i == j){
	      printf(" . ");
	    }
	    else
	      printf("   ");
	  }
	}
	putchar('\n');
      }
    }
  }

  if(show_contacts || verbose){
    max_prec = 0;
    for(i = 0; i < alphlen; i++){
      if(hist[i].unique_prec > max_prec)
	max_prec = hist[i].unique_prec;
    }
    for(i = 0; i < alphlen; i++)
      printf("%2d ", hist[i].count);
    putchar('\n');
    for(i = 0; i < alphlen; i++)
      printf("%2d ", hist[i].uniques);
    putchar('\n');
    for(i = 0; i < alphlen; i++)
      printf("%2c ", toupper(hist[i].ct));
    putchar('\n');
    putchar('\n');

    for(i = 0; i < alphlen && hist[i].count != 0; i++){
	/* show the least common preceding first, and the most common 
	** postceding first.
	*/
	for(j = alphlen-1; j >= 0; j--){
	  if(hist[i].precede[j]->count != 0){
	    printf(" %c", hist[i].precede[j]->ct);
	    if(hist[i].precede[j]->count > 1)
	      printf("%d", hist[i].precede[j]->count);
	    else
	      putchar(' ');
	  }
	  else{
	    if(j < max_prec+1)
	      printf("   ");
	  }
	}
	printf("  %c ", toupper(hist[i].ct));
	for(j = 0; j < alphlen; j++){
	  if(hist[i].postcede[j]->count != 0){
	    printf(" %c", hist[i].postcede[j]->ct);
	    if(hist[i].postcede[j]->count > 1)
	      printf("%d", hist[i].postcede[j]->count);
	    else
	      printf(" ");
	  }
	}
	putchar('\n');
      }
  }

  else{
    putchar('\n');
    for(i = 0; i < alphlen; i++)
      if(hist[i].count)
	printf("%2d ", hist[i].count);
    putchar('\n');
    for(i = 0; i < alphlen; i++)
      if(hist[i].count)
	printf("%2c ", hist[i].ct);
    putchar('\n');
  }

  if(verbose && numflag)
    printf("\n\n\n 1  2  3  4  5  6  7  8  9  0\n");
  if(verbose && !numflag)
    printf("\n\n\n A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z\n");
  
  (void) exit(0);
}

int get_index(char letter){
  if(numflag)
    letter -= '0';
  else
    letter -= 'a';
  return letter;
}

int
compare(LETTER *a, LETTER *b){
  if(a->count > b->count)
    return -1;
  else if(a->count < b->count)
    return 1;
  else if(a->uniques > b->uniques)
    return -1;
  else if(a->uniques < b->uniques)
    return 1;
  else
    return 0;
}

void
sort(LETTER **list, int alphlen){       /* A simple bubble sort */
  int list_sorted=FALSE, test, i, j;
  LETTER *temp_list;

  for(i = 0; i < alphlen /*&& list_sorted == FALSE*/; i++){

    list_sorted = TRUE;
    j = 0;
    while(j < alphlen){
      test = compare(list[i], list[j]);

      if(test < 0){
	
	temp_list = list[i];
	list[i] = list[j];
	list[j] = temp_list;

	list_sorted = FALSE;
      }
      else{
	j++;
      }
    }
  }
}
