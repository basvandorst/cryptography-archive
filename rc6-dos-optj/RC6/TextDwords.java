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

final class TextDwords {
  int A, B, C, D;



  TextDwords() {
  }



  // Convert an array of 16 bytes to ints.
  void fromBytes(byte[] in, int inOffset) {
    // Note that the following expressions have multiple side
    // effects, and that they depend on the order of evaluation
    // specified for Java.
    A = ((((int) in[inOffset]) & 0xff)) |
        ((((int) in[++inOffset]) & 0xff) << 8) |
        ((((int) in[++inOffset]) & 0xff) << 16) |
        ((((int) in[++inOffset]) & 0xff) << 24);

    B = ((((int) in[++inOffset]) & 0xff)) |
        ((((int) in[++inOffset]) & 0xff) << 8) |
        ((((int) in[++inOffset]) & 0xff) << 16) |
        ((((int) in[++inOffset]) & 0xff) << 24);

    C = ((((int) in[++inOffset]) & 0xff)) |
        ((((int) in[++inOffset]) & 0xff) << 8) |
        ((((int) in[++inOffset]) & 0xff) << 16) |
        ((((int) in[++inOffset]) & 0xff) << 24);

    D = ((((int) in[++inOffset]) & 0xff)) |
        ((((int) in[++inOffset]) & 0xff) << 8) |
        ((((int) in[++inOffset]) & 0xff) << 16) |
        ((((int) in[++inOffset]) & 0xff) << 24);
  }



  // Convert ints to an array of 16 bytes.
  void toBytes(byte[] out, int outOffset) {
    // Note that the following expressions have multiple side
    // effects, and that they depend on the order of evaluation
    // specified for Java.
    out[outOffset] = (byte) (A);
    out[++outOffset] = (byte) (A >>> 8);
    out[++outOffset] = (byte) (A >>> 16);
    out[++outOffset] = (byte) (A >>> 24);

    out[++outOffset] = (byte) (B);
    out[++outOffset] = (byte) (B >>> 8);
    out[++outOffset] = (byte) (B >>> 16);
    out[++outOffset] = (byte) (B >>> 24);

    out[++outOffset] = (byte) (C);
    out[++outOffset] = (byte) (C >>> 8);
    out[++outOffset] = (byte) (C >>> 16);
    out[++outOffset] = (byte) (C >>> 24);

    out[++outOffset] = (byte) (D);
    out[++outOffset] = (byte) (D >>> 8);
    out[++outOffset] = (byte) (D >>> 16);
    out[++outOffset] = (byte) (D >>> 24);
  }
}
