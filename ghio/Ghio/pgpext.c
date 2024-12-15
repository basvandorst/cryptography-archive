#include <stdio.h>
#include <string.h>
 
FILE *outfile;
FILE *pgpoutfile;

void main(int argc,char *argv[]) {
  char input[256];

  pgpoutfile=fopen(argv[1],"w");
  outfile=fopen(argv[2],"w");

  while((fgets(input,255,stdin)>0)
       &&(strcmp(input,"-----BEGIN PGP MESSAGE-----\n")!=0)) {}
  fprintf(pgpoutfile,"%s",input);
  while((fgets(input,255,stdin)>0)
       &&(strcmp(input,"-----END PGP MESSAGE-----\n")!=0)) {
    fprintf(pgpoutfile,"%s",input);
  }
  fprintf(pgpoutfile,"%s",input);
  while(fgets(input,255,stdin)>0) {
    fprintf(outfile,"%s",input);
  }

  fclose(pgpoutfile);
  fclose(outfile);
}
