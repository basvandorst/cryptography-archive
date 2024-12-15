#include <stdio.h>
#include "fprintfile.h"

char line[200];
unsigned char h[57];

main()
{
 int i;
 int len;

 while (fgets(line,sizeof(line),stdin))
  {
   if (!(len = fingerprintfile_scan(line,h)))
     printf("bad fingerprint\n");
   else
    {
     for (i = 32;i < 48;++i) printf("%02x",(int) h[i]);
     printf("%s",line + len);
    }
  }
 exit(0);
}
