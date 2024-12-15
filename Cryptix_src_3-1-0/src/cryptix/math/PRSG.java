// This file is currently unlocked (change this line if you lock the file)
//
// $Log: PRSG.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/06/22  David Hopwood
//   Cosmetic changes.
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
//   Moved this file here from old namespace.
//
// Revision 0.2.5.0  1997/02/24  Original Author not stated
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
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.math;

/**
 * A pseudo-random sequence generator based on a 160-bit LFSR (this is <em>not</em>
 * a cryptographically secure generator).
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author  unattributed
 * @since   Cryptix 2.2
 */
public final class PRSG
{
    private final static int mask0 = 0x80000001;
    private final static int mask1 = 0;
    private final static int mask2 = 0;
    private final static int mask3 = 0;
    private final static int mask4 = 0x80000000;

    int i0, i1, i2, i3, i4;

// Is a default a wise idea???
//    /**
//     * Creates a pseudo random number generator from a default seed.
//     */
//    public PRSG()
//    {
//        i0 = i1 = i2 = i3 = 0;
//        i4 = 0x1000000;
//    }

    /**
     * Creates a pseudo random number generator from the seed <i>b</i>.
     *
     * @param b the seed for this generator
     * @exception MathError if the seed is less than 20 bytes.
     */
    public PRSG(byte[] b)
    {
        if (b.length < 20)
            throw new IllegalArgumentException("The seed byte array must be at least twenty bytes.");

        i4 = b[ 3] | (b[ 2]<<8) | (b[ 1]<<16) | (b[ 0]<<24);
        i3 = b[ 7] | (b[ 6]<<8) | (b[ 5]<<16) | (b[ 4]<<24);
        i2 = b[11] | (b[10]<<8) | (b[ 9]<<16) | (b[ 8]<<24);
        i1 = b[15] | (b[14]<<8) | (b[13]<<16) | (b[12]<<24);
        i0 = b[19] | (b[18]<<8) | (b[17]<<16) | (b[16]<<24);
        if ((i0 | i1 | i2 | i3 | i4) == 0)
            i4 = 0x1000000; // must not start with all bits zero.
    }

    /**
     * Returns the current state of the generator.
     *
     * @return a 20-byte array representing the current state of the generator.
     */
    // Normally the internal state is kept hidden - why should it be accessible
    // directly? --DJH
    public synchronized byte[] toByteArray()
    {
        byte[] b = new byte[20];
        b[ 3]=(byte)i4; b[ 2]=(byte)(i4>>>8); b[ 1]=(byte)(i4>>>16); b[ 0]=(byte)(i4>>>24);
        b[ 7]=(byte)i3; b[ 6]=(byte)(i3>>>8); b[ 5]=(byte)(i3>>>16); b[ 4]=(byte)(i3>>>24);
        b[11]=(byte)i2; b[10]=(byte)(i2>>>8); b[ 9]=(byte)(i2>>>16); b[ 8]=(byte)(i2>>>24);
        b[15]=(byte)i1; b[14]=(byte)(i1>>>8); b[13]=(byte)(i1>>>16); b[12]=(byte)(i1>>>24);
        b[19]=(byte)i0; b[18]=(byte)(i0>>>8); b[17]=(byte)(i0>>>16); b[16]=(byte)(i0>>>24);
        return b;
    }
    
    /**
     * Moves the internal state on one cycle.
     */
    public synchronized void clock()
    {
        if ((i0 & 0x00000001) == 0)
        {
            i0 = ((i0 >>> 1) | (((i1 & 0x00000001)==0) ? 0 : 0x80000000));
            i1 = ((i1 >>> 1) | (((i2 & 0x00000001)==0) ? 0 : 0x80000000));
            i2 = ((i2 >>> 1) | (((i3 & 0x00000001)==0) ? 0 : 0x80000000));
            i3 = ((i3 >>> 1) | (((i4 & 0x00000001)==0) ? 0 : 0x80000000));
            i4 >>>= 1;
        }
        else
        {
            i0 ^= mask0;
            i1 ^= mask1;
            i2 ^= mask2;
            i3 ^= mask3;
            i4 ^= mask4;
            
            i0 = ((i0 >>> 1) | (((i1 & 0x00000001)==0) ? 0 : 0x80000000));
            i1 = ((i1 >>> 1) | (((i2 & 0x00000001)==0) ? 0 : 0x80000000));
            i2 = ((i2 >>> 1) | (((i3 & 0x00000001)==0) ? 0 : 0x80000000));
            i3 = ((i3 >>> 1) | (((i4 & 0x00000001)==0) ? 0 : 0x80000000));
            i4 = ((i4 >>> 1) | 0x80000000);
        }
    }
}
