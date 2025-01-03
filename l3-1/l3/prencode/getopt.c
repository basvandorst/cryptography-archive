/*--- mygetopt.c -- Routine to crack the command line.
 *
 *  Functionally equivalent to the "mygetopt" that comes with many
 *  Unix systems.  Provided for use on Unix, OS/2, and MS-DOS
 *  systems that do not already have a mygetopt.
 *
 *  I grabbed this uncommented code off uunet.uu.net--I think it's
 *  public domain.   /mrr
 */
/*LINTLIBRARY*/
#include <stdio.h>
#include <string.h>
#include "getoptpr.h"
#ifdef __MSDOS__
#define MSDOS
#endif
#ifdef MSDOS
#include <io.h>
#endif

#if 0
#define ERR(s, c)       if(xx_opterr){\
        char errbuf[2];\
        errbuf[0] = (char)c; errbuf[1] = '\n';\
        (void) write(2, argv[0], strlen(argv[0]));\
        (void) write(2, s, strlen(s));\
          (void) write(2, errbuf, 2);}
#else
#define ERR(s,c)
#endif

int     xx_opterr = 1;
int     xx_optind = 1;
int   optsp = 1;
int     optopt;
char    *optarg;

int
mygetopt(argc, argv, opts)
int     argc;
char    **argv, *opts;
{
        register int c;
        register char *cp;

        if(optsp == 1)
                if(xx_optind >= argc ||
                   argv[xx_optind][0] != '-' || argv[xx_optind][1] == '\0')
                        return(EOF);
                     else if(strcmp(argv[xx_optind], "--") == 0) {
                                xx_optind++;
                        return(EOF);
                }
        optopt = c = argv[xx_optind][optsp];
        if(c == ':' || (cp=strchr(opts, c)) == NULL) {
                ERR(": illegal option -- ", c);
                if(argv[xx_optind][++optsp] == '\0') {
                        xx_optind++;
                        optsp = 1;
                }
                return('?');
        }
        if(*++cp == ':') {
                if(argv[xx_optind][optsp+1] != '\0')
                        optarg = &argv[xx_optind++][optsp+1];
                else if(++xx_optind >= argc) {
                        ERR(": option requires an argument -- ", c);
                        optsp = 1;
                        return('?');
                } else
                        optarg = argv[xx_optind++];
                optsp = 1;
        } else {
                if(argv[xx_optind][++optsp] == '\0') {
                        optsp = 1;
                        xx_optind++;
                }
                optarg = NULL;
        }
        return(c);
}
