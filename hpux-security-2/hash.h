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
 * @(#) hash.h : Hash Table Interface for sysmng
 */
/*
 * $Author: marco $
 * $Id: hash.h,v 2.2 1991/10/21 16:44:09 marco Exp marco $
 * $Date: 1991/10/21 16:44:09 $
 * $Revision: 2.2 $
 */

#define HASHSIZE	1024
#define HASHMASK	(HASHSIZE - 1)

typedef struct hash_def {
	char		*name;
	u_short		uid,
			gid,
			mode;
	time_t		mtime;
	u_long		size,
			checksum;
	long		ino,
			dev;
	struct hash_def *next;
} *hash;
