/*
 * Copyright (c) 1990 Dennis Ferguson.  All rights reserved.
 *
 * Commercial use is permitted only if products which are derived from
 * or include this software are made available for purchase and/or use
 * in Canada.  Otherwise, redistribution and use in source and binary
 * forms are permitted.
 */

/*
 * des_ecb_buffer.c - encrypt a buffer of data by doing repeated ECB ciphers
 */
#include "des.h"
#include "des_tables.h"

/*
 * des_ecb_buffer - {en,de}crypt a stream in ECB mode
 */
int
des_ecb_buffer(in, out, length, schedule, encrypt)
	des_cblock *in;
	des_cblock *out;
	long length;
	des_key_schedule schedule;
	int encrypt;
{
	register unsigned long left, right;
	register unsigned long temp;
	register unsigned long *kp;
	register unsigned char *ip, *op;

	/*
	 * Get key, just once
	 */
	kp = (unsigned long *)schedule;

	/*
	 * Deal with encryption and decryption separately.
	 */
	if (encrypt) {
		/*
		 * Work down the length, 8 bytes at a time
		 */
		ip = (unsigned char *)in;
		op = (unsigned char *)out;
		while (length > 0) {
			/*
			 * Get more input, shift it in.  If the length is
			 * greater than or equal to 8 this is straight
			 * forward.  Otherwise we have to fart around.
			 */
			if (length >= 8) {
				GET_HALF_BLOCK(left, ip);
				GET_HALF_BLOCK(right, ip);
				length -= 8;
			} else {
				/*
				 * Oh, shoot.  We need to pad the
				 * end with zeroes.  Work backwards
				 * to do this.
				 */
				ip += (int) length;
				left = right = 0L;
				switch(length) {
				case 7:
					right |= (*(--ip) & 0xff) << 8;
				case 6:
					right |= (*(--ip) & 0xff) << 16;
				case 5:
					right |= (*(--ip) & 0xff) << 24;
				case 4:
					left |= *(--ip) & 0xff;
				case 3:
					left |= (*(--ip) & 0xff) << 8;
				case 2:
					left |= (*(--ip) & 0xff) << 16;
				case 1:
					left |= (*(--ip) & 0xff) << 24;
					break;
				}
				length = 0;
			}

			/*
			 * Encrypt what we have
			 */
			DES_DO_ENCRYPT(left, right, temp, kp);

			/*
			 * Copy the results out
			 */
			PUT_HALF_BLOCK(left, op);
			PUT_HALF_BLOCK(right, op);
		}
	} else {
		/*
		 * Decrypting is almost the same except that it is
		 * the output which could be short a few bytes.
		 */
		ip = (unsigned char *)in;
		op = (unsigned char *)out;
		for (;;) {		/* check done inside loop */
			/*
			 * Read a block from the input into left and
			 * right.  Save this cipher block for later.
			 */
			GET_HALF_BLOCK(left, ip);
			GET_HALF_BLOCK(right, ip);

			/*
			 * Decrypt this.
			 */
			DES_DO_DECRYPT(left, right, temp, kp);

			/*
			 * Output 8 or less bytes
			 */
			if (length > 8) {
				length -= 8;
				PUT_HALF_BLOCK(left, op);
				PUT_HALF_BLOCK(right, op);
			} else {
				/*
				 * Trouble here.  Start at end of output,
				 * work backwards.
				 */
				op += (int) length;
				switch(length) {
				case 8:
					*(--op) = right & 0xff;
				case 7:
					*(--op) = (right >> 8) & 0xff;
				case 6:
					*(--op) = (right >> 16) & 0xff;
				case 5:
					*(--op) = (right >> 24) & 0xff;
				case 4:
					*(--op) = left & 0xff;
				case 3:
					*(--op) = (left >> 8) & 0xff;
				case 2:
					*(--op) = (left >> 16) & 0xff;
				case 1:
					*(--op) = (left >> 24) & 0xff;
					break;
				}
				break;		/* we're done */
			}
		}
	}

	/*
	 * Done, return nothing.
	 */
	return 0;
}
