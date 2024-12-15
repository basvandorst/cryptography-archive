/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */

/**
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

import cryptix.security.rsa.PublicKey;

public final class SignedData
{
	public byte[] data;
	public Signature signature;

	public ByteArrayInputStream
	dataAsByteArrayInputStream()
	{
		return new ByteArrayInputStream(data);
	}

	public String
	dataAsString()
	{
		return new String(data, 0);
	}

	public
	SignedData(byte[] data, Signature sig)
	{
		this.data = data;
		this.signature = sig;
	}

	public KeyID
	getKeyId()
	{
		return signature.getKeyId();
	}

	public byte[]
	keyId()
	{
		return signature.keyId();
	}

	public boolean
	verify(PublicKey key)
	{
		return signature.verify(key, data);
	}
}
