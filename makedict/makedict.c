/* used for splitting up a file into separate files based on length.
*/

#include <stdio.h>
#include <string.h>

#define MAXLENGTH      80
#define NUMFILES       27

main(int argc, char **argv){
  char filename[MAXLENGTH], temp_word[MAXLENGTH];
  int length, i;
  FILE *ofptr[NUMFILES], *ifptr;
  
  if(argc != 2){
    printf("Usage:  spl filename\n");
    exit(0);
  }

  strcpy(filename, argv[1]);
  ifptr = fopen(filename, "r");
  if(!ifptr){
    printf("Error opening file %s.\n", filename);
    exit(0);
  }
  for(i = 1; i < NUMFILES; i++){
    sprintf(temp_word, "len%02d", i);
    ofptr[i] = fopen(temp_word, "a");
  }

  while(!feof(ifptr)){
    fgets(temp_word, MAXLENGTH, ifptr);
    if(!feof(ifptr) && !strchr(temp_word, ' ')){
      *strrchr(temp_word, '\n') = '\0';
      length = strlen(temp_word);
      fprintf(ofptr[length], "%s\n", temp_word);
    }
  }

  for(i = 1; i < NUMFILES; i++){
    fclose(ofptr[i]);
  }

  exit(0);
}
