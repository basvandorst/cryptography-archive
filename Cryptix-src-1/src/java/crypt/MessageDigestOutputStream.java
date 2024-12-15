/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */
 
package java.crypt;

import java.io.OutputStream;

/**
 * This class is an Output stream that hashes the data sent to it using
 * the message digest that it has been constructed with.
 * @see MD5OutputStream
 * @see SHAOutputStream
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.00, 22/05/1996
 * @author      Systemics Ltd. (Michael Wynn)
 */
public class MessageDigestOutputStream extends OutputStream
{
	private byte tmp_byte_buf[] = new byte[1]; // temporary buffer for the write( int ) function.
	private MessageDigest md;
	
	/**
	 * @param md0 the message digestor that is uses to hash the data.
	 */
	public MessageDigestOutputStream( MessageDigest md0 )
	{
		md = md0;
	}

    /**
     * Writes a byte to the message digestor.
     * @param b	the byte
     */
    public final synchronized void
	write( int b )
	{
		tmp_byte_buf[0] = (byte)b;
		md.add( tmp_byte_buf );
	}

    /**
     * Writes an array of bytes to the message digestor
     * @param b	the data to be added to the hash
     */
    public final void
	write( byte b[] )
	{
		md.add( b );
	}

    /**
     * Writes a sub array of bytes to the message digestor. 
     * @param b	the data to be added to the hash
     * @param off	the start offset in the data
     * @param len	the number of bytes that are written
     */
    public final void
	write(byte b[], int off, int len)
	{
		md.add( b, off, len );
    }
	
	
	// what about toByteArray and toMessageHash functions like ByteArrayOutputStream ???
    /**
	 * @return the final digest (as a byte array) of the data added and resets the digestor
	 */
	public final byte[]
	digest()
	{
		return md.digest();
	}

    /**
	 * @return the final digest (as an object) of the data added and resets the digestor
	 */
	public final MessageHash
	digestAsHash()
	{
		return md.digestAsHash();
	}
}

