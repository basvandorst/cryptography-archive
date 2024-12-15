/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */

package cryptix.pgp;

import cryptix.math.RandomStream;
import cryptix.security.BlockCipher;
import cryptix.security.IDEA;
import java.io.ByteArrayInputStream;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.io.FileOutputStream;

/**
 * @author Original author not stated
 *
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 */
public final class ConvEncryptedData extends Packet
{
	private byte buffer[] = null;

	public ConvEncryptedData( DataInput in, int length )
		throws IOException
	{
		super( in, length );
	}
	
	public ConvEncryptedData( byte data[], byte key[], RandomStream rand )
		throws IOException
	{
		buffer = new byte[data.length + 10];
		IDEA block_cipher = new IDEA(key);
		CFB cipher = new CFB(block_cipher);
		byte pre[] = getKeyCheck(rand);

		int blen = block_cipher.blockLength();

		cipher.encrypt(pre, 0, buffer, 0, 10);
		cipher.decrypt(buffer, 2, new byte[blen], 0, blen);	// Set the IV
		cipher.encrypt( data, 0, buffer, 10, data.length );
	}
	
        public byte[] getBuffer() { return buffer; }

	private byte[]
	getKeyCheck( RandomStream rand )
	{
		byte tmp[] = new byte[10];
		for ( int i = 7; i >= 0; i-- )
		{
			byte fill;
			do
				fill = (byte)rand.read();
			while ( fill == 0 );
			tmp[i] = fill;
		}
		tmp[8] = tmp[6];
		tmp[9] = tmp[7];
		return tmp;
	}
	
	public void 
	read( DataInput in, int length )
		throws IOException
	{
		if (length <= 11 )
			throw new FormatException("Length too short");

		buffer = new byte[length];
		in.readFully( buffer );
	}

	public int
	write( DataOutput out )
		throws IOException
	{
		out.write( buffer );
		return buffer.length;
	}

	public byte[]
	data( byte key[] )
		throws DecryptException
	{
		IDEA block_cipher = new IDEA(key);
		CFB cipher = new CFB(block_cipher);
		int blen = block_cipher.blockLength();

		byte returnBuf[] = new byte[buffer.length - 10];
		byte iv[] = new byte[10];

		System.arraycopy( buffer, 0, iv, 0, 10 );
		System.arraycopy( buffer, 10, returnBuf, 0, returnBuf.length );

		// Set the IV and verify check digits
		cipher.decrypt(buffer, 0, iv, 0, iv.length);
		if ( ( iv[6] != iv[8] ) || ( iv[7] != iv[9] ) )
			throw new DecryptException("Invalid decryption key");

		cipher.decrypt(buffer, 10, returnBuf, 0, returnBuf.length);
		return returnBuf;
	}

    public int getType()
    {
        return Packet.CONV_ENCRYPTED;
    }
}


