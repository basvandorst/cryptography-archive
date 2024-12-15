// This file is currently unlocked (change this line if you lock the file)
//
// $Log: RawKey.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:56  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0.2  1997/08/21  David Hopwood
// + Added (String algorithm, byte[] data, int offset, int length)
//   constructor.
//
// Revision 0.1.0.1  1997/07/31  David Hopwood
// + Prevent keys with a null algorithm string from being constructed.
//
// Revision 0.1.0.0  1997/07/22  R. Naffah
// + Original version from the old cryptix.provider.key.RawSecretKey
//   class. The rationale is to (a) allow future implementations of
//   raw-encoded key classes to have minimal code by making them extend
//   this, and (b) make supporting features such as serialization
//   easier to implement in the future.
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.provider.key;

import java.security.Key;

/**
 * RawKey implements a key in raw format, represented as a byte array.
 * <p>
 *
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author  David Hopwood
 * @author  Raif S. Naffah
 * @since   Cryptix 2.2.2
 */
public class RawKey
implements Key
{

// Constants and variables
//............................................................................

    private static final String FORMAT = "RAW";
    private String algorithm;
    private byte[] data;


// Constructors
//............................................................................

    /**
     * Constructs a key with the specified algorithm name and raw-encoded
     * data array.
     *
     * @param algorithm the name of the algorithm
     * @param data      the key's raw-encoded data
     * @exception NullPointerException if algorithm == null || data == null
     */
    public RawKey (String algorithm, byte[] data) {
        if (algorithm == null) throw new NullPointerException("algorithm == null");
        this.algorithm = algorithm;
        this.data = (byte[]) (data.clone());
    }

    /**
     * Constructs a secret key with the specified algorithm and raw-encoded
     * data subarray.
     *
     * @param algorithm the name of the algorithm
     * @param data      the key's raw-encoded data
     * @param offset    the offset of the encoding in <i>data</i>
     * @param length    the length of the encoding
     * @exception NullPointerException if algorithm == null || data == null
     */
    public RawKey (String algorithm, byte[] data, int offset, int length) {
        if (algorithm == null) throw new NullPointerException("algorithm == null");
        this.algorithm = algorithm;
        this.data = new byte[length];
        System.arraycopy(data, offset, this.data, 0, length);
    }


// Key interface implementation
//............................................................................

    /**
     * @return the standard algorithm name this key was generated for.
     */
    public String getAlgorithm() { return algorithm; }

    /**
     * @return the format used to encode this key. For this class it's
     *         always <code>RAW</code>.
     */
    public String getFormat() { return FORMAT; }

    /**
     * @return a copy of the raw-encoded key data
     */
    public byte[] getEncoded() { return (byte[]) (data.clone()); }
}
