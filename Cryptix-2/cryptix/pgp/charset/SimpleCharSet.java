package cryptix.pgp.charset;

import cryptix.pgp.*;

/**
 * @author Jill Baker
 */
public abstract class SimpleCharSet
{
    public final static byte UNK = (byte)'?';

    protected final static byte[] translate( byte[] in, byte[] lookupTable )
    {
        int len = in.length;
        byte[] out = new byte[ len ];

        System.arraycopy( in, 0, out, 0, len );
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



