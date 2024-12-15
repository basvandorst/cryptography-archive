/*_____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: install.h,v 1.30.6.2 1999/06/07 21:27:25 build Exp $
_____________________________________________________________________________*/
#pragma once


/*_____________________________________________________________________________
	This is the common header file for the install dll.   It contains flags
	that apply to the install it is being used with.  Before building, make
	sure the flags here are set appropriately.
	
_____________________________________________________________________________*/


#define PGPCERTD					0		// 1 yes 0 no
#define PGPDEBUG					0		// 1 yes 0 no
//This is used to set _asm { int 3 }; 
//for debugging
#define SETASMINT					0       // 1 yes 0 no
		