/****************************************************************
 *                                                              *
 *  This program is part of an implementation of the block      *
 *  cipher RC6(TM) that is being submitted to NIST as a         *
 *  candidate for the AES.                                      *
 *                                                              *
 *  Ronald L. Rivest and RSA Laboratories, the submitters of    *
 *  RC6, retain all rights to RC6 and to this code, except for  *
 *  those which NIST requires to be waived for submissions.     *
 *                                                              *
 *  Copyright (C) 1998 RSA Data Security, Inc.                  *
 *                                                              *
 ****************************************************************/

// This file contains an optimized Java implementation of RC6-CBC.




package RC6;

import java.security.InvalidParameterException;

public final class RC6_CBC extends RC6_Mode {

  // No need for our constructor to accomplish anything.
  public RC6_CBC() {
  }



  // Update the ciphering object with more plaintext or ciphertext.
  public byte[] update(byte[] in, int inOff, int inLen) {
    int ciphertextLength = (bytesInBuffer+inLen) & 0xfffffff0;
    byte[] out = new byte[ciphertextLength];
    TextDwords text = new TextDwords();
    int outOff = 0;
    TextDwords spareIv = new TextDwords();

    TextDwords thisIv = iv;
    TextDwords nextIv = spareIv;


    if (bytesInBuffer > 0) {
      // Not even a single block of text?
      if (ciphertextLength < 16) {
        System.arraycopy(in, inOff, buffer, bytesInBuffer, inLen);
        bytesInBuffer += inLen;
        return out;
      }

      int partLen = 16-bytesInBuffer;
      System.arraycopy(in, inOff, buffer, bytesInBuffer, partLen);
      inOff += partLen;
      inLen -= partLen;


      text.fromBytes(buffer, 0);


      // Handle initial partial block of text
      switch (direction) {
        case NIST.NIST_Cipher.ENCRYPT_STATE:
          // XOR in IV with plaintext
          text.A ^= iv.A;
          text.B ^= iv.B;
          text.C ^= iv.C;
          text.D ^= iv.D;

          // Encrypt
          RC6_Algorithm.encrypt(text, text, keySchedule);

          // Save ciphertext as new IV
          iv.A = text.A;
          iv.B = text.B;
          iv.C = text.C;
          iv.D = text.D;
          break;


        case NIST.NIST_Cipher.DECRYPT_STATE:
          // Save ciphertext as new IV
          spareIv.A = text.A;
          spareIv.B = text.B;
          spareIv.C = text.C;
          spareIv.D = text.D;

          // Decrypt
          RC6_Algorithm.decrypt(text, text, keySchedule);

          // XOR out IV to get plaintext
          text.A ^= iv.A;
          text.B ^= iv.B;
          text.C ^= iv.C;
          text.D ^= iv.D;

          // Modify pointer to new IV
          thisIv = spareIv;
          nextIv = iv;
          break;


        default:
          throw new IllegalStateException();
      }


      text.toBytes(out, 0);
      outOff = 16;
    }


    // Handle main body of text
    switch (direction) {
      case NIST.NIST_Cipher.ENCRYPT_STATE:
        for ( ; inLen >= 16; inLen -= 16) {
          text.fromBytes(in, inOff);

          // XOR in IV with plaintext
          text.A ^= iv.A;
          text.B ^= iv.B;
          text.C ^= iv.C;
          text.D ^= iv.D;

          // Encrypt
          RC6_Algorithm.encrypt(text, text, keySchedule);

          // Save ciphertext as new IV
          iv.A = text.A;
          iv.B = text.B;
          iv.C = text.C;
          iv.D = text.D;

          text.toBytes(out, outOff);
          inOff += 16;
          outOff += 16;
        }
        break;


      case NIST.NIST_Cipher.DECRYPT_STATE:
        for ( ; inLen >= 16; inLen -= 16) {
          text.fromBytes(in, inOff);

          // Save ciphertext as next block's IV
          nextIv.A = text.A;
          nextIv.B = text.B;
          nextIv.C = text.C;
          nextIv.D = text.D;

          // Decrypt
          RC6_Algorithm.decrypt(text, text, keySchedule);

          // XOR out IV to get plaintext
          text.A ^= thisIv.A;
          text.B ^= thisIv.B;
          text.C ^= thisIv.C;
          text.D ^= thisIv.D;
          text.toBytes(out, outOff);

          inOff += 16;
          outOff += 16;

          // Swap IVs
          TextDwords tempIv = nextIv;
          nextIv = thisIv;
          thisIv = tempIv;
        }
        break;


      default:
        throw new IllegalStateException();
    }


    // Copy the last remnant into the buffer
    System.arraycopy(in, inOff, buffer, 0, bytesInBuffer = inLen);


    // Make sure stored iv is correct
    iv = thisIv;


    return out;
  }



  // A way to check if there are no remnants left to process.
  protected void assertBufferIsEmpty() throws InvalidParameterException {
    if (bytesInBuffer != 0)
      throw new InvalidParameterException();
  }
}
