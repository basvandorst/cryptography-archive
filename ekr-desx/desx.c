/* desx.c -- a simple desx implementation
   Copyright (C) 1994 ekr@rtfm.com

   This software may be used, redistributed, copied, or sold
   for any purpose and without fee provided that this notice
   is maintained.
*/

#include "desx.h"

UCHAR DESXSubstTable[] ={
  189, 86,234,242,162,241,172, 42,176,147,209,156, 27, 51,253,208,
  48,  4,182,220,125,223, 50, 75,247,203, 69,155, 49,187, 33, 90,
  65,159,225,217, 74, 77,158,218,160,104, 44,195, 39, 95,128, 54,
  62,238,251,149, 26,254,206,168, 52,169, 19,240,166, 63,216, 12,
  120, 36,175, 35, 82,193,103, 23,245,102,144,231,232,  7,184, 96,
  72,230, 30, 83,243,146,164,114,140,  8, 21,110,134,  0,132,250,
  244,127,138, 66, 25,246,219,205, 20,141, 80, 18,186, 60,  6, 78,
  236,179, 53, 17,161,136,142, 43,148,153,183,113,116,211,228,191,
  58,222,150, 14,188, 10,237,119,252, 55,107,  3,121,137, 98,198,
  215,192,210,124,106,139, 34,163, 91,  5, 93,  2,117,213, 97,227,
  24,143, 85, 81,173, 31, 11, 94,133,229,194, 87, 99,202, 61,108,
  180,197,204,112,178,145, 89, 13, 71, 32,200, 79, 88,224,  1,226,
  22, 56,196,111, 59, 15,101, 70,190,126, 45,123,130,249, 64,181,
  29,115,248,235, 38,199,135,151, 37, 84,177, 40,170,152,157,165,
  100,109,122,212, 16,129, 68,239, 73,214,174, 46,221,118, 92, 47,
  167, 28,201,  9,105,154,131,207, 41, 57,185,233, 76,255, 67,171
  };

#define BLK8XORINPLACE(a,b) ((UINT4 *)a)[0]^=((UINT4 *)b)[0]; \
                            ((UINT4 *)a)[1]^=((UINT4 *)b)[1];

desx_setkey(context,key,white)
  desx_context *context;
  UCHAR *key,*white;
  {
    desinit(0);
    memcpy((UCHAR *)context->prewhite,(UCHAR *)white,8);
   mkpostwhite(key,(UCHAR *)(context->prewhite),(UCHAR *)(context->postwhite));
   setpar(key);
    setkey(context->ks,(char *)key); /*DES setup*/
  }

desx_done()
  {
    desdone();
  }

desx_encblock(context,blk)
  desx_context *context;
  UCHAR *blk;
  {
    BLK8XORINPLACE(blk,context->prewhite);
    endes(context->ks,blk);
    BLK8XORINPLACE(blk,context->postwhite);
  }

desx_decblock(context,blk)
  desx_context *context;
  char *blk;
  {
    BLK8XORINPLACE(blk,context->postwhite);
    dedes(context->ks,blk);
    BLK8XORINPLACE(blk,context->prewhite);
  }

/* mkpostwhite -- compute the postwhitener */
static int mkpostwhite(key,pre,post)
  UCHAR *key,*pre,*post;
  {
    int i;
    UCHAR lastbyte;
    
    memset(post,0,8);
    for(i=0;i<8;i++){
      lastbyte=DESXSubstTable[post[0]^post[1]];
      shl(post);
      post[7]=lastbyte^key[i];
    }
    for(i=0;i<8;i++){
      lastbyte=DESXSubstTable[post[0]^post[1]];
      shl(post);
      post[7]=lastbyte^pre[i];
    }
    return(0);
  }

static shl(x)
  UCHAR *x;
  {
    int i;
    
    for(i=0;i<7;i++)
      x[i]=x[i+1];
      
  }
    
static int setpar(x)
  UCHAR *x;
  {
    int i,j,k;

    for(i=0;i<8;i++){
      k=0;
      for(j=0;j<7;j++)
        k^=!!(x[i]&(1<<j));
      if(k)
        x[i]&=~0x80;
      else
        x[i]|=0x80;
    }
  }

