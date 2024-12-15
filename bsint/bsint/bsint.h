#ifndef	BSINT_H	//{
#define	BSINT_H

//
// Symantec C++ has some problems debugging code that has any inline
// functions.  Defining BSINT_DISABLE_INLINE as 1 will get rid of
// all the inline functions.
//

#define	BSINT_DISABLE_INLINE			0

//
// Several groups of functions can be selectively inlined or not under
// the control of the following several definitions.
//
#define	BSINT_INLINE_ALLOC			(0 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_LIST			(0 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_SERIAL			(0 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_CONSTRUCTORS	(1 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_INFO			(1 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_WRITE			(0 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_EXPANDERS		(0 && ! BSINT_DISABLE_INLINE)

#define	BSINT_INLINE_PLUS_EQ		(1 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_MINUS_EQ		(1 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_TIMES_EQ		(1 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_DIV_EQ			(1 && ! BSINT_DISABLE_INLINE)

#define	BSINT_INLINE_PLUS			(1 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_MINUS			(1 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_TIMES			(1 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_DIV			(1 && ! BSINT_DISABLE_INLINE)

#define	BSINT_INLINE_EQ				(1 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_NOT_EQ			(1 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_LT				(1 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_LT_EQ			(1 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_GT				(1 && ! BSINT_DISABLE_INLINE)
#define	BSINT_INLINE_GT_EQ			(1 && ! BSINT_DISABLE_INLINE)

#define	BSINT_MULB_CUT		20

class bsint;

void	bsintSetSizes( int numSize = 1024, int cacheSize = 0, int base = 10 );
int 	bsintGetNumberSize( int base = 10 );
int		bsintGetNumberCached( void );

	//
	// Functions that operate on bsints.
	//
void 	SetRandom( bsint & U, int numBits );
char *	ConvertToReversedDecimalString( const bsint & U );
bsint	pow( const bsint & U, const bsint & V );
bsint	powmod( const bsint & U, const bsint & V, const bsint & M );
bsint	mod_mp( const bsint & U, int p );

enum bsint_sign { NEGATIVE=0, POSITIVE=1 };
	
class bsint
{
	struct DigitRep
	{
		int				refs;		// how many references to this string
		int				used;		// how many digits are used
		int				serial;		// serial number
		unsigned long	digit[1];	// the actual digits
	};
	
	static DigitRep	**	FreeCache;
	static int			FreeCacheSize;
	static int			NumCached;
	static int			NumberSize;
	static int			NextSerialNumber;
	
	static bsint * 		head;		// linked list of all existing bsints
	static bsint * 		tail;
	
	static int			QRNSerial, QRDSerial;
	
	DigitRep		* dp;		// pointer to digits
	bsint_sign		sign;		// sign of this integer
	bsint			* next;		// links for placing on list
	bsint			* prev;
	
	void	newDigitRep( void );
	void 	freeDigitRep( void );
	void	insertOnList( void );
	void	deleteFromList( void );
	void 	setSerialNumber( void );
	
public:
	bsint( void );							// Constructor: bsint foo;
	bsint( const int initValue );			// Constructor: bsint foo = 12;
	bsint( const bsint & init );			// Constructor: bsint bar = foo;
	bsint( const char * str );				// Constructor: bsint foo = "12";
	
	bsint & operator=( const bsint & U );	// Assignment: foo = bar;
	
	~bsint();
	
	operator int( void );
	
	int	bsintBad( void );					// Test for bad DigitRep
	
	//
	// Routines that the arithmetic functions can use to manipulate the digit
	// information
	//
		//
		// needModify() and needWrite() deal with unlinking when we need to modify
		// or overwrite a shared DigitRep.  They update the serial number of the
		// DigitRep.
		//
	void 	needModify( void );
	void 	needOverwrite( void );
	
	int		bsintBad( void ) const;
		//
		// x[i]		Read and write digits, 0 based indexing
		// x(i)		Read digits, 1 based indexing
		//
	unsigned long &		operator[]( int index );
	unsigned long &		operator()( int index );

		//
		// Manipulate the number of digits.
		//
	void	setSize( int newUsed );
	int		getSize( void ) const;
	int		getSerial( void ) const;
	
		//
		// Manipulate the number of digits, by adding or removing them from the
		// ends.
		//
	void insertDigit( unsigned long newDigit = 0 );
	void appendDigit( unsigned long newDigit = 0, int repeat = 1 );
	void removeLeadingZeros( void );
	
	bsint_sign	getSign( void ) const;
	void		setSign( bsint_sign newSign );
	void		changeSign( void );
	
	bsint &	operator+=( const bsint & U );
	bsint &	operator-=( const bsint & U );
	bsint &	operator*=( const bsint & U );
	bsint &	operator/=( const bsint & U );
	bsint &	operator%=( const bsint & U );
	
	bsint & operator+=( const int U );
	bsint & operator-=( const int U );
	bsint & operator*=( const int U );
	bsint & operator/=( const int U );
	bsint & operator%=( const int U );
	
	bsint operator++( void );
	bsint operator++( int );
	bsint operator--( void );
	bsint operator--( int );
	
	friend void		test( void );
	friend void		bsintSetSizes( int numSize, int cacheSize, int base );
	friend int 		bsintGetNumberSize( int base );
	friend int 		bsintGetNumberCached( void );
};

	
bsint	operator+( const bsint & U, const bsint & V );
bsint	operator-( const bsint & U, const bsint & V );
bsint	operator*( const bsint & U, const bsint & V );
bsint	operator/( const bsint & U, const bsint & V );
bsint	operator%( const bsint & U, const bsint & V );

bsint	operator+( const bsint & U, const int V );
bsint	operator-( const bsint & U, const int V );
bsint	operator*( const bsint & U, const int V );
bsint	operator/( const bsint & U, const int V );
bsint	operator%( const bsint & U, const int V );
	
bsint	operator+( const int U, const bsint & V );
bsint	operator-( const int U, const bsint & V );
bsint	operator*( const int U, const bsint & V );
int		operator/( const int U, const bsint & V );
bsint	operator%( const int U, const bsint & V );
	
int		operator==( const bsint & U, const bsint & V );
int		operator<=( const bsint & U, const bsint & V );
int		operator<( const bsint & U, const bsint & V );
int		operator!=( const bsint & U, const bsint & V );
int		operator>( const bsint & U, const bsint & V );
int		operator>=( const bsint & U, const bsint & V );

int		operator==( const bsint & U, const int V );
int		operator<=( const bsint & U, const int V );
int		operator<( const bsint & U, const int V );
int		operator!=( const bsint & U, const int V );
int		operator>( const bsint & U, const int V );
int		operator>=( const bsint & U, const int V );

int		operator==( const int U, const bsint & V );
int		operator<=( const int U, const bsint & V );
int		operator<( const int U, const bsint & V );
int		operator!=( const int U, const bsint & V );
int		operator>( const int U, const bsint & V );
int		operator>=( const int U, const bsint & V );

bsint	operator-( const bsint & U );
	
void 	SetRandom( bsint & U, int numBits );
char * 	ConvertToReversedDecimalString( const bsint & U );
bsint	pow( const bsint & U, const bsint & V );
bsint	powmod( const bsint & U, const bsint & V, const bsint & M );
bsint	mod_mp( const bsint & U, int p );
	
void	bui_add( bsint & W, const bsint & U, const bsint & V );
void	bui_sub( bsint & W, const bsint & U, const bsint & V );
void	bui_mul0( bsint & W, const bsint & U, const bsint & V );
void	bui_mul1( bsint & W, const bsint & U, const bsint & V, int cut = BSINT_MULB_CUT );
void	bui_div( bsint & Q, bsint & R, const bsint & U, const bsint & V );
void	bui_pow( bsint & W, const bsint & U, const bsint & V );
void	bui_powmod( bsint & W, const bsint & U, const bsint & V, const bsint & M );
void	bui_mod_mp( bsint & W, int p );
int		bui_equal( const bsint & U, const bsint & V );
int		bui_less( const bsint & U, const bsint & V );
int		bui_less_or_equal( const bsint & U, const bsint & V );

void	bui_split( bsint & H, bsint & L, bsint & U, int nDigits );

void	bui_dumpnumber( const bsint & Q );
void	bui_setrandom( bsint & W, int ndigits );

void			bui_add_digit( bsint & W, unsigned long change );
void			bui_sub_digit( bsint & W, unsigned long change );
void			bui_mul_digit( bsint & W, unsigned long multiplier );
unsigned long	bui_div_digit( bsint & Q, unsigned long divisor );

//
// Pick up definition of inline functions by including bsi_mf.h and bsi_of.h.
// Defining BSINT_FROM_H as 1 causes those include files to include the code
// for the inline functions.  Defining BSINT_FROM_C as 0 causes those include
// files to omit code for non-inline functions.
//
#define	BSINT_FROM_H	1
#define BSINT_FROM_C 	0

#include "bsi_ops.h"

#undef	BSINT_FROM_C
#undef	BSINT_FROM_H

#endif			//}
