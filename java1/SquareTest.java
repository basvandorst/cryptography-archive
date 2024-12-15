import java.lang.*;
import Square;
import SquareEcb;
import SquareCbc;
import SquareCts;
import SquareCfb;
import SquareOfb;

public final class SquareTest {

    private static final byte[] key = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    };

    private static final byte[] iv = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    private static final String hexDigit [] = {
        "0", "1", "2", "3", "4", "5", "6", "7",
        "8", "9", "a", "b", "c", "d", "e", "f",
    };

    protected static void printBuffer (byte[] buf, int len, String tag)
    {
        String s = new String ();

    	for (int i = 0; i < len; i++) {
    	    byte b = buf[i];
    		s += hexDigit[(b >>> 4) & 0x0f] + hexDigit[b & 0x0f];
    		if (((i + 1) & 15) == 0) {
    			s += " "; // put a space every 16 bytes
    		} else if (((i + 1) & 31) == 0) {
    			s += "\n"; // break line every 32 bytes
    		}
    	}
    	if ((len & 15) == 0 || (len & 31) == 0) {
    	    System.out.println (s + tag);
    	} else {
    	    System.out.println (s + " " + tag);
    	}
    } // printBuffer


    public static final void testRaw (int iterations)
        throws java.io.IOException
    {
        System.out.println ("Raw test:");
        byte[] plaintext = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        };
    	byte[] ciphertext = {
    		(byte)0x7c, (byte)0x34, (byte)0x91, (byte)0xd9,
    		(byte)0x49, (byte)0x94, (byte)0xe7, (byte)0x0f,
    		(byte)0x0e, (byte)0xc2, (byte)0xe7, (byte)0xa5,
    		(byte)0xcc, (byte)0xb5, (byte)0xa1, (byte)0x4f,
        };
        byte[] data = new byte[Square.BLOCK_LENGTH];
    	printBuffer (key, key.length, "key");
    	printBuffer (plaintext, plaintext.length, "plaintext");
   	    Square sq = new Square (key);
    	sq.blockEncrypt (plaintext, 0, data, 0);
    	CHECK_ENCRYPTION: {
        	for (int i = 0; i < Square.BLOCK_LENGTH; i++) {
        	    if (data[i] != ciphertext[i]) {
            		printBuffer (data, data.length, "encrypted(ERROR)");
            		printBuffer (ciphertext, ciphertext.length, "expected");
            		break CHECK_ENCRYPTION;
            	}
        	}
       		printBuffer (data, data.length, "encrypted(OK)");
       	}
    	sq.blockDecrypt (data, 0, data, 0);
    	CHECK_DECRYPTION: {
        	for (int i = 0; i < Square.BLOCK_LENGTH; i++) {
        	    if (data[i] != plaintext[i]) {
            		printBuffer (data, data.length, "decrypted(ERROR)");
            		break CHECK_DECRYPTION;
            	}
        	}
       		printBuffer (data, data.length, "decrypted(OK)");
       	}
       	if (iterations > 0) {
       	    System.out.println ("Speed test for " + iterations + " iterations:");
       	    long elapsed; float secs;
       	    System.out.println ("Measuring encryption speed...");
           	// measure encryption speed:
            elapsed = -System.currentTimeMillis ();
            for (int i = 0; i < iterations; i++) {
    	        sq.blockEncrypt (data, 0, data, 0);
            }
            elapsed += System.currentTimeMillis ();
            secs = (elapsed > 1) ? (float)elapsed/1000 : 1;
            System.out.println ("Elapsed time = " + secs + ", speed = "
                + ((float)iterations*Square.BLOCK_LENGTH/1024/secs) + " kbytes/s");
       	    System.out.println ("Measuring decryption speed...");
           	// measure encryption speed:
            elapsed = -System.currentTimeMillis ();
            for (int i = 0; i < iterations; i++) {
    	        sq.blockDecrypt (data, 0, data, 0);
            }
            elapsed += System.currentTimeMillis ();
            secs = (elapsed > 1) ? (float)elapsed/1000 : 1;
            System.out.println ("Elapsed time = " + secs + ", speed = "
                + ((float)iterations*Square.BLOCK_LENGTH/1024/secs) + " kbytes/s");
        }
    } // testRaw


    public static final void testMode (String tag, SquareMode sq, byte[] key, byte[] iv,
        byte[] plaintext, byte[] ciphertext, int iterations)
    {
        System.out.println (tag + " test:");
    	printBuffer (key, key.length, "key");
    	printBuffer (plaintext, plaintext.length, "plaintext");
        byte[] data = new byte[plaintext.length];
        System.arraycopy (plaintext, 0, data, 0, plaintext.length);

        sq.setKey (key);
        sq.setIV (iv);
    	sq.encrypt (data, 0, data.length);
    	CHECK_ENCRYPTION: {
        	for (int i = 0; i < ciphertext.length; i++) {
        	    if (data[i] != ciphertext[i]) {
            		printBuffer (data, data.length, "encrypted(ERROR)");
            		printBuffer (ciphertext, ciphertext.length, "expected");
            		break CHECK_ENCRYPTION;
            	}
        	}
       		printBuffer (data, data.length, "encrypted(OK)");
       	}

        sq.setKey (key);
        sq.setIV (iv);
    	sq.decrypt (data, 0,  data.length);
    	CHECK_DECRYPTION: {
        	for (int i = 0; i < plaintext.length; i++) {
        	    if (data[i] != plaintext[i]) {
            		printBuffer (data, plaintext.length, "decrypted(ERROR)");
            		break CHECK_DECRYPTION;
            	}
        	}
       		printBuffer (data, data.length, "decrypted(OK)");
       	}
       	if (iterations > 0) {
       	    System.out.println ("Speed test for " + iterations + " iterations:");
       	    long elapsed; float secs;
       	    System.out.println ("Measuring encryption speed...");
           	// measure encryption speed:
            elapsed = -System.currentTimeMillis ();
            for (int i = 0; i < iterations; i++) {
    	        sq.encrypt (data, 0, data.length);
            }
            elapsed += System.currentTimeMillis ();
            secs = (elapsed > 1) ? (float)elapsed/1000 : 1;
            System.out.println ("Elapsed time = " + secs + ", speed = "
                + ((float)iterations*plaintext.length/1024/secs) + " kbytes/s");
       	    System.out.println ("Measuring decryption speed...");
           	// measure encryption speed:
            elapsed = -System.currentTimeMillis ();
            for (int i = 0; i < iterations; i++) {
    	        sq.decrypt (data, 0, data.length);
            }
            elapsed += System.currentTimeMillis ();
            secs = (elapsed > 1) ? (float)elapsed/1000 : 1;
            System.out.println ("Elapsed time = " + secs + ", speed = "
                + ((float)iterations*plaintext.length/1024/secs) + " kbytes/s");
        }
    } // testMode


    public static final void testEcb (int iterations)
    {
        byte[] plaintext = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        };
    	byte[] ciphertext = {
    		(byte)0x7c, (byte)0x34, (byte)0x91, (byte)0xd9,
    		(byte)0x49, (byte)0x94, (byte)0xe7, (byte)0x0f,
    		(byte)0x0e, (byte)0xc2, (byte)0xe7, (byte)0xa5,
    		(byte)0xcc, (byte)0xb5, (byte)0xa1, (byte)0x4f,
    		(byte)0x7c, (byte)0x34, (byte)0x91, (byte)0xd9,
    		(byte)0x49, (byte)0x94, (byte)0xe7, (byte)0x0f,
    		(byte)0x0e, (byte)0xc2, (byte)0xe7, (byte)0xa5,
    		(byte)0xcc, (byte)0xb5, (byte)0xa1, (byte)0x4f,
        };
        testMode ("ECB", new SquareEcb(), key, iv, plaintext, ciphertext, iterations);
    } // testEcb


    protected static void testCbc (int iterations)
        throws java.io.IOException
    {
        byte[] plaintext = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        };
    	byte[] ciphertext = {
    		(byte)0x7c, (byte)0x34, (byte)0x91, (byte)0xd9,
    		(byte)0x49, (byte)0x94, (byte)0xe7, (byte)0x0f,
    		(byte)0x0e, (byte)0xc2, (byte)0xe7, (byte)0xa5,
    		(byte)0xcc, (byte)0xb5, (byte)0xa1, (byte)0x4f,
    		(byte)0x41, (byte)0xd2, (byte)0xf1, (byte)0x9d,
    		(byte)0x7e, (byte)0x87, (byte)0x8d, (byte)0xb5,
    		(byte)0x6c, (byte)0x74, (byte)0x46, (byte)0xd4,
    		(byte)0x24, (byte)0xc3, (byte)0xad, (byte)0xfc,
    	};
        testMode ("CBC", new SquareCbc(), key, iv, plaintext, ciphertext, iterations);
    } // testCbc


    protected static void testCts (int iterations)
        throws java.io.IOException
    {
        byte[] plaintext = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
            0x00, 0x01, 0x02, 0x03,
        };
    	byte[] ciphertext = {
    		(byte)0x02, (byte)0xde, (byte)0x82, (byte)0x56,
    		(byte)0x73, (byte)0xf5, (byte)0xca, (byte)0xf5,
    		(byte)0xa9, (byte)0x5c, (byte)0xd6, (byte)0x3c,
    		(byte)0xbf, (byte)0x33, (byte)0x9c, (byte)0x85,
    		(byte)0xd5, (byte)0xca, (byte)0x51, (byte)0x8d,
        };
        testMode ("CTS", new SquareCts(), key, iv, plaintext, ciphertext, iterations);
    } // testCts


    protected static void testCfb (int iterations)
        throws java.io.IOException
    {
        byte[] plaintext = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        };
    	byte[] ciphertext = {
    		(byte)0xff, (byte)0x58, (byte)0x6d, (byte)0xa5,
    		(byte)0x6c, (byte)0xba, (byte)0xc5, (byte)0x06,
    		(byte)0x4a, (byte)0x09, (byte)0xa4, (byte)0x0a,
    		(byte)0xee, (byte)0xb6, (byte)0xae, (byte)0xaf,
    		(byte)0xd5, (byte)0xcb, (byte)0x53, (byte)0x8e,
    		(byte)0xea, (byte)0x28, (byte)0x97, (byte)0x4f,
    		(byte)0x7c, (byte)0x75, (byte)0xe7, (byte)0x9b,
    		(byte)0xcb, (byte)0x0d, (byte)0x4d, (byte)0x0e,
    	};
        testMode ("CFB", new SquareCfb(), key, iv, plaintext, ciphertext, iterations);
    } // testCfb


    protected static void testOfb (int iterations)
        throws java.io.IOException
    {
        byte[] plaintext = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        };
    	byte[] ciphertext = {
    		(byte)0xff, (byte)0x58, (byte)0x6d, (byte)0xa5,
    		(byte)0x6c, (byte)0xba, (byte)0xc5, (byte)0x06,
    		(byte)0x4a, (byte)0x09, (byte)0xa4, (byte)0x0a,
    		(byte)0xee, (byte)0xb6, (byte)0xae, (byte)0xaf,
		    (byte)0x35, (byte)0xc8, (byte)0x33, (byte)0xd3,
		    (byte)0x5c, (byte)0x29, (byte)0x44, (byte)0x37,
		    (byte)0x35, (byte)0xd2, (byte)0x25, (byte)0xbc,
		    (byte)0x95, (byte)0x28, (byte)0xc3, (byte)0xc8,
    	};
        testMode ("OFB", new SquareOfb(), key, iv, plaintext, ciphertext, iterations);
    } // testOfb


    public static void main (String argv[])
        throws java.io.IOException
    {
    	try {
            testRaw (1000000);
            testEcb (0);
            testCbc (0);
            testCts (0);
            testCfb (0);
            testOfb (0);
            System.out.println ("Done, press any key...");
    	} catch (Exception e) {
    	    System.out.println ("Exception caught: " + e.toString());
        }
        System.in.read ();
    } // main

} // SquareTest
