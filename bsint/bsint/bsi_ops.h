#if (BSINT_FROM_H && BSINT_INLINE_ALLOC) || (BSINT_FROM_C && ! BSINT_INLINE_ALLOC) //{

inline void	bsint::newDigitRep( void )
{
	if ( NumCached == 0 )
		dp = (DigitRep *)new unsigned long [sizeof(DigitRep)/sizeof(long) + NumberSize];
	else
		dp = FreeCache[--NumCached];
	dp->refs = 1;
	dp->used = 1;
	dp->digit[0] = 0;
	setSerialNumber();
}

inline void		bsint::freeDigitRep( void )
{
	if ( NumCached < FreeCacheSize )
		FreeCache[NumCached++] = dp;
	else
		delete[] (unsigned long *)dp;
}

#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_LIST) || (BSINT_FROM_C && ! BSINT_INLINE_LIST) //{

inline void	bsint::insertOnList( void )
{
	next = head;
	prev = 0;
	if ( head == 0 )
		tail = this;
	else
		head->prev = this;
	head = this;
}

inline void	bsint::deleteFromList( void )
{
	if ( next )
		next->prev = prev;
	if ( prev )
		prev->next = next;
	if ( head == this )
		head = next;
	if ( tail == this )
		tail = prev;
}

#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_SERIAL) || (BSINT_FROM_C && ! BSINT_INLINE_SERIAL) //{

inline void		bsint::setSerialNumber( void )
{
	if ( ++NextSerialNumber == 0 )
	{
		QRNSerial = 0;
		QRDSerial = 0;
		++NextSerialNumber;
	}
	dp->serial = NextSerialNumber;
}
#endif //}
	
#if (BSINT_FROM_H && BSINT_INLINE_CONSTRUCTORS) || (BSINT_FROM_C && ! BSINT_INLINE_CONSTRUCTORS) //{
inline bsint::bsint( const bsint & init )
{
	dp = init.dp;
	dp->refs++;
	sign = init.sign;
	insertOnList();
}

inline bsint::bsint( void )
{
	newDigitRep();
	sign = POSITIVE;
	insertOnList();
}

inline	bsint::bsint( const int initValue )
{
	newDigitRep();
	if ( initValue < 0 )
	{
		sign = NEGATIVE;
		dp->digit[0] = -initValue;
	}
	else
	{
		sign = POSITIVE;
		dp->digit[0] = initValue;
	}
	insertOnList();
}

inline	bsint::bsint( const char * str )
{
	newDigitRep();
	if ( *str == '-' )
	{
		sign = NEGATIVE;
		str++;
	}
	else
		sign = POSITIVE;
	insertOnList();
	
	while ( *str != '\0' )
	{
		bui_mul_digit( *this, 10 );
		bui_add_digit( *this, *str++ - '0' );
	}
}

inline bsint & bsint::operator=( const bsint & U )
{
	U.dp->refs++;
	if ( --dp->refs == 0 )
		freeDigitRep();
	dp = U.dp;
	sign = U.sign;
	return *this;
}

inline bsint::~bsint()
{
	if ( dp && --dp->refs == 0 )
		freeDigitRep();
	deleteFromList();
}

inline bsint::operator int( void )
{
	if ( sign == POSITIVE )
		return dp->digit[dp->used-1];
	else
		return -dp->digit[dp->used-1];
}

#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_INFO) || (BSINT_FROM_C && ! BSINT_INLINE_INFO) //{
inline int bsint::getSerial( void ) const
{
	return dp->serial;
}

inline int	bsint::bsintBad( void )
{
	return dp == 0;
}

inline int bsint::getSize( void ) const
{
	return dp->used;
}

inline unsigned long & bsint::operator[]( int index )
{
	return dp->digit[ index ];
}

inline unsigned long & bsint::operator()( int index )
{
	return dp->digit[index-1];
}

inline void bsint::setSize( int newUsed )
{
	dp->used = newUsed;
	setSerialNumber();
}

inline bsint_sign	bsint::getSign( void ) const
{
	return sign;
}

inline void		bsint::setSign( bsint_sign newSign )
{
	sign = newSign;
}

inline void		bsint::changeSign( void )
{
	if ( dp->used != 1 || dp->digit[0] != 0 )
		sign = POSITIVE - sign;
	else
		sign = POSITIVE;	// 0 is always positive
}
	
#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_WRITE) || (BSINT_FROM_C && ! BSINT_INLINE_WRITE) //{
inline void bsint::needModify( void )
{
	if ( dp->refs > 1 )
	{
		DigitRep * op = dp;
		newDigitRep();
		if ( dp )
		{
			op->refs--;
			for ( int i = 0; i < op->used; i++ )
				dp->digit[i] = op->digit[i];
			dp->used = op->used;
		}
	}
	else
		setSerialNumber();
}

inline void bsint::needOverwrite( void )
{
	if ( dp->refs > 1 )
	{
		dp->refs--;
		newDigitRep();
		if ( dp )
			dp->refs = 1;
	}
	else
		setSerialNumber();
}
#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_EXPANDERS) || (BSINT_FROM_C && ! BSINT_INLINE_EXPANDERS) //{
inline void bsint::insertDigit( unsigned long newDigit )
{
	int		i;
	
	for ( i = dp->used; i >= 1; i-- )
		dp->digit[i] = dp->digit[i-1];
	dp->digit[0] = newDigit;
	dp->used++;
}

inline void bsint::appendDigit( unsigned long newDigit, int repeat )
{
	while ( repeat-- > 0 )
		dp->digit[dp->used++] = newDigit;
}

inline void bsint::removeLeadingZeros( void )
{
	for ( int fnz = 0; fnz < dp->used; fnz++ )
		if ( dp->digit[fnz] != 0 )
			break;
	if ( fnz >= dp->used )
	{
		dp->used = 1;
	}
	else
	{
		if ( fnz )
			for ( int i = fnz; i < dp->used; i++ )
				dp->digit[i-fnz] = dp->digit[i];
		dp->used -= fnz;
	}
}
#endif

#if (BSINT_FROM_H && BSINT_INLINE_PLUS_EQ) || (BSINT_FROM_C && ! BSINT_INLINE_PLUS_EQ) //{
inline bsint &	bsint::operator+=( const bsint & U )
{
	if ( sign == U.sign )
		bui_add(*this,*this,U);
	else if ( bui_less( *this, U ) )
	{
		bui_sub(*this,U,*this);
		sign = POSITIVE - sign;
	}
	else
	{
		bui_sub(*this,*this,U);
		if ( dp->used == 1 && dp->digit[0] == 0 )
			sign = POSITIVE;
	}
	return *this;
}

inline bsint &	bsint::operator+=( const int U )
{
	unsigned long absU = U < 0 ? -U : U;
	
	if ( (U >= 0 && sign == POSITIVE) || (U <= 0 && sign == NEGATIVE) )
		bui_add_digit( *this, absU );
	else if ( dp->used > 1 )
		bui_sub_digit( *this, absU );
	else if ( (*this)[0] < absU )
	{
		(*this)[0] = absU - (*this)[0];
		sign = POSITIVE - sign;
	}
	else if ( (*this)[0] > absU )
		(*this)[0] -= absU;
	else
	{
		(*this)[0] = 0;
		sign = POSITIVE;
	}
	return *this;
}

inline bsint	bsint::operator++( void )
{
	*this = *this + 1;
	return *this;
}

inline bsint	bsint::operator++( int )
{
	bsint	result = *this;
	*this = *this + 1;
	return result;
}

#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_MINUS_EQ) || (BSINT_FROM_C && ! BSINT_INLINE_MINUS_EQ) //{
inline bsint &	bsint::operator-=( const bsint & U )
{
	bsint	tmp = U;
	
	tmp.sign = POSITIVE - tmp.sign;
	*this += tmp;
	return *this;
}

inline bsint &	bsint::operator-=( const int U )
{
	*this += -U;
	return *this;
}

inline bsint	bsint::operator--( void )
{
	*this = *this - 1;
	return *this;
}

inline bsint	bsint::operator--( int )
{
	bsint	result = *this;
	*this = *this - 1;
	return result;
}
	
inline bsint	operator-( const bsint & U )
{
	bsint	result = U;
	result.changeSign();
	return result;
}

#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_TIMES_EQ) || (BSINT_FROM_C && ! BSINT_INLINE_TIMES_EQ) //{
inline bsint &	bsint::operator*=( const bsint & U )
{
	bui_mul1(*this,*this,U);
	sign = (sign == U.sign) ? POSITIVE : NEGATIVE;
	return *this;
}

inline bsint &	bsint::operator*=( const int U )
{
	bui_mul_digit( *this, U < 0 ? -U : U );
	if ( U < 0 )
		changeSign();
	return *this;
}
#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_DIV_EQ) || (BSINT_FROM_C && ! BSINT_INLINE_DIV_EQ) //{
inline bsint &	bsint::operator/=( const bsint & U )
{
	extern bsint Q, R;
	if ( dp->serial != QRNSerial || U.dp->serial != QRDSerial )
	{
		bui_div(Q,R,*this,U);
		QRNSerial = dp->serial;
		QRDSerial = U.dp->serial;
	}
	*this = Q;
	
	if ( sign == NEGATIVE && (R.dp->used != 1 || R.dp->digit[0] != 0) )
		bui_add_digit(*this,1);
	sign = (sign == U.sign) ? POSITIVE : NEGATIVE;
	return *this;
}

inline bsint &	bsint::operator/=( const int U )
{
	unsigned long absU = U < 0 ? -U : U;
	unsigned long r;
	
	r = bui_div_digit( *this, absU );
	if ( sign == NEGATIVE && r != 0 )
		*this -= 1;
	if ( (sign == POSITIVE && U < 0) || (sign == NEGATIVE && U > 0) )
		sign = NEGATIVE;
	else
		sign = POSITIVE;
	return *this;
}

inline bsint &	bsint::operator%=( const bsint & U )
{
	extern bsint Q, R;
	if ( dp->serial != QRNSerial || U.dp->serial != QRDSerial )
	{
		bui_div(Q,R,*this,U);
		QRNSerial = dp->serial;
		QRDSerial = U.dp->serial;
	}
	*this = R;
	
	if ( sign == NEGATIVE && (R.dp->used != 1 || R.dp->digit[0] != 0) )
	{
		sign = POSITIVE;
		if ( U.sign == NEGATIVE )
			*this += U;
		else
			*this -= U;
	}
	sign = POSITIVE;
	return *this;
}

inline bsint &	bsint::operator%=( const int U )
{
	unsigned long absU = U < 0 ? -U : U;
	unsigned long r = bui_div_digit( *this, absU );
	if ( sign == NEGATIVE && r != 0 )
		r = absU - r;
	dp->digit[0] = r;
	dp->used = 1;
	sign = POSITIVE;
	return *this;
}
#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_PLUS) || (BSINT_FROM_C && ! BSINT_INLINE_PLUS) //{
inline bsint	operator+( const bsint & U, const bsint & V )
{
	bsint	result = U;
	result += V;
	return result;
}

inline bsint	operator+( const bsint & U, const int V )
{
	bsint result = U;
	result += V;
	return result;
}

inline bsint	operator+( const int U, const bsint & V )
{
	bsint result = V;
	result += U;
	return result;
}
#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_MINUS) || (BSINT_FROM_C && ! BSINT_INLINE_MINUS) //{
inline bsint	operator-( const bsint & U, const bsint & V )
{
	bsint	result = U;
	result -= V;
	return result;
}

inline bsint	operator-( const bsint & U, const int V )
{
	bsint	result = U;
	result -= V;
	return result;
}

inline bsint	operator-( const int U, const bsint & V )
{
	bsint	result = V;
	result -= U;
	result.changeSign();
	return result;
}
#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_TIMES) || (BSINT_FROM_C && ! BSINT_INLINE_TIMES) //{
inline bsint	operator*( const bsint & U, const bsint & V )
{
	bsint	result = U;
	result *= V;
	return result;
}

inline bsint	operator*( const bsint & U, const int V )
{
	bsint	result = U;
	int		factor = V;
	if ( factor < 0 )
	{
		result.changeSign();
		factor = -factor;
	}
	bui_mul_digit( result, factor );
	return result;
}

inline bsint	operator*( const int U, const bsint & V )
{
	return operator*( V, U );
}

#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_DIV) || (BSINT_FROM_C && ! BSINT_INLINE_DIV) //{
inline bsint	operator/( const bsint & U, const bsint & V )
{
	bsint	result = U;
	result /= V;
	return result;
}

inline bsint	operator%( const bsint & U, const bsint & V )
{
	bsint	result = U;
	result %= V;
	return result;
}

inline bsint	operator/( const bsint & U, const int V )
{
	bsint	result = U;
	result /= V;
	return result;
}

inline bsint	operator%( const bsint & U, const int V )
{
	bsint	result = U;
	result %= V;
	return result;
}

inline int		operator/( const int U, const bsint & Va )
{
	int				q;
	bsint			V = Va;
	unsigned long	absU = U < 0 ? -U : U;
	unsigned long	r;
	
	
	if ( V.getSize() > 1 )
	{
		q = 0;
		r = absU;
	}
	else
	{
		q = absU / V[0];
		r = absU % V[0];
	}
	if ( U < 0 && r != 0 )
		q++;
	if ( (U < 0 && V.getSign() == POSITIVE) || (U > 0 && V.getSign() == NEGATIVE) )
		q = -q;
		
	return q;
}

inline bsint	operator%( const int U, const bsint & V )
{
	bsint	result = U;
	return	result % V;
}

#endif //}
	
#if (BSINT_FROM_H && BSINT_INLINE_EQ) || (BSINT_FROM_C && ! BSINT_INLINE_EQ) //{
inline int		operator==( const bsint & U, const bsint & V )
{
	if ( U.getSign() != V.getSign() )
		return 0;
	return bui_equal( U, V );
}

inline int		operator==( const bsint & Ua, const int V )
{
	unsigned long absV = V < 0 ? -V : V;
	bsint U = Ua;
	
	if ( U.getSize() != 1 )
		return 0;
	if ( U[0] != absV )
		return 0;
	if ( (U.getSign() == POSITIVE && V < 0) || (U.getSign() == NEGATIVE && V >= 0) )
		return 0;
	return 1;
}

inline int		operator==( const int U, const bsint & V )
{
	return V == U;
}
#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_LT_EQ) || (BSINT_FROM_C && ! BSINT_INLINE_LT_EQ) //{
inline int		operator<=( const bsint & U, const bsint & V )
{
	if ( U.getSign() == POSITIVE && V.getSign() == NEGATIVE )
		return 0;
	if ( U.getSign() == NEGATIVE && V.getSign() == POSITIVE )
		return 1;
	if ( U.getSign() == POSITIVE )
		return bui_less_or_equal( U, V );
	else
		return bui_less_or_equal( V, U );
}

inline int		operator<=( const bsint & Ua, const int V )
{
	bsint U = Ua;
	unsigned long	absV = V < 0 ? -V : V;
	
	if ( U.getSign() == POSITIVE && V < 0 )
		return 0;
	if ( U.getSign() == NEGATIVE && V >= 0 )
		return 1;
	// signs are the same
	if ( U.getSize() == 1 )
	{
		if ( (U[0] <= absV && V >= 0) || (U[0] >= absV && V < 0) )
			return 1;
		else
			return 0;
	}
	else
	{
		return V < 0;
	}
}

inline int		operator<=( const int U, const bsint & V )
{
	return V >= U;
}
#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_LT) || (BSINT_FROM_C && ! BSINT_INLINE_LT) //{
inline int		operator<( const bsint & U, const bsint & V )
{
	if ( U.getSign() == POSITIVE && V.getSign() == NEGATIVE )
		return 0;
	if ( U.getSign() == NEGATIVE && V.getSign() == POSITIVE )
		return 1;
	if ( U.getSign() == POSITIVE )
		return bui_less( U, V );
	else
		return bui_less( V, U );
}

inline int		operator<( const bsint & Ua, const int V )
{
	bsint U = Ua;
	unsigned long	absV = V < 0 ? -V : V;
	
	if ( U.getSign() == POSITIVE && V < 0 )
		return 0;
	if ( U.getSign() == NEGATIVE && V >= 0 )
		return 1;
	// signs are the same
	if ( U.getSize() == 1 )
	{
		if ( (U[0] < absV && V >= 0) || (U[0] > absV && V < 0) )
			return 1;
		else
			return 0;
	}
	else
	{
		return V < 0;
	}
}

inline int		operator<( const int U, const bsint & V )
{
	return V > U;
}

#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_NOT_EQ) || (BSINT_FROM_C && ! BSINT_INLINE_NOT_EQ) //{
inline int		operator!=( const bsint & U, const bsint & V )
{
	return ! (U == V);
}

inline int		operator!=( const bsint & U, const int V )
{
	return ! (U == V);
}

inline int		operator!=( const int U, const bsint & V )
{
	return ! (U == V);
}
#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_GT) || (BSINT_FROM_C && ! BSINT_INLINE_GT) //{
inline int		operator>( const bsint & U, const bsint & V )
{
	return ! (U <= V);
}

inline int		operator>( const bsint & U, const int V )
{
	return ! (U <= V);
}

inline int		operator>( const int U, const bsint & V )
{
	return ! (U <= V);
}
#endif //}

#if (BSINT_FROM_H && BSINT_INLINE_GT_EQ) || (BSINT_FROM_C && ! BSINT_INLINE_GT_EQ) //{
inline int		operator>=( const bsint & U, const bsint & V )
{
	return ! (U < V);
}

inline int		operator>=( const bsint & U, const int V )
{
	return ! (U < V);
}

inline int		operator>=( const int U, const bsint & V )
{
	return ! (U < V);
}
#endif //}

