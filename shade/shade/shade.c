#include <string.h>
#include <stdio.h>
#include "shade.h"

#define M_UU    0x01    /* parts are uuencoded (implies M_ID) */
#define M_SHD   0x02    /* There are shadows */
#define M_ID    0x04    /* each part and shade gets a header */
#define M_MRG   0x08    /* merge, don't split */

struct {
    char *filename, *prefix;
    UWORD mode;
    int n, k;
    ULONG b, l;
} parg;

struct {
    char *filename, *prefix;
    UWORD mode;
    int n, k;
    ULONG s, b, l;
    UBYTE **shades;
    UWORD *base;
    FILE *fd;
} pglob;

struct {
    ULONG s, l; int n;
} pstart;

struct {
    int i; ULONG crc;
} ppart;

map_t map;
char buffer[80];
ULONG crc_tab[256];


void usage()
{
    puts( "shade version 1.00 13-dec-92 (c) Philippe Hebrais 1992");
    puts( "shade [-m|u|i] [-n sections] [-k shades]");
    puts( "      [-l len] [-b block] [-o prefix] [-] filename\n");
    puts( " opt default  : Description.");
    puts( "  -m off      : Merge parts from stdin (implies '-i').");
    puts( "  -u off      : Uuencode(d) parts (implies '-i -b 45').");
    puts( "  -i off      : Identify each part.");
    puts( "  -n 0        : Suggested number of parts to create.");
    puts( "  -k 0        : Number of shades to create (implies '-b 2').");
    puts( "  -l 45000    : Maximium length of parts to create.");
    puts( "  -b 1        : Length of parts should be multiples of this.");
    puts( "                Note: '-b 2 -b 45' is the same as '-b 90'.");
    puts( "  -o filename : Output file name. Output parts are named");
    puts( "                'filename.nnn'.");
    puts( "  -           : Separates options from the filename.");

    exit( 1);
}

long int fsize( FILE *fd)
{
    long int size, tell = ftell( fd);
    if (fseek( fd, 0, 2)) return 0;
    size = ftell( fd);
    fseek( fd, tell, 0);
    return size;
}


ULONG lgcd( ULONG x, ULONG y)
{
    ULONG z;
    for (;;) {
        if (!y) return x;
        z = x % y;
        if (!z) return y;
        x = y % z;
        if (!x) return z;
        y = z % x;
    }
}

void init_crc()
{
    UWORD x; ULONG y, z;
    for (x=0;x<256;x++) {
        y = mul16( x, CRC_POLY);
        z = mul16( x, CRC_POLY>>16);
        y ^= (z << 16);
        z = (z>>16) ^ x;
        crc_tab[(UBYTE)z] = y;
    }
}

ULONG crc( UBYTE *buff, ULONG len)
{
    ULONG r = 0;
    for (; len; len--,buff++)
        r = (r<<8) ^ *buff ^ crc_tab[r>>24];
    r = (r<<8) ^ crc_tab[r>>24];
    r = (r<<8) ^ crc_tab[r>>24];
    r = (r<<8) ^ crc_tab[r>>24];
    r = (r<<8) ^ crc_tab[r>>24];
    return r;
}

void init_args( int argc, char **argv)
{
    ULONG t;

    parg.filename = parg.prefix = NULL;
    parg.mode = 0;
    parg.n = parg.k = 0;
    parg.b = 1; parg.l = 0;

    for (argc--,argv++; argc; argc--,argv++) {
        if (argv[0][0] != '-') break;
        if (argv[0][1] == 0) { argc--; argv++; break; }
        if (argv[0][2] != 0) usage();
        switch (argv[0][1]) {
        case 'm': parg.mode |= M_MRG | M_ID;
                  break;
        case 'u': parg.mode |= M_UU;
                  parg.b /= lgcd( parg.b, 45);
                  parg.b *= 45;
                  /* fall through */
        case 'i': parg.mode |= M_ID;
                  break;
        case 'n': argc--; argv++;
                  parg.n = strtol( argv[0], NULL, 0);
                  break;
        case 'k': argc--; argv++;
                  parg.k = strtol( argv[0], NULL, 0);
                  parg.b /= lgcd( parg.b, 2);
                  parg.b *= 2;
                  parg.mode |= M_SHD;
                  break;
        case 'l': argc--; argv++;
                  parg.l = strtol( argv[0], NULL, 0);
                  break;
        case 'b': argc--; argv++;
                  t = strtol( argv[0], NULL, 0);
                  parg.b /= lgcd( parg.b, t);
                  parg.b *= t;
                  break;
        case 'o': argc--; argv++;
                  parg.prefix = argv[0];
                  break;
        default: usage();
        }
    }
    if (argc) parg.filename = argv[0];
}


void split_init_map()
{
    FILE *fd; char line[40]; int i;
    pglob.base = (UWORD*) malloc( (pglob.n+pglob.k) * sizeof(UWORD));
    pglob.shades = (UBYTE**) malloc( pglob.k * sizeof(UBYTE*));
    if (!pglob.base || !pglob.shades) {
        fprintf( stderr, "Out of memory.\n");
        pglob.mode &= ~M_SHD;
        return;
    }
    for (i=0; i<pglob.k; i++) {
        pglob.shades[i] = (UBYTE*) malloc( pglob.l * sizeof(UBYTE));
        if (!pglob.shades[i]) {
            fprintf( stderr, "Out of memory.\n");
            pglob.mode &= ~M_SHD;
            return;
        }
        memset( pglob.shades[i], 0, pglob.l);
    }

    fd = fopen( PRIME_FILE, "r");
    if (!fd) {
        fprintf( stderr, "%s: can't open for input.\n", PRIME_FILE);
        pglob.mode &= ~M_SHD;
        return;
    }

    for (line[39]=0,i=0; i<(pglob.n+pglob.k); i++) {
        do if (!fgets( line, 39, fd)) {
            fprintf( stderr, "%s: error while reading.\n", PRIME_FILE);
        } while (!(pglob.base[i] = strtol( line, NULL, 0)));
    }

    map_init( &map, pglob.n, pglob.k, pglob.base);
    fclose( fd);
}


void split_section( int i, UBYTE *buff, ULONG len)
{
    char fn[128]; FILE *fd;

    sprintf( fn, "%s%s.%03d", pglob.prefix, (pglob.mode&M_UU)?".uu":"", i);
    fd = fopen( fn, "w");
    if (!fd) {
        fprintf( stderr, "%s: can't open for output.\n", fn);
        exit( 10);
    }
    if (pglob.mode & M_ID) {
        fprintf( fd, "start %s %d %d %d\n", pglob.filename, pglob.s, pglob.n, pglob.l);
        fprintf( fd, "section %d %08X\n", i, crc( buff, len));
    }
    if (!(pglob.mode & M_UU)) {
        fwrite( buff, len, 1, fd);
    } else {
        fprintf( fd, "begin 666 %s.%03d\n", pglob.filename, i);
        uue_block( fd, buff, len);
        fprintf( fd, "`\nend\n");
    }
    fclose( fd);
    fprintf( stderr, "[%03d]%c", i, (i%10)?' ':'\n'); fflush( stderr);
}


void split_shadow( int i, UBYTE *buff, ULONG len)
{
    char fn[128]; FILE *fd;

    sprintf( fn, "%s%s.%c%c%c", pglob.prefix, (pglob.mode&M_UU)?".uu":"", 'a'+i/26/26, 'a'+(i/26)%26, 'a'+i%26);
    fd = fopen( fn, "w");
    if (!fd) {
        fprintf( stderr, "%s: can't open for output.\n", fn);
        exit( 10);
    }
    map.b[pglob.n] = map.b[pglob.n+i];
    fprintf( fd, "start %s %d %d %d\n", pglob.filename, pglob.s, pglob.n, pglob.l);
    fprintf( fd, "shadow %08X\n", crc( buff, len));
    if (!(pglob.mode & M_UU)) {
        fwrite( map.b, (pglob.n+1)*sizeof(UWORD), 1, fd);
        fwrite( buff, len, 1, fd);
    } else {
        uue_block( fd, (UBYTE*)map.b, (pglob.n+1)*sizeof(UWORD));
        fprintf( fd, "begin 666 %s.%c%c%c\n", pglob.filename, 'a'+i/26/26, 'a'+(i/26)%26, 'a'+i%26);
        uue_block( fd, buff, len);
        fprintf( fd, "`\nend\n");
    }
    fclose( fd);
    fprintf( stderr, "[%c%c%c]%c", 'a'+i/26/26, 'a'+(i/26)%26, 'a'+i%26, ((i+pglob.n+1)%10)?' ':'\n'); fflush( stderr);
}


void split()
{
    ULONG i, len;
    UBYTE *buff;
    if (!parg.filename) {
        fprintf( stderr, "No file to split.\n");
        usage();
    }
    pglob.fd = fopen( parg.filename, "r");
    if (!pglob.fd) {
        fprintf( stderr, "%s: can't open.\n", parg.filename);
        return;
    }
    pglob.filename = parg.filename;
    pglob.prefix = parg.prefix?parg.prefix:parg.filename;
    pglob.mode = parg.mode;
    pglob.n = parg.n; pglob.k = parg.k;
    pglob.s = fsize( pglob.fd); pglob.b = parg.b;
    pglob.l = parg.l?parg.l:45000;
    pglob.shades = NULL; pglob.base = NULL;

    pglob.l -= pglob.l % pglob.b;
    if (!pglob.n) pglob.n = (pglob.s-1) / pglob.l + 1;
    pglob.l = (pglob.s-1) / pglob.n;
    pglob.l += pglob.b - (pglob.l % pglob.b);
    pglob.n = (pglob.s-1) / pglob.l + 1;

    buff = (UBYTE*) malloc( pglob.l * sizeof(UBYTE));
    if (!buff) {
        fprintf( stderr, "Out of memory!\n");
        fclose( pglob.fd);
        return;
    }
    if (pglob.mode&M_SHD) split_init_map();

    for (i=0,len=pglob.s; len>pglob.l; i++,len-=pglob.l) {
        fread( buff, pglob.l, 1, pglob.fd);
        if (pglob.mode&M_SHD) map_block( &map, (UWORD**)pglob.shades, i, (UWORD*)buff, pglob.l);
        split_section( i+1, buff, pglob.l);
    }
    fread( buff, len, 1, pglob.fd);
    if (len<pglob.l) memset( buff+len, 0, pglob.l-len);
    if (pglob.mode&M_SHD) map_block( &map, (UWORD**)pglob.shades, i, (UWORD*)buff, pglob.l);
    split_section( i+1, buff, len);

    if (pglob.mode&M_SHD) {
        for (i=0; i<pglob.k; i++)
            split_shadow( i, pglob.shades[i], pglob.l);
        map_free( &map);
    }

    fprintf( stderr, "Done.\n");
    if (pglob.fd) fclose( pglob.fd);
    if (pglob.base) free( pglob.base);
    if (pglob.shades) {
        for (i=0; i<pglob.k; i++) if (pglob.shades[i]) free( pglob.shades[i]);
        free( pglob.shades);
    }
}


int merge_start( char *line)
{
    if (sscanf( line, "start %s %ld %d %ld", buffer, &pstart.s, &pstart.n, &pstart.l) != 4) return 0;
    if (  (pglob.filename && strcmp( pglob.filename, buffer))
       || (pglob.s && pglob.s != pstart.s)
       || (pglob.n && pglob.n != pstart.n)
       || (pglob.l && pglob.l != pstart.l)) {
       fprintf( stderr, "Rejected: %s", line);
       return 0;
    }
    if (!pglob.filename) pglob.filename = pglob.prefix = strdup( buffer);
    if (!pglob.s) fprintf( stderr, "Merging %s (%d bytes)\n", buffer, pstart.s);
    pglob.s = pstart.s;
    pglob.n = pstart.n;
    pglob.l = pstart.l;
    return 1;
}

int merge_section( char *line)
{
    UBYTE *buff; ULONG l;
    if (sscanf( line, "section %d %lX", &ppart.i, &ppart.crc) != 2) return 0;
    if (!ppart.i || ppart.i > pglob.n) {
        fprintf( stderr, "Invalid part number: %s", line);
        return 1;
    }
    if (pglob.shades && pglob.shades[ppart.i-1]) {
        fprintf( stderr, "Duplicate: %s", line);
        return 1;
    }
    buff = (UBYTE*) malloc( pglob.l * sizeof(UBYTE));
    if (!buff) {
        fprintf( stderr, "Out of memory!\n");
        return 1;
    }
    l = (ppart.i!=pglob.n)?pglob.l:pglob.s-(pglob.n-1)*pglob.l;
    if (pglob.mode&M_UU) {
        if (!fgets( buffer, 79, stdin) || strncmp( buffer, "begin ", 6)) {
            fprintf( stderr, "No begin line: %s", line);
            free( buff); return 1;
        }
        if (l != uud_block( buff, l, stdin) || uud_pending()) {
            fprintf( stderr, "Bad body: %s", line);
            uud_drop(); free( buff); return 1;
        }
    } else {
        if (!fread( buff, l, 1, stdin)) {
            fprintf( stderr, "Bad body: %s", line);
            free( buff); return 1;
        }
    }
    if (ppart.crc != crc( buff, l)) {
        fprintf( stderr, "Bad crc: %s", line);
        free( buff); return 1;
    }
    if (l != pglob.l) memset( buff+l, 0, pglob.l-l);
    if (!pglob.shades) {
        pglob.shades = (UBYTE**) malloc( 3 * pglob.n * sizeof(UBYTE*));
        if (!pglob.shades) {
            fprintf( stderr, "Out of memory.\n");
            free( buff); return 1;
        }
        for (l=0; l<3*pglob.n; l++) pglob.shades[l] = NULL;
    }
    pglob.shades[ppart.i-1] = buff;
    l = (ppart.i!=pglob.n)?pglob.l:pglob.s-(pglob.n-1)*pglob.l;
    fprintf( stderr, "got section %d (%d bytes)\n", ppart.i, l);
    return 1;
}


int merge_shadow( char *line)
{
    UBYTE *buff; UWORD *base; ULONG l;
    if (sscanf( line, "shadow %lX", &ppart.crc) != 1) return 0;
    ppart.i = pglob.n + pglob.k + 1;
    base = (UWORD*) malloc( (pglob.n+1) * sizeof(UWORD));
    if (!base) {
        fprintf( stderr, "Out of memory!\n");
        return 1;
    }

    l = (pglob.n+1) * sizeof(UWORD);
    if (pglob.mode&M_UU) {
        if (l != uud_block( (UBYTE*)base, l, stdin) || uud_pending()) {
            fprintf( stderr, "Bad base: %s", line);
            uud_drop(); free( base); return 1;
        }
    } else {
        if (!fread( base, l, 1, stdin)) {
            fprintf( stderr, "Bad base: %s", line);
            free( base); return 1;
        }
    }

    if (pglob.base) {
        for (l=0; l<pglob.n; l++) if (pglob.base[l] != base[l]) {
            fprintf( stderr, "Bad base: %s", line);
            free( base); return 1;
        }
        for (; l<pglob.n+pglob.k; l++) if (pglob.base[l] == base[pglob.n]) {
            fprintf( stderr, "Bad base: %s", line);
            free( base); return 1;
        }
    }

    buff = (UBYTE*) malloc( pglob.l * sizeof(UBYTE));
    if (!buff) {
        fprintf( stderr, "Out of memory!\n");
        free( base); return 1;
    }

    l = pglob.l;
    if (pglob.mode&M_UU) {
        if (!fgets( buffer, 79, stdin)) return 1;
        if (strncmp( buffer, "begin ", 6)) {
            fprintf( stderr, "No begin line: %s", line);
            free( buff); free( base); return 1;
        }
        if (l != uud_block( buff, l, stdin) || uud_pending()) {
            fprintf( stderr, "Bad body: %s", line);
            uud_drop(); free( buff); free( base); return 1;
        }
    } else {
        if (!fread( buff, l, 1, stdin)) {
            fprintf( stderr, "Bad body: %s", line);
            free( buff); free( base); return 1;
        }
    }
    if (ppart.crc != crc( buff, l)) {
        fprintf( stderr, "Bad crc: %s", line);
        free( buff); return 1;
    }

    if (!pglob.base) {
        pglob.base = (UWORD*) malloc( 2 * pglob.n * sizeof(UWORD));
        if (!pglob.shades) {
            fprintf( stderr, "Out of memory.\n");
            free( buff); free( base); return 1;
        }
        memcpy( pglob.base, base, pglob.n * sizeof(UWORD));
    }

    if (!pglob.shades) {
        pglob.shades = (UBYTE**) malloc( 3 * pglob.n * sizeof(UBYTE*));
        if (!pglob.shades) {
            fprintf( stderr, "Out of memory.\n");
            free( buff); free( base); return 1;
        }
        for (l=0; l<3*pglob.n; l++) pglob.shades[l] = NULL;
    }

    pglob.k += 1;
    pglob.base[ppart.i-1] = base[pglob.n];
    pglob.shades[ppart.i-1] = buff;
    fprintf( stderr, "got shadow %d (%d bytes)\n", pglob.k, pglob.l);
    free( base); return 1;
}


int merge_recover()
{
    ULONG t, k;
    map.n = map.k = 0;

    for (t=0; t<pglob.n; t++) {
        if (pglob.shades && pglob.shades[t]) map.n++;
        else {
            if (pglob.base) pglob.base[t] ^= pglob.base[pglob.n+map.k] ^= pglob.base[t] ^= pglob.base[pglob.n+map.k];
            if (pglob.shades) {
                pglob.shades[2*pglob.n+map.k] = (UBYTE*) malloc( pglob.l * sizeof(UBYTE));
                memset( pglob.shades[2*pglob.n+map.k], 0, pglob.l);
            }
            if (!map.k) fprintf( stderr, "\nMissing: ");
            map.k++;
            fprintf( stderr, "[%03d]%c", t+1, ((t+1)%10)?' ':'\n'); fflush( stderr);
        }
    }
    if (map.k) fprintf( stderr, "\n");

    if (map.k > pglob.k) {
        fprintf( stderr, "\nNeed %d more parts/shadows to recover\n", map.k - pglob.k);
        return 0;
    }
    if (map.k) {
        fprintf( stderr, "\nRecovering %d sections:\n", map.k);
        map_init( &map, pglob.n, map.k, pglob.base);
        for (t=0,k=pglob.n; t<pglob.n; t++) {
            if (pglob.shades[t]) {
                map_block( &map, (UWORD**)pglob.shades+2*pglob.n, t, (UWORD*)pglob.shades[t], pglob.l);
                fprintf( stderr, "[%03d]%c", t+1, ((t+1)%10)?' ':'\n'); fflush( stderr);
            } else {
                map_block( &map, (UWORD**)pglob.shades+2*pglob.n, t, (UWORD*)pglob.shades[k], pglob.l);
                fprintf( stderr, "[%c%c%c]%c", 'a'+(k-pglob.n)/26/26, 'a'+((k-pglob.n)/26)%26, 'a'+(k-pglob.n)%26, ((t+1)%10)?' ':'\n'); fflush( stderr);
                k++;
            }
        }
        for (t=0,k=2*pglob.n; t<pglob.n; t++) {
            if (pglob.shades[t]) continue;
            pglob.shades[t] = pglob.shades[k];
            pglob.shades[k++] = NULL;
        }
        map_free( &map);
    }
    fprintf( stderr, "\n");
    return 1;
}

void merge_output()
{
    ULONG t;
    if (!pglob.fd) pglob.fd = fopen( parg.prefix?parg.prefix:pglob.filename, "w");
    for (t=0; t<pglob.n-1; t++)
        fwrite( pglob.shades[t], pglob.l, 1, pglob.fd);
    fwrite( pglob.shades[pglob.n-1], pglob.s-(pglob.n-1)*pglob.l, 1, pglob.fd);
}


void merge()
{
    char line[80]; ULONG t;

    pglob.filename = parg.filename;
    pglob.prefix = NULL;
    pglob.mode = parg.mode;
    pglob.n = parg.n; pglob.k = 0;
    pglob.s = 0; pglob.l = parg.l;
    pglob.shades = NULL;
    pglob.base = NULL;
    pglob.fd = NULL;

    line[79] = 0;
    for (;;) {
        if (!fgets( line, 79, stdin)) break;
        if (!merge_start( line)) continue;
        if (!fgets( line, 79, stdin)) break;
        if (merge_section( line)) continue;
        if (merge_shadow( line)) continue;
        fprintf( stderr, "Invalid part: %s", line);
    }

    if (merge_recover()) merge_output();
    if (pglob.fd) fclose( pglob.fd);
    if (pglob.prefix) free( pglob.prefix);
    if (pglob.base)   free( pglob.base);
    if (pglob.shades) {
        for (t=0;t<3*pglob.n;t++)
            if (pglob.shades[t])
                free( pglob.shades[t]);
        free( pglob.shades);
    }
}


int main( argc, argv)
    int argc;
    char **argv;
{
    buffer[79] = 0;
    init_args( argc, argv);
    init_crc();
    if (parg.mode&M_MRG)
        merge();
    else
        split();
    return 0;
}

