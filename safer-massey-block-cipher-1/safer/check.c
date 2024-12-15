/*******************************************************************************
*
* FILE:           check.c
*
* DESCRIPTION:    check               create file 'check.in'
*                 check in_file_name  dump file in decimal notation
*
* AUTHOR:         Richard De Moliner (demoliner@isi.ee.ethz.ch)
*                 Signal and Information Processing Laboratory
*                 Swiss Federal Institute of Technology
*                 CH-8092 Zuerich, Switzerland
*
* DATE:           March 23, 1995
*
* CHANGE HISTORY:
*
*******************************************************************************/

/******************* External Headers *****************************************/
#include <stdio.h>

/******************* Functions ************************************************/
#ifndef NOT_ANSI_C
    int main(int argc, char *argv[])
#else
    int main(argc, argv)
    int argc;
    char *argv[];
#endif

{   unsigned int i, k;
    unsigned char ch;
    FILE *fd;

    if (argc == 1)
    {
        printf("creating file check.in...\n");
        if ((fd = fopen("check.in", "wb")) == NULL)
        {
            fprintf(stderr, "could not create file 'check.in'\n");
            return -1;
        }
        for (k = 0; k < 2; k++)
        {
            ch =0;
            for (i = 0; i < 8; i++)
                fwrite(&ch, sizeof(unsigned char), 1, fd);
            for (i = 0; i < 8; i++)
            {
                ch = (unsigned char)(i + 1);
                fwrite(&ch, sizeof(unsigned char), 1, fd);
            }
        }
        for (i = 0; i < 256; i++)
        {
            ch = (unsigned char)(i);
            fwrite(&ch, sizeof(unsigned char), 1, fd);
        }
        fclose(fd);
    }
    else if (argc == 2)
    {
        if ((fd = fopen(argv[1], "rb")) == NULL)
        {
            fprintf(stderr, "could not open input file \"%s\"\n", argv[1]);
            return -1;
        }
        for (k = 0; fread(&ch, sizeof(unsigned char), 1, fd) == 1; k++)
        {
            if ((k % 16) == 0) printf("\n ");
            else if ((k % 8) == 0) printf("  ");
            printf(" %3u", (unsigned int)ch);
        }
        printf("\n\n");
    }
    else
    {
        fprintf(stderr, "Usage: check [ in_file_name ]\n");
        return -1;
    }
    return 0;
} /* main */

/******************************************************************************/
