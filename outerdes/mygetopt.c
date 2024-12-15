/*LINTLIBRARY*/
#include <string.h>
#ifdef MSDOS
#include <io.h>
#endif

#ifndef NULL
#define NULL    0
#endif
#define EOF     (-1)
#define ERR(s, c)       if(myopterr){\
        char errbuf[2];\
        errbuf[0] = c; errbuf[1] = '\n';\
        (void) write(2, argv[0], (unsigned)strlen(argv[0]));\
        (void) write(2, s, (unsigned)strlen(s));\
        (void) write(2, errbuf, 2);}

int     myopterr = 1;
int     myoptind = 1;
int     myoptopt;
char    *myoptarg;

int
mygetopt(argc, argv, opts)
int     argc;
char    **argv, *opts;
{
        static int sp = 1;
        register int c;
        register char *cp;

        if(sp == 1)
                if(myoptind >= argc ||
                   argv[myoptind][0] != '-' || argv[myoptind][1] == '\0')
                        return(EOF);
                else if(strcmp(argv[myoptind], "--") == 0) {
                        myoptind++;
                        return(EOF);
                }
        myoptopt = c = argv[myoptind][sp];
        if(c == ':' || (cp=strchr(opts, c)) == NULL) {
                ERR(": illegal option -- ", c);
                if(argv[myoptind][++sp] == '\0') {
                        myoptind++;
                        sp = 1;
                }
                return('?');
        }
        if(*++cp == ':') {
                if(argv[myoptind][sp+1] != '\0')
                        myoptarg = &argv[myoptind++][sp+1];
                else if(++myoptind >= argc) {
                        ERR(": option requires an argument -- ", c);
                        sp = 1;
                        return('?');
                } else
                        myoptarg = argv[myoptind++];
                sp = 1;
        } else {
                if(argv[myoptind][++sp] == '\0') {
                        sp = 1;
                        myoptind++;
                }
                myoptarg = NULL;
        }
        return(c);
}
