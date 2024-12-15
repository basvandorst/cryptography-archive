package cryptix.pgp_examples;

import java.io.*;
import java.util.Vector;
import cryptix.security.rsa.*;

import cryptix.pgp.*;

/**
 * Encrypts a string (messageText) and writes the ciphertext to e-test.asc
 * Demonstrates the ArmouredMessage and PublicKeyRing classes.
 * Run this in your PGP directory, or provide the full path to your public keyring.
 * @author Ian Brown
 * @version 1.0
 **/
public final class eTest
{
    public static final void main( String argv[] ) throws FileNotFoundException, IOException, DecryptException
    {

	// See sTest.java for detailed comments

	PublicKeyRing publicKeyRing;
	PublicKey toKey;
	ArmouredMessage message;
	Vector to = new Vector();
	FileOutputStream fos;

	String toAddress = "Ian Brown <Ian.Brown@newcastle.ac.uk>";
	String messageText = "Put your test message here." + Armoury.LINEFEED
		+ "It can be as long as you like.";
	System.out.println( "Writing '" + messageText + "' encrypted for " + toAddress + " to e-test.asc.");
 
	publicKeyRing = new PublicKeyRing( "pubring.pgp" );
 	toKey = publicKeyRing.getKey( toAddress );
 	if( toKey != null ) to.addElement( toKey );
	else System.err.println( "Couldn't find key for " + toAddress );

	message = new ArmouredMessage( messageText, null, to );

	fos = new FileOutputStream( "e-test.asc" );
 	new PrintStream( fos ).print( message.ciphertext() );
  	fos.close();
    }
}
