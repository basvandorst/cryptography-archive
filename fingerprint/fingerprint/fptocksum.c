#include <stdio.h>
#include "fprintfile.h"

char line[200];
unsigned char h[57];

main()
{
 int i;

 while (fgets(line,sizeof(line),stdin))
  {
   if (!(i = fingerprintfile_scan(line,h)))
     printf("bad fingerprint\n");
   else
     printf("%10lu %8lu%s"
       ,h[48] + 256 * (h[49] + 256 * (h[50] + 256 * (unsigned long) h[51]))
       ,h[52] + 256 * (h[53] + 256 * (h[54] +
          256 * (h[55] + 256 * (unsigned long) h[56])))
       ,line + i
       );
  }
 exit(0);
}
