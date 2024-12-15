#include <stdio.h>
#include "fprintfile.h"

static unsigned char h[FINGERPRINTFILE_HASHLEN];
static unsigned char input[1024];

void sum(fi,out)
FILE *fi;
char *out;
{
 fingerprintfile f;
 int i;

 fingerprintfile_clear(&f);
 while (i = fread(input,1,sizeof(input),fi))
   fingerprintfile_addn(&f,input,i);
 fingerprintfile_hash(&f,h);

 out[fingerprintfile_fmt(out,h)] = 0;
}
