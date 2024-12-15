// syfre.c -- main() for syfre.
//
// written by Marc Wallace, 01 May 93.
// last change: version 1.0 (08 May 93).


#include "include.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static int input  = 0;   // input stream
static int output = 1;   // output stream

static int order[32];    // which cyphers to apply when

static Data d1(1024);
static Data d2(1024);


void
error(char* s1, char* s2)
    {
    fprintf(stderr, "syfre: error: %s", s1);
    if (*s2)
        fprintf(stderr, " '%s'.\n", s2);
    else
        fprintf(stderr, ".\n");
    fflush(stderr);
    exit(1);
    }


int
encode(int baby, int text)
    {
    int last, size, numpass, want;
    int r, v;
    unsigned char data[4];

    // how much data do we need to read for this block?
    // depends on number of passes (16 or 32) and
    // the text flag.
    numpass = baby ? 16 : 32;
    want = (text ? 748 : 1020) - 4 * numpass;

    // we might be reading from stdin.
    // so read() might return, say, 8 bytes.  this ain't enough.
    // read in blocks of data 'til we have a large block or an EOF.
    // '(last < 1)' keeps track of EOF condition.
    if ((size = read(input, d1.data, want)) > 0)
        {
        last = 1;
        while ((last > 0) && (size < want))
            {
            last = read(input, d1.data + size, want - size);
            if (last > 0)
                size += last;   
            }
        if (last < 1)
            last = 0;
        }
    else
        last = 0;

    if (size < 1)
        return 0;  // not much to do here

    // pad out the input
    for (r = size; r < want; r++)
        d1.data[r] = randc();
    d1.size = want;

    // rev up the cypher machine
    v = verlimit[VERSION];
    for (r = 0; r < numpass;)
        {
        (cypherlist[order[r++] % v]->encode)(d1, d2);
        (cypherlist[order[r++] % v]->encode)(d2, d1);
        }

    // convert to text, if necessary
    if (text)
        {
        // need to pad from 748 to 750
        d1.data[d1.size++] = randc();
        d1.data[d1.size++] = randc();
        bin2txt(d1);
        }

    // create the header for the size, version number, and text flag
    data[0] = (text ? 'T' : 'B') + (baby ? 32 : 0);
    data[1] = 'A' + (size % 32);
    data[2] = 'A' + (size / 32);
    data[3] = 'A' + VERSION;

    if (write(output, data, 4) < 4)
        error("could not write output.", "");
    if (write(output, d1.data, d1.size) < d1.size)
        error("could not write output.", "");

    return last;
    }


int
decode()
    {
    int baby, text;
    int size, vers, numpass;
    int r, v;
    unsigned char data[4];

    // read in the first four bytes
    if ((r = read(input, data, 4)) < 1)
        return 0;

    // parse out the format, size, and version number
    baby = (data[0] > 'Z') ? 1 : 0;
    text = ((data[0] == 'T') || (data[0] == 't')) ? 1 : 0;
    size = data[1] - 'A' + 32 * (data[2] - 'A');
    vers = data[3] - 'A';
    numpass = (baby) ? 16 : 32;

    // read in the data and convert if necessary
    if (read(input, d1.data, 1020) != 1020)
        error("input is not a syfre file.", "");
    d1.size = 1020;
    if (text)
        {
        txt2bin(d1);
        // cut off the padding bytes
        d1.size = 748;
        }

    // start up ye old cypher machine
    v = verlimit[vers];
    for (r = numpass - 1; r > 0;)
        {
        (cypherlist[order[r--] % v]->decode)(d1, d2);
        (cypherlist[order[r--] % v]->decode)(d2, d1);
        }

    // output it all
    write(output, d1.data, size);

    return 1;
    }


int
main(int argc, char** argv)
    {
    int  size;      // size of current data segment
    int  code = 0;  // encoding = 0, decoding = 1
    int  baby = 0;  // do less cyphering
    int  text = 0;  // output only text, not binary

    char  passwd[17];
    int   r, s, t;
    char  **p;

    *passwd = 0;


    // parse the argc/argv structure
    p = argv;
    while(*++p)
       {
       if (strcmp("-h", *p) == 0)
           {
           printf("%s [-edbt] [-i inputfile] [-oO outputfile]\n", *argv);
           exit(0);
           }
       else if (strcmp("-e", *p) == 0)
           code = 0;
       else if (strcmp("-d", *p) == 0)
           code = 1;
       else if (strcmp("-b", *p) == 0)
           baby = 1;
       else if (strcmp("-t", *p) == 0)
           text = 1;
       else if (strcmp("-i", *p) == 0)
           {
           if ((input = open(*++p, O_RDONLY, 0)) == -1)
               error("cannot open input file", *p);
           }
       else if (strcmp("-o", *p) == 0)
           {
           if ((output = open(*++p, O_WRONLY, 0)) != -1)
               error("will not clobber existing file", *p);
           if ((output = creat(*p, 0600)) == -1)
               error("cannot open output file", *p);
           }
       else if (strcmp("-O", *p) == 0)
           {
           if ((output = creat(*p, 0600)) == -1)
               error("cannot open output file", *p);
           }
       else
           error("bad argument:", *p);
       }

    // read in the password (two iterations)
    if (!*passwd)
        {
        sprintf(passwd, "default password");
        sprintf(passwd,   "%-8.8s", getpass("Password: "));
        sprintf(passwd+8, "%-8.8s", getpass("Password: "));
        }

    // fill the order[] array
    // these are just raw integers: to get the
    // actual cypher number to apply, take the
    // entry modulo (number of cyphers in this version)
    // (version number may change for decodes, eh?)
    for (r = 0; r < 16; r++)
        for (s = 0; s < 2; s++)
            {
            t = passwd[r] + r + 5 * s;
            t += passwd[(3 * s + 2 * r) % 16];
            t -= (s % 3) ? 17 : 0;
            order[(r << 1) + s] = t + 1024;
            }

    // now repeatedly encode/decode block by block
    t = 1;
    while(t)
        t = (code) ? decode() : encode(baby, text);
    }

