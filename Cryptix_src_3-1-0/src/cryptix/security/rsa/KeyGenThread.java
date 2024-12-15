// This file is currently unlocked (change this line if you lock the file)
//
// $Log: KeyGenThread.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/04/13  Ian Brown
//   Did Mike's bugfix
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
//   Moved this file here from old namespace.
//
// Revision 0.2.5.0  1997/02/24  Original Author not stated
//   Original version
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */
package cryptix.security.rsa;

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
