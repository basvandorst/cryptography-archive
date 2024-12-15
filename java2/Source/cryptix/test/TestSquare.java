/*	-----------------------------------------------------------------------------
	TestSquare.java
	
	Copyright ©1997, Type & Graphics Pty Limited. All rights reserved.
	----------------------------------------------------------------------------- */
 
package cryptix.test;


import cryptix.security.ByteArrayKey;
import cryptix.util.Util;

import java.security.Cipher;
import java.util.Date;

/**
 *	Tests the output of the Square cipher algorithm implementation against
 *	certified pre-computed output for a given set of reference input.
 *
 *	@revision	1.0 --July 97
 *	@author		Raif S. Naffah
 */
public final class TestSquare
{

// Variables and constants
//................................................................................

	private static Cipher alg;
	private static final byte[] key = new byte[16];
	{
		for (int i = 0; i < 16; i++)
			key[i] = (byte)i;
	}
	private static final ByteArrayKey aKey = new ByteArrayKey(key);

// Constructor
//................................................................................
	
	private TestSquare () {
		System.out.println("\n*** Square:");
		try {
			test1();
			test2();
			test3();
			test4();
			test5();
		} catch (Throwable t) {
			t.printStackTrace();
		}
	}


// main/test methods
//................................................................................
	
	public static final void main (String[] args) {
		Util.installDefaultProviders();
		try {
			TestSquare x = new TestSquare();
		} catch (Throwable t) {
			t.printStackTrace();
		}
	}
	
	private void test1 ()
	throws Exception {

		byte[] input = new byte[16];
		{
			for (int i = 0; i < 16; i++)
				input[i] = (byte)i;
		}

		byte[] output = {
			(byte)0x7C, (byte)0x34, (byte)0x91, (byte)0xD9,
			(byte)0x49, (byte)0x94, (byte)0xE7, (byte)0x0F,
			(byte)0x0E, (byte)0xC2, (byte)0xE7, (byte)0xA5,
			(byte)0xCC, (byte)0xB5, (byte)0xA1, (byte)0x4F};

		System.out.println("\nTest vector (raw/ECB):\nEncrypting:");
		alg = Cipher.getInstance("Square", "Cryptix");
			
		alg.initEncrypt(aKey);
		compareIt(alg.crypt(input), output);
		
		System.out.println("\nDecrypting:");
		alg.initDecrypt(aKey);
		compareIt(alg.crypt(output), input);
	}
	
	private void test2 ()
	throws Exception {

		byte[] input = new byte[32];
		{
			for (int i = 0; i < 16; i++)
				input[i] = input[i + 16] = (byte)i;
		}
		byte[] output = {
			(byte)0x7C, (byte)0x34, (byte)0x91, (byte)0xD9,
			(byte)0x49, (byte)0x94, (byte)0xE7, (byte)0x0F,
			(byte)0x0E, (byte)0xC2, (byte)0xE7, (byte)0xA5,
			(byte)0xCC, (byte)0xB5, (byte)0xA1, (byte)0x4F,

			(byte)0x7C, (byte)0x34, (byte)0x91, (byte)0xD9,
			(byte)0x49, (byte)0x94, (byte)0xE7, (byte)0x0F,
			(byte)0x0E, (byte)0xC2, (byte)0xE7, (byte)0xA5,
			(byte)0xCC, (byte)0xB5, (byte)0xA1, (byte)0x4F
		};

		System.out.println("\nTest vector (ECB):\nEncrypting:");
		alg = Cipher.getInstance("Square/ECB", "Cryptix");
			
		alg.initEncrypt(aKey);
		compareIt(alg.crypt(input), output);
		
		System.out.println("\nDecrypting:");
		alg.initDecrypt(aKey);
		compareIt(alg.crypt(output), input);
	}

	private void test3 ()
	throws Exception {

		byte[] input = new byte[32];
		{
			for (int i = 0; i < 16; i++)
				input[i] = input[i + 16] = (byte)i;
		}
		byte[] output = {
			(byte)0x7C, (byte)0x34, (byte)0x91, (byte)0xD9,
			(byte)0x49, (byte)0x94, (byte)0xE7, (byte)0x0F,
			(byte)0x0E, (byte)0xC2, (byte)0xE7, (byte)0xA5,
			(byte)0xCC, (byte)0xB5, (byte)0xA1, (byte)0x4F,

			(byte)0x41, (byte)0xD2, (byte)0xF1, (byte)0x9D,
			(byte)0x7E, (byte)0x87, (byte)0x8D, (byte)0xB5,
			(byte)0x6C, (byte)0x74, (byte)0x46, (byte)0xD4,
			(byte)0x24, (byte)0xC3, (byte)0xAD, (byte)0xFC
		};

		System.out.println("\nTest vector (CBC):\nEncrypting:");
		alg = Cipher.getInstance("Square/CBC", "Cryptix");
		alg.setInitializationVector(new byte[16]);
			
		alg.initEncrypt(aKey);
		compareIt(alg.crypt(input), output);
		
		System.out.println("\nDecrypting:");
		alg.initDecrypt(aKey);
		compareIt(alg.crypt(output), input);
	}

	private void test4 ()
	throws Exception {

		byte[] input = new byte[32];
		{
			for (int i = 0; i < 16; i++)
				input[i] = input[i + 16] = (byte)i;
		}
		byte[] output = {
			(byte)0xff, (byte)0x58, (byte)0x6d, (byte)0xa5,
			(byte)0x6c, (byte)0xba, (byte)0xc5, (byte)0x06,
			(byte)0x4a, (byte)0x09, (byte)0xa4, (byte)0x0a,
			(byte)0xee, (byte)0xb6, (byte)0xae, (byte)0xaf,

			(byte)0xd5, (byte)0xcb, (byte)0x53, (byte)0x8e,
			(byte)0xea, (byte)0x28, (byte)0x97, (byte)0x4f,
			(byte)0x7c, (byte)0x75, (byte)0xe7, (byte)0x9b,
			(byte)0xcb, (byte)0x0d, (byte)0x4d, (byte)0x0e
		};

		System.out.println("\nTest vector (CFB):\nEncrypting:");
		alg = Cipher.getInstance("Square/CFB", "Cryptix");
		alg.setInitializationVector(new byte[16]);
			
		alg.initEncrypt(aKey);
		compareIt(alg.crypt(input), output);
		
		System.out.println("\nDecrypting:");
		alg.initDecrypt(aKey);
		compareIt(alg.crypt(output), input);
	}

	private void test5 ()
	throws Exception {

		byte[] input = new byte[32];
		{
			for (int i = 0; i < 16; i++)
				input[i] = input[i + 16] = (byte)i;
		}
		byte[] output = {
			(byte)0xff, (byte)0x58, (byte)0x6d, (byte)0xa5,
			(byte)0x6c, (byte)0xba, (byte)0xc5, (byte)0x06,
			(byte)0x4a, (byte)0x09, (byte)0xa4, (byte)0x0a,
			(byte)0xee, (byte)0xb6, (byte)0xae, (byte)0xaf,

			(byte)0x35, (byte)0xc8, (byte)0x33, (byte)0xd3,
			(byte)0x5c, (byte)0x29, (byte)0x44, (byte)0x37,
			(byte)0x35, (byte)0xd2, (byte)0x25, (byte)0xbc,
			(byte)0x95, (byte)0x28, (byte)0xc3, (byte)0xc8
		};

		System.out.println("\nTest vector (OFB):\nEncrypting:");
		alg = Cipher.getInstance("Square/OFB", "Cryptix");
		alg.setInitializationVector();
			
		alg.initEncrypt(aKey);
		compareIt(alg.crypt(input), output);
		
		System.out.println("\nDecrypting:");
		alg.initDecrypt(aKey);
		compareIt(alg.crypt(output), input);
	}

	private void compareIt (byte[] o1, byte[] o2) {
		if (o1.length < 33) {
			System.out.println("  computed: " + Util.toHexString(o1));
			System.out.println(" certified: " + Util.toHexString(o2));
		} else {
			System.out.println("  computed: ");
			Util.hexDump(o1);
			System.out.println(" certified: ");
			Util.hexDump(o2);
		}
		if (Util.areEqual(o1, o2))
			System.out.println(" *** GOOD");
		else
			System.out.println(" *** FAILED");
	}
}
