#include <stdio.h>
#include "shade.h"

/*
 * This is the standard encoding used to convert binary files
 * to printable characters,  It expands the data by 38%, storing
 * 6 bits per output character and adding 2 more characters to
 * each output line of 45 input bytes.
 *
 * To avoid problems with the sign bit when shifting, we use
 * the previously defined UBYTE type for the binary bytes
 * but use char for the uuencoded bytes.
 */

/*
 * UUencoding a line. The output buffer should be of size
 * uu_size(n)+1 where n is the number of bytes you want to
 * stuff on the line. n is normally 45 and uu_size(45) ==
 * 62.
 */
#define uue(c) ((c)?(c)+' ':'`')
#define uuee(out,v) c = (v); (out) = uue(c);
void uue_line( char *out, UBYTE *in, ULONG len)
{
    int i; UBYTE c;
    uuee( *out++, len)
    for (i=0;i<len;i+=3) {
        uuee( *out++,                     ((*in)>>2))
        uuee( *out++, (((*in++)& 3)<<4) + ((*in)>>4))
        uuee( *out++, (((*in++)&15)<<2) + ((*in)>>6))
        uuee( *out++, ( (*in++)&63    )             )
    }
    *out++ = '\n';
    *out++ = '\0';
}

/*
 * UUencoding a block of data.  A block of len binary bytes
 * is read from the buffer in.  As many uuencoded lines are
 * output to the file out as needed.  No "begin" of "end"
 * line is output.  The last line is not empty.
 */
void uue_block( FILE *out, UBYTE *in, ULONG len)
{
    int i; UBYTE bin[45]; char bout[uu_size(45)+1];
    while (len > 45) {
        memcpy( bin, in, 45);
        uue_line( bout, bin, 45);
        fputs( bout, out);
        len -= 45; in += 45;
    }
    memcpy( bin, in, len);
    uue_line( bout, bin, len);
    fputs( bout, out);
}

/*
 * Decoding a uuencoded line.  Two functions are provided to
 * facilitate the decoding of uuencoded lines: uu_valid()
 * which returns 1 when the line is a valid uuencoded line and
 * 0 otherwise; and uud_line() which actually decodes a
 * valid line, there is no telling what it will do if the line
 * is invalid.
 */
#define uud(c) (((c)-' ')&63)
int uu_valid( char *line)
{
    UBYTE len = uud(*line); int i;
    if (len > 45) return 0;
    if (strlen(line) < uu_size(len) - 1) return 0;
    if (strlen(line) > uu_size(len) + 1) return 0;
    for (i=0;i<uu_size(len)-1;i++)
        if (line[i] < ' ' || line[i] > '`') return 0;
    return 1;
}

ULONG uud_line( UBYTE *out, char *in)
{
    ULONG len; int i;
    len = uud(*in++);
    for (i=0;i<len;i+=3) {
        UBYTE a, b;
        a = uud(*in++);
        b = uud(*in++); *out++ = ( a    <<2) + (b>>4);
        a = uud(*in++); *out++ = ((b&15)<<4) + (a>>2);
        b = uud(*in++); *out++ = ((a& 3)<<6) + (b   );
    }
    return len;
}

/*
 * UUdecoding a block of data directly from the file in
 * into the buffer out.  Data is read into the buffer until
 * a line containing only "end" is reached, an invalid line
 * is found or the buffer is full.  The function
 * uud_pending() returns the number of uudecoded bytes from
 * the last input line that have not been returned yet by
 * uud_block().
 */
int uud_mem_len = 0;
UBYTE uud_mem_data[60];

int uud_pending()
{
    return uud_mem_len;
}

void uud_drop()
{
    uud_mem_len = 0;
}

ULONG uud_block( UBYTE *out, ULONG len, FILE *in)
{
    ULONG l=len; char bin[uu_size(60)+2];

    bin[uu_size(60)+1] = '\0';
    while (l) {
        if (uud_mem_len >= l) {
            memcpy( out, uud_mem_data, l);
            uud_mem_len -= l;
            memcpy( uud_mem_data, uud_mem_data+l, uud_mem_len);
            return len;
        }
        memcpy( out, uud_mem_data, uud_mem_len);
        out += uud_mem_len; l -= uud_mem_len; uud_mem_len = 0;
        if (!fgets( bin, uu_size(60)+1, in)) break;
        if (!uu_valid( bin)) break;
        uud_mem_len = uud_line( uud_mem_data, bin);
    }
    return len-l;
}

