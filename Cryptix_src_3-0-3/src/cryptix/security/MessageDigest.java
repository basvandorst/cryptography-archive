// $Id: MessageDigest.java,v 1.2 1997/12/07 06:40:26 hopwood Exp $
//
// $Log: MessageDigest.java,v $
// Revision 1.2  1997/12/07 06:40:26  hopwood
// + Major overhaul of cryptix.security.*.
//
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.3.0.0  1997/08/14  David Hopwood
// + Instead of having digest implementations extend this class, we now
//   keep a reference to a pure JCA MessageDigest. This removes the
//   dependency of the JCA code on this deprecated class.
// + Removed bitcount, data_length, buf and buf_off methods.
//
// Revision 0.2.5.2  1997/06/29  David Hopwood
// + Added JCA support.
// + Deprecated this class, and added documentation showing the equivalent
//   JCA methods.
// + Moved some of the implementation into BlockMessageDigest.
// + Changed to use cryptix.mime.LegacyString where ISO-Latin-1 was assumed.
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

import cryptix.mime.LegacyString;

/**
 * This is the abstract superclass for message digests that support the
 * Cryptix V2.2 API. New code should use <samp>java.security.MessageDigest</samp>
 * in preference.
 * <p>
 * Note: the following methods are no longer supported (starting from Cryptix
 * 2.2.2), because they either have no JCE equivalent, or were part of the
 * implementation of MessageDigest and should not have been public.
 * <ul>
 *    <li> <code>public long bitcount();</code>
 *    <li> <code>public int data_length();</code>
 *    <li> <code>public byte[] buf();</code>
 *    <li> <code>public int buf_off();</code>
 * </ul>
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.2 $</b>
 * @author  Systemics Ltd
 * @author  David Hopwood
 * @since   Cryptix 2.2
 * @deprecated
 *
 * @see java.security.MessageDigest
 */
public abstract class MessageDigest
{

// Variables
//...........................................................................

    /**
     * The JCA message digest that is delegated to.
     */
    private java.security.MessageDigest md;


// Constructor
//...........................................................................

    /**
     * Constructs a V2.2 message digest from a JCA message digest.
     *
     * @param  md   the java.security.MessageDigest object.
     */
    protected MessageDigest(java.security.MessageDigest md) {
        this.md = md;
    }


// Own methods
//............................................................................

    /**
     * Returns the digest length in bytes.
     * <p>
     * The JCA equivalent to this method,
     * <code>java.security.MessageDigest.getDigestLength</code>, is only supported
     * when the IJCE version of the <samp>java.security.MessageDigest</samp> class
     * is first on the classpath. <code>getDigestLength</code> will also be
     * supported by Java 1.2.
     *
     * @deprecated Use <code><a href="java.security.MessageDigest.html#getDigestLength">getDigestLength</a>()</code> instead.
     */
    public final int hash_length() {
        try {
            java.security.MessageDigest newmd = (java.security.MessageDigest)
                (md.getClass().newInstance());
            return newmd.digest().length;
        } catch (ClassCastException e) { e.printStackTrace();
        } catch (IllegalAccessException e) { e.printStackTrace();
        } catch (InstantiationException e) { e.printStackTrace();
        }
        throw new CryptoError(md.getAlgorithm() + ": hash_length() failed");
    }

    /**
     * Identical to <code>hash_length</code>.
     *
     * @deprecated Use <code><a href="java.security.MessageDigest.html#getDigestLength">getDigestLength</a>()</code> instead.
     */
    public final int length() {
        return hash_length();
    }

    /**
     * Returns the message digest name.
     *
     * @deprecated Use <code><a href="java.security.MessageDigest.html#getAlgorithm">getAlgorithm</a>()</code> instead.
     */
    public final String name() {
        return md.getAlgorithm();
    }

    /**
     * Obtains a MessageHash object for this digest.
     * <p>
     * N.B. this method resets the digest.
     *
     * @return the MessageHash of all the data added.
     * @deprecated Replace <code>md.digestAsHash()</code> with
     *             <code><a href="cryptix.security.MessageHash.html#fromDigest(java.security.MessageDigest)">MessageHash.fromDigest</a>(md)</code>.
     * @see cryptix.security.MessageHash
     */
    public final MessageHash digestAsHash() {
        return MessageHash.fromDigest(this);
    }

    /**
     * Adds the low bytes of a string to the digest. This should not be used
     * when internationalization is a concern, since it does not handle
     * Unicode characters above <code>&#92;u00FF</code> gracefully.
     *
     * @param  message  the string to add.
     * @param  offset   the start of the data string.
     * @param  length   the length of the data string.
     * @exception NullPointerException if message == null
     * @exception StringIndexOutOfBoundsException if offset or length are
     *                  out of range
     * @deprecated Use <code><a href="java.security.MessageDigest.html#update(byte[])">update</a>(cryptix.mime.LegacyString.toByteArray(message,
                        offset, length))</code> instead.
     */
    public final void add(String message, int offset, int length) {
        md.update(LegacyString.toByteArray(message, offset, length));
    }

    /**
     * Adds the low bytes of a string to the digest. This should not be used
     * when internationalization is a concern, since it does not handle
     * Unicode characters above <code>&#92;u00FF</code> gracefully.
     *
     * @param  message  the string to add.
     * @exception NullPointerException if message == null
     * @deprecated Use <code><a href="java.security.MessageDigest.html#update(byte[])">update</a>(cryptix.mime.LegacyString.toByteArray(message))</code> instead.
     */
    public final void add(String message) {
        md.update(LegacyString.toByteArray(message));
    }

    /**
     * Adds a byte array to the digest.
     *
     * @param  data     the data to be added.
     * @exception NullPointerException if data == null
     * @deprecated Use <code><a href="java.security.MessageDigest.html#update(byte[])">update</a>(data)</code> instead.
     */
    public final void add(byte[] data) {
        md.update(data);
    }

    /**
     * Adds a section of a byte array to the digest.
     *
     * @param  data     the data to add.
     * @param  offset   the start of the data to add.
     * @param  length   the length of the data to add.
     * @exception NullPointerException if data == null
     * @exception ArrayIndexOutOfBoundsException if offset or length are
     *                  out of range
     * @deprecated Use <code><a href="java.security.MessageDigest.html#update(byte[], int, int)">update</a>(data, offset, length)</code>.
     */
    public final void add(byte[] data, int offset, int length) {
        md.update(data, offset, length);
    }

    public final byte[] digest(byte[] data) {
        return md.digest(data);
    }

    public final byte[] digest() {
        return md.digest();
    }

    public final void reset() {
        md.reset();
    }

    /**
     * A convenience function for hashing the low bytes of a string. This should
     * not be used when internationalization is a concern, since it does not
     * handle Unicode characters above <code>&#92;u00FF</code> gracefully.
     * <p>
     * E.g.:
     * <pre>
     *   import cryptix.security.MessageDigest;
     * <br>
     *   byte[] key = MessageDigest.hash(passphrase, new SHA());
     * </pre>
     * which should be replaced with:
     * <pre>
     *   import java.security.MessageDigest;
     *   import cryptix.mime.LegacyString;
     * <br>
     *   MessageDigest sha = MessageDigest.getInstance("SHA-1");
     *   byte[] key = sha.digest(LegacyString.toByteArray(passphrase));
     * </pre>
     * or (better for new applications, but not completely compatible):
     * <pre>
     *   ...
     *   byte[] key = sha.digest(passphrase.getBytes("UTF8"));
     * </pre>
     *
     * @param  message  the string to hash.
     * @param  md       an instance of a message digest.
     * @deprecated
     */
    public static byte[] hash(String message, cryptix.security.MessageDigest md) {
        return md.digest(LegacyString.toByteArray(message));
    }

    /**
     * A convenience function for hashing a byte array.
     * <p>
     * E.g.:
     * <pre>
     *   import cryptix.security.MessageDigest;
     * <br>
     *   byte[] key = MessageDigest.hash(bytearray, new SHA());
     * </pre>
     * which should be replaced with
     * <pre>
     *   import java.security.MessageDigest;
     * <br>
     *   MessageDigest sha = MessageDigest.getInstance("SHA-1");
     *   byte[] key = sha.digest(bytearray);
     * </pre>
     *
     * @param  message  the byte array to hash
     * @param  md       an instance of a message digest
     * @deprecated Use <code>md.<a href="java.security.MessageDigest.html#digest(byte[])">digest</a>(data)</code> instead.
     */
    public static byte[] hash(byte[] data, cryptix.security.MessageDigest md) {
        return md.digest(data);
    }
}
