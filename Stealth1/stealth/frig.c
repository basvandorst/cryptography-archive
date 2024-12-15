
/*
 * Frig V1.0 by Henry Hastur
 *
 * May well be covered by US ITAR encryption export regulations, if
 * in doubt, don't export it. It would be pretty stupid if it was, but
 * hey, governments have done some pretty stupid things before now....
 *
 * This code is copyright Henry Hastur 1994, but may be freely distributed,
 * modified, incorporated into other programs and used, as long as the
 * copyright stays attached and you obey any relevant import or export
 * restrictions on the code. No warranty is offered, and no responsibility
 * is taken for any damage use of this program may cause. In other words,
 * do what you want with it, but don't expect me to pay up if anything
 * unexpected goes wrong - you're using it at your own risk...
 *
 */

#include "ext.h"
#include "frig.h"

/* I might get round to writing an assembler version for PCs, if so
   USE_ASM will be defined */

#ifdef USE_ASM
#define maybe_static
#else
#define	maybe_static	static
#endif

#ifdef FRIG

/* Substitution array, defined in conv.c */

extern	byte	conv_array [CONV_ARRAYS][256];

/* Decryption arrays, created automatically from encryption arrays */

maybe_static	byte	decrypt_array [CONV_ARRAYS][256];

/* Pointer to array for each round and byte, generated once per key */

maybe_static	byte	*arrays [ROUNDS][BLOCK_SIZE];

/* Permutation requires us to use two blocks and swap between them,
   unfortunately. */

maybe_static	byte	d1 [BLOCK_SIZE];

/* Permutation function to use */

#ifdef USE_ASM
byte	decrypting;
#else
static	void	(*permute)();
#endif

/* Encryption permutation */

#ifndef USE_ASM
static	void	permute_forward (d, d1)

byte	*d;
byte	*d1;

{
	byte	*s, *x, *y, *e;
	byte	a, b;

	s = d1;
	e = s + BLOCK_SIZE;
	x = d;
	y = d + (BLOCK_SIZE - 1);

	while (s < e) {
		a = *x++;
		b = *y--;

		*s++ = (a & 0xF0) | (b & 0x0F);
		*s++ = (a & 0x0F) | (b & 0xF0);
	}
}

/* Decryption permutation */

static	void	permute_backward (d, d1)

byte	*d;
byte	*d1;

{
	byte	*s, *x, *y, *e;
	byte	a, b;

	s = d;
	e = s + BLOCK_SIZE;
	x = d1;
	y = d1 + (BLOCK_SIZE - 1);

	while (s < e) {
		a = *s++;
		b = *s++;

		*x++ = (a & 0xF0) | (b & 0x0F);
		*y-- = (a & 0x0F) | (b & 0xF0);
	}
}
#endif /* USE_ASM */

/* Take key, generate arrays */

void	initfrig (key, decrypt)

byte	key [KEY_SIZE];
byte	decrypt;

{
	int	i, j, k;
	static	int	created = FALSE;

	/* Create decryption arrays */

	if (decrypt && !created) {
		for (i = 0; i < CONV_ARRAYS; i++) {
			for (j = 0; j < 256; j++) {
				decrypt_array [i] [conv_array[i][j]] = j;
			}
		}

		created = TRUE;
	}

#ifndef USE_ASM
	/* Choose permutation function */

	if (decrypt) {
		permute = permute_backward;
	}
	else {
		permute = permute_forward;
	}
#else
	decrypting = decrypt;
#endif

	/* Perform initialisation stuff here */

	for (i = 0; i < ROUNDS; i++) {
		for (j = 0; j < BLOCK_SIZE; j++) {
			k = key [((j+i) / 2) & (KEY_SIZE-1)];
			if (!(j&1))
				k >>= 4;
			k &= 0xF;

			k+= (i*4 + j);

			k &= (CONV_ARRAYS-1);

			if (decrypt)
				arrays [(ROUNDS-1)-i][j] = decrypt_array [k];
			else	
				arrays [i][j] = conv_array [k];
		}
	}
}

#ifndef USE_ASM
/* Process a BLOCK_SIZE sized block of bytes */

void	frigblock (d)

byte	*d;

{
	byte	**a, *s, *e;
	int	i;
	int	j, k, l;

	/* Initial permutation */

	(*permute) (d, d1);

	s = d1;

	for (i = 0; i < ROUNDS ; i+=2) {

		/* Perform substitutions */

		a = arrays [i];
		e = s + BLOCK_SIZE;
		while (s < e) {
			*s = *(*(a++)+*s);
			s++;
		}

		/* Permute */

		(*permute) (d1, d);
		s = d;

		/* Substitution */

		a = arrays [i+1];
		e = s + BLOCK_SIZE;
		while (s < e) {
			*s = *(*(a++)+*s);
			s++;
		}

		/* Permutation */

		(*permute) (d, d1);
		s = d1;
	}

	/* Copy result back into input */

	for (i = 0; i < BLOCK_SIZE; i++)
		*d++ = *s++;
}
#endif /* USE_ASM */

/* Shut down code and clear all sensitive data */

void	closefrig ()

{
	memset (d1, 0, BLOCK_SIZE);
	memset (arrays, 0, (ROUNDS*BLOCK_SIZE*sizeof (void *)));
}
#endif

