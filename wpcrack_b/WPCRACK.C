#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void main(int,char **);

void main(argc,argv)
  int  argc;
  char *argv[];
    {
      register int  current_char;
      static   long freq [24] [256];
               FILE *infile;
      register int  key_1;
      static   int  key_length;
      static   int  key_offset;
      static   int  max_char;
      static   long max_freq;
      static   int  most_common;

      if ((argc == 2) || (argc == 3))
        if ((infile=fopen(argv[1],"rb")) == NULL)
          printf("Fatal error:  cannot open %s for input.\n",argv[1]);
        else
          {
            if (argc == 3)
              most_common=atoi(argv[2]);
            else
              most_common=0;
            printf("Possible keys:\n");
            for (key_length=1; key_length <= 24; key_length++)
              {
                fseek(infile,(long) 16,SEEK_SET);
                for (key_offset=0; key_offset < key_length; key_offset++)
                  for (current_char=0; current_char < 256; current_char++)
                    freq[key_offset][current_char]=(long) 0;
                key_1=key_length+1;
                key_offset=0;
                while ((current_char=fgetc(infile)) != EOF)
                  {
                    (freq[key_offset++][current_char ^ (key_1++)])++;
                    if (key_offset >= key_length)
                      key_offset=0;
                    if (key_1 >= 256)
                      key_1=0;
                  }
                printf("     \"");
                for (key_offset=0; key_offset < key_length; key_offset++)
                  {
                    max_freq=(long) 0;
                    max_char=0;
                    for (current_char=0; current_char < 255; current_char++)
                      if (freq[key_offset][current_char] > max_freq)
                        {
                          max_freq=freq[key_offset][current_char];
                          max_char=current_char;
                        }
                    max_char^=most_common;
                    if ((max_char >= 32) && (max_char < 127))
                      fputchar(max_char);
                    else
                      fputchar((int) '?');
                  }
                printf("\"\n");
              }
            fclose(infile);
          }
      else
        {
          printf(
           "\nWPCRACK recovers the password from a password protected Word "
           "Perfect file.\n\n");
          printf(
           "Usage:  WPCRACK <protected file> [<most common character>]\n\n");
          printf(
           "   The most common character in the original (unprotected) file "
           "may be entered\n");
          printf(
           "   as a decimal value.  It defaults to 0.  If 0 doesn't work, try "
           "32 (ASCII\n");
          printf(
           "   space).\n\n");
          printf(
           "Example: WPCRACK C:\\WP51\\CIPHERED\n");
        }      
      return;
    }
