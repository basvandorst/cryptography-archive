
/*
 * WPCRACK.C - author unknown
 *
 * 7/10/95 Added support for WP 4.x files, based on code from the 
 * 	   WPUncryp program by Ron Dippold (rdippold@qualcomm.com)
 *			- Mark Grant (mark@unicorn.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef unix
#include <unistd.h>
#define fputchar(x)	putchar(x)
#endif

void main();

void main(argc,argv)
  int  argc;
  char *argv[];
    {
	register int  current_char;
	register int  key_1;
	static   long freq [24] [256];
	FILE *infile;
      	static   int  key_length;
	static   int  key_offset;
	static   int  max_char;
	static   long max_freq;
	static   int  most_common;
	static	long	text_start;
	unsigned char	buf[4];

      if ((argc == 2) || (argc == 3))
        if ((infile=fopen(argv[1],"rb")) == NULL)
          printf("Fatal error:  cannot open %s for input.\n",argv[1]);
        else
          {

		/* Check for version 4 or 5 */

		if( fread( buf, 1, 4, infile ) != 4) {
			fclose( infile );
			printf( "Couldn't read 4 bytes from file %s!\n", 
				argv[1]);
			exit( 3 );
		}

		if( buf[0]==0xff && buf[1]==0x57 && 
			buf[2]==0x50 && buf[3]==0x43 ) {
			printf( "This is a Word Perfect 5.x file.\n");
			text_start = 16l;
		} else {
			if( buf[0]==0xfe && buf[1]==0xff && 
				buf[2]==0x61 && buf[3]==0x61 ) {
				printf( "This is a Word Perfect 4.1 encrypted file.\n");
				text_start = 6l;
			} else {
				fclose( infile );
				printf( "The identification bytes in this file are not those of a\n" );
				printf( "Word Perfect 5.x or 4.x encrypted file.\n" );
				exit( 4 );
			}
		}

            if (argc == 3)
              most_common=atoi(argv[2]);
            else
              most_common=0;
            printf("Possible keys:\n");
            for (key_length=1; key_length <= 24; key_length++)
              {
         	fseek(infile,text_start,SEEK_SET);

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
           "\nWPCRACK recovers the password from a password protected WordPerfect file.\n\n");
          printf(
           "Usage:  WPCRACK <protected file> [<most common character>]\n\n");
          printf(
           "   The most common character in the original (unprotected) file may be entered\n");
          printf(
           "   as a decimal value.  It defaults to 0.  If 0 doesn't work, try 32 (ASCII\n");
          printf("   space).\n\n");
#ifdef unix
	printf("Example: wpcrack /wp51/ciphered\n");
#else
          printf("Example: WPCRACK C:\\WP51\\CIPHERED\n");
#endif
        }      
      return;
    }
