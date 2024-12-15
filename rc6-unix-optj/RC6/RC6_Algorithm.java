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

// This file contains an optimized Java implementation of RC6.



package RC6;

import java.security.InvalidKeyException;

public final class RC6_Algorithm {

  // A convenient table of values for f(x) = (x+1) mod 36
  private static final byte[] nextScheduleDwordIndex = {
     1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,  0
  };



  // Make a key schedule object which can be passed to encrypt(),
  // decrypt(), blockEncrypt(), or blockDecrypt() to perform
  // encryption or decryption.
  public static synchronized Object makeKey(byte[] key)
  throws InvalidKeyException {
    int keyLength = key.length;
    if (keyLength > 255)
      throw new InvalidKeyException();


    // Compute how many dwords of key we need
    int c = ((keyLength+3) >> 2)
           +((keyLength-1) >>> 31);


    int L[] = new int[c];
    short nextKeyDwordIndex[] = new short[c];


    // Load all the key dwords that get a full 4 bytes of key.  At
    // the same time, set up a table of values for g(x) = (x+1) mod c.
    int almostEnd = keyLength & 0xfffffffc;
    for (int i = almostEnd-4; i >= 0; i -= 4) {
      int index = i >> 2;
      L[index] = ((((int) key[i]) & 0xff))
                +((((int) key[i+1]) & 0xff) << 8)
                +((((int) key[i+2]) & 0xff) << 16)
                +((((int) key[i+3]) & 0xff) << 24);
      nextKeyDwordIndex[index] = (short) (index+1);
    }


    // Now fill up last dword
    int remainder = keyLength & 3;
    if (remainder > 0) {
      int lastDword = ((int) key[almostEnd]) & 0xff;
      if (remainder > 1) {
	    lastDword |= (((int) key[almostEnd+1]) & 0xff) << 8;
        if (remainder > 2)
          lastDword |= (((int) key[almostEnd+2]) & 0xff) << 16;
      }

      L[c-1] = lastDword;
    }


    // Finish setting up table by setting last value
    nextKeyDwordIndex[c-1] = 0;


    // Key schedule table.  This starts out having the values generated
    // from the magic constants P32 and Q32.
    int[] S = {
      0xb7e15163, 0x5618cb1c,  // Pseudo-round #0
      0xf45044d5, 0x9287be8e,  // Round #1
      0x30bf3847, 0xcef6b200,  // Round #2
      0x6d2e2bb9, 0x0b65a572,  // Round #3
      0xa99d1f2b, 0x47d498e4,  // Round #4
      0xe60c129d, 0x84438c56,  // Round #5
      0x227b060f, 0xc0b27fc8,  // Round #6
      0x5ee9f981, 0xfd21733a,  // Round #7
      0x9b58ecf3, 0x399066ac,  // Round #8
      0xd7c7e065, 0x75ff5a1e,  // Round #9
      0x1436d3d7, 0xb26e4d90,  // Round #10
      0x50a5c749, 0xeedd4102,  // Round #11
      0x8d14babb, 0x2b4c3474,  // Round #12
      0xc983ae2d, 0x67bb27e6,  // Round #13
      0x05f2a19f, 0xa42a1b58,  // Round #14
      0x42619511, 0xe0990eca,  // Round #15
      0x7ed08883, 0x1d08023c,  // Round #16
      0xbb3f7bf5, 0x5976f5ae,  // Round #17
      0xf7ae6f67, 0x95e5e920,  // Round #18
      0x341d62d9, 0xd254dc92,  // Round #19
      0x708c564b, 0x0ec3d004,  // Round #20
      0xacfb49bd, 0x4b32c376   // Pseudo-round #21
    };


    int i = 0;
    int j = 0;
    int A = 0;
    int B = 0;

    int sum;

    // Now we actually mix the key into the key schedule array
    for (int counter = ((c <= 44) ? 44 : c); --counter >= 0; ) {

      // A = S[i] = (S[i]+A+B) rotated left by 3
      A += S[i]+B;
      S[i] = A = (A << 3) | (A >>> 29);

      // B = L[j] = (L[j]+A+B) rotated left by (A+B)
      sum = A+B;
      B = sum+L[j];
      L[j] = B = (B << sum) | (B >>> (-sum));

      // i = (i+1) mod t and j = (j+1) mod c
      i = nextScheduleDwordIndex[i];
      j = nextKeyDwordIndex[j];


      // A = S[i] = (S[i]+A+B) rotated left by 3
      A += S[i]+B;
      S[i] = A = (A << 3) | (A >>> 29);

      // B = L[j] = (L[j]+A+B) rotated left by (A+B)
      sum = A+B;
      B = sum+L[j];
      L[j] = B = (B << sum) | (B >>> (-sum));

      // i = (i+1) mod t and j = (j+1) mod c
      i = nextScheduleDwordIndex[i];
      j = nextKeyDwordIndex[j];


      // A = S[i] = (S[i]+A+B) rotated left by 3
      A += S[i]+B;
      S[i] = A = (A << 3) | (A >>> 29);

      // B = L[j] = (L[j]+A+B) rotated left by (A+B)
      sum = A+B;
      B = sum+L[j];
      L[j] = B = (B << sum) | (B >>> (-sum));

      // i = (i+1) mod t and j = (j+1) mod c
      i = nextScheduleDwordIndex[i];
      j = nextKeyDwordIndex[j];
    }


    KeySchedule keySchedule = new KeySchedule();
    keySchedule.S0 = S[0];
    keySchedule.S1 = S[1];

    int count = 0;
    int offset = 1;
    do {
      KeySchedule.FourRounds fourRounds = keySchedule.middle[count++];
      fourRounds.Sp0 = S[++offset];
      fourRounds.Sp1 = S[++offset];
      fourRounds.Sp2 = S[++offset];
      fourRounds.Sp3 = S[++offset];
      fourRounds.Sp4 = S[++offset];
      fourRounds.Sp5 = S[++offset];
      fourRounds.Sp6 = S[++offset];
      fourRounds.Sp7 = S[++offset];
    } while (offset <= 33);

    keySchedule.S42 = S[42];
    keySchedule.S43 = S[43];
    return keySchedule;
  }



  // Encrypt a block of plaintext.
  static void encrypt(TextDwords plaintext, TextDwords ciphertext,
                      KeySchedule keySchedule) {
    // Get A, B, C, D and do pseudo-round #0
    int B = plaintext.B+keySchedule.S0;
    int D = plaintext.D+keySchedule.S1;
    int A = plaintext.A;
    int C = plaintext.C;

    int t,u;


    int i = 0;
    do {
      KeySchedule.FourRounds fourRounds = keySchedule.middle[i];


      // Round #1, #5, #9, #13, #17
      t = B*((B << 1)+1);
      u = D*((D << 1)+1);

      t = (t << 5) | (t >>> 27);
      u = (u << 5) | (u >>> 27);

      A ^= t;
      C ^= u;
      A = ((A << u) | (A >>> -u))+fourRounds.Sp0;
      C = ((C << t) | (C >>> -t))+fourRounds.Sp1;


      // Round #2, #6, #10, #14, #18
      t = C*((C << 1)+1);
      u = A*((A << 1)+1);

      t = (t << 5) | (t >>> 27);
      u = (u << 5) | (u >>> 27);

      B ^= t;
      D ^= u;
      B = ((B << u) | (B >>> -u))+fourRounds.Sp2;
      D = ((D << t) | (D >>> -t))+fourRounds.Sp3;


      // Round #3, #7, #11, #15, #19
      t = D*((D << 1)+1);
      u = B*((B << 1)+1);

      t = (t << 5) | (t >>> 27);
      u = (u << 5) | (u >>> 27);

      C ^= t;
      A ^= u;
      C = ((C << u) | (C >>> -u))+fourRounds.Sp4;
      A = ((A << t) | (A >>> -t))+fourRounds.Sp5;


      // Round #4, #8, #12, #16, #20
      t = A*((A << 1)+1);
      u = C*((C << 1)+1);

      t = (t << 5) | (t >>> 27);
      u = (u << 5) | (u >>> 27);

      D ^= t;
      B ^= u;
      D = ((D << u) | (D >>> -u))+fourRounds.Sp6;
      B = ((B << t) | (B >>> -t))+fourRounds.Sp7;


    } while (++i <= 4);


    // Do pseudo-round #21 and return ciphertext
    ciphertext.A = A+keySchedule.S42;
    ciphertext.B = B;
    ciphertext.C = C+keySchedule.S43;
    ciphertext.D = D;
  }



  // Decrypt a block of ciphertext
  static void decrypt(TextDwords ciphertext, TextDwords plaintext,
                      KeySchedule keySchedule) {
    // Get A, B, C, D and undo pseudo-round #21
    int C = ciphertext.C-keySchedule.S43;
    int A = ciphertext.A-keySchedule.S42;
    int B = ciphertext.B;
    int D = ciphertext.D;

    int t,u;


    int i = 4;
    do {
      KeySchedule.FourRounds fourRounds = keySchedule.middle[i];


      // Round #20, #16, #12, #8, #4
      t = A*((A << 1)+1);
      u = C*((C << 1)+1);
      t = (t << 5) | (t >>> 27);
      u = (u << 5) | (u >>> 27);

      B -= fourRounds.Sp7;
      D -= fourRounds.Sp6;
      B = ((B >>> t) | (B << -t)) ^u;
      D = ((D >>> u) | (D << -u)) ^t;


      // Round #19, #15, #11, #7, #3
      t = D*((D << 1)+1);
      u = B*((B << 1)+1);
      t = (t << 5) | (t >>> 27);
      u = (u << 5) | (u >>> 27);

      A -= fourRounds.Sp5;
      C -= fourRounds.Sp4;
      A = ((A >>> t) | (A << -t)) ^u;
      C = ((C >>> u) | (C << -u)) ^t;


      // Round #18, #14, #10, #6, #2
      t = C*((C << 1)+1);
      u = A*((A << 1)+1);
      t = (t << 5) | (t >>> 27);
      u = (u << 5) | (u >>> 27);

      D -= fourRounds.Sp3;
      B -= fourRounds.Sp2;
      D = ((D >>> t) | (D << -t)) ^u;
      B = ((B >>> u) | (B << -u)) ^t;


      // Round #17, #13, #9, #5, #1
      t = B*((B << 1)+1);
      u = D*((D << 1)+1);
      t = (t << 5) | (t >>> 27);
      u = (u << 5) | (u >>> 27);

      C -= fourRounds.Sp1;
      A -= fourRounds.Sp0;
      C = ((C >>> t) | (C << -t)) ^u;
      A = ((A >>> u) | (A << -u)) ^t;


    } while (--i >= 0);


    // Undo pseudo-round #0 and return plaintext
    plaintext.D = D-keySchedule.S1;
    plaintext.B = B-keySchedule.S0;
    plaintext.C = C;
    plaintext.A = A;
  }




  // The required basic AES Java API calls for encrypting and
  // decrypting are implemented below.



  // Encrypt a block of plaintext.
  public static byte[] blockEncrypt(byte[] in, int inOffset,
                                    Object sessionKey) {
    // Get plaintext ints
    TextDwords text = new TextDwords();
    text.fromBytes(in, inOffset);


    // Encrypt
    encrypt(text, text, (KeySchedule) sessionKey);


    // Return ciphertext bytes
    byte[] out = new byte[16];
    text.toBytes(out, 0);
    return out;
  }



  // Decrypt a block of plaintext.
  public static byte[] blockDecrypt(byte[] in, int inOffset,
                                    Object sessionKey) {
    // Get ciphertext ints
    TextDwords text = new TextDwords();
    text.fromBytes(in, inOffset);


    // Decrypt
    decrypt(text, text, (KeySchedule) sessionKey);


    // Return plaintext bytes
    byte[] out = new byte[16];
    text.toBytes(out, 0);
    return out;
  }



  // Do a self-test.  We're not actually going to test anything.
  public static boolean self_test() {
    return true;
  }
}
