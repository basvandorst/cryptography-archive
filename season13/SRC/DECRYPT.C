/*
 * This module contains the critical algorithm and secret key
 * used by British Sky Broadcasting in their Videocrypt pay-TV
 * chip card in 1993 and early 1994. It is expected that most
 * information in this file becomes obsolete when the next card
 * generation (09) is activated. The same algorithm is used
 * for the Adult Channel, but the key's are not included in this
 * file.
 */

#include "decrypt.h"

/*
 * the secret keys of the Sky 07 card -- for your eyes only :-)
 */

const unsigned char sky_keys[56] = {
  0x65, 0xe7, 0x71, 0x1a, 0xb4, 0x88, 0xd7, 0x76,
  0x28, 0xd0, 0x4c, 0x6e, 0x86, 0x8c, 0xc8, 0x43,
  0xa9, 0xec, 0x60, 0x42, 0x05, 0xf2, 0x3d, 0x1c,
  0x6c, 0xbc, 0xaf, 0xc3, 0x2b, 0xb5, 0xdc, 0x90,
  0xf9, 0x05, 0xea, 0x51, 0x46, 0x9d, 0xe2, 0x60,
  0x70, 0x52, 0x67, 0x26, 0x61, 0x49, 0x42, 0x09,
  0x50, 0x99, 0x90, 0xa2, 0x36, 0x0e, 0xfd, 0x39
};

/* #include "adult.key" */


/*
 * This is the core function of the secure hash algorithm
 * which is called 99 times by hash(). This code assumes that
 * unsigned char is exactly 8-bit long.
 */
void kernel(unsigned char *out, int *oi, const unsigned char in,
            const unsigned char *key)
{
  unsigned char c;

  out[*oi] ^= in;
  c = ~(key[out[*oi] >> 4] + key[(out[*oi] & 0x0f) + 16]);
  c = (c << 1) | (c >> 7);    /* rotate 1 bit left */
  c += in;
  c = (c >> 3) | (c << 5);    /* rotate 3 bit right */
  *oi = (*oi + 1) & 7;
  out[*oi] ^= c;

  return;
}


/*
 * This is a secure hash function that transforms a 32-byte message
 * in an 8-byte answer sequence and tests a 4-byte signature and a 1-byte
 * checksum in the 32-byte message. The transformation and the signature
 * check are done using a secret 32-byte key. This function is designed
 * in a way that makes it very difficult to guess the key bytes or to
 * write an identical function without knowing the key bytes by
 * looking only at the results of this function.
 */
int hash(const unsigned char *msg, unsigned char *answ,
         const unsigned char *key)
{
  int i;
  int oi = 0;           /* index in output array answ[] */
  int check = 0;        /* flag for incorrect signature/checksum  */
  unsigned char b = 0;

  for (i = 0; i < 8; i++) answ[i] = 0;

  /* initial hashing */
  for (i = 0; i < 27; i++)
    kernel(answ, &oi, msg[i], key);

  /* check the 4 signature bytes and continue hashing */
  for (i = 27; i < 31; i++) {
    kernel(answ, &oi, b, key);
    kernel(answ, &oi, b, key);
    b = msg[i];
    if (answ[oi] != msg[i]) check |= 1;  /* test signature */
    oi = (oi + 1) & 7;
  }

  /* final hashing with last byte */
  for (i = 0; i < 64; i++)
    kernel(answ, &oi, msg[31], key);

  /* test 8-bit checksum */
  b = 0;
  for (i = 0; i < 32; i++)
    b += msg[i];
  if (b != 0) check |= 2;

  answ[7] &= 0x0f;   /* only 60-bit are needed by decoder */

  return check;
}


/*
 * The decoder requests every ~2.5 seconds an answer to a 32-byte
 * packet (msg) from the chip card. The card's 8-byte answer (answ) to
 * this request is calculated by this function using hash() and the
 * right key bytes.
 */
int decode(const unsigned char *msg, unsigned char *answ)
{
  int i;
  int check;                  /* flag for incorrect signature/checksum  */
  const unsigned char *key;   /* pointer to the 32-byte key to be used */

  if ((msg[0] & 0xe0) == 0xc0) {
    for (i = 0; i < 8; i++) answ[i] = (i == 0);
    return 0;
  }

  /* Try Sky 07 key */
  if (msg[1] > 0x3a)      key = sky_keys + 24;
  else if (msg[1] > 0x32) key = sky_keys + 8;
  else key = sky_keys;
  check = hash(msg, answ, key);
  /* the following fix has been necessary since 1994-04-27 */
  if ((msg[6] & 0xf0) == 0xa0)
    for (i = 0; i < 8; i++) answ[i] = 0;
  if ((check & 1) == 0) return check | (1 << 2);  /* Sky key was ok */

#ifdef WITH_ADULT
  /* if Sky failed, try Adult Channel */
  if (msg[1] > 0x4a)      key = adult_keys + 48;
  else if (msg[1] > 0x43) key = adult_keys + 40;
  else if (msg[1] > 0x3a) key = adult_keys + 32;
  else if (msg[1] > 0x33) key = adult_keys + 8;
  else key = adult_keys;
  check = hash(msg, answ, key);
  if ((check & 1) == 0) return check | (2 << 2);  /* Adult key was ok */
#endif

  /* no known key resulted in a correct signature test */
  for (i = 0; i < 8; i++) answ[i] = 0;
  return check;
}
