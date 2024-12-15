/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */
package cryptix.pgp_examples;

import java.util.*;
import java.io.*;
import cryptix.security.*;
import cryptix.security.rsa.*;
import cryptix.math.*;

import cryptix.pgp.*;

/**
 * This class is a RSA key pair generator.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */
final class VanityRSAKeyGen extends RSAKeyGen
{
	public VanityRSAKeyGen( RandomStream rand0 )
	{
		super( rand0 );
	}

	public BigInteger
	randomPrime( int bitlen, long lsbFix, RSAKeyGenObserver obsrv )
	{
		byte buf[] = new byte[8];
		for( int i = 0; i< 8; i++ )
			buf[i] = (byte)( ( lsbFix >>> ( (7 - i) * 8 ) ) & 0xFF );
		return randomPrime( bitlen, buf, obsrv );
	}

	public BigInteger
	randomPrime( int bitlen, int lsbFix, RSAKeyGenObserver obsrv  )
	{
		byte buf[] = new byte[4];
		for( int i = 0; i< 4; i++ )
			buf[i] = (byte)( ( lsbFix >>> ( (3 - i) * 8 ) ) & 0xFF );
		return randomPrime( bitlen, buf, obsrv );
	}
	
	public BigInteger
	randomPrime( int bitlen, byte lsbFix[], RSAKeyGenObserver obsrv )
	{
		BigInteger ret = new BigInteger();
		int bytelen = 1 + ( bitlen / 8 ), offset = bitlen % 8;
		byte buf[] = new byte[bytelen];
		int last = lsbFix.length - 1;
		lsbFix[last] |= 1; // make sure its odd.
		
		while ( true )
		{
		// make big-endian random number.
			if ( rand.read( buf ) != bytelen )
				throw new Error( "Not using a java.math.RandomStream !!!" );

			for ( int ptr = 1, i = last; i >= 0 ; i--, ptr++ )
				buf[bytelen-ptr] = lsbFix[i];
		// set top bit, and clear the others.
			if ( offset == 0 )
			{
				buf[0] = 0;
				buf[1] |= (byte)0x80;
			}
			else
			{
				buf[0] &= (byte)( 0xFF >>> ( 8 - offset ) );
				buf[0] |= (byte)( 1 << ( offset - 1 ) );
			}
			ret.fromByteArray( buf );
			if ( TestPrime.isPrime( ret, obsrv ) )
				return ret;
		}
	}

	public cryptix.security.rsa.SecretKey
	createKey( int bitlen, int id, RSAKeyGenObserver obsrv )
	{
		int plen = bitlen/2; // PGP requires that p and q are the same length.
		int qlen = bitlen-plen;
		
		BigInteger p = randomPrime( plen, id, obsrv );
		System.out.println( "p:" + p);

		BigInteger q = randomPrime( qlen, 1, obsrv );
		System.out.println( "q:" + q );

		BigInteger e = new BigInteger( 17 );
		return deriveKeys( p, q, e );
	}
}

public final class VanityKeyGen extends KeyGen
{
	public VanityKeyGen()
	{
		super();
	}

	public static void
	main( String argv[] )
	{
		new VanityKeyGen().begin();
	}

	protected cryptix.security.rsa.SecretKey
	getKey( int length, RandomStream rs )
	{
		return new VanityRSAKeyGen( rs ).createKey( length, getEnding(), this );
	}

	private int
	getEnding()
	{
		while ( true )
		{
			String id;
			out.print( "Please enter key id in hex ( e.g., C001D00D )." );
			out.flush();
			try
			{
				id = in.readLine().trim();
				if ( id.length() != 8 )
				{
					out.println( "Key id must be eight chars long." );
					continue;
				}
				return Integer.parseInt( id, 16 );
			}
			catch ( IOException ioe )
			{
				throw new RuntimeException( "in stream has caused " + ioe.getClass().getName() + " : " + ioe.getMessage() );
			}
			catch ( NumberFormatException nfe )
			{
				out.println( "Unable to parse key ID." );
			}
		}
	}
}

