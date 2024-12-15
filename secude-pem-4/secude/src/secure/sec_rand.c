/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1994, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

/*-----------------------------------------------------------------------*/
/* sec_random.c: Funktionen sec_random_*()                               */
/*-----------------------------------------------------------------------*/

#include "secure.h"
extern ObjId *Random_OID;
#ifdef TEST
#include <stdio.h>
#endif
#include "arithmetic.h"

extern int      errno;


/* Parameters of random number generator *
 *
 * random_seed = random_seed * random_mult + random_add ( mod 2 ^ (32*RANDOM_SIZE) )
 *
 * RANDOM_SIZE: Number of integers ( 2..8 )
 * random_seed: Actual random seed number of length RANDOM_SIZE ( Must have memory for 2*RANDOM_SIZE integers)
 * random_mult: A number of length RANDOM_SIZE which must be 1 ( mod 4) 
 * random_add:  A number of length RANDOM_SIZE which must be 1 ( mod 2)
 */

#define RANDOM_SIZE 5

static L_NUMBER random_seed[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static L_NUMBER random_mult[] = { RANDOM_SIZE, 0x12345675, 0x89abcdef, 0x47110815, 0x1f2e3d4c, 0x256345fc, 0xfdbc2454, 0x2345fbac, 0xfd6a87fb};
static L_NUMBER random_add[] =  { RANDOM_SIZE, 0x23571113, 0x17192123, 0x29313741, 0x43475359, 0x456fbcae, 0x456fbd67, 0x245fbcdd, 0x43563567};




Boolean random_init_from_tty = FALSE;	/* if TRUE, init random number from TTY */
Boolean random_from_pse = FALSE;	/* if TRUE, read random number from PSE */
Boolean random_update_always = TRUE;	/* if TRUE, update random number of PSE after each request for a
					   random number ( if random_from_pse == TRUE only ) */
Boolean random_destroy = FALSE;		/* if TRUE, destroy random seed after each request for a
					   random number  */





/***************************************************************
 *
 * Procedure rndout
 *
 ***************************************************************/
#ifdef __STDC__

static int rndout(
)

#else

static int rndout(
)

#endif

{
	int n;
	printf("%x %x %x\n",  random_from_pse, random_update_always, random_destroy);
	for(n=0; n<=random_seed[0]; n++) printf("%x ",  random_seed[n]);
	printf("\n");
	for(n=0; n<=random_mult[0]; n++) printf("%x ",  random_mult[n]);
	printf("\n");
	for(n=0; n<=random_add[0]; n++) printf("%x ",  random_add[n]);
	printf("\n");

	return(0);
}


/***************************************************************
 *
 * Procedure sec_init_random_seed_from_keyboard
 *
 ***************************************************************/
#ifdef __STDC__

static RC sec_init_random_seed_from_keyboard(
)

#else

static RC sec_init_random_seed_from_keyboard(
)

#endif

{
	char	    * proc = "sec_init_random_seed_from_keyboard";
	char 	input[1000];
	int len, pos = 0, n = 1, bit = 0;

	do {
		if(aux_input_from_device("Input random string : ", input, FALSE) < 0 ) {
			aux_add_error(EMALLOC, "Can't get tty input", CNULL, 0, proc);
			return(-1);
		}
	} while (!(len = strlen(input)));

	while (n<=RANDOM_SIZE) {
		random_seed[n] ^= input[pos++] << bit;
		bit += 4;
		if(bit == 32 ) {
			n++;
			bit = 0;
		}
		if (pos == len) pos = 0;
	}
	random_seed[0] = RANDOM_SIZE;

	for(n = 0; n<len; n++) input[n] = '\0';

	
	return(0);
}

/***************************************************************
 *
 * Procedure init_random_seed
 *
 ***************************************************************/
#ifdef __STDC__

static RC init_random_seed(
)

#else

static RC init_random_seed(
)

#endif

{
	time_t 	 ti;
	int 	 mem, n,  pid;
	char 	 buf[2048],
		*seed = (char *) &random_seed[1];
	char	*proc = "init_random_seed";

	time(&ti);
	pid = (int) getpid();

	mem = open("/dev/mem", 0);
	read(mem, buf, 2048);
	close(mem);

	for(n = 0; n<RANDOM_SIZE; n++) {
		seed[n] += (char) ti;
	}
	for(n = RANDOM_SIZE; n<2*RANDOM_SIZE; n++) {
		seed[n] += (char) pid;
	}
	for(n = 2*RANDOM_SIZE; n<3*RANDOM_SIZE; n++) {
		seed[n] += ((int) &ti) & 255;
	}
	for(n = 3*RANDOM_SIZE; n<4*RANDOM_SIZE; n++) {
		seed[n] += (int) seed;
	}
	for(n = 0; n<2048; n++) {
		seed[n % (4*RANDOM_SIZE)] = seed[n % (4*RANDOM_SIZE)] ^ buf[n];
	}

	random_seed[0] = RANDOM_SIZE;

	return(0);
}


/***************************************************************
 *
 * Procedure sec_init_random
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_init_random(
	L_NUMBER	  seed[]
)

#else

RC sec_init_random(
	seed
)
L_NUMBER	  seed[];

#endif

{
	char	    * proc = "sec_init_random";

	if(seed) {
		if(seed[0]>RANDOM_SIZE || seed[0]<1) {
			aux_add_error(EINVALID, "Invalid seed length", CNULL, 0, proc);
			return (-1);
		}
		
		trans(seed, random_seed);
		return(0);
	}

    	if(random_seed[0]) return(0);

	if(random_init_from_tty)
	    if(sec_init_random_seed_from_keyboard() < 0) {
		aux_add_error(EINVALID, "Can't get random number", CNULL, 0, proc);
		return (-1);
	    }
	    else return(0);

	if(random_from_pse) 
	    if(sec_get_random() < 0) {
		aux_add_error(EINVALID, "Can't get random number", CNULL, 0, proc);
		return (-1);
	    }
	
	if(!random_seed[0]) 
	    if(init_random_seed() < 0) {
		aux_add_error(EINVALID, "Can't init random number", CNULL, 0, proc);
		return (-1);
	    }

	return(0);
}


/***************************************************************
 *
 * Procedure sec_get_random
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_get_random(
)

#else

RC sec_get_random(
)

#endif

{
	PSESel      * pse_sel;
	OctetString   content, *random;
	ObjId         objid;
	char	    * proc = "sec_get_random";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);	
#endif


	if (!(pse_sel = (PSESel *)af_pse_open(Random_OID, FALSE))) {
		return (1);
	}


	if (sec_read_PSE(pse_sel, &objid, &content) < 0 ) {
		/*aux_add_error(EREADPSE, "sec_read_PSE failed", (char *) pse_sel, PSESel_n, proc);*/
        	aux_free_PSESel(&pse_sel);
		return (1);
	}
       	aux_free_PSESel(&pse_sel);
        if(aux_cmp_ObjId(&objid, Random_OID)) {
		aux_add_error(EDAMAGE, "PSE object has wrong objid", CNULL, 0, proc);
		return (-1);
	}

	random = d_OctetString(&content);
	if (! random) {
		aux_add_error(EDECODE, "d_OctetString failed", CNULL, 0, proc);
		return (-1);
	}
	while(content.noctets) content.octets[--content.noctets] = 0;
	free(content.octets);

	INTEGERtoln(random, random_seed);

	while(random->noctets) random->octets[--random->noctets] = 0;
	aux_free_OctetString(&random);


	return(0);
	

}

/***************************************************************
 *
 * Procedure sec_update_random
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_update_random(
)

#else

RC sec_update_random(
)

#endif

{
	PSESel 		* pse_sel;
	OctetString 	  random, *content;
	char		* proc = "sec_update_random", octets[4*RANDOM_SIZE];
	int 		  ret;

#ifdef TEST
	fprintf(stderr, "%s\n", proc);	
#endif
	

	if (!(pse_sel = (PSESel *)af_pse_open(Random_OID, TRUE))) {
		aux_add_error(LASTERROR, "af_pse_open failed", CNULL, 0, proc);
		return (-1);
	}

	random.noctets = 0;
	random.octets = octets;

	lntoINTEGER(random_seed, &random);

	if (!(content = e_OctetString(&random)) ) {
		aux_free_PSESel(&pse_sel);
		aux_add_error(EENCODE, "e_OctetString failed", CNULL, 0, proc);
		return (-1);
	}
	while(random.noctets) octets[--random.noctets] = 0;

	ret = sec_write_PSE(pse_sel, Random_OID, content);

	if(ret<0) aux_add_error(EWRITEPSE, "sec_write_PSE failed", (char *) pse_sel, PSESel_n, proc);

	aux_free_PSESel(&pse_sel);
	aux_free_OctetString(&content);


	return(ret);
	

}
/***************************************************************
 *
 * Procedure sec_random_destroy_seed
 *
 ***************************************************************/
#ifdef __STDC__

void sec_random_destroy_seed(
)

#else

void sec_random_destroy_seed(
)

#endif

{
	int n;

	for(n = 0; n<=RANDOM_SIZE; n++) random_seed[n] = 0;
	

}







/***************************************************************
 *
 * Procedure next_random
 *
 ***************************************************************/
#ifdef __STDC__

static RC next_random(
)

#else

static RC next_random(
)

#endif

{
	char	    * proc = "next_random";
	
	if(sec_init_random(0) < 0) {
		aux_add_error(EINVALID, "Can't init random number", CNULL, 0, proc);
		return (-1);
	}

	mult(random_seed, random_mult, random_seed);
	add(random_seed, random_add, random_seed);
	if(random_seed[0]>RANDOM_SIZE) random_seed[0] = RANDOM_SIZE;

	return(0);
}


/***************************************************************
 *
 * Procedure sec_random_ostr
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *sec_random_ostr(
	unsigned int	  noctets
)

#else

OctetString *sec_random_ostr(
	noctets
)
unsigned int	  noctets;

#endif

{
	OctetString    *p;
	char           *proc = "sec_random_ostr";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (noctets <= 0)
		return (NULLOCTETSTRING);
	if (!(p = (OctetString *) malloc(sizeof(OctetString)))) {
		aux_add_error(EMALLOC, "p", CNULL, 0, proc);
		return (NULLOCTETSTRING);
	}
	if (!(p->octets = malloc(noctets))) {
		aux_add_error(EMALLOC, "p->octets", CNULL, 0, proc);
		return (NULLOCTETSTRING);
	}
	p->noctets = noctets;
	{
		int             i, j;
		char           *cp, *rnd;

		for(j = 0; j<noctets; j += 16) {
			if(next_random() < 0) {
				aux_add_error(EINVALID, "Can't get random number", CNULL, 0, proc);
				return (NULLOCTETSTRING);
			}
			rnd = (char *) &random_seed[2];
			for (i = j; i < noctets && i-j<16; i++) p->octets[i] = rnd[i-j];
		}
	}


	if(random_from_pse && random_update_always)
	    if(sec_update_random() < 0) {
		aux_add_error(EINVALID, "Can't update random number", CNULL, 0, proc);
		return (NULLOCTETSTRING);
	    }
	if(random_destroy) sec_random_destroy_seed();


	return p;
}


/***************************************************************
 *
 * Procedure sec_random_bstr
 *
 ***************************************************************/
#ifdef __STDC__

BitString *sec_random_bstr(
	unsigned int	  nbits
)

#else

BitString *sec_random_bstr(
	nbits
)
unsigned int	  nbits;

#endif

{
	OctetString    *p;
	char           *proc = "sec_random_ostr";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (nbits <= 0)
		return (NULLBITSTRING);
	if (NULLOCTETSTRING == (p = sec_random_ostr((nbits + 7) / 8)))
		return (NULLBITSTRING);
	if (nbits & 7)
		p->octets[p->noctets - 1] &= 0xFF00 >> (nbits & 7);
	p->noctets = nbits;
	return (BitString *) p;
}



/***************************************************************
 *
 * Procedure sec_random_str
 *
 ***************************************************************/
#ifdef __STDC__

char *sec_random_str(
	int	  nchars,
	char	 *chars
)

#else

char *sec_random_str(
	nchars,
	chars
)
int	  nchars;
char	 *chars;

#endif

{
	char           *p;
	char           *proc = "sec_random_ostr";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);
#endif

	if (nchars <= 0)
		return (CNULL);
	if (!(p = malloc(nchars))) {
		aux_add_error(EMALLOC, "p", CNULL, 0, proc);
		return (CNULL);
	} {
		int             i, s = 0;
		char           *cp;

		if (chars)
			s = strlen(chars);

		for (i = nchars, cp = p; i > 0; i--, cp++) {
			if(next_random() < 0) {
				aux_add_error(EINVALID, "Can't get random number", CNULL, 0, proc);
				return (CNULL);
			}
			*cp = (s ? chars[random_seed[2] % s] : random_seed[2] % (0x7E - ' ') + ' ' + 1);
		}
	}

	if(random_from_pse && random_update_always)
	    if(sec_update_random() < 0) {
		aux_add_error(EINVALID, "Can't update random number", CNULL, 0, proc);
		return (CNULL);
	    }
	if(random_destroy) sec_random_destroy_seed();

	return p;
}


/***************************************************************
 *
 * Procedure sec_random_int
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_random_int(
	int	  r1,
	int	  r2,
	int	  *ret
)

#else

RC sec_random_int(
	r1,
	r2,
	ret
)
int	  r1;
int	  r2;
int	  *ret;

#endif

{
	char           *proc = "sec_random_int";

	if (r1 >= r2)
		return (-1);

	if(next_random() < 0) {
		aux_add_error(EINVALID, "Can't get random number", CNULL, 0, proc);
		return (-1);
	}

	*ret = random_seed[2] % (r2 - r1 + 1) + r1;

	if(random_from_pse && random_update_always)
	    if(sec_update_random() < 0) {
		aux_add_error(EINVALID, "Can't update random number", CNULL, 0, proc);
		return (-1);
	    }
	if(random_destroy) sec_random_destroy_seed();

	return (0);

}


/***************************************************************
 *
 * Procedure sec_random_long
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_random_long(
	long	  r1,
	long	  r2,
	long	  *ret
)

#else

RC sec_random_long(
	r1,
	r2,
	ret
)
long	  r1;
long	  r2;
long	  *ret;

#endif

{
	char           *proc = "sec_random_long";

	if (r1 >= r2)
		return (-1);

	if(next_random() < 0) {
		aux_add_error(EINVALID, "Can't get random number", CNULL, 0, proc);
		return (-1);
	}

	*ret = random_seed[2] % (r2 - r1) + r1;

	if(random_from_pse && random_update_always)
	    if(sec_update_random() < 0) {
		aux_add_error(EINVALID, "Can't update random number", CNULL, 0, proc);
		return (-1);
	    }
	if(random_destroy) sec_random_destroy_seed();

	return (0);

}

/***************************************************************
 *
 * Procedure sec_random_unsigned
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_random_unsigned(
	unsigned int	  *ret
)

#else

RC sec_random_unsigned(
	ret
)
unsigned int	  *ret;

#endif

{
	char           *proc = "sec_random_unsigned";

	if(next_random() < 0) {
		aux_add_error(EINVALID, "Can't get random number", CNULL, 0, proc);
		return (-1);
	}

	*ret = random_seed[2];

	if(random_from_pse && random_update_always)
	    if(sec_update_random() < 0) {
		aux_add_error(EINVALID, "Can't update random number", CNULL, 0, proc);
		return (-1);
	    }
	if(random_destroy) sec_random_destroy_seed();

	return (0);

}


