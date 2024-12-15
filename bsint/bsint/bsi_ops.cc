#include "bsint.h"

#define	inline
#define	BSINT_FROM_C	1
#define	BSINT_FROM_H	0		
#include "bsi_ops.h"
#undef	inline

#include <math.h>

bsint	Q, R;
int		bsint::QRNSerial = 0;
int		bsint::QRDSerial = 0;

bsint::DigitRep	**	bsint::FreeCache = 0;
int				bsint::FreeCacheSize = 0;
int				bsint::NumCached = 0;
int				bsint::NumberSize = 128;
int				bsint::NextSerialNumber = 1;
bsint	* 		bsint::head = 0;
bsint	* 		bsint::tail = 0;

void	bsintSetSizes( int numSize, int cacheSize, int base )
{
	//
	// Any bsints already allocated will be trashed.  We have to go through and
	// deal with this.
	//
	for ( bsint * ip = bsint::head; ip; ip = ip->next )
	{
		if ( ip->dp && --ip->dp->refs == 0 )
			delete[] (unsigned long *)(ip->dp);
	}
	while ( bsint::NumCached )
		delete[] (unsigned long *)(bsint::FreeCache[--bsint::NumCached]);

	if ( cacheSize && bsint::FreeCacheSize == 0 )
	{
		bsint::FreeCache = new bsint::DigitRep *[cacheSize];
		bsint::FreeCacheSize = cacheSize;
	}
	
	bsint::NumCached = 0;
	bsint::NumberSize = int((numSize * log(base) / log(2) + 31)/32) * 2;
	
	for ( ip = bsint::head; ip; ip = ip->next )
	{
		ip->dp = (bsint::DigitRep *)new unsigned long [sizeof(bsint::DigitRep)/sizeof(long) + bsint::NumberSize];
		ip->dp->refs = 1;
		ip->dp->used = 1;
		ip->dp->digit[0] = 0;
		ip->setSerialNumber();
		bsint::QRNSerial = bsint::QRDSerial = 0;
	}
}

int		bsintGetNumberSize( int base )
{
	if ( base == 0 )
		return bsint::NumberSize / 2;
	else
		return bsint::NumberSize * 32 * log(2) / log(base) / 2;
}

int		bsintGetNumberCached( void )
{
	return bsint::NumCached;
}
	
void SetRandom( bsint & U, int numBits )
{
	int		newSize = (numBits+31)/32;
	numBits %= 32;
	U.setSize( newSize );
	bui_setrandom( U, newSize );
	if ( numBits )
		U[0] &= ~(0xFFFFFFFF << numBits);
	U.setSign( POSITIVE );
	U.removeLeadingZeros();
}

const int maxDigits = 512;
char outDigits[maxDigits+5];

char * ConvertToReversedDecimalString( const bsint & U )
{
	bsint			q = U;
	unsigned long	r;
	int				i = 0;
	
	if ( q.getSize() == 1 && q[0] == 0 )
	{
		outDigits[i++] = '0';
		goto done;
	}
	while ( q.getSize() != 1 || q[0] != 0 )
	{
		r = bui_div_digit( q, 10 );
		outDigits[i] = r + '0';
		if ( ++i >= maxDigits )
		{
			outDigits[i++] = '.';
			outDigits[i++] = '.';
			outDigits[i++] = '.';
			break;
		}
	}
	
done:
	if ( U.getSign() == NEGATIVE )
		outDigits[i++] = '-';
	outDigits[i++] = '\0';
	return outDigits;
}


bsint	pow( const bsint & Ua, const bsint & Va )
{
	bsint	result;
	bsint	U = Ua;
	bsint	V = Va;
	
	bui_pow(result,U,V);
	if ( U.getSign() == NEGATIVE && (V[V.getSize()-1] & 1) != 0 )
		result.setSign(NEGATIVE);
	else
		result.setSign(POSITIVE);
	return result;
}

bsint	powmod( const bsint & Ua, const bsint & Va, const bsint & M )
{
	bsint	result,tmp;
	bsint	U = Ua, V = Va;
	bui_powmod(result,U,V,M);
	result.setSign(POSITIVE);
	if ( U.getSign() == NEGATIVE && (V[V.getSize()-1] & 1) != 0 )
	{
		tmp = M;
		result = tmp - result;
	}
	return result;
}

bsint	mod_mp( const bsint & U, int p )
{
	bsint	result = U;
	bui_mod_mp(result,p);
	return result;
}
