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

// This file contains part of an optimized Java implementation of RC6.




package RC6;

import java.security.InvalidKeyException;
import java.security.InvalidParameterException;


abstract class RC6_Mode implements NIST.NIST_CipherSpi {

  // Key schedule
  protected KeySchedule keySchedule;

  // ENCRYPT_STATE or DECRYPT_STATE
  protected int direction;

  // Some modes don't really need an IV, but we're going to include it
  // here anyway, since they all have to support the setIV() method.
  protected TextDwords iv = new TextDwords();


  // A buffer to hold as-yet unprocessed data.  Not all modes actually
  // need a buffer (in particular, 1-bit CFB doesn't).
  protected byte[] buffer = new byte[16];
  protected int bytesInBuffer;



  // No need for our protected constructor to accomplish anything.
  protected RC6_Mode() {
  }



  // Set the key and the direction.
  public void init(int state, byte[] key) throws InvalidKeyException {
    // Compute key schedule
    keySchedule = (KeySchedule) RC6_Algorithm.makeKey(key);


    direction = state;


    bytesInBuffer = 0;
  }



  // Set the IV.
  public void setIV(byte[] iv) throws InvalidParameterException {
    if (iv.length != 16)
      throw new InvalidParameterException();

    this.iv.fromBytes(iv, 0);
  }



  // A way to check if there are no remnants left to process.
  protected abstract void assertBufferIsEmpty()
  throws InvalidParameterException;



  // Finish up an encryption or decryption operation.
  public byte[] doFinal(byte[] in, int inOff, int inLen)
  throws InvalidParameterException {
    byte[] returnValue = update(in, inOff, inLen);
    assertBufferIsEmpty();

    return returnValue;
  }



  // Do a self-test.  Obviously, we're not actually going to test anything.
  public boolean self_test() {
    return true;
  }
}
