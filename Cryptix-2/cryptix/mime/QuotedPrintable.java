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
 * This class provides static functions to convert strings to and from
 * the QuotedPrintable format described in RFC 2045 (Section 6.7). Note
 * that although the arguments are passed and returned as Strings, they more
 * correctly represent byte[]s, since every char is expected to be in
 * the range \u0000 to \u00FF.<p>
 *
 * @author Jill Baker
 */

public abstract class QuotedPrintable
{
    /**
     * Convert a string from text canonical form to Quoted Printable form.
     * Line breaks (both before and after conversion) MUST be represented
     * as \r\n - any other combination is treated as binary data.
     *
     * @param s canonical text to be encoded
     * @return text in Quoted Printable form
     */
    public static String encode( String s )
    {
        int len = s.length();

        StringBuffer buffer = new StringBuffer( 2 * len );
        StringBuffer line   = new StringBuffer( 240 );
        StringBuffer white  = new StringBuffer( 80 );

        for (int i=0; i<len; i++)
        {
            int c = s.charAt( i );

            if ((c == '\r') && (i != len-1) && (s.charAt( i+1 ) == '\n'))
            {
                encodeEndOfLine( buffer, line, white );
                i++;
                continue;
            }
            else if ((c == ' ') || (c == '\t'))
            {
                white.append( (char)c );
            }
            else
            {
                if (white.length() != 0)
                {
                    line.append( white );
                    white.setLength( 0 );
                }

                if ((c >= '!') && (c <= '~') && (c != '='))
                {
                    line.append( (char)c );
                }
                else
                {
                    appendHex( line, c );
                }
            }
        }
        if ((line.length() != 0) || (white.length() != 0))
        {
            encodeEndOfLine( buffer, line, white );
        }
        return buffer.toString();
    }

    private static void encodeEndOfLine(
        StringBuffer buffer, StringBuffer line, StringBuffer white )
    {
        int i,j;

        for (i=0; i<white.length(); i++)
        {
            appendHex( line, white.charAt( i ));
        }
        String s = line.toString();
        int len = s.length();
        boolean split = false;
        for (i=0; i<len; i=j)
        {
            j = i + 75;
            if ((j == len-1) || (j > len)) j = len;
            if (j > 2)
            {
                if (s.charAt( j-1 ) == '=') j -= 1;
                if (s.charAt( j-2 ) == '=') j -= 2;
            }
            buffer.append( s.substring( i, j ));
            if (j != len) buffer.append( '=' );
            buffer.append( "\r\n" );
        }
        line.setLength( 0 );
        white.setLength( 0 );
    }

    private static void appendHex( StringBuffer line, int c )
    {
        c &= 0xFF;
        line.append( '=' );
        line.append( "0123456789ABCDEF".charAt( c >> 4 ));
        line.append( "0123456789ABCDEF".charAt( c & 0x0F ));
    }

    /**
     * Convert a string from Quoted Printable form to text canonical form.
     * Unrecognised sequences starting with '=' are passed through unmodified.
     * Note that this method will strip trailing whitespace from each line.
     *
     * @param s canonical text to be encoded
     * @return text in Quoted Printable form
     */
    public static String decode( String s )
    {
        StringBuffer buffer = new StringBuffer( s.length() );
        StringBuffer white = new StringBuffer( 80 );
        int len = s.length();

        for (int i=0; i<len; i++)
        {
            char c = s.charAt( i );

            if ((c == '=') && (i+2 < len))
            {
                char a = s.charAt( ++i );
                char b = s.charAt( ++i );
                if ((a == '\r') && (b == '\n')) continue;

                int ah = "0123456789ABCDEF".indexOf( a );
                int bh = "0123456789ABCDEF".indexOf( b );
                if ((ah == -1) || (bh == -1))
                {
                    buffer.append( '=' );
                    buffer.append( a );
                    buffer.append( b );
                }
                else
                {
                    buffer.append( (char)((ah << 4) | bh) );
                }
            }
            else if ((c == ' ') || (c == '\t'))
            {
                white.append( c );
            }
            else
            {
                if (white.length() != 0) buffer.append( white );
                buffer.append( c );
                white.setLength( 0 );
            }
        }
        return buffer.toString();
    }

    /**
     * Test whether or not a String could have been generated by encode()
     * or any similar generator which complies with the RFC.
     */
    public static boolean isProbablyEncoded( String s )
    {
        int len = s.length();
        int lineStart = 0;
        boolean white = false;

        for (int i=0; i<len; i++)
        {

            char c = s.charAt( i );
            if (i - lineStart > 76) return false;

            if ((c == '=') && (i+2 < len))
            {
                char a = s.charAt( ++i );
                char b = s.charAt( ++i );
                if ((a == '\r') && (b == '\n'))
                {
                    white = false;
                    lineStart = i+1;
                    continue;
                }

                if ("0123456789ABCDEF".indexOf( a ) == -1) return false;
                if ("0123456789ABCDEF".indexOf( b ) == -1) return false;
            }
            else if ((c == '\r') && (i+1 < len))
            {
                if (white) return false;
                char a = s.charAt( ++i );
                if (a != '\n') return false;
                lineStart = i+1;
            }
            else
            {
                if ((c == ' ') || (c == '\t'))
                    white = true;
                else if ((c >= '!') && (c <= '~') && (c != '='))
                    white = false;
                else
                    return false;
            }
        }
        return true;
    }

    public static void main( String[] arg )
    throws java.io.FileNotFoundException, java.io.IOException
    {
        java.io.FileInputStream fs = new java.io.FileInputStream( arg[0] );
        java.io.ByteArrayOutputStream bs = new java.io.ByteArrayOutputStream();
        for (int c=fs.read();c!=-1;c=fs.read())bs.write(c);
        byte[] b = bs.toByteArray();
        String s = LegacyString.toString(b);

        System.out.println( isProbablyEncoded(s) );

    }   
}
