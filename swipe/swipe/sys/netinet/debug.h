/*
 * The author of this software is John Ioannidis, ji@cs.columbia.edu.
 * Copyright (C) 1993, by John Ioannidis.
 *	
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTY. IN PARTICULAR, THE AUTHOR DOES MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE
 * MERCHANTABILITY OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR
 * PURPOSE.
 */


#ifdef DEBUGSW

#define DSW_SENDINGTO	0x0001
#define DSW_NOOPOL	0x0010
#define DSW_NOKEY	0x0020
#define DSW_TRYROUTE	0x0100
#define DSW_ROUTECHECK	0x0200
#define DSW_SHOWROUTE	0x0400
extern unsigned long debugsw;

#endif DEBUGSW

#ifdef SWIPEDEBUG

#define SWD_SHORTENCAP	0x0001
#define SWD_SHORTHDR	0x0002
#define SWD_PRTHDR	0x0004
#define SWD_NOKEYS	0x0010
#define SWD_NODCRKEY	0x0020
#define SWD_NOAUTHKEY	0x0040
#define SWD_BADEALG	0x0100
#define SWD_BADAALG	0x0200

extern unsigned long swipedebug;

#endif
