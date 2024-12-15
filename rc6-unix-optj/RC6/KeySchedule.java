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

final class KeySchedule {
  static final class FourRounds {
    int Sp0, Sp1;
    int Sp2, Sp3;
    int Sp4, Sp5;
    int Sp6, Sp7;

    FourRounds() {
    }
  }

  int S0, S1;
  FourRounds[] middle = new FourRounds[5];
  int S42, S43;



  KeySchedule() {
    middle[0] = new FourRounds();
    middle[1] = new FourRounds();
    middle[2] = new FourRounds();
    middle[3] = new FourRounds();
    middle[4] = new FourRounds();
  }
}
