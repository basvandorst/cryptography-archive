/*
 * mdes.h -- header for minimal des
 * 1993 Risto Paasivirta, paasivir@jyu.fi
 * Public Domain, no warranty. 
 */

#ifndef MDES_H
#define MDES_H

#define DES_ENCRYPT  1
#define DES_DECRYPT  0

typedef struct des_key_schedule {
  unsigned char b[16][48];
} des_key_schedule;

typedef struct des_cblock {
  unsigned char _[8];
} des_cblock;

void des_string_to_key(char *str, des_cblock *key);
void des_set_odd_parity(des_cblock *key);

int des_set_key(des_cblock *k, des_key_schedule *key);
int des_ecb_encrypt(des_cblock *input, des_cblock *output,
  des_key_schedule *schedule, int mode);
int des_cbc_encrypt(des_cblock *input, des_cblock *output,
  int length, des_key_schedule *schedule,
  des_cblock *ivec, int mode);

#endif /* MDES_H */

