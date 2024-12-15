/* stubs for the necessary functions from the standard kerberos des library 
 *
 * to save you having to look it up from des.h:
 *    	typedef unsigned char des_cblock[8];   
 *	typedef struct des_ks_struct { des_cblock _; } des_key_schedule[16];
 *
 */

#include <des.h>


/* the four actual des function stubs */

int des_key_sched(des_cblock k, des_key_schedule s)
{

}

int des_ecb_encrypt(des_cblock in, des_cblock out, 
		    des_key_schedule s, int direction)
{
	/* direction: 0 => decrypt, other => encrypt */

}

int des_cbc_encrypt(des_cblock *in, des_cblock *out, long len, 
		    des_key_schedule s, des_cblock *init, int direction)
{
	/* direction: 0 => decrypt, other => encrypt */

}

int des_pcbc_encrypt(des_cblock *in, des_cblock *out, long len, 
		     des_key_schedule s, des_cblock *init, int direction)
{
	/* direction: 0 => decrypt, other => encrypt */

}


/* support routines for key handling */

int des_read_password(des_cblock *k, char *prompt, int verify)
{

}

int des_string_to_key(char *str, des_cblock *k)
{

}

void des_set_random_generator_seed(des_cblock k)
{

}

int des_new_random_key(des_cblock k)
{

}

void des_fixup_key_parity(des_cblock k)
{

}


