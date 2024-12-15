/*
 * $Author: marco $
 * $Id: fastpwent.h,v 1.1 1991/11/26 12:23:29 marco Exp marco $
 * $Date: 1991/11/26 12:23:29 $
 * $Revision: 1.1 $
 */

#ifndef	FASTPWENT_INCLUDED
#define	FASTPWENT_INCLUDED

/*
 * Fast pw ent mode library
 */

#define	GET_FROM_PASSWD		01
#define	GET_FROM_SHARED		02
#define	GET_FROM_BOTH		(GET_FROM_PASSWD | GET_FROM_SHARED)

#define	PANIC_ON_BAD_ORDER	04

extern int		FastSetPwEnt();
extern int		FastEndPwEnt();
extern struct passwd	*FastGetPwEnt();

#endif	FASTPWENT_INCLUDED
