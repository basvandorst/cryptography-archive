/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */
package cryptix.security.rsa;

import cryptix.security.rsa.*;
import cryptix.pgp.*;
import cryptix.math.RandomStream;

public class KeyGenThread extends Thread implements RSAKeyGenObserver
{
	RandomStream rs;
	int len;
	RSAKeyGenObserver observer;
	cryptix.security.rsa.SecretKey key;

	public cryptix.security.rsa.SecretKey key() { return key; }

	public KeyGenThread(String name, RandomStream rs, int len, RSAKeyGenObserver observer)
	{
		super(name);
		this.rs = rs;
		this.len = len;
		this.observer = observer;
	}

	public void
	run()
	{
		key = new RSAKeyGen(rs).createKey(len, this);
	}

	public void handleEvent(int evnt)
	{
		Thread.yield(); // don't rely on pre-emptive threads
		// These events shouldn't be ints, they should be like exceptions,
		// an object with an int
		observer.handleEvent(evnt);
	}

}
