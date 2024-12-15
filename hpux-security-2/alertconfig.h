/*
 * Copyright (c) 1990 Marco Negri & siLAB Staff
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the authors not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission of the authors.
 * The authors makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Please send any improvements, bug fixes, useful modifications, and comments
 * to security@ghost.unimi.it
 */
/*
 * @(#)alert.c	: Send "Alert Messages" to System Managers (Rel. 2.0)
 * @(#)alert.c	: By Marco & Carlo	Milano, 10/89, 12/90
 */
/*
 * $Author: marco $
 * $Id: alertconfig.h,v 2.2 1991/10/21 16:44:09 marco Exp marco $
 * $Date: 1991/10/21 16:44:09 $
 * $Revision: 2.2 $
 */

/*
 * Max. # of people supported in alert.list.
 */
#define	MAXPEOPLE	128

/*
 * The alert.list itself.
 */
#ifdef	DEBUG
#define CONFFILE	"alert.list"
#else	DEBUG
#define CONFFILE	"/usr/etc/security/alert.list"
#endif	DEBUG

#ifdef	SENDMAIL
#define	PMAIL		"/usr/lib/sendmail"
#define	MAIL		"sendmail"
#else	SENDMAIL
#define	PMAIL		"/bin/mail"		/* Simple mailer	*/
#define	MAIL		"mail"
#endif	SENDMAIL
