#include <stdio.h>
#ifdef VMS
#	include <string.h>		/* For memset/memcpy */
#else
#	include <memory.h>
#endif

#include "descrip.h"
#include "b_order.h"

#define STRING_OUTPUT			/* Define this for comparing   */
					/* strings, otherwise you will */
					/* get a numerically correct   */
					/* answer.  Also define down   */
					/* in the LGI$HPWD routine.    */


typedef struct dsc$descriptor_s string;


extern int lgi$hpwd (
	string *output_hash,
	string *password,
	unsigned char encrypt,
	unsigned short salt,
	string *username);

extern void reverse_buf (char *buf, int size);



main()
{
char obuf[8];				/* Output buffer */
char pbuf[31];				/* Password buffer */
char ubuf[31];				/* Username buffer */
unsigned char encrypt;			/* Encryption algorithm */
unsigned short salt;			/* Random salt from SYSUAF.DAT */
unsigned long l_low, l_high;		/* High and low part of hash */
int temp;


/* Initialize descriptors & buffers */

#ifndef __alpha
struct dsc$descriptor_s outdsc = {sizeof(obuf),0,0,obuf};
struct dsc$descriptor_s pwddsc = {sizeof(pbuf),0,0,pbuf};
struct dsc$descriptor_s usrdsc = {sizeof(ubuf),0,0,ubuf};
#else
struct dsc$descriptor_s outdsc = {sizeof(obuf),0,0,obuf};
struct dsc$descriptor_s pwddsc = {sizeof(pbuf),0,0,pbuf};
struct dsc$descriptor_s usrdsc = {sizeof(ubuf),0,0,ubuf};

outdsc.dsc$a_pointer = (char *)&obuf;
pwddsc.dsc$a_pointer = (char *)&obuf;
usrdsc.dsc$a_pointer = (char *)&obuf;
#endif /* __alhpa */ /* Thanks Ehud */

memset (obuf, 0, sizeof(obuf));
memset (pbuf, 0, sizeof(pbuf));
memset (ubuf, 0, sizeof(ubuf));


printf("Enter username in uppercase:  ");
fgets(ubuf, 31, stdin);			/* fgets stores the newline    */
usrdsc.dsc$w_length = strlen(ubuf)-1;	/* Get rid of the CR character */


printf("Enter password in uppercase:  ");
fgets(pbuf, 31, stdin);
pwddsc.dsc$w_length = strlen(pbuf)-1;

printf("Enter the encryption method [1,2,3]:  ");
scanf("%d", &temp);			/* Read the encryption method */
encrypt=(unsigned char)temp;		/* Convert from ASCII to decimal */


printf("Enter the random salt [0-65535]:  ");
scanf("%d", &temp);			/* Read the short */
salt=(unsigned short)temp;


/* Hash the password */
lgi$hpwd(&outdsc, &pwddsc, encrypt, salt, &usrdsc);


/* Get the numeric answer */
#ifdef STRING_OUTPUT

   /* Examine the characters in the output buffer */
   printf("\nHash (8 decimal values):  ");
   for (temp=0; temp<=7; temp++) printf("%d ", *(obuf+temp));
   printf ("\n");

   /* Extract the numeric answer */
   if (BIG_ENDIAN) {
      reverse_buf(obuf, 8);
      memcpy(&l_low, obuf+4, 4);
      memcpy(&l_high, obuf, 4);
      }
   else {
      memcpy(&l_low, obuf, 4);
      memcpy(&l_high, obuf+4, 4);
   }


#else                 		/* Numerically correct output */

   /* Output buffer is already formatted for correct byte order, */
   /* so just copy the bytes over into the quadword.             */
   memcpy(&l_low, obuf, 4);
   memcpy(&l_high, obuf+4, 4);

#endif


printf("\nNumeric result (high low):  %08x %08x\n", l_high, l_low);

} /* main */
