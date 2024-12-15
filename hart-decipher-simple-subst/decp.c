/*                                                         File: DECP.C
   Cryptogram deciphering program coded by Jian Wang for G. Hart. 
   Version of April 20, 1992.  Requires word list file: DECIPHER.DCT
   See: G. Hart "To Decode Short Cryptograms" Communications of ACM,
   September, 1994, p. 102, for detailed description of the algorithm.
   Note: different word-order heuristics were employed in different 
   versions; I am not sure if this agrees completely with the version
   in the paper.  One difference is that this first organizes the
   dictionary into pattern sets, as mentioned in footnote 1.
   This program is hereby placed in the public domain, 
                                September 1994,  ---George W. Hart--- */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <alloc.h>
#include <conio.h>

#define TRUE		1
#define FALSE		0
#define LEVEL		2	/* sort LEVEL+1 levels */
#define MAXLEN		16      /* longest word length */
#define MAXCRYPT	50	/* Max # of words in cryptogram */
#define MAXCL      	5       /* Max # of lines in cryptogram */
#define THRESHOLD	10	/* Max # of words in a pat set */
#define DICT "decipher.dct"   	/* File name of list of common words */

typedef struct WordGrp {	/* Group of words with same pattern */
	int		pos;
	int		row;
	char		word[MAXLEN];
	struct WordGrp  *next;
}	WORDGRP;

typedef struct WordLst {	/* List of word groups */
	int		num;
	int		pos;
	int		pa_row;
	int		pa_pos;
	struct WordGrp  *wgp;
	struct WordLst  *next;
}	WORDLST;

int     NCL = 0,	/* # of lines in crypto text, associated with CLine[] */
	NL,		/* # of plain words in dict., a/w *L */
	NC = 0,		/* # of code words, a/w Ch[] */
	NS,		/* # of distinct pats in dict & crypt., a/w I[], K[] */
	K[MAXCRYPT],	/* K[l] - sorted word order index */
	I[MAXCRYPT],  	/* I[l] - parent level of l */
	KBHIT = 0,	/* flag for interrupt */
	dup[MAXCRYPT],  /* flag for duplicated words */
	NUM[MAXCRYPT],	/* # of words in a restricted pat set */
	LEN[MAXCRYPT],  /* length of a pattern set */
	NC_USE = 0,	/* # of coded words actually used */
	N[MAXCRYPT][MAXCRYPT], /* # of letters in common btw Wi & Wj */
	Mark[MAXCRYPT],	/* flag for used words, while constructing restr. pat set */
	MARK[MAXCRYPT], /* global flag for words used as roots */
	EXTRA[MAXCRYPT],/* flag for the last 'solve' */
	High = 0;	/* the highest score */
char	CLine[MAXCL][80],	/* input text lines, max 80 chars */
	Ch[MAXCRYPT][MAXLEN];	/* input words */
WORDGRP	*WGP[MAXCRYPT],		/* word list at each level */
	*SET[MAXCRYPT];		/* original plain word list for coded words */
WORDLST *Pat[MAXCRYPT],		/* restricted pat set, given parent set */
	*L;			/* dictionary list */

void	read_dict();		/* read plain word dictionary */
int	new_pattern();		/* check if new plain word has new pattern */
void	read_crypto();		/* read cryptogram */
void	form_pattern();		/* form and sort word list for each pat in crypto */
int	count();		/* # of distinct letters in a word */
int	in_common();		/* # of common letters in two words */
int	match();		/* matching two words */
int	match2();               /* obtain subsets given a set of words */
void	solve();		/* try different permutations and */
void	reorder();		/* re-sort coded word order */
void	free_pat();
void	free_lst();		/* release mem */
void	free_grp();
void	reset_grp();		/* update word grp ptr and pos */
int	consistent();		/* check consistency */
void	write_solution();
WORDGRP *alloc_grp();
WORDLST	*alloc_lst();

main()
{
	int	i, P0[91], Q0[91];	/* P: permutation, Q: CharMap */

	for (i='A'; i<='Z'; i++)	/* initialize the permutation */
		P0[i] = Q0[i] = FALSE;	/* unmarked */
	for (i=0; i<MAXCRYPT; i++)
		MARK[i] = EXTRA[i] = FALSE;

	read_dict();
	read_crypto();
	form_pattern();
	solve(0, P0, Q0, 0, TRUE);		/* initial call */
	fprintf(stderr, "\nDONE!\n");
}

WORDGRP	*alloc_grp()
{
	WORDGRP	*wgp;

	if ((wgp = (WORDGRP *)malloc(sizeof(WORDGRP))) == NULL) {
		fprintf(stderr, "Out of memory - WORDGRP\n");
		exit(-1);
	}
	return wgp;
}

WORDLST	*alloc_lst()
{
	WORDLST	*wlp;

	if ((wlp = (WORDLST *)malloc(sizeof(WORDLST))) == NULL) {
		fprintf(stderr, "Out of memory - WORDLST\n");
		exit(-1);
	}
	return wlp;
}


void	read_dict()
{
	FILE	*f;
	char	word[MAXLEN];
	WORDGRP	*Wgp;
	WORDLST	*Wlp, *l_end;

	if ((f = fopen(DICT, "r")) == NULL) {
		fprintf(stderr, "DICTIONARY not found\n");
		exit(-1);
	}

	for (NL=0; fscanf(f, "%s", &word) != EOF; NL++) {
	    if (strlen(word) > MAXLEN) {
		fprintf(stderr, "Word %s is too long in %s\n", word, DICT);
		exit(-1);
	    }
	    Wgp = alloc_grp();
	    Wgp->row = 0;			/* initialize as the first row */
	    strcpy(Wgp->word, word);		/* word group unit */
	    Wgp->next = NULL;

	    if (L == NULL) {		/* first word */
		L = l_end = alloc_lst();
		L->num = 1;
		L->wgp = Wgp;
		L->next = NULL;
	    } else
		if (new_pattern(word, Wgp)) {		/* new pattern */
		    Wgp->pos = 0;			/* first word */
		    Wlp = alloc_lst();
		    Wlp->num = 1;
		    Wlp->wgp = Wgp;
		    Wlp->next = NULL;
		    l_end->next = Wlp;		/* add to the end */
		    l_end = Wlp;
		}
	}
	fclose(f);
	fprintf(stderr, "Total of %d entries\n", NL);

/*		print the sorted dict
	for (Wlp = L; Wlp != NULL; Wlp = Wlp->next) {
		for (Wgp=Wlp->wgp; Wgp != NULL; Wgp = Wgp->next)
			printf("%s ", Wgp->word);
		getch();
	}
*/
}

int	new_pattern(char *word, WORDGRP *Wgp)	/* test if a word pat is new */
{
	int 	k;
	WORDLST	*wlp;
	WORDGRP	*wgp;

	for (wlp=L; wlp!=NULL; wlp=wlp->next) {
	    if (match(word, wlp->wgp->word)) { 		/* new pattern ? */
		k = 1;
		for (wgp=wlp->wgp; wgp->next!=NULL; wgp=wgp->next)
			k++;
		Wgp->pos = wgp->pos + 1;	/* pos of word */
		wlp->num = k + 1;	/* num of words in the set */
		wgp->next = Wgp;  		/* add to the head */
		return FALSE;
	    }
	}
	return TRUE;
}

void	read_crypto()
{
	int	i, j, k = 0, SP = FALSE,	/* space mark */
		END = FALSE;			/* END - flag */
	char	ch;

	fprintf(stderr,"Enter Cryptogram: [Double ENTER to finish; hit ANY key to stop]\n\n");
	do {
		if (NCL >= MAXCL) {
			fprintf(stderr, "Max %d lines\n", MAXCL);
			exit(0);
		}
		gets(&CLine[NCL][k]);	/* save char from the kth pos */
		if (strlen(CLine[NCL]) > 80) {
			fprintf(stderr, "Max 80 chars per line\n");
			exit(0);
		}
		CLine[NCL][strlen(CLine[NCL])]=' '; /* add a space at end of line */
		NCL++;
		k = 0;                  /* start from the 1st char */
		if ((ch = getchar()) == '\n') 	 /* is double ENTER ? */
			END = TRUE;
		else {
			CLine[NCL][0] = ch;	/* record the first char */
			k = 1;		/* read next line from the 2nd char */
		}
	} while (!END);

	k = 0;				/* initial char in a word */
	for (i = 0; i< NCL; i++) {	/* each line of crypto */
		for (j=0; j < strlen(CLine[i]); j++) {	/* each char of a line */
			ch = CLine[i][j] = toupper(CLine[i][j]);
			if (ch > 64 && ch < 91) {	/* remove marks */
				Ch[NC][k++] = ch;
				SP = FALSE;
			}
			else if (isspace(ch) && !SP) {  /* accept only one space */
				SP = TRUE;
				NC++;			/* next word */
				k = 0;			/* first char */
			}
		}
	}
}

int	match(char *w1, char *w2)
{
	int	i, j, Len = strlen(w1);

	if (Len != strlen(w2))
		return FALSE;
	for (i=1; i < Len; i++)
		for (j=0; j<i; j++)
			if ((w1[i]==w1[j]) != (w2[i]==w2[j]))  /* pat comp */
				return FALSE;
	return TRUE;
}

void	free_grp(WORDGRP *Wgp)
{
	WORDGRP	*wgp;

	while (Wgp != NULL) {
		wgp=Wgp->next;
		free((void *)Wgp);
		Wgp = wgp;
	}
}

int     count(char *word)
{
	int i, n = 0, P[91], Len = strlen(word);

	for (i='A'; i<='Z'; i++)
	    P[i] = FALSE;		/* flag for unused letter */
	for (i=0; i<Len; i++)
	        if (!P[word[i]]) {      /* unrepeated letter */
		    n++;
		    P[word[i]] = TRUE;
		}
	return n;
}

int     in_common(char *w1, char *w2)
{
        char tmp1[MAXLEN], tmp2[MAXLEN];

	strcpy(tmp1, w1);		/* don't destroy original word */
	strcpy(tmp2, w2);		/* since 'strcat' does */
        return (count(w1) + count(w2) - count(strcat(tmp1, tmp2)));
}

void	form_pattern()
{
	int	i, j, OK;
	float 	rate, best;
	WORDLST	*wlp;

	fprintf(stderr, "PRECOMPUTING ... \n");
	for (i=0; i<NC-1; i++)             /* form Nij */
	    for (j=i+1; j<NC; j++)
		N[i][j] = N[j][i] = in_common(Ch[i], Ch[j]);

	for (j=0; j<NC; j++) {		/* precomputing flags and word pat list */
	    dup[j] = FALSE;
	    for (i=0; !dup[j] && i<j; i++) 		/* duplication flag */
		if (strcmp(Ch[j], Ch[i]) == 0)
			dup[j] = TRUE;
	    OK = FALSE;
	    for (wlp=L; !OK && wlp!=NULL; wlp=wlp->next) /* check word in dict */
		if (match(Ch[j], wlp->wgp->word)) {
		    OK = TRUE;
		    SET[j] = wlp->wgp; /* assign list for each word */
				/* and add a flag word to help extra search */
		    LEN[j] = wlp->num;
		}
	    if (!OK || dup[j]) 		/* no matched pat set */
		LEN[j] = 0;
	    else
		NC_USE++;
	}

	/* release unused memory */
	for (wlp = L; wlp!= NULL; wlp=wlp->next) {
		OK = TRUE;			/* unused mem */
		for (j=0; j<NC; j++)
			if (match(Ch[j], wlp->wgp->word))
				OK = FALSE;	/* use it */
		if (OK)
			free_grp(wlp->wgp);
	}
	fprintf(stderr, "SOLVE ... \n");
	reorder(0);
}

void	free_lst(WORDLST *Wlp, int k)
{
	WORDLST	*wlp;

	while (Wlp != NULL) {
		wlp = Wlp->next;
/*		if (Wlp->num != LEN[k]) {		*/
			free_grp(Wlp->wgp);
			free((void *)Wlp);
/*		}					*/
		Wlp = wlp;
	}
}

void	free_pat(int Is)
{
	int 	i;

	for (i=Is; i<=NS; i++)
		free_lst(Pat[K[i]], K[i]);
}

WORDGRP	*same_grp(int k, int num, WORDGRP *Wgp)
{
	int	OK;
	WORDGRP	*wgp1, *wgp2;
	WORDLST	*wlp;

	if (num == LEN[k])		/* save memory */
		return SET[k];		/* Wgp must be the whole set */
	for (wlp = Pat[k]; wlp != NULL; wlp = wlp->next) {
	    if (wlp->num == num) {
		OK = TRUE;
		wgp2 = Wgp;
		for (wgp1 = wlp->wgp; OK && wgp1 != NULL; wgp1 = wgp1->next) {
		    if (strcmp(wgp1->word, wgp2->word) != 0)
			OK = FALSE;
		    wgp2 = wgp2->next;
		}
		if (OK)
		    return wlp->wgp;
	    }
	}
	return NULL;
}

void	reorder(int Is)
{
	int	i, j, k, l, Sum, II, KK, best_x, Mark2[MAXCRYPT], Len;
	float	best, rate;
	WORDGRP *wgp, *wgp2, *Wgp, *g_end, *PP;
	WORDLST	*wlp, *Pat2, *l_end;

	if (Is != 0) {
		if (Is == NS+1) {
			fprintf(stderr, "\nDONE!\n");
			exit(0);
		}
/*		fprintf(stderr, "Re-ordering at level %2d - ", Is);	*/
		free_pat(Is);
	}

	for (i=0; i<NC; i++) {			/* initialize flag */
		if (dup[i] || LEN[i] == 0 || MARK[i])
			Mark[i] = TRUE;
		else
			Mark[i] = FALSE;
		Mark2[i] = 0;
	}
	best = 0;
	for (j=0; j<NC; j++) {
	    if (!Mark[j]) {
		rate = count(Ch[j])/(float)LEN[j];
		if (rate > best) {
			 /* the Is_th word in new order */
		    best = rate;
		    KK = j;
		}
	    }
	}
	K[Is] = KK;
	Mark[KK]= TRUE;
	Pat[KK] = alloc_lst();
	WGP[KK] = Pat[KK]->wgp = SET[KK];  /* first list */
	Pat[KK]->num = LEN[KK];
	Pat[KK]->pos = Pat[KK]->pa_row = Pat[KK]->pa_pos = 0;
	Pat[KK]->next = NULL;
	I[Is] = 0;   			/* avoid null pointer for root */
/*	fprintf(stderr, "root word  is  %s ", Ch[K[Is]]);	*/

	for (k=Is+1; k<NC_USE; k++) {
	    best_x = 0;
	    Len = NL;
	    for (j=0; j<NC; j++) {              /* get K[] */
		if (!MARK[j] && !Mark[j]) {	/* unused words */
		    Sum = 0;
		    for (i=Is; i<k; i++) 	/* from the marked words */
		        Sum += N[j][K[i]];
		    if (k == NC_USE - 1)
			KK = j;
		    if (Sum > best_x || (Sum == best_x && LEN[j] < Len)) {
			best_x = Sum;
			KK = j;
			Len = LEN[j];
		    }
		}
	    }

	    K[k] = KK; 		/* index for current crypt word */
	    Mark[KK] = TRUE;	    	/* mark K[NS] */

	    best_x = 0;                         /* get I[] */
	    for (i=Is; i<k; i++)
		if (!Mark2[K[i]] && N[KK][K[i]] > best_x) {
		    II = i;
		    best_x = N[KK][K[i]];
	    }
	    if (best_x == 0) { 		/* if no parent matched */
		II = Is;		/* choose root */
		Mark2[KK] = TRUE;	/* not used as parent */
	    }
	    I[k] = II;
	    II = K[I[k]];
	    NUM[KK] = 0;		/* # of words in restricted set */
	    Pat[KK] = NULL;
	    i = 0;			/* i - count the position of parent w */
	    for (wlp=Pat[II]; wlp != NULL; wlp = wlp->next) {
	      for (wgp2=wlp->wgp; wgp2!=NULL; wgp2=wgp2->next) {
		Wgp = NULL;
		j=0;
		for (wgp = SET[KK]; wgp != NULL; wgp = wgp->next) {
		  if (match2(Ch[II],wgp2->word,Ch[KK],wgp->word)) {
		    NUM[KK]++;
		    if (Wgp == NULL) {  /* create word subset */
		      Wgp = g_end = alloc_grp();
		      Wgp->row = i;		/* record the row pos */
		      Wgp->pos = j++;
		      strcpy(Wgp->word, wgp->word);
		      Wgp->next = NULL;
		    } else
		    {
		      PP = alloc_grp();
		      PP->row = i;  		/* row pos */
		      PP->pos = j++;
		      strcpy(PP->word, wgp->word);
		      PP->next = NULL;
		      g_end->next = PP;	/* add to the tail */
		      g_end = PP;
		    }
		  }
		}
		if (Wgp != NULL) {
		  if (Pat[KK] == NULL) {  /* create pat set */
		    Pat[KK] = l_end = alloc_lst();
		    Pat[KK]->num = j;		/* # of words in the lsit */
		    Pat[KK]->pos = i++;		/* row pos */
		    Pat[KK]->pa_row = wlp->pos;	/* parent row pos */
		    Pat[KK]->pa_pos = wgp2->pos;/* parent word pos in that row */
		    Pat[KK]->wgp = Wgp;
		    Pat[KK]->next = NULL;
		  } else
		  {
		    Pat2 = alloc_lst();
		    Pat2->num = j;		/* # of words in the list */
		    Pat2->pos = i++;		/* row pos */
		    Pat2->pa_row = wlp->pos;	/* parent row pos */
		    Pat2->pa_pos = wgp2->pos;	/* parent pos in that row */
/*		    if ((PP = same_grp(KK, j, Wgp)) != NULL) {
		      free_grp(Wgp);	 save memory for identical grp
		      Pat2->wgp = PP;
		    } else 			*/
		      Pat2->wgp = Wgp;
		    Pat2->next = NULL;
		    l_end->next = Pat2;
		    l_end = Pat2;
		  }
		}
	     }
	    }
	    if (NUM[KK] > THRESHOLD)
		Mark2[KK] = TRUE;
	}
	if (Is == 0) {
		NS = k - 1;
		I[k] = 0;			/* avoid null pointer */
	}
/*	fprintf(stderr, "... \n");		*/

/* 		test the sorted list
	for (i=Is; i<=NS; i++) {
	    printf("Ch = %s  Is = %d  P = %d :\n",
		    Ch[K[i]], i, I[i]);
	    for (wlp=Pat[K[i]]; wlp != NULL; wlp = wlp->next) {
		if (wlp->wgp != NULL) {
		    printf("    pos = %d Pa_row %d pa_pos %d :",
				wlp->pos, wlp->pa_row, wlp->pa_pos);
		    for (wgp=wlp->wgp; wgp != NULL; wgp = wgp->next) {
			printf("%d %s  ", wgp->pos, wgp->word);
		    }
		    printf("\n");
		}
	    }
	    fprintf(stderr,"                    press ENTER to continue\n");
	    getchar();
	}
*/
}

int	match2(char *w1, char *x, char *w2, char *y)
{
	int	i, j, Len1 = strlen(w1), Len2 = strlen(w2);

	for (i=0; i<Len1; i++)
		for (j=0; j<Len2; j++)
			if ((w1[i]==w2[j]) != (x[i]==y[j]))
				return FALSE;
	return TRUE;
}

void	solve(int Is, int *P, int *Q, int score, int Top)
{
	int 	i, NewP[91], NewQ[91], OK;
	char	word[MAXLEN];
	WORDGRP *wgp;
	WORDLST	*wlp;

	if (!kbhit()) {		/* interrupt, I/O function */
	    if (Is > NS || (score + NS - Is + 1) < High) {
			   /* compare score and give possible answers */
		if (score >= High) {
			write_solution(P, score);
			High = score;
		}
	    } else
	    {
		for (wgp=WGP[K[Is]]; wgp!=NULL; wgp=wgp->next) {
		    for (i=Is+1; i<=NS; i++)
			if (I[i] == Is) {
			    if (EXTRA[Is]) {
				EXTRA[i] = TRUE;
				WGP[K[i]] = SET[K[i]];
			    }
			    else {
				OK = FALSE;
				for (wlp=Pat[K[i]]; !OK && wlp!= NULL; wlp=wlp->next) {
				    if (wlp->pa_row == wgp->row &&  /* match row */
					wlp->pa_pos == wgp->pos) {  /* match pos */
					WGP[K[i]] = wlp->wgp;
					OK = TRUE;
				    }       /* find the pat subset */
				}
				EXTRA[i] = 0;
			    }
			}
		    for (i='A'; i<='Z'; i++) {      /* new permut */
			NewP[i] = P[i];
			NewQ[i] = Q[i];
		    }
		    if (consistent(Ch[K[Is]], wgp->word, NewP, NewQ))
			    solve(Is+1, NewP, NewQ, score+1, 0);
				/* next word with parent word at level I[] */
		}	 	/* for its child call */

		if (Top == 1 && Is < LEVEL) {	/* sorting the top 3 level */
			MARK[K[Is]] = TRUE;	/* level Is has been used up */
			reorder(Is+1);
			solve(Is+1, P, Q, score, 1);
		} else {
			for (i=Is+1; i<=NS; i++)
				if (I[i] == Is) {
					WGP[K[i]] = SET[K[i]];
					EXTRA[i] = TRUE;
				}
			solve(Is+1, P, Q, score, 0);
		}
	    }
	} else {
	    fprintf(stderr, "\nPROGRAM INTERRUPTED!\n");
	    exit(0);
	}
}

int	consistent(char *w1, char *w2, int *NewP, int *NewQ)
{
	int	i, OK = TRUE, Len = strlen(w1);

	for (i = 0; OK && i != Len; i++) {
		if (!NewP[w1[i]]) {		/* char w1[i] not filled yet */
			if (NewQ[w2[i]])        /* char w2[i] not filled yet */
				OK = FALSE;	/* continue */
			else {
				NewP[w1[i]] = w2[i];	/* fill the char */
				NewQ[w2[i]] = TRUE;	/* mark w2[i] */
			}
		} else
			if (NewP[w1[i]] != w2[i])	/* if mark not correct */
				OK = FALSE;		/* not consistent yet */
	}
	return OK;
}

void	write_solution(int *P, int score)
{
	int	i, j, Len;
	char	ch;

	for (i=0; i< NCL; i++) {
		if (i==0)	/* first line of crypto */
			printf("%2d :  ", score);
		else
			printf("      ");
		Len = strlen(CLine[i]);
		for (j=0; j<Len; j++) {   /* each word in crypto */
			ch = CLine[i][j];
			if (ch >= 'A' && ch <= 'Z') {
				if (P[ch] == 0)
					printf("=");	   /* no char matched */
				else
					printf("%c", P[ch]);  /* matched char */
			} else
				printf("%c", ch);	/* non-letter char */
		}
		printf("\n");
	}
}
