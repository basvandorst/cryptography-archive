#include	"des.h"
#include	"compile.h"
#include	"version.h"

/*
 * This software may be freely distributed an modified without any restrictions
 * from the author.
 * Additional restrictions due to national laws governing the use, import or
 * export of cryptographic software is the responsibility of the software user,
 * importer or exporter to follow.
 *
 *					     _
 *					Stig Ostholm
 *					Department of Computer Engineering
 *					Chalmers University of Technology
 */

/*
 * des_setkey
 *
 *	Generates a key schedule from a key.
 *
 *	result  0 => the key was generated.
 *	result -1 => the key has correct parity.
 *	result -2 => the key is weak:
 *
 */

int	des_set_key(
#ifdef __STDC__
	des_cblock		*key,
	des_key_schedule	schedule)
#else
	key, schedule)
des_cblock 		*key;
des_key_schedule	schedule;
#endif
{
	if (! des_test_key_parity(key))
		return -1;	/* The key has the wrong parity. */
	if (des_is_weak_key(key))
		return -2;	/* The key is weak		 */

	VOID des_key(key, schedule);

	return 0;
}
