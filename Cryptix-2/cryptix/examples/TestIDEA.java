package cryptix.examples;

import cryptix.security.IDEA;

public final class TestIDEA
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
		test( "00010002000300040005000600070008", "0000000100020003", "11FBED2B01986DE5" );
		test( "0005000A000F00140019001E00230028", "0102030405060708", "3EC04780BEFF6E20" );
		test( "3A984E2000195DB32EE501C8C47CEA60", "0102030405060708", "97BCD8200780DA86" );
		test( "006400C8012C019001F4025802BC0320", "05320A6414C819FA", "65BE87E7A2538AED" );
		test( "9D4075C103BC322AFB03E7BE6AB30006", "0808080808080808", "F5DB1AC45E5EF9F9" );
	}
	
	private static void
	test( String keyStr, String plainStr, String cipherStr )
	{
		byte key[] = fromString( keyStr );
		byte plain[] = fromString( plainStr );
		byte cipher[] = fromString( cipherStr );
		IDEA idea = new IDEA( key );
		byte encP[] = new byte[plain.length];
		byte decC[] = new byte[plain.length];
		idea.encrypt( plain, encP );
		String a,b;
		System.out.println( "plain:" + toString( plain ) + " enc:" + ( a = toString( encP ) ) + " real:" + ( b = toString( cipher) ) );
		if ( a.equals( b ) )
			System.out.println( "encryption good" );
		else
			System.out.println( " ********* IDEA ENC FAILED ********* " );
		idea.decrypt( encP, decC );
		System.out.println( "  enc:" + toString( encP ) + " dec:" + ( a = toString( decC ) ) + " real:" + ( b = toString( plain ) ) );
		if ( a.equals( b ) )
			System.out.println( "decryption good" );
		else
			System.out.println( " ********* IDEA DEC FAILED ********* " );
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
# 00010002000300040005000600070008  0000000100020003  11FBED2B01986DE5
# 00010002000300040005000600070008  0102030405060708  540E5FEA18C2F8B1
# 00010002000300040005000600070008  0019324B647D96AF  9F0A0AB6E10CED78
# 00010002000300040005000600070008  F5202D5B9C671B08  CF18FD7355E2C5C5
# 00010002000300040005000600070008  FAE6D2BEAA96826E  85DF52005608193D
# 00010002000300040005000600070008  0A141E28323C4650  2F7DE750212FB734
# 00010002000300040005000600070008  050A0F14191E2328  7B7314925DE59C09
# 0005000A000F00140019001E00230028  0102030405060708  3EC04780BEFF6E20
# 3A984E2000195DB32EE501C8C47CEA60  0102030405060708  97BCD8200780DA86
# 006400C8012C019001F4025802BC0320  05320A6414C819FA  65BE87E7A2538AED
# 9D4075C103BC322AFB03E7BE6AB30006  0808080808080808  F5DB1AC45E5EF9F9
*/
