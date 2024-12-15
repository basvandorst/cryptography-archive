#include <stdio.h>
#include "md5file.h"

static unsigned char h[MD5FILE_HASHLEN];
static unsigned char input[1024];

void sum(fi,out)
FILE *fi;
char *out;
{
 md5file s;
 int i;
 char *x;

 md5file_clear(&s);
 while (i = fread(input,1,sizeof(input),fi))
   md5file_addn(&s,input,i);
 md5file_hash(&s,h);

 x = out;
 for (i = 0;i < MD5FILE_HASHLEN;++i)
  {
   sprintf(x,"%02x",(int) h[i]);
   x += strlen(x);
  }
 *x = 0;
}
