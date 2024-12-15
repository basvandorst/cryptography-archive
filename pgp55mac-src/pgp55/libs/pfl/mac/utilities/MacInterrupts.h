/*____________________________________________________________________________
	Copyright (C) 1996 Pretty Good Privacy, Inc.
	All rights reserved.
____________________________________________________________________________*/


#pragma parameter __D0 GetInterruptMask( )
extern pascal short	GetInterruptMask( void )	\
	THREEWORDINLINE( 0x40C0, 0x0240, 0x0700 );


	inline Boolean
InterruptsAreEnabled()
	{
	return( GetInterruptMask() == 0 );
	}
	
	
#if ! GENERATINGPOWERPC	// [
 
#pragma parameter __D0 InterruptsOff( )
extern pascal short	InterruptsOff( void )	\
	THREEWORDINLINE( 0x40C0, 0x007C, 0x0700 );
	
	
#pragma parameter RestoreInterrupts( __D0 )
extern pascal void		RestoreInterrupts( short sr )
	ONEWORDINLINE( 0x46C0 );


#define EnterCriticalSection()	{ short saveSR	= InterruptsOff()
#define ExitCriticalSection()	 RestoreInterrupts( saveSR ); }


Boolean	TestAndSet( volatile Boolean *value );


#endif	// ]


