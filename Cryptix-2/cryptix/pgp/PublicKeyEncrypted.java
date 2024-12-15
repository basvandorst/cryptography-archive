/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */

package cryptix.pgp;

/**
 *
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
import cryptix.math.BigInteger;
import cryptix.math.MPI;
import cryptix.math.RandomStream;
import cryptix.security.rsa.PublicKey;
import cryptix.security.rsa.SecretKey;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.io.EOFException;
import java.util.Vector;

public final class PublicKeyEncrypted extends Packet
{
	private BigInteger enckey;
	private KeyID keyId;
	
	public PublicKeyEncrypted( PublicKey publickey, byte Ideakey[], RandomStream rand )
	{
		keyId = new KeyID( publickey.id() );
                enckey = HashFactory.bigIntFromHash( publickey, Ideakey, rand );
	}
	
	public PublicKeyEncrypted( DataInput in, int length )
		throws IOException
	{
		super( in, length );
	}

	public void 
	read( DataInput in, int length )
		throws IOException
	{
		in.readByte(); // version.
		byte[] keyIdBytes = new byte[8];
		in.readFully( keyIdBytes ); // key id;
		keyId = new KeyID( keyIdBytes );
		if ( in.readByte() != 0x01 ) // RSA algorythm identifier.
			throw new FormatException( "Incorrect algorithm identifier." );
		try
		{
			enckey = MPI.read( in );
		}
		catch ( EOFException e )
		{
			throw new IOException("MPI too long.");
		}
	}

	public int
	write( DataOutput out )
		throws IOException
	{
		out.write( 0x02 );
		out.write( keyId.toByteArray() );		
		out.write( 0x01 );
		byte tmp[] = MPI.save( enckey );
		out.write( tmp );
		return tmp.length + 10;
	}
	
	/**
	 * Returns the ID of the secret key needed to decrypt this packet
	 * Can be used with SecretKeyRing.getKey( KeyID keyID, Passphrase passphrase )
	 */
	public KeyID
	secretKeyID()
	{
		return keyId;
	}

        public BigInteger getEncryptionKey()
        {
            return enckey;
        }

	public 	byte[]
	getKey( SecretKey key )
	{
		byte buf[] = key.decrypt( enckey ).toByteArray();

		int sum = 0;
		for ( int i = buf.length - 18; i < buf.length-2; i++)
			sum += buf[i] & 0xFF;
		int check = ( ( buf[buf.length-2] & 0xFF ) << 8 ) + ( buf[buf.length-1] & 0xFF );
		if (sum != check)
			throw new Error( "sum mismatch sum:" + sum + " check:" + check );
		byte ideaKey[] = new byte[16];
		System.arraycopy( buf, buf.length - 18, ideaKey, 0, 16 );
		return ideaKey;
	}
	
	public String
	toString()
	{
		return "Public key encrypted packet - number: " + enckey.toString();
	}

    public int getType()
    {
        return Packet.PUBLIC_KEY_ENCRYPTED;
    }

}
