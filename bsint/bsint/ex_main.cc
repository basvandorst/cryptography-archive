#include <iostream.h>
#include "bsint.h"
#include "bsint_io.h"
#include <math.h>

void	main( void )
{
	const int	ndigits = 50;
	bsint	x = 1;
	
	bsint	factorial = 1;
	bsint	x_to_n = 1;
	bsint	one = pow(bsint(10),bsint(ndigits+4));
	bsint	sum = one;
	bsint	newTerm = 1;
	for ( bsint n = 1; newTerm != 0; n++ )
	{
		factorial *= n;
		x_to_n *= x;
		newTerm = one * x_to_n / factorial;
		sum += newTerm;
		cout << n << ' ' << sum << endl;
	}
}

void	test( void )
{
	bsint	* ip;
	extern void bui_dump( const bsint & );
	
	for ( ip = bsint::head; ip; ip = ip->next )
	{
		cout << ip->prev << ' ' << ip << ' ' << ip->next << ' ';
		cout << ip->dp->used << ' ' << ip->dp->refs << ' ';
		bui_dump( *ip ); cout << endl;
	}
}
