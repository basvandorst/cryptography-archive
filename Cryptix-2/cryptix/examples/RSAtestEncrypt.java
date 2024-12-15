/**
 * Encrypts an ASCII file from disk to standard output.<p>
 * You should redirect the output to another file for use with
 * <code>RSAtestDecrypt</code>.
 * @author Patrick
 * @version 1.0 4/21/97
 * @see RSAtestDecrypt RSAtestKeyGen
 */
 
package cryptix.examples;

import cryptix.security.rsa.*;
import cryptix.security.*;
import cryptix.math.*;
import java.util.*;
import java.io.*;

class RSAtestEncrypt {

    static public void main (String[] args) {
        boolean debug = false;

        if ( args.length == 0 ) {
            System.out.println("Syntax: RSAtestEncrypt filename [> newfile.rsa]");
            System.exit(0);
        }

        // get the current time
        long st = (new Date()).getTime();

        // open the file to encrypt
        File file = new File(args[0]);
        FileInputStream fis = null;
        try {
            fis = new FileInputStream(file);
        } catch (IOException exception) {
            System.out.println(file+" not found");
            System.exit(0);
        }

        // get the public key off disk
        Properties pkProp = new Properties();
        try {
            FileInputStream pkFis = new FileInputStream("public.key");
            pkProp.load(pkFis);
        } catch (IOException exception) {
            System.out.println("Error reading Public Key");
        }
        BigInteger e = new BigInteger((String)pkProp.getProperty("e","Error"));
        BigInteger n = new BigInteger((String)pkProp.getProperty("n","Error"));

        // make the PublicKey
        PublicKey pk = new PublicKey(n,e);
        if (debug) System.out.println("Public exponent = "+pk.e());
        if (debug) System.out.println("Public modulus = "+pk.n());

        // encrypt data
        byte[] ba = new byte[(pk.bitLength()/8)-8];
        int charsRead = 0;
        try {
            while ( (charsRead=fis.read(ba)) > -1 ) {
                if (charsRead < ba.length) {  // last chunk of file
                    byte[] shortBA = new byte[charsRead];
                    for (int i=0; i<charsRead; i++) {
                        shortBA[i] = ba[i];
                    }
                    System.out.println(pk.encrypt(new BigInteger(shortBA)));
                    break;
                }
                System.out.println(pk.encrypt(new BigInteger(ba)));
            }
        } catch (IOException exception) {
            System.out.println("Error reading File to encrypt");
        }

        // show duration
        if (debug) System.out.println("That took "+((new Date()).getTime()-st)/1000+" seconds");
    }
}
