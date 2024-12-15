package cryptix.pgp.charset;

import cryptix.pgp.*;

/**
 * @author Jill Baker
 */
public abstract class Next extends SimpleCharSet
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
        (byte)  UNK, (byte)  UNK, (byte)  UNK, (byte)  UNK, 
        (byte)  UNK, (byte)  UNK, (byte)  UNK, (byte)  UNK, 
        (byte)  UNK, (byte)  UNK, (byte)  UNK, (byte)  UNK, 
        (byte)  UNK, (byte)  UNK, (byte)  UNK, (byte)  UNK,
        (byte) 0300, (byte) 0301, (byte) 0302, (byte) 0303, 
        (byte) 0304, (byte) 0305, (byte) 0306, (byte) 0307, 
        (byte) 0310, (byte)  UNK, (byte) 0312, (byte) 0313, 
        (byte)  UNK, (byte) 0272, (byte) 0316, (byte) 0247,
        (byte)  UNK, (byte) 0241, (byte) 0242, (byte) 0243, 
        (byte) 0250, (byte) 0245, (byte) 0265, (byte)  UNK, 
        (byte) 0310, (byte) 0240, (byte) 0343, (byte) 0253, 
        (byte) 0276, (byte) 0261, (byte) 0260, (byte) 0320,
        (byte) 0312, (byte) 0321, (byte) 0311, (byte) 0314, 
        (byte) 0270, (byte) 0235, (byte) 0266, (byte) 0267, 
        (byte) 0313, (byte) 0300, (byte)  UNK, (byte) 0273, 
        (byte) 0322, (byte) 0323, (byte) 0324, (byte) 0277,
        (byte) 0201, (byte) 0202, (byte) 0203, (byte) 0204, 
        (byte) 0205, (byte) 0206, (byte) 0341, (byte) 0207, 
        (byte) 0210, (byte) 0211, (byte) 0212, (byte) 0213, 
        (byte) 0214, (byte) 0215, (byte) 0216, (byte) 0217,
        (byte) 0220, (byte) 0221, (byte) 0222, (byte) 0223, 
        (byte) 0224, (byte) 0225, (byte) 0226, (byte) 0236, 
        (byte) 0351, (byte) 0227, (byte) 0230, (byte) 0231, 
        (byte) 0232, (byte) 0233, (byte) 0234, (byte) 0373,
        (byte) 0325, (byte) 0326, (byte) 0327, (byte) 0330, 
        (byte) 0331, (byte) 0332, (byte) 0361, (byte) 0333, 
        (byte) 0334, (byte) 0335, (byte) 0336, (byte) 0337, 
        (byte) 0340, (byte) 0342, (byte) 0344, (byte) 0345,
        (byte) 0346, (byte) 0347, (byte) 0354, (byte) 0355, 
        (byte) 0356, (byte) 0357, (byte) 0360, (byte) 0237, 
        (byte) 0371, (byte) 0362, (byte) 0363, (byte) 0364, 
        (byte) 0366, (byte) 0367, (byte) 0374, (byte) 0375
    };

    private final static byte toInt[] =
    {
        (byte)  UNK, (byte) 0300, (byte) 0301, (byte) 0302, 
        (byte) 0303, (byte) 0304, (byte) 0305, (byte) 0307, 
        (byte) 0310, (byte) 0311, (byte) 0312, (byte) 0313, 
        (byte) 0314, (byte) 0315, (byte) 0316, (byte) 0317,
        (byte) 0320, (byte) 0321, (byte) 0322, (byte) 0323, 
        (byte) 0324, (byte) 0325, (byte) 0326, (byte) 0331, 
        (byte) 0332, (byte) 0333, (byte) 0334, (byte) 0335, 
        (byte) 0336, (byte) 0265, (byte) 0337, (byte) 0267,
        (byte) 0251, (byte) 0241, (byte) 0242, (byte) 0243, 
        (byte) 0057, (byte) 0245, (byte) 0146, (byte) 0247, 
        (byte) 0244, (byte) 0140, (byte) 0042, (byte) 0253, 
        (byte) 0074, (byte) 0076, (byte)  UNK, (byte)  UNK,
        (byte) 0256, (byte) 0255, (byte)  UNK, (byte)  UNK, 
        (byte) 0056, (byte) 0246, (byte) 0266, (byte) 0267, 
        (byte) 0054, (byte) 0042, (byte) 0235, (byte) 0273, 
        (byte)  UNK, (byte)  UNK, (byte) 0254, (byte) 0277,
        (byte) 0220, (byte) 0221, (byte) 0222, (byte) 0223, 
        (byte) 0224, (byte) 0225, (byte) 0226, (byte) 0232, 
        (byte) 0230, (byte) 0262, (byte) 0227, (byte) 0270, 
        (byte) 0263, (byte) 0042, (byte) 0236, (byte) 0226,
        (byte) 0257, (byte) 0261, (byte) 0274, (byte) 0275, 
        (byte) 0276, (byte) 0340, (byte) 0341, (byte) 0342, 
        (byte) 0343, (byte) 0344, (byte) 0345, (byte) 0347, 
        (byte) 0350, (byte) 0351, (byte) 0352, (byte) 0353,
        (byte) 0354, (byte) 0306, (byte) 0355, (byte) 0252, 
        (byte) 0356, (byte) 0357, (byte) 0365, (byte) 0361, 
        (byte) 0243, (byte) 0330, (byte)  UNK, (byte) 0272, 
        (byte) 0362, (byte) 0363, (byte) 0364, (byte) 0365,
        (byte) 0366, (byte) 0346, (byte) 0371, (byte) 0372, 
        (byte) 0373, (byte) 0151, (byte) 0374, (byte) 0375, 
        (byte) 0154, (byte) 0370, (byte)  UNK, (byte) 0337, 
        (byte) 0376, (byte) 0377, (byte)  UNK, (byte)  UNK
    };
}



