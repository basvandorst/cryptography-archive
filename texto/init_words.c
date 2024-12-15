#include "texto.h"

void init_words()
{

   int i, j, k;
	char what[20];
	int partofs;
	FILE* words;
	char c;

	struct entry temp;

	/*
   fprintf(stderr, "Initializing words\n");
	*/

   words = fopen("words", "r");
   if (words == NULL) {
		fprintf(stderr, "Error opening wordfile.  Exiting...\n");
		exit(1);
		}

	htable = OHT_Create( sizeof(struct entry), NUMWORDS*5, 
								(int (*)(void *, void *)) comp_entries,
								(int (*)(void *)) get_index,
								(void (*)(void *)) print_entry );
	
	while ( (c = getc(words)) != (char)EOF ) {

		if (isspace(c)) continue;

		if ( c == '#' ) {
			fscanf(words, "%s", what);

			if ( strcmp(what, "Things") == 0 ) {
				partofs = THING;
				}
			else if ( strcmp(what, "Verbs") == 0 ) {
				partofs = VERB;
				}
			else if ( strcmp(what, "Adverbs") == 0 ) {
				partofs = ADVERB;
				}
			else if ( strcmp(what, "Adjectives") == 0 ) {
				partofs = ADJECTIVE;
				}
			else if ( strcmp(what, "Places") == 0 ) {
				partofs = PLACE;
				}
			else {
				printf("Ignored.\n");
				}
			for ( i=0; i<NUMWORDS; i++ ) {
				fscanf(words, "%s", temp.word);
				temp.pos   = partofs;
				temp.length   = strlen(temp.word);
				temp.index = i;
				if ( OHT_Insert( (void *)&temp, htable ) == OHT_FAILURE ) {
					fprintf(stderr, "Could not insert %s.\n", temp.word);
					}
				sprintf(dict[partofs][i], "%s", temp.word);
				}
			}

		else if (isalpha(c)) {		/* Not a comment */
      	}
      }

/*
fprintf(stderr, "Finished reading words\n");
OHT_Dump( htable );
*/

k = fclose(words);
if (k == EOF) {
	fprintf(stderr, "Error closing wordfile.  Exiting...\n");
	exit(1);
	}

}

