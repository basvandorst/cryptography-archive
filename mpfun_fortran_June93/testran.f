C   This is the test program 'testran.f' for TRANSMP.
C
C   David H. Bailey     June 10, 1992
C
CMP+ PRECISION LEVEL 100
CMP+ MIXED MODE FAST
CMP+ OUTPUT PRECISION 56
CMP+ EPSILON 1E-110
C
      PROGRAM TESTRAN
CMP+ IMPLICIT MULTIP REAL (A-H, O-Z)
CMP+ MULTIP INTEGER IA, IB, IC
CMP+ MULTIP REAL A, B
CMP+ MULTIP COMPLEX C, D, E
      PARAMETER (N = 25)
      IMPLICIT DOUBLE PRECISION (A-H, O-Z)
      DOUBLE PRECISION A(N), B(N)
      DOUBLE COMPLEX C, D, E, DPCMPL
C
C   MP parameter definitions.
C
      PARAMETER (DPEPS = 1D-15, DPPIC = 3.141592653589793D0)
C
      EE = EXP (1.D0+0)
      WRITE (6, *) DPPIC, EE
      S = 0.D0
C
C   Loop with subscripted MP variables.
C
      DO 100 I = 1, N
        A(I) = 2 * I + 1
        B(I) = 2.D0 * A(I) * (A(I) + 1.D0)
        S = S + B(I) ** 2
 100  CONTINUE
C
      WRITE (6, *) S
C
C   An expression with mixed MPI and MPR entities.
C
      IA = S
      IB = 262144
      S = (S + 327.25D0) * MOD (IA, 4 * IB)
      WRITE (6, *) S
C
C   A complex square root reference.
C
      E = SQRT (DPCMPL (2.D0 * S, S))
      WRITE (6, *) E
C
C   External and intrinsic MP function references in expressions.
C
      S = DOT (N, A, B)
      T = 2.D0 * SQRT (S) ** 2
      WRITE (6, *) S, T
      S = S / 1048576.D0
      T = S + 2.D0 * LOG (S)
      X = 3 + NINT (T) * 5
      WRITE (6, *) S, T, X
C
C   Deeply nested expressions and function references.
C
      X = (S + (2 * (S - 5) + 3 * (T - 5))) * EXP (COS (LOG (S)))
      WRITE (6, *) X
C
C   A "special" subroutine call (computes both cos and sin of S).
C
      CALL DPCSSN (S, X, Y)
      T = 1.D0 - (X ** 2 + Y ** 2)
C
C   IF-THEN-ELSE construct involving MP variables.
C
      IF (S .GT. 0. .AND. T .LT. DPEPS) THEN
        WRITE (6, *) T
      ELSE
        WRITE (6, *) DPEPS
      ENDIF
C
      STOP
      END
C
C   MP function subprogram.
C
CMP+ MULTIP REAL A, B, DOT, S
      FUNCTION DOT (N, A, B)
      DOUBLE PRECISION A(N), B(N), DOT, S
C
      S = 0.D0
C
      DO 100 I = 1, N
        S = S + A(I) * B(I)
 100  CONTINUE
C
      DOT = S
      RETURN
      END
C
C   DP equivalent of special subroutine DPCSSN.
C
      SUBROUTINE DPCSSN (A, X, Y)
      DOUBLE PRECISION A, X, Y
      X = COS (A)
      Y = SIN (A)
      RETURN
      END
C
C   DP equivalent is special function DPCMPL.
C
      FUNCTION DPCMPL (A, B)
      DOUBLE COMPLEX DPCMPL
      DOUBLE PRECISION A, B
      DPCMPL = DCMPLX (A, B)
      RETURN
      END
