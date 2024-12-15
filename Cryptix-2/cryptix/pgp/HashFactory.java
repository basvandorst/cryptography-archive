/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package cryptix.pgp;

import cryptix.math.BigInteger;
import cryptix.math.RandomStream;
import cryptix.security.MessageHash;
import cryptix.security.MD5;
import cryptix.security.HashMD5;
import cryptix.security.SHA;
import cryptix.security.HashSHA;
import cryptix.security.rsa.PublicKey;
import cryptix.util.*;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInput;
import java.io.DataInputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.IOException;

/**
 * static class that reads and writes message hashes.
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
public final class HashFactory
{
	private static final int NONE_IDENT = 0;
	private static final int MD5_IDENT = 1;
	private static final int SHA_IDENT = 2;

	private static final byte PADDING[] =
	{
		(byte)0x00, (byte)0x30, (byte)0x20, (byte)0x30, (byte)0x0C, (byte)0x06, (byte)0x08, (byte)0x2A,
		(byte)0x86, (byte)0x48, (byte)0x86, (byte)0xF7, (byte)0x0D, (byte)0x02, (byte)0x05, (byte)0x05,
		(byte)0x00, (byte)0x04, (byte)0x10
	};

	private HashFactory()
	{
	}
	
	public static final MessageHash
	load( byte buf[] )
	throws IOException
	{
		return read( new DataInputStream( new ByteArrayInputStream( buf ) ) );
	}

	public static final byte[]
	save( MessageHash mh )
	throws IOException
	{
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		write( new DataOutputStream( out ), mh );
		return out.toByteArray();
	}
	
	public static final MessageHash
	read( DataInput in )
	throws IOException
	{
		int type;
		byte buf[];
		switch( type = in.readByte() & 0xFF )
		{
		case NONE_IDENT:
			return null;
		case MD5_IDENT:
			buf = new byte[MD5.HASH_LENGTH];
			in.readFully( buf );
			return new HashMD5( buf );
		case SHA_IDENT:
			buf = new byte[SHA.HASH_LENGTH];
			in.readFully( buf );
			return new HashSHA( buf );
		default:
			throw new IOException( "Hash type (" + type + ") not recognized" );
		}
	}

	public static final void
	write( DataOutput out, MessageHash mh )
	throws IOException
	{
		if ( mh == null )
			out.writeByte( NONE_IDENT );
		else
		{
			if ( mh instanceof HashMD5 )
				out.writeByte( MD5_IDENT );
			else if ( mh instanceof HashSHA )
				out.writeByte( SHA_IDENT );
			else
				throw new IOException( "Hash type (" + mh + ") not recognized" );
			out.write( mh.toByteArray() );
		}
	}
	
	public static final String
	toString( MessageHash mh )
	{
		if ( mh == null )
			return "NONE:";
		return mh.toString();
	}
	
	public static final MessageHash
	fromString( String str )
	{
		if ( str.startsWith( "MD5" ) )
			return new HashMD5( ByteArrayFactory.fromHexString( str.substring( 4 ) ) );
		else if( str.startsWith( "SHA" ) )
			return new HashSHA( ByteArrayFactory.fromHexString( str.substring( 4 ) ) );
		else if( str.startsWith( "NONE" ) )
			return null;
		else
			throw new RuntimeException( "string is not a hash" + str );
	}

	//
	//	Hack - can't have colons in DOS, remember?
	//
	public static final String
	toFilename( MessageHash mh )
	{
		if ( mh == null )
			return "MT";
		return mh.toString().replace( ':', '_' );
	}

	public static final BigInteger
        bigIntFromHash( PublicKey key, byte hash[], RandomStream rand )
	{
                if (rand == null) return bigIntFromHash( key, hash );

		if ( hash.length != 16 )
			throw new Error( "The code for hashes of lengths other than 16 is not in place yet." );
		int byteLength = key.bitLength() / 8;
		int fillLength = byteLength - ( 6 + hash.length );
		byte buffer[] = new byte[byteLength];
		int i=0;
		buffer[i++] =0;
		buffer[i++] =2;
		for (int count = fillLength; count > 0 ; count--)
		{
			byte fill;
			do		
				fill =(byte)rand.read();
			while ( fill == 0 );
			buffer[i++] = fill;
		}
		buffer[i++] =0;
		buffer[i++] =1;
		System.arraycopy( hash, 0, buffer, i, hash.length );
		int sum = 0;
		for (int count = hash.length - 1; count >= 0; count--)
			sum += hash[count] & 0xFF;
		i += hash.length;
		buffer[i++] = (byte)( ( sum >>> 8 ) & 0xFF );
		buffer[i++] = (byte)( sum & 0xFF );

		return key.encrypt( new BigInteger( buffer ) );
	}

	public static BigInteger bigIntFromHash( PublicKey key, byte hash[] )
	{
		if ( hash.length != 16 )
			throw new Error( "The code for hashes of lengths other than 16 is not in place yet." );
		int byteLength = key.bitLength() / 8;
		int fillLength = byteLength - ( 19 + hash.length ); // note that fillLength is 2 longer that the required number of FF's.
		byte buffer[] = new byte[byteLength];
		buffer[0] =0;
		buffer[1] =1;
		for (int i = 2; i < fillLength; i++)
			buffer[i] = (byte)0xFF;
		System.arraycopy( PADDING, 0, buffer, fillLength, 19 );
		System.arraycopy( hash, 0, buffer, fillLength + 19, hash.length );
		return new BigInteger( buffer );
	}

    public static final byte[]
    hashFromBigInt( BigInteger b )
    {
        byte[] temp = b.toByteArray();

        int start = temp.length - 18;
        if (start < 0)
            throw new Error( "The code for hashes of lengths other than 16 is not in place yet." );

        int sum = 0;
        for (int i=start; i<start+16; i++)
            sum += temp[i] & 0xFF;
        if ((sum >> 8)   != (temp[ start + 16 ] & 0xFF)) return null;
        if ((sum & 0xFF) != (temp[ start + 17 ] & 0xFF)) return null;

        byte[] hash = new byte[16];
        System.arraycopy( temp, start, hash, 0, 16 );
        return hash;
    }

}