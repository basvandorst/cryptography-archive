
#include <stdio.h>
#include "DES/des.h"

/* we have a problem, the des routines want to des on
 * aligned boundaries, we want to des 1 byte into a packet
 * so , we have to do some copies in here
 */

char temp[8];

des_setkey(data) 
char *data;
{
#ifdef DEBUG
  int i;
  for(i=0;i<8;i++) fprintf(stderr,"%x%x ",(data[i]>>4)&0xf,data[i]&0xf);
  fprintf(stderr,"\n");
#endif DEBUG
  memcpy(temp,data,8);  
  keyinit(temp);
}

des(data)
char *data;
{
  memcpy(temp,data,8);
#ifndef NOCRYPT
  endes(temp);  /* */ 
#endif NOCRYPT
  memcpy(data,temp,8);
  return;
}

undes(data)
char *data;
{
  memcpy(temp,data,8);
#ifndef NOCRYPT
  dedes(temp);  /* */
#endif NOCRYPT
  memcpy(data,temp,8);
  return;
}

