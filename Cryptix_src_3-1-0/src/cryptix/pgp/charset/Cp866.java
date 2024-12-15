// This file is currently unlocked (change this line if you lock the file)
//
// $Log: Cp866.java,v $
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
public class Cp866 extends SimpleCharSet
{
    private Cp866() {} // static methods only

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
        (byte) 0xc4, (byte) 0xb3, (byte) 0xda, (byte) 0xbf,
        (byte) 0xc0, (byte) 0xd9, (byte) 0xc3, (byte) 0xb4,
        (byte) 0xc2, (byte) 0xc1, (byte) 0xc5, (byte) 0xdf,
        (byte) 0xdc, (byte) 0xdb, (byte) 0xdd, (byte) 0xde,
        (byte) 0xb0, (byte) 0xb1, (byte) 0xb2, (byte) 0xf4,
        (byte) 0xfe, (byte) 0xf9, (byte) 0xfb, (byte) 0xf7,
        (byte) 0xf3, (byte) 0xf2, (byte) 0xff, (byte) 0xf5,
        (byte) 0xf8, (byte) 0xfd, (byte) 0xfa, (byte) 0xf6,
        (byte) 0xcd, (byte) 0xba, (byte) 0xd5, (byte) 0xf1,
        (byte) 0xd6, (byte) 0xc9, (byte) 0xb8, (byte) 0xb7,
        (byte) 0xbb, (byte) 0xd4, (byte) 0xd3, (byte) 0xc8,
        (byte) 0xbe, (byte) 0xbd, (byte) 0xbc, (byte) 0xc6,
        (byte) 0xc7, (byte) 0xcc, (byte) 0xb5, (byte) 0xf0,
        (byte) 0xb6, (byte) 0xb9, (byte) 0xd1, (byte) 0xd2,
        (byte) 0xcb, (byte) 0xcf, (byte) 0xd0, (byte) 0xca,
        (byte) 0xd8, (byte) 0xd7, (byte) 0xce, (byte) 0xfc,
        (byte) 0xee, (byte) 0xa0, (byte) 0xa1, (byte) 0xe6,
        (byte) 0xa4, (byte) 0xa5, (byte) 0xe4, (byte) 0xa3,
        (byte) 0xe5, (byte) 0xa8, (byte) 0xa9, (byte) 0xaa,
        (byte) 0xab, (byte) 0xac, (byte) 0xad, (byte) 0xae,
        (byte) 0xaf, (byte) 0xef, (byte) 0xe0, (byte) 0xe1,
        (byte) 0xe2, (byte) 0xe3, (byte) 0xa6, (byte) 0xa2,
        (byte) 0xec, (byte) 0xeb, (byte) 0xa7, (byte) 0xe8,
        (byte) 0xed, (byte) 0xe9, (byte) 0xe7, (byte) 0xea,
        (byte) 0x9e, (byte) 0x80, (byte) 0x81, (byte) 0x96,
        (byte) 0x84, (byte) 0x85, (byte) 0x94, (byte) 0x83,
        (byte) 0x95, (byte) 0x88, (byte) 0x89, (byte) 0x8a,
        (byte) 0x8b, (byte) 0x8c, (byte) 0x8d, (byte) 0x8e,
        (byte) 0x8f, (byte) 0x9f, (byte) 0x90, (byte) 0x91,
        (byte) 0x92, (byte) 0x93, (byte) 0x86, (byte) 0x82,
        (byte) 0x9c, (byte) 0x9b, (byte) 0x87, (byte) 0x98,
        (byte) 0x9d, (byte) 0x99, (byte) 0x97, (byte) 0x9a
    };

    private final static byte toInt[] =
    {
        (byte) 0xe1, (byte) 0xe2, (byte) 0xf7, (byte) 0xe7,
        (byte) 0xe4, (byte) 0xe5, (byte) 0xf6, (byte) 0xfa,
        (byte) 0xe9, (byte) 0xea, (byte) 0xeb, (byte) 0xec,
        (byte) 0xed, (byte) 0xee, (byte) 0xef, (byte) 0xf0,
        (byte) 0xf2, (byte) 0xf3, (byte) 0xf4, (byte) 0xf5,
        (byte) 0xe6, (byte) 0xe8, (byte) 0xe3, (byte) 0xfe,
        (byte) 0xfb, (byte) 0xfd, (byte) 0xff, (byte) 0xf9,
        (byte) 0xf8, (byte) 0xfc, (byte) 0xe0, (byte) 0xf1,
        (byte) 0xc1, (byte) 0xc2, (byte) 0xd7, (byte) 0xc7,
        (byte) 0xc4, (byte) 0xc5, (byte) 0xd6, (byte) 0xda,
        (byte) 0xc9, (byte) 0xca, (byte) 0xcb, (byte) 0xcc,
        (byte) 0xcd, (byte) 0xce, (byte) 0xcf, (byte) 0xd0,
        (byte) 0x90, (byte) 0x91, (byte) 0x92, (byte) 0x81,
        (byte) 0x87, (byte) 0xb2, (byte) 0xb4, (byte) 0xa7,
        (byte) 0xa6, (byte) 0xb5, (byte) 0xa1, (byte) 0xa8,
        (byte) 0xae, (byte) 0xad, (byte) 0xac, (byte) 0x83,
        (byte) 0x84, (byte) 0x89, (byte) 0x88, (byte) 0x86,
        (byte) 0x80, (byte) 0x8a, (byte) 0xaf, (byte) 0xb0,
        (byte) 0xab, (byte) 0xa5, (byte) 0xbb, (byte) 0xb8,
        (byte) 0xb1, (byte) 0xa0, (byte) 0xbe, (byte) 0xb9,
        (byte) 0xba, (byte) 0xb6, (byte) 0xb7, (byte) 0xaa,
        (byte) 0xa9, (byte) 0xa2, (byte) 0xa4, (byte) 0xbd,
        (byte) 0xbc, (byte) 0x85, (byte) 0x82, (byte) 0x8d,
        (byte) 0x8c, (byte) 0x8e, (byte) 0x8f, (byte) 0x8b,
        (byte) 0xd2, (byte) 0xd3, (byte) 0xd4, (byte) 0xd5,
        (byte) 0xc6, (byte) 0xc8, (byte) 0xc3, (byte) 0xde,
        (byte) 0xdb, (byte) 0xdd, (byte) 0xdf, (byte) 0xd9,
        (byte) 0xd8, (byte) 0xdc, (byte) 0xc0, (byte) 0xd1,
        (byte) 0xb3, (byte) 0xa3, (byte) 0x99, (byte) 0x98,
        (byte) 0x93, (byte) 0x9b, (byte) 0x9f, (byte) 0x97,
        (byte) 0x9c, (byte) 0x95, (byte) 0x9e, (byte) 0x96,
        (byte) 0xbf, (byte) 0x9d, (byte) 0x94, (byte) 0x9a
    };
}
