// $Id: QuotedPrintable.java,v 1.2 1999/06/30 22:53:04 edwin Exp $
//
// $Log: QuotedPrintable.java,v $
// Revision 1.2  1999/06/30 22:53:04  edwin
// Most of the methods are not static, so a private constructor doesn't make sense.
//
// Revision 1.1.1.1  1997/12/22 03:28:42  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0  1997/12/22  hopwood
// + Original version (based on cryptix.mime.QuotedPrintable).
//
// $Endlog$
/*
 * Copyright (c) 1995-1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.util.mime;

import java.io.PrintWriter;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

/**
 * This class provides methods for encoding and decoding data in
 * MIME quoted-printable format, as described in RFC 2045 (Section 6.7).
 * <p>
 * QuotedPrintable is a Content-Transfer-Encoding. That is, the input to
 * encoding is a sequence of bytes, and the output is a sequence of
 * characters (similar to base64). It is orthogonal to the character
 * encoding.
 * <p>
 * For example, suppose a MIME message consists of Unicode text with a
 * Content-Type of "text/plain; charset=UTF8", and a
 * Content-Transfer-Encoding of "quoted-printable" (this would be unusual,
 * but valid). A MIME message should be viewed as a sequence of abstract
 * characters, which in this case could be decoded as bytes according
 * to the "quoted-printable" transfer encoding, and then back to a
 * (different, in general) sequence of characters according to the "UTF8"
 * character encoding.
 * <p>
 * The input to the <code>encode</code> methods is always a byte array.
 * Strictly speaking the output represents a sequence of characters, but
 * since these characters are from a subset of both the Unicode and ASCII
 * character repertoires, it is possible to express the output either as
 * a String or as a byte array.
 * <p>
 * <b>References:</b>
 * <ol>
 *   <li> RFC 2045, <cite>Multipurpose Internet Mail Extensions (MIME) Part One,
 *        Format of Internet Message Bodies</cite>,
 *        "Section 6.7 Quoted-Printable Content-Transfer-Encoding,"
 *        <a href="http://www.imc.org/rfc2045">http://www.imc.org/rfc2045</a>
 *        <p>
 *   <li> Dan Connolly, draft-ietf-html-charset-harmful-00.txt,
 *        <cite>"Character Set" Considered Harmful</cite>,
 *        <a href="http://www.w3.org/pub/WWW/MarkUp/html-spec/charset-harmful.html">
 *        http://www.w3.org/pub/WWW/MarkUp/html-spec/charset-harmful.html</a>
 * </ol>
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.2 $</b>
 * @author  Jill Baker
 * @since   Cryptix 3.0.2
 */
public class QuotedPrintable
extends TransferEncoding
{
    public QuotedPrintable() {}

    /**
     * Encodes a byte array using quoted-printable encoding, and returns
     * the result as a String.
     * <p>
     * Line breaks in the input MUST be represented as "\r\n" - any other
     * combination is treated as binary data. Line breaks in the output
     * will always be represented as "\r\n".
     *
     * @param  b    canonical data to be encoded
     * @return output in quoted-printable form.
     */
    public String encode(byte[] b)
    {
        int len = b.length;

        StringBuffer buffer = new StringBuffer(2 * len);
        StringBuffer line   = new StringBuffer(240);
        StringBuffer white  = new StringBuffer(80);

        for (int i=0; i<len; i++)
        {
            char c = (char) (b[i]);

            if (c == '\r' && i != len-1 && b[i+1] == '\n')
            {
                encodeEndOfLine(buffer, line, white);
                i++;
                continue;
            }
            else if (c == ' ' || c == '\t')
            {
                white.append(c);
            }
            else
            {
                if (white.length() != 0)
                {
                    line.append(white);
                    white.setLength(0);
                }

                if (c >= '!' && c <= '~' && c != '=')
                {
                    line.append(c);
                }
                else
                {
                    appendHex(line, c);
                }
            }
        }
        if (line.length() != 0 || white.length() != 0)
        {
            encodeEndOfLine(buffer, line, white);
        }
        return buffer.toString();
    }

    private void encodeEndOfLine(StringBuffer buffer, StringBuffer line,
                                 StringBuffer white)
    {
        int i,j;

        for (i=0; i<white.length(); i++)
        {
            appendHex(line, white.charAt(i));
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
                if (s.charAt(j-1) == '=') j -= 1;
                if (s.charAt(j-2) == '=') j -= 2;
            }
            buffer.append(s.substring(i, j));
            if (j != len) buffer.append('=');
            buffer.append("\r\n");
        }
        line.setLength(0);
        white.setLength(0);
    }

    private void appendHex(StringBuffer line, int c)
    {
        c &= 0xFF;
        line.append('=');
        line.append("0123456789ABCDEF".charAt(c >> 4));
        line.append("0123456789ABCDEF".charAt(c & 0x0F));
    }

    /**
     * Takes a quoted-printable encoding and decodes it to find the corresponding
     * sequence of bytes.
     * <p>
     * Unrecognised sequences starting with '=' are passed through unmodified,
     * as are characters that do not correspond to bytes that could have been
     * output by a correct quoted-printable encoder. Note that this method will
     * strip trailing whitespace from each line.
     *
     * @param  s    input string in quoted-printable form
     * @return the decoded data.
     */
    public byte[] decode(String s)
    {
        StringBuffer buffer = new StringBuffer(s.length());
        StringBuffer white = new StringBuffer(80);
        int len = s.length();

        for (int i=0; i<len; i++)
        {
            char c = s.charAt(i);

            if ((c == '=') && (i+2 < len))
            {
                char a = s.charAt(++i);
                char b = s.charAt(++i);
                if ((a == '\r') && (b == '\n')) continue;

                int ah = "0123456789ABCDEF".indexOf(a);
                int bh = "0123456789ABCDEF".indexOf(b);
                if ((ah == -1) || (bh == -1))
                {
                    buffer.append('=');
                    buffer.append(a);
                    buffer.append(b);
                }
                else
                {
                    buffer.append((char) ((ah << 4) | bh));
                }
            }
            else if ((c == ' ') || (c == '\t'))
            {
                white.append(c);
            }
            else
            {
                if (white.length() != 0) buffer.append(white);
                buffer.append(c);
                white.setLength(0);
            }
        }
        return ISOLatin1.toByteArray(buffer.toString());
    }

    /**
     * Tests whether or not an encoding, expressed as if it were an ASCII String,
     * could have been generated by <code>encode()</code> or any similar generator
     * that complies with the RFC.
     */
    public static boolean isProbablyEncoded(String s)
    {
        int len = s.length();
        int lineStart = 0;
        boolean white = false;

        for (int i=0; i<len; i++)
        {

            char c = s.charAt(i);
            if (i - lineStart > 76) return false;

            if ((c == '=') && (i+2 < len))
            {
                char a = s.charAt(++i);
                char b = s.charAt(++i);
                if ((a == '\r') && (b == '\n'))
                {
                    white = false;
                    lineStart = i+1;
                    continue;
                }

                if ("0123456789ABCDEF".indexOf(a) == -1) return false;
                if ("0123456789ABCDEF".indexOf(b) == -1) return false;
            }
            else if ((c == '\r') && (i+1 < len))
            {
                if (white) return false;
                char a = s.charAt(++i);
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

    /**
     * Tests whether or not an encoding could have been generated by
     * <code>encodeAsByteArray()</code> or any similar generator that complies
     * with the RFC.
     */
    public static boolean isProbablyEncoded(byte[] buf)
    {
        return isProbablyEncoded(ISOLatin1.toString(buf));
    }


// Test methods
//...........................................................................

/* need a better test than this.
    public static void main(String[] args) {
        try {
            // Changed to read input from stdin rather than a file, to avoid
            // security problems.
            self_test(new PrintWriter(System.out, true), System.in);
        } catch (Exception e) { e.printStackTrace(); }
    }

    private static void self_test(PrintWriter out, InputStream in)
    throws Exception {
        ByteArrayOutputStream bs = new ByteArrayOutputStream();
        for (int c = in.read(); c != -1; c = in.read())
            bs.write(c);

        byte[] b = bs.toByteArray();
        out.println("Is probably encoded: " + isProbablyEncoded(b));
    }
*/
}
