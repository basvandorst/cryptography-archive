/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */

//
//	This module still has IDEA hard coded all over the place
//	This must be fixed
//

package cryptix.pgp;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import cryptix.math.BigInteger;
import cryptix.math.MPI;
import cryptix.math.RandomStream;
import cryptix.security.StreamCipher;
import cryptix.security.MD5;
import cryptix.security.IDEA;
import cryptix.security.rsa.SecretKey;

import cryptix.util.Streamable;
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
final class EncryptedBigInt extends Streamable
{
	private byte buf_[];
	private int bitLen_;
	private int checksum_;

	EncryptedBigInt( DataInput in )
		throws IOException
	{
		super( in );
	}
	
	EncryptedBigInt( BigInteger num, StreamCipher cipher )
	{
		buf_ = num.toByteArray();
		bitLen_ = num.bitLength();
		int len = buf_.length;
		int sum = 0;
		for (int i = len - 1; i >= 0; i-- )
			sum += ( buf_[i] & 0xFF );
		checksum_ = sum & 0xFFFF;
		cipher.encrypt( buf_ );
	}
	
	BigInteger
	decrypt( StreamCipher cipher )
	{
		int len = buf_.length;
		byte tmp[] = new byte[len];
		cipher.decrypt( buf_, tmp );
		
		int sum = 0;
		for (int i = len - 1; i >= 0; i-- )
			sum += ( tmp[i] & 0xFF );

		checksum_ = sum & 0xFFFF;
	
		return new BigInteger( tmp );
	}
	
	public void
	read( DataInput in )
		throws IOException
	{
		bitLen_ = in.readShort();
		buf_ = new byte[(bitLen_ + 7) / 8];
		in.readFully( buf_ );
	}

	public void
	write( DataOutput out )
		throws IOException
	{
		out.writeShort( bitLen_ );
		out.write( buf_ );
	}
	
	public int
	checksum()
	{
		return ( ( bitLen_ & 0xFF ) + ( ( bitLen_ >> 8 ) & 0xFF ) + checksum_ ) & 0xFFFF;
	}
}

final class EncryptedSecretKey extends Streamable
{
	private BigInteger e_, n_;
	private EncryptedBigInt d_, p_, q_, u_;
	private byte iv_[] = null;
	private int checksum_;

	public EncryptedSecretKey( SecretKey sk, Passphrase passphrase, RandomStream rs )
		throws IOException
	{
                IDEA block_cipher = new IDEA( passphrase.md5hash() );
		CFB cipher = new CFB(block_cipher);

		n_ = sk.n();
		e_ = sk.e();

		rs.read( iv_ = new byte[block_cipher.blockLength()] );
		cipher.encrypt(iv_);

		d_ = new EncryptedBigInt( sk.d(), cipher );
		p_ = new EncryptedBigInt( sk.p(), cipher );
		q_ = new EncryptedBigInt( sk.q(), cipher );
		u_ = new EncryptedBigInt( sk.u(), cipher );

		checksum_ = ( d_.checksum() + p_.checksum() + q_.checksum() + u_.checksum() ) & 0xFFFF;
	}

	public EncryptedSecretKey( byte buf[] )
		throws IOException
	{
		load( buf );
	}

	public EncryptedSecretKey( DataInput in )
		throws IOException
	{
		read( in );
	}

	public SecretKey
	secretKey( StreamCipher cipher )
		throws DecryptException
	{
		cipher.decrypt( iv_, new byte[iv_.length] );

		BigInteger d = d_.decrypt( cipher );
		BigInteger p = p_.decrypt( cipher );
		BigInteger q = q_.decrypt( cipher );
		BigInteger u = u_.decrypt( cipher );

		if ( ( ( ( d_.checksum() + p_.checksum() + q_.checksum() + u_.checksum() ) ) & 0xFFFF ) != checksum_ )
			throw new DecryptException( "Check sum failure: passphrase wrong." );
		return new SecretKey( n_, e_, d, p, q, u );
	}
		
	public void read(DataInput in)
		throws IOException
	{
		n_ = MPI.read( in );
		e_ = MPI.read( in );

		int encrypted = in.readByte();
		if ( encrypted != 0 )
		{
			if ( encrypted != 1 ) // IDEA
				throw new FormatException( "Unknown cipher type" );

			in.readFully( iv_ = new byte [8] ); // IV for cipher.

			d_ = new EncryptedBigInt( in );
			p_ = new EncryptedBigInt( in );
			q_ = new EncryptedBigInt( in );
			u_ = new EncryptedBigInt( in );

			checksum_ = in.readShort() & 0xFFFF;
		}
		else
			throw new FormatException( "Found unencrypted SecretKey - unencrypted secret keys are not supported." );
	}
	
	public void write( DataOutput out )
		throws IOException
	{
		MPI.write( out, n_ );
		MPI.write( out, e_ );
		out.write( 1 );
		out.write( iv_ );

		d_.write( out );
		p_.write( out );
		q_.write( out );
		u_.write( out );

		out.writeShort( checksum_ );
	}

	/**
	 * gets the key id. Needed to index secret key certificates by ID.
	 * @returns The lower 8 bytes of n, the public modulus.
	 */
	public final KeyID
	id()
	{
		byte nBuffer[] = n_.toByteArray();
		byte ident[] = new byte[8];
		System.arraycopy( nBuffer, nBuffer.length - 8, ident, 0, 8 );
		return new KeyID( ident );
	}

	
	public String
	toString()
	{
		return "secret key public parts n:" + n_.toString() + " e:"+ e_.toString() + " all the rest is encrypted";
	}
}

public final class SecretKeyCertificate extends Certificate 
{
	private EncryptedSecretKey key;
	
	public SecretKeyCertificate( SecretKey key0, Passphrase passphrase, RandomStream rand )
		throws IOException
	{
		key = new EncryptedSecretKey( key0, passphrase, rand );
	}

	public SecretKeyCertificate( DataInput in, int length )
		throws IOException
	{
		super( in, length );
	}

	public void
	read( DataInput in, int length )
		throws IOException
	{
		super.read( in, length );
		
		if ( in.readByte() != 0x01 )
			throw new FormatException( "Bad algorithm byte (not RSA)." );

		key = new EncryptedSecretKey( in );
	}
	
	public int
	write( DataOutput out )
		throws IOException
	{
		int len = super.write( out ) + 1;
		out.write( 0x01 );
		byte tmp[];
		out.write( tmp = key.save() );
		len += tmp.length;
		return len;
	}

	public SecretKey
	getSecretKey( Passphrase passphrase )
		throws DecryptException
	{
                CFB cipher = new CFB( new IDEA( passphrase.md5hash() ) );
		return key.secretKey( cipher );
	}

	/**
	 * gets the key id.
	 * @returns The key in the certificate's ID
	 */
	public final KeyID
	id()
	{
		return key.id();
	}

	public String
	toString()
	{
		return "Secret key Cert " + super.toString();
	}

    public int getType()
    {
        return Packet.SECRET_KEY;
    }

}
