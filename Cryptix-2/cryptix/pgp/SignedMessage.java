/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */
package cryptix.pgp;

import java.io.DataOutput;
import java.io.IOException;

import cryptix.math.RandomStream;
import cryptix.security.MD5;
import cryptix.security.rsa.SecretKey;
import cryptix.security.rsa.PublicKey;

import cryptix.util.StreamInterface;
/**
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 */
 
public final class SignedMessage extends Message
{
	private Signature messageSignature;
	
	public SignedMessage( byte data[], SecretKey secretkey, byte key[], RandomStream rand )
	throws IOException
	{
                super( data, secretkey, key, rand );
		MD5 md = new MD5();
		md.add( data );
                messageSignature = new Signature( secretkey, md, rand );
	}

	public void
	write( DataOutput out )
	throws IOException
	{
		super.write( out );
		PacketFactory.write( out, messageSignature );
	}
}
