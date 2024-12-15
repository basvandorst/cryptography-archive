#include <stdio.h>
#include <string.h>

#include "square.h"

static void squarePrint (const byte *block, const char *tag)
{
	printf ("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x %s\n",
		block[ 0], block[ 1], block[ 2], block[ 3],
		block[ 4], block[ 5], block[ 6], block[ 7],
		block[ 8], block[ 9], block[10], block[11],
		block[12], block[13], block[14], block[15],
		tag);
} /* squarePrint */


int main (void)
{
	int i;
	squareBlock key, block;
	squareKeySchedule roundKeys_e;

	printf ("===========================================================================\n\n"
			"Validation data set for Square v1.0\n\n");

	printf ("===========================================================================\n\n"
			"Encryption of the null data block with every key where a single bit is set:\n\n");
	memset (block, 0, SQUARE_BLOCKSIZE);
	squarePrint (block, "plaintext\n");
	for (i = 0; i < 128; i++) {
		memset (key, 0, SQUARE_BLOCKSIZE);
		key[i/8] |= 1 << (7 - i%8); /* set only the i-th bit of the i-th test key */
		squarePrint (key, "key");
		memset (block, 0, SQUARE_BLOCKSIZE);
		squareExpandKey (key, roundKeys_e);
		squareEncrypt ((word32 *)block, roundKeys_e);
		squarePrint (block, "ciphertext\n\n");
	}

	printf ("===========================================================================\n\n"
			"Encryption of a sample data block with every key where a single bit is set:\n\n");
	memcpy (block, "\x0f\x1e\x2d\x3c\x4b\x5a\x69\x78\x87\x96\xa5\xb4\xc3\xd2\xe1\xf0", SQUARE_BLOCKSIZE);
	squarePrint (block, "plaintext\n");
	for (i = 0; i < 128; i++) {
		memset (key, 0, SQUARE_BLOCKSIZE);
		key[i/8] |= 1 << (7 - i%8); /* set only the i-th bit of the i-th test key */
		squarePrint (key, "key");
		memcpy (block, "\x0f\x1e\x2d\x3c\x4b\x5a\x69\x78\x87\x96\xa5\xb4\xc3\xd2\xe1\xf0", SQUARE_BLOCKSIZE);
		squareExpandKey (key, roundKeys_e);
		squareEncrypt ((word32 *)block, roundKeys_e);
		squarePrint (block, "ciphertext\n\n");
	}

	printf ("===========================================================================\n\n");

	return 0;
}
