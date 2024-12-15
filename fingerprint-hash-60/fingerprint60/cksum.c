#include <stdio.h>
#include "crc32file.h"

static unsigned char h[CRC32FILE_HASHLEN];
static unsigned char input[1024];

void sum(fi,out)
FILE *fi;
char *out;
{
 crc32file s;
 int i;
 int len;

 crc32file_clear(&s);
 len = 0;
 while (i = fread(input,1,sizeof(input),fi))
  {
   crc32file_addn(&s,input,i);
   len += i;
  }
 crc32file_hash(&s,h);

 sprintf(out,"%10lu %8lu"
   ,h[0] + 256 * (h[1] + 256 * (h[2] + 256 * (unsigned long) h[3]))
   ,len
   );
}
