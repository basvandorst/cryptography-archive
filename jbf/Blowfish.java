/****************************************************************************
* Copyright (c) 1997 Andrew Tuman.<andrew@parexnet.lv>
* All rights reserved.                                 
*  
* Free for commercial and non-commercial use.
*
* Pure Java implementation of Blowfish cipher.
*
* public class Blowfish   - pure java replacement of Blowfish in Criptix
*
* pulic  class BF_cipher  - pure java implementation of Blowfish cipher
*
*****************************************************************************/

package cryptix.security;

import BFcipher;

public final class Blowfish extends BlockCipher {

       BFcipher bf = null;

       public int
       blockLength() 
       { 
        return 8; 
       }

       public Blowfish( byte userKey[] ) 
       {
         byte[] iv = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 };
                bf = new BFcipher( userKey , iv );
       }

       protected void 
       blockEncrypt( byte in[], int in_offset, byte out[], int out_offset )
       {
          int[] data = new int[2];
          data[0] = ((int)in[in_offset++] << 24) & 0xff000000;
          data[0]|= ((int)in[in_offset++] << 16) & 0x00ff0000;
          data[0]|= ((int)in[in_offset++] <<  8) & 0x0000ff00;
          data[0]|= ((int)in[in_offset++])       & 0x000000ff;
          data[1] = ((int)in[in_offset++] << 24) & 0xff000000;
          data[1]|= ((int)in[in_offset++] << 16) & 0x00ff0000;
          data[1]|= ((int)in[in_offset++] <<  8) & 0x0000ff00;
          data[1]|= ((int)in[in_offset++])       & 0x000000ff;
          bf.BF_encrypt( data , 1 );
          out[ out_offset++ ] = (byte)((data[0] >>> 24) & 0xff);
          out[ out_offset++ ] = (byte)((data[0] >>> 16) & 0xff);
          out[ out_offset++ ] = (byte)((data[0] >>>  8) & 0xff);
          out[ out_offset++ ] = (byte)((data[0]       ) & 0xff);
          out[ out_offset++ ] = (byte)((data[1] >>> 24) & 0xff);
          out[ out_offset++ ] = (byte)((data[1] >>> 16) & 0xff);
          out[ out_offset++ ] = (byte)((data[1] >>>  8) & 0xff);
          out[ out_offset++ ] = (byte)((data[1]       ) & 0xff);
          return ;
       }

        protected void
        blockDecrypt( byte in[], int in_offset, byte out[], int out_offset )
        {
          int[] data = new int[2];
          data[0] = ((int)in[in_offset++] << 24) & 0xff000000;
          data[0]|= ((int)in[in_offset++] << 16) & 0x00ff0000;
          data[0]|= ((int)in[in_offset++] <<  8) & 0x0000ff00;
          data[0]|= ((int)in[in_offset++])       & 0x000000ff;
          data[1] = ((int)in[in_offset++] << 24) & 0xff000000;
          data[1]|= ((int)in[in_offset++] << 16) & 0x00ff0000;
          data[1]|= ((int)in[in_offset++] <<  8) & 0x0000ff00;
          data[1]|= ((int)in[in_offset++])       & 0x000000ff;
          bf.BF_encrypt( data , 0 );
          out[ out_offset++ ] = (byte)((data[0] >>> 24) & 0xff);
          out[ out_offset++ ] = (byte)((data[0] >>> 16) & 0xff);
          out[ out_offset++ ] = (byte)((data[0] >>>  8) & 0xff);
          out[ out_offset++ ] = (byte)((data[0]       ) & 0xff);
          out[ out_offset++ ] = (byte)((data[1] >>> 24) & 0xff);
          out[ out_offset++ ] = (byte)((data[1] >>> 16) & 0xff);
          out[ out_offset++ ] = (byte)((data[1] >>>  8) & 0xff);
          out[ out_offset++ ] = (byte)((data[1]       ) & 0xff);
          return ;
        }

        public int
        keyLength()
        {
          return bf.Fkey.length;
        }

};
