/* rc4 test bed
 based on RC4 code developed by E A Young, apparently from code
 circulated on the Internet in 1994/5. Part of EAY's SSLEAY code is used here.

 Idea here is to compare singlekey/plain/cipher test time against
 single key, multiple plain/cipher pairs
 Is this approach more efficient ????

 */

#define PROTO

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "time.h"
#include "rc4.h"

static char *version="\0RC4 part of SSLeay v 0.4.3 15/06/95";

typedef  unsigned char UCHAR;

typedef struct rc4_test_pair1
	{
    char plain[8];
    char cipher[8];
    int found;
    char key_found[8];                                //
	} rc4_test_pair;

unsigned int key_buff[256];                 //used to speed key scheduling

RC4_KEY key;                                //master key is made here

RC4_KEY keytest;

#define NO_OF_TESTS  126
rc4_test_pair master_plain_cipher[NO_OF_TESTS];     //master plain/cipher test pairs
                                            //acquisition 0f these is
                                            //another question
RC4_KEY master_keys[NO_OF_TESTS];                   //master key array

unsigned char buffer[] =  "12345678";
unsigned char key4[] = {0x0, 0x0, 0x0, 0x01, 0x02, 0x0, 0x0, 0x0};
unsigned char key5[] = {0x0, 0x0, 0x0, 0x01, 0x02, 0x0, 0x0, 0x0};

unsigned int key_start = 0x0;         //start, endpoint for key test routine
unsigned int key_end   = 0xff;

unsigned char a,b,c,d,e;

clock_t start, end;                     //timer values
clock_t clock();

unsigned long blocks = 4096;            //default block count
long k;
unsigned long loops;



float tz;
int z;
int key_data_len = 8;
int buffer_len = 8;


//declarations
void rc4(unsigned char *buffer_ptr, int buffer_len, RC4_KEY *key);
static void swap_byte( unsigned char *a, unsigned char *b);
void rc4_key_test(void);

//===============

void main(int argc, char *argv[])
{
if (argc != 2) 		  /* get number of blocks */
 printf("\nNo block size specified, using %d as number of blocks.\n", blocks);
else
 sscanf(argv[1],"%I",&blocks);	    //get count

 for (z=0; z<256; z++)
	key_buff[z]=z;                  //init key_buff

    //measure raw speed

 for (a=0; a<NO_OF_TESTS; a++)      //make test data
  {
  key5[4] = a;
  RC4_set_key(&key, key_data_len, &key5);
  memcpy(&master_plain_cipher[a].plain, &buffer, buffer_len); //copy plain text
  RC4(&key, buffer_len, master_plain_cipher[a].plain, master_plain_cipher[a].cipher);
  }

rc4_key_test();         //measure key testing speed
};                      //end main
//  =============

/*  idea is based on making a master rc4_key, copying that to NO_OF_TESTS key spaces
    Then, each copy of the key is tested against a plain/ciphertect pair.
    If a match found, a flag is set for that plain/ciphertect pair, and the
    key stored with it.
    this should reduce overheads associated with key scheduling - I hope.
*/

void rc4_key_test(void)
{
int i;
int j=NO_OF_TESTS;
printf("\nMeasuring the speed with which this CPU can test for valid keys.");
loops=0;
end = clock();
start = clock();
while (end == start)
 start = clock();       //find point immediately after clock tick has happened

for( a = key_start; a<key_end; a++)
{
 key4[7] =a;
 for(b=key_start; b<key_end; b++)
 {
  key4[6]=b;
  for(c=key_start; c<key_end; c++)
  {
   key4[5]=c;

   for(d=key_start; d<key_end; d++)
   {
    key4[4]=d;
//    printf("\n number of keys found for plain/cipher pair = %d", j);
//   if(d)
//    {
//     end = clock()-start;
//	 tz =  (end/CLK_TCK);
//	 printf("\nTime taken = %6.1f seconds, number of keys tested = %ld.\n", tz, loops);
//	 printf("%6.1f ", (float)(loops/tz));
//    }
//printf("\n number of keys found for plain/cipher pair = %d", NO_OF_TESTS-j);
    for(e=key_start; e<key_end; e++)
    {
    key4[3]=e;
    RC4_set_key_test(&key, key_data_len, &key4);    //make master key
    for (i=0; i<NO_OF_TESTS; i++)                         //move master key to array
      memcpy(master_keys[i].data, &key, sizeof(key));

      for (i=0; i<NO_OF_TESTS; i++)
       {
        if (!master_plain_cipher[i].found)
        {
        loops++;                             //increment this as a counter
         if(RC4_test(master_keys[i].data, buffer_len, master_plain_cipher[i].plain, master_plain_cipher[i].cipher))
          {                                                  //till key matches
          master_plain_cipher[i].found = 1;
          memcpy(master_plain_cipher[i].key_found, key4, 8);
          j--;
//          printf("\n number of keys found for plain/cipher pair = %d", NO_OF_TESTS-j);
          if (!j)
           {
           end = clock()-start;
	       tz =  (end/CLK_TCK);
	       printf("\nTime taken = %6.1f seconds", tz);
	       printf("\nSpeed = %6.1f tests per second.", (float)(loops/tz));
           printf("\nout of a total of %d plain/cipher pairs.\n", NO_OF_TESTS);
           return;
           }
          }
        }
       }        //end for i
     }               //e
    }                //d
   }                 //c
  }                  //b
 }                   //a
};
//  =============

/* RC4 as implemented from a posting from
 * Newsgroups: sci.crypt
 * From: sterndark@netcom.com (David Sterndark)
 * Subject: RC4 Algorithm revealed.
 * Message-ID: <sternCvKL4B.Hyy@netcom.com>
 */

void RC4_set_key(key, len, data)
RC4_KEY *key;
int len;
register unsigned char *data;
	{
        register RC4_INT tmp;
        register unsigned int id1,id2;
        register RC4_INT *d;
        int i;

        d=&(key->data[0]);
	for (i=0; i<256; i++)
		d[i]=i;

        key->x = 0;
        key->y = 0;
        id1=id2=0;
        for (i=0; i < 256; i++)
		{
		id2 = (data[id1] + d[i] + id2) & 0xff;
		tmp=d[i]; d[i]=d[id2]; d[id2]=tmp;
		id1=(id1+1)%len;
		}
	};
//  =============
void RC4(key, len, indata, outdata)
RC4_KEY *key;
unsigned int len;
unsigned char *indata;
unsigned char *outdata;
	{
        register RC4_INT *d,tmp;
        register unsigned int x,y;
	unsigned int i;

        x=key->x;
        y=key->y;

        d=key->data;

        for (i=0; i<len; i++)
		{
		x=((x+1)&0xff);
		tmp=d[x];
		y=(tmp+y)&0xff;
		d[x]=d[y];
		d[y]=tmp;
		outdata[i]=indata[i]^d[(d[x]+tmp)&0xff];
		}
	key->x=x;
	key->y=y;
	}
//  =============
int RC4_test(key, len, indata, outdata)
RC4_KEY *key;
unsigned int len;
unsigned char *indata;
unsigned char *outdata;
	{
        register RC4_INT *d,tmp;
        register unsigned int x,y;
	unsigned int i;
    char temp;

        x=key->x;
        y=key->y;
        d=key->data;
        for (i=0; i<len; i++)
		{
		x=((x+1)&0xff);
		tmp=d[x];
		y=(tmp+y)&0xff;
		d[x]=d[y];
		d[y]=tmp;
//		outdata[i]=;
        temp = indata[i]^d[(d[x]+tmp)&0xff];
        if (temp != outdata[i])            //small speed up improvement
         return(0);                        //first byte not right, next key
		}
	key->x=x;
	key->y=y;
    return(1);
	};
//  =============
void RC4_set_key_test(key, len, data)
RC4_KEY *key;
int len;
register unsigned char *data;
	{
        register RC4_INT tmp;
        register unsigned int id1,id2;
        register RC4_INT *d;
        int i;
        d=&(key->data[0]);
        memcpy(d, &key_buff, sizeof(key_buff));  //small speed improvement

        key->x = 0;
        key->y = 0;
        id1=id2=0;
        for (i=0; i < 256; i++)
		{
		id2 = (data[id1] + d[i] + id2) & 0xff;
		tmp=d[i]; d[i]=d[id2]; d[id2]=tmp;
		id1=(id1+1)%len;
		}
	};

//  ============= 
