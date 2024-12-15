/**
 * Illustrates how to make an RSA key pair.<p>
 * All output is written in ASCII for these <i>sample</i> programs
 * to allow humans to view the results.<p>
 * <code>java cryptix.examples.RSAtestKeyGen</code> generates a key pair
 * and writes it to disk in ASCII format.
 * @author Patrick 
 * @version 1.0 4/21/97
 * @see RSAtestDecrypt RSAtestEncrypt 
 */

package cryptix.examples;

import cryptix.security.rsa.*;
import cryptix.security.*;
import cryptix.math.*;
import java.util.*;
import java.io.*;

class RSAtestKeyGen {

    static public void main (String[] args) {

        boolean debug = false;
        int keySize = 512;

        // get a time stamp and make a random seed
        long st = (new Date()).getTime();
        Random rand = new Random(st);                   // no flames please...
        byte[] seed = new byte[50];                     // change seed size here
        for (int i=0; i< seed.length; i++) {
            seed[i] = (byte)((float)rand.nextFloat()*256);
        }

        // generate an RSA key pair
        CSRandomStream rs = new CSRandomStream(seed);
        RSAKeyGen rsaKG = new RSAKeyGen(rs);
        System.out.println("Generating a "+keySize+" bit key");
        SecretKey sk = rsaKG.createKey(rs,keySize);

        // dump it on screen
        if (debug) {
            System.out.println("Secret d = "+sk.d());
            System.out.println("Secret p = "+sk.p());
            System.out.println("Secret q = "+sk.q());
            System.out.println("Secret u = "+sk.u());
            System.out.println("Public exponent = "+sk.e());
            System.out.println("Public modulus = "+sk.n());
        }

        // save them to disk
        Properties secretKey = new Properties();
        secretKey.put("d",sk.d().toString());
        secretKey.put("p",sk.p().toString());
        secretKey.put("q",sk.q().toString());
        secretKey.put("u",sk.u().toString());
        secretKey.put("e",sk.e().toString());
        secretKey.put("n",sk.n().toString());

        try {
            FileOutputStream fos = new FileOutputStream("secret.key");
            secretKey.save(fos,"secret.key");
            System.out.println("Secret key written to secret.key");
        } catch (IOException e) {
            System.out.println("Error writing Secret Key");
        }

        Properties publicKey = new Properties();
        publicKey.put("e",sk.e().toString());
        publicKey.put("n",sk.n().toString());
        try {
            FileOutputStream fos = new FileOutputStream("public.key");
            publicKey.save(fos,"public.key");
            System.out.println("Public key written to public.key");
        } catch (IOException e) {
            System.out.println("Error writing Public Key");
        }

        // show duration
        System.out.println("That took "+((new Date()).getTime()-st)/1000+" seconds");
    }
}
