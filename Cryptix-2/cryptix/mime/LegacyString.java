/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package cryptix.mime;

/**
 * This class contains static functions useful in converting to between
 * Strings and byte arrays. The name of this class is to remind you that
 * there are better ways of performing such conversions these days -
 * Java 1.1 supplies its own byte-to-char converters, and the Reader and
 * Writer classes take care of most of the more finicky details in a much
 * better way than is done here. Nonetheless, this class exists in order
 * to maintain backwards compatibility with entities which were written
 * before the advent of Java and Unicode.<p>
 *
 * Note that class contains only static functions and has been declared
 * abstract since it will never need to be instantiated.<p>
 *
 * @author Jill Baker
 */
public abstract class LegacyString
{

    /**
     * This conversion function is neccessary because Java 1.1+ uses
     * machine dependent char-to-byte and byte-to-char conversions. This
     * is normally a useful feature, allowing machine independence.
     * In order to maintain backwards with pre-Java conventions, we
     * implement out own machine-independent deterministic converter.
     *
     * @param s string to be converted to bytes
     * @return array of bytes containing lower 8 bits of each char
     */

    public static final byte[] toByteArray( String s )
    {
        int len = s.length();
        byte[] temp = new byte[ len ];
        for (int i=0; i<len; i++) temp[i] = (byte) s.charAt( i );
        return temp;
    }

    /**
     * This conversion function is neccessary because Java 1.1+ uses
     * machine dependent char-to-byte and byte-to-char conversions. This
     * is normally a useful feature, allowing machine independence.
     * In order to maintain backwards with pre-Java conventions, we
     * implement out own machine-independent deterministic converter.
     *
     * @param b array of bytes to be converted to a String
     * @return String containing the bytes. Each char in the string
     *         will be in the range \u0000 to \u00FF
     */

    public static final String toString( byte[] b )
    {
        int len = b.length;
        StringBuffer s = new StringBuffer( len );
        for (int i=0; i<len; i++) s.append( (char)(b[i] & 0xFF) );
        return new String( s );
    }

}
