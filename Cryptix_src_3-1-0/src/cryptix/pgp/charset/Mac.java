// This file is currently unlocked (change this line if you lock the file)
//
// $Log: Mac.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
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
public class Mac extends SimpleCharSet
{
    private Mac() {} // static methods only

    public final static byte[] encode(byte[] buffer)
    {
        return SimpleCharSet.translate(buffer, toExt);
    }

    public final static byte[] decode(byte[] buffer)
    {
        return SimpleCharSet.translate(buffer, toInt);
    }

/*
 * The following tables are derived from tables which originally appeared
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
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) 202, (byte) 193, (byte) 162, (byte) 163,
        (byte) 217, (byte) 180, (byte) 124, (byte) 164,
        (byte) 172, (byte) 169, (byte) 187, (byte) 199,
        (byte) 194, (byte) 209, (byte) 168, (byte)  95,
        (byte) 188, (byte) 177, (byte)  50, (byte)  51,
        (byte) 171, (byte) 181, (byte) 166, (byte) 165,
        (byte)  44, (byte)  49, (byte) 161, (byte) 200,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) 192,
        (byte) 203, (byte) 135, (byte) 129, (byte) 204,
        (byte) 128, (byte) 129, (byte) 174, (byte) 130,
        (byte) 143, (byte) 131, (byte) 144, (byte) 145,
        (byte) 147, (byte) 146, (byte) 148, (byte) 149,
        (byte)  68, (byte) 132, (byte) 152, (byte) 151,
        (byte) 153, (byte) 205, (byte) 133, (byte) 120,
        (byte) 175, (byte) 157, (byte) 156, (byte) 158,
        (byte) 134, (byte)  89, (byte)  84, (byte) 167,
        (byte) 136, (byte) 135, (byte) 137, (byte) 139,
        (byte) 138, (byte) 140, (byte)  97, (byte) 141,
        (byte) 143, (byte) 142, (byte) 144, (byte) 145,
        (byte) 147, (byte) 146, (byte) 148, (byte) 149,
        (byte) 100, (byte) 150, (byte) 152, (byte) 151,
        (byte) 153, (byte) 155, (byte) 154, (byte) 214,
        (byte) 191, (byte) 157, (byte) 156, (byte) 158,
        (byte) 159, (byte) 121, (byte) 116, (byte) 216
    };

    private final static byte toInt[] =
    {
        (byte) 196, (byte) 197, (byte) 199, (byte) 201,
        (byte) 209, (byte) 214, (byte) 220, (byte) 225,
        (byte) 224, (byte) 226, (byte) 228, (byte) 227,
        (byte) 229, (byte) 231, (byte) 233, (byte) 232,
        (byte) 234, (byte) 235, (byte) 237, (byte) 236,
        (byte) 238, (byte) 239, (byte) 241, (byte) 243,
        (byte) 242, (byte) 244, (byte) 246, (byte) 245,
        (byte) 250, (byte) 249, (byte) 251, (byte) 252,
        (byte) UNK, (byte) 186, (byte) 162, (byte) 163,
        (byte) 167, (byte) 183, (byte) 182, (byte) 223,
        (byte) 174, (byte) 169, (byte) UNK, (byte) 180,
        (byte) 168, (byte) UNK, (byte) 198, (byte) 216,
        (byte) UNK, (byte) 177, (byte) UNK, (byte) UNK,
        (byte) 165, (byte) 181, (byte) 100, (byte)  83,
        (byte)  80, (byte) 112, (byte)  83, (byte) 170,
        (byte) 176, (byte)  79, (byte) 230, (byte) 248,
        (byte) 191, (byte) 161, (byte) 172, (byte) UNK,
        (byte) 102, (byte) 126, (byte)  68, (byte) 171,
        (byte) 187, (byte) UNK, (byte) 160, (byte) 192,
        (byte) 195, (byte) 213, (byte)  79, (byte) 111,
        (byte)  45, (byte) 173, (byte)  34, (byte)  34,
        (byte)  96, (byte)  39, (byte) 247, (byte) UNK,
        (byte) 255, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK
    };
}
