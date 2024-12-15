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
 * $Author: marco $
 * $Id: config.h,v 2.4 1991/11/26 12:23:29 marco Exp marco $
 * $Date: 1991/11/26 12:23:29 $
 * $Revision: 2.4 $
 */

#define	SYSV		/* Delete if not a System V (i.e. bsd4.2)	*/

	/*
	 * Copy of /etc/passwd to allow passord change check
	 */
#ifdef	DEBUG
#define	UPDF		"./pwd_perm"
#else
#define	UPDF		"/usr/etc/security/pwd_perm"
#endif	DEBUG
#define	SUPDF		"/usr/etc/security/pwd_perm.secure"

	/*
	 * Default scheck db file
	 */
#define	SPERM		"/usr/etc/security/s_perm" /* File of legal s-Bit */

	/*
	 * Inetd Log File
	 */
#define	INETDLOG	"/usr/adm/inetd.log"

	/*
	 * Su operations log
	 */
#define	SULOG		"/usr/adm/sulog"

	/*
	 * Minimal interest [GU]ID (test <=)
	 */
#define	MINIMAL_UID	170
#define	MINIMAL_GID	13

	/*
	 * Find Program
	 */
#ifdef	BFIND
#define	FIND		BFIND
#else	BFIND
#define	FIND		"/bin/find"
#endif	BFIND


	/*
	 * Default poll time (where required)
	 */
#define	LOGCHK_POLL	60
#define	SUCHK_POLL	60
#define	PWDCHK_POLL	120
#define	PROC_POLL	120

	/*
	 * Password file
	 */
#define	PASSWORD_FILE		"/etc/passwd"
#define	SPASSWORD_FILE		"/.secure/etc/passwd"

	/*
	 * Miscellaneous
	 */
#ifndef	UTMP_FILE
#define	UTMP_FILE	"/etc/utmp"
#endif	UTMP_FILE

#ifndef	WTMP_FILE
#ifdef	SYSV
#define	WTMP_FILE	"/etc/wtmp"
#else	SYSV
#define	WTMP_FILE	"/usr/adm/wtmp"
#endif	SYSV
#endif	WTMP_FILE

/*
 * Define this if you not have YP and you want to use fast pwent package
 */
#ifdef	SYSV
#define	FAST_PWENT
#endif	SYSV

/*
 * Define this if you have audit id & Flags in struct passwd
 */
#define	AUDIT
