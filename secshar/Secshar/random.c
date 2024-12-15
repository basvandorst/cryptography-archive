#include "crypt.h"

/* get_chars returns  a string of the requested number of characters. */
void get_chars(int n, char *buffer)
{
	char letter;
	int	i,num;
	
	for(i=1;i<=n;i++) {
	  letter = getchar();
	  printf("\b");
	  strncat(buffer,&letter,1);
	}
}

/* truerand generates n unsigned ints of random numbers
   from keyboard timings. */
void truerand(buff,n)
unsigned *buff,n;
{
	unsigned i,j,accume, t_stamp;
	static unsigned pool[5] = {0,0,0,0,0};
	static unsigned first=TRUE, pool_key[7][10];
	Ukey usekey;
	char charbuffer[1024]="";
	
	if(first) {
	  /* this is just a constant key for hashing the pool */
	  usekey[1] = 0xAAAAU; /*10101010 10101010*/
	  usekey[2] = 0xCCCCU; /*11001100 11001100*/
	  usekey[3] = 0x6666U; /*01100110 01100110*/
	  usekey[4] = 0xFAB4U; /*11111010 10110100*/
	  usekey[5] = 0xb646U; /*10110110 01000110*/
	  usekey[7] = 0x5654U; /*01010110 01010100*/
	  usekey[8] = 0xF95CU; /*11111001 01011100*/
	  key(usekey,pool_key);
	  first = FALSE;
	}
	fprintf(stderr,"Enter %u random characters for random number generation.\n",16*n);
	get_chars(16*n, charbuffer);
	for(i=0;i<n;i++) { /* loop through for each int */
	  for(j=0;j<3;j++) { /* assume one bit per character, and none for time */
	    t_stamp = time(NULL);
	    pool[1] ^= charbuffer[i*16 + j*4];
	    pool[2] ^= charbuffer[i*16 + j*4 +1];
	    pool[3] ^= charbuffer[i*16 + j*4 +2];
	    pool[4] ^= charbuffer[i*16 + j*4 +3];
	    pool[1] ^= t_stamp;
	    cip(pool,pool,pool_key); /* hash the pool */
	  }
	  buff[i]=pool[1]; /* take the first part of the pool as the random unsigned */
	}
	printf("\nThat is enough text.\n");
}


/* rand_init initializes the IDEA key and 64 bit buffer. */
void rand_init(uskey,buff,counter)
short unsigned *uskey;
unsigned *buff,*counter;
{
	unsigned	randbuff[13];
	
	truerand(randbuff,13); /* get 13 unsigned ints of real random numbers */

	uskey[1] = randbuff[0];
	uskey[2] = randbuff[1];
	uskey[3] = randbuff[2];
	uskey[4] = randbuff[3];
	uskey[5] = randbuff[4];
	uskey[6] = randbuff[5];
	uskey[7] = randbuff[6];
	uskey[8] = randbuff[7];
	
	buff[1] = randbuff[8];
	buff[2] = randbuff[9];
	buff[3] = randbuff[10];
	buff[4] = randbuff[11];
	
	*counter = randbuff[12];
}


/* nrand produces n random unsigned integers.
   if the procedure has not been called before, it
   is initialized, or if n<=0 */
void nrand(rnum,n)
unsigned *rnum,n;
{
	Ukey uskey;
	static unsigned	counter,first=TRUE;
	static Idata buff;
	static Ikey Z;
	int i,j;
	
	if(n<0 || first) {
	  rand_init(uskey,buff,&counter);
	  key(uskey,Z);
	  first=FALSE;
	}
	if(n<=0) n=1;	
	for(j=0;j<n;j++) {
	  for(i=1;i<=5;i++) {
	    counter++;
	    buff[i] ^= counter;
	  }
	  cip(buff,buff,Z); /* turn the crank */
	  rnum[j] = buff[1]; /* output the next random number */
	}
}

/* prand just returns one unsigned random number using nrand */
/* reinit random number generator with n<0 */
unsigned prand(n)
unsigned n;
{
	unsigned numb;
	if(n>0) n=1;
	nrand(&numb,n);
	return numb;
}
