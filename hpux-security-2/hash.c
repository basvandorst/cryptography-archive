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
 * @(#) hash.c : General Hash
 *
 * By Marco	Milano 1988
 */
/*
 * $Author: marco $
 * $Id: hash.c,v 2.2 1991/10/21 16:44:09 marco Exp marco $
 * $Date: 1991/10/21 16:44:09 $
 * $Revision: 2.2 $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: hash.c,v 2.2 1991/10/21 16:44:09 marco Exp marco $";
#endif	lint

#include <sys/types.h>
#include "hash.h"
#include "config.h"

#define	NULL		((hash)	0)

hash		Htable[HASHSIZE];

extern hash 	malloc();

void Hinit()
{
	memset(Htable, 0, sizeof(Htable));
}

static short Hhash(name)
char *name;
{
	register int	tot;
	register char	*p;

	for (tot = 0, p = name; *p; tot += (*p << 3) ^ *p, p++);
	return tot & HASHMASK;
}

hash	Hfind(name)
char	*name;
	/*
	 * Find name on hash
	 */
{
	hash p;

	for (p = Htable[Hhash(name)]; p && strcmp(p->name, name); p = p->next);

	return p;
}

int	Hinsert(name, uid, gid, mode, mtime, size, ino, dev, checksum)
char	*name;
u_short	uid, 
	gid,
	mode;
time_t	mtime;
u_long	size,
	checksum;
long	ino,
	dev;
{
	hash	p;
	int	entry;

	for (p = Htable[entry = Hhash(name)]; 
		p && strcmp(p->name, name); p = p->next);

	if (! p) {
		if ((p = (hash) malloc(sizeof(*p))) == NULL)
			return -1;

		p->next 	= Htable[entry];
		Htable[entry] 	= p;
	}

	p->name		= name;
	p->uid		= uid;
	p->gid		= gid;
	p->mode		= mode;
	p->mtime	= mtime;
	p->size		= size;
	p->ino		= ino;
	p->dev		= dev;
	p->checksum	= checksum;

	return 1;
}


#ifdef DEBUG
void Htest()
{
	register int i;
	hash p;
	int	tot = 0;
	int	max = 0;
	int	empty = 0;

	for (i = 0; i < HASHSIZE; i++) {
		register int j;

		if (! Htable[i]) {
			empty++;
			continue;
		}
		for (j = 0, p = Htable[i]; p; j++, p = p->next);
		tot += j;
		if (j > max)
			max = j;
	}
	printf("\nTot	%5d\n", tot);
	printf("Max	%5d\n", max);
	printf("Empty	%5d\n", empty);
	printf("! Empty	%5d\n", HASHSIZE - empty + 1);
	printf("Average	%5.2f\n", tot / (float) HASHSIZE);
	printf("R. Av.	%5.2f\n", tot / (float) (HASHSIZE - empty + 1));
	printf("Av/RAv	%9.4f%%\n", 100 * ((tot / (float) HASHSIZE) / 
				(tot / (float) (HASHSIZE - empty + 1))));
}
#endif
