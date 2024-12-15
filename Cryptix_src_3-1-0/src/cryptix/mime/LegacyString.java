// This file is currently locked by jbaker@madge.com
//
// $Log: LegacyString.java,v $
// Revision 1.2  1997/11/20 19:20:52  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/08/14  David Hopwood
// + Import Hex from cryptix.util.core, not cryptix.util.
//
// Revision 0.2.5.1  1997/06/29  David Hopwood
// + Added more documentation, and changed "\" to "&92;" in javadoc comments.
// + Added methods to accept substrings and subarrays.
// + Added toByteArrayLossless methods.
// + Changed to avoid StringBuffer for efficiency.
// + Used private LegacyString() {} convention, instead of the class being
//   abstract.
//
// Revision 0.2.5.0  1997/02/28  Jill Baker
// + Original version.
//
// $Endlog$
/*
 * Copyright (c) 1995-1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.mime;

import cryptix.util.core.Hex;

/**
 * This class contains static functions useful in converting between
 * Strings and byte arrays. The name of this class is to remind you that
 * there are better ways of performing such conversions these days -
 * Java 1.1 supplies its own byte-to-char converters, and the Reader and
 * Writer classes take care of most of the more finicky details in a much
 * better way than is done here. Nonetheless, this class exists in order
 * to maintain backwards compatibility with entities which were written
 * before the advent of Java and Unicode.
 * <p>
 * Note that class contains only static functions, and has no public
 * constructor.
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.2 $</b>
 * @author  Jill Baker
 * @author  David Hopwood
 * @since   Cryptix 2.2
 */
public class LegacyString
{
    private LegacyString() {} // static methods only

    /**
     * Converts a String to an ISO-Latin-1 encoded byte array.
     * <p>
     * This conversion function is neccessary because Java 1.1+ uses
     * machine dependent char-to-byte and byte-to-char conversions. This
     * is normally a useful feature, allowing machine independence.
     * In order to maintain backwards compatibility with pre-Java
     * conventions, we implement our own machine-independent
     * deterministic converter.
     *
     * @param s         the string to be converted to bytes.
     * @param offset    the character offset within the string to start at.
     * @param length    the number of characters to be converted.
     * @return an array of bytes containing the lower 8 bits of each char.
     *
     * @since Cryptix 2.2.2
     */
    public static byte[] toByteArray(String s, int offset, int length)
    {
        byte[] buf = new byte[length];
        for (int i = 0; i < length; i++)
            buf[i] = (byte) s.charAt(offset + i);

        return buf;
    }

    /**
     * Equivalent to <code>toByteArray(s, 0, s.length())</code>.
     */
    public static byte[] toByteArray(String s)
    {
        return toByteArray(s, 0, s.length());
    }

    /**
     * This method is similar to <code>toByteArray(s, offset, length)</code>,
     * but throws an <samp>IllegalArgumentException</code> if any of the
     * characters in <i>s</i> are outside the range &#92;u0000 to &#92;u00FF.
     *
     * @param s         the string to be converted to bytes.
     * @param offset    the character offset within the string to start at.
     * @param length    the number of characters to be converted.
     * @return an array of bytes containing the lower 8 bits of each char.
     * @exception IllegalArgumentException if any of the characters in s are out of range.
     *
     * @since Cryptix 2.2.2
     */
    public static byte[] toByteArrayLossless(String s, int offset, int length)
    {
        byte[] buf = new byte[length];
        char c;
        for (int i = 0; i < length; i++) {
            c = s.charAt(offset + i);
            if (c > '\u00FF')
                throw new IllegalArgumentException("non-ISO-Latin-1 character in input: \\"
                    + "u" + Hex.shortToString(c));
            buf[i] = (byte) c;
        }

        return buf;
    }

    /**
     * This method is similar to <code>toByteArray(s)</code>,
     * but throws an <samp>IllegalArgumentException</code> if any of the
     * characters in <i>s</i> are outside the range &#92;u0000 to &#92;u00FF.
     *
     * @param s the string to be converted to bytes.
     * @return an array of bytes containing the lower 8 bits of each char.
     * @exception IllegalArgumentException if any of the characters in s are out of range.
     *
     * @since Cryptix 2.2.2
     */
    public static byte[] toByteArrayLossless(String s)
    {
        return toByteArrayLossless(s, 0, s.length());
    }

    /**
     * Converts an ISO-Latin-1 encoded byte array to a String.
     * <p>
     * This conversion function is neccessary because Java 1.1+ uses
     * machine dependent char-to-byte and byte-to-char conversions. This
     * is normally a useful feature, allowing machine independence.
     * In order to maintain backwards compatibility with pre-Java
     * conventions, we implement our own machine-independent
     * deterministic converter.
     *
     * @param b         an array of bytes to be converted to a String.
     * @param offset    the character offset within the string to start at.
     * @param length    the number of characters to be converted.
     * @return a String containing the bytes. Each char in the string
     *          will be in the range &#92;u0000 to &#92;u00FF.
     *
     * @since Cryptix 2.2.2
     */
    public static String toString(byte[] b, int offset, int length)
    {
        char[] cbuf = new char[length];
        for (int i = 0; i < length; i++)
            cbuf[i] = (char) (b[i] & 0xFF);

        return new String(cbuf);
    }

    /**
     * Equivalent to <code>toString(b, 0, b.length)</code>.
     */
    public static String toString(byte[] b)
    {
        return toString(b, 0, b.length);
    }
}