// This file is currently unlocked (change this line if you lock the file)
//
// $Log: MessageHash.java,v $
// Revision 1.3  1998/01/11 20:22:03  iang
// Hash???: added exit(1) on error, added test to SHA0.
// ???: added exit(1).
// MessageHash: needs to return digest name as SHA/SHA0/MD5, not the
// names listed as aliases in Cj3.0.   Added a hack to check and change.
//
// Revision 1.2  1997/11/21 05:30:19  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.3.0.0  1997/08/14  David Hopwood
// + Changed fromDigest methods to take cryptix.security.MessageDigest
//   objects as a parameter, rather than java.security.MessageDigest.
//
// Revision 0.2.5.2  1997/06/29  David Hopwood
// + Made this class non-abstract, since message hashes can be
//   implemented generically without any need for subclasses HashMD5,
//   HashSHA, etc. Added various methods to support this.
// + Changed to use cryptix.util.core.Hex and ArrayUtil.
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
// + Moved this file here from old namespace.
//
// Revision 0.2.5.0  1997/02/24  Original Author not stated
// + Original version.
//
// $Endlog$
/*
 * Copyright (c) 1995-1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security;

import cryptix.util.core.Hex;
import cryptix.util.core.ArrayUtil;

/**
 * This class represents the output from a message digest, in a form
 * where the type can be ascertained.
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.3 $</b>
 * @author  Systemics Ltd
 * @author  David Hopwood
 * @since   Cryptix 2.2 (was abstract before 2.2.2)
 */
public class MessageHash
{
    private String algorithm;
    private byte[] hash;

    /**
     * Constructs a MessageHash for the specified algorithm name. The hash value
     * is a copy of the byte array <i>hash</i>.
     *
     * @param algorithm the name of the MessageDigest algorithm.
     * @param hash      a byte array containing the hash.
     * @exception NullPointerException if algorithm == null || hash == null
     */
    public MessageHash(String algorithm, byte[] hash) {
        if (algorithm == null) throw new NullPointerException("algorithm == null");
        this.hash = new byte[hash.length];
	if (algorithm.equals("SHA-0"))        /* old names were different */
	    this.algorithm = "SHA0";          /* is this the right place? */
	else if (algorithm.equals("SHA-1"))
	    this.algorithm = "SHA";
	else
	    this.algorithm = algorithm;
        System.arraycopy(hash, 0, this.hash, 0, hash.length);
    }

    /**
     * Creates a MessageHash with the value found by calling the
     * <a href="MessageDigest.html#digest"><code>digest()</code></a>
     * method on <i>md</i>. This causes <i>md</i> to be reset. It is equivalent to:
     * <pre>
     *    new MessageHash(md.name(), md.digest())
     * </pre>
     */
    public static MessageHash fromDigest(MessageDigest md) {
        return new MessageHash(md.name(), md.digest());
    }

    /**
     * Creates a MessageHash with the value found by calling the
     * <a href="MessageDigest.html#digest(byte[])"><code>digest(byte[])</code></a>
     * method on <i>md</i>. This causes <i>md</i> to be reset. It is equivalent to:
     * <pre>
     *    new MessageHash(md.name(), md.digest(data))
     * </pre>
     */
    public static MessageHash fromDigest(MessageDigest md, byte[] data) {
        return new MessageHash(md.name(), md.digest(data));
    }

    /**
     * Returns the name of the algorithm associated with this hash.
     */
    public String getAlgorithm() {
        return algorithm;
    }

    /**
     * Returns the hash as a new byte array.
     */
    public final byte[] toByteArray() {
        byte buf[] = new byte[hash.length];
        System.arraycopy(hash, 0, buf, 0, hash.length);
        return buf;
    }
    
    /**
     * Returns the hash length in bytes.
     */
    public final int length() {
        return hash.length;
    }
    
    public int hashCode() {
        switch(hash.length) {
          case 0:
            return 0;
          case 1:
            return hash[0];
          case 2:
            return hash[0] ^ (hash[1] << 8);
          case 3:
            return hash[0] ^ (hash[1] << 8) ^ (hash[2] << 16);
          default:
            return hash[0] ^ (hash[1] << 8) ^ (hash[2] << 16) ^ (hash[3] << 24);
        }
    }

    /**
     * Tests whether this object is equal to another object.
     * <p>
     * Two MessageHash objects are equal iff they are for the same algorithm, and
     * have the same value as a sequence of bytes.
     *
     * @return true iff the objects are equal
     */
    public boolean equals(Object obj) {
        if (!(obj instanceof MessageHash))
            return false;

        MessageHash otherHash = (MessageHash) obj;
        return getAlgorithm().equals(otherHash.getAlgorithm()) &&
               ArrayUtil.areEqual(hash, otherHash.hash);
    }

    /**
     * Returns a big endian Hex string showing the value of the hash,
     * prefixed by the standard algorithm name and a colon.
     *
     * @return a string representing the hash.
     */
    public String toString() {
        return getAlgorithm() + ":" + Hex.toString(hash);
    }
}
