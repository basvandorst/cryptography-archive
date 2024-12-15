/*
From: Tom Christiansen <tchrist@convex.COM>
Subject: Re: anagram.c - a quicker little anagram finder outter in C
Date: Thu, 4 Feb 1993 19:47:49 GMT
Organization: Convex Computer Corporation, Colorado Springs, CO

Archive-Name: anagram.c
Submitted-By: tchrist@pixel.convex.com

I don't believe I ever published this.  I'd been thinking of doing
it in Perl, but couldn't figure out a good, fast way of constructing a
27-way radix search trie in that language, so chose to use C instead.

There's a lot one could do for optimizing this.  Oh, beware the "long
long" types.  You probably want them to be "long"'s on most systems.
And don't forget to trim the signature at the end.  And use the -p options.

--tom
*/

#include <stdio.h>
#include <ctype.h>
#include <sysexits.h>
#include <signal.h>
#include <setjmp.h>

#define INPUT "/usr/dict/words"

#define COUNT ('z' - 'a' + 2)
#define  EOW   ('z' - 'a' + 1)

struct dict_level {
    struct dict_level *ltr[COUNT];
} Table; 

struct letterpad {
    int count;
    char ltr[COUNT];
};

jmp_buf Prompt_Env;

char Line[100];
char Word[100];
char *Letter;

#define longlong long

longlong Word_Count = 0;
longlong Allocated = 0;
longlong Slots;

int Debug = 0;
int Phrase_Mode;
int Max_Idx;
char *Program;


int from_a_tty, to_a_tty;

int fetch(), interrupt();

main(ac, av)
    char **av;
{
    FILE *fp;
    char *input;
    struct dict_level *dp;

    from_a_tty = isatty(0);
    to_a_tty   = isatty(1);

    Program = *av;

    if (ac > 4) 
	usage(Program);

    if (ac == 3) {
        if (strcmp(av[1], "-p"))
            usage();
        ac--; av++;
        if (ac == 1) 
            usage();
        Phrase_Mode = 1;
    } else if(ac == 2) {
        if (strcmp(av[1],"-p") == 0) Phrase_Mode = 1;
    } 

    Max_Idx = Phrase_Mode ? COUNT : EOW;
    input = ac == 2 && strcmp(av[1],"-p") ? av[1] : INPUT;

    if (!(fp = fopen(input, "r"))) {
	perror(input);
	exit(EX_NOINPUT);
    } 

    setlinebuf(stdout, 0);

    while ( fgets(Line, sizeof(Line), fp) ) {
	Word_Count++;
	if (Debug) printf("storing %s", Line);
	store(Line);
    }

    *(Letter = Word) = '\0';

    {
	longlong slotmax = Allocated * 26;

	dump(&Table);
	fprintf(stderr, "%lld Words, %lld levels\n", Word_Count, Allocated);
	fprintf(stderr, "%lld/%lld pointers used (%4.1f%%)\n", 
			Slots, slotmax, 
			100 * ((double)Slots)/((double)slotmax));
    }

    signal(SIGINT, interrupt);

    while (1) {
	setjmp(Prompt_Env);
	if (from_a_tty) 
	    printf("> ");
	if (!gets(Line))
	    break;
	anagram(Line);
    }

    exit(EX_OK);
}

store (s)
    char *s;
{
    struct dict_level *dp = &Table;
    char a,*cp;

    for (cp = s; *cp && *cp != '\n'; cp++) {
	if (isupper(*cp))
	    *cp = tolower(*cp);
	if (*cp < 'a' || *cp > 'z') 
	    return;
	a = *cp - 'a';
	if (!dp->ltr[a]) {
	    if (!(dp->ltr[a] = (struct dict_level *) 
				calloc(1, sizeof(struct dict_level)))) 
	    {
		perror("calloc");
		exit(EX_OSERR);
	    } 
	    Allocated++;
	}
	dp = dp->ltr[a];
    } 
    dp->ltr[EOW] = (struct dict_level *) -1;
} 

dump(dp)
    struct dict_level *dp;
{
    char digit;

    if (!dp) {
	fprintf(stderr, "can't get here!!!\n");
	return;
    } 

    for (digit = 0; digit < COUNT; digit++) {
	if (dp->ltr[digit]) {
	    Slots++;
	    if (digit == EOW) {
		if (Debug) printf("Word! %s\n", Word);
		*--Letter = '\0';
		return;
	    } 
	    *Letter = digit + 'a';
	    *++Letter = '\0';
	    dump(dp->ltr[digit]);
	} 
    } 
    *--Letter = '\0';
    
} 

anagram(s)
    char *s;
{
    struct letterpad master;
    int i;
    char *cp;

    for (i = 0; i < 26; i++) 
	master.ltr[i] = 0;

    master.count = 0;

    for (cp = s; *cp; cp++) {
	if (isupper(*cp))
	    *cp = tolower(*cp);
	if (*cp < 'a' || *cp > 'z') 
	    continue;
	master.count++;
	master.ltr[*cp - 'a']++;
    } 

    *(Letter = Word) = '\0';

    fetch(&Table, &master); 
} 

fetch (tp, pp)
    struct dict_level *tp;
    struct letterpad  *pp;
{
    int i;

    if (pp->count == 0) {
	if (tp->ltr[EOW] && strcmp(Line,Word)) {
	    printf("%s: %s\n", Line, Word);
	    return 1;
	} 
	return 0;
    } 

    for (i = 0; i < Max_Idx; i++) {
	if (pp->ltr[i] && tp->ltr[i]) {
	    if (Phrase_Mode && i == EOW) {
		*Letter++ = ' ';
		fetch(&Table, pp);
	    } else {
		*Letter++ = i + 'a';
		pp->ltr[i]--;
		pp->count--;
		fetch(tp->ltr[i], pp);
		pp->ltr[i]++;
		pp->count++;
	    }
	    *--Letter = '\0';
	} 
    } 
    return 0;
} 

usage () 
{
    fprintf(stderr, "usage: %s [-p] [dictionary]\n", Program);
    fprintf(stderr," Solves anagrams based on dictionary.  -p means phrase mode.\n");
    exit(EX_USAGE);
} 

interrupt() {
    printf("\n");
    longjmp(Prompt_Env);
} 

