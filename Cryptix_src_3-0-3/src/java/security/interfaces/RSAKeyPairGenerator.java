/*
// $Log: RSAKeyPairGenerator.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package java.security.interfaces;

import java.math.BigInteger;
import java.security.SecureRandom;
import java.security.InvalidParameterException;

/**
 * An interface to an object capable of generating RSA key pairs.  The
 * generator is first initialized, then used to generate one or more
 * key pairs.
 * <p>
 * Users wishing to indicate the public exponent, and to generate a key 
 * pair suitable for use with the RSA algorithm typically
 * <ol>
 *   <li> Get a key pair generator for the RSA algorithm by calling the 
 *        KeyPairGenerator <code>getInstance</code> method with "RSA" 
 *        as its argument.<p> 
 *   <li> Initialize the generator by casting the result to an 
 *        RSAKeyPairGenerator and calling the <code>initialize</code> 
 *        method from this RSAKeyPairGenerator interface.<p>
 *   <li> Generate one or more key pairs by calling the 
 *        <code>generateKeyPair</code> method from the KeyPairGenerator 
 *        class, as often as desired.
 * </ol> 
 * <p>
 * Note: it is not always necessary to do algorithm-specific
 * initialization for an RSA key pair generator. That is, it is not always
 * necessary to call the <code>initialize</code> method in this interface.
 * Algorithm-independent initialization using the <code>initialize</code>
 * method in the KeyPairGenerator interface is all that is needed when you 
 * accept defaults for algorithm-specific parameters.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author David Hopwood
 * @since  IJCE 1.0
 * @see java.security.KeyPairGenerator
 */
public interface RSAKeyPairGenerator {
    /**
     * Initializes the key pair generator using the specified "strength"
     * (desired key length in bits), public exponent, and source of random
     * bits. Typical values for the public exponent are the Fermat primes
     * F4 and F0 (65537 and 3) and, less commonly, 17.
     */
    void initialize(int strength, BigInteger publicExponent,
        SecureRandom random) throws InvalidParameterException;
}
