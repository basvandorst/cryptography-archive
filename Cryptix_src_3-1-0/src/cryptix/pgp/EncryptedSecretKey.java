/*
// $Log: EncryptedSecretKey.java,v $
// Revision 1.3  1999/07/12 20:27:13  edwin
// Basic support for reading (not writing) unencrypted secret keys.
//
// Revision 1.2  1999/06/30 23:02:00  edwin
// Fixing references to moved and/or changed classes. It should compile now with cryptix 3.0.x
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package cryptix.pgp;

import java.io.IOException;
import java.io.DataInput;
import java.io.DataOutput;

import cryptix.math.BigInteger;
import cryptix.math.MPI;
import cryptix.math.RandomStream;
import cryptix.security.StreamCipher;
import cryptix.util.Streamable;
import cryptix.security.IDEA;
import cryptix.security.rsa.*;

//
// This module still has IDEA hard coded all over the place
// This must be fixed
//

/**
 * An encrypted secret key class (used by SecretKeyCertificate).
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.3 $</b>
 * @author unattributed
 * @since  Cryptix 2.2 or earlier
 */
final class EncryptedSecretKey extends Streamable
{
    private BigInteger e_, n_;
    private EncryptedBigInt d_, p_, q_, u_;
    private BigInteger d, p, q, u;
    private boolean isEncrypted = true;
    private byte[] iv_; // defaults to null
    private int checksum_;

    public EncryptedSecretKey(SecretKey sk, Passphrase passphrase,
                               RandomStream rs)
    throws IOException
    {
        IDEA block_cipher = new IDEA(passphrase.md5hash());
        CFB cipher = new CFB(block_cipher);

        n_ = sk.n();
        e_ = sk.e();

        rs.read(iv_ = new byte[block_cipher.blockLength()]);
        cipher.encrypt(iv_);

        d_ = new EncryptedBigInt(sk.d(), cipher);
        p_ = new EncryptedBigInt(sk.p(), cipher);
        q_ = new EncryptedBigInt(sk.q(), cipher);
        u_ = new EncryptedBigInt(sk.u(), cipher);

        checksum_ = (d_.checksum() + p_.checksum() + q_.checksum() + u_.checksum()) & 0xFFFF;
    }

    public EncryptedSecretKey(byte[] buf)
    throws IOException
    {
        load(buf);
    }

    public EncryptedSecretKey(DataInput in)
    throws IOException
    {
        read(in);
    }

    public SecretKey secretKey(StreamCipher cipher)
    throws DecryptException
    {
        if (isEncrypted) {
            cipher.decrypt(iv_, new byte[iv_.length]);
    
            d = d_.decrypt(cipher);
            p = p_.decrypt(cipher);
            q = q_.decrypt(cipher);
            u = u_.decrypt(cipher);

            if ((((d_.checksum() + p_.checksum() + q_.checksum() + u_.checksum())) & 0xFFFF) != checksum_)
                throw new DecryptException("Check sum failure: passphrase wrong.");
        }
        
        return new SecretKey(n_, e_, d, p, q, u);
    }

    public void read(DataInput in)
    throws IOException
    {
        n_ = MPI.read(in);
        e_ = MPI.read(in);

        int encrypted = in.readByte();
        if (encrypted != 0)
        {
            if (encrypted != 1) // IDEA
                throw new FormatException("Unknown cipher type");

            in.readFully(iv_ = new byte [8]); // IV for cipher.

            d_ = new EncryptedBigInt(in);
            p_ = new EncryptedBigInt(in);
            q_ = new EncryptedBigInt(in);
            u_ = new EncryptedBigInt(in);

            checksum_ = in.readShort() & 0xFFFF;
        }
        else
        {
            isEncrypted = false;

            d = MPI.read(in);
            p = MPI.read(in);
            q = MPI.read(in);
            u = MPI.read(in);

            checksum_ = in.readShort() & 0xFFFF;
        }
    }

    public void write(DataOutput out)
    throws IOException
    {
        MPI.write(out, n_);
        MPI.write(out, e_);
        out.write(1);
        out.write(iv_);

        d_.write(out);
        p_.write(out);
        q_.write(out);
        u_.write(out);

        out.writeShort(checksum_);
    }

    /**
     * Gets the key ID. This is needed to index secret key certificates
     * by ID.
     *
     * @return the lower 8 bytes of n, the public modulus.
     */
    public final KeyID id()
    {
        byte[] nBuffer = n_.toByteArray();
        byte[] ident = new byte[8];
        System.arraycopy(nBuffer, nBuffer.length - 8, ident, 0, 8);
        return new KeyID(ident);
    }

    /**
     * Returns a string representation of only the public parts of this secret
     * key, in hex.
     */
    public String toString()
    {
        return "secret key public parts n:" + n_.toString() + " e:"+ e_.toString() + " all the rest is encrypted";
    }
    
    public boolean isEncrypted() { return isEncrypted; }
}
