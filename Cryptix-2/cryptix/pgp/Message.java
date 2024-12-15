/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */
 
/**
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

package cryptix.pgp;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import cryptix.math.RandomStream;
import cryptix.security.rsa.PublicKey;

import cryptix.util.Streamable;

public class Message extends Streamable
{
	private PublicKeyEncrypted messageHdr;
	private ConvEncryptedData messageData;
	
	public Message( byte data[], PublicKey publickey, byte key[], RandomStream rand )
	throws IOException
	{
		messageHdr = new PublicKeyEncrypted( publickey, key, rand );
		messageData = new ConvEncryptedData( data, key, rand );
	}

	public byte[]
	data( byte key[] )
	throws IOException, DecryptException
	{
		return messageData.data( key );
	}
	
	public void 
	read( DataInput in )
	throws IOException
	{
		throw new IOException( "This is an outgoing-only data type." );
	}
	
	public void
	write( DataOutput out )
	throws IOException
	{
		PacketFactory.write( out, messageHdr );
		PacketFactory.write( out, messageData );
	}
}
