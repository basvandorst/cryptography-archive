/*____________________________________________________________________________
    language.h

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

	Include file for PGP foreign language translation facility

    $Id: language.h,v 1.4 1999/05/12 21:01:04 sluu Exp $
____________________________________________________________________________*/


/*
 * Strings with LANG() around them are found by automatic tools and put
 * into the special text file to be translated into foreign languages.
 * LANG () (note the space between 'G' and '(') should be used if there
 * is no string to be extracted (eg. prototype).
 */

extern char *LANG (char *s);

/*
 * Use the dummy macro _LANG for strings that should be extracted, but
 * shouldn't be processed by the LANG function (eg. array initializers).
 */
#define _LANG(x) x

/*char *get_language(void);*/
/*char *set_language(void);*/
