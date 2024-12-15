// This file is currently unlocked (change this line if you lock the file)
//
// $Log: Keybcs.java,v $
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
public class Keybcs extends SimpleCharSet
{
    private Keybcs() {} // static methods only

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
        (byte) UNK, (byte)  65, (byte) UNK, (byte)  76,
        (byte) UNK, (byte) 156, (byte)  83, (byte) 173,
        (byte) UNK, (byte) 155, (byte)  83, (byte) 134,
        (byte)  90, (byte) UNK, (byte) 146, (byte)  90,
        (byte) 248, (byte) UNK, (byte) UNK, (byte) 108,
        (byte) UNK, (byte) 140, (byte) 115, (byte) UNK,
        (byte) UNK, (byte) 168, (byte) 115, (byte) 159,
        (byte) 122, (byte) UNK, (byte) 145, (byte) 122,
        (byte) 171, (byte) 143, (byte)  65, (byte)  65,
        (byte) 142, (byte) 138, (byte)  67, (byte)  67,
        (byte) 128, (byte) 144, (byte)  69, (byte)  69,
        (byte) 137, (byte) 139, (byte)  73, (byte) 133,
        (byte)  68, (byte)  78, (byte) 165, (byte) 149,
        (byte) 167, (byte)  79, (byte) 153, (byte) UNK,
        (byte) 158, (byte) 166, (byte) 151, (byte)  85,
        (byte) 154, (byte) 157, (byte)  84, (byte) 225,
        (byte) 170, (byte) 160, (byte)  97, (byte)  97,
        (byte) 132, (byte) 141, (byte)  99, (byte)  99,
        (byte) 135, (byte) 130, (byte) 101, (byte) UNK,
        (byte) 136, (byte) 161, (byte) 105, (byte) 131,
        (byte) 100, (byte) 110, (byte) 164, (byte) 162,
        (byte) 147, (byte) 111, (byte) 148, (byte) UNK,
        (byte) 169, (byte) 150, (byte) 163, (byte) 117,
        (byte) 129, (byte) 152, (byte) 116, (byte) UNK
    };

    private final static byte toInt[] =
    {
        (byte) 200, (byte) 252, (byte) 233, (byte) 239,
        (byte) 228, (byte) 207, (byte) 171, (byte) 232,
        (byte) 236, (byte) 204, (byte) 197, (byte) 205,
        (byte) 181, (byte) 229, (byte) 196, (byte) 193,
        (byte) 201, (byte) 190, (byte) 174, (byte) 244,
        (byte) 246, (byte) 211, (byte) 249, (byte) 218,
        (byte) 253, (byte) 214, (byte) 220, (byte) 169,
        (byte) 165, (byte) 221, (byte) 216, (byte) 187,
        (byte) 225, (byte) 237, (byte) 243, (byte) 250,
        (byte) 242, (byte) 210, (byte) 217, (byte) 212,
        (byte) 185, (byte) 248, (byte) 224, (byte) 192,
        (byte) UNK, (byte) 167, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) 223, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) 176, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK
    };
}
