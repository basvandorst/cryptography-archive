/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: Jimi.h,v 1.2.10.1 1997/12/05 22:15:18 mhw Exp $
____________________________________________________________________________*/

#pragma once 

typedef pascal void (*JimiInitProcPtr) (Handle inJimiResH);
typedef pascal void	(*InitMenusProc)(void);

pascal	void	MyInitMenus(void);
