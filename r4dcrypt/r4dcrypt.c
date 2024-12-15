/*********************************************************
*   r4dcrypt version 1.0
**********************************************************
*
*   purpose:  Deciphers files encrypted with the utility
*             crypt.com version 3.4, which is part of the
*             Reflection 4 software package.  Reflection 4
*             is widely used communications and terminal
*             emulation software sold by Walker Richer and
*             Quinn, Inc.
*
*   input  :  Any file enciphered using crypt.com
*             Usually, this is an RCL script file.
*
*   output :  File r4dcrypt.dat contains plaintext.
*             Password (if used) is reported.
*
*   written:  June 25, 1994.
*
**********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <process.h>
#include <string.h>

#define CYCLE_LENGTH 32
#define MAXLENGTH 30
#define FILE_ID 0x0200001aL
#define CIRCSHFT(a,n) ((a)=(((a)<<(8-(n))) & mask[n]) | ((a)>>(n))) ; 

int main( void )
{
    unsigned long   id=0L, nbytes=0L ;
    unsigned short  mask[] =  { 0xff, 0x80, 0xc0, 0xe0,
                                0xf0, 0xf8, 0xfc, 0xfe} ; 
    unsigned short  temp, shift, lngthpasswrd, offset, i ;
    int             nread, in ;
    unsigned char   inbyte, szPassword[MAXLENGTH] ;
    char            szNamein[256] ;
    char *          szNameout = "r4dcrypt.dat" ;
    FILE *          hFileIn ;
    FILE *          hFileOut ;
/*
 * The following sequence of circular shifts is used by crypt.com:
 */
    unsigned short  shift_seq[] =   {3,6,7,5,
                                     4,5,6,2,
                                     6,6,5,0,
                                     3,7,7,6,
                                     5,4,1,2,
                                     6,1,3,4,
                                     7,4,1,5,
                                     6,7,0,7} ;

    printf("Enter the name of a file to be deciphered:");
    scanf("%s", szNamein);
    if( (hFileIn = fopen( (char *) szNamein, "rb" )) == NULL )
    {
        printf("File could not be opened.\n");
        return 1;
    }
/*
 * first four bytes from header of ciphertext file confirm valid ID 
 */
    nread = (int) fread( (char *) &id, sizeof(long), 1, hFileIn );
    if(id != FILE_ID)
    {
        printf("That file was not encrypted by crypt.com version 3.4\n"); 
        fclose(hFileIn);
        return 0;
    }
    printf("Creating plaintext file: %s\n", szNameout);     
    if( (hFileOut = fopen( (char *) szNameout, "wb" )) == NULL )
    {
        /* r4dcrypt.dat already exists and is read-only */
        printf("Output file could not be opened.\n");
        fclose(hFileIn);
        return 1;
    }
/*
 * The fifth byte determines the initial offset in the shift sequence.
 */
    nread = (int) fread( (char *) &inbyte, 1, 1, hFileIn );
    offset = (258 - (unsigned short) inbyte) % CYCLE_LENGTH ;
/*
 * Remaining bytes of header contain a password (if used) or nulls:
 */
    lngthpasswrd = 0 ;
    for (i=0; i<MAXLENGTH; i++)
    {
        if((in = getc(hFileIn)) == EOF)
        {
            fclose(hFileIn);
            fclose(hFileOut);
            return 1;
        }
        if((temp = (unsigned short) in) != 0)
        {
            shift = shift_seq[(++lngthpasswrd + MAXLENGTH) % CYCLE_LENGTH];
            /* circular right shift within the low-order byte in temp */
            szPassword[lngthpasswrd-1] = (unsigned char) CIRCSHFT(temp, shift) ; 
        }
    }
/* display the deciphered password, if one was found */
    szPassword[lngthpasswrd] = '\0' ;
    if(lngthpasswrd != 0)
        printf("A password was found: %s\n", szPassword);
    else
        printf("No password was found.\n");    
/*
 * Finished with header -- reset the shift count 
 * and decipher the rest of the file.
 */
    while ((in = getc(hFileIn)) != EOF)
    {
        shift = shift_seq[(++nbytes + CYCLE_LENGTH -
                            (unsigned long) offset) % CYCLE_LENGTH] ;
        temp = (unsigned short) in ; 
        temp =  CIRCSHFT(temp, shift) ; 
        if(putc((int) temp, hFileOut) == EOF) break; /* disk full, etc. */
    }
    fclose(hFileIn) ;
    fclose(hFileOut) ;
    return (0);
}
