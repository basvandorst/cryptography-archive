// This file is currently unlocked (change this line if you lock the file)
//
// $Log: MessageDigestOutputStream.java,v $
// Revision 1.2  1997/12/07 06:40:26  hopwood
// + Major overhaul of cryptix.security.*.
//
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
//   Moved this file here from old namespace.
//
// Revision 0.2.5.0  1997/02/24  Systemics
//   Original version
//
// $Endlog$
/*      
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */
 
package cryptix.security;

import java.io.OutputStream;

/**
 * This class is an output stream that hashes the data sent to it using
 * the message digest that it has been constructed with.
 *
 * <P>
 * <b>References</b>
 *
 * <P>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 *
 * <P>
 * <p><b>$Revision: 1.2 $</b>
 * @author Systemics Ltd
 * @see MessageDigest
 * @see SHAOutputStream
 * @see MD5OutputStream
 */
public class MessageDigestOutputStream extends OutputStream
{
    private byte tmp_byte_buf[] = new byte[1]; // temporary buffer for the write(int) function.
    private MessageDigest md;
    
    /**
     * @param md0 the message digestor that is uses to hash the data.
     */
    public MessageDigestOutputStream(MessageDigest md0)
    {
        md = md0;
    }

    /**
     * Writes a byte to the message digestor.
     * @param b     the byte
     */
    public final synchronized void
    write(int b)
    {
        tmp_byte_buf[0] = (byte)b;
        md.add(tmp_byte_buf);
    }

    /**
     * Writes an array of bytes to the message digestor
     * @param b     the data to be added to the hash
     */
    public final void
    write(byte b[])
    {
        md.add(b);
    }

    /**
     * Writes a sub array of bytes to the message digestor. 
     * @param b     the data to be added to the hash
     * @param off   the start offset in the data
     * @param len   the number of bytes that are written
     */
    public final void
    write(byte b[], int off, int len)
    {
        md.add(b, off, len);
    }
    
    
    //
    // what about toByteArray and toMessageHash functions
    // like ByteArrayOutputStream ???
    /**
     * Completes the digest and resets.
     * @return      the final digest (as a byte array) of the data added
     */
    public final byte[]
    digest()
    {
        return md.digest();
    }

    /**
     * Completes the digest and resets.
     * @return      the final digest (as a Hash object) of the data added
     */
    public final MessageHash
    digestAsHash()
    {
        return md.digestAsHash();
    }
}
