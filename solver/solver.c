/* This program will solve cryptograms using the database of words
** in DICT_DIR.
**
** NOTE:  The dictionary must be formatted in a specific way.  The words
** be separated into different files based on length.  One letter words
** would go in "len01", 11-letter words in "len11", etc.  Each line in
** the file should contain a word.  This is so that the program doesn't 
** have to read in the entire (potentially huge) dictionary; only the word 
** lengths appearing in the cipher need be read in.
**
** wart@ugcs.caltech.edu
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "solver.h"

/* Global variables */

FILE *ofptr;
WORD wordlist[MAXWORDS];
char keylist[MAXKEYS][27];
char cipher[MAXLINES][LINELENGTH];
int lines_in_cipher = 0, lhist[26], num_ulets=0;
int num_words, num_keys=0, vflag=FALSE, Dflag = FALSE, rflag = FALSE;
int threshhold=0, vlevel=0;

main(int argc, char **argv){
  FILE *fptr, *dfptr;
  char filename[MAXLENGTH], temp_word[MAXLENGTH];
  int i, j;
  int aflag, pflag, dflag, qflag, sflag;
  int pword_index = 0;
  aflag = pflag = dflag = qflag = sflag = FALSE;

  threshhold = DEF_THRESH;

  /* Make sure that we have a dictionary.
  */
  strcpy(filename, get_dfname("test"));
  dfptr = fopen(filename, "r");
  if(dfptr == NULL){
    printf("I'm sorry, but I can't find the dictionary.  Please\n");
    printf("check that it is on line and in the proper location:\n");
    printf("%s\n", DICT_DIR);
    exit(0);
  }
    
  strcpy(filename, "\0");

  /* Deal with the arguments.
  */

  ++argv, --argc;
  while(argc){
    if(**argv != '-'){
      printf("Unknown option %s\n", *argv);
      exit(1);
    }
    else{
      switch(*++*argv){
	/* Set the threshhold for the number of "bad" words allowed.
	*/
	case 't':
	  ++argv, --argc;
	  strcpy(temp_word, *argv);
	  --argc, ++argv;
	  sscanf(temp_word, "%d", &threshhold);
	  break;
	/* What file is the cipher being held captive in?
	*/
	case 'f':
	  ++argv, --argc;
	  strcpy(filename, *argv);
	  ++argv, --argc;
	  break;
	/* Show the depth of the recursion during the solving.  This has
	** only been handy for determing effective sorting criteria.
	*/
	case 'D':
	  ++argv, --argc;
	  Dflag = TRUE;
	  break;
	/* Don't solve, just check if the word is in the dictionary.
	*/
	case 'q':
	  ++argv, --argc;
	  query_word(*argv);
	  ++argv, --argc;
	  qflag = TRUE;
	  break;
	/* Add the word to the dictionary
	*/
	case 'a':
	  ++argv, --argc;
	  add_word(*argv);
	  ++argv, --argc;
	  aflag = TRUE;
	  break;
	/* Show statistics of the cipher.  word order, nubmer of
	** dictionary matches, etc.
	*/
	case 's':
	  ++argv, --argc;
	  sflag = TRUE;
	  break;
	/* Verbose flag.  This way you can see how quickly it is solving
	** the cipher.
	*/
	case 'v':
	  ++argv, --argc;
	  strcpy(temp_word, *argv);
	  --argc, ++argv;
	  sscanf(temp_word, "%d", &vlevel);
	  vflag = TRUE;
	  break;
	/* Remove a word from the dictionary
	*/
	case 'd':
	  ++argv, --argc;
	  delete_word(*argv);
	  ++argv, --argc;
	  dflag = TRUE;
	  break;
	/* Print all dictionary matches to a given word.  Use with the
	** 's' flag to determine the number for that word.
	*/
	case 'r':
	  ++argv, --argc;
	  rflag = TRUE;
	  break;
	case 'p':
	  ++argv, --argc;
	  strcpy(temp_word, *argv);
	  ++argv, --argc;
	  sscanf(temp_word, "%d", &pword_index);
	  pflag = TRUE;
	  break;
      }
    }
  }

  /* Make sure that we have a cipher.
  */
  if(!(*filename)){
    if(!aflag && !dflag && !qflag){
      printf("What file would you like to use? ");
      scanf("%s", filename);
    }
    else{
      /* Exit silently...
      */
      exit(0);
    }
  }

  /* Remove the output file if it exists.
  */

  (void) remove(OFILE);

  fptr = fopen(filename, "r");
  if(!fptr){
    printf("Bad cipher filename.\n");
    exit(1);
  }

  /*Initialize the variables
  */

  num_words = 0;

  for(i = 0; i < MAXWORDS; i++){
    *(wordlist[i].word) = '\0';
    wordlist[i].length = 0;
    wordlist[i].mult = 0;
    wordlist[i].dict = NULL;
    wordlist[i].dictsize = 0;
  }

  /* Read in the cipher as an array of lines.  We'll use this later
  ** on when we print the cipher.
  */

  i = 0;
  while(!feof(fptr)){
    fgets(cipher[i], MAXLENGTH, fptr);
    if(!feof(fptr))
      i++;
  }
  lines_in_cipher = i;

  rewind(fptr);

  /* Read in the cipher as an array of word structures
  */

  while(!feof(fptr)){
    /* Get the next word in the cipher
    */

    get_next_word(fptr, temp_word);

    /* If the word isn't in the array, then add it, otherwise increase
    ** the multiplicity of the word in the array.
    */

    if(*temp_word != '\0' && strlen(temp_word) != 1){
      for(i = 0; 
	  i < num_words && (strcmp(temp_word, wordlist[i].word) != 0); 
	  i++);
      if(i == num_words){
	strcpy(wordlist[num_words].word, temp_word);
	wordlist[num_words].length = strlen(temp_word);
	wordlist[num_words].mult++;
	num_words++;

	/* Add the letters to the histogram
	*/

	for(j = strlen(temp_word); j >= 0; j--){
	  if(isalpha(temp_word[j]))
	    lhist[temp_word[j] - 'a']++;
	}
      }
      /* Otherwise just increase the multiplicity of the word.
      */
      else
	wordlist[i].mult++;
    }
  }
  (void) fclose(fptr);

  /* count the number of unique letters in the histogram
  */

  for(i = 0; i < 26; i++)
    if(lhist[i])
      num_ulets++;

  /* Check for a non-valid threshhold
  */
  if((threshhold > num_words) )
    threshhold = DEF_THRESH;

  /* Loop through the wordlist.  For each word, find the length of the list
  ** and read in the valid words.
  ** Get the histogram value for each word, too.
  */

  printf("Reading dictionary..."), fflush(stdout);
  for(i = 0; i < num_words; i++){

    /* Set the hash values for the words
    */

    wordlist[i].hvalue = get_hvalue(wordlist[i].word, lhist);

    /* Open the correct dictionary file
    */

    strcpy(filename, get_dfname(wordlist[i].word));
    dfptr = fopen(filename, "r");
    if(!dfptr){
      printf("Could not open dictionary file %s.\n", filename);
      exit(1);
    }

    /* Count the number of words in the dictionary and malloc an array
    ** that's large enough to hold the whole thing.  We won't actually
    ** use all of it, but this way we won't be caught shorthanded.
    */

    while(!feof(dfptr)){
      fgets(temp_word, MAXLENGTH, dfptr);
      if(!feof(dfptr))
	wordlist[i].dictsize++;
    }
    rewind(dfptr);

    /* Read in the valid words now and put them into the previous
    ** array.
    */

    wordlist[i].dict = (char **)malloc(sizeof(char *) * wordlist[i].dictsize);
    wordlist[i].dictsize = 0;
    if(wordlist[i].dict == NULL){
      /* Oops!
      */
      printf("Error in allocating memory for the wordlist.\n");
      exit(1);
    }

    while(!feof(dfptr)){
      fgets(temp_word, MAXLENGTH, dfptr);
      if(!feof(dfptr)){
	*strrchr(temp_word, '\n') = '\0';
	lowerchar(temp_word);
	if(validate_mapping(temp_word, wordlist[i].word))
	  if(validate_mapping(wordlist[i].word, temp_word))
	    wordlist[i].dict[wordlist[i].dictsize++] = strdup(temp_word);
      }
    }
    (void) fclose(dfptr);
  }
  putchar('\n');

  /* Sort the list according to the hash value for each word
  */
  qsort((void *)wordlist, (size_t)num_words, 
	sizeof(WORD), (void *) compare);

  /* Sort it according to the convoluted criteria in the wordsort 
  ** function, but only if we have a threshhold of zero.  This sorting
  ** method works slowly for a threshhold of > 0.
  */
  if(threshhold == 0)
    wordsort();

  /* Now start solving.
  */
  if(sflag){
    printf("Total words:  %d\n", num_words);
    for(i = 0; i < num_words; i++){
      printf("  word %2d = %-13s  d.length = %5d \tmult. = %d \thvalue = %d\n", 
	     i+1, wordlist[i].word, wordlist[i].dictsize, 
	     wordlist[i].mult, wordlist[i].hvalue);
    }
  }

  if(pflag){
    if(pword_index != 0)
      for(i = 0; i < wordlist[pword_index-1].dictsize; i++)
	printf("%s\n", wordlist[pword_index-1].dict[i]);
    else
      for(pword_index = 0; pword_index < num_words; pword_index++)
	for(i = 0; i < wordlist[pword_index].dictsize; i++)
	  printf("%s\n", wordlist[pword_index].dict[i]);
    exit(0);
  }
  printf("Solving"), fflush(stdout);
  solve_cipher("                          ", 0, 0);
  putchar('\n');

  exit(0);
}

void
wordsort(){
  WORD temp_word, *cword, *maxword=NULL;
  double cvalue = 0., mvalue = 0.;
  int mhvalue = 0, i, j, k;
  int chvalue=0, ulets[26], hist[26];

  /* Initilize the used letter list to all 0's.
  ** Initialize the histogram to be identical to the original
  ** global histogram.
  */

  for(i = 0; i < 26; i++){
    ulets[i] = FALSE;
    hist[i] = lhist[i];
  }

  for(i = 0; i < num_words; i++){
    maxword = NULL;
    mvalue = 0;
    for(j = i; j < num_words; j++){

      cword = &wordlist[j];
      cvalue = 0.;
      chvalue = get_hvalue(cword->word, hist);

      /* Loop through the letters in cword and add up the number that
      ** have been used.
      */

      for(k = 0; k < cword->length; k++){
	if(ulets[cword->word[k] - 'a'])
	  cvalue++;
      }
      cvalue = cvalue / cword->length;

      /* Does this word have more used letters than the current
      ** maximum?
      **
      */

      if(cvalue > mvalue || maxword == NULL){
	maxword = cword;
	mvalue = cvalue;
	mhvalue = chvalue;
      }
      else if(cvalue == mvalue){
	if(chvalue > mhvalue){
	  maxword = cword;
	  mvalue = cvalue;
	  mhvalue = chvalue;
	}
	else if(cword->length == maxword->length){
	  if(cword->length > maxword->length){
	    maxword = cword;
	    mvalue = cvalue;
	    mhvalue = chvalue;
	  }
	}
      }
    }

    /* Swap the maxword with the current word.
    ** This involves copying the word info.
    */

    if(maxword){
      copy_word(&temp_word, &wordlist[i]);
      copy_word(&wordlist[i], maxword);
      copy_word(maxword, &temp_word);
    }

    /* Update the used letter list.
    */

    for(k = 0; k < wordlist[i].length; k++){
      ulets[wordlist[i].word[k] - 'a'] = TRUE;
    }

    /* Remove the letters of the word added from the histogram.
    */

    for(k = 0; k < wordlist[i].length; k++){
      hist[wordlist[i].word[k] - 'a']=0;
    }
  }
}

int
get_hvalue(char *word, int *hist){
  int value=0, i;
  int ulets[26];

  for(i = 0; i < 26; i++)
    ulets[i] = FALSE;

  while(*word){
    if(isalpha(*word)){
      if(ulets[*word - 'a'] == FALSE){
	value += hist[*word - 'a'];
	ulets[*word - 'a'] = TRUE;
      }
    }
    word++;
  }

  return value;
}

void
copy_word(WORD *word1, WORD *word2){
  /*
  word1 = word2;
  */
  strcpy(word1->word, word2->word);
  word1->length = word2->length;
  word1->mult = word2->mult;
  word1->dict = word2->dict;
  word1->dictsize = word2->dictsize;
  word1->hvalue = word2->hvalue;
}
  
char *decode_word(char *key, char *cword){
  char dword[MAXLENGTH];
  int i=0;

  while(*cword){
    if(isalpha(*cword))
      dword[i] = key[(*cword | ' ') - 'a'];
    else
      dword[i] = *cword;
    cword++, i++;
  }
  dword[i] = '\0';

  return dword;
}

void
get_next_word(FILE *fptr, char *word){
  char c=0;

  /* Find the next string of letters, not excluding ' and -
  */

  while(!isalpha(c) && !feof(fptr) && c != '\'' && c != '-')
    c = getc(fptr);
    if(isalpha(c))
      c |= ' ';

  /* Copy the string of letters into the word
  */

  while(( (isalpha(c)) || c == '\'' || c == '-') && !feof(fptr)){
    *word++ = c;
    c = getc(fptr);
    if(isalpha(c))
      c |= ' ';
  }
  
  *word = '\0';
  lowerchar(word);
}

void
lowerchar(char *word){
  while(*word){
    if(isalpha(*word))
      *word |= ' ';
    word++;
  }
}

int
compare(WORD *word1, WORD *word2){
  /*
  double value1;
  double value2;
  */

  if(word1->hvalue < word2->hvalue)
    return 1;
  else if(word1->hvalue > word2->hvalue)
    return -1;
  if(word1->length > word2->length)
    return -1;
  else if(word1->length < word2->length)
    return 1;
  else
    return 0;

  /*  Commented out because I'm still trying various sorting
  ** criteria.  I don't want to delete this because I might want to 
  ** put it back in later.

  value1 = (double) word1->hvalue / word1->dictsize;
  value2 = (double) word2->hvalue / word2->dictsize;

  if(value1 < value2)
    return 1;
  else if(value1 > value2)
    return -1;
  else
    return 0;
  */
}

int
charcomp(char *c, char *d){
  if(*c < *d)
    return 1;
  else if(*c > *d)
    return -1;
  else return 0;
}

int
validate_mapping(char *word1, char *word2){
  char key[26];
  int mapping = TRUE, i;

  for(i = 0; i < 26; i++)
    key[i] = ' ';

  if(strlen(word1) == strlen(word2)){
    while(*word1 && *word2 && mapping == TRUE){
      if(*word1 == *word2 && rflag == FALSE){
	if(isalpha(*word1))
	  mapping = FALSE;
      }
      else if(key[*word1 - 'a'] == ' ')
	key[*word1 - 'a'] = *word2;
      else if(key[*word1 - 'a'] != *word2)
	mapping = FALSE;
      word1++, word2++;
    }
  }
  else
    mapping = FALSE;

  return mapping;
}

char *get_dfname(char *word){
  int length;
  char string[256];

  length = strlen(word);
  sprintf(string, "%s/len%02d", DICT_DIR, length);

  return string;
}

void
delete_word(char *word){
  FILE *tfptr, *dfptr;
  char tmpfname[MAXLENGTH], filename[MAXLENGTH];
  char tmp_word[MAXLENGTH];

  /* Get the name of the dictionary file where the word should reside
  */

  strcpy(filename, get_dfname(word));

  strcpy(tmpfname, filename);
  strcat(tmpfname, ".tmp");

  dfptr = fopen(filename, "r");
  tfptr = fopen(tmpfname, "w");

  /* Loop through the old dictionary and copy it to the tmp file
  ** sans the word to be deleted.
  */

  lowerchar(word);
  while(!feof(dfptr)){
    fgets(tmp_word, MAXLENGTH, dfptr);
    if(!feof(dfptr)){
      lowerchar(tmp_word);
      *strrchr(tmp_word, '\n') = '\0';
      if(strcmp(tmp_word, word) == 0);
      else{
	/* The word didn't match.  Copy it.
	*/
	fprintf(tfptr, "%s\n", tmp_word);
      }
    }
  }

  (void) rename(tmpfname, filename);
  (void) fclose(dfptr);
  (void) fclose(tfptr);
}

void
query_word(char *word){
  FILE *dfptr;
  int word_found = FALSE;
  char filename[MAXLENGTH], temp_word[MAXLENGTH];

  strcpy(filename, get_dfname(word));

  dfptr = fopen(filename, "r");
  if(!dfptr){
    printf("Error opening dictionary file %s.\n", filename);
  }
  while(!feof(dfptr)){
    fgets(temp_word, MAXLENGTH, dfptr);
    if(!feof(dfptr)){
      *strrchr(temp_word, '\n') = '\0';
      if(strcmp(temp_word, word) == 0){
	word_found = TRUE;
	printf("%s is in the dictionary.\n", temp_word);
      }
      *temp_word = (isupper(*temp_word))?(*temp_word | ' '):toupper(*temp_word);
      if(strcmp(temp_word, word) == 0){
	word_found = TRUE;
	printf("%s is in the dictionary.\n", temp_word);
      }
    }
  }
  if(!word_found)
    printf("%s not found.\n", word);
  
  (void) fclose(dfptr);
}

void
move_to_end(char *word){
  char filename[MAXLENGTH];
  char tfname[MAXLENGTH];
  char tmp_word[MAXLENGTH];
  FILE *dfptr, *tfptr;

  strcpy(filename, get_dfname(word));
  strcpy(tfname, filename);
  strcat(tfname, ".tmp");

  dfptr = fopen(filename, "r");
  tfptr = fopen(tfname, "w");

  if(dfptr == NULL || tfptr == NULL){
    printf("Error opening dictionary files.\n");
  }
  else{
    while(!feof(dfptr)){
      fgets(tmp_word, MAXLENGTH, dfptr);
      if(!feof(dfptr)){
	*strrchr(tmp_word, '\n') = '\0';
	lowerchar(tmp_word);
	lowerchar(word);
	if(strcmp(word, tmp_word)){
	  fprintf(tfptr, "%s\n", tmp_word);
	}
      }
    }
    fprintf(tfptr, "%s\n", word);
  }

  (void) rename(tfname, filename);

  (void) fclose(tfptr);
  (void) fclose(dfptr);
}


void
add_word(char *word){
  char word1[MAXLENGTH];
  char filename[MAXLENGTH], temp_word[MAXLENGTH];
  int i, valid = TRUE;
  FILE *dfptr;

  strcpy(temp_word, word);
  strcpy(filename, get_dfname(temp_word));

  /* Check that the word is valid.
  ** Is it made up of valid characters?
  ** Is it already in the dictionary?
  */

  for(i = 0; i < strlen(temp_word); i++)
    if(!isalpha(temp_word[i]) && temp_word[i] != '\'' && temp_word[i] != '-')
      valid = FALSE;

  if(valid){
    dfptr = fopen(filename, "r");
    if(!dfptr){
      printf("Error opening dicitonary file %s\n", filename);
    }
    else{
      while(!feof(dfptr)){
	fgets(word1, MAXLENGTH, dfptr);
	if(!feof(dfptr)){
	  *strrchr(word1, '\n') = '\0';
	  if(strcmp(word1, temp_word) == 0){
	    printf("Word %s already exists in dictionary.\n", temp_word);
	    valid = FALSE;
	  }
	}
      }
      (void) fclose(dfptr);
    }
  }

  if(valid){
    dfptr = fopen(filename, "a");
    if(!dfptr){
      printf("Error opening dicitonary file %s\n", filename);
    }
    else{
      fprintf(dfptr, "%s\n", temp_word);
      (void) fclose(dfptr);
    }
  }
}


int
update_key(char *key, char *cword, char *dword){
  char dkey[27], skey[27];
  register int valid=TRUE, i;

  strcpy(dkey, key);

  if(strlen(cword) == strlen(dword)){
    while(*cword && *dword && valid){
      
      /* Does this try to map a letter onto itself?
      */

      if(*cword == *dword && rflag == FALSE){
	if(isalpha(*cword))
	  return FALSE;
      }
      else if(dkey[*cword - 'a'] == ' '){
	dkey[*cword - 'a'] = *dword;
      }
      else if(dkey[*cword - 'a'] != *dword)
	return FALSE;
      cword++, dword++;
    }
    if(valid){
      strcpy(skey, dkey);
      qsort((void *)skey, 26, sizeof(char), (void *) charcomp);
      for(i = 0; i < 26 && valid && skey[i] != ' '; i++)
	if(skey[i] == skey[i+1])
	  return FALSE;
    }
  }
  else
    valid = FALSE;
  
  if(valid)
    strcpy(key, dkey);

  return valid;
}

void
solve_cipher(char *key, int word_index, int real_words){
  register int valid_key=TRUE;
  register int i, j;
  char nkey[27];
  char temp_word[MAXLENGTH];

  /* Can we make enough real words with the remaining words in the
  ** list?  If not, then don't bother trying.
  */

  if((threshhold + real_words) < word_index);

  /* Are we at the end of the list now?
  */

  else if(word_index >= num_words){

    /* Now that we're at the end of the list, have we made enough real
    ** words?
    */

    if(num_words - real_words <= threshhold){

      /* Has this key been used before?
      */

      valid_key = TRUE;
      for(i = 0; i < num_keys; i++)
	if(strcmp(keylist[i], key) == 0)
	  valid_key = FALSE;

      /* If not, then print it.
      */

      if(valid_key){
	if(num_keys < MAXKEYS)
	  strcpy(keylist[num_keys++], key);
	else if(num_keys == MAXKEYS-1)
	  printf("MAXKEYS reached.  Duplicate solutions may appear.\n");
	putchar('.'), fflush(stdout);

	/* If the threshhold is zero, then we want to move the word
	** to the top of the list so that eventually the most
	** common words are analyzed first.
	*/

	if(threshhold == 0){
          for(i = 0; i < num_words; i++){
	    strcpy(temp_word, decode_word(key, wordlist[i].word));
	    move_to_end(temp_word);
	  }
	}
	    
	/* Print the solution to the output file
	*/

	ofptr = fopen(OFILE, "a");
	fprintf(ofptr, "\nkey = %s\n", key);
	for(i = 0; i < lines_in_cipher; i++){
	  j = 0;
	  while(cipher[i][j]){
	    if(isalpha(cipher[i][j]))
	      fputc(key[(cipher[i][j]-'a')], ofptr);
	    else
	      fputc(cipher[i][j], ofptr);
	    j++;
	  }
	  fputs(cipher[i], ofptr);
	}
	(void) fclose(ofptr);
      }
    }
  }

  /* Since we're not at the end of the wordlist, try the next word 
  ** in the list
  */

  else{
    for(i = wordlist[word_index].dictsize-1; i >= 0; i--){

      strcpy(nkey, key);

      valid_key= 
      update_key(nkey, wordlist[word_index].word, wordlist[word_index].dict[i]);

      if(valid_key){

	/* Are we running in verbose mode?
	*/

	  if(vflag){
	    if(word_index <= vlevel){
	      printf("index = %2d,   dict index = %5d,  %s -> %s\n",
		      word_index, i, wordlist[word_index].word, 
		      wordlist[word_index].dict[i]);
	    }
	  }

	solve_cipher(nkey, word_index+1, real_words+1);
      }
    }
    strcpy(nkey, key);

    if(vflag)
      if(word_index <= vlevel)
	printf("index = %2d,   dict_index = (nil),  dict_word = (null)\n", word_index);
    solve_cipher(nkey, word_index+1, real_words);
  }
  if(Dflag){
    printf("Current depth = %d\n", word_index);
  }
}
