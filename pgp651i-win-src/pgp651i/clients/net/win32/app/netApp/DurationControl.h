/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	DurationControl.h - Duration edit custom control
	
	$Id: DurationControl.h,v 1.2 1999/04/01 14:53:55 pbj Exp $
____________________________________________________________________________*/

#ifndef _PGP_DURATIONCONTROL_H
#define _PGP_DURATIONCONTROL_H

#define WC_PGPDURATION			("PGPDurationControl32")

#define DAYS		0
#define HOURS		1
#define MINS		2
#define NUM_FIELDS	3

#define PGP_DCM_FIRST			0x2000
#define PGP_DCM_SETDURATION		PGP_DCM_FIRST+0
#define PGP_DCM_GETDURATION		PGP_DCM_FIRST+1
#define PGP_DCM_SETMINDURATION	PGP_DCM_FIRST+2
#define PGP_DCM_SETMAXDURATION	PGP_DCM_FIRST+3
#define PGP_DCM_SETTEXT			PGP_DCM_FIRST+4

#define PGP_DCN_FIRST           (0U-100U)       
#define PGP_DCN_LAST            (0U-199U)

VOID WINAPI 
InitPGPDurationControl (VOID); 

#endif //_PGP_DURATIONCONTROL_H