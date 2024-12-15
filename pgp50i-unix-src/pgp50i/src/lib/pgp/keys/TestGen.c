/*
 * TestGen.c
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: TestGen.c,v 1.1.2.2 1997/06/07 09:50:33 mhw Exp $
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
		FILE *ffsec, *ffpub;
		FILE *ffsec2, *ffpub2;
		struct PgpFile *fsec, *fsec2, *fpub, *fpub2;
		struct RingPool *pool;
		struct RingFile *filesec, *filepub;
		struct RingIterator *iter;
		struct PgpSecKey *seckey;
		struct PgpKeySpec *ks;
		struct RingSet const *set;
		struct RingSet *nsec, *npub;
		struct PgpRandomContext *rc = pgpRandomCreate();
		union RingObject *newsec, *nameobj;
		char *name;
		int err;
		unsigned count;
		int bits;
		byte pkalg;
		struct PgpEnv *pgpenv = pgpenvCreate();

		if (argc != 8) {
				fprintf (stderr,
						"Usage: %s secin secout pubin pubout type name bits\n",
								argv[0]);
				return 1;
		}

		if (strcmp(argv[5], "rsa")==0)
				pkalg = PGP_PKALG_RSA;
		else if (strcmp(argv[5], "elg")==0)
				pkalg = PGP_PKALG_ELGAMAL;
		else if (strcmp(argv[5], "dsa")==0)
				pkalg = PGP_PKALG_DSA;
		else {
				fprintf (stderr,
				"Unknown type, should be \"rsa\", \"elg\", or \"dsa\"\n");
				return 1;
		}

		name = argv[6];

		if ((bits = atoi(argv[7])) <= 0 || bits >= 10000) {
				fprintf (stderr, "bits should be >0 and < 10000\n");
				return 1;
		}

		pool = ringPoolCreate(NULL);
		if (!pool) {
				perror("Unable to allocate ringpool");
				return 1;
		}
		puts("Pool successfully opened");

		randseedread(pgpenv);

		ffsec = fopen(argv[1], "rb");
		fsec = pgpFileReadOpen(ffsec, NULL, NULL); /* OK to chain errs */
		if (!ffsec || !fsec) {
				perror(argv[1]);
				return 1;
		}
		puts("Secret file successfully opened.");

		filesec = ringFileOpen(pool, fsec, 1, &err);
		printf("file = %p err = %d\n", filesec, err);
		ringFileSetDestructor(filesec, destructor, (void *)0);
		printf("Trouble = %p\n", (void *)ringFileTrouble(filesec));
		err = ringTopListTrouble(stdout, filesec, 0);
		printf("err = %d\n", err);

		ffpub = fopen(argv[3], "rb");
		fpub = pgpFileReadOpen(ffpub, NULL, NULL); /* OK to chain errs */
		if (!ffpub || !fpub) {
		perror(argv[3]);
		return 1;
		}
puts("Public file successfully opened.");

		filepub = ringFileOpen(pool, fpub, 1, &err);
		printf("file = %p err = %d\n", filepub, err);
		ringFileSetDestructor(filepub, destructor, (void *)0);
		printf("Trouble = %p\n", (void *)ringFileTrouble(filepub));
		err = ringTopListTrouble(stdout, filepub, 0);
		printf("err = %d\n", err);

		set = ringFileSet(filesec);

		nsec = ringSetCreate(pool);
		ringSetAddSet(nsec, set);
		ks = pgpKeySpecCreate(pgpenv);
		pgpKeySpecSetValidity(ks, 0xffff);

		seckey = pgpSecKeyGenerate(pkalg, bits, rc, progress,
												NULL, &err);
		printf ("\npgpSecKeyGenerate returnsec key=%p, err=%d\n", seckey, err);

		err = pgpSecKeyChangeLock(seckey, pgpenv, rc, "x", 1);
		printf ("ChangeLock returnsec %d\n", err);
		pgpSecKeyLock(seckey);
		err = pgpSecKeyUnlock(seckey, "x", 1);
		printf ("Unlock now returnsec %d\n", err);
		pgpSecKeyLock(seckey);

		newsec = ringCreateSec(nsec, seckey, ks, pkalg);
		printf ("newsec = %p\n", newsec);

		nameobj = ringCreateName(nsec, newsec, name, strlen(name));
		printf ("nameobj = %p\n", nameobj);

		ringSetFreeze(nsec);
		iter = ringIterCreate(nsec);
		print_it (iter, 1);
		ringIterDestroy(iter);

		/* Now add key to public key file */
		set = ringFileSet(filepub);
		npub = ringSetCreate(pool);
		ringSetAddSet(npub, set);
		ringSetAddObject(npub, newsec);	/* add just key object */
		ringSetAddObject(npub, nameobj);
	
		ringSetFreeze(npub);
		iter = ringIterCreate(npub);
		print_it (iter, 1);
		ringIterDestroy(iter);
	
		ffsec2 = fopen(argv[2], "wb");
		fsec2 = pgpFileWriteOpen(ffsec2, NULL);	/* OK to chain */
		if (!ffsec2 || !fsec2) {
				perror(argv[2]);
				return 1;
		}
		puts("Output file successfully opened.");
		err = ringSetWrite(nsec, fsec2, (struct RingFile **)NULL, 4,
				WRITETRUST_SEC|WRITETRUST_PUB);
		pgpFileClose(fsec2);
		if (err < 0)
				printf("*** ");
		printf("ringPoolWrite returned %d on sec\n", err);

		ffpub2 = fopen(argv[4], "wb");
		fpub2 = pgpFileWriteOpen(ffpub2, NULL);	/* OK to chain */
		if (!ffpub2 || !fpub2) {
				perror(argv[4]);
				return 1;
		}
		puts("Output file successfully opened.");
		err = ringSetWrite(npub, fpub2, (struct RingFile **)NULL, 4,
				WRITETRUST_SEC|WRITETRUST_PUB);
		pgpFileClose(fpub2);
		if (err < 0)
				printf("*** ");
		printf("ringPoolWrite returned %d on pub\n", err);

done:
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
