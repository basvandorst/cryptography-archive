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

// This file contains an optimized Java implementation of RC6-CFB.




package RC6;

public final class RC6_CFB extends RC6_Mode {

  // No need for our constructor to accomplish anything.
  public RC6_CFB() {
  }



  // Update the ciphering object with more plaintext or ciphertext.
  public byte[] update(byte[] in, int inOff, int inLen) {
    int encryptOrDecrypt;

    switch (direction) {
      case NIST.NIST_Cipher.ENCRYPT_STATE:
        encryptOrDecrypt = 0x80;
        break;

      case NIST.NIST_Cipher.DECRYPT_STATE:
        encryptOrDecrypt = 0x00;
        break;

      default:
        throw new IllegalStateException();
    }


    byte[] out = new byte[inLen];
    TextDwords encryptedIv = new TextDwords();
    int outOff = 0;
    int outDword = 0;

    for ( ; --inLen >= 0; ) {
      int inDword = in[inOff++];
      int bit = 8;

      do {
        RC6_Algorithm.encrypt(iv, encryptedIv, keySchedule);
        outDword = (outDword << 1) |
                   (((inDword ^ encryptedIv.A) >>> 7) & 0x1);


        // Get feedback bit in bit position 7
        int feedback = inDword ^ (encryptedIv.A & encryptOrDecrypt);


        // Shift next input bit into place
        inDword <<= 1;


        // Left-shift the whole IV by 1 bit and add feedback bit
        iv.A = ((iv.A & 0x7f7f7f7f) << 1) |
               ((iv.A & 0x80808000) >>> 15) |
               ((iv.B & 0x80) << 17);
        iv.B = ((iv.B & 0x7f7f7f7f) << 1) |
               ((iv.B & 0x80808000) >>> 15) |
               ((iv.C & 0x80) << 17);
        iv.C = ((iv.C & 0x7f7f7f7f) << 1) |
               ((iv.C & 0x80808000) >>> 15) |
               ((iv.D & 0x80) << 17);
        iv.D = ((iv.D & 0x7f7f7f7f) << 1) |
               ((iv.D & 0x80808000) >>> 15) |
               ((feedback & 0x80) << 17);

      } while (--bit > 0);

      out[outOff++] = (byte) outDword;
    }


    return out;
  }



  // A way to check if there are no remnants left to process.
  protected void assertBufferIsEmpty() {
    // 1-bit CFB doesn't do any buffering
  }
}
