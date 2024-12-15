/* Bitslice code for the SolNET DES Client             */
/* This code is based on the bitslice code released by */
/* Matthew Kwan (mkwan@cs.mu.oz.au)                    */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "desc.h"

void build_challenge(unsigned long *p,unsigned long *c,int complement);
void build_practice(unsigned long *p,unsigned long *c);

/* External functions */
extern unsigned long	deseval (const unsigned long *p,
			const unsigned long *c, const unsigned long *k);

/* External variables */
extern int verboselevel;

/*
 * Local variables.
 */

static int	bitlength = 32;
static int	bitlength_log2 = 5;
unsigned int    twiddles[22] = {1,4,5,10,12,15,18,31,32,35,36,42,46,47,48,49,50,51,52,53,54,55};
unsigned int    bkey = 0;
unsigned long   gkey = 0;

/*
 * Set the bit length variables.
 */

static void
set_bitlength (void) {
  unsigned long	x = ~0UL;
  
  bitlength = 0;
  for (x = ~0UL; x != 0; x >>= 1)
    bitlength++;

#ifdef DEBUG
  printf("%d-bit machine\n",bitlength);
#endif
  
  if (bitlength == 64)
    bitlength_log2 = 6;
  else {
    printf("Bitslice code is only available on 64-bit platforms!\n");
    exit(1);
  }
}

/*
 * Return an odd parity bit for a number.
 */

static int odd_parity (int n) {
  int	parity = 1;
  
  while (n != 0) {
    if ((n & 1) != 0)
      parity ^= 1;
    n >>= 1;
  }
  
  return (parity);
}

/*
 * The key has been found.
 * Turn the value into something readable, and print it out.
 */
static void key_found (
	unsigned long	slice,
	unsigned long	*keybits) {
  int		i;
  
  printf ("Key:");
  
  for (i=0; i<8; i++) {
    int		j, kc = 0;
    
    for (j=0; j<7; j++)
      if ((keybits[49 - i*7 + j] & slice) != 0)
	kc |= (1 << j);
    
    kc = (kc << 1) | odd_parity (kc);
    
    printf (" %02x", kc);
  }
  printf ("\n");
  
#ifdef DEBUG
  printf("bkey: %d   gkey:%d\n",bkey,gkey);
#endif
  
  exit(0);
}

/*
 * Set the bit slice pattern on the low key bits.
 */
static void set_low_keys (unsigned long	*key) {
  int i;
  
  if(bitlength == 64) {
    key[3] = 0xffffffff00000000;
    key[8] = 0xffff0000ffff0000;
    key[11] = 0xff00ff00ff00ff00;
    key[38] = 0xf0f0f0f0f0f0f0f0;
    key[39] = 0x3333333333333333;
    key[43] = 0x5555555555555555;
  }
  else 
    {
      fprintf(OUTPUT,"!64 bits - bummer..\n");
      exit(1);
    }
  
#ifdef DEBUG
  printf("key after set_low_keys: \n");
  for(i=0; i<56; i++)
    printf("%lx ",key[i]);
  printf("\n");
#endif
}

/*
 * Test the DES key evaluation routine for speed.
 */
int crypt_perf_slice(void) {
  int		i,j,x;
  double	td;
  unsigned long	key[56];
  unsigned long tmp;
  unsigned long res;
  int           complement=0;
  unsigned long p[64],c[64],k[64];
  unsigned long ticks;

  bkey=0;gkey=0;tmp=0;

  for (i=0; i<56; i++)
    key[i] = 0;
  
  set_bitlength();
  build_practice(p, c);
  set_low_keys (key);

  /* Do a dummy run to get the function loaded into memory */
  deseval(p, c, key);
  
  /* Begin the actual run */
  ticks=clock();
  while(1) {
    
    res = deseval(p, c, key);
    
    if (res != 0)   /* Wheee! */
      key_found(res, key);
    
    /* increment "binary-order" key */
    /* flip the server assigned bits in order to do the complemnt */
    if(++bkey & 0x8000) { /* We're at the end of the 2^28 block */
      if(!complement) {     /* Should we test the complement keys too? */
	/* Invert the whole key */
	for(x=0;x<56;x++)
	  key[x] = ~key[x];
	/* Zero out the bits that are to be varied */
	for(x=0; x<22; x++)
	  key[twiddles[x]] = 0;
	complement = 1;           /* We're testing the complement... */
	bkey=0; gkey=0; tmp = 0;  /* Restart */
	res = deseval(p, c, key); /* Test the key with all "active" bits a 0 */
       	if (res != 0)             /* Wheee! */
	  return 0;
	bkey++;                   /* Go to the next key */
      }
      else
	break;                   /* We're at the end and compl keys are done */
    }
    
    /* Increment the gray-order key and find out which bit changed */ 
    tmp = gkey;
    gkey = bkey ^ (bkey>>1);
    tmp ^= gkey;
    for (j = 0; j < 23; ++j)
      if ((tmp>>j)&1)
	break;
    
    /* Update the keys */
    key[twiddles[j]] = key[twiddles[j]] ^ ~0UL;
  }

  ticks-=clock();
  ticks= -ticks;

  return (float)(((float)0x400000*(float)CLOCKS_PER_SEC)/(float)ticks);
}

/* The key-search routine */
int des_search_slice(unsigned char *a_key) {
  int		i,j,x;
  unsigned long	key[56],p[64],c[64];
  unsigned long res;
  unsigned long tmp;
  int           complement=0;
  int           cnt=0;
  
#ifdef DEBUG
  printf("Entering slice...\n");
  printf("a_key: ");
  for(i=0;i<8;i++)
    printf("%02x ",a_key[i]);
  printf("\n");
#endif
  
  /* Setup the bit-length and build the challenge data */ 
  set_bitlength();
  
#ifdef TEST_NET
  build_practice(p, c);
#else
  build_challenge(p, c, complement); /* build normal challenge data */
#endif

  /* Strip the parity from the key and convert it to "slice-mode"*/
  for (i=0; i<56; i++) {
    if (a_key[i/7] & (128 >> (i%7)))
      key[55 - i] = ~0UL;
    else
      key[55 - i] = 0;
  } 

#ifdef DEBUG
  printf("\nKey after parity strip:\n");
  for(i=0;i<56;i++)
    printf("%d ; %lx \n",i,key[i]);  
#endif

  /* Zero out all the bits that are to be varied */
  for (i=0; i<22; i++)
    key[twiddles[i]] = 0;

  /* Set the pattern to give 6 unique keys on each call */
  set_low_keys(key);

  i=0; /* Reset for progress count */

  /* Start the main search loop, will serach 2^29 key-pairs */
  while(1) {
    
    res = deseval(p, c, key);
    
    if (res != 0)   /* Wheee! */
      return 1;
    
    /* increment "binary-order" key */
    /* flip the server assigned bits in order to do the complement */
    if(++bkey & 0x400000) { /* We're at the end of the 2^28 block */
      if(!complement) {     /* Should we test the complement keys too? */
	/* Invert the whole key */
	for(x=0;x<56;x++)
	  key[x] = ~key[x];
	/* Zero out the bits that are to be varied */
	for (x=0; x<22; x++)
	  key[twiddles[x]] = 0;
	complement = 1;           /* We're testing the complement... */
	build_challenge(p, c, complement); /* build complement challenge data */
	bkey=0; gkey=0; tmp = 0;  /* Restart */
	res = deseval(p, c, key); /* Test the key with all "active" bits a 0 */
       	if (res != 0)             /* Wheee! */
	  key_found(res, key);
	bkey++;                   /* Go to the next key */
      }
      else
	break;                   /* We're at the end and compl keys are done */
    }
    
    /* Increment the gray-order key and find out which bit changed */ 
    tmp = gkey;
    gkey = bkey ^ (bkey>>1);
    tmp ^= gkey;
    for (j = 0; j < 23; ++j)
      if ((tmp>>j)&1)
	break;

    if(tmp & 0x40000) {
      if(verboselevel > 2) {
	printf("%4.2f%%..",++i*6.25);
	fflush(stdout);
      }
    }

    /* Update the keys */
    key[twiddles[j]] = key[twiddles[j]] ^ ~0UL;
  }
  printf("\n");
  return 0; /* Bummer, no key found */
}

/*
 * Unroll the bits contained in the plaintext, ciphertext, and key values.
 */

static void unroll_bits (
	unsigned long		*p,
	unsigned long		*c,
	const unsigned char	*ivc,
	const unsigned char	*pc,
	const unsigned char	*cc
	) {
  int			i;
  unsigned char		ptext[8];
  
  /* Apply the IV */
  for (i=0; i<8; i++)
    ptext[i] = ivc[i] ^ pc[i];
  
  /*  */
  for (i=0; i<64; i++)
    if ((ptext[i/8] & (128 >> (i % 8))) != 0)
      p[63 - i] = ~0UL;
    else
      p[63 - i] = 0;
  
  /*  */
  for (i=0; i<64; i++)
    if ((cc[i/8] & (128 >> (i % 8))) != 0)
      c[63 - i] = ~0UL;
    else
      c[63 - i] = 0;
}

/*
 * Set up the sample plaintext, ciphertext and key values.
 */

void build_practice (unsigned long *p,unsigned long *c) {
  
  /* Test 1 */ 
  unsigned char	iv_p[8]
    = {0xa2, 0x18, 0x5a, 0xbf, 0x45, 0x96, 0x60, 0xbf};
  unsigned char	pt_p[8]
    = {0x54, 0x68, 0x65, 0x20, 0x75, 0x6e, 0x6b, 0x6e};
  unsigned char	ct_p[8]
    = {0x3e, 0xa7, 0x86, 0xf9, 0x1d, 0x76, 0xbb, 0xd3};
  unsigned char	key_p[8]      
    = {0x5e, 0xd9, 0x20, 0x4f, 0xec, 0xe0, 0xb9, 0x67};

  /* Test 2 */ 
  /*
    unsigned char	iv_p[8]
      = {0xca, 0x15, 0x5c, 0x71, 0x90, 0xce, 0x61, 0x49};
    unsigned char	pt_p[8]
      = {0x31, 0x65, 0xa6, 0x1c, 0x4a, 0xd2, 0x75, 0x45};
    unsigned char	ct_p[8]
      = {0x9a, 0xdd, 0x94, 0x27, 0xbc, 0x8d, 0x7d, 0x1d};
    unsigned char	key_p[8]      
      = {0x10, 0xb9, 0x13, 0xf1, 0x4f, 0xc4, 0x01, 0x9d};
      */ 
    unroll_bits (p, c, iv_p, pt_p, ct_p);
}


void build_challenge(unsigned long *p,unsigned long *c,int complement) {
  
  /* Normal Challenge Data */
  unsigned char	iv_s[8]
    = {0x99, 0xe9, 0x7c, 0xbf, 0x4f, 0x7a, 0x6e, 0x8f};
  unsigned char	pt_s[8]
    = {0x54, 0x68, 0x65, 0x20, 0x75, 0x6e, 0x6b, 0x6e};
  unsigned char	ct_s[8]
    = {0x79, 0x45, 0x81, 0xc0, 0xa0, 0x6e, 0x40, 0xa2};

  /* If complement != 0 use the complement for the c and p-texts */
  if(complement) { /* Use the complement of the data */
  
    pt_s[0]=0xab;
    pt_s[1]=0x97;
    pt_s[2]=0x9a;
    pt_s[3]=0xdf;
    pt_s[4]=0x8a;
    pt_s[5]=0x91;
    pt_s[6]=0x94;
    pt_s[7]=0x91;

    ct_s[0]=0x86;
    ct_s[1]=0xba;
    ct_s[2]=0x7e;
    ct_s[3]=0x3f;
    ct_s[4]=0x5f;
    ct_s[5]=0x91;
    ct_s[6]=0xbf;
    ct_s[7]=0x5d;      
  }
    unroll_bits (p, c, iv_s, pt_s, ct_s);
}


/* The key-search routine */
int des_test_slice() {
  int		i;
  unsigned long	key[56],p[64],c[64];
  unsigned long res;
  int           test1=0; 
  int           test2=0;
  unsigned char a_key[8]={0x5e, 0xd9, 0x20, 0x4f, 0xec, 0xe0, 0xb9, 0x67};

  /* Setup the bit-length and build the challenge data */ 
  set_bitlength();
  build_practice(p, c);

  /* Strip the parity from the key and convert it to "slice-mode"*/
  for (i=0; i<56; i++) {
    if (a_key[i/7] & (128 >> (i%7)))
      key[55 - i] = ~0UL;
    else
      key[55 - i] = 0;
  } 

  /* Set the pattern to give 6 unique keys on each call */
  set_low_keys(key);
  /* See that the correct key is found */
  res = deseval(p, c, key);
  if (res != 0)   /* Wheee! */
      test1=1;

  /* Se that an icorrect key is handled corretly */
  key[0] = ~key[0];
  res = deseval(p, c, key);
  if (res == 0)
    test2=1;
  
  if(test1 && test2)
    return 1;
  else 
    return -1;
}







