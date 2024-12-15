package cryptix.examples;

import cryptix.security.MD5;

public final class TestMD5
{
	public static final void
	main( String argv[] )
	{
		try
		{
			myMain( argv );
		}
		catch( Throwable t )
		{
			t.printStackTrace();
		}
	}

	private static void
	myMain( String argv[] )
	throws Exception
	{
	 	test( "", "d41d8cd98f00b204e9800998ecf8427e" );
		test( "a", "0cc175b9c0f1b6a831c399e269772661" );
		test( "abc", "900150983cd24fb0d6963f7d28e17f72" );
		test( "message digest", "f96b697d7cb7938d525a2f31aaf161d0" );
		test( "abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b" );
		test( 
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
			"d174ab98d277d9f5a5611c2c9f419d9f"
			);
		test( 
			"12345678901234567890123456789012345678901234567890123456789012345678901234567890",
			"57edf4a22be3c955ac49da2e2107b67a"
		);
	}

	private static void
	test( String msg, String hashStr )
	{
		hashStr = hashStr.toUpperCase();
		MD5 md5 = new MD5();
		md5.add( msg );
		String x = toString( md5.digest() );
		System.out.println( "Message " + msg );
		System.out.println( "calc hash:" + x );
		System.out.println( "real hash:" + hashStr );
		if ( hashStr.equals( x ) )
			System.out.println( "Good" );
		else
			System.out.println( "************* MD5 FAILED **************" );
	}

	private static byte[]
	fromString( String inHex )
	{
		int len=inHex.length();
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
		return buffer;
	}

	private static final String
	toString( byte buffer[] )
	{
		StringBuffer returnBuffer = new StringBuffer();
		for ( int pos = 0, len = buffer.length; pos < len; pos++ )
			returnBuffer.append( hexToAscii( ( buffer[pos] >>> 4 ) & 0x0F ) )
						.append( hexToAscii( buffer[pos] & 0x0F ) );
		return returnBuffer.toString();
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
		throw new Error("ascii to hex failed");
	}

	private static char
	hexToAscii(int h)
	{
		if ( ( h >= 10 ) && ( h <= 15 ) )
			return (char)( 'A' + ( h - 10 ) );
		if ( ( h >= 0 ) && ( h <= 9 ) )
			return (char)( '0' + h );
		throw new Error("hex to ascii failed");
	}

}

/*
Left as an excersize for the reader ... 
msg  1 ""
hash 1 "d41d8cd98f00b204e9800998ecf8427e"
msg  2 "a"
hash 2 "0cc175b9c0f1b6a831c399e269772661"
msg  3 "abc"
hash 3 "900150983cd24fb0d6963f7d28e17f72"
msg  4 "message digest"
hash 4 "f96b697d7cb7938d525a2f31aaf161d0"
msg  5 "abcdefghijklmnopqrstuvwxyz"
hash 5 "c3fcd3d76192e4007dfb496cca67e13b"
msg  6 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
hash 6 "d174ab98d277d9f5a5611c2c9f419d9f"
msg  7 "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
hash 7 "57edf4a22be3c955ac49da2e2107b67a"
*/
