/* DESTEST.C - validates Data Encryption Standard (DES) Algorithm */

#include <windows.h>
#include <memory.h>
#include <stdio.h>
#include "windes.h"	/* DES function prototypes */

/* size of standard DES block, in bits */
#define	BITSZ	(64)
/* size of standard DES block, in bytes */
#define	BLKSZ	(8)
/* how many standard DES blocks defined in ANSI X9.9 Appendix B */
#define	X99MX	(4)

void main(void)
{
//    static BYTE vecbits[BLKSZ] = { 0x73,0x4c,0x51,0xd9,0x70,0xf2,0x1a,0x6e };
    /* for validation, keybits and patterns taken from ANSI X9.9 Appendix B */
    static BYTE keybits[BLKSZ] = { 0xe6,0xa1,0x2f,0x07,0x9d,0x15,0xc4,0x37 };
    static BYTE pattern[X99MX][BLKSZ] = {
        { 0x0a,0x20,0x20,0x20,0x54,0x4f,0x20,0x59 },
        { 0x51,0x44,0x2d,0x38,0x30,0x20,0x30,0x37 },
        { 0x54,0x4f,0x20,0x59,0x4f,0x55,0x52,0x20 },
        { 0x51,0x44,0x2d,0x38,0x30,0x20,0x30,0x37 }
    };
    BYTE results[X99MX*BLKSZ], clrtext[X99MX*BLKSZ];
    WORD i, j;

    for (i = 0; i < X99MX; i++)
    {
	DesEnCrypt((LPBYTE) keybits,(LPBYTE) pattern[i],(LPBYTE) results, 1);
	DesDeCrypt((LPBYTE) keybits,(LPBYTE) results,(LPBYTE) clrtext, 1);
        printf(
	"ECB %02x%02x%02x%02x%02x%02x%02x%02x -> %02x%02x%02x%02x%02x%02x%02x%02x \
-> %02x%02x%02x%02x%02x%02x%02x%02x\n",
            pattern[i][0], pattern[i][1], pattern[i][2], pattern[i][3],
            pattern[i][4], pattern[i][5], pattern[i][6], pattern[i][7],
            results[0], results[1], results[2], results[3],
            results[4], results[5], results[6], results[7],
            clrtext[0], clrtext[1], clrtext[2], clrtext[3],
            clrtext[4], clrtext[5], clrtext[6], clrtext[7] );
    }
    printf("\n");

} /* main () */

/* end of source */
