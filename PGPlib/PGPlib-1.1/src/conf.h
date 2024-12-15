/*********************************************************************
 * Filename:      conf.h
 * Description:   
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Tue May 13 02:13:43 1997
 * Modified by:   Tage Stabell-Kulo <tage@acm.org>
 * 	
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/



#ifndef _CONF_H_
#define _CONF_H_

extern char *LIB_VERSION;
extern char *PGP_VERSION;
extern char *PGP_COMMENT;

/* 
 * armor.c
 */

#define MAXLINE		1024
#define ARMORLINE	64
#define DATALEN	   	(ARMORLINE/4*3)

/* 
 * conventional.c
 */

/* We will not handle packets larger than this */
#define	MAX_ENC_PACKET	(1024*1024)

/*
 * keybase.c
 */

/* Tailor the database, see hash(3); */
#define CACHE_SIZE	(1024*1024)
#define NELEM		(40000)
#ifndef BSIZE
#  define BSIZE		(4096)
#endif

#endif 
