#include "misc.h"

void main(int argc, char **argv)
{
  FILE *infile;
  struct HT HTEntry;

  printf("\nQExt %s", Ver);

  if(argc < 2) {
    printf("Usage: QExt [infile]\n"); 
    return;
  }

  infile = fopen(argv[1], "rb");

  fread(&HTEntry, sizeof(struct HT), 1, infile); 

  while (!feof(infile)) {
    printf("%s\n", HTEntry.currentword); 
    fread(&HTEntry, sizeof(struct HT), 1, infile); 
  }
}
