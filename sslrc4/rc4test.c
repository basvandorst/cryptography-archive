/* Simple sample how to use EAYSSL lib routines */
#include <stdio.h>
#include <string.h>
#include "rc4.h"

#define print(foo) \
	for(i=0;i<sizeof(foo);i++) printf("0x%02x ",foo[i]); \
	printf("\n");

void main()
{
	int i;
        RC4_KEY rc4key;
	unsigned char key[8];
	unsigned char input[8],output[8];

	memset(key,0,sizeof(key));
	memset(input,0,sizeof(input));
	memset(output,0,sizeof(output));

	RC4_set_key(&rc4key,sizeof(key),key);

	printf("Key:\t"); print(key);
	printf("Input:\t"); print(input);

	RC4(&rc4key,sizeof(input),input,output);
	printf("Output:\t"); print(output);
}
