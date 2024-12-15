/*---------------------------------------------------------------------------
 * destest.c
 *
 *
 * 27-Feb-92 RP
 * 06-Jan-93 RP (for libdes)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mdes.h"

#define ctod(d) ((d) <= '9' ? (d) - '0' : (d) <= 'Z' ? (d) - ('A'-10):\
  (d) - ('a' - 10))

int des_hex_to_cblock(des_cblock *b, char *c)
{
  int i,n,m;
  for(i = 0; i < 8; i++) {
    if((n=ctod(c[i*2])) < 0 || n > 15) return 0;
    if((m=ctod(c[i*2+1])) < 0 || m > 15) return 0;
    ((char *)b)[i] = (n<<4)|m;
  }
  return 1;
}

void des_print_cblock(des_cblock *b)
{
  int i;
  for(i = 0; i < 8; i++) printf("%02X",((unsigned char *)b)[i]);
}

char linebuf[512];

struct Library *ACryptBase=0;


void main(int argc,char **argv)
{
  FILE *testdata;
  des_cblock  k,p,c,er,dr;
  des_key_schedule ks;
  int line=0,test=0,ok=1;
  char *fname;

  if(argc<2) fname = "des_test_data";
    else fname = argv[1];

  if(!(testdata = fopen(fname,"r"))) {
    printf("destest: can't open file %s\n",fname);
    exit(5);
  }

  while(fgets(linebuf,512,testdata))
  {
    line++;
    if(des_hex_to_cblock(&k,linebuf) &&
      des_hex_to_cblock(&p,linebuf+17) &&
      des_hex_to_cblock(&c,linebuf+34)) {
      test++;
      des_set_key(&k,&ks);
      des_ecb_encrypt(&p,&er,&ks,DES_ENCRYPT);
      des_ecb_encrypt(&c,&dr,&ks,DES_DECRYPT);
      if(memcmp(&er,&c,sizeof(des_cblock))||
        memcmp(&dr,&p,sizeof(des_cblock))) {
        printf("destest failed '%s' line %d, test %d\n",
          fname,line,test);
        printf("key= ");
        des_print_cblock(&k);
        printf("\n plain= ");
        des_print_cblock(&p);
        printf("\n chipher= ");
        des_print_cblock(&c);
        printf("\nenc= ");
        des_print_cblock(&er);
        printf("\ndec= ");
        des_print_cblock(&er);
        printf("\n");
        ok = 0;
        break;
      } 
    } else { 
      fputs(linebuf, stdout);
      fflush(stdout);
    }
  }
  printf("%s test with file %s\n", test?(ok?"successful":"failed"):
        "nothing to",fname);
  exit(ok ? EXIT_SUCCESS : EXIT_FAILURE);
}


