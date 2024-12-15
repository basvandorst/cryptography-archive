package cryptix.pgp_examples;

import java.io.*;
import cryptix.security.rsa.*;

import cryptix.pgp.*;

/**
 * Decrypts a file (name in argv[0]) to the screen
 * Demonstrates the ArmouredMessage, Passphrase and SecretKeyRing classes.
 * Run this in your PGP directory, or provide the full path to your keyrings.
 * @author Ian Brown
 * @version 1.0
 **/
public final class dTest
{
    public static final void main( String argv[] )
    throws FileNotFoundException, IOException, DecryptException, FormatException, InvalidChecksumException
    {
	KeyGlob keyGlob = new KeyGlob();
	ArmouredMessage message;
	String line;

	String messageText = "";

	 if ( argv.length < 2 )
	{
		System.out.println( "Usage: java dTest filename passphrase" );
		return;
	}

	DataInputStream in = new DataInputStream( new FileInputStream( argv[0] ) );
	while ( ( line = in.readLine() ) != null ) messageText += line + Armoury.LINEFEED;
	in.close();

	System.out.println( "Decrypting " + argv[0] );

	// Get keyrings
	keyGlob.addKeyRing( new PublicKeyRing( "pubring.pgp" ) );
 	keyGlob.addKeyRing( new SecretKeyRing( "secring.pgp" ) );

	// Decrypt message
	message = new ArmouredMessage( messageText, keyGlob, new Passphrase( argv[1] )  );

	// Display plaintext
	if ( message.authenticated() ) System.out.println( "-The following message is signed-------------" );
	System.out.println( message.plaintext() );
	if ( message.authenticated() ) 
	{
		String sender = message.signedBy();
		String dashes = ""; // Stores the start of the signed by line
		// Java doesn't have a String * int operator, unfortunately
		for ( int i = 0; i < ( 44 - sender.length() ); i++ ) dashes += "-";
		System.out.println( dashes + sender + "-" );
	}
    }
}