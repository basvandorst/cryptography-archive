/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpMacTestMain.c,v 1.3 1997/10/08 02:27:24 mhw Exp $
____________________________________________________________________________*/

#include "pgpConfig.h"
#include "pgpTest.h"



	static void
InitMac()
{
	UnloadScrap();
	
	InitGraf( &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( nil );
	InitCursor();
	MaxApplZone();
}


	static void
RealMain()
	{
	TestMain();
	}
	
	
	void
main(void)
{
	InitMac();
	
	RealMain();
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
