/* 
** 12/1995 written by Gino Corradin (gino@dei.unipd.it)
*/

/*
** byteorder.c : produces the include file byteorder.h
*/
#include <stdio.h>

void main()
{
int i;
union {
unsigned int l;
char c[sizeof(int)];
} u;
FILE *fin;

if (sizeof(int) <= 4)
	u.l = 0x04030201L;
else
	/*u.l = (0x08070605L << 32) | 0x04030201L;/**/ ;

fin=fopen("byteorder.h", "w");
fprintf(fin, "#define BYTEORDER 0x");
for (i=0; i<sizeof(int); i++) fprintf(fin, "%c", u.c[i]+'0');
fprintf(fin, "\n");
fclose(fin);

exit(0);
}

