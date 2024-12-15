// $Id: $
//
// $Log: $
// Revision 1.1.1  1998/04/10  raif
// + added code to generate Intermediate Values KAT.
// + cosmetics.
//
// Revision 1.1  1998/04/07  Serpent authors
// + revised slightly (endianness, and key schedule for variable lengths)
//
// Revision 1.0  1998/04/06  raif
// + original version.
//
// $Endlog$
/*
 * Copyright (c) 1997, 1998 Systemics Ltd on behalf of
 * the Cryptix Development Team. All rights reserved.
 */
package Serpent;

import java.io.PrintWriter;
import java.security.InvalidKeyException;

//...........................................................................
/**
 * A standard implementation in Java of the Serpent cipher.<p>
 *
 * Serpent is a 128-bit 32-round block cipher with variable key lengths,
 * including 128-, 192- and 256-bit keys conjectured to be at least as
 * secure as three-key triple-DES.<p>
 *
 * Serpent was designed by Ross Anderson, Eli Biham and Lars Knudsen as a
 * candidate algorithm for the NIST AES Quest.<p>
 *
 * References:<ol>
 *  <li>Serpent: A New Block Cipher Proposal. This paper was published in the
 *  proceedings of the "Fast Software Encryption Workshop No. 5" held in
 *  Paris in March 1998. LNCS, Springer Verlag.<p>
 *  <li>Reference implementation of the standard Serpent cipher written in C
 *  by <a href="http://www.cl.cam.ac.uk/~fms/"> Frank Stajano</a>.</ol><p>
 *
 * <b>Copyright</b> &copy; 1997, 1998
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.<p>
 *
 * <b>$Revision: $</b>
 * @author  Raif S. Naffah
 * @author  Serpent authors (Ross Anderson, Eli Biham and Lars Knudsen)
 */
public final class Serpent_Standard // implicit no-argument constructor
{
// Debugging methods and variables
//...........................................................................

    static final String NAME = "Serpent_Standard";
    static final boolean IN = true, OUT = false;

    static final boolean DEBUG = Serpent_Properties.GLOBAL_DEBUG;

    /**
     * Depending on the value assigned to debuglevel, the following sensitive
     * information is displayed:
     *
     * 9    all intermediary key computation
     * 8    final session key values
     * 7    all block encryption/decryption intermediary values
     * 6    final block encryption/decryption values before final permutation
     */
    static final int debuglevel =
        DEBUG ? Serpent_Properties.getLevel("Serpent_Algorithm") : 0;
    static final PrintWriter err =
        DEBUG ? Serpent_Properties.getOutput() : null;

    static final boolean TRACE =
        Serpent_Properties.isTraceable("Serpent_Algorithm");

    static void debug (String s) { err.println(">>> "+NAME+": "+s); }
    static void trace (boolean in, String s) {
        if (TRACE) err.println((in?"==> ":"<== ")+NAME+"."+s);
    }
    static void trace (String s) { if (TRACE) err.println("<=> "+NAME+"."+s); }


// Constants and variables
//...........................................................................

    static final int BLOCK_SIZE =  16; // bytes in a data-block

    static final int ROUNDS = 32;              // nbr of rounds
    static final int PHI = 0x9E3779B9; // (sqrt(5) - 1) * 2**31

    /**
     * An array of 32 (number of rounds) S boxes.<p>
     *
     * An S box is an array of 16 distinct quantities, each in the range 0-15.
     * A value v at position p for a given S box, implies that if this S box
     * is given on input a value p, it will return the value v.
     */


    static final byte[][] Sbox = new byte[][] {
	{ 3, 8,15, 1,10, 6, 5,11,14,13, 4, 2, 7, 0, 9,12 },/* S0: */
	{15,12, 2, 7, 9, 0, 5,10, 1,11,14, 8, 6,13, 3, 4 },/* S1: */
	{ 8, 6, 7, 9, 3,12,10,15,13, 1,14, 4, 0,11, 5, 2 },/* S2: */
	{ 0,15,11, 8,12, 9, 6, 3,13, 1, 2, 4,10, 7, 5,14 },/* S3: */
	{ 1,15, 8, 3,12, 0,11, 6, 2, 5, 4,10, 9,14, 7,13 },/* S4: */
	{15, 5, 2,11, 4,10, 9,12, 0, 3,14, 8,13, 6, 7, 1 },/* S5: */
	{ 7, 2,12, 5, 8, 4, 6,11,14, 9, 1,15,13, 3,10, 0 },/* S6: */
	{ 1,13,15, 0,14, 8, 2,11, 7, 4,12,10, 9, 3, 5, 6 },/* S7: */
	{ 3, 8,15, 1,10, 6, 5,11,14,13, 4, 2, 7, 0, 9,12 },/* S0: */
	{15,12, 2, 7, 9, 0, 5,10, 1,11,14, 8, 6,13, 3, 4 },/* S1: */
	{ 8, 6, 7, 9, 3,12,10,15,13, 1,14, 4, 0,11, 5, 2 },/* S2: */
	{ 0,15,11, 8,12, 9, 6, 3,13, 1, 2, 4,10, 7, 5,14 },/* S3: */
	{ 1,15, 8, 3,12, 0,11, 6, 2, 5, 4,10, 9,14, 7,13 },/* S4: */
	{15, 5, 2,11, 4,10, 9,12, 0, 3,14, 8,13, 6, 7, 1 },/* S5: */
	{ 7, 2,12, 5, 8, 4, 6,11,14, 9, 1,15,13, 3,10, 0 },/* S6: */
	{ 1,13,15, 0,14, 8, 2,11, 7, 4,12,10, 9, 3, 5, 6 },/* S7: */
	{ 3, 8,15, 1,10, 6, 5,11,14,13, 4, 2, 7, 0, 9,12 },/* S0: */
	{15,12, 2, 7, 9, 0, 5,10, 1,11,14, 8, 6,13, 3, 4 },/* S1: */
	{ 8, 6, 7, 9, 3,12,10,15,13, 1,14, 4, 0,11, 5, 2 },/* S2: */
	{ 0,15,11, 8,12, 9, 6, 3,13, 1, 2, 4,10, 7, 5,14 },/* S3: */
	{ 1,15, 8, 3,12, 0,11, 6, 2, 5, 4,10, 9,14, 7,13 },/* S4: */
	{15, 5, 2,11, 4,10, 9,12, 0, 3,14, 8,13, 6, 7, 1 },/* S5: */
	{ 7, 2,12, 5, 8, 4, 6,11,14, 9, 1,15,13, 3,10, 0 },/* S6: */
	{ 1,13,15, 0,14, 8, 2,11, 7, 4,12,10, 9, 3, 5, 6 },/* S7: */
	{ 3, 8,15, 1,10, 6, 5,11,14,13, 4, 2, 7, 0, 9,12 },/* S0: */
	{15,12, 2, 7, 9, 0, 5,10, 1,11,14, 8, 6,13, 3, 4 },/* S1: */
	{ 8, 6, 7, 9, 3,12,10,15,13, 1,14, 4, 0,11, 5, 2 },/* S2: */
	{ 0,15,11, 8,12, 9, 6, 3,13, 1, 2, 4,10, 7, 5,14 },/* S3: */
	{ 1,15, 8, 3,12, 0,11, 6, 2, 5, 4,10, 9,14, 7,13 },/* S4: */
	{15, 5, 2,11, 4,10, 9,12, 0, 3,14, 8,13, 6, 7, 1 },/* S5: */
	{ 7, 2,12, 5, 8, 4, 6,11,14, 9, 1,15,13, 3,10, 0 },/* S6: */
	{ 1,13,15, 0,14, 8, 2,11, 7, 4,12,10, 9, 3, 5, 6 } /* S7: */
    };

    static final byte[][] SboxInverse = new byte[][] {
	{13, 3,11, 0,10, 6, 5,12, 1,14, 4, 7,15, 9, 8, 2 },/* InvS0: */
	{ 5, 8, 2,14,15, 6,12, 3,11, 4, 7, 9, 1,13,10, 0 },/* InvS1: */
	{12, 9,15, 4,11,14, 1, 2, 0, 3, 6,13, 5, 8,10, 7 },/* InvS2: */
	{ 0, 9,10, 7,11,14, 6,13, 3, 5,12, 2, 4, 8,15, 1 },/* InvS3: */
	{ 5, 0, 8, 3,10, 9, 7,14, 2,12,11, 6, 4,15,13, 1 },/* InvS4: */
	{ 8,15, 2, 9, 4, 1,13,14,11, 6, 5, 3, 7,12,10, 0 },/* InvS5: */
	{15,10, 1,13, 5, 3, 6, 0, 4, 9,14, 7, 2,12, 8,11 },/* InvS6: */
	{ 3, 0, 6,13, 9,14,15, 8, 5,12,11, 7,10, 1, 4, 2 },/* InvS7: */
	{13, 3,11, 0,10, 6, 5,12, 1,14, 4, 7,15, 9, 8, 2 },/* InvS0: */
	{ 5, 8, 2,14,15, 6,12, 3,11, 4, 7, 9, 1,13,10, 0 },/* InvS1: */
	{12, 9,15, 4,11,14, 1, 2, 0, 3, 6,13, 5, 8,10, 7 },/* InvS2: */
	{ 0, 9,10, 7,11,14, 6,13, 3, 5,12, 2, 4, 8,15, 1 },/* InvS3: */
	{ 5, 0, 8, 3,10, 9, 7,14, 2,12,11, 6, 4,15,13, 1 },/* InvS4: */
	{ 8,15, 2, 9, 4, 1,13,14,11, 6, 5, 3, 7,12,10, 0 },/* InvS5: */
	{15,10, 1,13, 5, 3, 6, 0, 4, 9,14, 7, 2,12, 8,11 },/* InvS6: */
	{ 3, 0, 6,13, 9,14,15, 8, 5,12,11, 7,10, 1, 4, 2 },/* InvS7: */
	{13, 3,11, 0,10, 6, 5,12, 1,14, 4, 7,15, 9, 8, 2 },/* InvS0: */
	{ 5, 8, 2,14,15, 6,12, 3,11, 4, 7, 9, 1,13,10, 0 },/* InvS1: */
	{12, 9,15, 4,11,14, 1, 2, 0, 3, 6,13, 5, 8,10, 7 },/* InvS2: */
	{ 0, 9,10, 7,11,14, 6,13, 3, 5,12, 2, 4, 8,15, 1 },/* InvS3: */
	{ 5, 0, 8, 3,10, 9, 7,14, 2,12,11, 6, 4,15,13, 1 },/* InvS4: */
	{ 8,15, 2, 9, 4, 1,13,14,11, 6, 5, 3, 7,12,10, 0 },/* InvS5: */
	{15,10, 1,13, 5, 3, 6, 0, 4, 9,14, 7, 2,12, 8,11 },/* InvS6: */
	{ 3, 0, 6,13, 9,14,15, 8, 5,12,11, 7,10, 1, 4, 2 },/* InvS7: */
	{13, 3,11, 0,10, 6, 5,12, 1,14, 4, 7,15, 9, 8, 2 },/* InvS0: */
	{ 5, 8, 2,14,15, 6,12, 3,11, 4, 7, 9, 1,13,10, 0 },/* InvS1: */
	{12, 9,15, 4,11,14, 1, 2, 0, 3, 6,13, 5, 8,10, 7 },/* InvS2: */
	{ 0, 9,10, 7,11,14, 6,13, 3, 5,12, 2, 4, 8,15, 1 },/* InvS3: */
	{ 5, 0, 8, 3,10, 9, 7,14, 2,12,11, 6, 4,15,13, 1 },/* InvS4: */
	{ 8,15, 2, 9, 4, 1,13,14,11, 6, 5, 3, 7,12,10, 0 },/* InvS5: */
	{15,10, 1,13, 5, 3, 6, 0, 4, 9,14, 7, 2,12, 8,11 },/* InvS6: */
	{ 3, 0, 6,13, 9,14,15, 8, 5,12,11, 7,10, 1, 4, 2 } /* InvS7: */
    };

    static final byte[] IPtable = new byte[] {
         0, 32, 64,  96,  1, 33, 65,  97,  2, 34, 66,  98,  3, 35, 67,  99,
         4, 36, 68, 100,  5, 37, 69, 101,  6, 38, 70, 102,  7, 39, 71, 103,
         8, 40, 72, 104,  9, 41, 73, 105, 10, 42, 74, 106, 11, 43, 75, 107,
        12, 44, 76, 108, 13, 45, 77, 109, 14, 46, 78, 110, 15, 47, 79, 111,
        16, 48, 80, 112, 17, 49, 81, 113, 18, 50, 82, 114, 19, 51, 83, 115,
        20, 52, 84, 116, 21, 53, 85, 117, 22, 54, 86, 118, 23, 55, 87, 119,
        24, 56, 88, 120, 25, 57, 89, 121, 26, 58, 90, 122, 27, 59, 91, 123,
        28, 60, 92, 124, 29, 61, 93, 125, 30, 62, 94, 126, 31, 63, 95, 127
    };

    static final byte[] FPtable = new byte[] {
         0,  4,  8, 12, 16, 20, 24, 28, 32,  36,  40,  44,  48,  52,  56,  60,
        64, 68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120, 124,
         1,  5,  9, 13, 17, 21, 25, 29, 33,  37,  41,  45,  49,  53,  57,  61,
        65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125,
         2,  6, 10, 14, 18, 22, 26, 30, 34,  38,  42,  46,  50,  54,  58,  62,
        66, 70, 74, 78, 82, 86, 90, 94, 98, 102, 106, 110, 114, 118, 122, 126,
         3,  7, 11, 15, 19, 23, 27, 31, 35,  39,  43,  47,  51,  55,  59,  63,
        67, 71, 75, 79, 83, 87, 91, 95, 99, 103, 107, 111, 115, 119, 123, 127
    };

    static final byte xFF = (byte) 0xFF;

    /**
     * The Linear Transformation is represented as an array of 128 rows,
     * one for each output bit. Each one of the 128 rows, terminated by a
     * marker (xFF) which isn't part of the data, is composed of up to 7
     * quantities in the range 0-127 specifying the positions of the input
     * bits that must be XORed together (say, 72, 144 and 125) to yield the
     * output bit corresponding to the position of that list (say, 1).
     */
    static final byte[][] LTtable = new byte[][] {
        {16,  52,  56,  70,  83,  94, 105, xFF},
        {72, 114, 125, xFF, xFF, xFF, xFF, xFF},
        { 2,   9,  15,  30,  76,  84, 126, xFF},
        {36,  90, 103, xFF, xFF, xFF, xFF, xFF},
        {20,  56,  60,  74,  87,  98, 109, xFF},
        { 1,  76, 118, xFF, xFF, xFF, xFF, xFF},
        { 2,   6,  13,  19,  34,  80,  88, xFF},
        {40,  94, 107, xFF, xFF, xFF, xFF, xFF},
        {24,  60,  64,  78,  91, 102, 113, xFF},
        { 5,  80, 122, xFF, xFF, xFF, xFF, xFF},
        { 6,  10,  17,  23,  38,  84,  92, xFF},
        {44,  98, 111, xFF, xFF, xFF, xFF, xFF},
        {28,  64,  68,  82,  95, 106, 117, xFF},
        { 9,  84, 126, xFF, xFF, xFF, xFF, xFF},
        {10,  14,  21,  27,  42,  88,  96, xFF},
        {48, 102, 115, xFF, xFF, xFF, xFF, xFF},
        {32,  68,  72,  86,  99, 110, 121, xFF},
        { 2,  13,  88, xFF, xFF, xFF, xFF, xFF},
        {14,  18,  25,  31,  46,  92, 100, xFF},
        {52, 106, 119, xFF, xFF, xFF, xFF, xFF},
        {36,  72,  76,  90, 103, 114, 125, xFF},
        { 6,  17,  92, xFF, xFF, xFF, xFF, xFF},
        {18,  22,  29,  35,  50,  96, 104, xFF},
        {56, 110, 123, xFF, xFF, xFF, xFF, xFF},
        { 1,  40,  76,  80,  94, 107, 118, xFF},
        {10,  21,  96, xFF, xFF, xFF, xFF, xFF},
        {22,  26,  33,  39,  54, 100, 108, xFF},
        {60, 114, 127, xFF, xFF, xFF, xFF, xFF},
        { 5,  44,  80,  84,  98, 111, 122, xFF},
        {14,  25, 100, xFF, xFF, xFF, xFF, xFF},
        {26,  30,  37,  43,  58, 104, 112, xFF},
        { 3, 118, xFF, xFF, xFF, xFF, xFF, xFF},
        { 9,  48,  84,  88, 102, 115, 126, xFF},
        {18,  29, 104, xFF, xFF, xFF, xFF, xFF},
        {30,  34,  41,  47,  62, 108, 116, xFF},
        { 7, 122, xFF, xFF, xFF, xFF, xFF, xFF},
        { 2,  13,  52,  88,  92, 106, 119, xFF},
        {22,  33, 108, xFF, xFF, xFF, xFF, xFF},
        {34,  38,  45,  51,  66, 112, 120, xFF},
        {11, 126, xFF, xFF, xFF, xFF, xFF, xFF},
        { 6,  17,  56,  92,  96, 110, 123, xFF},
        {26,  37, 112, xFF, xFF, xFF, xFF, xFF},
        {38,  42,  49,  55,  70, 116, 124, xFF},
        { 2,  15,  76, xFF, xFF, xFF, xFF, xFF},
        {10,  21,  60,  96, 100, 114, 127, xFF},
        {30,  41, 116, xFF, xFF, xFF, xFF, xFF},
        { 0,  42,  46,  53,  59,  74, 120, xFF},
        { 6,  19,  80, xFF, xFF, xFF, xFF, xFF},
        { 3,  14,  25, 100, 104, 118, xFF, xFF},
        {34,  45, 120, xFF, xFF, xFF, xFF, xFF},
        { 4,  46,  50,  57,  63,  78, 124, xFF},
        {10,  23,  84, xFF, xFF, xFF, xFF, xFF},
        { 7,  18,  29, 104, 108, 122, xFF, xFF},
        {38,  49, 124, xFF, xFF, xFF, xFF, xFF},
        { 0,   8,  50,  54,  61,  67,  82, xFF},
        {14,  27,  88, xFF, xFF, xFF, xFF, xFF},
        {11,  22,  33, 108, 112, 126, xFF, xFF},
        { 0,  42,  53, xFF, xFF, xFF, xFF, xFF},
        { 4,  12,  54,  58,  65,  71,  86, xFF},
        {18,  31,  92, xFF, xFF, xFF, xFF, xFF},
        { 2,  15,  26,  37,  76, 112, 116, xFF},
        { 4,  46,  57, xFF, xFF, xFF, xFF, xFF},
        { 8,  16,  58,  62,  69,  75,  90, xFF},
        {22,  35,  96, xFF, xFF, xFF, xFF, xFF},
        { 6,  19,  30,  41,  80, 116, 120, xFF},
        { 8,  50,  61, xFF, xFF, xFF, xFF, xFF},
        {12,  20,  62,  66,  73,  79,  94, xFF},
        {26,  39, 100, xFF, xFF, xFF, xFF, xFF},
        {10,  23,  34,  45,  84, 120, 124, xFF},
        {12,  54,  65, xFF, xFF, xFF, xFF, xFF},
        {16,  24,  66,  70,  77,  83,  98, xFF},
        {30,  43, 104, xFF, xFF, xFF, xFF, xFF},
        { 0,  14,  27,  38,  49,  88, 124, xFF},
        {16,  58,  69, xFF, xFF, xFF, xFF, xFF},
        {20,  28,  70,  74,  81,  87, 102, xFF},
        {34,  47, 108, xFF, xFF, xFF, xFF, xFF},
        { 0,   4,  18,  31,  42,  53,  92, xFF},
        {20,  62,  73, xFF, xFF, xFF, xFF, xFF},
        {24,  32,  74,  78,  85,  91, 106, xFF},
        {38,  51, 112, xFF, xFF, xFF, xFF, xFF},
        { 4,   8,  22,  35,  46,  57,  96, xFF},
        {24,  66,  77, xFF, xFF, xFF, xFF, xFF},
        {28,  36,  78,  82,  89,  95, 110, xFF},
        {42,  55, 116, xFF, xFF, xFF, xFF, xFF},
        { 8,  12,  26,  39,  50,  61, 100, xFF},
        {28,  70,  81, xFF, xFF, xFF, xFF, xFF},
        {32,  40,  82,  86,  93,  99, 114, xFF},
        {46,  59, 120, xFF, xFF, xFF, xFF, xFF},
        {12,  16,  30,  43,  54,  65, 104, xFF},
        {32,  74,  85, xFF, xFF, xFF, xFF, xFF},
        {36,  90, 103, 118, xFF, xFF, xFF, xFF},
        {50,  63, 124, xFF, xFF, xFF, xFF, xFF},
        {16,  20,  34,  47,  58,  69, 108, xFF},
        {36,  78,  89, xFF, xFF, xFF, xFF, xFF},
        {40,  94, 107, 122, xFF, xFF, xFF, xFF},
        { 0,  54,  67, xFF, xFF, xFF, xFF, xFF},
        {20,  24,  38,  51,  62,  73, 112, xFF},
        {40,  82,  93, xFF, xFF, xFF, xFF, xFF},
        {44,  98, 111, 126, xFF, xFF, xFF, xFF},
        { 4,  58,  71, xFF, xFF, xFF, xFF, xFF},
        {24,  28,  42,  55,  66,  77, 116, xFF},
        {44,  86,  97, xFF, xFF, xFF, xFF, xFF},
        { 2,  48, 102, 115, xFF, xFF, xFF, xFF},
        { 8,  62,  75, xFF, xFF, xFF, xFF, xFF},
        {28,  32,  46,  59,  70,  81, 120, xFF},
        {48,  90, 101, xFF, xFF, xFF, xFF, xFF},
        { 6,  52, 106, 119, xFF, xFF, xFF, xFF},
        {12,  66,  79, xFF, xFF, xFF, xFF, xFF},
        {32,  36,  50,  63,  74,  85, 124, xFF},
        {52,  94, 105, xFF, xFF, xFF, xFF, xFF},
        {10,  56, 110, 123, xFF, xFF, xFF, xFF},
        {16,  70,  83, xFF, xFF, xFF, xFF, xFF},
        { 0,  36,  40,  54,  67,  78,  89, xFF},
        {56,  98, 109, xFF, xFF, xFF, xFF, xFF},
        {14,  60, 114, 127, xFF, xFF, xFF, xFF},
        {20,  74,  87, xFF, xFF, xFF, xFF, xFF},
        { 4,  40,  44,  58,  71,  82,  93, xFF},
        {60, 102, 113, xFF, xFF, xFF, xFF, xFF},
        { 3,  18,  72, 114, 118, 125, xFF, xFF},
        {24,  78,  91, xFF, xFF, xFF, xFF, xFF},
        { 8,  44,  48,  62,  75,  86,  97, xFF},
        {64, 106, 117, xFF, xFF, xFF, xFF, xFF},
        { 1,   7,  22,  76, 118, 122, xFF, xFF},
        {28,  82,  95, xFF, xFF, xFF, xFF, xFF},
        {12,  48,  52,  66,  79,  90, 101, xFF},
        {68, 110, 121, xFF, xFF, xFF, xFF, xFF},
        { 5,  11,  26,  80, 122, 126, xFF, xFF},
        {32,  86,  99, xFF, xFF, xFF, xFF, xFF}
    };

    static final byte[][] LTtableInverse = new byte[][] {
        { 53,  55,  72, xFF, xFF, xFF, xFF, xFF},
        {  1,   5,  20,  90, xFF, xFF, xFF, xFF},
        { 15, 102, xFF, xFF, xFF, xFF, xFF, xFF},
        {  3,  31,  90, xFF, xFF, xFF, xFF, xFF},
        { 57,  59,  76, xFF, xFF, xFF, xFF, xFF},
        {  5,   9,  24,  94, xFF, xFF, xFF, xFF},
        { 19, 106, xFF, xFF, xFF, xFF, xFF, xFF},
        {  7,  35,  94, xFF, xFF, xFF, xFF, xFF},
        { 61,  63,  80, xFF, xFF, xFF, xFF, xFF},
        {  9,  13,  28,  98, xFF, xFF, xFF, xFF},
        { 23, 110, xFF, xFF, xFF, xFF, xFF, xFF},
        { 11,  39,  98, xFF, xFF, xFF, xFF, xFF},
        { 65,  67,  84, xFF, xFF, xFF, xFF, xFF},
        { 13,  17,  32, 102, xFF, xFF, xFF, xFF},
        { 27, 114, xFF, xFF, xFF, xFF, xFF, xFF},
        {  1,   3,  15,  20,  43, 102, xFF, xFF},
        { 69,  71,  88, xFF, xFF, xFF, xFF, xFF},
        { 17,  21,  36, 106, xFF, xFF, xFF, xFF},
        {  1,  31, 118, xFF, xFF, xFF, xFF, xFF},
        {  5,   7,  19,  24,  47, 106, xFF, xFF},
        { 73,  75,  92, xFF, xFF, xFF, xFF, xFF},
        { 21,  25,  40, 110, xFF, xFF, xFF, xFF},
        {  5,  35, 122, xFF, xFF, xFF, xFF, xFF},
        {  9,  11,  23,  28,  51, 110, xFF, xFF},
        { 77,  79,  96, xFF, xFF, xFF, xFF, xFF},
        { 25,  29,  44, 114, xFF, xFF, xFF, xFF},
        {  9,  39, 126, xFF, xFF, xFF, xFF, xFF},
        { 13,  15,  27,  32,  55, 114, xFF, xFF},
        { 81,  83, 100, xFF, xFF, xFF, xFF, xFF},
        {  1,  29,  33,  48, 118, xFF, xFF, xFF},
        {  2,  13,  43, xFF, xFF, xFF, xFF, xFF},
        {  1,  17,  19,  31,  36,  59, 118, xFF},
        { 85,  87, 104, xFF, xFF, xFF, xFF, xFF},
        {  5,  33,  37,  52, 122, xFF, xFF, xFF},
        {  6,  17,  47, xFF, xFF, xFF, xFF, xFF},
        {  5,  21,  23,  35,  40,  63, 122, xFF},
        { 89,  91, 108, xFF, xFF, xFF, xFF, xFF},
        {  9,  37,  41,  56, 126, xFF, xFF, xFF},
        { 10,  21,  51, xFF, xFF, xFF, xFF, xFF},
        {  9,  25,  27,  39,  44,  67, 126, xFF},
        { 93,  95, 112, xFF, xFF, xFF, xFF, xFF},
        {  2,  13,  41,  45,  60, xFF, xFF, xFF},
        { 14,  25,  55, xFF, xFF, xFF, xFF, xFF},
        {  2,  13,  29,  31,  43,  48,  71, xFF},
        { 97,  99, 116, xFF, xFF, xFF, xFF, xFF},
        {  6,  17,  45,  49,  64, xFF, xFF, xFF},
        { 18,  29,  59, xFF, xFF, xFF, xFF, xFF},
        {  6,  17,  33,  35,  47,  52,  75, xFF},
        {101, 103, 120, xFF, xFF, xFF, xFF, xFF},
        { 10,  21,  49,  53,  68, xFF, xFF, xFF},
        { 22,  33,  63, xFF, xFF, xFF, xFF, xFF},
        { 10,  21,  37,  39,  51,  56,  79, xFF},
        {105, 107, 124, xFF, xFF, xFF, xFF, xFF},
        { 14,  25,  53,  57,  72, xFF, xFF, xFF},
        { 26,  37,  67, xFF, xFF, xFF, xFF, xFF},
        { 14,  25,  41,  43,  55,  60,  83, xFF},
        {  0, 109, 111, xFF, xFF, xFF, xFF, xFF},
        { 18,  29,  57,  61,  76, xFF, xFF, xFF},
        { 30,  41,  71, xFF, xFF, xFF, xFF, xFF},
        { 18,  29,  45,  47,  59,  64,  87, xFF},
        {  4, 113, 115, xFF, xFF, xFF, xFF, xFF},
        { 22,  33,  61,  65,  80, xFF, xFF, xFF},
        { 34,  45,  75, xFF, xFF, xFF, xFF, xFF},
        { 22,  33,  49,  51,  63,  68,  91, xFF},
        {  8, 117, 119, xFF, xFF, xFF, xFF, xFF},
        { 26,  37,  65,  69,  84, xFF, xFF, xFF},
        { 38,  49,  79, xFF, xFF, xFF, xFF, xFF},
        { 26,  37,  53,  55,  67,  72,  95, xFF},
        { 12, 121, 123, xFF, xFF, xFF, xFF, xFF},
        { 30,  41,  69,  73,  88, xFF, xFF, xFF},
        { 42,  53,  83, xFF, xFF, xFF, xFF, xFF},
        { 30,  41,  57,  59,  71,  76,  99, xFF},
        { 16, 125, 127, xFF, xFF, xFF, xFF, xFF},
        { 34,  45,  73,  77,  92, xFF, xFF, xFF},
        { 46,  57,  87, xFF, xFF, xFF, xFF, xFF},
        { 34,  45,  61,  63,  75,  80, 103, xFF},
        {  1,   3,  20, xFF, xFF, xFF, xFF, xFF},
        { 38,  49,  77,  81,  96, xFF, xFF, xFF},
        { 50,  61,  91, xFF, xFF, xFF, xFF, xFF},
        { 38,  49,  65,  67,  79,  84, 107, xFF},
        {  5,   7,  24, xFF, xFF, xFF, xFF, xFF},
        { 42,  53,  81,  85, 100, xFF, xFF, xFF},
        { 54,  65,  95, xFF, xFF, xFF, xFF, xFF},
        { 42,  53,  69,  71,  83,  88, 111, xFF},
        {  9,  11,  28, xFF, xFF, xFF, xFF, xFF},
        { 46,  57,  85,  89, 104, xFF, xFF, xFF},
        { 58,  69,  99, xFF, xFF, xFF, xFF, xFF},
        { 46,  57,  73,  75,  87,  92, 115, xFF},
        { 13,  15,  32, xFF, xFF, xFF, xFF, xFF},
        { 50,  61,  89,  93, 108, xFF, xFF, xFF},
        { 62,  73, 103, xFF, xFF, xFF, xFF, xFF},
        { 50,  61,  77,  79,  91,  96, 119, xFF},
        { 17,  19,  36, xFF, xFF, xFF, xFF, xFF},
        { 54,  65,  93,  97, 112, xFF, xFF, xFF},
        { 66,  77, 107, xFF, xFF, xFF, xFF, xFF},
        { 54,  65,  81,  83,  95, 100, 123, xFF},
        { 21,  23,  40, xFF, xFF, xFF, xFF, xFF},
        { 58,  69,  97, 101, 116, xFF, xFF, xFF},
        { 70,  81, 111, xFF, xFF, xFF, xFF, xFF},
        { 58,  69,  85,  87,  99, 104, 127, xFF},
        { 25,  27,  44, xFF, xFF, xFF, xFF, xFF},
        { 62,  73, 101, 105, 120, xFF, xFF, xFF},
        { 74,  85, 115, xFF, xFF, xFF, xFF, xFF},
        {  3,  62,  73,  89,  91, 103, 108, xFF},
        { 29,  31,  48, xFF, xFF, xFF, xFF, xFF},
        { 66,  77, 105, 109, 124, xFF, xFF, xFF},
        { 78,  89, 119, xFF, xFF, xFF, xFF, xFF},
        {  7,  66,  77,  93,  95, 107, 112, xFF},
        { 33,  35,  52, xFF, xFF, xFF, xFF, xFF},
        {  0,  70,  81, 109, 113, xFF, xFF, xFF},
        { 82,  93, 123, xFF, xFF, xFF, xFF, xFF},
        { 11,  70,  81,  97,  99, 111, 116, xFF},
        { 37,  39,  56, xFF, xFF, xFF, xFF, xFF},
        {  4,  74,  85, 113, 117, xFF, xFF, xFF},
        { 86,  97, 127, xFF, xFF, xFF, xFF, xFF},
        { 15,  74,  85, 101, 103, 115, 120, xFF},
        { 41,  43,  60, xFF, xFF, xFF, xFF, xFF},
        {  8,  78,  89, 117, 121, xFF, xFF, xFF},
        {  3,  90, xFF, xFF, xFF, xFF, xFF, xFF},
        { 19,  78,  89, 105, 107, 119, 124, xFF},
        { 45,  47,  64, xFF, xFF, xFF, xFF, xFF},
        { 12,  82,  93, 121, 125, xFF, xFF, xFF},
        {  7,  94, xFF, xFF, xFF, xFF, xFF, xFF},
        {  0,  23,  82,  93, 109, 111, 123, xFF},
        { 49,  51,  68, xFF, xFF, xFF, xFF, xFF},
        {  1,  16,  86,  97, 125, xFF, xFF, xFF},
        { 11,  98, xFF, xFF, xFF, xFF, xFF, xFF},
        {  4,  27,  86,  97, 113, 115, 127, xFF}
    };

    private static final char[] HEX_DIGITS = {
        '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
    };


// Basic API methods
//...........................................................................

    /**
     * Expand a user-supplied key material into a session key.
     *
     * @param key  The user-key bytes (multiples of 4) to use.
     * @exception  InvalidKeyException  If the key is invalid.
     */
    public static synchronized Object makeKey (byte[] key)
    throws InvalidKeyException {
if (DEBUG) trace(IN, "makeKey("+key+")");
if (DEBUG && debuglevel > 7) {
System.out.println("Intermediate Standard Session Key Values");
System.out.println();
System.out.println("Raw="+toString(key));
}
        // compute prekeys w[]:
        // (a) from user key material
        int[] w = new int[4 * (ROUNDS + 1)];
        int offset = 0;
        int limit = key.length / 4;
        int i, j;
        for (i = 0; i < limit; i++)
            w[i] = (key[offset++] & 0xFF) |
                   (key[offset++] & 0xFF) <<  8 |
                   (key[offset++] & 0xFF) << 16 |
                   (key[offset++] & 0xFF) << 24;

        if (i < 8)
            w[i++] = 1;
//        for (; i < 8; i++)
//            w[i] = 0;

        // (b) and expanding them to full 132 x 32-bit material
        // this is a literal implementation of the Serpent paper
        // (section 4 The Key Schedule, p.226)
        int t;
        // start by computing the first 8 values using the second
        // lot of 8 values as an intermediary buffer
        for (i = 8, j = 0; i < 16; i++) {
            t = w[j] ^ w[i-5] ^ w[i-3] ^ w[i-1] ^ PHI ^ j++;
            w[i] = t << 11 | t >>> 21;
        }
        // translate the buffer by -8
        for (i = 0, j = 8; i < 8; ) w[i++] = w[j++];
        limit = 4 * (ROUNDS + 1); // 132 for a 32-round Serpent
        // finish computing the remaining intermediary subkeys
        for ( ; i < limit; i++) {
            t = w[i-8] ^ w[i-5] ^ w[i-3] ^ w[i-1] ^ PHI ^ i;
            w[i] = t << 11 | t >>> 21;
        }
if (DEBUG && debuglevel > 8) for(i=0;i<limit;i++) debug("w["+i+"]: "+intToString(w[i]));
        // compute intermediary key into k[]
        int[] k = new int[limit];
        int box, a, b, c, d, in, out;
        for (i = 0; i < ROUNDS + 1; i++) {
            box = (ROUNDS + 3 - i) % ROUNDS;
            a = w[4*i    ];
            b = w[4*i + 1];
            c = w[4*i + 2];
            d = w[4*i + 3];
            for (j = 0; j < 32; j++) {
                in = getBit(a, j)      |
                     getBit(b, j) << 1 |
                     getBit(c, j) << 2 |
                     getBit(d, j) << 3;
                out = S(box, in);
                k[4*i    ] |= getBit(out, 0) << j;
                k[4*i + 1] |= getBit(out, 1) << j;
                k[4*i + 2] |= getBit(out, 2) << j;
                k[4*i + 3] |= getBit(out, 3) << j;
            }
        }
        // renumber the 32-bit values k[] as 128-bit subkeys K[][]
        int[][] K = new int[ROUNDS + 1][4];
        for (i = 0, offset = 0; i < ROUNDS + 1; i++) {
            K[i][0] = k[offset++];
            K[i][1] = k[offset++];
            K[i][2] = k[offset++];
            K[i][3] = k[offset++];
if (DEBUG && debuglevel > 8) debug("K["+i+"]: "+toReversedString(K[i]));
        }
        // we now apply IP to the round key in order to place the key bits
        // in the correct column; ie. Khat[i] = IP(K[i]) --we use same K
        for (i = 0; i < ROUNDS + 1; i++) {
            K[i] = IP(K[i]);
if (DEBUG && debuglevel > 7) debug("Khat["+i+"]: "+toReversedString(K[i]));
        }
if (DEBUG && debuglevel > 7) {
for (i=0;i<ROUNDS+1;i++) System.out.println("K"+i+"="+toString(K[i]));
System.out.println();
}
if (DEBUG) trace(OUT, "makeKey()");
        return K;
    }

    /**
     * Encrypt exactly one block of plaintext.
     *
     * @param  in         The plaintext.
     * @param  inOffset   Index of in from which to start considering data.
     * @param  sessionKey The session key to use for encryption.
     * @return The ciphertext generated from a plaintext using the session key.
     */
    public static byte[]
    blockEncrypt (byte[] in, int inOffset, Object sessionKey) {
if (DEBUG) trace(IN, "blockEncrypt("+toString(in)+", "+inOffset+", "+sessionKey+")");
        int[][] Khat = (int[][]) sessionKey;
        int[] x = {
            (in[inOffset++] & 0xFF)       | (in[inOffset++] & 0xFF) <<  8 |
            (in[inOffset++] & 0xFF) << 16 | (in[inOffset++] & 0xFF) << 24,
            (in[inOffset++] & 0xFF)       | (in[inOffset++] & 0xFF) <<  8 |
            (in[inOffset++] & 0xFF) << 16 | (in[inOffset++] & 0xFF) << 24,
            (in[inOffset++] & 0xFF)       | (in[inOffset++] & 0xFF) <<  8 |
            (in[inOffset++] & 0xFF) << 16 | (in[inOffset++] & 0xFF) << 24,
            (in[inOffset++] & 0xFF)       | (in[inOffset++] & 0xFF) <<  8 |
            (in[inOffset++] & 0xFF) << 16 | (in[inOffset++] & 0xFF) << 24
        };
        int[] Bhat = IP(x);
if (DEBUG && debuglevel > 6) System.out.println("IP(PT)="+toString(Bhat));
        for (int i = 0; i < ROUNDS; i++) {
            Bhat = R(i, Bhat, Khat);
if (DEBUG && debuglevel > 6) System.out.println("CT"+i+"="+toString(Bhat));
        }
if (DEBUG && debuglevel > 5) debug("Bhat[32]: "+toReversedString(Bhat));
        x = FP(Bhat);
        
        int a = x[0], b = x[1], c = x[2], d = x[3];
        byte[] result = new byte[] {
            (byte)(a), (byte)(a >>> 8), (byte)(a >>> 16), (byte)(a >>> 24),
            (byte)(b), (byte)(b >>> 8), (byte)(b >>> 16), (byte)(b >>> 24),
            (byte)(c), (byte)(c >>> 8), (byte)(c >>> 16), (byte)(c >>> 24),
            (byte)(d), (byte)(d >>> 8), (byte)(d >>> 16), (byte)(d >>> 24)
        };
if (DEBUG && debuglevel > 6) {
System.out.println("CT="+toString(result));
System.out.println();
}
if (DEBUG) trace(OUT, "blockEncrypt()");
        return result;
    }

    /**
     * Decrypt exactly one block of ciphertext.
     *
     * @param  in         The ciphertext.
     * @param  inOffset   Index of in from which to start considering data.
     * @param  sessionKey The session key to use for decryption.
     * @return The plaintext generated from a ciphertext using the session key.
     */
    public static byte[]
    blockDecrypt (byte[] in, int inOffset, Object sessionKey) {
if (DEBUG) trace(IN, "blockDecrypt("+toString(in)+", "+inOffset+", "+sessionKey+")");
        int[][] Khat = (int[][]) sessionKey;
        int[] x = {
            (in[inOffset++] & 0xFF)       | (in[inOffset++] & 0xFF) <<  8 |
            (in[inOffset++] & 0xFF) << 16 | (in[inOffset++] & 0xFF) << 24,
            (in[inOffset++] & 0xFF)       | (in[inOffset++] & 0xFF) <<  8 |
            (in[inOffset++] & 0xFF) << 16 | (in[inOffset++] & 0xFF) << 24,
            (in[inOffset++] & 0xFF)       | (in[inOffset++] & 0xFF) <<  8 |
            (in[inOffset++] & 0xFF) << 16 | (in[inOffset++] & 0xFF) << 24,
            (in[inOffset++] & 0xFF)       | (in[inOffset++] & 0xFF) <<  8 |
            (in[inOffset++] & 0xFF) << 16 | (in[inOffset++] & 0xFF) << 24
        };
        int[] Bhat = FPinverse(x);
if (DEBUG && debuglevel > 6) System.out.println("FP'(CT)="+toString(Bhat));
        for (int i = ROUNDS - 1; i >= 0; i--) {
            Bhat = Rinverse(i, Bhat, Khat);
if (DEBUG && debuglevel > 6) System.out.println("PT"+i+"="+toString(Bhat));
        }
if (DEBUG && debuglevel > 5) debug("Bhat[0]: "+toReversedString(Bhat));
        x = IPinverse(Bhat);

        int a = x[0], b = x[1], c = x[2], d = x[3];
        byte[] result = new byte[] {
            (byte)(a), (byte)(a >>> 8), (byte)(a >>> 16), (byte)(a >>> 24),
            (byte)(b), (byte)(b >>> 8), (byte)(b >>> 16), (byte)(b >>> 24),
            (byte)(c), (byte)(c >>> 8), (byte)(c >>> 16), (byte)(c >>> 24),
            (byte)(d), (byte)(d >>> 8), (byte)(d >>> 16), (byte)(d >>> 24)
        };
if (DEBUG && debuglevel > 6) {
System.out.println("PT="+toString(result));
System.out.println();
}
if (DEBUG) trace(OUT, "blockDecrypt()");
        return result;
    }



    public static byte[]
    blockDecryptGetP (int in, int val, int inOffset, Object sessionKey) {
        int[][] Khat = (int[][]) sessionKey;
        int[] x = {0, 0, 0, 0};
        int[] Bhat = FPinverse(x);
        for (int i = ROUNDS - 1; i >= 0; i--) {
            Bhat = Rinverse(i, Bhat, Khat, in, val);
        }
        x = IPinverse(Bhat);

        int a = x[0], b = x[1], c = x[2], d = x[3];
        byte[] result = new byte[] {
            (byte)(a), (byte)(a >>> 8), (byte)(a >>> 16), (byte)(a >>> 24),
            (byte)(b), (byte)(b >>> 8), (byte)(b >>> 16), (byte)(b >>> 24),
            (byte)(c), (byte)(c >>> 8), (byte)(c >>> 16), (byte)(c >>> 24),
            (byte)(d), (byte)(d >>> 8), (byte)(d >>> 16), (byte)(d >>> 24)
        };
if (DEBUG && debuglevel > 6) {
System.out.println("PT="+toString(result));
System.out.println();
}
if (DEBUG) trace(OUT, "blockDecrypt()");
        return result;
    }


// own methods
//...........................................................................

    /**
     * @return The bit value at position <code>i</code> in a 32-bit entity,
     *      where the least significant bit (the rightmost one) is at
     *      position 0.
     */
    private static int getBit (int x, int i) { return (x >>> i) & 0x01; }
    
    /**
     * @return The bit value at position <code>i</code> in an array of 32-bit
     *      entities, where the least significant 32-bit entity is at index
     *      position 0 and the least significant bit (the rightmost one) in
     *      any 32-bit entity is at position 0.
     */
    private static int getBit (int[] x, int i) {
        return (x[i / 32] >>> (i % 32)) & 0x01;
    }

    /**
     * Set the bit at position <code>i</code> in an array of 32-bit entities
     * to a given value <code>v</code>, where the least significant 32-bit
     * entity is at index position 0 and the least significant bit (the
     * rightmost one) in any 32-bit entity is at position 0.
     */
    private static void setBit (int[] x, int i, int v) {
        if ((v & 0x01) == 1)
            x[i / 32] |= 1 << (i % 32); // set it
        else
            x[i / 32] &= ~(1 << (i % 32)); // clear it
    }

    /**
     * @return The nibble --a 4-bit entity-- in <code>x</code> given its
     *      position <code>i</code>, where the least significant nibble
     *      (the rightmost one) is at position 0.
     */
    private static int getNibble (int x, int i) { return (x >>> (4 * i)) & 0x0F; }

    /**
     * @return A 128-bit entity which is the result of applying the Initial
     *      Permutation (IP) to a 128-bit entity <code>x</code>.
     */
    private static int[] IP (int[] x) { return permutate(IPtable, x); }

    /**
     * @return A 128-bit entity which is the result of applying the inverse of
     *      the Initial Permutation to a 128-bit entity <code>x</code>.
     */
    private static int[] IPinverse (int[] x) { return permutate(FPtable, x); }

    /**
     * @return A 128-bit entity which is the result of applying the Final
     *      Permutation (FP) to a 128-bit entity <code>x</code>.
     */
    private static int[] FP (int[] x) { return permutate(FPtable, x); }

    /**
     * @return A 128-bit entity which is the result of applying the inverse of
     *      the Final Permutation to a 128-bit entity <code>x</code>.
     */
    private static int[] FPinverse (int[] x) { return permutate(IPtable, x); }
    
    /**
     * @return A 128-bit entity which is the result of applying a permutation
     *      coded in a given table <code>T</code> to a 128-bit entity
     *      <code>x</code>.
     */
    private static int[] permutate (byte[] T, int[] x) {
        int[] result = new int[4];
        for (int i = 0;  i < 128; i++)
            setBit(result, i, getBit(x, T[i] & 0x7F));
        return result;
    }

    /**
     * @return A 128-bit entity as the result of XORing, bit-by-bit, two given
     *      128-bit entities <code>x</code> and <code>y</code>.
     */
    private static int[] xor128 (int[] x, int[] y) {
        return new int[] {x[0] ^ y[0], x[1] ^ y[1], x[2] ^ y[2], x[3] ^ y[3]};
    }

    /**
     * @return The nibble --a 4-bit entity-- obtained by applying a given
     *      S-box to a 32-bit entity <code>x</code>.
     */
    private static int S (int box, int x) { return Sbox[box][x] & 0x0F; }

    /**
     * @return The nibble --a 4-bit entity-- obtained byapplying the inverse
     *      of a given S-box to a 32-bit entity <code>x</code>.
     */
    private static int Sinverse (int box, int x) { return SboxInverse[box][x] & 0x0F; }

    /**
     * @return A 128-bit entity being the result of applying, in parallel,
     *      32 copies of a given S-box to a 128-bit entity <code>x</code>.
     */
    private static int[] Shat (int box, int[] x) {
        int[] result = new int[4];
        for (int i = 0; i < 4; i++)
            for (int nibble = 0; nibble < 8; nibble++)
                result[i] |= S(box, getNibble(x[i], nibble)) << (nibble * 4);
        return result;
    }

    /**
     * @return A 128-bit entity being the result of applying, in parallel,
     *      32 copies of the inverse of a given S-box to a 128-bit entity
     *      <code>x</code>.
     */
    private static int[] ShatInverse (int box, int[] x) {
        int[] result = new int[4];
        for (int i = 0; i < 4; i++)
            for (int nibble = 0; nibble < 8; nibble++)
                result[i] |= Sinverse(box, getNibble(x[i], nibble)) << (nibble * 4);
        return result;
    }

    /**
     * @return A 128-bit entity being the result of applying the linear
     *      transformation to a 128-bit entity <code>x</code>.
     */
    private static int[] LT (int[] x) { return transform(LTtable, x); }

    /**
     * @return A 128-bit entity being the result of applying the inverse of
     *      the linear transformation to a 128-bit entity <code>x</code>.
     */
    private static int[] LTinverse (int[] x) {
        return transform(LTtableInverse, x);
    }

    /**
     * @return A 128-bit entity being the result of applying a transformation
     *      coded in a table <code>T</code> to a 128-bit entity <code>x</code>.
     *      Each row, of say index <code>i</code>, in <code>T</code> indicates
     *      the bits from <code>x</code> to be XORed together in order to
     *      produce the resulting bit at position <code>i</code>.
     */
    private static int[] transform (byte[][] T, int[] x) {
        int j, b;
        int[] result = new int[4];
        for (int i = 0; i < 128; i++) {
            b = 0;
            j = 0;
            while (T[i][j] != xFF) {
                b ^= getBit(x, T[i][j] & 0x7F);
                j++;
            }
            setBit(result, i, b);
        }
        return result;
    }

    /**
     * @return the 128-bit entity as the result of applying the round function
     *      R at round <code>i</code> to the 128-bit entity <code>Bhati</code>,
     *      using the appropriate subkeys from <code>Khat</code>.
     */
    private static int[] R (int i, int[] Bhati, int[][] Khat) {
if (DEBUG && debuglevel > 6) debug("Bhat["+i+"]: "+toReversedString(Bhati));
        int[] xored = xor128(Bhati, Khat[i]);
if (DEBUG && debuglevel > 6) debug("xored["+i+"]: "+toReversedString(xored));
        int[] Shati = Shat(i, xored);
if (DEBUG && debuglevel > 6) debug("Shat["+i+"]: "+toReversedString(Shati));
        int[] BhatiPlus1;
        if ((0 <= i) && (i <= ROUNDS - 2))
            BhatiPlus1 = LT(Shati);
        else if (i == ROUNDS - 1)
            BhatiPlus1 = xor128(Shati, Khat[ROUNDS]);
        else
            throw new RuntimeException(
                "Round "+i+" is out of 0.."+(ROUNDS-1)+" range");
        return BhatiPlus1;
    }

    /**
     * @return the 128-bit entity as the result of applying the inverse of
     *      the round function R at round <code>i</code> to the 128-bit
     *      entity <code>Bhati</code>, using the appropriate subkeys from
     *      <code>Khat</code>.
     */
    private static int[] Rinverse (int i, int[] BhatiPlus1, int[][] Khat) {
if (DEBUG && debuglevel > 6) debug("Bhat["+i+"+1]: "+toReversedString(BhatiPlus1));
        int[] Shati = new int[4];
        if ((0 <= i) && (i <= ROUNDS - 2))
            Shati = LTinverse(BhatiPlus1);
        else if (i == ROUNDS - 1)
            Shati = xor128(BhatiPlus1, Khat[ROUNDS]);
        else
            throw new RuntimeException(
                "Round "+i+" is out of 0.."+(ROUNDS-1)+" range");
if (DEBUG && debuglevel > 6) debug("Shat["+i+"]: "+toReversedString(Shati));
        int[] xored = ShatInverse(i, Shati);
if (DEBUG && debuglevel > 6) debug("xored["+i+"]: "+toReversedString(xored));
        int[] Bhati = xor128(xored, Khat[i]);
        return Bhati;
    }

    private static int[] Rinverse (int i, int[] BhatiPlus1, int[][] Khat, int in, int val) {
        int[] Shati = new int[4];
        if ((0 <= i) && (i <= ROUNDS - 2))
            Shati = LTinverse(BhatiPlus1);
        else if (i == ROUNDS - 1)
            Shati = xor128(BhatiPlus1, Khat[ROUNDS]);
        else
            throw new RuntimeException(
                "Round "+i+" is out of 0.."+(ROUNDS-1)+" range");
        int[] xored = ShatInverse(i, Shati);
	if(i==in) {
	  xored[0] = val | (val<<4);
	  xored[0] |= (xored[0]<<8);
	  xored[0] |= (xored[0]<<16);
	  xored[1] = xored[2] = xored[3] = xored[0];
	}
        int[] Bhati = xor128(xored, Khat[i]);
        return Bhati;
    }


// utility static methods (from cryptix.util.core.Hex class)
//...........................................................................

    /**
     * Returns a string of 8 hexadecimal digits (most significant
     * digit first) corresponding to the integer <i>n</i>, which is
     * treated as unsigned.
     */
    public static String intToString (int n) {
        char[] buf = new char[8];
        for (int i = 7; i >= 0; i--) {
            buf[i] = HEX_DIGITS[n & 0x0F];
            n >>>= 4;
        }
        return new String(buf);
    }

    /**
     * Returns a string of hexadecimal digits from a byte array. Each
     * byte is converted to 2 hex symbols.
     */
    private static String toString (byte[] ba) {
        int length = ba.length;
        char[] buf = new char[length * 2];
        for (int i = 0, j = 0, k; i < length; ) {
            k = ba[i++];
            buf[j++] = HEX_DIGITS[(k >>> 4) & 0x0F];
            buf[j++] = HEX_DIGITS[ k        & 0x0F];
        }
        return new String(buf);
    }

    /**
     * Returns a string of hexadecimal digits from an integer array. Each
     * int is converted to 4 hex symbols.
     */
    private static String toString (int[] ia) {
        int length = ia.length;
        char[] buf = new char[length * 8];
        for (int i = 0, j = 0, k; i < length; i++) {
            k = ia[i];
            buf[j++] = HEX_DIGITS[(k >>> 28) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>> 24) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>> 20) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>> 16) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>> 12) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>>  8) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>>  4) & 0x0F];
            buf[j++] = HEX_DIGITS[ k         & 0x0F];
        }
        return new String(buf);
    }


// other utility static methods
//...........................................................................

    /**
     * Returns an hexadecimal number (respresented as a string of hexadecimal 
     * digits from a byte array). Each byte is converted to 2 hex symbols.
     * The order is however, as of printing a number from a little-endian
     * internal representation (i.e., reverse order).
     */
    public static String toReversedString (byte[] ba) {
        int length = ba.length;
        char[] buf = new char[length * 2];
        for (int i = length-1, j = 0, k; i >=0; ) {
            k = ba[i--];
            buf[j++] = HEX_DIGITS[(k >>> 4) & 0x0F];
            buf[j++] = HEX_DIGITS[ k        & 0x0F];
        }
        return new String(buf);
    }

    /**
     * Returns a string of hexadecimal digits from an integer array. Each
     * int is converted to 4 hex symbols.
     */
    private static String toReversedString (int[] ia) {
        int length = ia.length;
        char[] buf = new char[length * 8];
        for (int i = length-1, j = 0, k; i >= 0; i--) {
            k = ia[i];
            buf[j++] = HEX_DIGITS[(k >>> 28) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>> 24) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>> 20) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>> 16) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>> 12) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>>  8) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>>  4) & 0x0F];
            buf[j++] = HEX_DIGITS[ k         & 0x0F];
        }
        return new String(buf);
    }
}
