
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import java.util.Random;

import java.crypt.IDEA;
import java.crypt.MessageDigest;
import java.crypt.MD5;
import java.crypt.CipherFeedback;

/**
 * This is a demo of how to use the IDEA for encrypting data.
 * a file is opened and then en/de-crypted to stdout
 * do encrypt use java IDEAcryptAfile plaintext-filename XXX > ciphertext-filename
 * do decrypt use java IDEAcryptAfile -d ciphertext-filename XXX > plaintext-filename
 *
 * Written by Systemics Ltd (http://www.systemics.com/).
 * Enjoy!
 */
public class IDEAcryptAFile
{
//	private static final int BUF_LENGTH = 1024;

	private static final int BUF_LENGTH = 8;

	public static void
	usage()
	{
		System.err.println( "usage java IDEAcryptAfile [-d] filename passphrase" );
		System.err.println( "Use -d to decrypt, encrypt otherwise" );
	}

	public static void
	main( String argv[] )
	{
		String filename = null, passphrase = null;
		boolean encrypt = true;

		if ( argv.length == 2 )
		{
			filename = argv[0];
			passphrase = argv[1];
		}
		else if ( argv.length == 3 && "-d".equals( argv[0] ) )
		{
			filename = argv[1];
			passphrase = argv[2];
			encrypt = false;
		}
		

		if ( filename != null )
			try
			{
				doIdea( filename, passphrase, encrypt );
			}
			catch( IOException ioe )
			{
				System.err.print( "There has been an IO exception to the file was not " );
				System.err.println( encrypt?"encrypted":"decrypted" );
				ioe.printStackTrace();
			}
		else
			usage();
	}
	
	/**
	 * This function opens a file and en/de-crypts it printing the result to stdout
	 * @param filename The name of the file to en/de-crypt.
	 * @param passphrase The string to hash to get the Idea key.
	 * @param encrypt The switch to tell the code to encrypt or decrypt.
	 */
	private static void
	doIdea( String filename, String passphrase, boolean encrypt )
	throws IOException
	{
		FileInputStream file_in = new FileInputStream( filename );
		IDEA idea_cipher = new IDEA( MessageDigest.hash( passphrase, new MD5() ) );
		byte iv_buf[] = new byte[idea_cipher.blockLength()];
		if ( encrypt )
		{
			Random rnd = new Random();
			for ( int i = 0, j = iv_buf.length; j >0 ; i++, j-- )
				iv_buf[i] = (byte)rnd.nextInt();
			System.out.write( iv_buf );
		}
		else
			new DataInputStream( file_in ).readFully( iv_buf );
		CipherFeedback cipher = new CipherFeedback( idea_cipher, iv_buf );
		
		// create a cipher feedback from an idea object and an IV.
		byte buf[] = new byte[BUF_LENGTH];
		int position = 0;
		while( true )
		{
			int data = file_in.read();
			if ( data == -1 ) // eof
				break;
			buf[position++] = (byte)data;
			if ( position == BUF_LENGTH )
			{
				position = 0;
				if ( encrypt )
					cipher.encrypt( buf );
				else
					cipher.decrypt( buf );
				System.out.write( buf );
			}
		}
		if ( position > 0 )
		{
			if ( encrypt )
				cipher.encrypt( buf, 0, buf, 0, position );
			else
				cipher.decrypt( buf, 0, buf, 0, position );
			System.out.write( buf, 0, position );
		}
		System.out.flush();
		file_in.close();
	}
}

