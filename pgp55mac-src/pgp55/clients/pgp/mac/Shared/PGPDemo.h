/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PGPDemo.h,v 1.2 1997/06/30 10:38:08 wprice Exp $
____________________________________________________________________________*/
#pragma once

#ifndef PGP_DEMO
PGP_DEMO is defined in PGPmailCompileFlags.h
be sure to #include that file in your prefix
#endif

#if PGP_DEMO	// [

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	kDemoStateUnknown	= 1,
	kDemoStateActive,
	kDemoStateEncryptionSigningDisabled,
	kDemoStateExpired

} PGPDemoState;

PGPDemoState	GetDemoState(void);

// App should quit if ShowDemoDialog() returns kDemoExpired
PGPDemoState	ShowDemoDialog(Boolean canOpenURL);

#ifdef __cplusplus
}
#endif

#endif	// ]