/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */

package cryptix.util;

import java.io.IOException;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import cryptix.util.ByteArray;

/**
 * static class that reads and write message byte arrays.
 */
public final class ByteArrayFactory
{
	private ByteArrayFactory()
	{
	}
	
	public static final byte[]
	load( byte buf[] )
	throws IOException
	{
		return load( buf, 0 );
	}

	public static final byte[]
	load( byte buf[], int offset )
	throws IOException
	{
		DataInputStream in = new DataInputStream( new ByteArrayInputStream( buf ) );
		if ( offset > 0 )
			in.skip( offset );
		return read( in );
	}

	public static final byte[]
	save( byte data[] )
	throws IOException
	{
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		write( new DataOutputStream( out ), data );
		return out.toByteArray();
	}
	
	private static final int SIGN_MASK = 0xFF;
	private static final int LENGTH_MASK = 0x7F;
	private static final int CONTINUE_MASK = 0x80;
	private static final int SHIFT_SIZE = 7;
	private static final int MAX_SHIFT = 3;
	
	public static final byte[]
	read( DataInput in )
	throws IOException
	{
		int data = in.readByte() & SIGN_MASK;
//		System.err.println( "Data read : 0x" + Integer.toString( data, 16 ) );
		int length = data & LENGTH_MASK;
//		System.err.println( "new length: " + Integer.toString( length ) );
		while( ( data & CONTINUE_MASK ) != 0 )
		{
			data = in.readByte() & SIGN_MASK;
//			System.err.println( "Data read : 0x" + Integer.toString( data, 16 ) );
			length <<= SHIFT_SIZE;
			length |= data & LENGTH_MASK;
//			System.err.println( "new length: " + Integer.toString( length ) );
		}
//		System.err.println( "length : " + Integer.toString( length ) + " : 0x" + Integer.toString( length, 16 ) );
		byte buffer[] = new byte [length];
		if ( length > 0 )
			in.readFully( buffer );
		return buffer;
	}

	public static final void
	write( DataOutput out, ByteArray data )
		throws IOException
	{
		write( out, data.toByteArray() );
	}

	public static final void
	write( DataOutput out, byte data[] )
	throws IOException
	{
		int length = data.length;
//		System.err.println( "length : " + Integer.toString( length ) + " : 0x" + Integer.toString( length, 16 ) );
		int shift = MAX_SHIFT;
		int mask = LENGTH_MASK << ( SHIFT_SIZE * shift );
		while ( ( ( length & mask ) == 0 ) && ( shift > 0 ) )
			mask = LENGTH_MASK << ( SHIFT_SIZE * --shift );
		while( shift > 0 )
		{
			out.write( ( ( length & mask ) >>> ( SHIFT_SIZE * shift ) ) | CONTINUE_MASK );
			mask = LENGTH_MASK << ( SHIFT_SIZE * --shift );
		}
		out.write( length & LENGTH_MASK );
		if ( length > 0 )
			out.write( data );
	}

	public static final String
	toHexString( byte buf[] )
	{
		return toHexString( buf, 0, buf.length );
	}

	public static String
	toHexString( byte buf[], int ptr, int len )
	{
		StringBuffer sb = new StringBuffer( len * 4 );
		for ( ; len > 0; len-- )
		{
			byte b = buf[ptr++];
			sb.append( hexChar( ( b >>> 4 ) & 0xF ) ).append( hexChar( b & 0xF ) );
		}
		return sb.toString();
	}

	public static final String
	toHexdumpString( byte buf[] )
	{
		return toHexdumpString( buf, 0,  buf.length );
	}
	
	public static String
	toHexdumpString( byte buf[], int ptr, int len )
	{
		StringBuffer sb = new StringBuffer( len * 4 );
		sb.append( "Byte array length:" ).append( len ).append( '\n' );
		while ( len > 16 )
		{
			for ( int shft = 12; shft >= 0; shft -= 4 )
				sb.append( hexChar( ( ptr >>> shft ) & 0xF ) );
			sb.append( ' ' );
			for ( int i = 0; i < 16; i++ )
				sb.append( ascii( buf[ptr + i] ) );
			sb.append( ' ' );
			for ( int i = 0; i < 16; i++ )
			{
				byte b = buf[ptr + i];
				sb.append( hexChar( ( b >>> 4 ) & 0xF ) ).append( hexChar( b & 0xF ) ).append( ' ' );
			}
			sb.append( '\n' );
			ptr+=16;
			len-=16;
		}

		if ( len > 0 )
		{
			for ( int shft = 12; shft >= 0; shft -= 4 )
				sb.append( hexChar( ( ptr >>> shft ) & 0xF ) );
			sb.append( ' ' );
			int i;
			for( i = 0; i < len; i++ )
				sb.append( ascii( buf[ptr + i] ) );
			for( ; i < 17; i++ ) // N.B. this includes the separating space.
				sb.append( ' ' );
			for( i = 0; i < len; i++ )
			{
				byte b = buf[ptr + i];
				sb.append( hexChar( ( b >>> 4 ) & 0xF ) ).append( hexChar( b & 0xF ) ).append( ' ' );
			}
			sb.append( '\n' );
		}
		return sb.toString();
	}

	public static final byte[]
	fromHexString( String str )
	{
		int len=str.length();
		if ( len == 0 )
			return new byte[0];
		int pos =0;
		byte buffer[] = new byte [( ( len + 1 ) / 2 )];
		if ( ( len % 2 ) == 1 )
		{
			buffer[0]=(byte)asciiToHex(str.charAt(0));
			pos=1;
			len--;
		}

		for(int ptr = pos; len > 0; len -= 2 )
			buffer[pos++] = (byte)( 
					( asciiToHex( str.charAt( ptr++ ) ) << 4 ) |
					( asciiToHex( str.charAt( ptr++ ) ) )
					);
		return buffer;
	}

	private static char
	ascii( byte i )
	{
		if ( i > 31 )
			return (char)i;
		return '.';
	}

	private static char
	hexChar( int i )
	{
		if ( i >= 0 )
		{
			if ( i < 10 )
				return (char)( '0' + i );
			if ( i < 16 )
				return (char)( 'A' - 10 + i  );
		}
		return '?';
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
		throw new RuntimeException( "ascii to hex failed" );
	}

}
