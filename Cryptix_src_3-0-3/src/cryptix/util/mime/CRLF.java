// $Id: CRLF.java,v 1.1.1.1 1997/12/22 03:28:42 hopwood Exp $
//
// $Log: CRLF.java,v $
// Revision 1.1.1.1  1997/12/22 03:28:42  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/06/21  David Hopwood
// + Documented the possible character encodings for byte arrays.
// + Changed use of System.getProperties to System.getProperty
//   (which does not require security privileges).
// + Use "char c", not "int c", so that fewer casts are needed.
// + Added shortcut if the local newline is already "\r\n".
//
// Revision 0.2.5.1  1997/03/02  Jill Baker
// + Bug Fix. Forgot to convert something to a char and it all went wrong.
//
// Revision 0.2.5.0  1997/02/28  Jill Baker
// + Original version.
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.util.mime;

/**
 * This class provides static methods to convert strings to and from
 * the MIME "canonical" form, in which all line breaks are represented
 * as ASCII CR followed by ASCII LF.
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.1.1.1 $</b>
 * @author  Jill Baker
 * @since   Cryptix 2.2
 */
public class CRLF
{
    private CRLF() {} // static methods only

    /**
     * Converts all line breaks within a string (represented by either "\r",
     * "\n" or "\r\n") to canonical ("\r\n") linebreaks.
     *
     * @param s text to be converted
     * @return text with canonical line breaks
     */
    public static String encode(String s)
    {
        StringBuffer buffer = new StringBuffer(s.length());

        int len = s.length();
        for (int i=0; i<len; i++)
        {
            char c = s.charAt(i);

            if ((c == '\r') && (i != len-1) && (s.charAt(i+1) == '\n'))
            {
                buffer.append("\r\n");
                i++;
            }
            else if ((c == '\r') || (c == '\n'))
            {
                buffer.append("\r\n");
            }
            else
            {
                buffer.append(c);
            }
        }
        return buffer.toString();
    }

    /**
     * Converts all line breaks within a byte array (represented by either
     * "\r", "\n" or "\r\n") to canonical ("\r\n") linebreaks.
     * <p>
     * The byte array can be in ASCII, ISO-Latin-x, UTF8, UTF7, or
     * Quoted-Printable encoding, or any similar character encoding for which
     * CR and LF are represented in the same way as ASCII, and no other bytes
     * can have the values 10 or 13.
     *
     * @param b text to be converted
     * @return text with canonical line breaks
     */
    public static byte[] encode(byte[] b)
    {
        return ISOLatin1.toByteArray(encode(ISOLatin1.toString(b)));
    }

    /**
     * Converts all canonical line breaks with a string (represented by
     * "\r\n") to machine-specific linebreaks.
     *
     * @param s text (with canonical line breaks) to be decoded
     * @return text with machine-specific line breaks
     */
    public static String decode(String s)
    {
        String localNewline = System.getProperty("line.separator", "\r\n");
        if (localNewline.equals("\r\n"))
            return s;

        StringBuffer buffer = new StringBuffer(s.length());
        int len = s.length();
        for (int i=0; i<len; i++)
        {
            char c = s.charAt(i);

            if ((c == '\r') && (i != len-1) && (s.charAt(i+1) == '\n'))
            {
                buffer.append(localNewline);
                i++;
            }
            else
            {
                buffer.append(c);
            }
        }
        return buffer.toString();
    }

    /**
     * Convert all canonical line breaks with a byte array (represented by
     * "\r\n") to machine-specific linebreaks.
     * <p>
     * The byte array can be in ASCII, ISO-Latin-x, UTF8, UTF7, or
     * Quoted-Printable encoding, or any similar character encoding for which
     * CR and LF are represented in the same way as ASCII, and no other bytes
     * can have the values 10 or 13.
     *
     * @param b text (with canonical line breaks) to be decoded
     * @return text with machine-specific line breaks
     */
    public static byte[] decode(byte[] b)
    {
        return ISOLatin1.toByteArray(decode(ISOLatin1.toString(b)));
    }

    /**
     * Test whether or not all line breaks within a String are of the
     * form "\r\n".
     *
     * @param s String to be tested.
     * @return true if all line breaks are of the form "\r\n".
     */
    public static boolean isProbablyEncoded(String s)
    {
        int len = s.length();
        for (int i=0; i<len; i++)
        {
            char c = s.charAt(i);

            if ((c == '\r') && (i != len-1) && (s.charAt(i+1) == '\n'))
                continue;

            if ((c == '\r') || (c == '\n')) return false;
        }
        return true;
    }

    /**
     * Tests whether or not all line breaks within a byte array are of the
     * form "\r\n".
     * <p>
     * The byte array can be in ASCII, ISO-Latin-x, UTF8, UTF7, or
     * Quoted-Printable encoding, or any similar character encoding for which
     * CR and LF are represented in the same way as ASCII, and no other bytes
     * can have the values 10 or 13.
     *
     * @param b byte array to be tested.
     * @return true if all line breaks are of the form "\r\n".
     */
    public static boolean isProbablyEncoded(byte[] b)
    {
        int len = b.length;
        for (int i=0; i<len; i++)
        {
            char c = (char) b[i]; // signed vs unsigned doesn't matter in this case

            if ((c == '\r') && (i != len-1) && (b[i+1] == '\n'))
                continue;

            if ((c == '\r') || (c == '\n')) return false;
        }
        return true;
    }
}
