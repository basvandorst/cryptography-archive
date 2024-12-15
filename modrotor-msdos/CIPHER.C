/*
 * cipher - encrypt messages
 *
 * cipher implements a four rotor enigma machine with 94 element
 * rotors. It is intended to be used to encrypt network mail so that
 * it will not be routinely decrypted.
 *
 *  Modified by gompa 18 Nov 91. (See README file).
 *     Main changes: o ANSIfied
 *                   o Ported to OS/2
 *                   o Some error checking removed
 *
 *
 * Disclaimer:
 *
 * No warranty of any kind is made concerning any use of this
 * program. The author assumes no responsibility for it's use or for
 * any problems arising from it's use by any recipient.
 *
 * Use:
 *
 * cipher [-r[k key]] [file] . . . rcipher . . .
 *
 * Description:
 *
 * Copy standard input to standard output, performing an encryption
 * similar to crypt(1) with the following differences:
 *
 * - only printing character including space and excluding newline
 * and tilde are encrypted (into the same set). - maximun of 512 byte
 * lines (including \n) are allowed - if cipher produces a line
 * beginning "From " while encrypting it will prepend a tilde to the
 * line. - there are four full rotors and one half rotor. -
 * encryption occurs only on text on lines between (and not
 * including) lines beginning "[cipher]" and "[clear]".
 *
 * If a filename is given, reads from the file instead of stdin.
 *
 * Options:
 *
 * -r   causes cipher to accept a list of files and replace them with
 * their encrypted form.  Any number of filenames may be used with
 * this option.
 *
 * -k key       specify a key  (otherwise you are asked for it)
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <os2io.h>

#define RS      94
#define RN      4
#define RMASK   0x7fff

#define LINESIZE        512
#define START   "[cipher]"
#define STOP    "[clear]"

char r[RS][RN];                 /* rotors */
char ir[RS][RN];                /* inverse rotors */
char h[RS];                     /* half rotor */
char s[RS];                     /* shuffle vector */
int p[RN];                      /* rotor indices */

extern char *getpass(char *);
extern char *crypt(char *,char *);

void fatal(int, char *);
void makekey(char *);
void passfile(void);
void setup(void);
char sgetchar(void);
void sputchar(int);
void usage(void);

char second[512];
char first[16];

int cipher, gotkey, gotfile, replacefile;

char *lock;

main(argc, argv)
    char **argv;
{
    char *argp;
   char *lastname(char *);
   int i;

   if (*lastname(argv[0]) == 'r')
       replacefile = 1;

   for (i = 1; i < argc; i++)
   {
       argp = argv[i];
       if (*argp == '-')
       {
           argp++;
           for (; *argp != '\0'; argp++)
           {
               switch (*argp)
               {
                case 'e':
                    cipher = 1;
                    break;
                case 'r':
		    replacefile = 1;
		    break;
		case 'k':
		    if (++i < argc)
		    {
			gotkey = 1;
			argp = argv[i];
			makekey(argp);
			while (*argp)
			{
			    *argp = '\0';
			    argp++;
			}
			argp--;
		    } else
			fatal(1, "Need a key following -k\n");
		    break;
		default:
                    usage();
		    break;

		}
	    }
	} else
	{

	    /* argument is a file.  process it */

	    freopen(argp, "r", stdin);
	    if (gotfile && !replacefile)
		fatal(4, "Only one file allowed\n");

	    if (replacefile)
	    {
		strcpy(first, argp);
		strcpy(second, tmpnam(NULL));
		freopen(second, "w", stdout);
                /* So now first holds the original name and is stdin */
		/* and second hold the temporary name and is stdout */
	    }
	    passfile();

	    if (replacefile)
	    {
		fclose(stdin);
		fclose(stdout);
		remove(argp);
                rename(second,first);
            }
	    gotfile = 1;
	}
    }

    if (!gotfile)
    {
	if (replacefile)
	    fatal(4, "need filenames with -r flag\n");
	passfile();
    }
    exit(0);
}

void
passfile(void)
{
    register int i;
    register j, ph = 0;
    static char keybuffer[9];

    while (!gotkey)
    {
	strcpy(keybuffer, getpass("Enter key: "));
	if (strcmp(keybuffer, getpass("Once more: ")))
	{
	    fatal(1, "Your keys were not the same\n");
	    continue;
	}
	makekey(keybuffer);
	gotkey = 1;
    }

    setup();

    while ((i = sgetchar()) != EOF)
    {
        if (cipher)
	{
	    if ((i >= ' ') && (i < '~'))
	    {
		i -= ' ';

		for (j = 0; j < RN; j++)	/* rotor forwards */
		    i = r[(i + p[j]) % RS][j];

		i = ((h[(i + ph) % RS]) - ph + RS) % RS;	/* half rotor */

		for (j--; j >= 0; j--)	/* rotor backwards */
		    i = (ir[i][j] + RS - p[j]) % RS;

		j = 0;		/* rotate rotors */
		p[0]++;
		while (p[j] == RS)
		{
		    p[j] = 0;
		    j++;
		    if (j == RN)
			break;
		    p[j]++;
		}

		if (++ph == RS)
		    ph = 0;

		i += ' ';
	    }
	}
	sputchar(i);
    }

}

char
sgetchar(void)
{
    static char buffer[LINESIZE], *bP;
    char nextchar;

    while (bP == (char *) 0)
    {
	if (NULL == fgets(buffer, LINESIZE, stdin))
	    return (EOF);
	if (*buffer == '[')
	{
	    if (!strncmp(buffer, START, strlen(START)))
	    {
		fputs(buffer, stdout);
                cipher = 1;
		continue;
	    }
	    if (!strncmp(buffer, STOP, strlen(STOP)))
	    {
		fputs(buffer, stdout);
                cipher = 0;
		continue;
	    }
	}
	bP = buffer;
    }

    nextchar = *bP;
    if (*(++bP) == '\0')
    {
	if (*(bP - 1) != '\n')
	{
	    fatal(1, "line in text is too long\n");
	}
	bP = (char *) 0;
    }
    return (nextchar);

}

void
sputchar(int out)
{
    static char obuffer[LINESIZE], *obP = obuffer;

    *(obP++) = out;
    if (out == '\n')
    {
        if (cipher && !strncmp(obuffer, "From ", 5))
	{
	    if (obP - obuffer < LINESIZE)
		putc('~', stdout);	/* prepend a wiggle */
	    else
		fatal(1, "line with \"From \" is too long\n");
	}
	*(obP++) = '\0';
	fputs(obuffer, stdout);
	obP = obuffer;
    }
}

void
makekey(char *rkey)
{
    char key[8], salt[2], *crypt();

    strncpy(key, rkey, 8);
    salt[0] = key[0];
    salt[1] = key[1];
    lock = crypt(key, salt);
}

/*
 * shuffle rotors. shuffle each of the rotors indiscriminately.  shuffle the half-rotor using a
 * special obvious and not very tricky algorithm which is not as sophisticated as the one in
 * crypt(1) and Oh God, I'm so depressed. After all this is done build the inverses of the rotors.
 */

void
setup(void)
{
    register long i, j, k, temp;
    long seed;

    for (j = 0; j < RN; j++)
    {
	p[j] = 0;
	for (i = 0; i < RS; i++)
	    r[i][j] = i;
    }

    seed = 123;
    for (i = 0; i < 13; i++)	/* now personalize the seed */
	seed = (seed * lock[i] + i) & RMASK;

    for (i = 0; i < RS; i++)	/* initialize shuffle vector */
	h[i] = s[i] = i;

    for (i = 0; i < RS; i++)
    {				/* shuffle the vector */
	seed = (5 * seed + lock[i % 13]) & RMASK;;
	k = ((seed % 65521) & RMASK) % RS;
	temp = s[k];
	s[k] = s[i];
	s[i] = temp;
    }

    for (i = 0; i < RS; i += 2)
    {				/* scramble the half-rotor */
	temp = h[s[i]];		/* swap rotor elements ONCE */
	h[s[i]] = h[s[i + 1]];
	h[s[i + 1]] = temp;
    }

    for (j = 0; j < RN; j++)
    {				/* select a rotor */

	for (i = 0; i < RS; i++)
	{			/* shuffle the vector */
	    seed = (5 * seed + lock[i % 13]) & RMASK;;
	    k = ((seed % 65521) & RMASK) % RS;
	    temp = r[i][j];
	    r[i][j] = r[k][j];
	    r[k][j] = temp;
	}

	for (i = 0; i < RS; i++)
	{			/* create inverse rotors */
	    ir[r[i][j]][j] = i;
	}
    }
}

char *
lastname(char *name)
{
    char *where;
    where = strrchr(name, '/');
    return (where ? (where + 1) : name);
}

/* VARARGS2 */
void
fatal(int code, char *msg)
{
    fprintf(stderr, "cipher:  %s", msg);

    if (code != -1)
	exit(code);
}

void
usage(void)
{
    fprintf(stderr, "ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\n");
    fprintf(stderr, "³ CIPHER: Simple text-only encryption.                  ³\n");
    fprintf(stderr, "³                                                       ³\n");
    fprintf(stderr, "³ USAGE: cipher [-e][-r] textfile                       ³\n");
    fprintf(stderr, "³                                                       ³\n");
    fprintf(stderr, "³ NOTES: The -e switch causes the entire file to be     ³\n");
    fprintf(stderr, "³   encrypted. Default is to encrypt text between lines ³\n");
    fprintf(stderr, "³   beginning with [cipher] and [clear].                ³\n");
    fprintf(stderr, "³   The -r switch replaces the original file with the   ³\n");
    fprintf(stderr, "³   ciphertext. Otherwise output is to stdout.          ³\n");
    fprintf(stderr, "³                                                       ³\n");
    fprintf(stderr, "³ EXAMPLES:                                             ³\n");
    fprintf(stderr, "³   cipher textfile > ciphertext                        ³\n");
    fprintf(stderr, "³   cipher ciphertext                                   ³\n");
    fprintf(stderr, "³   cipher -er textfile                                 ³\n");
    fprintf(stderr, "³   cipher < textfile                                   ³\n");
    fprintf(stderr, "³                                                       ³\n");
    fprintf(stderr, "³ VERSION: 1.0                        OS: MSDOS or OS/2 ³\n");
    fprintf(stderr, "³                                                       ³\n");
    fprintf(stderr, "³[7m    Crypto Tools from: Cyber City BBS 416/593-6000     [m³\n");
    fprintf(stderr, "ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\n");
    exit(1);
}
