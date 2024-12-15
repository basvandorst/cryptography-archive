package cryptix.pgp.charset;

import cryptix.pgp.*;

/**
 * @author Jill Baker
 */
public abstract class Cp860 extends SimpleCharSet
{
    public final static byte[] encode( byte[] buffer )
    {
        return SimpleCharSet.translate( buffer, toExt );
    }

    public final static byte[] decode( byte[] buffer )
    {
        return SimpleCharSet.translate( buffer, toInt );
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
        (byte) 186, (byte) 205, (byte) 201, (byte) 187,
        (byte) 200, (byte) 188, (byte) 204, (byte) 185,
        (byte) 203, (byte) 202, (byte) 206, (byte) 223,
        (byte) 220, (byte) 219, (byte) 254, (byte)  95,
        (byte) 179, (byte) 196, (byte) 218, (byte) 191,
        (byte) 192, (byte) 217, (byte) 195, (byte) 180,
        (byte) 194, (byte) 193, (byte) 197, (byte) 176,
        (byte) 177, (byte) 178, (byte) UNK, (byte) UNK,
        (byte) 255, (byte) 173, (byte) 155, (byte) 156,
        (byte)  36, (byte)  89, (byte) 124, (byte)  80,
        (byte)  34, (byte)  67, (byte) 166, (byte) 174,
        (byte) 170, (byte)  45, (byte)  82, (byte)  95,
        (byte) 248, (byte) 241, (byte) 253, (byte)  51,
        (byte)  39, (byte) 230, (byte)  45, (byte) 250,
        (byte)  44, (byte)  49, (byte) 167, (byte) 175,
        (byte) 172, (byte) 171, (byte) UNK, (byte) 168,
        (byte) 145, (byte) 134, (byte) 143, (byte) 142,
        (byte)  65, (byte)  65, (byte)  65, (byte) 128,
        (byte) 146, (byte) 144, (byte) 137, (byte)  69,
        (byte) 152, (byte) 139, (byte)  73, (byte)  73,
        (byte)  68, (byte) 165, (byte) 169, (byte) 159,
        (byte) 140, (byte) 153, (byte)  79, (byte) 120,
        (byte)  79, (byte) 157, (byte) 150, (byte)  85,
        (byte) 154, (byte)  89, (byte)  84, (byte) 225,
        (byte) 133, (byte) 160, (byte) 131, (byte) 132,
        (byte)  97, (byte)  97, (byte)  97, (byte) 135,
        (byte) 138, (byte) 130, (byte) 136, (byte) 101,
        (byte) 141, (byte) 161, (byte) 105, (byte) 105,
        (byte) 100, (byte) 164, (byte) 149, (byte) 162,
        (byte) 147, (byte) 148, (byte) 111, (byte) 246,
        (byte) 111, (byte) 151, (byte) 163, (byte) 117,
        (byte) 129, (byte) 121, (byte) 116, (byte) 121
    };

    private final static byte toInt[] =
    {
        (byte) 199, (byte) 252, (byte) 233, (byte) 226,
        (byte) 227, (byte) 224, (byte) 193, (byte) 231,
        (byte) 234, (byte) 202, (byte) 232, (byte) 205,
        (byte) 212, (byte) 236, (byte) 195, (byte) 194,
        (byte) 201, (byte) 192, (byte) 200, (byte) 244,
        (byte) 245, (byte) 242, (byte) 218, (byte) 249,
        (byte) 204, (byte) 213, (byte) 220, (byte) 162,
        (byte) 163, (byte) 217, (byte) 164, (byte) 211,
        (byte) 225, (byte) 237, (byte) 243, (byte) 250,
        (byte) 241, (byte) 209, (byte) 170, (byte) 186,
        (byte) 191, (byte) 210, (byte) 172, (byte) 189,
        (byte) 188, (byte) 161, (byte) 171, (byte) 187,
        (byte) 155, (byte) 156, (byte) 157, (byte) 144,
        (byte) 151, (byte) 151, (byte) 135, (byte) 147,
        (byte) 131, (byte) 135, (byte) 128, (byte) 131,
        (byte) 133, (byte) 149, (byte) 133, (byte) 147,
        (byte) 148, (byte) 153, (byte) 152, (byte) 150,
        (byte) 145, (byte) 154, (byte) 150, (byte) 134,
        (byte) 132, (byte) 130, (byte) 137, (byte) 136,
        (byte) 134, (byte) 129, (byte) 138, (byte) 137,
        (byte) 153, (byte) 136, (byte) 152, (byte) 148,
        (byte) 132, (byte) 130, (byte) 146, (byte) 154,
        (byte) 138, (byte) 149, (byte) 146, (byte) 141,
        (byte) 140, (byte) 141, (byte) 141, (byte) 139,
        (byte) UNK, (byte) 223, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) 181, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte)  61, (byte) UNK, (byte) UNK, (byte) UNK,
        (byte) UNK, (byte) UNK, (byte) 247, (byte) 126,
        (byte) 176, (byte) 183, (byte) 183, (byte) UNK,
        (byte) 110, (byte) 178, (byte) 142, (byte) 160
    };
}



