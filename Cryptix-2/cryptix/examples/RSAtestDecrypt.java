/**
 * RSA-decrypts a file from disk to standard output, using a
 * secret key generated by <code>RSAtestKeyGen</code>.
 * @author Patrick 
 * @version 1.0 4/21/97
 * @see RSAtestKeyGen RSAtestEncrypt  
 */
 
package cryptix.examples;
 
import cryptix.security.rsa.*;
import cryptix.security.*;
import cryptix.math.*;
import java.util.*;
import java.io.*;

class RSAtestDecrypt {

    static public void main (String[] args) {
        boolean debug = false;

        if ( args.length == 0 ) {
            System.out.println("Syntax: RSAtestDecrypt filename");
            System.exit(0);
        }

        // get the current time
        long st = (new Date()).getTime();

        // open the file to decrypt
        File file = new File(args[0]);
        DataInputStream dis = null;
        try {
            dis = new DataInputStream(new FileInputStream(file));
        } catch (IOException exception) {
            System.out.println(file+" not found");
            System.exit(0);
        }

        // get the secret key off disk
        Properties skProp = new Properties();
        try {
            FileInputStream skFis = new FileInputStream("secret.key");
            skProp.load(skFis);
        } catch (IOException exception) {
            System.out.println("Error reading secret Key");
        }
        BigInteger n = new BigInteger((String)skProp.getProperty("n","Error"));
        BigInteger e = new BigInteger((String)skProp.getProperty("e","Error"));
        BigInteger d = new BigInteger((String)skProp.getProperty("d","Error"));
        BigInteger p = new BigInteger((String)skProp.getProperty("p","Error"));
        BigInteger q = new BigInteger((String)skProp.getProperty("q","Error"));
        BigInteger u = new BigInteger((String)skProp.getProperty("u","Error"));

        // make the secretKey
        SecretKey sk = new SecretKey(n,e,d,p,q,u);
        if (debug) System.out.println("secret exponent = "+sk.e());
        if (debug) System.out.println("secret modulus = "+sk.n());

        // decrypt data
        String line = "";
        byte[] ba = null;
        try {
            while (true) {
                line = dis.readLine();
                if (line == null) break;
                ba = sk.decrypt(new BigInteger(line)).toByteArray();
                System.out.print(new String(ba,0));
            }
        } catch (IOException exception) {
            System.out.println("Error reading encrypted File");
        }

        // show duration
        if (debug) System.out.println("That took "+((new Date()).getTime()-st)/1000+" seconds");

    }
}