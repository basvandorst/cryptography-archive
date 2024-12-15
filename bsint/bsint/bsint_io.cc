#include "bsint_io.h"
#include <iostream.h>

const int maxDigits = 512;
char	outdigits[maxDigits];

ostream & operator<<( ostream & out, const bsint & number )
{
#ifdef NOTDEF
	int		i;
	char	* output = ConvertToReversedDecimalString( number );
	for ( i = 0; output[i] != '\0'; i++ )
		;
	while ( i-- > 0 )
		out << output[i];
	return out;
#endif


	bsint	u = number;
	bsint	t = 1;
	bsint	q, r;
	int		digits = 0;
	
	if ( u < 0 )
	{
		out << '-';
		u = -u;
	}
	if ( u == 0 )
	{
		out << '0';
		return out;
	}
	while ( t <= u )
	{
		digits++;
		t *= 10;
	}
	while ( digits-- > 0 )
	{
		t /= 10;
		q = u / t;
		u = u % t;
		out << "0123456789"[int(q)];
		//if ( digits && digits % 3 == 0 )
		//	out << ',';
	}
	return out;
}
