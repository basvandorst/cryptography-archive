import java.crypt.Blowfish;

public final class TestBlowfish
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
		test( "6162636465666768696a6b6c6d6e6f707172737475767778797a",
				"424c4f5746495348", "324ed0fef413a203" );

		test( "57686f206973204a6f686e2047616c743f",
				"fedcba9876543210", "cc91732b8022f684" );
	}

	private static void
	test( String keyStr, String plainText, String cipherText )
	{
		System.out.println( "Key string in : " + keyStr );
		System.out.println( "Key strg lenght : " + keyStr.length() );
		byte key[] = fromString( keyStr );
		System.out.println( "Key string out: " + toString( key ) );
		System.out.println( "Key lenght java : " + key.length );
		byte plain[] = fromString( plainText );
		byte cipher[] = fromString( cipherText );
		Blowfish bf = new Blowfish( key );
		byte encP[] = new byte[plain.length];
		byte decC[] = new byte[plain.length];
		bf.encrypt( plain, encP );
		check( toString( plain ), toString( encP ), toString( cipher) );
		bf.decrypt( encP, decC );
		check( toString( encP ), toString( decC ), toString( plain ) );
	}

	private static void
	check( String before, String after, String good )
	{
		System.out.println( " b4:" + before + " after:" + after + " check:" + good );
		if ( good.equals( after ) )
			System.out.println( "Good." );
		else
			System.out.println( "******** BAD EN/DECRYTION *****" );
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
#
#	Some tests...
#
Left as an excersize for the reader ... 
# KEY              PLAINTEXT        CIPHERTEXT
key = pack("H*", "6162636465666768696a6b6c6d6e6f707172737475767778797a");
in = pack("H*", "424c4f5746495348");
out = pack("H*", "324ed0fef413a203");

key = pack("H*", "57686f206973204a6f686e2047616c743f");
in = pack("H*", "fedcba9876543210");
out = pack("H*", "cc91732b8022f684");

*/
