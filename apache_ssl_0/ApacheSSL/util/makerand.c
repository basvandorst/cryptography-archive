#include<stdio.h>

unsigned long trand32(void);

#define SIZEOFRAND 1024

int
main(void)
{
  int count = 0;
  unsigned long r32;

  fprintf(stderr, "Generating %d bits of randomness", SIZEOFRAND);
  while(count++ < SIZEOFRAND/32)
    {
      r32 = trand32();
      write(1, &r32, sizeof r32);
      fputc('.', stderr);
    }
  fputc('\n', stderr);
}


