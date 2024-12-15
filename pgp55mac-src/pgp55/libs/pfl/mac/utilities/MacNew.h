/*____________________________________________________________________________
	Debug.c: general purpose debugging code.
____________________________________________________________________________*/
#pragma once

/*____________________________________________________________________________
	'USE_PGP_OPERATOR_NEW' controls whether we use our own operator new() and
	operator delete().  By default, this flag is set.
	
	The dummy class is referenced elsewhere in order to ensure that MacNew.cp
	has been linked in.
____________________________________________________________________________*/

#ifndef USE_PGP_OPERATOR_NEW
	#error you must define 'USE_PGP_OPERATOR_NEW' as 0 or 1
#endif



#if USE_PGP_OPERATOR_NEW	// [ 

class MacNewDummy
{
	public:
		MacNewDummy( void );
};

#endif	// ]