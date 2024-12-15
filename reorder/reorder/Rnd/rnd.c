#include <stdio.h>
#include "rc4.h"

main(argc, argv)
int argc;
char *argv[];
{
	int seed,i,res,num;
	rc4_key key;
	int key_len;
	FILE *fptr;
	char line[11];
	unsigned char password[20000];


	seed = time(NULL);
	if((fptr = fopen("rnd.state","r"))!=NULL) {
	  read(fileno(fptr),password,256);
	  i = 256;
	  fclose(fptr);
	} else {
	  srand(seed);
	  for(i=0;i<128;i++) {
	    password[i]=rand()%256;
	  }
	}
	if(argc == 1) {
	  num = 1;
	} else {
	  num = atoi(argv[1]);
	}
	if (num > 10000) num = 10000;
	prepare_key(password, i, &key);
	rc4(password,seed%4096,&key); /* dump a whole buch of values */
	num *= 2; /* going to get number 1 to 256^2 */
	rc4(password,num,&key);
	for (i=0;i<num;i+=2){
	  printf("%d\n",password[i] + password[i+1]*256);
	}
	rc4(password,seed%4096,&key); /* dump a whole buch of values */
	fptr = fopen("rnd.state","w+");
	rc4(password,256,&key);
	write(fileno(fptr),password,256); /* write out 256 values to use for the next key */
	fclose(fptr);
}
