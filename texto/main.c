#include "texto.h"


main( argc, argv )
int argc;
char *argv[];
{

int i, j, k;
FILE *result;
char filename[20];

int pgpout = 0;
int uuout = 0;

if (argc <= 1) {
   fprintf(stderr, "\nUsage: texto [-p|-u] message \n\n");
   exit(1);
   }
else {

	i = 1;
	while (argv[i] != NULL) {

		if ( strncmp(argv[i], "-d", 2) == 0 ) {
			Debug = 1; 
			}
		else if ( strncmp(argv[i], "-p", 2) == 0 ) {
			pgpout = 1; 
			}
		else if ( strncmp(argv[i], "-u", 2) == 0 ) {
			uuout = 1;
			}
		else {
			strcpy(filename, argv[i]);
			message = fopen(filename, "r");
			if (message == NULL) {
				fprintf(stderr, "Error opening %s.  Exiting...\n", filename);
				exit(1);
				}
			}

		i++;
		}

   }

structs = fopen("structs", "r");
if (message == NULL) {
	fprintf(stderr, "Error opening structfile.  Exiting...\n");
	exit(1);
	}

init_words();

if (pgpout) {
	txt_to_pgp();
	}
else if (uuout) {
	txt_to_uu();
	}
else {
	msg_to_txt();
	}

k = fclose(message);
if (k == EOF) {
	fprintf(stderr, "Error closing %s.  Exiting...\n", filename);
	exit(1);
	}

k = fclose(structs);
if (k == EOF) {
	fprintf(stderr, "Error closing structfile.  Exiting...\n");
	exit(1);
	}

OHT_Destroy( htable );

exit(0);

}



void msg_to_txt()
{
char orig;
char temp[80];

while ( (orig=getc(message)) != 'B' && orig != 'b' ) {
	if (orig == (char)EOF ) {
		printf("\nError: message not in PGP or uuencode format.\n");
		printf("	Could not find \"begin\" or \"BEGIN\" at top of file.\n\n");
		exit(0);
		}
	/* else do nothing */
	}

fscanf(message, "%s", temp);

if ( strncmp(temp, "EGIN", 4) != 0 ) {
	if ( strncmp(temp, "egin", 4) != 0 ) {
		printf("\nError: message not in PGP or uuencode format.\n");
		printf("	Could not find \"begin\" or \"BEGIN\" at top of file.\n\n");
		exit(0);
		}
	else {	/* uuencoded, or at least it says begin */
		uu_to_txt();
		}
	}
else {	/* pgp ascii-armour, or at least it says BEGIN */
	pgp_to_txt();
	}

}

void pgp_to_txt()
{
char orig, c, last;
int whatlen;
int done;
int i, j;
char what[20];
char temp[80];
int outlinelen = 0;
int index;

/* 
**	read and ignore until blank line (2 consecutive newlines) 
*/
while ( (orig=getc(message)) ) {
	if (orig == '\n') {
		orig = getc(message);
		if (orig == '\n') {
			break;
			}
		else {
			/*
			fprintf(stderr, "%c", orig);
			*/
			}
		}
	}

while ( (orig=getc(message)) != (char)EOF ) {

	if (isspace(orig)) continue;	/* ignore whitespace */

	if (isdigit(orig)) index = atoi(&orig);       		/*  0 - 9  */
	else if (islower(orig)) index = (int)(orig - 87);  /* 10 - 35 */
	else if (isupper(orig)) index = (int)(orig - 29);  /* 36 - 61 */
	else if (orig == '/') index = 62;
	else if (orig == '+') index = 63;
	else if (orig == '=') index = 64;
	else if (orig == '-') break;

	done = 0;

	while ( !done ) {

		c=getc(structs);
		if (c == 0x0a) continue; /* ignore newlines */
			
		if (c == (char)EOF ) {
			rewind(structs);
			c = getc(structs);
			}

		if ( c == '_' ) {

			fscanf(structs, "%s", what);
			done = 1;

			if ( strncmp(what, "THING", 5) == 0 ) {
				printf("%s", dict[THING][index]);
				outlinelen += strlen(dict[THING][index]);
				}
			else if ( strncmp(what, "VERB", 4) == 0 ) {
				printf("%s", dict[VERB][index]);
				outlinelen += strlen(dict[VERB][index]);
				}
			else if ( strncmp(what, "PLACE", 5) == 0 ) {
				printf("%s", dict[PLACE][index]);
				outlinelen += strlen(dict[PLACE][index]);
				}
			else if ( strncmp(what, "ADJECTIVE", 9) == 0 ) {
				printf("%s", dict[ADJECTIVE][index]);
				outlinelen += strlen(dict[ADJECTIVE][index]);
				}
			else if ( strncmp(what, "ADVERB", 6) == 0 ) {
				printf("%s", dict[ADVERB][index]);
				outlinelen += strlen(dict[ADVERB][index]);
				}
			else {
				printf("%s", what);
				outlinelen += strlen(what);
				done = 0;
				}
			whatlen = strlen(what);
			last = what[whatlen-2];
			if (last == 's') {	/* check 2nd to last char for "s." */
				putchar(last);
				outlinelen +=2;
				}
			else if (last == 'g') {
				printf("ing");
				outlinelen +=4;
				}
			last = what[whatlen-1];	
			if (last == '.' || last == '?' || last == '!') {
				printf("%c  ", last);
				outlinelen +=4;
				}
			else if (last == ',') {
				printf(",");
				outlinelen +=3;
				}
			else if (last == 's') {
				putchar(last);
				outlinelen +=2;
				}
			else if (last == 'g') {
				printf("ing");
				outlinelen +=4;
				}

			if (outlinelen >= 60) {
				printf("\n");
				outlinelen = 0;
				if ((c = getc(structs)) != ' ') {
					ungetc(c, structs);
					}
				}

			}
		else {
			putchar(c);
			outlinelen += 1;
			}
		}
	}

printf(".\n");

}	/* END pgp_to_txt() */



void txt_to_uu()
{
int i, j;
char word[WORDLEN];
int outlinelen = 0;
int wordlen;
struct entry tempent;
struct entry *newent;
int last_apost = 0;
char null_char;

#ifdef UU_SPACE
null_char = ' ';
#else
null_char = '`';
#endif


printf("begin 644 texto.out\n");

while ( fscanf(message, "%s", word) != EOF ) {

	wordlen = strlen(word);
	if (word[wordlen-1] == '.' || word[wordlen-1] == ',' ||
		 word[wordlen-1] == '?' || word[wordlen-1] == '!') {
 		word[wordlen-1] = (char)NULL;
		wordlen -= 1;
		}
	if (word[wordlen-1] == 's' ) {
 		word[wordlen-1] = (char)NULL;
		wordlen -= 1;
		}
	if (strcmp(&word[wordlen-3], "ing") == 0) {
		word[wordlen-1] = word[wordlen-2] = word[wordlen-3] = (char)NULL;
		}
	if (Debug) printf("%s", word);

	sprintf(tempent.word, "%s", word);
	tempent.length = strlen(word);
	tempent.index = 0;
	tempent.pos = 0;

	if ( (newent = OHT_Access( &tempent, htable )) != (void *)OHT_FAILURE ) {

		i = newent->index;

		if (last_apost) {
			printf("%c", null_char);
			outlinelen++;
			if (outlinelen == 61) {
				printf("\n");
				outlinelen = 0;
				}
			}

#ifdef UU_SPACE
		if ( i == 0 ) {
#else
		if ( i == 63 ) {
#endif
			last_apost = 1;
			}
		else {

#ifdef UU_SPACE
			printf("%c", i+32);	
#else
			printf("%c", i+33);	
#endif

			outlinelen++;
			if (outlinelen == 61) {
				printf("\n");
				outlinelen = 0;
				}
			last_apost = 0;
			}

		if (Debug) printf(" %s %s %s %s %s\n", dict[THING][i], dict[PLACE][i],
				dict[ADJECTIVE][i], dict[ADVERB][i], dict[VERB][i]);

		}
	else {
		/*
		printf("Access failed for %s.\n", word);
		*/
		}
	}

	if (last_apost) {
		printf("%c", null_char);
		}
	printf("\n%c\nend\n", null_char);

}

void uu_to_txt()
{
char orig, c, last;
int whatlen;
int done;
int i, j;
char what[20];
char temp[80];
int outlinelen = 0;
int index;

while ( (orig=getc(message)) != 0x0a ) {  
	/* ignore everything up to the first newline */
	}

while ( (orig=getc(message)) != (char)EOF ) {

	if ( orig == 0x0a ) continue;	/* ignore newlines */
	if (islower(orig)) continue;	/* ignore 'end' line */
	else {

#ifdef UU_SPACE
		index = (int)(orig - 32); /* " " =0 up to _=63  for solaris */
#else
		index = (int)(orig - 33); /* !=0 up to `=63 */
#endif

		}

	done = 0;

	while ( !done ) {

		c=getc(structs);
		if (c == 0x0a) continue; /* ignore newlines */
			
		if (c == (char)EOF ) {
			rewind(structs);
			c = getc(structs);
			}

		if ( c == '_' ) {

			fscanf(structs, "%s", what);
			done = 1;

			if ( strncmp(what, "THING", 5) == 0 ) {
				printf("%s", dict[THING][index]);
				outlinelen += strlen(dict[THING][index]);
				}
			else if ( strncmp(what, "VERB", 4) == 0 ) {
				printf("%s", dict[VERB][index]);
				outlinelen += strlen(dict[VERB][index]);
				}
			else if ( strncmp(what, "PLACE", 5) == 0 ) {
				printf("%s", dict[PLACE][index]);
				outlinelen += strlen(dict[PLACE][index]);
				}
			else if ( strncmp(what, "ADJECTIVE", 9) == 0 ) {
				printf("%s", dict[ADJECTIVE][index]);
				outlinelen += strlen(dict[ADJECTIVE][index]);
				}
			else if ( strncmp(what, "ADVERB", 6) == 0 ) {
				printf("%s", dict[ADVERB][index]);
				outlinelen += strlen(dict[ADVERB][index]);
				}
			else {
				printf("%s", what);
				outlinelen += strlen(what);
				done = 0;
				}
			whatlen = strlen(what);
			last = what[whatlen-2];
			if (last == 's') {	/* check 2nd to last char for "s." */
				putchar(last);
				outlinelen +=2;
				}
			else if (last == 'g') {
				printf("ing");
				outlinelen +=4;
				}
			last = what[whatlen-1];	
			if (last == '.' || last == '?' || last == '!') {
				printf("%c  ", last);
				outlinelen +=4;
				}
			else if (last == ',') {
				printf(",");
				outlinelen +=3;
				}
			else if (last == 's') {
				putchar(last);
				outlinelen +=2;
				}
			else if (last == 'g') {
				printf("ing");
				outlinelen +=4;
				}

			if (outlinelen >= 60) {
				printf("\n");
				outlinelen = 0;
				if ((c = getc(structs)) != ' ') {
					ungetc(c, structs);
					}
				}

			}
		else {
			putchar(c);
			outlinelen += 1;
			}
		}
	}

printf(".\n");

}

void txt_to_pgp()
{

int i, j;
char word[WORDLEN];
int outlinelen = 0;
int wordlen;
int equal = 0;
struct entry tempent;
struct entry *newent;

printf("-----BEGIN PGP MESSAGE-----\nVersion: 2.?\n\n");

while ( fscanf(message, "%s", word) != EOF ) {

	wordlen = strlen(word);
	if (word[wordlen-1] == '.' || word[wordlen-1] == ',' ||
		 word[wordlen-1] == '?' || word[wordlen-1] == '!') {
 		word[wordlen-1] = (char)NULL;
		wordlen -= 1;
		}
	if (word[wordlen-1] == 's' ) {
 		word[wordlen-1] = (char)NULL;
		wordlen -= 1;
		}
	if (strcmp(&word[wordlen-3], "ing") == 0) {
		word[wordlen-1] = word[wordlen-2] = word[wordlen-3] = (char)NULL;
		}
	if (Debug) printf("%s", word);

	sprintf(tempent.word, "%s", word);
	tempent.length = strlen(word);
	tempent.index = 0;
	tempent.pos = 0;

	if ( ( newent = OHT_Access( &tempent, htable )) != (void *)OHT_FAILURE ) {

		i = newent->index;

			if (i < 10) {
				if (equal) { printf("\n="); equal = 0; }
				printf("%c", i+48);	/* 0-9 */
				}
			else if (i <= 35) {
				if (equal) { printf("\n="); equal = 0; }
				printf("%c", i+87);	/* a-z */
				}
			else if (i <= 61) {
				if (equal) { printf("\n="); equal = 0; }
				printf("%c", i+29);	/* A-Z */
				}
			else if (i == 62) {
				if (equal) { printf("\n="); equal = 0; }
				printf("/");
				}
			else if (i == 63) {
				if (equal) { printf("\n="); equal = 0; }
				printf("+");
				}
			else if (i == 64) {
				if (equal) printf("=");
				equal = 1;
				}
			else {
				fprintf(stderr, "Got unknown index %d\n", i);
				}

		if (Debug) printf(" %s %s %s %s %s\n", dict[THING][i], dict[PLACE][i],
				dict[ADJECTIVE][i], dict[ADVERB][i], dict[VERB][i]);


			outlinelen++;
			if (outlinelen == 64) {
				printf("\n");
				outlinelen = 0;
				}

		}
	else {
		/*
		printf("Access failed for %s.\n", word);
		*/
		}
	}

	printf("\n");
	printf("-----END PGP MESSAGE-----\n");

	/*
	OHT_Dump( htable );
	*/

}


int comp_entries( void *e1, void *e2 )
{
	
	if ( e1 == NULL || e2 == NULL ) {
		/*
		printf("Can't compare null entry.\n");
		*/
		return( OHT_FAILURE );
		}
	if ( ((struct entry *) e1)->length != ((struct entry *)e2)->length ) {
		/*
		printf("Differing lengths %d %d.\n", 
	         ((struct entry *) e1)->length, ((struct entry *)e2)->length);
		*/
		return( OHT_FAILURE );
		}
	if ((strcmp(((struct entry *)e1)->word, ((struct entry *)e2)->word)) != 0){
		/*
		printf("Same length, different word.\n");
		*/
		return( OHT_FAILURE );
		}

	return( OHT_SUCCESS );

}

int get_index ( void *this )
{
	
	int i;
	int total;

	total = 0;

	for ( i=0; i<WORDLEN; i++ ) {
		total += (int) ((struct entry *)this)->word[i] * 31;
		if (((struct entry *)this)->word[i] == (char)NULL) break;
		}

	total %= (NUMWORDS*5) * 10;

	/*
	printf("%s -> %d\n", ((struct entry *)this)->word, total);
	*/
	return(total);

}


void print_entry ( void *this )
{

	if (this == (void *)NULL) {
		printf("### NULL entry ###\n");
		}
	else {
		printf("%15s length=%4d pos=%2d.\n", 
				((struct entry *)this)->word,
				((struct entry *)this)->length,
				((struct entry *)this)->pos
				);
		}

}
