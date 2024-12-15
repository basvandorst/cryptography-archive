/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */

package cryptix.pgp;

import cryptix.math.BigInteger;
import cryptix.math.MPI;
import cryptix.math.RandomStream;
import cryptix.security.MD5;
import cryptix.security.MessageHash;
import cryptix.security.rsa.SecretKey;
import cryptix.security.rsa.PublicKey;
import cryptix.util.ByteArrayFactory;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Date;

/**
 * @author Original author not stated
 * @author Jill Baker
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
public final class Signature extends Packet
{
        // Signature types
        public static final int BINARY_IMAGE                   = 0x00;
        public static final int CANONICAL_TEXT                 = 0x01;
        public static final int KEY_CERT_GENERIC               = 0x10;
        public static final int KEY_CERT_PERSONA               = 0x11;
        public static final int KEY_CERT_CASUAL_ID             = 0x12;
        public static final int KEY_CERT_POSITIVE_ID           = 0x13;
        public static final int KEY_COMPROMISE_CERTIFICATE     = 0x20;
        public static final int USER_ID_REVOCATION_CERTIFICATE = 0x30;
        public static final int TIMESTAMP_CERTIFICATE          = 0x40;

	private BigInteger number;
	private byte extra[];
	private byte keyId[];
        private int  pkeAlgorithm  = Algorithm.RSA;
        private int  hashAlgorithm = Algorithm.MD5;
	private byte hashId[];

        public Signature( SecretKey key, MD5 md )
        {
                this( key, md, null );
        }

        public Signature( SecretKey key, MD5 md, RandomStream rand )
	{
		int unixtime = (int)(System.currentTimeMillis()/1000);
		extra = new byte[5];
                extra[0] = (byte) BINARY_IMAGE; // binary data.
		extra[1] = (byte)( (unixtime >>> 24 ) & 0xFF );
		extra[2] = (byte)( (unixtime >>> 16 ) & 0xFF );
		extra[3] = (byte)( (unixtime >>> 8 ) & 0xFF );
		extra[4] = (byte)( unixtime & 0xFF );
		md.add( extra );
		byte hash[] = md.digest();
                BigInteger tmp = HashFactory.bigIntFromHash( key, hash, rand );
		number = key.decrypt( tmp ); // signing is encrypting with secret therefore normal decryption.
		hashId = new byte[2];
		hashId[0] = hash[0];
		hashId[1] = hash[1];
		keyId = key.id();
	}

	public Signature( DataInput in, int length )
	throws IOException
	{
		super( in, length );
	}

	public void
	read( DataInput in, int length )
	throws IOException
	{
		in.readByte(); // version
		int extraLen = in.readByte() & 0xFF;

		if ( extraLen != 5 && extraLen != 7 )
			throw new FormatException( "The length of the extra data field is incorrect." );

		in.readFully( extra = new byte[extraLen] );
		in.readFully( keyId = new byte[8] );

                pkeAlgorithm   = in.readByte();
                hashAlgorithm = in.readByte();

		in.readFully( hashId = new byte[2] );

		number = MPI.read( in );
	}

	public int
	write( DataOutput out )
	throws IOException
	{
		out.write( 0x02 );
		out.write( extra.length );
		out.write( extra );
		out.write( keyId );
                out.write( pkeAlgorithm ); 
                out.write( hashAlgorithm );
		out.write( hashId );
		byte tmp[] = MPI.save( number );
		out.write( tmp );
		return extra.length + tmp.length + 14;
	}

	public String
	typeString()
	{
		switch ( extra[0] )
		{
                case BINARY_IMAGE:
                        return "Binary image";
                case CANONICAL_TEXT:
			return "Text image";
                case KEY_CERT_GENERIC:
			return "Key cert";
                case KEY_CERT_PERSONA:
                        return "Key certificate anon";
                case KEY_CERT_CASUAL_ID:
                        return "Key certificate casual";
                case KEY_CERT_POSITIVE_ID:
                        return "Key certificate heavy duty";
                case KEY_COMPROMISE_CERTIFICATE:
                        return "Key compromise certificate";
                case USER_ID_REVOCATION_CERTIFICATE:
                        return "User ID revocation certificate";
                case TIMESTAMP_CERTIFICATE:
                        return "Time stamp certificate";
		}
		return " unknown type (" + extra[0] + ")";
	}

	public String
	toString()
	{
		return "Signature packet " + typeString() + " key id : " + ByteArrayFactory.toHexString( keyId );
	}

	private boolean check( PublicKey key, byte filehash[] )
	{
//System.out.println("in  KeyId:" + ByteArrayFactory.toHexString( key.id() ) );
//System.out.println("sig KeyId:" + ByteArrayFactory.toHexString( keyId ) );
//System.out.println("Hash:" + ByteArrayFactory.toHexString( filehash ) );
		if ( ( filehash[0] != hashId[0] ) || ( filehash[1] != hashId[1] ) )
			return false;

		byte id[] = key.id();
		int len = id.length;
		if ( len == keyId.length )
		{
			int i;
			for ( i = 0; i < len; i++ )
				if ( id[i] != keyId[i] )
					return false;
		}

//		hash id and key id both match now check the real Hashes.

		BigInteger hashFromPacket = key.encrypt( number );
                BigInteger hashFromFile = HashFactory.bigIntFromHash( key, filehash );

//System.out.println( "hash " + hashFromFile );
//System.out.println( "number " + hashFromPacket );

		return hashFromPacket.equals( hashFromFile );
	}

	public boolean
	check( PublicKey key, MD5 md )
	{
		md.add( extra );
		return check( key, md.digest() );
	}

	public boolean
	verify( PublicKey key, byte[] data )
	{
		MD5 md = new MD5();
		md.add( data );
		md.add( extra );
		return check( key, md.digest() );
	}

	public byte[]
	checkAndGetHash( PublicKey key, MD5 md )
	{
		md.add( extra );
		byte hash[] = md.digest();
		if ( !check( key, hash ) )
			return null;
		return hash;
	}

	// use with caution.
	public void
	addExtasToHash( MD5 md )
	{
		md.add( extra );
	}

        public int getSignatureType()
        {
                return extra[0];
        }

	public boolean
	check( PublicKey key, MessageHash completeHash )
	{
		return check( key, completeHash.toByteArray() );
	}

	public byte[]
	keyId()
	{
 		byte tmp[];
 		int len = keyId.length;
		System.arraycopy( keyId, 0, tmp = new byte[len], 0, len );
		return tmp;
	}

	public KeyID
	getKeyId()
	{
                return new KeyID( keyId );
	}

    public int getType()
    {
        return Packet.SIGNATURE;
    }
}
