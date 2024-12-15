
import java.io.FileInputStream;
import java.io.IOException;

import java.crypt.MD5;

/**
 * This is a demo of how to use the MD5 or SHA classes for hashing data.
 *
 * Written by Systemics Ltd (http://www.systemics.com/).
 * Enjoy!
 */
public final class MD5AFile
{
	private static final int BUF_LENGTH = 1024;
	
	public static void
	main( String argv[] )
	{
		if ( argv.length != 1 )
			System.err.println( "usage java MD5Afile filename" );
		else
			try
			{
				printHash( doHash( argv[0] ) );
			}
			catch( IOException ioe )
			{
				System.err.println( "There has been an IO exception to the file was not hashed." );
				ioe.printStackTrace();
			}
	}
	
	/**
	 * This function opens a file and returns the hash
	 * @param filename The name of the file to hash.
	 * @return The hash of the lines of the file.
	 */
	private static byte[]
	doHash( String filename )
	throws IOException
	{
		FileInputStream in = new FileInputStream( filename );
		MD5 md = new MD5();
		byte buf[] = new byte[BUF_LENGTH];
		// read all the lines in the file.
		int position = 0;
		while( true )
		{
			int data = in.read();
			if ( data == -1 ) // eof
				break;
			buf[position++] = (byte)data;
			if ( position == BUF_LENGTH )
			{
				position = 0;
				md.add( buf );
			}
		}
		if ( position != 0 )
			md.add( buf, 0, position );
		// close the file.
		in.close();
		return md.digest();
	}
	

	/**
	 * This function prints out the hash it is given.
	 * @param buf The hash to print.
	 */
	private static void
	printHash( byte buf[] )
	{
		System.out.println( "hash of file is;" );
		System.out.print( "MD5 : " );
		for ( int i = 0, j = buf.length; j > 0; i++, j-- )
		{
			int val = (int)buf[i];
			System.out.print( Integer.toString( ( val >> 4 ) & 0xF, 16 ) );
			System.out.print( Integer.toString( val & 0xF, 16 ) );
		}
		System.out.println();
	}
}

