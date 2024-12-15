/**
 * Represents an ASCII-armoured PGP encrypted message.<p>
 * Can encrypt a String, given public keys to encrypt to,
 * and sign with the relevant secret key.<p>
 * Can also decrypt an armoured message - as long as it doesn't
 * contain compressed data. That will be dealt with in the next version.<p>
 * <h3>FUTURE CHANGES</h3><ul>
 * <li>ArmouredData will be used instead of String in function calls/returns 
 * where appropriate</li>
 * <li>Will use the new charset package to deal better with clearsigned messages</li>
 * <li>Will use the next version's PGP engine rather than doing all the work</li></ul>
 * @author Ian Brown
 * @see cryptix.pgp
 *
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 */

package cryptix.pgp;

import java.io.*;

import cryptix.security.*;
import cryptix.security.rsa.*;

import java.util.Vector;
import java.util.Date;

public final class ArmouredMessage
{
	private String ciphertext_ = null;
	private String plaintext_ = null;
	private String signedBy_ = null;
	private boolean authenticated_ = false;
	private boolean encrypted_ = false;

	private cryptix.security.rsa.SecretKey from = null;
	private Vector to = null;

	private static final String LINEFEED = Armoury.LINEFEED;

	// Packets used to store encrypted data as it is created
	private PublicKeyEncrypted PKE = null;
	private ConvEncryptedData CKE = null;

	/**
	* Creates a new ASCII-armoured encrypted and signed message.<p>
	* If <i>from</i> is null, the message will only be encrypted.<p>
	* If <i>to</i> is empty, the message will be clearsigned.<p>
	* <i>plaintext_</i> and <i>ciphertext_</i> will be set appropriately.
	* Retrieve them using the <b>plaintext()</b> and <b>ciphertext()</b> functions.
	* @param msg A <b>string</b> that you want to encrypt
	* @param from Your PGP <b>secret key</b>
	* @param to A <code>Vector</code> containing the public keys of the message's recipient(s)
	*/
	public ArmouredMessage( String msg, SecretKey sender, Vector recipients )
	throws IOException
	{
		plaintext_ = msg;

		from = sender;
 		to = recipients;

		Date date = new Date();
		String keySeed = String.valueOf( date.getTime() * Math.random() );
		byte ideaKey[] = MD5.hash( keySeed );

		FileRandomStream rs = new FileRandomStream();

		// Temporary streams used to join packets together
		ByteArrayOutputStream eData= new ByteArrayOutputStream();
		PacketOutputStream peData = new PacketOutputStream( eData );
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		PacketOutputStream dout = new PacketOutputStream( out );

 		if ( from == null && (to == null || to.isEmpty() ) )
		{
			// Without secret or public keys, cryptography cannot take place
			// Or should this cause an error?
			ciphertext_ = msg;
 		}

  		else if ( from == null )
  		{
			// Encrypt message only
			for ( int i = 0;  i < to.size(); i++ )
			{
				// Create a public-key encrypted packet containing the IDEA key for each recipient
				PKE = new PublicKeyEncrypted ( ( PublicKey )( to.elementAt( i ) ), ideaKey, rs );
				peData.writePacket( PKE );
			}

			// IDEA-encrypt the data with ideaKey
			LiteralData stuff = new LiteralData( byteEncode( msg ), "stuff.txt", 't' );
			CKE = new ConvEncryptedData( PacketFactory.save( stuff ), ideaKey, rs );
			peData.writePacket( CKE );

			ciphertext_ = Armoury.message( "MESSAGE", eData.toByteArray() );
			encrypted_ = true;
		}

 		else if ( to == null || to.isEmpty() )
 		{
  			// Message should be clearsigned only
			Signature msgSig = sign( byteEncode( msg ) );
			// Process msg to change lines starting with - to start with - - (some PGP thing )
			// The signature should be of the UN-processed version
			ciphertext_ = "-----BEGIN PGP SIGNED MESSAGE-----" + LINEFEED + LINEFEED;
			ciphertext_ += dashAdd( msg ) + LINEFEED;
        			ciphertext_ += Armoury.message ( "SIGNATURE", PacketFactory.save( msgSig ) );
			authenticated_ = true;
		}

		else
		{
			// Message will be encrypted and signed
			for ( int i = 0;  i < to.size(); i++ )
			{
				// Create a public-key encrypted packet containing the IDEA key for each recipient
				PKE = new PublicKeyEncrypted ( ( PublicKey )( to.elementAt( i ) ), ideaKey, rs );
				peData.writePacket( PKE );
			}

			// Create PGP data and signature packets
			LiteralData stuff = new LiteralData( byteEncode( msg ), "stuff.txt", 't' );
			Signature sig = sign( stuff.data() );

			// Write the signature and data to out
			dout.writePacket( sig );
			dout.writePacket( stuff );

			// Encrypt the signature and data packets into an IDEA-encrypted packet
			CKE = new ConvEncryptedData( out.toByteArray(), ideaKey, rs );

			// Add all packets together, then armour them
			peData.writePacket( PKE );
			peData.writePacket( CKE );

			ciphertext_ = Armoury.message( "MESSAGE", eData.toByteArray()  );
			authenticated_ = true;
			encrypted_ = true;
 		}
	}

	/**
	* Decrypts and verifies an ASCII-armoured encrypted and/or signed message
	* <i>plaintext()</i> will return the decrypted message.<p>
	* @param PGPmsg A <b>string</b> that you want to decrypt.
	* It <b>must</b> include the -----BEGIN PGP and -----END PGP wrapper.
	*/
	public ArmouredMessage( String PGPmsg, KeyGlob keyGlob, Passphrase passphrase )
	throws DecryptException, FormatException, IOException, InvalidChecksumException
	{
		ciphertext_ = PGPmsg;
		if( PGPmsg.indexOf( "-----BEGIN PGP SIGNED MESSAGE-----") != -1 )
		{
			// This is a clearsigned message
			// We need to extract the message and the signature, then compare them
			byte[] sigBuf; // Will hold de-armoured signature packet
			String line, text = "";
			String sigArmour="";
			Signature sig = null;
			PublicKey fromKey = null;
			int pos = 0;

			// Get start of message
			pos = PGPmsg.indexOf( "-----BEGIN PGP SIGNED MESSAGE-----" );
			// Now get rid of version / comments
			pos = PGPmsg.indexOf( LINEFEED + LINEFEED, pos );
			// Get text section
			text = PGPmsg.substring( pos + 4, PGPmsg.indexOf( LINEFEED + "-----BEGIN PGP SIGNATURE-----" ) );
			// Get armoured signature
			sigArmour = PGPmsg.substring( PGPmsg.indexOf( "-----BEGIN PGP SIGNATURE-----" ) );

			// Process message for lines beginning with -
			text = dashSubtract( text );

			// We now have the message in text and its signature in sigArmour
			plaintext_ = text;
			sigBuf = Armoury.strip( sigArmour );
			sig = (Signature) PacketFactory.load( sigBuf );
			// Now check this signature
			fromKey = keyGlob.getKey( sig.getKeyId() );
			if ( fromKey != null )
			{
				// We have the public key to check this sig
				MD5 msgHash = new MD5();
				msgHash.add( byteEncode( text ) );
				if ( sig.check ( fromKey, msgHash ) ) authenticated_ = true;
				else throw new DecryptException( "Signature does not match message." );
				signedBy_ = keyGlob.getUserName( sig.getKeyId() );
			}
		}
		else if( PGPmsg.indexOf( "-----BEGIN PGP MESSAGE-----") != -1 )
		{
			byte[] msgBuf = Armoury.strip( PGPmsg );
			PacketInputStream msgStream = new PacketInputStream( new ByteArrayInputStream( msgBuf ) );
			byte[] convPkt = null;  // Stores decrypted IDEA packet
			PacketInputStream convBuf = null;
			byte[] ideaKey = null; // Holds key from RSA-encrypted packet to decrypt IDEA-encrypted packet
			cryptix.security.rsa.SecretKey secretKey = null; // Used to decrypt RSA-encrypted packet
			Signature sig = null; // Holds signature (if present in PGPmsg)
			Packet pkt = null;
			byte[] msgData = null;

			encrypted_ = true;

			while ( ( pkt = msgStream.readPacket( ) ) instanceof PublicKeyEncrypted )
			{
				// If we don't already have the conventional key, try and get it from this packet
				if (  ideaKey == null )
				{
					// If we have the corresponding secret key and passphrase,
					// use it to decrypt this packet
					KeyID encryptedFor = ( (PublicKeyEncrypted)pkt ).secretKeyID();
					if ( ( secretKey = keyGlob.getSecretKey ( encryptedFor, passphrase ) ) != null )
						ideaKey = ( (PublicKeyEncrypted)pkt ).getKey( secretKey );
				}
			}

			if ( ideaKey == null ) throw new DecryptException( "You don't possess the secret key needed to read this message" );
			if ( ! ( pkt instanceof ConvEncryptedData ) ) throw new FormatException( "Unexpected PGP packet: " + pkt.getClass().getName() );

			// We can now decrypt the IDEA conventionally-encrypted packet
			convPkt = ( (ConvEncryptedData)pkt ).data( ideaKey );
			convBuf = new PacketInputStream( new ByteArrayInputStream( convPkt ) );
			// Iterate over packets contained in convBuf
			while( ( pkt = convBuf.readPacket() ) != null )
			{
				if ( pkt instanceof Signature ) // Store to check later
					sig = (Signature)pkt;
				// Might a Vector be needed to store multiple signatures? Later...

				else if ( pkt instanceof LiteralData )
				{
					if ( ( (LiteralData)pkt ).mode() == 't' )
					{
					    	// is a text message
						msgData = ( (LiteralData)pkt ).data();
						plaintext_ = new String( msgData, 0 );
					}
					else throw new FormatException( "Message contains binary data - PGP is needed to read it." );
				}
			}

			// We now should have set plaintext_
			// Do we have a corresponding signature?
			if ( sig != null )
			{
				// Check it
				PublicKey fromKey;
				MD5 msgHash = new MD5();
				if ( ( fromKey = keyGlob.getKey( sig.getKeyId() ) ) != null )
				{
					// We have the public key needed to check the signature
					msgHash.add( msgData );
					if ( sig.check ( fromKey, msgHash ) ) authenticated_ = true;
					else throw new DecryptException( "Signature does not match message." );
					signedBy_ = keyGlob.getUserName( sig.getKeyId() );
				}
			}
		}

		else throw new FormatException( "This class can only decrypt PGP *messages*" );
	}

	/**
	* @return A string representing the ASCII-armoured encrypted text (including the -----BEGIN PGP wrapper)
	*/
	public final String ciphertext()
	{
		return ciphertext_;
	}

	/**
	* @return A string representing the decrypted text
	*/
	public final String plaintext()
	{
		return plaintext_;
	}

	/**
	* @return The name of the person who signed the message
	*/
	public final String signedBy()
	{
		return signedBy_;
	}

	/**
	* @return <b>true</b> if the decrypted message has a good signature
	*/
	public final boolean authenticated()
	{
		return authenticated_;
	}

	/**
	* @return <b>true</b> if the message was/is encrypted
	*/
	public final boolean encrypted()
	{
		return encrypted_;
	}

 	/**
 	*@return A Signature containing a signed hash of msgBuf
 	*/
   	private Signature sign( byte[] msgBuf ) throws IOException
   	{
		MD5 md = new MD5();
		md.add(msgBuf);
		return new Signature( from, md );
	}

	/**
	* These PGP classes work on byte arrays, not strings.
	* This function converts from one to the other.
	*@return A byte array representing text
	*/
	public static byte[] byteEncode( String text )
	{
		int len = text.length();
		byte buf[];
 		text.getBytes( 0, len, buf = new byte[len], 0 );
		return buf;
	}

	private String dashAdd( String text )
	{
		int pos = 0;
		while ( ( pos = text.indexOf( LINEFEED + "-", pos ) ) != -1 )
		{
			// If a line starts with -, change it to start with - -
			text = text.substring( 0, pos + 2 ) + "- " + text.substring( pos + 2 );
			pos+=3;
		}

		return text;
	}

	private String dashSubtract( String text )
	{
		int pos = 0;
		while ( ( pos = text.indexOf( LINEFEED + "- -", pos ) ) != -1 )
		{
			// If a line starts with - -, change it to start with -
			text = text.substring( 0, pos + 2 ) + text.substring( pos + 4 );
			pos +=3;
		}

		return text;
	}
}
