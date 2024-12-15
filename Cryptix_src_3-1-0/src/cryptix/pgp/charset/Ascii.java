// This file is currently unlocked (change this line if you lock the file)
//
// $Log: Ascii.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.4  1997/03/04  Jill Baker
//   Bug fix. Now converts correctly via table.
//
// Revision 0.2.5.3  1997/02/28  Jill Baker
//   Complete rewrite
//
// Revision 0.2.5.2  1997/02/26  Jill Baker
//   Complete rewrite.
//
// Revision 0.2.5.1  1997/02/24  Jill Baker
//   Original version
//
//   If this file is unlocked, or locked by someone other than you, then you
//   may download this file for incorporation into your build, however any
//   changes you make to your copy of this file will be considered volatile,
//   and will not be incorporated into the master build which is maintained
//   at this web site.
//
//   If you wish to make permanent changes to this file, you must wait until
//   it becomes unlocked (if it is not unlocked already), and then lock it
//   yourself. Whenever the file is locked by you, you may upload new master
//   versions to the master site. Make sure you update the history information
//   above. When your changes are complete, you should relinquish the lock to
//   make it available for other developers.
// $Endlog$
package cryptix.pgp.charset;

import cryptix.pgp.*;

/**
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author Jill Baker
 */
public class Ascii extends SimpleCharSet
{
    private Ascii() {} // static methods only

    public final static byte[] encode(byte[] buffer)
    {
        return SimpleCharSet.translate(buffer, toExt);
    }

    public final static byte[] decode(byte[] in)
    {
        int len = in.length;
        byte[] out = new byte[ len ];

        System.arraycopy(in, 0, out, 0, len);
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

