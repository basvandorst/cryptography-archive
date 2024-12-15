/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#ifdef KEYSERVER_MAIN
#define EXTERN
#else
#define EXTERN extern
#endif;

HINSTANCE EXTERN g_hinst
#ifdef KEYSERVER_MAIN
	 = NULL
#endif
	 ;
