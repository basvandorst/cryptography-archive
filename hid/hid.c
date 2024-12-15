/* 
 * program to generate a script to change value returned by gethostid(2) 
 * on a Sun 4 running SUN OS 4.1.  Mark Henderson - markh@wimsey.bc.ca
 * I hereby place this file in the public domain.
 * 28 Jan 1994
 * Disclaimer: USE AT YOUR OWN RISK. I make no warranty concerning
 *  the use of this program. 
 * Usage: (as root)
 *  hid febd0023 | sh
 * will change the value returned by gethostid to 0xfebd0023
 * or you can add the generated script to /etc/rc.local
 * to compile
 *  cc -o hid hid.c   -- where hid.c consists of exactly this file
 *
 * don't ask me about doing this for SUN OS 5.x, as I don't have
 *  access to a Sun OS 5.x machine (and I'm really glad I don't !!!).
 */

#include <stdio.h>
main(argc,argv)
int argc;
char *argv[];
{
    unsigned int hid;
    unsigned int hi;
    unsigned int low;

    if (argc != 2) {
        fprintf(stderr, "usage: %s hostid\n",argv[0]);
        exit(1);
    }
    sscanf(argv[1],"%08x",&hid);

    hi = (hid & 0xfffffc00) >> 10;

    printf("#!/bin/sh\n");
    printf("# this script sets the hostid to %08x\n",hid);
    printf("adb -w -k /vmunix /dev/mem <<END\n");
    printf("_gethostid+4/W 11");
    printf("%06x\n", hi);

    low = hid & 0x3ff;
    printf("_gethostid+8/W 90122");
    printf("%03x\n",low);
    printf("END\n");
}
