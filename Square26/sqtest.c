#include <stdio.h>
#include <string.h>
#include <time.h>

#include "square.h"
#include "sqecb.h"
#include "sqcbc.h"
#include "sqcts.h"
#include "sqcfb.h"
#include "sqofb.h"
#include "sqhash.h"

#define TIMING_ITERATIONS 100000L


static const byte key[] =
	"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";
static const byte text[] =
	"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
	"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";
static byte data[1024];


static void squarePrintBlock (const byte *block, unsigned length, const char *tag)
{
	unsigned count = 0;

	while (length > 4) {
		printf ("%02x%02x%02x%02x", block[0], block[1], block[2], block[3]);
		block += 4;
		length -= 4;
		count += 4;
		if (count == 16) {
			printf (" ");
		} else if (count == 32) {
			count = 0;
			printf ("\n");
		}
	}
	switch (length) {
	case 0:
		printf ("%s\n", tag);
		break;
	case 1:
		printf ("%02x %s\n", block[0], tag);
		break;
	case 2:
		printf ("%02x%02x %s\n", block[0], block[1], tag);
		break;
	case 3:
		printf ("%02x%02x%02x %s\n", block[0], block[1], block[2], tag);
		break;
	case 4:
		printf ("%02x%02x%02x%02x %s\n", block[0], block[1], block[2], block[3], tag);
		break;
	}
} /* squarePrintBlock */


static void squareTestRaw (void)
{
	const byte cipher[] =
		"\x7c\x34\x91\xd9\x49\x94\xe7\x0f\x0e\xc2\xe7\xa5\xcc\xb5\xa1\x4f";
	squareKeySchedule roundKeys_e, roundKeys_d;

	memcpy (data, text, SQUARE_BLOCKSIZE);
	squarePrintBlock (data, SQUARE_BLOCKSIZE, "plaintext");

	squareGenerateRoundKeys (key, roundKeys_e, roundKeys_d);
	squareEncrypt ((word32 *)data, roundKeys_e);
	if (memcmp (data, cipher, SQUARE_BLOCKSIZE) == 0) {
		squarePrintBlock (data, SQUARE_BLOCKSIZE, "encrypted(OK)");
	} else {
		squarePrintBlock (data, SQUARE_BLOCKSIZE, "encrypted(ERROR)\a");
		squarePrintBlock (cipher, SQUARE_BLOCKSIZE, "expected");
	}

	squareDecrypt ((word32 *)data, roundKeys_d);
	if (memcmp (data, text, SQUARE_BLOCKSIZE) == 0) {
		squarePrintBlock (data, SQUARE_BLOCKSIZE, "decrypted(OK)");
	} else {
		squarePrintBlock (data, SQUARE_BLOCKSIZE, "decrypted(ERROR)\a");
	}

	printf ("\n");
} /* squareTestRaw */


static void squareTestEcb (void)
{
	const byte cipher[] =
		"\x7c\x34\x91\xd9\x49\x94\xe7\x0f\x0e\xc2\xe7\xa5\xcc\xb5\xa1\x4f"
		"\x7c\x34\x91\xd9\x49\x94\xe7\x0f\x0e\xc2\xe7\xa5\xcc\xb5\xa1\x4f";
	squareEcbContext ctxEcb;

	printf ("Testing ECB...\n");
	memcpy (data, text, 2*SQUARE_BLOCKSIZE);
	squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "plaintext");

	squareEcbInit (&ctxEcb, key);
	squareEcbEncrypt (&ctxEcb, data, 2*SQUARE_BLOCKSIZE);
	if (memcmp (data, cipher, 2*SQUARE_BLOCKSIZE) == 0) {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "encrypted(OK)");
	} else {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "encrypted(ERROR)\a");
		squarePrintBlock (cipher, 2*SQUARE_BLOCKSIZE, "expected");
	}
	squareEcbFinal (&ctxEcb);

	squareEcbInit (&ctxEcb, key);
	squareEcbDecrypt (&ctxEcb, data, 2*SQUARE_BLOCKSIZE);
	if (memcmp (data, text, 2*SQUARE_BLOCKSIZE) == 0) {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "decrypted(OK)");
	} else {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "decrypted(ERROR)\a");
	}
	squareEcbFinal (&ctxEcb);

	printf ("\n");
} /* squareTestEcb */


static void squareTestCbc (const squareBlock iv)
{
	const byte cipher[] =
		"\x7c\x34\x91\xd9\x49\x94\xe7\x0f\x0e\xc2\xe7\xa5\xcc\xb5\xa1\x4f"
		"\x41\xd2\xf1\x9d\x7e\x87\x8d\xb5\x6c\x74\x46\xd4\x24\xc3\xad\xfc";
	squareCbcContext ctxCbc;

	printf ("Testing CBC...\n");
	memcpy (data, text, 2*SQUARE_BLOCKSIZE);
	squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "plaintext");

	squareCbcInit (&ctxCbc, key);
	squareCbcSetIV (&ctxCbc, iv);
	squareCbcEncrypt (&ctxCbc, data, 2*SQUARE_BLOCKSIZE);
	if (memcmp (data, cipher, 2*SQUARE_BLOCKSIZE) == 0) {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "encrypted(OK)");
	} else {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "encrypted(ERROR)\a");
		squarePrintBlock (cipher, 2*SQUARE_BLOCKSIZE, "expected");
	}
	squareCbcFinal (&ctxCbc);

	squareCbcInit (&ctxCbc, key);
	squareCbcSetIV (&ctxCbc, iv);
	squareCbcDecrypt (&ctxCbc, data, 2*SQUARE_BLOCKSIZE);
	if (memcmp (data, text, 2*SQUARE_BLOCKSIZE) == 0) {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "decrypted(OK)");
	} else {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "decrypted(ERROR)\a");
	}
	squareCbcFinal (&ctxCbc);

	printf ("\n");
} /* squareTestCbc */


static void squareTestCts (const squareBlock iv)
{
	const byte cipher[20 + 1] =
		"\x02\xde\x82\x56\x73\xf5\xca\xf5\xa9\x5c\xd6\x3c\xbf\x33\x9c\x85"
		"\xd5\xca\x51\x8d";
	squareCtsContext ctxCts;

	printf ("Testing CTS...\n");
	memcpy (data, text, 20);
	squarePrintBlock (data, 20, "plaintext");

	squareCtsInit (&ctxCts, key);
	squareCtsSetIV (&ctxCts, iv);
	squareCtsEncrypt (&ctxCts, data, 20);
	if (memcmp (data, cipher, 20) == 0) {
		squarePrintBlock (data, 20, "encrypted(OK)");
	} else {
		squarePrintBlock (data, 20, "encrypted(ERROR)\a");
		squarePrintBlock (cipher, 20, "expected");
	}
	squareCtsFinal (&ctxCts);

	squareCtsInit (&ctxCts, key);
	squareCtsSetIV (&ctxCts, iv);
	squareCtsDecrypt (&ctxCts, data, 20);
	if (memcmp (data, text, 20) == 0) {
		squarePrintBlock (data, 20, "decrypted(OK)");
	} else {
		squarePrintBlock (data, 20, "decrypted(ERROR)\a");
	}
	squareCtsFinal (&ctxCts);

	printf ("\n");
} /* squareTestCts */


static void squareTestCfb (const squareBlock iv)
{
	const byte cipher[] =
		"\xff\x58\x6d\xa5\x6c\xba\xc5\x06\x4a\x09\xa4\x0a\xee\xb6\xae\xaf"
		"\xd5\xcb\x53\x8e\xea\x28\x97\x4f\x7c\x75\xe7\x9b\xcb\x0d\x4d\x0e";
	squareCfbContext ctxCfb;

	printf ("Testing CFB...\n");
	memcpy (data, text, 2*SQUARE_BLOCKSIZE);
	squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "plaintext");

	squareCfbInit (&ctxCfb, key);
	squareCfbSetIV (&ctxCfb, iv);
	squareCfbEncrypt (&ctxCfb, data, 2*SQUARE_BLOCKSIZE);
	if (memcmp (data, cipher, 2*SQUARE_BLOCKSIZE) == 0) {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "encrypted(OK)");
	} else {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "encrypted(ERROR)\a");
		squarePrintBlock (cipher, 2*SQUARE_BLOCKSIZE, "expected");
	}
	squareCfbFinal (&ctxCfb);

	squareCfbInit (&ctxCfb, key);
	squareCfbSetIV (&ctxCfb, iv);
	squareCfbDecrypt (&ctxCfb, data, 2*SQUARE_BLOCKSIZE);
	if (memcmp (data, text, 2*SQUARE_BLOCKSIZE) == 0) {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "decrypted(OK)");
	} else {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "decrypted(ERROR)\a");
	}
	squareCfbFinal (&ctxCfb);

	printf ("\n");
} /* squareTestCfb */


static void squareTestOfb (const squareBlock iv)
{
	const byte cipher[] =
		"\xff\x58\x6d\xa5\x6c\xba\xc5\x06\x4a\x09\xa4\x0a\xee\xb6\xae\xaf"
		"\x35\xc8\x33\xd3\x5c\x29\x44\x37\x35\xd2\x25\xbc\x95\x28\xc3\xc8";
	squareOfbContext ctxOfb;

	printf ("Testing OFB...\n");
	memcpy (data, text, 2*SQUARE_BLOCKSIZE);
	squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "plaintext");

	squareOfbInit (&ctxOfb, key);
	squareOfbSetIV (&ctxOfb, iv);
	squareOfbEncrypt (&ctxOfb, data, 2*SQUARE_BLOCKSIZE);
	if (memcmp (data, cipher, 2*SQUARE_BLOCKSIZE) == 0) {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "encrypted(OK)");
	} else {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "encrypted(ERROR)\a");
		squarePrintBlock (cipher, 2*SQUARE_BLOCKSIZE, "expected");
	}
	squareOfbFinal (&ctxOfb);

	squareOfbInit (&ctxOfb, key);
	squareOfbSetIV (&ctxOfb, iv);
	squareOfbDecrypt (&ctxOfb, data, 2*SQUARE_BLOCKSIZE);
	if (memcmp (data, text, 2*SQUARE_BLOCKSIZE) == 0) {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "decrypted(OK)");
	} else {
		squarePrintBlock (data, 2*SQUARE_BLOCKSIZE, "decrypted(ERROR)\a");
	}
	squareOfbFinal (&ctxOfb);

	printf ("\n");
} /* squareTestOfb */


static void squareTestHashing (void)
{
	const byte check[] =
		"\xe6\x40\x13\xc6\x2d\xa2\x09\x6f\x67\x49\xb4\x8c\x5d\x7b\x18\xeb";
	squareHashContext ctxHash;
	squareBlock digest;

	printf ("Testing hashing...\n");
	squareHashInit   (&ctxHash);
	squareHashUpdate (&ctxHash, text, SQUARE_BLOCKSIZE);
	squareHashFinal  (&ctxHash, digest);
	if (memcmp (digest, check, SQUARE_BLOCKSIZE) == 0) {
		squarePrintBlock (digest, SQUARE_BLOCKSIZE, "digest #1 (OK)");
	} else {
		squarePrintBlock (digest, SQUARE_BLOCKSIZE, "digest #1 (ERROR)\a");
		squarePrintBlock (check, SQUARE_BLOCKSIZE, "expected");
	}
	squareHash (text, SQUARE_BLOCKSIZE, digest);
	if (memcmp (digest, check, SQUARE_BLOCKSIZE) == 0) {
		squarePrintBlock (digest, SQUARE_BLOCKSIZE, "digest #2 (OK)");
	} else {
		squarePrintBlock (digest, SQUARE_BLOCKSIZE, "digest #2 (ERROR)\a");
		squarePrintBlock (check, SQUARE_BLOCKSIZE, "expected");
	}
	printf ("\n");
} /* squareTestHashing */


static void squareMeasureRawSpeed (byte *text)
{
	squareKeySchedule roundKeys_e, roundKeys_d;
	long n; clock_t elapsed; double sec;

	squareGenerateRoundKeys (key, roundKeys_e, roundKeys_d);

	printf ("Measuring raw encryption speed...");
	elapsed = -clock ();
	for (n = 64*TIMING_ITERATIONS; n > 0; n--) {
		squareEncrypt((word32 *)text, roundKeys_e);
	}
	elapsed += clock ();
	sec = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf (" %.2f sec, %.1f K/sec.\n",
		sec, 16*64*TIMING_ITERATIONS/1024/sec);

	printf ("Measuring raw decryption speed...");
	elapsed = -clock ();
	for (n = 64*TIMING_ITERATIONS; n > 0; n--) {
		squareDecrypt((word32 *)text, roundKeys_d);   
	}
	elapsed += clock ();
	sec = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf (" %.2f sec, %.1f K/sec.\n",
		sec, 16*64*TIMING_ITERATIONS/1024/sec);
} /* squareMeasureRawSpeed */


static void squareMeasureEcbSpeed (byte *text, unsigned length)
{
	squareEcbContext ctxEcb;
	long n; clock_t elapsed; double sec;

	squareEcbInit (&ctxEcb, key);

	printf ("Measuring ECB encryption speed...");
	elapsed = -clock ();
	for (n = TIMING_ITERATIONS; n > 0; n--) {
		squareEcbEncrypt (&ctxEcb, text, length);
	}
	elapsed += clock ();
	sec = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf (" %.2f sec, %.1f K/sec.\n",
		sec, (float)length*TIMING_ITERATIONS/1024.0/sec);

	printf ("Measuring ECB decryption speed...");
	elapsed = -clock ();
	for (n = TIMING_ITERATIONS; n > 0; n--) {
		squareEcbDecrypt (&ctxEcb, text, length);
	}
	elapsed += clock ();
	sec = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf (" %.2f sec, %.1f K/sec.\n",
		sec, (float)length*TIMING_ITERATIONS/1024.0/sec);

	squareEcbFinal (&ctxEcb);
} /* squareMeasureEcbSpeed */


static void squareMeasureCbcSpeed (byte *text, unsigned length)
{
	squareCbcContext ctxCbc;
	long n; clock_t elapsed; double sec;

	squareCbcInit (&ctxCbc, key);

	printf ("Measuring CBC encryption speed...");
	elapsed = -clock ();
	for (n = TIMING_ITERATIONS; n > 0; n--) {
		squareCbcEncrypt (&ctxCbc, text, length);
	}
	elapsed += clock ();
	sec = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf (" %.2f sec, %.1f K/sec.\n",
		sec, (float)length*TIMING_ITERATIONS/1024.0/sec);

	printf ("Measuring CBC decryption speed...");
	elapsed = -clock ();
	for (n = TIMING_ITERATIONS; n > 0; n--) {
		squareCbcDecrypt (&ctxCbc, text, length);
	}
	elapsed += clock ();
	sec = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf (" %.2f sec, %.1f K/sec.\n",
		sec, (float)length*TIMING_ITERATIONS/1024.0/sec);

	squareCbcFinal (&ctxCbc);
} /* squareMeasureCbcSpeed */


static void squareMeasureCtsSpeed (byte *text, unsigned length)
{
	squareCtsContext ctxCts;
	long n; clock_t elapsed; double sec;

	squareCtsInit (&ctxCts, key);

	printf ("Measuring CTS encryption speed...");
	elapsed = -clock ();
	for (n = TIMING_ITERATIONS; n > 0; n--) {
		squareCtsEncrypt (&ctxCts, text, length);
	}
	elapsed += clock ();
	sec = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf (" %.2f sec, %.1f K/sec.\n",
		sec, (float)length*TIMING_ITERATIONS/1024.0/sec);

	printf ("Measuring CTS decryption speed...");
	elapsed = -clock ();
	for (n = TIMING_ITERATIONS; n > 0; n--) {
		squareCtsDecrypt (&ctxCts, text, length);
	}
	elapsed += clock ();
	sec = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf (" %.2f sec, %.1f K/sec.\n",
		sec, (float)length*TIMING_ITERATIONS/1024.0/sec);

	squareCtsFinal (&ctxCts);
} /* squareMeasureCtsSpeed */


static void squareMeasureCfbSpeed (byte *text, unsigned length)
{
	squareCfbContext ctxCfb;
	long n; clock_t elapsed; double sec;

	squareCfbInit (&ctxCfb, key);

	printf ("Measuring CFB encryption speed...");
	elapsed = -clock ();
	for (n = TIMING_ITERATIONS; n > 0; n--) {
		squareCfbEncrypt (&ctxCfb, text, length);
	}
	elapsed += clock ();
	sec = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf (" %.2f sec, %.1f K/sec.\n",
		sec, (float)length*TIMING_ITERATIONS/1024.0/sec);

	printf ("Measuring CFB decryption speed...");
	elapsed = -clock ();
	for (n = TIMING_ITERATIONS; n > 0; n--) {
		squareCfbDecrypt (&ctxCfb, text, length);
	}
	elapsed += clock ();
	sec = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf (" %.2f sec, %.1f K/sec.\n",
		sec, (float)length*TIMING_ITERATIONS/1024.0/sec);

	squareCfbFinal (&ctxCfb);
} /* squareMeasureCfbSpeed */


static void squareMeasureOfbSpeed (byte *text, unsigned length)
{
	squareOfbContext ctxOfb;
	long n; clock_t elapsed; double sec;

	squareOfbInit (&ctxOfb, key);

	printf ("Measuring OFB encryption speed...");
	elapsed = -clock ();
	for (n = TIMING_ITERATIONS; n > 0; n--) {
		squareOfbEncrypt (&ctxOfb, text, length);
	}
	elapsed += clock ();
	sec = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf (" %.2f sec, %.1f K/sec.\n",
		sec, (float)length*TIMING_ITERATIONS/1024.0/sec);

	printf ("Measuring OFB decryption speed...");
	elapsed = -clock ();
	for (n = TIMING_ITERATIONS; n > 0; n--) {
		squareOfbDecrypt (&ctxOfb, text, length);
	}
	elapsed += clock ();
	sec = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf (" %.2f sec, %.1f K/sec.\n",
		sec, (float)length*TIMING_ITERATIONS/1024.0/sec);

	squareOfbFinal (&ctxOfb);
} /* squareMeasureOfbSpeed */


static void squareMeasureHashingSpeed (byte *text, unsigned length)
{
	squareHashContext ctxHash;
	squareBlock digest;
	long n; clock_t elapsed; double sec;

	printf ("Measuring hashing speed...");
	elapsed = -clock ();
	squareHashInit (&ctxHash);
	for (n = TIMING_ITERATIONS; n > 0; n--) {
		squareHashUpdate (&ctxHash, text, length);
	}
	squareHashFinal (&ctxHash, digest);
	elapsed += clock ();
	sec = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf (" %.2f sec, %.1f K/sec.\n",
		sec, (float)length*TIMING_ITERATIONS/1024.0/sec);
} /* squareMeasureHashingSpeed */


int main (void)
{
	printf ("%s\n", squareBanner);
	printf ("Checking correctness...\n");

	squarePrintBlock (key, SQUARE_BLOCKSIZE, "user key");

	/* check raw encryption/decryption: */
	squareTestRaw ();

	/* check ECB mode: */
	squareTestEcb ();

	/* check CBC mode: */
	squareTestCbc (NULL);

	/* check CTS mode: */
	squareTestCts (NULL);

	/* check CFB mode: */
	squareTestCfb (NULL);

	/* check OFB mode: */
	squareTestOfb (NULL);

	/* check hashing: */
	squareTestHashing ();

	/* measure raw speed: */
	squareMeasureRawSpeed (data);

	/* measure ECB speed: */
	squareMeasureEcbSpeed (data, 1024);

	/* measure CBC speed: */
	squareMeasureCbcSpeed (data, 1024);

	/* measure CTS speed: */
	squareMeasureCtsSpeed (data, 1024);

	/* measure CFB speed: */
	squareMeasureCfbSpeed (data, 1024);

	/* measure OFB speed: */
	squareMeasureOfbSpeed (data, 1024);

	/* measure hashing speed: */
	squareMeasureHashingSpeed (data, 1024);

	return 0;
} /* main */
