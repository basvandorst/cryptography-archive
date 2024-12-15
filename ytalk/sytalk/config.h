/*			   NOTICE
 *
 * Copyright (c) 1990 Britt Yenne.  All rights reserved.
 * 
 * This software is provided AS-IS.  The author gives no warranty,
 * real or assumed, and takes no responsibility whatsoever for any 
 * use or misuse of this software, or any damage created by its use
 * or misuse.
 * 
 * This software may be freely copied and distributed provided that
 * no part of this NOTICE is deleted or edited in any manner.
 * 
 */

/* Mail comments or questions to yenne@ccwf.cc.utexas.edu */

/* ------------------------------------------------------------------
 * This file contains system configuration parameters for customizing
 * ytalk for your particular environment.  You may need to modify
 * this file.
 * ------------------------------------------------------------------
 */

/* If you are using a POSIX system (or a POSIX compatible system), it
 * MAY be necessary to uncomment the next line.
 */
/*#define POSIX /**/

/* This is for HP systems */
/* #define HPUX /**/

/* If the touchoverlap() function is not defined on your system, then
 * uncomment the next line.
 */
/* #define NOTOUCHOLAP /**/

/* If the cbreak() function is not defined on your system, then
 * uncomment the next line.
 */
/* #define NOCBREAK */

/*  Path to the PGP program.  This should be absolute to prevent a
 * common class of Trojan attacks.
 */

#define PGPPROG "/usr/local/bin/pgp"

/* EOF */
