#include <iostream.h>
#include "bsint.h"
#include "bsint_io.h"
#include <math.h>

//
// An open question is th behavior of this sequence:
//
// if A is odd, A     = 3 A + 1
//     n         n+1       n
//
// if A is even, A    = A  / 2
//     n          n+1    n
//
// This program finds the first starting value of this sequence for
// which more than 32 bit arithmentic is needed to follow the
// sequence.
//

bsint	maxn;

int	FindLength( bsint n );
int	FindLength( bsint n )
{
	for ( int i = 0; n != 1; i++ )
	{
		if ( n % 2 == 0 )
			n /= 2;
		else
		{
			n = 3 * n + 1;
			if ( n > maxn )
				maxn = n;
		}
	}
	return i;
}

int	main( void )
{
	int	maxlength = 0;
	bsint	maxlong = pow(bsint(2),32)-1;
	bsint	maxmaxn = 0;
	long	lastReport = 0;
	
	for ( bsint k = 1; ; ++k )
	{
		maxn = 0;
		int length = FindLength(k);
		if ( maxn > maxmaxn )
		{
			maxmaxn = maxn;
			cout << k << " reaches " << maxmaxn << endl;
			if ( maxmaxn > maxlong )
			{
				cout << "This cannot be stored in an unsigned long." << endl;
				return;
			}
		}
		if ( length > maxlength )
		{
			maxlength = length;
			// cout << k << ", length " << length << endl;
		}
	}
	return 0;
}

#ifdef NOTDEF
3 reaches 16
7 reaches 52
15 reaches 160
27 reaches 9232
255 reaches 13120
447 reaches 39364
639 reaches 41524
703 reaches 250504
1819 reaches 1276936
4255 reaches 6810136
4591 reaches 8153620
9663 reaches 27114424
20895 reaches 50143264
26623 reaches 106358020
31911 reaches 121012864
60975 reaches 593279152
77671 reaches 1570824736
113383 reaches 2482111348
138367 reaches 2798323360
159487 reaches 17202377752
This cannot be stored in an unsigned long.
#endif

