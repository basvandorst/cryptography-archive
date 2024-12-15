// $Id: TransferEncoding.java,v 1.1.1.1 1997/12/22 03:28:42 hopwood Exp $
//
// $Log: TransferEncoding.java,v $
// Revision 1.1.1.1  1997/12/22 03:28:42  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0  1997/12/20  hopwood
// + Original version (based on the interface of cryptix.mime.Base64,
//   converted to non-static methods).
//
// $Endlog$
/*
 * Copyright (c) 1995-1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.util.mime;
 
/**
 * This abstract class provides an API for encoding and decoding data according
 * to a MIME Content-Transfer-Encoding.
 * <p>
 * Note: The input to the encoding is always a sequence of bytes.
 * Strictly speaking the output is a sequence of characters, but since for
 * all MIME transfer encodings, these characters are from a subset of both
 * the Unicode and ASCII character repertoires, it is possible to represent
 * the output either as a String or as a byte array.
 * <p>
 * <b>References:</b>
 * <ol>
 *   <li> RFC 2045, <cite>Multipurpose Internet Mail Extensions (MIME) Part One,
 *        Format of Internet Message Bodies</cite>,
 *        <a href="http://www.imc.org/rfc2045">http://www.imc.org/rfc2045</a>
 * </ol>
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.1.1.1 $</b>
 * @author  Jill Baker
 * @author  David Hopwood
 * @since   Cryptix 3.0.2
 */
public abstract class TransferEncoding
{
    /**
     * Encodes <i>data</i> as a String using this Content-Transfer-Encoding.
     * Line breaks in the output are represented as CR LF.
     *
     * @return the encoded string.
     */
    public abstract String encode(byte[] data);

    /**
     * Encodes <i>data</i> as a byte array using this Content-Transfer-Encoding.
     * Each character of the output is mapped to its ASCII code point. Line
     * breaks in the output are represented as CR LF (codes 13 and 10).
     *
     * @return the encoded byte array.
     */
    public byte[] encodeAsByteArray(byte[] data) {
        return ISOLatin1.toByteArray(encode(data));
    }

    /**
     * Decodes a byte array containing encoded ASCII. Characters with
     * ASCII code points <= 32 (this includes whitespace and newlines) are
     * ignored.
     *
     * @return the decoded data.
     * @exception MIMEFormatException if data contains invalid characters,
     *            i.e. not codes 0-32 and the other codes allowed for this
     *            encoding, or is otherwise formatted incorrectly.
     */
    public byte[] decode(byte[] data) throws MIMEFormatException {
        return decode(ISOLatin1.toString(data));
    }

    /**
     * Decodes a encoded String. Characters with ASCII code points <= 32
     * (this includes whitespace and newlines) are ignored.
     *
     * @return the decoded data.
     * @exception MIMEFormatException if data contains invalid characters,
     *            i.e. not codes 0-32 and the other codes allowed for this
     *            encoding, or is otherwise formatted incorrectly.
     */
    public abstract byte[] decode(String msg) throws MIMEFormatException;
}
