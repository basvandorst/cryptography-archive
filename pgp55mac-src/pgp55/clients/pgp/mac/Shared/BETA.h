/*____________________________________________________________
	(C) 1996 PGP Inc. All Rights Reserved
____________________________________________________________*/
#pragma once


#ifndef BETA
BETA is defined in PGPmailCompileFlags.h
be sure to #include that file in your prefix
#endif

#if BETA

#ifdef __cplusplus
extern "C" {
#endif

Boolean		BetaExpired( void );

#ifdef __cplusplus
}
#endif

#endif
