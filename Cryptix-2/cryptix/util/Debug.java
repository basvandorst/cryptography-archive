/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package cryptix.util;

import java.io.PrintStream;

public final class Debug
{

	public static void println( byte buf[] , int len )
	{
		println( buf, len , 0 );
	}

	public static void println( byte buf[] , int len, int ptr )
	{
		println( System.err, buf, len, ptr );
	}

	public static void println( PrintStream err, byte buf[] , int len, int ptr )
	{
		while ( len > 16 )
		{
			err.print( hex( ptr ) + "  " );
			for (int i = 0; i < 16; i++)
				System.err.print( ascii( buf[ptr+i] ) );
			err.print( "  " );
			for (int i = 0; i < 16; i++)
				err.print( hex( buf[ptr+i] ) + " " );
			err.println( " " );
			ptr+=16;
			len-=16;
		}

		if ( len > 0 )
		{
			err.print( hex( ptr ) + "  " );
			int i;
			for( i = 0; i < len; i++ )
				err.print( ascii( buf[ptr+i] ) );
			for( ;i< 16;i++)
				err.print( " " );
			err.print( "  " );
			for( i = 0; i < len; i++ )
				err.print( hex( buf[ptr+i] ) + " " );
			err.println( " " );
		}
	}

	public static void print( byte buf[] )
	{
		int len = buf.length;
		System.err.println( "Byte array length:" + len );
		int pos = 0;
		while ( len > 16 )
		{
			System.err.print( hex( pos ) + "  " );
			for (int i=0; i<16; i++)
				System.err.print( ascii( buf[pos+i] ) );
			System.err.print( "  " );
			for (int i=0; i<16; i++)
				System.err.print( hex( buf[pos+i] ) + " " );
			System.err.println( " " );
			pos+=16;
			len-=16;
		}

		if ( len > 0 )
		{
			System.err.print( hex( pos ) + "  " );
			int i;
			for( i = 0; i < len; i++ )
				System.err.print( ascii( buf[pos+i] ) );
			for( ;i< 16;i++)
				System.err.print( " " );
			System.err.print( "  " );
			for( i = 0; i < len; i++ )
				System.err.print( hex( buf[pos+i] ) + " " );
			System.err.println( " " );
		}
		System.err.println( "Byte array End." );
		System.err.flush();
	}

	private static char ascii( byte i )
	{
		if ( i > 31 )
			return (char)i;
		return '.';
	}

	private static String hex( byte i )
	{
		return Integer.toString( ( i >>> 4 ) & 0x0F, 16 )+Integer.toString( i & 0x0F, 16 );
	}
	
	private static String hex( int i )
	{
		return 	Integer.toString( ( i >>> 28 ) & 0x0F, 16 ) + 
				Integer.toString( ( i >>> 24 ) & 0x0F, 16 ) + 
				Integer.toString( ( i >>> 20 ) & 0x0F, 16 ) + 
				Integer.toString( ( i >>> 16 ) & 0x0F, 16 ) + 
				Integer.toString( ( i >>> 12 ) & 0x0F, 16 ) + 				
				Integer.toString( ( i >>> 8 ) & 0x0F, 16 ) + 
				Integer.toString( ( i >>> 4 ) & 0x0F, 16 ) + 
				Integer.toString( i & 0x0F, 16 );
	}

	private static String hex( short i )
	{
		return 	Integer.toString( ( i >>> 12 ) & 0x0F, 16 ) + 				
				Integer.toString( ( i >>> 8 ) & 0x0F, 16 ) + 
				Integer.toString( ( i >>> 4 ) & 0x0F, 16 ) + 
				Integer.toString( i & 0x0F, 16 );
	}

}
