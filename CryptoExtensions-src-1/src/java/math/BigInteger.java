/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package java.math;

/**
 * A class implementing arbitrary length integers.
 * <p>The native code BigNum implementation used by the class was
 * developed by, and is owned by, Eric Young (eay@mincom.oz.au)
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.00, 11/12/1995
 * @author      Systemics (Michael Wynn)
 */

public class BigInteger extends BigNum
{
	
	public BigInteger()
	{
		super();
	}

	public BigInteger( BigInteger from )
	{
		super();
		copy( this, from );
	}

	public BigInteger(String hexString)
	{
		super();
		fromString( hexString );
	}

	public BigInteger( byte buffer[] )
	{
		super();
		fromByteArray( buffer );
	}

	public Object
	clone()
	{
		return new BigInteger( this );
	}
	
	public void
	copy( Object src )
	{
		copy( this, (BigInteger)src );
	}
	
	public void 
	fromString(String inHex)
	{
		int len=inHex.length();
		if ( len == 0 )
		{
			zero( this );
			return;
		}	
		int pos =0;
		byte buffer[] = new byte [( ( len + 1 ) / 2 )];
		if ( ( len % 2 ) == 1 )
		{
			buffer[0]=(byte)asciiToHex(inHex.charAt(0));
			pos=1;
			len--;
		}

		for(int ptr = pos; len > 0; len -= 2 )
			buffer[pos++] = (byte)( 
					( asciiToHex( inHex.charAt( ptr++ ) ) << 4 ) |
					( asciiToHex( inHex.charAt( ptr++ ) ) )
					);
		fromBinary( buffer );
	}
	
	public String
	toString()
	{
		byte buffer[] = toByteArray();

		StringBuffer returnBuffer = new StringBuffer();
		
		int pos = 0, len = buffer.length;
		// remove leading 0`s.
		while ( ( pos < len ) && ( buffer[pos] == 0 ) )
			pos++;
		if ( ( pos < len ) && ( ( ( buffer[pos] >>> 4 ) & 0x0F ) == 0 ) )
		{
			returnBuffer.append( buffer[0] & 0x0F );
			pos++;
		} 
		for (; pos<len; pos++)
			returnBuffer.append( hexToAscii( ( buffer[pos] >>> 4 ) & 0x0F ) )
						.append( hexToAscii( buffer[pos] & 0x0F ) );
		if ( returnBuffer.length() > 0 )
			return returnBuffer.toString();
		return "0";
	}
	
	protected void
	finalize()
	{
		super.finalize();
	}

	public BigInteger
	modExp(BigInteger power, BigInteger modulo)
	{
		modExp( this, this, power, modulo );
		return this;
	}

	public BigInteger
	mul( BigInteger a, BigInteger b )
	{
		mul( this, a, b );
		return this;
	}

	public int
	cmp( BigInteger a )
	{
		return cmp( this, a );
	}

	public BigInteger
	inc()
	{
		inc(this);
		return this;
	}

	public BigInteger
	dec()
	{
		dec(this);
		return this;
	}

	/**
	 * An internal function to create a buffer big enough in which
	 * to store a number.
	 * @param bitLen The number of <b>bits</b> in the number.
	 * @return a buffer in which to store a number.
	 */
	protected static final byte[]
	newBuffer( int bitLen )
	{
		return new byte[( bitLen + 7 ) / 8];
	}

	/**
	 * Convert a number into a byte array
	 * @return a byte array
	 */
	public byte[]
	toByteArray()
	{
		byte buffer[] = newBuffer( bitLength() );
		intoBinary( buffer );
		return buffer;
	}

	public void
	fromByteArray( byte buf[] )
	{
		fromBinary( buf );
	}

	public int
	numberOfBits()
	{
		return bitLength();
	}
		
	private static final int
	asciiToHex(char c)
	{
		if ( ( c >= 'a' ) && ( c <= 'f' ) )
			return ( c - 'a' + 10 );
		if ( ( c >= 'A' ) && ( c <= 'F' ) )
			return ( c - 'A' + 10 );
		if ( ( c >= '0' ) && ( c <= '9' ) )
			return ( c - '0' );
		throw new MathError("ascii to hex failed");
	}

	private static char
	hexToAscii(int h)
	{
		if ( ( h >= 10 ) && ( h <= 15 ) )
			return (char)( 'A' + ( h - 10 ) );
		if ( ( h >= 0 ) && ( h <= 9 ) )
			return (char)( '0' + h );
		throw new MathError("hex to ascii failed");
	}
}

