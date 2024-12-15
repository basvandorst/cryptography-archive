/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package cryptix.mime;

import java.util.Properties;

/**
 * This class provides static functions to convert strings to and from
 * the "canonical" form in which all line breaks are represented as
 * ASCII CR followed by ASCII LF.
 *
 * @author Jill Baker
 */

public abstract class CRLF
{
    /**
     * Convert all line breaks within a string (represented by either '\r',
     * '\n' or '\r\n') to canonical ('\r\n') linebreaks.
     *
     * @param s text to be encoded
     * @return text with canonical line breaks
     */
    public static String encode( String s )
    {
        StringBuffer buffer = new StringBuffer( s.length() );

        int len = s.length();
        for (int i=0; i<len; i++)
        {
            int c = s.charAt( i );

            if ((c == '\r') && (i != len-1) && (s.charAt( i+1 ) == '\n'))
            {
                buffer.append( "\r\n" );
                i++;
            }
            else if ((c == '\r') || (c == '\n'))
            {
                buffer.append( "\r\n" );
            }
            else
            {
                buffer.append( (char)c );
            }
        }
        return buffer.toString();
    }

    /**
     * Convert all line breaks within a byte array (represented by either '\r',
     * '\n' or '\r\n') to canonical ('\r\n') linebreaks.
     *
     * @param b text to be encoded
     * @return text with canonical line breaks
     */
    public static byte[] encode( byte[] b )
    {
        return LegacyString.toByteArray( encode( LegacyString.toString( b )));
    }

    /**
     * Convert all canonical line breaks with a string (represented by
     * "\r\n" to machine-specific linebreaks.
     *
     * @param s text (with canonical line breaks) to be decoded
     * @return text with machine-specific line breaks
     */
    public static String decode( String s )
    {
        StringBuffer buffer = new StringBuffer( s.length() );
        Properties p = System.getProperties();
        String localNewline = p.getProperty( "line.separator", "\r\n" );

        int len = s.length();
        for (int i=0; i<len; i++)
        {
            int c = s.charAt( i );

            if ((c == '\r') && (i != len-1) && (s.charAt( i+1 ) == '\n'))
            {
                buffer.append( localNewline );
                i++;
            }
            else
            {
                buffer.append( (char)c );
            }
        }
        return buffer.toString();
    }

    /**
     * Convert all canonical line breaks with a byte array (represented by
     * "\r\n" to machine-specific linebreaks.
     *
     * @param b text (with canonical line breaks) to be decoded
     * @return text with machine-specific line breaks
     */
    public static byte[] decode( byte[] b )
    {
        return LegacyString.toByteArray( decode( LegacyString.toString( b )));
    }

    /**
     * Test whether or not all line breaks within a String are of the
     * form "\r\n".
     *
     * @param s String to be tested.
     * @return true if all line breaks are of the form "\r\n".
     */
    public static boolean isProbablyEncoded( String s )
    {
        int len = s.length();
        for (int i=0; i<len; i++)
        {
            int c = s.charAt( i );

            if ((c == '\r') && (i != len-1) && (s.charAt( i+1 ) == '\n'))
                continue;

            if ((c == '\r') || (c == '\n')) return false;
        }
        return true;
    }

    /**
     * Test whether or not all line breaks within a byte array are of the
     * form "\r\n".
     *
     * @param b byte array to be tested.
     * @return true if all line breaks are of the form "\r\n".
     */
    public static boolean isProbablyEncoded( byte[] b )
    {
        int len = b.length;
        for (int i=0; i<len; i++)
        {
            int c = b[i];

            if ((c == '\r') && (i != len-1) && (b[i+1] == '\n'))
                continue;

            if ((c == '\r') || (c == '\n')) return false;
        }
        return true;
    }
}
