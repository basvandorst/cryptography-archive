/* $Id: TestPGP.java,v 1.1 1999/07/25 20:18:43 edwin Exp $
 *
 * Copyright (C) 1999 Systemics Ltd.
 * on behalf of the Cryptix Development Team. All rights reserved.
 *
 * Use, modification, copying and distribution of this software is subject
 * the terms and conditions of the Cryptix General Licence. You should have
 * received a copy of the Cryptix General License along with this library;
 * if not, you can download a copy from http://www.cryptix.org/ .
 */

package cryptix.test;


import cryptix.util.test.BaseTest;

import cryptix.pgp.Algorithm;
import cryptix.pgp.ArmouredData;
import cryptix.pgp.ArmouredMessage;
import cryptix.pgp.Armoury;
import cryptix.pgp.CFB;
import cryptix.pgp.Comment;
import cryptix.pgp.CompressedData;
import cryptix.pgp.ConvEncryptedData;
import cryptix.pgp.CRC;
import cryptix.pgp.DashProtected;
import cryptix.pgp.DecryptException;
import cryptix.pgp.EncryptException;
import cryptix.pgp.FileRandomStream;
import cryptix.pgp.FormatException;
import cryptix.pgp.HashFactory;
import cryptix.pgp.InvalidChecksumException;
import cryptix.pgp.KeyCache;
import cryptix.pgp.KeyClient;
import cryptix.pgp.KeyGlob;
import cryptix.pgp.KeyID;
import cryptix.pgp.KeyRing;
import cryptix.pgp.KeyRingEntry;
import cryptix.pgp.KeyRingTrust;
import cryptix.pgp.KeyStore;
import cryptix.pgp.LiteralData;
import cryptix.pgp.Message;
import cryptix.pgp.Packet;
import cryptix.pgp.PacketByteArray;
import cryptix.pgp.PacketFactory;
import cryptix.pgp.PacketHeader;
import cryptix.pgp.PacketInputStream;
import cryptix.pgp.PacketOutputStream;
import cryptix.pgp.Passphrase;
import cryptix.pgp.PublicKeyCertificate;
import cryptix.pgp.PublicKeyEncrypted;
import cryptix.pgp.PublicKeyRing;
import cryptix.pgp.SecretKeyCertificate;
import cryptix.pgp.SecretKeyRing;
import cryptix.pgp.Signature;
import cryptix.pgp.SignedData;
import cryptix.pgp.SignedMessage;
import cryptix.pgp.UserId;
import cryptix.pgp.UserIdCertificate;
import cryptix.pgp.WebClient;
import cryptix.pgp.charset.Ascii;
import cryptix.pgp.charset.CharSet;
import cryptix.pgp.charset.Cp850;
import cryptix.pgp.charset.Cp852;
import cryptix.pgp.charset.Cp860;
import cryptix.pgp.charset.Cp866;
import cryptix.pgp.charset.Keybcs;
import cryptix.pgp.charset.Mac;
import cryptix.pgp.charset.Next;
import cryptix.pgp.charset.SimpleCharSet;


/**
 * WARNING: This is not a test to see if PGP is working. It just tests whether
 * all the files are there. If this file does not compile then something is 
 * missing.
 *
 * @author Edwin Woudt (edwin@cryptix.org)
 */
public class TestPGP
extends BaseTest
{
    
    public static void main (String[] args) {
        new TestPGP().commandline(args);
    }
    
    protected void engineTest() throws Exception {
        setExpectedPasses(1);
        passIf(true, "");
    }

}
