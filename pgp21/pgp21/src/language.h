/*
 *	language.h
 *	Include file for PGP foreign language translation facility
 */


/* Strings with PSTR() around them are found by automatic tools and put
 * into the special text file to be translated into foreign languages.
 * PSTR () (note the space between 'R' and '(') should be used if there
 * is no string to be extracted (eg. prototype).
 */

extern char	*PSTR (char *s);

extern char language[]; /* language selector prefix for string file */

/* Internal/External representation conversion macros for conversion of
 * user ID's (will be implemented in a future release of PGP, now they
 * are just dummies)
 */
#define	CONVERT_TO_CANONICAL_CHARSET(s) /* String to internal string (at same place) */
#define	LOCAL_CHARSET(s)	s	/* String to external string */

extern char INT_C(char c);      /* Char to internal char */
extern char EXT_C(char c);      /* Char to external char */

/* Plaintext files import/export conversion modes */

#define NO_CONV  0      /* No conversion needed */
#define INT_CONV 1      /* Convert text to internal representation */
#define EXT_CONV 2      /* Convert text to external representation */

extern int CONVERSION;  /* Global var to rule copyfiles */
