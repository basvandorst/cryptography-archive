#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>

/*
** pgpd.c
**
** Implements the PGP Key Server PGPD
**
** Ben Cox <thoth@netcom.com>, 19 Nov 1993
**
** See the file COPYING distributed with this program for
** information regarding distribution of this program.
*/

#include "ktp.h"

#define PGPD_VERSION "0.1a (experimental)"

static char *pgpenv[] = { "PGPPATH=" PGPPATH, (char *) NULL };


#ifdef NO_MKSTEMP
/*
** mkstemp
**
** This is a quick-n-dirty implementation of
** mkstemp for machines that don't have it.
*/
int mkstemp(char *template)
{
    extern char *mktemp(char *template);

    if ( strlen(mktemp(template)) == 0 ) {
        errno = EAGAIN;         /* whatever */
        return -1;
    }
    return open(template, O_RDWR | O_CREAT);
}
#endif


#ifdef NO_STRERROR
extern char *sys_errlist[];
#define strerror(E) (sys_errlist[E])
#endif


/*
** mygets
**
** This is used in place of fgets.  The only difference is that CR
** or LF is stripped at the end of the string (unlike fgets).
*/

static char *mygets(char *s, int n, FILE *stream)
{
    char *res;
    int i;

    if ( (res = fgets(s, n, stream)) == NULL )
        return NULL;

    /* string might have CR or LF or both at the end */

    for ( i = 0; res[i]; i++ ) {
        if ( (res[i] == '\r') || (res[i] == '\n') ) {
            res[i] = '\0';
            break;
        }
    }

    return res;
}



static void DoError(char *where)
{
    printf("%03d ", N_ERR);
    printf(S_ERR, where, strerror(errno), errno);
    printf("\n");
    return;
}


static void DoOk(void)
{
    printf("%03d %s\n", N_OK, S_OK);
    return;
}


static void DoNoise(char *noise)
{
    printf("%03d %s\n", N_NOISE, noise);
    return;
}


/*
** GetKey
**
** This function calls PGP to retrieve a key from the key ring.
** The matching key, if found, is printed (in ASCII-Armor format) on
** stdout.  If no key matches, an appropriate string is printed.
*/

void GetKey(const char *keyID)
{
    char keytmp[50];
    off_t kflen;
    int keyfd;
    pid_t pgppid;
    int childstat;

    /*
    ** The race condition introduced by the fact that we are placing
    ** a key into a tmp file before returning it is not a security
    ** problem, because, I mean, hey: these are PUBLIC keys... :^)
    */

    strcpy(keytmp, "/tmp/pgpd.XXXXXX");

    if ( (keyfd = mkstemp(keytmp)) == -1 ) {
        DoError("mkstemp");
        return;
    }

    /*
    ** We can unlink the file now, and it will disappear from
    ** the directory, but it will still be there, because we
    ** have it open.  As soon as we close it, it's toast.
    */
    unlink(keytmp);

    if ( (pgppid = fork()) == (pid_t) -1 ) {
        DoError("fork");
        return;
    }

    if ( pgppid == 0 ) {        /* child */
        dup2(keyfd, 1);
        close(keyfd);
        freopen("/dev/null", "w", stderr);

        execle(PGPBIN, "pgp", "-f", "-o-", "-kxa", keyID, (char *) NULL,
               pgpenv);

        exit(1);
    } else {                    /* parent */
        (void) waitpid(pgppid, &childstat, 0);
        if ( WEXITSTATUS(childstat) == 0 ) {
            kflen = lseek(keyfd, (off_t) 0, SEEK_END);

            if ( kflen == (off_t) 0 ) {
                printf("%03d %s\n", N_NOKEY, S_NOKEY);
            } else {
                (void) lseek(keyfd, (off_t) 0, SEEK_SET);
                printf("%03d %s\n", N_KEY, S_KEY);
                {
                    char keybuf[512];
                    size_t nr;

                    while ( (nr = read(keyfd, keybuf, 512)) > 0 ) {
                        fwrite(keybuf, 1, nr, stdout);
                    }
                }
                printf(".\n");
            }
        } else {
            printf("%03d ", N_PGPERR);
            printf(S_PGPERR, WEXITSTATUS(childstat));
            printf("\n");
        }
        close(keyfd);
        DoOk();
        return;
    }

    /* Shouldn't get here. */
    DoNoise("Fell through to bottom of GetKey.");
    DoError("GetKey");
    return;
}


/*
** AddKey
**
** Receives key data from the client, stores it into a temporary
** file, then runs "pgp -f -ka" with the file as stdin.
*/

void AddKey(void)
{
    char keytmp[50];
    char keyline[100];
    int gotdata = 0;
    int done = 0;
    int keyfd;
    pid_t pgppid;
    int childstat;

    strcpy(keytmp, "/tmp/pgpd.XXXXXX");

    if ( (keyfd = mkstemp(keytmp)) == -1 ) {
        DoError("mkstemp");
        return;
    }

    /*
    ** We can unlink the file now, and it will disappear from
    ** the directory, but it will still be there, because we
    ** have it open.  As soon as we close it, it's toast.
    */
    /* unlink(keytmp); */

    printf("%03d %s\n", N_GIMME, S_GIMME);
    fflush(stdout);

    while ( !done ) {
        if ( mygets(keyline, 100, stdin) == NULL ) {
            close(keyfd);
            DoError("receiving key data");
            return;
        }

        if ( !strcmp(keyline, ".") ) {
            done = 1;
        } else {
            gotdata = 1;
            write(keyfd, (void *) keyline, strlen(keyline));
            /* rather do it this way than worry if strlen(keyline) == 99 */
            keyline[0] = '\n';
            write(keyfd, (void *) keyline, 1);
        }
    }

    if ( !gotdata ) {
        close(keyfd);
        DoNoise("Ignoring null key info.");
        DoOk();
        return;
    }

    DoNoise("Please hold on, this may take a moment...");
    fflush(stdout);

    (void) lseek(keyfd, (off_t) 0, SEEK_SET);

    if ( (pgppid = fork()) == (pid_t) -1 ) {
        DoError("fork");
        return;
    }

    if ( pgppid == 0 ) {        /* child */
        dup2(keyfd, 0);
        close(keyfd);
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        execle(PGPBIN, "pgp", "-f", "-ka", (char *) NULL, pgpenv);

        exit(1);
    } else {                    /* parent */
        close(keyfd);
        (void) waitpid(pgppid, &childstat, 0);
        if ( WEXITSTATUS(childstat) == 12 ) {
            printf("%03d PGP key add failed.\n", N_PGPERR);
        } else if ( WEXITSTATUS(childstat) != 0 ) {
            printf("%03d ", N_PGPERR);
            printf(S_PGPERR, WEXITSTATUS(childstat));
            printf("\n");
        }
        DoOk();
        return;
    }

    /* Shouldn't get here. */
    DoNoise("Fell through to bottom of AddKey.");
    DoError("AddKey");
    return;
}


/*
** GetIndex
**
** This function calls PGP to retrieve the index of the key ring.
** Output generated on stdout or stderr is sent to the client.
*/

void GetIndex(int verbose)
{
    char idxtmp[50];
    int idxfd;
    pid_t pgppid;
    int childstat;

    strcpy(idxtmp, "/tmp/pgpd.XXXXXX");

    if ( (idxfd = mkstemp(idxtmp)) == -1 ) {
        DoError("mkstemp");
        return;
    }

    unlink(idxtmp);

    if ( (pgppid = fork()) == (pid_t) -1 ) {
        DoError("fork");
        return;
    }

    if ( pgppid == 0 ) {        /* child */
        dup2(idxfd, 1);
        dup2(idxfd, 2);         /* pgp -kvv produces output on stderr (grrr) */
        close(idxfd);

        execle(PGPBIN, "pgp", verbose ? "-kvv" : "-kv", (char *) NULL, pgpenv);

        exit(1);
    } else {                    /* parent */
        (void) waitpid(pgppid, &childstat, 0);
        if ( WEXITSTATUS(childstat) == 0 ) {
            (void) lseek(idxfd, (off_t) 0, SEEK_SET);
            printf("%03d %s\n", N_INDEX, S_INDEX);
            {
                char idxbuf[1024];
                size_t nr;

                while ( (nr = read(idxfd, idxbuf, 1024)) > 0 ) {
                    fwrite(idxbuf, 1, nr, stdout);
                }
            }
            printf(".\n");
        } else {
            printf("%03d ", N_PGPERR);
            printf(S_PGPERR, WEXITSTATUS(childstat));
            printf("\n");
        }
        close(idxfd);
        DoOk();
        return;
    }

    /* Shouldn't get here. */
    DoNoise("Fell through to bottom of GetIndex.");
    DoError("GetKey");
    return;
}


void main(int argc, char *argv[])
{
    int done = 0;
    int i;
#ifdef DEBUG
    int debug = 1;
#else
    int debug = 0;
#endif
    char command[100];
    char *arg;

    if ( argc != 1 ) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        exit(1);
    }

    printf("%03d Welcome to the PGP key server, PGPD version %s.\n",
           N_NOISE, PGPD_VERSION);
    if ( debug )
        DoNoise("Debugging option on.");
    DoNoise("Type 'help' for help.");
    DoOk();

    while ( !done ) {
        fflush(stdout);

        if ( mygets(command, 100, stdin) == NULL ) {
            done = 1;
            break;
        }

        if ( debug ) {
            printf("%03d Command (length %d) received: %s", N_NOISE,
                   strlen(command), command);
        }

        /*
        ** find the first space, replace with NULL to terminate
        ** first word in command, then point arg to the remainder
        ** of the command line.
        */
        for ( i = 0; command[i] && (command[i] != ' '); i++ );

        if ( command[i] == ' ' ) {
            command[i] = '\0';
            arg = &command[i+1];
        } else {
            arg = NULL;
        }

        for ( i = 0; i < N_KTP_COMMANDS; i++ ) {
            if ( !strcasecmp(command, KTPcommands[i]) )
                break;
        }

        switch ( i ) {
        case ADD:
            AddKey();
            break;
        case GET:
            if ( arg == NULL )
                printf("%03d %s\n", N_SYNTAX, S_SYNTAX);
            else
                GetKey(arg);
            break;
        case DBG:
            debug = !debug;
            DoOk();
            break;
        case INDEX:
            GetIndex(0);
            break;
        case VINDEX:
            GetIndex(1);
            break;
        case HELP:
            DoNoise("Valid commands are:");
            printf("%03d     %s <keyspec>\n", N_NOISE, KTPcommands[GET]);
            printf("%03d     %s\n", N_NOISE, KTPcommands[ADD]);
            printf("%03d     %s\n", N_NOISE, KTPcommands[DBG]);
            printf("%03d     %s\n", N_NOISE, KTPcommands[INDEX]);
            printf("%03d     %s\n", N_NOISE, KTPcommands[VINDEX]);
            printf("%03d     %s\n", N_NOISE, KTPcommands[HELP]);  
            printf("%03d     %s\n", N_NOISE, KTPcommands[QUIT]);
            DoOk();
            break;
        case QUIT:
            done = 1;
            break;
        default:
            printf("%03d %s\n", N_SYNTAX, S_SYNTAX);
        }
    }

    printf("%03d %s\n", N_BYE, S_BYE);
    exit(0);
}
