/* des.h -- Headers and defines for des.c
 *
 * Based on work which is Copyright (c) 1988,1989,1990,1991 by R. Outerbridge.
 */

extern void deskey (unsigned long cooked[64], const unsigned char key[8]);
/*
 * Convert the ASCII key string pointed to by the second argument to
 * an encryption key and decryption key.  The encryption key is stored
 * in the first 32 elements of longs pointed to by the first argument,
 * and the decryption key is stored in the following 32 elements.
 */

extern void desfunc (unsigned long block[2], const unsigned long keys[32]);
/*
 * Encrypts/Decrypts using the key pointed to by the second argument
 * one block of eight bytes at the address pointed to by the first
 * argument.  Original data is overwritten.
 */
