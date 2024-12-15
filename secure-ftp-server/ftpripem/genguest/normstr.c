/*-- normid.c -- Functions to "normalize" a userid or password.
 *  (Remove nonprinting characters, switch to a single case, etc.)
 *
 *  Mark Riordan  6 Oct 91
 */

#include <string.h>
#include "normstrp.h"


/*--- function NormalizeString ------------------------------------
 *
 *   Normalize a password.
 *
 *		Entry		pw contains a zero-terminated string.
 *
 *		Exit		pw contains the name with special characters
 *						deleted, and all characters switched to lower
 *						case.  May be shorter upon exit than entry.
 */

void
NormalizeString(pw)
char *pw;
{
	ToLower(pw);
	RemoveNonprinting(pw);
}

/*--- function ToLower ---------------------------------------
 *
 *  Switch all characters to lower-case in zero-terminated string.
 */
void
ToLower(str)
char *str;
{
	while(*str) {
		if(isupper(*str)) *str = tolower(*str);
		str++;
	}
}

/*--- function RemoveNonprinting -----------------------------
 *
 *  Removes all non-printing characters from a string.
 *
 *		Entry		str is a zero-terminated string.
 *
 *		Exit		str has had all non-printing characters removed
 *						from it.  Space is treated as non-printing.
 *						The string is again zero-terminated.
 *						It may be shorter than upon entry.
 */
void
RemoveNonprinting(str)
char *str;
{
	char *target=str;

	do {
		if(isprint(*str) && *str != ' ') {
			*(target++) = *str;
		}
	} while (*(str++));
	*target = '\0';
}
