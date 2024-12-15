#ifndef __TEXTO_H__
#define __TEXTO_H__

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "oht.h"

#define WORDLEN 15
#define NUMWORDS 65

char dict[5][NUMWORDS][WORDLEN];

#define THING     0
#define PLACE     1
#define VERB      2
#define ADVERB    3
#define ADJECTIVE 4

struct entry {
	char word[WORDLEN];
	int length;
	int pos;
	int index;
	};

FILE *message;
FILE *structs;

int Debug;

struct OHT *htable;

extern void init_words();
extern void msg_to_txt();
extern void uu_to_txt();
extern void pgp_to_txt();
extern void txt_to_uu();
extern void txt_to_pgp();
extern int comp_entries();
extern int get_index();
extern void print_entry();

#endif
