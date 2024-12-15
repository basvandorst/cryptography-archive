#include <ctype.h>
#define MAXKEY  2907    /* maximum length of key text */

char key[MAXKEY];       /* collected key text letters */
int use[MAXKEY];        /* general purpose usage array */

#define MAXBUF  1000    /* character buffer */
char buf[MAXBUF];

int msg1[] = {
#include "1"    /* paper #1: The Locality of the Vault */
};

int msg2[] = {
#include "2"    /* paper #2: The Contents of the Vault */
};

int msg3[] = {
#include "3"    /* paper #3: Names and Residences */
};

#define SIZE1	(sizeof(msg1)/sizeof(msg1[0]))
#define SIZE2	(sizeof(msg2)/sizeof(msg2[0]))
#define SIZE3	(sizeof(msg3)/sizeof(msg3[0]))
