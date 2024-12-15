#include <stdio.h>
#include "snefrufile.h"

static unsigned char h[SNEFRUFILE_HASHLEN];
static unsigned char input[1024];

void sum(fi,out)
FILE *fi;
char *out;
{
 snefrufile s;
 int i;
 char *x;

 snefrufile_clear(&s);
 while (i = fread(input,1,sizeof(input),fi))
   snefrufile_addn(&s,input,i);
 snefrufile_hash(&s,h);

 x = out;
 for (i = 0;i < SNEFRUFILE_HASHLEN;++i)
  {
   if (!(i & 3))
     *x++ = ' ';
   sprintf(x,"%02x",(int) h[i]);
   x += strlen(x);
  }
 *x = 0;
}
