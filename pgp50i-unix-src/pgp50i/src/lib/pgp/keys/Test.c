/*
 * Test.c
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: Test.c,v 1.1.2.1 1997/06/07 09:50:33 mhw Exp $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pgpMakeSig.h"
#include "pgpRngPub.h"
#include "pgpRngRead.h"
#include "pgpRingUI.h"
#include "pgpRngMnt.h"
#include "pgpHash.h"
#include "pgpKeySpec.h"
#include "pgpMem.h"
#include "pgpEnv.h"
#include "pgpFile.h"
#include "pgpPubKey.h"
#include "pgpRndom.h"
#include "pgpRndSeed.h"
#include "pgpSigSpec.h"
#include "pgpTimeDate.h"

static void
destructor(struct RingFile *file, struct PgpFile *f, void *arg)
{
		(void)file;
		pgpFileClose(f);
		(void)arg;
}

static void
print_it(struct RingIterator *iter, unsigned level)
{
		union RingObject *obj;

		while (ringIterNextObject(iter, level) > 0) {
				obj = ringIterCurrentObject(iter, level);
				ringObjPrint(stdout, ringIterSet(iter), obj, level*2);
				print_it(iter, level+1);
		}
}

static void
print_check(void *arg, struct RingIterator *iter, int status)
{
		(void)iter;
		status = status < 0 ? '#' : (status ? '!' : '?');
		putc(status, (FILE *)arg);
		fflush((FILE *)arg);
}

static int
progress(void *arg, int c)
{
		putchar(c);
		fflush(stdout);
		return 0;
}

#define RANDFILE "randseed.bin"

static void
randseedread(struct PgpEnv *env)
{
		char *pc = getenv("PGPPATH");
		char *rand;
		FILE *f;
		if (pc) {
				pgpenvSetString(env, PGPENV_PGPPATH, pc,
								PGPENV_PRI_PUBDEFAULT);
				rand = (char *)pgpMemAlloc(strlen(pc) + strlen(RANDFILE)+2);
				if (rand) {
						strcpy(rand, pc);
						strcat(rand, "/");
						strcat(rand, RANDFILE);
						pgpenvSetString(env, PGPENV_RANDSEED, rand,
										PGPENV_PRI_PUBDEFAULT);
						f = fopen(rand, "rb");
						if (f) {
								pgpRandSeedRead(f, NULL);
								fclose(f);
						}
						pgpMemFree(rand);
				}
		}
}

static void
randseedwrite (struct PgpEnv *env)
{
		char const *rand = pgpenvGetString(env, PGPENV_RANDSEED, NULL, NULL);
		FILE *f;

		if (rand) {
				f = fopen(rand, "wb");
				if (f) {
						pgpRandSeedWrite(f, NULL, NULL);
						fclose(f);
				}
		}
}


int
main(int argc, char **argv)
{
		FILE *ff;
		FILE *ff2;
		struct PgpFile *f, *f2;
		struct RingPool *pool;
		struct RingFile *file;
		struct RingSet const *set;
		struct RingIterator *iter;
		unsigned count;
		int error;
		struct PgpEnv *pgpenv = pgpenvCreate();

		if (argc != 2 && argc != 3) {
				fprintf(stderr, "Usage: %s file [destfile]\n", argv[0]);
				return 1;
		}

		pool = ringPoolCreate(NULL);
		if (!pool) {
				perror("Unable to allocate ringpool");
				return 1;
		}
		puts("Pool successfully opened");

		randseedread(pgpenv);

		ff = fopen(argv[1], "rb");
		f = pgpFileReadOpen(ff, NULL, NULL); /*	OK to chain errors */
		if (!ff || !f) {
				perror(argv[1]);
				return 1;
		}
		puts("File successfully opened.");

		file = ringFileOpen(pool, f, 1, &error);
		printf("file = %p error = %d\n", file, error);
		ringFileSetDestructor(file, destructor, (void *)0);
		printf("Trouble = %p\n", (void *)ringFileTrouble(file));
		error = ringTopListTrouble(stdout, file, 0);
		printf("error = %d\n", error);
		set = ringFileSet(file);

		printf("%u signatures to check\n", ringPoolCheckCount(set, set, 1));
		error = ringPoolCheck(set, set, 1, print_check, (void *)stdout);
		printf("Check returned %d\n", error);
	
		iter = ringIterCreate(set);
		printf("iter = %p\n", iter);

		print_it(iter, 1);

		ringIterDestroy(iter);

		count = ringMnt(set, (struct RingSet *)NULL, pgpTimeStamp(0));

#ifndef NOKEYSIGNTEST
		{
		struct PgpSigSpec *ss;
		struct RingSet *ns;
		struct PgpRandomContext *rc = pgpRandomCreate();
		union RingObject *newsig, *key, *name;
		byte *sigbuf;
		int siglen;

		FILE *ffsec = fopen("secring.pgp", "rb");
		struct PgpFile *fsec = pgpFileReadOpen(ffsec, NULL, NULL);
		struct RingFile *secfile = ringFileOpen(pool, fsec, 1, &error);
		union RingObject *secobj = ringLatestSecret(ringFileSet(secfile), "");
		struct PgpSecKey *seckey = ringSecSecKey(ringFileSet(secfile), secobj);
		int err = pgpSecKeyUnlock(seckey, "x", 1);

		printf ("Unlock returns %d\n", err);
		ns = ringSetCreate(pool);
		ringSetAddSet(ns, set);
		if (err > 0) {
				ss = pgpSigSpecCreate(seckey, PGP_HASH_MD5,
#if 1
						PGP_SIGTYPE_KEY_PERSONA,
#else
						PGP_SIGTYPE_KEY_GENERIC,
#endif
						pgpTimeStamp (
								pgpenvGetInt(pgpenv, PGPENV_TZFIX,NULL,NULL)),
						pgpenvGetInt(pgpenv, PGPENV_VERSION, NULL, NULL));
				iter = ringIterCreate(set);
				ringIterNextObject(iter, 1);
#if 0
				/*	Sign key */
				name = ringIterCurrentObject(iter, 1);
#else
				do {
						ringIterNextObject(iter, 2);
						name = ringIterCurrentObject(iter, 2);
				} while (ringObjectType(name)!=RINGTYPE_NAME);
#endif
				printf ("name = %p\n", name);
				sigbuf = (byte *)pgpMemAlloc(pgpMakeSigMaxSize(ss));
				printf ("sigbuf = %p\n", sigbuf);
				siglen = ringSignObj(sigbuf, ns, name, ss, rc);
				printf ("siglen = %d\n", siglen);
				newsig = ringCreateSig(ns, name, sigbuf, siglen);
				printf ("newsig = %p\n", newsig);
				ringSetFreeze(ns);
				iter = ringIterCreate(ns);
				print_it (iter, 1);
				ringIterDestroy(iter);
				set = ns;
		}
		}
#endif

#ifdef NAMETEST
		{
		char *name = "hubert humphrey";
		struct RingSet *ns = ringSetCreate(pool);
		union RingObject *key, *nameobj;
		printf("new RingSet ns = %p\n", ns);
		ringSetAddSet(ns, set);
		iter = ringIterCreate(set);
		printf("iter = %p\n", iter);
		ringIterNextObject(iter, 1);
		key = ringIterCurrentObject (iter, 1);
		printf("key = %p\n", key);
		ringIterRewind(iter, 1);
		print_it (iter, 1);
		ringIterDestroy(iter);
		nameobj = ringCreateName(ns, key, name, strlen(name));
		printf("new name = %p\n", nameobj);
		if (nameobj) {
				ringSetFreeze(ns);
				iter = ringIterCreate(ns);
				print_it (iter, 1);
				ringIterDestroy(iter);
				set = ns;
		}
		/*	Now test a duplicate */
		ns = ringSetCreate(pool);
		ringSetAddSet(ns, set);
		iter = ringIterCreate(set);
		ringIterNextObject(iter, 1);
		key = ringIterCurrentObject (iter, 1);
		printf ("key = %p\n", key);
		ringIterDestroy(iter);
		nameobj = ringCreateName(ns, key, name, strlen(name));
		printf ("new dup name = %p\n", nameobj);
		if (nameobj) {
				ringSetFreeze(ns);
				iter = ringIterCreate(ns);
				print_it (iter, 1);
				ringIterDestroy(iter);
				set = ns;
		}
		}
#endif
#ifdef KEYADDTEST
		{
		struct PgpPubKey *pubkey;
		struct PgpKeySpec *ks;
		struct RingSet *ns;
		union RingObject *key, *newkey;

		ns = ringSetCreate(pool);
		ringSetAddSet(ns, set);
		iter = ringIterCreate(set);
		ringIterNextObject(iter, 1);
		key = ringIterCurrentObject(iter, 1);
		printf ("key = %p\n", key);
		pubkey = ringKeyPubKey(set, key);
		printf ("pubkey = %p\n", pubkey);
		ks = pgpKeySpecCreate(pgpenv);
		pgpKeySpecSetCreation(ks, ringKeyCreation(set, key));
		newkey = ringCreateKey(ns, pubkey, ks, pubkey->pkAlg);
		printf ("newkey = %p\n", newkey);
		pgpPubKeyDestroy(pubkey);
		pgpKeySpecDestroy(ks);
		}
#endif

#ifdef KEYEDITTEST
{
		struct PgpSecKey *seckey;
		struct PgpKeySpec *ks;
		struct RingSet *ns;
		struct PgpRandomContext *rc = pgpRandomCreate();
		union RingObject *key, *newsec;

		ns = ringSetCreate(pool);
		ringSetAddSet(ns, set);
		iter = ringIterCreate(set);
		ringIterNextObject(iter, 1);
		key = ringIterCurrentObject(iter, 1);
		printf ("key = %p\n", key);
		ks = pgpKeySpecCreate(pgpenv);
		pgpKeySpecSetCreation(ks, ringKeyCreation(set, key));
		seckey = ringSecSecKey(set, key);
		printf ("seckey = %p\n", seckey);
		if (seckey) {
				int err = pgpSecKeyUnlock(seckey, "x", 1);
				printf ("Unlock returns %d\n", err);
				if (err>0) {
						err = pgpSecKeyChangeLock(seckey, pgpenv, rc, "y", 1);
						printf ("ChangeLock returns %d\n", err);
						pgpSecKeyLock(seckey);
						err = pgpSecKeyUnlock(seckey, "y", 1);
						printf ("Unlock now returns %d\n", err);
						pgpSecKeyLock(seckey);
						newsec = ringCreateSec(ns, seckey, ks, seckey->pkAlg);
						printf ("newsec = %p\n", newsec);
						ringSetFreeze(ns);
						iter = ringIterCreate(ns);
						print_it (iter, 1);
						ringIterDestroy(iter);
						set = ns;
				}
		}
		}
#endif

#ifdef KEYGENTEST
		{
		struct PgpSecKey *seckey;
		struct PgpKeySpec *ks;
		struct RingSet *ns;
		struct PgpRandomContext *rc = pgpRandomCreate();
		union RingObject *newsec, *nameobj;
		char *name = "Gumby";
		int err;

		ns = ringSetCreate(pool);
		ringSetAddSet(ns, set);
		ks = pgpKeySpecCreate(pgpenv);

		seckey = pgpSecKeyGenerate(PGP_PKALG_RSA, 1024, rc, progress,
												NULL, &err);
		printf ("\npgpSecKeyGenerate returns key=%p, err=%d\n", seckey, err);

		err = pgpSecKeyChangeLock(seckey, pgpenv, rc, "x", 1);
		printf ("ChangeLock returns %d\n", err);
		pgpSecKeyLock(seckey);
		err = pgpSecKeyUnlock(seckey, "x", 1);
		printf ("Unlock now returns %d\n", err);
		pgpSecKeyLock(seckey);

		newsec = ringCreateSec(ns, seckey, ks, PGP_PKALG_RSA);
		printf ("newsec = %p\n", newsec);

		nameobj = ringCreateName(ns, newsec, name, strlen(name));
		printf ("nameobj = %p\n", nameobj);

		ringSetFreeze(ns);
		iter = ringIterCreate(ns);
		print_it (iter, 1);
		ringIterDestroy(iter);
		set = ns;
		}
#endif
		if (argc == 3) {
				ff2 = fopen(argv[2], "wb");
				f2 = pgpFileWriteOpen(ff2, NULL);		/*	OK to chain */
				if (!ff2 || !f2) {
						perror(argv[2]);
						return 1;
				}
				puts("Output file successfully opened.");
				error = ringSetWrite(set, f2, (struct RingFile **)NULL, 4, 0);
				pgpFileClose(f2);
				if (error < 0)
						printf("*** ");
				printf("ringPoolWrite returned %d\n", error);
		}

		ringPoolDestroy(pool);
		randseedwrite(pgpenv);
		pgpenvDestroy(pgpenv);
		return 0;
}

#if __BORLANDC__
#if __BORLANDC <= 0x400
unsigned _stklen = 16384;
#else
unsigned const _stklen = 16384;
#endif
#endif
