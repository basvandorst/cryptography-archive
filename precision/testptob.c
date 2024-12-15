#include <stdio.h>
#include <string.h>
#include "precision.h"

unsigned char alphabet[256] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

int map[256];

main()
{
   int i, j, count, size, radix;
   precision p = pUndef;
   char num[256];

   for (i = 0; i < 256; i++) {
      map[i] = -1;
   }
   for (i = 0; i < strlen(alphabet); i++) {
      map[alphabet[i]] = i;
   }

   do {
      pset(&p, fgetp(stdin));
      if (p == pUndef) break;
      count = scanf("%d %d", &size, &radix);
      j = ptob(p, num, size, alphabet, radix);
      fwrite(num, size, 1, stdout);
      printf(" %d\n", j);
      fgets(num, 256, stdin);
      j = btop(&p, num, strlen(num), map, radix);
      printf(" %d\n", j);
      putp(p);
   } while (1);
   pdestroy(p);
   return 0;
}
