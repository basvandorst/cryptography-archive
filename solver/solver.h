
/* This is the header file for the solver program.
** Change dict_dir to point to your own dictionary.
*/


#define OFILE      "output"
#define DICT_DIR   "/home/wart/src/dict"
#define MAXKEYS     500
#define MAXWORDS     80
#define MAXLINES     10
#define MAXLENGTH    80
#define LINELENGTH  100
#define FALSE         0
#define TRUE          1
#define DEF_THRESH    0

typedef struct word_structure{
  char word[MAXLENGTH];
  char **dict;
  int hvalue;
  int dictsize;
  int length;
  int mult;
} WORD;

/* Decodes the encoded word with the key.  */
char *decode_word(char *key, char *word);

/* Get the dictionary filename where 'word' can be found.  */
char *get_dfname(char *word);

/* Compare two items. Used for qsort() */
int charcomp(char *c, char *d);
int compare(WORD *element1, WORD *element2);

/* Update the key with the mapping used in cword goes to dword.     */
/* Returns FALSE if the mapping was not valid or otherwise failed.  */
int update_key(char *key, char *cword, char *dword);

/* Finds out if the mapping of word1 to word2 is valid.  Returns  */
/* TRUE if it is, FALSE otherwise.                                */
int validate_mapping(char *word1, char *word2);

/* Add the word to the dictionary  */
void add_word(char *word);

/* Move a word to the end of the dictionary */
void move_to_end(char *word);

/* Copy the data from one word structure to another   */
void copy_word(WORD *word1, WORD *word2);

/* Remove a word from the dictionary   */
void delete_word(char *word);

/* Read in another word from the cipher file  */
void get_next_word(FILE *file, char *word);

/* change 'word' to all lowercase letters.  */
void lowerchar(char *word);

/* Check to see if 'word' is in the dictionary  */
void query_word(char *word);

/* Main recursive routine for solving the cipher.  */
void solve_cipher(char *key, int wordindex, int real_words);

/* Sort the list of words in the cipher.  The speed of finding a  */
/* solution depends greatly on the order in which the words       */
/* are sought after.                                              */
void wordsort(void);

int get_hvalue(char *word, int *histogram);
