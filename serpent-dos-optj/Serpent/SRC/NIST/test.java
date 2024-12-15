// $Id: $
//
// $Log: $
// Revision 1.0  1998/04/06  raif
// + original version based on cryptix.tools.KAT.
//
// $Endlog$
/*
 * Copyright (c) 1998 Systemics Ltd on behalf of
 * the Cryptix Development Team. All rights reserved.
 */
package NIST;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.StringTokenizer;

/**
 * For a designated candidate AES block cipher algorithm, this command
 * tests the speed of encryption.<p>
 *
 * This code processes the user's request using NIST Basic API.<p>
 *
 * <b>Copyright</b> &copy; 1998
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.<p>
 *
 * <b>$Revision: $</b>
 * @author  Raif S. Naffah
 * changed to NIST.test (from NIST.KAT) by E.B.
 */
public final class test
{
// main method
//...........................................................................
    
    public static void main (String[] args) {
        System.out.println(
            "measures NIST AES cipher speed\n\n");
        test cmd = new test();
        cmd.processOptions(args);
        cmd.run();
    }


// Fields & constants
//...........................................................................

    static final String VERSION = "$Revision: 1.0$";
    static final String SUBMITTER = "<as stated on the submission cover sheet>";

    // current values of switches as set from the command line arguments
    boolean varKey = false ;  // -k  generate variable-key data
    boolean varText = false ; // -t  generate variable-text data
    String dirName = null;    // -d  output directory if != user.dir
    String keylengths = null; // -l  comma-separated key lengths

    String cipherName = null; // cipher algorithm name, default == provider
    File destination = null;  // destination directory File object
    int[] keys = new int[] {128, 192, 256}; // key-length values to test with
    
    final String vkFileName = "ecb_vk.txt"; // variable-key output filename
    final String vtFileName = "ecb_vt.txt"; // variable-text output filename

    // statistics fields
    long encBlocks; // total count of encrypted blocks
    long decBlocks; // total count of decrypted blocks
    long keyCount;  // total count of key creation requests

    Method makeKey = null; // reference to makeKey([B)
    Method encrypt = null; // reference to blockEncrypt([B, int, int)
    Method decrypt = null; // reference to blockDecrypt([B, int, int)


// Own methods
//...........................................................................

    /** Process command line arguments and initialise instance fields. */
    private void processOptions (String[] args) {
        int argc = args.length;
        if (argc == 0)
            printUsage();
        System.out.println(
            "(type \"java NIST.test\" with no arguments for help)\n\n");
        int i = -1;
        String cmd = "";
        boolean next = true;
        while (true) {
            if (next) {
                i++;
                if (i >= argc)
                    break;
                else
                    cmd = args[i];
            } else
                cmd = "-" + cmd.substring(2);
            
            if (cmd.startsWith("-k")) {
                varKey = true;
                next = (cmd.length() == 2);
            } else if (cmd.startsWith("-t")) {
                varText = true;
                next = (cmd.length() == 2);
            } else if (cmd.startsWith("-l")) {       // key lengths
                keylengths = args[i + 1];
                i++;
                next = true;
            } else if (cmd.startsWith("-d")) {       // destination directory
                dirName = args[i + 1];
                i++;
                next = true;
            } else // it's the cipher
                cipherName = cmd;
        }
        // sanity checks
        if (cipherName == null)
            halt("Missing cipher algorithm name");
        if (cipherName.length() > 1 &&
                (cipherName.startsWith("\"") || cipherName.startsWith("'")))
            cipherName = cipherName.substring(2, cipherName.length() - 2);

        if (keylengths != null) {
            int count = 0;
            int k;
            int[] keystemp = new int[3]; // maximum allowed
            StringTokenizer st = new StringTokenizer(keylengths, ", \t\"");
            while (st.hasMoreTokens()) {
                k = Integer.parseInt(st.nextToken());
                if (k <= 0)
                    halt("Negative key length not allowed: "+k);
                if (count == 3)
                    halt("Only three key-length values are allowed.");
                keystemp[count++] = k;
            }
            if (count != 0) {
                keys = new int[count];
                System.arraycopy(keystemp, 0, keys, 0, count);
            }
        }

        if (!varKey && !varText)
            varKey = varText = true;

        if (dirName == null)
            dirName = System.getProperty("user.dir");
        destination = new File(dirName);
        if (! destination.isDirectory())
            halt("Destination <" + destination.getName() +
                "> is not a directory");

        String aes = cipherName + "." + cipherName + "_Algorithm";
        try {
            Class algorithm = Class.forName(aes);
            // inspect the _Algorithm class
            Method[] methods = algorithm.getDeclaredMethods();
            for (i = 0; i < methods.length; i++) {
                String name = methods[i].getName();
                int params = methods[i].getParameterTypes().length;
                if (name.equals("makeKey") && (params == 1))
                    makeKey = methods[i];
                else if (name.equals("blockEncrypt") && (params == 3))
                    encrypt = methods[i];
                else if (name.equals("blockDecrypt") && (params == 3))
                    decrypt = methods[i];
            }
            if (makeKey == null)
                throw new NoSuchMethodException("makeKey()");
            if (encrypt == null)
                throw new NoSuchMethodException("blockEncrypt()");
            if (decrypt == null)
                throw new NoSuchMethodException("blockDecrypt()");
        } catch (ClassNotFoundException x1) {
            halt("Unable to find "+aes+" class");
        } catch (NoSuchMethodException x2) {
            halt("Unable to find "+aes+"."+x2.getMessage()+" method");
        }
    }

    /**
     * Print an error message to System.err and halts execution returning
     * -1 to the JVM.
     *
     * @param s  A message to output on System.err
     */
    static void halt (String s) {
        System.err.println("\n*** "+s+"...");
        System.exit(-1);
    }

    /**
     * Write a notification message to System.out.
     *
     * @param s  String to output to System.out.
     */
    static void notify (String s) { System.out.println("test: "+s+"..."); }
    
    /** write help text and quit. */
    void printUsage() {
        System.out.println(
        "NAME\n" +
        "  test: measures the speed of an AES cipher algorithm.\n\n" +
        "SYNTAX\n" +
        "  java NIST.test\n" +
        "    <cipher>\n\n" +
        "OPTIONS\n" +
        "  <cipher>\n" +
        "       Cipher algorithm name.\n\n");
        System.exit(0);
    }

    /** main action. */
    void run() {
        long time = System.currentTimeMillis();

        notify("Java interpreter used: Version "+System.getProperty("java.version"));
        notify("Java Just-In-Time (JIT) compiler: "+System.getProperty("java.compiler"));
        notify("");
        notify("Encrypting 10000 blocks:");

	try {
	  teste();
	} catch (Exception x) {
            halt("Exception encountered in a " + cipherName +
                "_Algorithm method:\n" + x.getMessage());
        }

        // print timing and stats info
        notify("Total encryption time (ms): "+(System.currentTimeMillis() - time));
        notify("");
        notify("Decrypting 10000 blocks:");

	time = System.currentTimeMillis();

	try {
	  testd();
	} catch (Exception x) {
            halt("Exception encountered in a " + cipherName +
                "_Algorithm method:\n" + x.getMessage());
        }

        // print timing and stats info
        notify("Total decryption time (ms): "+(System.currentTimeMillis() - time));
    }


//...........................................................................

    void teste ()
    throws IllegalAccessException, InvocationTargetException {

        Object[] args = {};         //actual arguments
        int count = 10000;    // number of trial encryptions
        byte[] keyMaterial = new byte[16];
        byte[] pt = new byte[16];   // plaintext (16=default AES block size)
        byte[] cpt;                 // computed plaintext
        byte[] ct;                  // ciphertext
        int round = 0;              // current round ord. number
        Object skeys;               // algorithm secret key
        
	args = new Object[] { keyMaterial };
	skeys = makeKey.invoke(null, args);
        
	for (int i = 0; i < count; i++) {
	  args = new Object[] {pt, new Integer(0), skeys};
	  pt = (byte[]) encrypt.invoke(null, args);
        }
    }

    void testd ()
    throws IllegalAccessException, InvocationTargetException {

        Object[] args = {};         //actual arguments
        int count = 10000;    // number of trial encryptions
        byte[] keyMaterial = new byte[16];
        byte[] pt = new byte[16];   // plaintext (16=default AES block size)
        byte[] cpt;                 // computed plaintext
        byte[] ct;                  // ciphertext
        int round = 0;              // current round ord. number
        Object skeys;               // algorithm secret key
        
	args = new Object[] { keyMaterial };
	skeys = makeKey.invoke(null, args);
        
	for (int i = 0; i < count; i++) {
	  args = new Object[] {pt, new Integer(0), skeys};
	  pt = (byte[]) decrypt.invoke(null, args);
        }
    }




// utility static methods
// (copied from cryptix.util.core ArrayUtil and Hex classes)
//...........................................................................
    
    /**
     * Compares two byte arrays for equality.
     *
     * @return true if the arrays have identical contents
     */
    private static boolean areEqual (byte[] a, byte[] b) {
        int aLength = a.length;
        if (aLength != b.length)
            return false;
        for (int i = 0; i < aLength; i++)
            if (a[i] != b[i])
                return false;
        return true;
    }

    private static final char[] HEX_DIGITS = {
        '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
    };

    /**
     * Returns a string of hexadecimal digits from a byte array. Each
     * byte is converted to 2 hex symbols.
     */
    private static String toString (byte[] ba) {
        int length = ba.length;
        char[] buf = new char[length * 2];
        for (int i = length, j = 0, k; i > 0; ) {
            k = ba[--i];
            buf[j++] = HEX_DIGITS[(k >>> 4) & 0x0F];
            buf[j++] = HEX_DIGITS[ k        & 0x0F];
        }
        return new String(buf);
    }
}
