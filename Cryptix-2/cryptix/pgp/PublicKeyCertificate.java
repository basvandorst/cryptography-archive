/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */

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
 
package cryptix.pgp;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import cryptix.math.BigInteger;
import cryptix.math.MPI;

import cryptix.security.rsa.PublicKey;

public final class PublicKeyCertificate extends Certificate
{
	private PublicKey publickey;

	public PublicKeyCertificate( PublicKey publickey0 )
	{
		publickey = publickey0;
	}
	
	public PublicKeyCertificate( DataInput in, int length )
	throws IOException
	{
		super( in, length );
	}

	public void 
	read( DataInput in, int length )
	throws IOException
	{
		super.read( in, length );
		if ( in.readByte() != 0x01 ) // RSA algorithm identifier.
			throw new FormatException( "Incorrect algorithm identifier." );
		BigInteger n = MPI.read( in );
		BigInteger e = MPI.read( in );
		publickey = new PublicKey( n, e );
	}

	public int
	write( DataOutput out )
	throws IOException
	{
		byte tmp[];
		int len = super.write( out ) + 1;
		out.write( 0x01 );
		out.write( tmp = MPI.save( publickey.n() ) );
		len += tmp.length;
		out.write( tmp = MPI.save( publickey.e() ) );
		len += tmp.length;
		return len;
	}

	public PublicKey publicKey() { return publickey; }

	// To be removed:
	public PublicKey publickey() { return publickey; }

	public String
	toString()
	{
		return "Public Key Certificate: " + super.toString() + "\n" + publickey.toString();
	}
}
