// This file is currently unlocked (change this line if you lock the file)
//
// $Log: SimpleCharSet.java,v $
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
public class SimpleCharSet
{
    SimpleCharSet() {} // static methods only

    public final static byte UNK = (byte)'?';

    protected final static byte[] translate(byte[] in, byte[] lookupTable)
    {
        int len = in.length;
        byte[] out = new byte[ len ];

        System.arraycopy(in, 0, out, 0, len);
        for (int i=0; i<len; i++)
        {
            int b = in[i];
            if (b < 0)
            {
                out[i] = lookupTable[ b & 0x7F ];
            }
        }
        return out;
    }
}
