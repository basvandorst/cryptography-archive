      PROGRAM TESTMP
C
C   This is the test program for DHB's multiprecision computation package
C   MPFUN (32 bit version).  It exercises most routines and verifies that they
C   are working properly.  If any of these tests fail, it cannot possibly be
C   due to a bug in DHB's MPFUN package, so it must therefore be due to some
C   local hardware or compiler problem (smile).
C
C     David H. Bailey     May 7, 1993
C
      DOUBLE PRECISION BBX, BDX, BX2, RBX, RDX, RX2, RXX
      DOUBLE PRECISION DS, DSUM, T1, X1
      CHARACTER*1 Z
      CHARACTER*80 ZA
      PARAMETER (MX = 6, NX = 2 ** MX, L = 5, N = 8, N4 = NX + 4,       
     $  LN = L * N, MT = 10 - 6 * NX, NP = 20,                          
     $  NS = N4 * (4 * N**2 + 5 * N + 15), NT = 64)
      DIMENSION AA(L,N), AC(2*L,N), A(N4), AL2(N4), B(N4), DS(NT),      
     $  F1(5), F2(5), IP(NP), IS1(NP), IS2(9), NX1(2), PI(N4), X(2*N4), 
     $  XX(N4,NP), X1(2), Y(2*N4), Z(600)
      COMMON /MPCOM0/ BBX, BDX, BX2, RBX, RDX, RX2, RXX, NBT, NPR
      COMMON /MPCOM1/ NW, IDB, LDB, IER, MCR, IRD, ICS, IHS, IMS
      COMMON /MPCOM3/ S(NS)
      DATA AA /                                                         
     $   1., 0., 40., 0., 0., 1., 0., 24., 0., 0., -1., 0., 15., 0., 0.,
     $  -1., 0.,  8., 0., 0., 1., 0.,  5., 0., 0.,  1., 0.,  2., 0., 0.,
     $   0., 0.,  0., 0., 0., 1., 0.,  1., 0., 0./
      DATA AC /                                                         
     $   1., 0., 40., 0., 0., 5 * 0.,  1., 0., 24., 0., 0., 5 * 0.,     
     $  -1., 0., 15., 0., 0., 5 * 0., -1., 0.,  8., 0., 0., 5 * 0.,     
     $   1., 0.,  5., 0., 0., 5 * 0.,  1., 0.,  2., 0., 0., 5 * 0.,     
     $   0., 0.,  0., 0., 0., 5 * 0.,  1., 0.,  1., 0., 0., 5 * 0./
      DATA DS /                                                         
     $   71647124.D0,          0.D0,  532002022.D0,  520881213.D0,      
     $  535684389.D0,  466695539.D0,  525576020.D0,  448479226.D0,      
     $  598587746.D0,  577629931.D0,  544393923.D0,  527030594.D0,      
     $  550529242.D0,  523320411.D0,  536143302.D0,  516904795.D0,      
     $  522577789.D0,  501964664.D0,  564046428.D0,  533883779.D0,      
     $  560037982.D0,  539041558.D0,  485070596.D0,  465190050.D0,      
     $  534435267.D0,  521869037.D0,  535883606.D0,  516544718.D0,      
     $  501186251.D0,  490005323.D0,  471471731.D0,  457042260.D0,      
     $  531664183.D0,  531664183.D0,  525224768.D0,  525224768.D0,      
     $  573727184.D0,  512972074.D0,  549225360.D0,  497418453.D0,      
     $  510801511.D0,  520623984.D0,  501266282.D0,  502986625.D0,      
     $  526887598.D0,  515152633.D0,  470800624.D0,  448276146.D0,      
     $  501444259.D0,  483347713.D0,  515452421.D0,  585480619.D0,      
     $  479413814.D0,  549866295.D0,  561015607.D0,  603898762.D0,      
     $  550269122.D0,  550269122.D0,  529966523.D0,  503319486.D0,      
     $         92.D0,         92.D0,   2 * 0.D0/
      DATA IS1 /                                                        
     $  20,   1,  11,  10,  16,   5,  17,  12,  15,  13,   9,   4,      
     $   2,  18,   6,   3,   7,  19,   8,  14/
      DATA IS2 /                                                        
     $  64,   0,   6,   0,   5,   1,   1, 21149, 21148/
C
C   Initialize.  MCR is set to 5 so that the advanced routines can be tested
C   with reasonably short run times.
C
      NW = NX
      MCR = 5
      IMS = NS
      CALL MPINIX (MX)
C
C   Test the input/output conversion routines.
C
      KT = 1
      ZA = '10 ^ - 50 x - 3. 14159 26535 89793 23846 26433 83279 50288'
      READ (ZA, '(80A1)') (Z(I), I = 1, 80)
      CALL MPINPC (Z, 80, A)
      LS = 9
      IF (DSUM (LS, A) .NE. DS(KT)) CALL DERR (KT, LS, A)
      WRITE (6, 1) KT
 1    FORMAT ('COMPLETED TEST',I3)
C
      KT = 2
      ZA ='10 ^       -50 x -3.14159265358979323846264338327950288'
      CALL MPOUTC (A, Z, NN)
      NN = MIN (NN, 55)
C
      DO 100 J = 1, NN
        IF (Z(J) .NE. ZA(J:J)) THEN
          WRITE (6, 2) KT, (Z(I), I = 1, NN)
 2        FORMAT ('TESTMP FAILED ON TEST NO.',I4/'RESULT: ',60A1)
          GOTO 110
        ENDIF
 100  CONTINUE
C
 110  WRITE (6, 1) KT
C
C   Compute 3. ^ (-13).
C
      KT = 3
      F1(1) = 1.
      F1(2) = 0.
      F1(3) = 3.
      NN = -13
      LS = NW + 2
      CALL MPNPWR (F1, NN, A)
      IF (DSUM (LS, A) .NE. DS(KT)) CALL DERR (KT, LS, A)
      WRITE (6, 1) KT
C
      KT = 4
      LS = NW
      CALL MPNPWX (F1, NN, A)
      IF (DSUM (LS, A) .NE. DS(KT)) CALL DERR (KT, LS, A)
      WRITE (6, 1) KT
C
C   Compute (4 - i) ^ (-25).
C
      KT = 5
      F1(1) = 1.
      F1(2) = 0.
      F1(3) = 4.
      F2(1) = -1.
      F2(2) = 0.
      F2(3) = 1.
      CALL MPMMPC (F1, F2, N4, X)
      NN = -25
      LS = NW + 2
      CALL MPCPWR (N4, X, NN, Y)
      IF (DSUM (LS, Y) .NE. DS(KT)) CALL DERR (KT, LS, Y)
      WRITE (6, 1) KT
      KT = 6
      IF (DSUM (LS, Y(N4+1)) .NE. DS(KT)) CALL DERR (KT, LS, Y(N4+1))
      WRITE (6, 1) KT
C
      KT = 7
      LS = NW
      CALL MPCPWX (N4, X, NN, Y)
      IF (DSUM (LS, Y) .NE. DS(KT)) CALL DERR (KT, LS, Y)
      WRITE (6, 1) KT
      KT = 8
      IF (DSUM (LS, Y(N4+1)) .NE. DS(KT)) CALL DERR (KT, LS, Y(N4+1))
      WRITE (6, 1) KT
C
C   Compute Sqrt (Sqrt (10)).
C
      KT = 9
      F1(1) = 1.
      F1(2) = 0.
      F1(3) = 10.
      CALL MPSQRT (F1, A)
      CALL MPSQRT (A, B)
      LS = NW + 2
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
      KT = 10
      LS = NW
      CALL MPSQRX (A, B)
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
C   Compute Cbrt (Sqrt (10)).
C
      KT = 11
      CALL MPCBRT (A, B)
      LS = NW + 2
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
      KT = 12
      LS = NW
      CALL MPCBRX (A, B)
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
C   Compute the 10th root of 10.
C
      KT = 13
      CALL MPNRT (F1, 10, B)
      LS = NW + 2
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
      KT = 14
      LS = NW
      CALL MPNRTX (F1, 10, B)
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
C   Compute the complex square root of (2., 1.).
C
      KT = 15
      F1(1) = 1.
      F1(2) = 0.
      F1(3) = 2.
      F2(1) = 1.
      F2(2) = 0.
      F2(3) = 1.
      CALL MPMMPC (F1, F2, N4, X)
      CALL MPCSQR (N4, X, Y)
      LS = NW + 1
      IF (DSUM (LS, Y) .NE. DS(KT)) CALL DERR (KT, LS, Y)
      WRITE (6, 1) KT
      KT = 16
      IF (DSUM (LS, Y(N4+1)) .NE. DS(KT)) CALL DERR (KT, LS, Y(N4+1))
      WRITE (6, 1) KT
C
      KT = 17
      CALL MPCSQX (N4, X, Y)
      LS = NW
      IF (DSUM (LS, Y) .NE. DS(KT)) CALL DERR (KT, LS, Y)
      WRITE (6, 1) KT
      KT = 18
      IF (DSUM (LS, Y(N4+1)) .NE. DS(KT)) CALL DERR (KT, LS, Y(N4+1))
      WRITE (6, 1) KT
C
C   Compute Pi.
C
      KT = 19
      CALL MPPI (B)
      LS = NW + 2
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
      CALL MPEQ (B, PI)
C
      KT = 20
      LS = NW
      CALL MPPIX (B)
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
C   Compute Log (2).
C
      KT = 21
      F1(1) = 1.
      F1(2) = 0.
      F1(3) = 2.
      CALL MPLOG (F1, AL2, B)
      LS = NW + 2
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
      CALL MPEQ (B, AL2)
C
      KT = 22
      CALL MPLOGX (F1, PI, AL2, B)
      LS = NW - 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
C   Compute Log (10).
C
      KT = 23
      F1(3) = 10.
      CALL MPLOG (F1, AL2, B)
      LS = NW + 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
      KT = 24
      CALL MPLOGX (F1, PI, AL2, B)
      LS = NW - 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
C   Compute Log (1/4).
C
      KT = 25
      F1(2) = -1
      F1(3) = 0.25D0 * BDX
      CALL MPLOG (F1, AL2, B)
      LS = NW + 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
      KT = 26
      CALL MPLOGX (F1, PI, AL2, B)
      LS = NW - 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
C   Compute Exp (1).
C
      KT = 27
      F1(1) = 1.
      F1(2) = 0.
      F1(3) = 1.
      CALL MPEXP (F1, AL2, B)
      LS = NW + 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
      KT = 28
      CALL MPEXPX (F1, PI, AL2, B)
      LS = NW - 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
C   Compute Exp (2).
C
      KT = 29
      F1(3) = 2.
      CALL MPEXP (F1, AL2, B)
      LS = NW + 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
      KT = 30
      CALL MPEXPX (F1, PI, AL2, B)
      LS = NW - 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
C   Compute Exp (-5).
C
      KT = 31
      F1(1) = -1.
      F1(3) = 5.
      CALL MPEXP (F1, AL2, B)
      LS = NW + 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
      KT = 32
      CALL MPEXPX (F1, PI, AL2, B)
      LS = NW - 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
C   Compute Cos and Sin of Pi/4.
C
      KT = 33
      CALL MPMULD (PI, 0.25D0, 0, A)
      CALL MPCSSN (A, PI, X, Y)
      LS = NW + 1
      IF (DSUM (LS, X) .NE. DS(KT)) CALL DERR (KT, LS, X)
      WRITE (6, 1) KT
C
      KT = 34
      IF (DSUM (LS, Y) .NE. DS(KT)) CALL DERR (KT, LS, Y)
      WRITE (6, 1) KT
C
      KT = 35
      CALL MPCSSX (A, PI, X, Y)
      LS = NW
      IF (DSUM (LS, X) .NE. DS(KT)) CALL DERR (KT, LS, X)
      WRITE (6, 1) KT
C
      KT = 36
      IF (DSUM (LS, Y) .NE. DS(KT)) CALL DERR (KT, LS, Y)
      WRITE (6, 1) KT
C
C   Compute Cos and Sin of 39/64 Pi.
C
      KT = 37
      CALL MPMULD (PI, 0.609375D0, 0, A)
      CALL MPCSSN (A, PI, X, Y)
      LS = NW + 1
      IF (DSUM (LS, X) .NE. DS(KT)) CALL DERR (KT, LS, X)
      WRITE (6, 1) KT
C
      KT = 38
      IF (DSUM (LS, Y) .NE. DS(KT)) CALL DERR (KT, LS, Y)
      WRITE (6, 1) KT
C
      KT = 39
      CALL MPCSSX (A, PI, X, Y)
      LS = NW - 1
      IF (DSUM (LS, X) .NE. DS(KT)) CALL DERR (KT, LS, X)
      WRITE (6, 1) KT
C
      KT = 40
      IF (DSUM (LS, Y) .NE. DS(KT)) CALL DERR (KT, LS, Y)
      WRITE (6, 1) KT
C
C   Compute Cos and Sin of -19/64 Pi.
C
      KT = 41
      CALL MPMULD (PI, -0.296875D0, 0, A)
      CALL MPCSSN (A, PI, X, Y)
      LS = NW + 1
      IF (DSUM (LS, X) .NE. DS(KT)) CALL DERR (KT, LS, X)
      WRITE (6, 1) KT
C
      KT = 42
      IF (DSUM (LS, Y) .NE. DS(KT)) CALL DERR (KT, LS, Y)
      WRITE (6, 1) KT
C
      KT = 43
      CALL MPCSSX (A, PI, X, Y)
      LS = NW - 1
      IF (DSUM (LS, X) .NE. DS(KT)) CALL DERR (KT, LS, X)
      WRITE (6, 1) KT
C
      KT = 44
      IF (DSUM (LS, Y) .NE. DS(KT)) CALL DERR (KT, LS, Y)
      WRITE (6, 1) KT
C
C   Compute inverse Cos and Sin of (1, 1).
C
      KT = 45
      F1(1) = 1.
      F1(2) = 0.
      F1(3) = 1.
      F2(1) = 1.
      F2(2) = 0.
      F2(3) = 1.
      CALL MPANG (F1, F2, PI, B)
      LS = NW + 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
      KT = 46
      CALL MPANGX (F1, F2, PI, B)
      LS = NW - 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
C   Compute inverse Cos and Sin of (1, 5).
C
      KT = 47
      F2(3) = 5.
      CALL MPANG (F1, F2, PI, B)
      LS = NW + 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
      KT = 48
      CALL MPANGX (F1, F2, PI, B)
      LS = NW - 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
C   Compute inverse Cos and Sin of (-1, 3).
C
      KT = 49
      F1(1) = -1.
      F2(3) = 3.
      CALL MPANG (F1, F2, PI, B)
      LS = NW + 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
      KT = 50
      CALL MPANGX (F1, F2, PI, B)
      LS = NW - 1
      IF (DSUM (LS, B) .NE. DS(KT)) CALL DERR (KT, LS, B)
      WRITE (6, 1) KT
C
C   Compute Cosh and Sinh of 0.5.
C
      KT = 51
      F1(1) = 1.
      F1(2) = -1.
      F1(3) = 0.5D0 * BDX
      CALL MPCSSH (F1, AL2, X, Y)
      LS = NW + 1
      IF (DSUM (LS, X) .NE. DS(KT)) CALL DERR (KT, LS, X)
      WRITE (6, 1) KT
C
      KT = 52
      IF (DSUM (LS, Y) .NE. DS(KT)) CALL DERR (KT, LS, Y)
      WRITE (6, 1) KT
C
      KT = 53
      CALL MPCSHX (F1, PI, AL2, X, Y)
      LS = NW - 2
      IF (DSUM (LS, X) .NE. DS(KT)) CALL DERR (KT, LS, X)
      WRITE (6, 1) KT
C
      KT = 54
      IF (ABS (DSUM (LS, Y) - DS(KT)) .GT. 1) CALL DERR (KT, LS, Y)
      WRITE (6, 1) KT
C
C   Compute the root near x = 1.42 + 0.69i of the polynomial
C   x^7 + 2 x^5 + 5 x^4 - 8 x^3 - 15 x^2 + 24 x + 40 = 0.
C
      KT = 55
      X1(1) = 1.42D0
      NX1(1) = 0
      X1(2) = 0.69D0
      NX1(2) = 0
      CALL MPCPOL (N - 1, L, AC, X1, NX1, N4, X)
      LS = NW + 2
      IF (DSUM (LS, X) .NE. DS(KT)) CALL DERR (KT, LS, X)
      WRITE (6, 1) KT
      KT = 56
      IF (DSUM (LS, X(N4+1)) .NE. DS(KT)) CALL DERR (KT, LS, X(N4+1))
      WRITE (6, 1) KT
C
      KT = 57
      CALL MPCPLX (N - 1, L, AC, X1, NX1, N4, X)
      LS = NW
      IF (DSUM (LS, X) .NE. DS(KT)) CALL DERR (KT, LS, X)
      WRITE (6, 1) KT
      KT = 58
      IF (DSUM (LS, X) .NE. DS(KT)) CALL DERR (KT, LS, X)
      WRITE (6, 1) KT
C
C   Compute the real root of the above polynomial near x = -1.34.
C
      KT = 59
      T1 = -1.34D0
      N1 = 0
      CALL MPPOL (N - 1, L, AA, T1, N1, X)
      LS = NW + 2
      IF (DSUM (LS, X) .NE. DS(KT)) CALL DERR (KT, LS, X)
      CALL MPEQ (X, Y)
      WRITE (6, 1) KT
C
      KT = 60
      CALL MPPOLX (N - 1, L, AA, T1, N1, X)
      LS = NW
      IF (DSUM (LS, X) .NE. DS(KT)) CALL DERR (KT, LS, X)
      WRITE (6, 1) KT
C
C   Recover the above polynomial from the computed value of the root.
C
      KT = 61
      CALL MPEQ (Y, XX)
C
      DO 120 K = 2, N
        CALL MPMUL (Y, XX(1,K-1), XX(1,K))
 120  CONTINUE
C
      DO 140 K = 1, N
        DO 130 J = 1, L
          AA(J,K) = 0.
 130    CONTINUE
 140  CONTINUE
C
      CALL MPINRL (N, NX + 4, XX, 5, MT, L, AA, IQ)
      IF (DSUM (LN, AA) .NE. DS(KT)) CALL DERR (KT, LN, AA)
      WRITE (6, 1) KT
C
      KT = 62
C
      DO 160 K = 1, N
        DO 150 J = 1, L
          AA(J,K) = 0.
 150    CONTINUE
 160  CONTINUE
C
      CALL MPINRX (N, NX + 4, XX, 5, MT, L, AA, IQ)
      IF (DSUM (LN, AA) .NE. DS(KT)) CALL DERR (KT, LN, AA)
      WRITE (6, 1) KT
C
C   Sort a pseudo-randomly generated vector.
C
      KT = 63
C
      DO 170 J = 1, NP
        CALL MPRAND (XX(1,J))
 170  CONTINUE
C
      CALL MPSORT (NP, N4, XX, IP)
      IF (ICHK (NP, IP, IS1) .NE. 0) CALL IERR (KT, NP, IP)
      WRITE (6, 1) KT
C
C   Check if parameters in MPCOM1 are correct.
C
      KT = 64
      IF (ICHK (9, NW, IS2) .NE. 0) CALL IERR (KT, 9, NW)
      WRITE (6, 1) KT
C
      STOP
      END
C
      FUNCTION DSUM (N, A)
      DOUBLE PRECISION DSUM, S
      DIMENSION A(N)
C
      S = 0.D0
C
      DO 100 I = 1, N
        S = S + A(I)
 100  CONTINUE
C
      DSUM = S
      RETURN
      END
C
      SUBROUTINE DERR (N, L, A)
      DOUBLE PRECISION DSUM
      DIMENSION A(L)
      CHARACTER*1 CX(1000)
C
      WRITE (6, 1) N
 1    FORMAT ('TESTMP FAILED ON TEST NO.',I4)
      WRITE (6, 2) (A(I), I = 1, L)
 2    FORMAT ('RESULT:'/(6F12.0))
      CALL MPOUT (6, A, INT (7.225 * (L - 2)), CX)
      WRITE (6, 3) DSUM (L, A)
 3    FORMAT ('CHECKSUM:', F20.0)
      RETURN
      END
C
      FUNCTION ICHK (N, IA, IB)
      DIMENSION IA(N), IB(N)
C
      IS = 0
C
      DO 100 I = 1, N
        IS = IS + ABS (IA(I) - IB(I))
 100  CONTINUE
C
      ICHK = IS
      RETURN
      END
C
      SUBROUTINE IERR (N, L, IA)
      DIMENSION IA(L)
C
      WRITE (6, 1) N
 1    FORMAT ('TESTMP FAILED ON TEST NO.',I4)
      WRITE (6, 2) (IA(I), I = 1, L)
 2    FORMAT ('RESULT:'/(8I9))
      RETURN
      END
