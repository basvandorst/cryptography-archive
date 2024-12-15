#include <stdio.h>
#include "shade.h"

/* Multiplication of 2 4-bit poly */
UBYTE mul4_tab[256];
void init_mul4_tab()
{
    UWORD i;
    mul4_tab[0] = 0;
    for (i=1; i<256; i++)
        if (i&1)       mul4_tab[i] = mul4_tab[i-1] ^ (i>>4);
        else if (i&16) mul4_tab[i] = mul4_tab[i-16] ^ (i&15);
        else           mul4_tab[i] = mul4_tab[i>>1]<<2;
}

UBYTE shft_tab[256];
UBYTE inv8_tab[256];
void init_inv8_tab()
{
    UWORD x, xx, y, i;
    for (x=1; x<256; x++) {
        i=0; xx = x;
        while (xx) { xx >>= 1; i++; }
        shft_tab[x] = 9-i;
    }
    for (x=0; x<256; x++) {
        y = x<<8;
        xx = y|1;
        for (i=8;i;i--)
            if (y&0x8000)
                y = (y<<1)^xx;
            else
                y = (y<<1);
        inv8_tab[x] = y;
    }
}


main()
{
    FILE *fd;
    UWORD i;
    init_mul4_tab();
    init_inv8_tab();
    fd = fopen( "table.c", "w");
    if (!fd) {
        puts( "ERROR: can't open table.h for output.");
        exit( 10);
    }
    fprintf( fd, "/* table.h\n *\n");
    fprintf( fd, " * tables for multiplication and division of polynomials\n */\n");
    fprintf( fd, "#include <stdio.h>\n\n");
    fprintf( fd, "#include \"shade.h\"\n\n");

    fprintf( fd, "UBYTE mul4_tab[257] = {\n%3d", mul4_tab[0]);
    for (i=1; i<256; i++)
        fprintf( fd, ",%c%3d", (i&0xF)?' ':'\n', mul4_tab[i]);
    fprintf( fd, "};\n\n");

    fprintf( fd, "UBYTE shft_tab[256] = {\n%3d", shft_tab[0]);
    for (i=1; i<256; i++)
        fprintf( fd, ",%c%3d", (i&0xF)?' ':'\n', shft_tab[i]);
    fprintf( fd, "};\n\n");

    fprintf( fd, "UBYTE inv8_tab[257] = {\n%3d", inv8_tab[0]);
    for (i=1; i<256; i++)
        fprintf( fd, ",%c%3d", (i&0xF)?' ':'\n', inv8_tab[i]);
    fprintf( fd, "};\n\n");

    fclose( fd);
    return 0;
}

