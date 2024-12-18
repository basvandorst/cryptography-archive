/*
Perl Extension for the random function
*/

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "truerand.h"

MODULE = Utils::TrulyRandom		PACKAGE = Utils::TrulyRandom

long
truly_random_value()
    CODE:
	{
		RETVAL = truerand();
	}
	OUTPUT:
	RETVAL
