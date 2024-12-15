#include "opt2-sj.h"

/*
 * testing framework
 */

#include <stdio.h>


static void test_vector ()
{
  uint8 in [8]   = { 0x33,0x22,0x11,0x00,0xdd,0xcc,0xbb,0xaa };
  uint8 key [10] = { 0x00,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11 };
  uint8 * subkeys [10] ;
  uint8 compare [8] = { 0x25,0x87,0xca,0xe2,0x7a,0x12,0xd3,0x00 } ;
  uint8 cip [8] ;
  uint8 out [8] ;
  int i ;

  printf ("key  %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7], key[8], key[9]) ;
  printf ("pln    %02x %02x %02x %02x %02x %02x %02x %02x\n", in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7]) ;
  subkey_table_gen (&key[0], &subkeys[0]) ;
  skipjack_forwards (&in[0], &cip[0], &subkeys[0]) ;
  printf ("cip    %02x %02x %02x %02x %02x %02x %02x %02x\n", cip[0], cip[1], cip[2], cip[3], cip[4], cip[5], cip[6], cip[7]) ;
  for (i = 0 ; i < 8 ; i++)
    if (cip[i] != compare[i])
      {
	printf ("failed forwards test\n") ;
	exit (4) ;
      }
  skipjack_backwards (&cip[0], &out[0], &subkeys[0]) ;
  printf ("chk    %02x %02x %02x %02x %02x %02x %02x %02x\n", out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7]) ;
  for (i = 0 ; i < 8 ; i++)
    if (out[i] != in[i])
      {
	printf ("failed backwards test\n") ;
	exit (4) ;
      }
}

static void test_speed (long count)
{
  uint8 in [8]   = { 0x33,0x22,0x11,0x00,0xdd,0xcc,0xbb,0xaa };
  uint8 key [10] = { 0x00,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11 };
  uint8 * subkeys [10] ;
  uint8 cipher [8] ;
  long i ;
  subkey_table_gen (&key[0], &subkeys[0]) ;
  printf ("encrypting %d blocks (%d bytes):\n", count, count<<3) ;
  for (i = 0 ; i < count ; i++)
    skipjack_forwards (&in[0], &cipher[0], &subkeys[0]) ;
}


extern int main (int argc, char ** argv, char ** envp)
{
  test_vector () ;
  test_speed (1000000) ;
  return (0) ;
}
