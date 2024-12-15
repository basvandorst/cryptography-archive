// This file is currently unlocked (change this line if you lock the file)
//
// $Log: PseudoRandomStream.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
//   Moved this file here from old namespace.
//
// Revision 0.2.5.0  1997/02/24  Original Author not stated
//   Original version
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.math;

/**
 * A class that produces pseudo-random bytes.
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author unattributed
 * @since  Cryptix 2.2 or earlier
 */
public class PseudoRandomStream extends RandomStream
{
    protected PRSG prng;
    protected byte[] buf;
    protected int ptr = 0, len = 0;

    /**
     * <i>seed</i> must be 20 bytes or greater (any excess is not used).
     */
    public PseudoRandomStream(byte[] seed)
    {
        prng = new PRSG(seed);
    }

    protected void seed(byte[] seed)
    {
        prng = new PRSG(seed);
        // shouldn't there be "ptr = 0; len = 0;" here?
    }
    
    protected void nextBuffer()
    {
        prng.clock();
        buf = prng.toByteArray();
        len = buf.length;
        ptr = 0;
    }
    
    //
    // It is very important that this method
    // is synchronised, otherwise we're in big
    // trouble when using global RandomStreams
    //
    protected final synchronized byte nextByte()
    {
        if (ptr >= len) // time to get next set of bytes
            nextBuffer();

// Oh, what a surprise! There's a bug in MS Java ...
//        return buf[ptr++];
        byte r = buf[ptr++]; return r;
    }
}
