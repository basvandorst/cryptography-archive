#include <iostream.h>
#include "bsint.h"
#include "bsint_io.h"
#include <math.h>

int	main( int argc, char * argv[] )
{
	int	firstP;

	if ( argc != 2 )
	{
		cout << "Usage: " << argv[0] << " starting_exponent" << endl;
		return -1;
	}

	firstP = atoi( argv[1] );

	bsintSetSizes( firstP * 3 / 2, 0, 2 );

	int		i;
	
	for ( int p = firstP; ; p += 2 )
	{
		if ( p > bsintGetNumberSize(2) )
		{
			cout << p << " is too large.  Need at least ";
			cout << p * log(2) / log(10) * 5 / 2 << " digits." << endl;
			break;
		}
		
		bsint	Mp = pow(bsint(2),bsint(p))-1;
		bsint	U = 4;
		
		for ( i = 0; i < p-2; i++ )
		{
			U = mod_mp(U*U,p)-2;
			// print what iteration we are on so that for benchmarking
			// we can watch these go by, rather than waiting for the
			// program to finish.
			cout << i << '\r'; cout.flush();
		}
		if ( U == 0 )
		{
			cout << "                                                     \r";
			cout << "2^" << p << "-1 is prime." << endl;
		}
		cout << "                \r";
	}
	return 0;
}
