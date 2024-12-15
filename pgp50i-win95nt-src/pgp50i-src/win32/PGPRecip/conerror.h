/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
// Header File for console error output
// Mark Elrod
// Monday, February 12 5:58 a.m. Yikes!!!


// foreground colors
#define fg_blue			FOREGROUND_BLUE
#define fg_green		FOREGROUND_GREEN
#define fg_red			FOREGROUND_RED

#define fg_orange    	FOREGROUND_RED 		| FOREGROUND_INTENSITY
#define fg_brite_green  FOREGROUND_GREEN 	| FOREGROUND_INTENSITY
#define fg_navy_blue    FOREGROUND_BLUE 	| FOREGROUND_INTENSITY

#define fg_unc_blue		FOREGROUND_BLUE 	| FOREGROUND_GREEN
#define fg_cyan			FOREGROUND_BLUE 	| FOREGROUND_GREEN 	\
						| FOREGROUND_INTENSITY

#define fg_purple    	FOREGROUND_BLUE 	| FOREGROUND_RED
#define fg_pink         FOREGROUND_BLUE 	| FOREGROUND_RED   	\
						| FOREGROUND_INTENSITY

#define fg_puke         FOREGROUND_GREEN 	| FOREGROUND_RED
#define fg_yellow		FOREGROUND_GREEN 	| FOREGROUND_RED	\
						| FOREGROUND_INTENSITY


#define fg_white     	FOREGROUND_RED 	| FOREGROUND_BLUE 	\
						| FOREGROUND_GREEN	| FOREGROUND_INTENSITY
#define fg_grey     	FOREGROUND_RED 	| FOREGROUND_BLUE   | FOREGROUND_GREEN
#define fg_dk_grey      FOREGROUND_INTENSITY
#define fg_black        BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED

#define fg_hilite		FOREGROUND_INTENSITY

//background colors
#define bg_blue			BACKGROUND_BLUE
#define bg_green		BACKGROUND_GREEN
#define bg_red			BACKGROUND_RED

#define bg_orange    	BACKGROUND_RED 	| BACKGROUND_INTENSITY
#define bg_brite_green  BACKGROUND_GREEN 	| BACKGROUND_INTENSITY
#define bg_navy_blue    BACKGROUND_BLUE 	| BACKGROUND_INTENSITY

#define bg_unc_blue		BACKGROUND_BLUE 	| BACKGROUND_GREEN
#define bg_cyan			BACKGROUND_BLUE 	| BACKGROUND_GREEN 	\
						| BACKGROUND_INTENSITY

#define bg_purple    	BACKGROUND_BLUE 	| BACKGROUND_RED
#define bg_pink         BACKGROUND_BLUE 	| BACKGROUND_RED   	\
						| BACKGROUND_INTENSITY

#define bg_puke         BACKGROUND_GREEN 	| BACKGROUND_RED
#define bg_yellow		BACKGROUND_GREEN 	| BACKGROUND_RED	\
						| BACKGROUND_INTENSITY


#define bg_white     	BACKGROUND_RED 	| BACKGROUND_BLUE 	\
						| BACKGROUND_GREEN	| BACKGROUND_INTENSITY
#define bg_grey     	BACKGROUND_RED 	| BACKGROUND_BLUE   | BACKGROUND_GREEN
#define bg_dk_grey      BACKGROUND_INTENSITY

#define bg_black        FOREGROUND_RED 	| FOREGROUND_BLUE 	\
						| FOREGROUND_GREEN	| FOREGROUND_INTENSITY

#define bg_hilite		BACKGROUND_INTENSITY

DWORD vErrorOut(WORD attrib, char* text, ...);
