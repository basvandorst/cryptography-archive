/*____________________________________________________________________________
____________________________________________________________________________*/

#pragma once

#if USE_MAC_DEBUG_LEAKS	// [

/*____________________________________________________________________________
	instantiating this class forces two lists in LPeriodical to be created
	typical use example:
	#if USE_MAC_DEBUG_LEAKS
	{CForceInitLPeriodical	dummy;}
	#endif
____________________________________________________________________________*/
// instantiating this class forces two lists in LPeriodical to be created
// typical use example:
class CForceInitLPeriodical : LPeriodical
{
public:
	CForceInitLPeriodical( void )
		{ StartIdling(); StartRepeating();}
	virtual void	SpendTime(const EventRecord &/*inMacEvent*/)	{}
};

#endif	// ]










