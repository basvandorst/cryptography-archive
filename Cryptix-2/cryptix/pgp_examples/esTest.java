package cryptix.pgp_examples;

import java.io.*;
import java.util.Vector;
import cryptix.security.rsa.*;

import cryptix.pgp.*;
import cryptix.util.EmailAddress;

/**
 * Encrypts and signs a string (messageText) and writes the ciphertext to es-test.asc
 * Demonstrates the ArmouredMessage, Passphrase, PublicKeyRing,
 * SecretKeyRing and EmailAddress classes.
 * Run this in your PGP directory, or provide the full path to your keyrings.
 * @author Ian Brown
 * @version 1.0
 **/
public final class esTest
{
    public static final void main( String argv[] ) throws FileNotFoundException, IOException, DecryptException, FormatException
    {

	SecretKeyRing secretKeyRing;
	PublicKeyRing publicKeyRing;
	cryptix.security.rsa.SecretKey myKey;
	PublicKey toKey;
	Vector to = new Vector();
	ArmouredMessage message;
	FileOutputStream fos;

	EmailAddress myAddress = new EmailAddress( "Ian Brown <Ian.Brown@ncl.ac.uk>") ;
	//String myAddress = "Ian Brown <Ian.Brown@ncl.ac.uk>";
	EmailAddress toAddress = new EmailAddress( "Ian Brown <Ian.Brown@newcastle.ac.uk>");
	String messageText = "Put your test message here." + Armoury.LINEFEED
		+ "It can be as Happy Xmas as you like.";

	 if ( argv.length < 1 )
	{
		System.out.println( "Usage: java esTest passphrase" );
		return;
	}

	System.out.println( "Writing a signed version of '" + messageText + "' encrypted for " + toAddress + " to es-test.asc.");

	// Get keys
 	secretKeyRing = new SecretKeyRing( "secring.pgp" );
	publicKeyRing = new PublicKeyRing( "pubring.pgp" );
 	myKey = secretKeyRing.getKey( myAddress, new Passphrase( argv[0] ) );
	if (myKey == null )
	{
		System.err.println( "Couldn't find key for " + myAddress.toString() );
		System.exit( 1 );
	}
	toKey = publicKeyRing.getKey( toAddress );
	if ( toKey != null ) to.addElement(toKey);
	else System.err.println( "Couldn't find key for " + toAddress.toString() );

	// Message will be encrypted and signed
	message = new ArmouredMessage( messageText, myKey, to );

	// Write clearsigned message to a file
	fos = new FileOutputStream( "es-test.asc" );
	new PrintStream( fos ).print( message.ciphertext() );
	fos.close();
    }
}
