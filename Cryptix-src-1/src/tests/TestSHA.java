import java.crypt.SHA;

public final class TestSHA
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
		test( "abc", "0164b8a914cd2a5e74c4f7ff082c4d97f1edf880" );
		test( 
			"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			"d2516ee1acfa5baf33dfc1c471e438449ef134c8"
			);
	}

	private static void
	test( String msg, String hashStr )
	{
		hashStr = hashStr.toUpperCase();
		SHA sha = new SHA();
		sha.add( msg );
		String x = toString( sha.digest() );
		System.out.println( "Message " + msg );
		System.out.println( "calc hash:" + x );
		System.out.println( "real hash:" + hashStr );
		if ( hashStr.equals( x ) )
			System.out.println( "Good" );
		else
			System.out.println( "************* SHA FAILED **************" );
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

msg  1 "abc"
hash 1 "0164b8a914cd2a5e74c4f7ff082c4d97f1edf880"

msg  2 "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
hash 2 "d2516ee1acfa5baf33dfc1c471e438449ef134c8"
*/
