/***************************************************************************/
/*                                                                         */
/* Bitstat - generate bit-wise [and byte-wise] statistics of a file        */
/*                                                                         */
/* Usage: bitstat [-v] file        (normal usage)                          */
/*    or: bitstat [-v] < file                                              */
/*    or: type file | bitstat [-v]                                         */
/*    or: bitstat [-v]             (keyboard input (terminate with ^Z))    */
/*                                                                         */
/* Version:  1.0                                                           */
/* Platform: DOS (easily transportable)                                    */
/* Compiler: gcc                                                           */
/* Date:     07/09/96                                                      */
/* License:  Freeware                                                      */
/* Author:   Winston Rayburn                                               */
/*                                                                         */
/***************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define INPUT_BUFFER_SIZE 1024         /* in bytes */

#define TRUE (1==1)
#define FALSE (1==0)

#define USAGE printf("%s:  Usage: %s [-v] file\n", argv[0], argv[0]); \
              exit(1)

/***************************************************************************/

main (argc, argv)
int argc;
char *argv[];
{
  float set_ratio, deviance_from_unity;
  float bit_percentage[8], byte_percentage[256];
  float average_byte_count, average_byte_percentage;
  float ones_bit_percentage, zeros_bit_percentage;
  unsigned int bytes = 0, ones = 0, zeros;
  unsigned int i, got;
  unsigned char uc;
  int in;
  unsigned int byte_count[256] = {0};
  unsigned int bit_count[8] = {0};
  unsigned int bits;
  char *p;
  char verbose = FALSE;
  char c[INPUT_BUFFER_SIZE];

/***************************************************************************/

  for (i=1; i<argc; i++)
  {
    p=argv[i];
    if (*p++ == '-')
    {
      switch (*p)
      {
        case 'v':
        case 'V':
          verbose = TRUE;
          break;
        default:
          USAGE;
          break;
      }
    }
  }

  p = argv[argc-1];
  if ((argc > 1) && (*p != '-'))
  {
    in = open(argv[argc-1], O_RDONLY|O_BINARY);
    if (in < 0)
    {
      printf("%s:  Error opening %s\n", argv[0], argv[argc-1]);
      USAGE;
    }
  }
  else
    in = 0;

  while ((got = read(in, c, INPUT_BUFFER_SIZE)) > 0)
  {
    bytes += got;

    for(i=0; i<got; i++)
    {
      uc = (unsigned char) c[i];
      byte_count[uc]++;

      if (uc & 0x01)
        bit_count[0]++;
      if (uc & 0x02)
        bit_count[1]++;
      if (uc & 0x04)
        bit_count[2]++;
      if (uc & 0x08)
        bit_count[3]++;
      if (uc & 0x10)
        bit_count[4]++;
      if (uc & 0x20)
        bit_count[5]++;
      if (uc & 0x40)
        bit_count[6]++;
      if (uc & 0x80)
        bit_count[7]++;
    }
  }

  if (bytes == 0)
  {
    printf("%s:  File %s empty\n", argv[0], in==0?"stdin":argv[argc-1]);
    USAGE;
  }
  else
    close(in);

  for (i=0; i<8; i++)
  {
    ones += bit_count[i];
    bit_percentage[i] = (float)100.0*bit_count[i]/bytes;
  }
  bits = bytes*8;
  zeros = bits - ones;
  ones_bit_percentage = (float)100.0*ones/(float)bits;
  zeros_bit_percentage = (float)100.0*zeros/(float)bits;
  set_ratio = (float)ones/zeros;
  deviance_from_unity = set_ratio-1;
  average_byte_count = (float)bytes/(float)256.0;
  average_byte_percentage = (float)100.0*1.0/256.0;
  for (i=0; i<256; i++)
  {
    byte_percentage[i] = (float)100*byte_count[i]/bytes;
  }

  printf("%s contains %d bytes (%d bits)\n",
         in==0?"stdin":argv[argc-1], bytes, bits);

  if (verbose)
  {
    printf("\n         BYTE ANALYSIS\n\n");
    printf(" Byte     Count    Percentage\n");
    printf(" ----     -----    ----------\n");
    printf(" norm    %8.1f  %10.6f\n", average_byte_count,
                                       average_byte_percentage);
    for (i=0; i<256; i++)
    {
      printf("%c %3d  %8d    %10.6f\n", isprint(i)?i:' ', i,
                                        byte_count[i], byte_percentage[i]);
    }
  }

  printf("\n         BIT ANALYSIS\n\n");
  printf("Value     Count    Percentage\n");
  printf("-----     -----    ----------\n");
  printf("  one  %8d    %10.6f\n", ones, ones_bit_percentage);
  printf(" zero  %8d    %10.6f\n", zeros, zeros_bit_percentage);
  printf("\n  Bit     Count    Percentage\n");
  printf("  ---     -----    ----------\n");
  for (i=0; i<8; i++)
  {
    if (i==0)
      printf("msb");
    else if (i==7)
      printf("lsb");
    else
      printf("   ");
    printf("%2d  %8d    %10.6f\n", 7-i, bit_count[7-i], bit_percentage[7-i]);
  }
  printf("\n1/0 ratio           %9.6f\n", set_ratio);
  printf("Deviance from unity %9.6f\n", deviance_from_unity);
}
