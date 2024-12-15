/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */
 
package java.crypt;

/**
 * This is the abstract base class for all message digests.
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @see java.crypt.MD5
 * @see java.crypt.SHA
 *
 * @version     1.00, 12/02/1996
 * @author      Systemics (Michael Wynn)
 */
public abstract class MessageDigest
{
	/**
	 * Both protected and abstract, so this class must be derived
	 * from in order to be useful.
	 */
	protected MessageDigest()
	{
	}

	/**
	 * Return the hash length
	 * <p><b>N.B.</b> It is advisable to add public static final int HASH_LENGTH
	 * @see java.crypt.MD5#HASH_LENGTH
	 * @see java.crypt.SHA#HASH_LENGTH
	 * @return   The length of the hash.
	 */
	public abstract int length();

	/**
	 * Initialise (reset) the message digest.
	 */
	public abstract void reset();
		
	/**
	 * Obtain the digest<p>
	 * <p><b>N.B.</b> this resets the digest.
	 * @return    the digest of all the data added to the message digest.
	 */
	public abstract byte[] digest();

	/**
	 * Add the low bytes of a string to the digest (ie. treat the string as ASCII).
	 * @param message    The string to add.
	 * @param offset     The start of the data string.
	 * @param length     The length of the data string.
	 */
	public final void add( String message, int offset, int length )
	{
		if ( message == null )
			throw new CryptoError( "Cannot hash a null string" );

		if  ( length < 0 )
			throw new CryptoError( "Negative length" );

		if ( offset < 0 )
			throw new CryptoError( "Negative offset" );

		if  ( ( length + offset ) > message.length() )
			throw new CryptoError( "Offset past end of data" );
		
		if ( length == 0 )
			return; // nothing to do so do nothing.
		
		byte data[] = new byte[length];
		
		message.getBytes( offset, offset + length, data, 0 );
		
		addToDigest( data, 0, length );
	}

	/**
	 * Add the low bytes of a string to the digest (ie. treat the string
	 * as ASCII ).
	 * @param message    The string to add.
	 */
	public final void add( String message )
	{
		if ( message == null )
			throw new CryptoError( "Cannot hash a null string" );

		int length = message.length();
		
		if ( length == 0 )
			return; // nothing to do so do nothing.
		
		byte data[] = new byte[ length ];
		
		message.getBytes( 0, length, data, 0 );
		
		addToDigest( data, 0, length );
	}

	/**
	 * Add a byte array to the digest
	 * @param data    The data to be added.
	 */
	public final void add( byte data[] )
	{
		addToDigest( data, 0, data.length );
	}

	/**
	 * Add a section of a byte array to the digest
	 * @param data    The data to add.
	 * @param offset     The start of the data to add.
	 * @param length     The length of the data to add.
	 */
	public final void add(byte data[], int offset, int length)
	{
		if ( data == null )
			throw new CryptoError( "Cannot hash a null array" );

		if ( length < 0 )
			throw new CryptoError( "Negative length" );

		if ( offset < 0 )
			throw new CryptoError( "Negative offset" );

		if ( ( length + offset ) > data.length )
			throw new CryptoError( "Offset past end of data" );

		// N.B. if data.length == 0, then length and offset
		// must also be zero otherwise an error would have been thrown.

		if ( length == 0 ) 
			return; // do nothing.

		addToDigest( data, offset, length );
	}

	/**
	 * Add the data to the hash.
	 * <p><b>N.B.</b> the implementor of this function can assume
	 * parameter validation has been done at this point.
	 * @param data    The data to be added.  This must remain unmodified.
	 * @param offset  The starting point within the array.
	 * @param length  The amount of data to add.
	 */
	protected abstract void addToDigest( byte data[], int offset, int length );
	
	/**
	 * A convenience function for hashing a string.<p>
	 * eg:
	 * <pre> byte key[] = MessageDigest.hash( passPhrase, new MD5() ); </pre>
	 * @param message  The string to hash.
	 * @param md       An instance of a message digester.
	 */
	public static final byte[] hash( String message, MessageDigest md )
	{
		md.add( message );
		return md.digest();
	}
}
