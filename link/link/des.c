
#include <stdio.h>
#include "DES/d3des.h"
#include "bcopy.h"

/* we have a problem, the des routines want to des on
 * aligned boundaries, we want to des 1 byte into a packet
 * so , we have to do some copies in here
 */

char temp[8];
unsigned long enkey[32],dekey[32];

des_setkey(data) 
char *data;
{
#ifdef DEBUG
  int i;
  for(i=0;i<8;i++) fprintf(stderr,"%x%x ",(data[i]>>4)&0xf,data[i]&0xf);
  fprintf(stderr,"\n");
#endif DEBUG
  bcopy(data,temp,8);
  /*memcpy(temp,data,8);   */
  deskey(temp,0);     /* set encryption key */
  cpkey(enkey);
  deskey(temp,1);     /* set decryption key */
  cpkey(dekey);
}

en_des(data)
char *data;
{
  bcopy(data,temp,8);
  /*memcpy(temp,data,8); */
#ifndef NOCRYPT
  usekey(enkey);    /* use encryption key */
  des(temp,data);   /* des temp into data */
#endif NOCRYPT
  return;
}

un_des(data)
char *data;
{
  bcopy(data,temp,8);
  /*memcpy(temp,data,8); */
#ifndef NOCRYPT
  usekey(dekey);   /* use decryption key */
  des(temp,data);  /* undes temp into data */
#endif NOCRYPT
  return;
}

