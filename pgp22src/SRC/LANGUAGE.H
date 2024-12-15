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

/* Use the dummy macro _PSTR for strings that should be extracted, but
 * shouldn't be processed by the PSTR function (eg. array initializers).
 */
#define _PSTR(x)	x

extern char language[]; /* language selector prefix for string file */
