package cryptix.pgp.charset;

import cryptix.pgp.*;

/**
 * @author Jill Baker
 */
public abstract class Ascii extends SimpleCharSet
{
    public final static byte[] encode( byte[] buffer )
    {
        return SimpleCharSet.translate( buffer, toExt );
    }

    public final static byte[] decode( byte[] in )
    {
        int len = in.length;
        byte[] out = new byte[ len ];

        System.arraycopy( in, 0, out, 0, len );
        for (int i=0; i<len; i++)
        {
            if (in[i] < 0) out[i] = UNK;
        }
        return out;
    }

/*
 * The following table is derived from a table which originally appeared
 * in the source file CHARSET.C from the PGP program. Here is the
 * original copyright information from that file....
 *
 * (c) Copyright 1990-1996 by Philip Zimmermann.  All rights reserved.
 * The author assumes no liability for damages resulting from the use
 * of this software, even if the damage results from defects in this
 * software.  No warranty is expressed or implied.
 *
 * Code that has been incorporated into PGP from other sources was
 * either originally published in the public domain or is used with
 * permission from the various authors.
 */

    private final static byte toExt[] =
    {
        UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
        UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
        UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
        UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
         32,  33,  99,  35,  36,  89, 124,  80,
         34,  67,  97,  34, 126,  45,  82,  95,
        111, UNK,  50,  51,  39, 117,  45,  45,
         44,  49, 111,  34, UNK, UNK, UNK,  63,
         65,  65,  65,  65,  65,  65,  65,  67,
         69,  69,  69,  69,  73,  73,  73,  73,
         68,  78,  79,  79,  79,  79,  79, 120,
         79,  85,  85,  85,  85,  89,  84, 115,
         97,  97,  97,  97,  97,  97,  97,  99,
        101, 101, 101, 101, 105, 105, 105, 105,
        100, 110, 111, 111, 111, 111, 111,  47,
        111, 117, 117, 117, 117, 121, 116, 121
    };
}



