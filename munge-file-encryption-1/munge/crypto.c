#include <stdio.h>
#include "idea.h"
#include "md5.h"
#include "d3des.h"
#include <sys/time.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <unistd.h>

#define ENCRYPT_IT FALSE	/* to pass to xx_file */
#define DECRYPT_IT TRUE	/* to pass to xx_file */
byte textbuf[DISKBUFSIZE];
extern int no_header;
extern int algorithm;
byte nk[8];
byte wbuf[128];
extern long tvbuf[];
extern int tvc;
extern struct stat qq;
extern int quiet;
extern int test_mode;
extern int two_keys;
extern int emode;
unsigned char rtmp[TVBUFSIZ + 2048];
static int rtlen = 0;
extern struct timeval init_time;
int dptb = 1;
extern int noenptb;  	/* bflag */
#define USE_RANDOM

#if defined(linux) || defined (_AIX) || defined(sun) || defined(sgi)
#define GETRUSAGE
#endif

#ifdef GETRUSAGE
#include <sys/resource.h>
#endif

byte rg() {
static MD5_CTX context;
int length;
int i;
static unsigned char digtmp[16];
static int digtmpptr=16;
static int initialised = 0;
static int iseed;
#ifdef GETRUSAGE
struct rusage foo3;
#endif
unsigned char retval;

	if (!initialised) {
		int i;
		char *s;
		int j = 0;
		struct timeval foo;
		struct tms foo2;
		struct timezone foo1;
		initialised = 1;

		iseed = time(NULL);
		j=0;
		for (i=0;i<256;i++) {
			iseed = (iseed * 31421 + 6927) & 0xffff;
			rtmp[i] = iseed & 0xff;
		}
		memcpy(&rtmp[j],&qq,sizeof(struct stat)); j+=sizeof(struct stat);
		gettimeofday(&foo,&foo1);   /* usually some delay between init_time and foo */
		memcpy(&rtmp[j],&foo,sizeof(struct timeval));j+=sizeof(struct timeval);
		memcpy(&rtmp[j],&init_time,sizeof(struct timeval));j+=sizeof(struct timeval);
		i=getpid(); memcpy(&rtmp[j],&i,sizeof(int)); j+=sizeof(int);
		i=getppid(); memcpy(&rtmp[j],&i,sizeof(int)); j+=sizeof(int);
		i=times(&foo2);
		memcpy(&rtmp[j],&foo2,sizeof(struct tms));j+=sizeof(struct tms);
		memcpy(&rtmp[j],&i,sizeof(int));j+=sizeof(int);
#ifdef GETRUSAGE
		getrusage(RUSAGE_SELF, &foo3);
		memcpy(&rtmp[j],&foo3,sizeof(struct rusage));j+=sizeof(struct rusage);
#endif
		if (tvc) {
		memcpy(&rtmp[j],&tvbuf[0],sizeof(long)*tvc); j+=sizeof(long)*tvc;
		}
		if (test_mode) {
			fprintf(stderr, "j=%d, tvc = %d, rb =\n",j,tvc);
			for (i=0;i<j;i++)
				fprintf(stderr, "%02x", rtmp[i]);
			fprintf(stderr, "\n");
		}
		rtlen = (j > 256) ? j : 256;
		for (i=0;i<rtlen;i++) {
			iseed = (iseed * 31421 + 6927) & 0xffff;
			rtmp[i] ^= iseed & 0xff;
		}
        MD5Init(&context);
        MD5Update(&context, &rtmp[0], rtlen);
        MD5Final(&context);
		for (i=0;i<16;i++)
			digtmp[i] = context.digest[i];
		digtmpptr = 16;
		/* reseed */
	}
	if (digtmpptr >= 15) {
		for (i=0;i<15;i++) {
			rtmp[i] = context.digest[i];
			rtmp[32-i] ^= context.digest[i];
		}
		for (i=32;i<256;i++) {
			iseed = (iseed * 31421 + 6927) & 0xffff;
			rtmp[i] ^= iseed & 0xff;
		}
		if (!(context.digest[16] & 0x07)) {
			i = iseed % 11;
			memcpy(&iseed, &context.digest[i],sizeof(int));
			iseed &= 0xffff;
		}
		MD5Init(&context);
		MD5Update(&context, &rtmp[0], rtlen);
		MD5Final(&context);
		for (i=0;i<16;i++)
			digtmp[i] ^= (context.digest[i] ^ (i+5));
		retval = digtmp[0];
		digtmpptr = 1;
	}
	else {
		retval = digtmp[digtmpptr];
		digtmpptr++;
	}
	return retval;
}

int xx_file(byte *xxkey, boolean decryp, FILE *f, FILE *g, word32 lenfile,
	byte *xxkey2, int no_write)
/*	Use IDEA in cipher feedback (CFB) mode to encrypt or decrypt a file.
	The encrypted material starts out with a 64-bit random prefix, which
	serves as an encrypted random CFB initialization vector, and
	following that is 16 bits of "key check" material, which is a
	duplicate of the last 2 bytes of the random prefix.  Encrypted key
	check bytes detect if correct IDEA key was used to decrypt ciphertext.

	no_write == 1 prevents writing to FILE *g on DECRYPTION ONLY
		used for validation pass
*/
{	int count;
	word16 iv[4];
	unsigned char pkey1[16],pkey2[16],ptb[8],eptb[8];
	int mdchk = 1;
	int keychk = 0;
	MD5_CTX mdcontext;
	int i;
#define RAND_PREFIX_LENGTH 8
	/* note: we ptb key iff we don't do key check bytes */
	dptb = 1;
	if (no_header) {
		mdchk = 0;
		keychk = 1;
	}
	if (test_mode) {
		fprintf(stderr, "debug: key = ");
		for (count=0; count<16;count++) 
			fprintf(stderr, "%02x", xxkey[count]);
		fprintf(stderr, "\n");
		if (two_keys) {
		fprintf(stderr, "debug: key2 = ");
		for (count=0; count<16;count++) 
			fprintf(stderr, "%02x", xxkey2[count]);
		fprintf(stderr, "\n");
		}
	}

	if (!decryp)	 /* encrypt */
	{	
		int	i;
		unsigned char c = (char)algorithm;
		/* init CFB key */
		if (!no_header) {
			if (!emode) {
				if (!noenptb)		/* mode Q */
					fwrite("\320NCRYPTD ", 1, 9, g);
				else		/* mode P */
					fwrite("PNCRYPTD ", 1, 9, g);
				fwrite(&c, 1, 1, g);
				keychk = 0;
			}
			else {
				fwrite("ENCRYPTD ", 1, 9, g);
				fwrite(&c, 1, 1, g);
				keychk = 1;
			}
		}
		if (keychk) {
			for (i=0;i<RAND_PREFIX_LENGTH;++i)
				textbuf[i]=rg();
			/* key check bytes are simply duplicates of final 2 random bytes */
			textbuf[i] = textbuf[i-2];	/* key check bytes for redundancy */
			textbuf[i+1] = textbuf[i-1];
			bzero(iv,sizeof(iv));	/* define initialization vector IV as 0 */
			initcfb_xx(iv,xxkey,decryp,xxkey2);
			xxcfb(textbuf,RAND_PREFIX_LENGTH+2);
			fwrite(textbuf,1,RAND_PREFIX_LENGTH+2,g);
		} /* keychk */
		else { /* !keychk */
			for (i=0;i<8;i++)
				ptb[i]=rg();
			if (!noenptb) {   /* MODE Q */
			    MD5_CTX tcontext;
				/* only partial key info goes into this for a reason 
				   in particular it really only depends on the
				   xor of the first and second 64 bit segments of the key
				*/
				memcpy(&wbuf[0],&xxkey[0],8);
				for (i=0;i<8;i++)
					wbuf[i] ^= xxkey[i+8];
				for (i=8;i<128;i++)
					wbuf[i] = (wbuf[(wbuf[i-1]*7+2) % 8] ^ i + 57) & 0xff;
				MD5Init(&tcontext);
				MD5Update(&tcontext, &wbuf[0], 128);
				MD5Final(&tcontext);
				memcpy(&nk[0],&tcontext.digest[0],8);
				deskey(&nk[0],EN0);
				if (test_mode) {
					fprintf(stderr, "debug:raw_ptb =");
					for (count=0; count<8;count++) 
						fprintf(stderr, "%02x", ptb[count]);
					fprintf(stderr, "\n");
				}
				des(&ptb[0],&eptb[0]);
			}
			else 
				memcpy(&eptb[0],&ptb[0],8);  /* mode P */
			if (test_mode) {
				fprintf(stderr, "debug:enc_ptb =");
				for (count=0; count<8;count++) 
					fprintf(stderr, "%02x", eptb[count]);
				fprintf(stderr, "\n");
			}
			if (test_mode) {
				fprintf(stderr, "nk = ");
				for (i=0;i<8;i++)
					fprintf(stderr, "%02x",nk[i]);
				fprintf(stderr, "\n");
			}
			fwrite(&eptb[0],1,8,g);
			deskey(&ptb[0],EN0);
			des(&xxkey[0],&pkey1[0]);
			des(&xxkey[8],&pkey1[8]);
			if (two_keys) {
				des(&xxkey2[0],&pkey2[0]);
				des(&xxkey2[8],&pkey2[8]);
			}
			if (test_mode) {
				fprintf(stderr, "debug:ptb =");
				for (count=0; count<8;count++) 
					fprintf(stderr, "%02x", ptb[count]);
				fprintf(stderr, "\n");
				fprintf(stderr, "debug:pkey = ");
				for (count=0; count<16;count++) 
					fprintf(stderr, "%02x", pkey1[count]);
				fprintf(stderr, "\n");
				if (two_keys) {
				fprintf(stderr, "debug:pkey2 = ");
				for (count=0; count<16;count++) 
					fprintf(stderr, "%02x", pkey2[count]);
				fprintf(stderr, "\n");
				}
			}
			bzero(iv,sizeof(iv));	/* define initialization vector IV as 0 */
			initcfb_xx(iv,pkey1,decryp,pkey2);
		} /* !keychk */
	}
	else	/* decrypt */
	{	/* See if the redundancy is present after the random prefix */
		if ((count = fread(textbuf,1,RAND_PREFIX_LENGTH+2,f)) < RAND_PREFIX_LENGTH + 2) 
			return (-3);
		if (!no_header) {
			if (!strncmp(textbuf, "ENCRYPT", 7)) {
					keychk = 1;
				if (!strncmp(textbuf,"ENCRYPTED", 9)) 
					mdchk = 0;
					algorithm = (int)textbuf[9];
					if (algorithm < 0 || algorithm > 3) {
						fprintf(stderr, "file header corruption\n");
						return (-2);
					}
					if (algorithm == 3 && !two_keys) {
						fprintf(stderr, "warning: algorithm 3, single key\n");
						algorithm = 0;
					}
					if (!quiet)
						fprintf(stderr, "\talgorithm = %d\n", algorithm);
					count = fread(textbuf,1,RAND_PREFIX_LENGTH+2,f);
					lenfile-=count;
			} 
			else if (!strncmp(textbuf,"PNCRYPTD ", 9)) {
			/* note that we're not going to alter textbuf here */
				keychk = 0;
				mdchk = 1;
				dptb = 0;
				algorithm = (int)textbuf[9];
				if (algorithm < 0 || algorithm > 3) {
					fprintf(stderr, "file header corruption\n");
					return (-2);
				}
				if (algorithm == 3 && !two_keys) {
					fprintf(stderr, "warning: algorithm 3, single key\n");
					algorithm = 0;
				}
				if (!quiet)
					fprintf(stderr, "\talgorithm = %d\n", algorithm);
			}
			else if (!strncmp(textbuf,"\320NCRYPTD ", 9)) {
			/* note that we're not going to alter textbuf here */
				keychk = 0;
				mdchk = 1;
				dptb = 1;
				algorithm = (int)textbuf[9];
				if (algorithm < 0 || algorithm > 3) {
					fprintf(stderr, "file header corruption\n");
					return (-2);
				}
				if (algorithm == 3 && !two_keys) {
					fprintf(stderr, "warning: algorithm 3, single key\n");
					algorithm = 0;
				}
				if (!quiet)
					fprintf(stderr, "\talgorithm = %d\n", algorithm);
			}
			else {
				mdchk=0;
				keychk=1;
			}
		}
		lenfile -= count;
		if (keychk) {
			bzero(iv,sizeof(iv));	/* define initialization vector IV as 0 */
			initcfb_xx(iv,xxkey,decryp,xxkey2);
			xxcfb(textbuf,RAND_PREFIX_LENGTH+2);
			if (count==(RAND_PREFIX_LENGTH+2))
			{	

				if ((textbuf[RAND_PREFIX_LENGTH] != textbuf[RAND_PREFIX_LENGTH-2])
					|| (textbuf[RAND_PREFIX_LENGTH+1] != textbuf[RAND_PREFIX_LENGTH-1]))
				{	return(-2);		/* bad key error */
				}
			}
			else	/* file too short for key check bytes */
				return(-3);		/* error of the weird kind */
		} /* if keychk */
		else { /* !keychk */
			if ((count = fread(ptb,1,8,f)) < 8) {
				fprintf(stderr, "short file\n");
				return(-7);
			}
			lenfile-=count;
			if (dptb)  {
			    MD5_CTX tcontext;
				memcpy(&wbuf[0],&xxkey[0],8);
				for (i=0;i<8;i++)
					wbuf[i] ^= xxkey[i+8];
				for (i=8;i<128;i++)
					wbuf[i] = (wbuf[(wbuf[i-1]*7+2) % 8] ^ i + 57) & 0xff;
				MD5Init(&tcontext);
				MD5Update(&tcontext, &wbuf[0], 128);
				MD5Final(&tcontext);
				memcpy(&nk[0],&tcontext.digest[0],8);
				deskey(&nk[0],DE1);
				if (test_mode) {
					fprintf(stderr, "debug:enc_ptb =");
					for (count=0; count<8;count++) 
						fprintf(stderr, "%02x", ptb[count]);
					fprintf(stderr, "\n");
				}
				des(&ptb[0],&ptb[0]);
				if (test_mode) {
					fprintf(stderr, "debug:raw_ptb =");
					for (count=0; count<8;count++) 
						fprintf(stderr, "%02x", ptb[count]);
					fprintf(stderr, "\n");
				}
				if (test_mode) {
					fprintf(stderr, "nk = ");
					for (i=0;i<8;i++)
						fprintf(stderr, "%02x",nk[i]);
					fprintf(stderr, "\n");
				}
			}
			deskey(&ptb[0],EN0);
			des(&xxkey[0],&pkey1[0]);
			des(&xxkey[8],&pkey1[8]);
			if (two_keys) {
				des(&xxkey2[0],&pkey2[0]);
				des(&xxkey2[8],&pkey2[8]);
			}
			if (test_mode) {
				fprintf(stderr, "debug:ptb =");
				for (count=0; count<8;count++) 
					fprintf(stderr, "%02x", ptb[count]);
				fprintf(stderr, "\n");
				fprintf(stderr, "debug:pkey = ");
				for (count=0; count<16;count++) 
					fprintf(stderr, "%02x", pkey1[count]);
				fprintf(stderr, "\n");
				if (two_keys) {
				fprintf(stderr, "debug:pkey2 = ");
				for (count=0; count<16;count++) 
					fprintf(stderr, "%02x", pkey2[count]);
				fprintf(stderr, "\n");
				}
			}
			bzero(iv,sizeof(iv));	/* define initialization vector IV as 0 */
			initcfb_xx(iv,pkey1,decryp,pkey2);
		}
		if (mdchk)
			lenfile-=16;
	}
	MD5Init(&mdcontext);

	do	/* read and write the whole file in CFB mode... */
	{	count = (lenfile < DISKBUFSIZE) ? (int)lenfile : DISKBUFSIZE;
		count = fread(textbuf,1,count,f);
		lenfile -= count;
		if (count>0)
		{	
			if (!decryp) 
				MD5Update(&mdcontext, (unsigned char *)textbuf, count);
			xxcfb(textbuf,count);
			if (decryp)
				MD5Update(&mdcontext, (unsigned char *)textbuf, count);
			if (!no_write)
				fwrite(textbuf,1,count,g);
		}
		/* if text block was short, exit loop */
	} while (count==DISKBUFSIZE);

	MD5Final(&mdcontext);
	if (!quiet) {
		fprintf(stderr, "MD5 = ");
		for (i=0;i<16;i++) 
			fprintf(stderr, "%02x ", mdcontext.digest[i]);
		fprintf(stderr, "\n");	
	}
	if (!decryp && mdchk)
		fwrite(&mdcontext.digest[0],1,16,g);
	if (decryp && mdchk) {
		count = fread(textbuf,1,16,f);
		if (count < 16) 
			return(-2);
		else 
			for (i=0;i<16;i++)
				if (mdcontext.digest[i] != textbuf[i]) 
					return(-7);
	}
	close_xx();		/* Clean up data structures */
	cleanup_des();	/* Clean up data structures */
	return(0);	/* should always take normal return */
}	/* xx_file */


