      PROGRAM TRANSMP
C
C   This translates a standard Fortran-77 code input on standard input (unit 5)
C   to a code that calls DHB's MPFUN multiprecision routines, which is output
C   on standard output (unit 6).  This output program may then be compiled and
C   linked with the MPFUN library file.
C
C   Version Date:   June 11, 1993
C
C   Author:
C
C      David H. Bailey                 Telephone:   415-604-4410
C      NASA Ames Research Center       Facsimile:   415-604-3957
C      Mail Stop T045-1                Internet:    dbailey@nas.nasa.gov
C      Moffett Field, CA 94035
C
C   Restrictions:
C
C   This software has now been approved by NASA for unrestricted distribution.
C   However, usage of this software is subject to the following:
C
C   1. This software is offered without warranty of any kind, either expressed
C      or implied.  The author would appreciate, however, any reports of bugs
C      or other difficulties that may be encountered.
C   2. If modifications or enhancements to this software are made to this
C      software by others, NASA Ames reserves the right to obtain this enhanced
C      software at no cost and with no restrictions on its usage.
C   3. The author and NASA Ames are to be acknowledged in any published paper
C      based on computations using this software.  Accounts of practical
C      applications or other benefits resulting from this software are of
C      particular interest.  Please send a copy of such papers to the author.
C
C******************************************************************************
C
C   The following information is a brief description of this program.  For
C   full details and instructions for usage, see the paper "Automatic
C   Translation of Fortran to Multiprecision", available from the author.
C   This program works in conjunction with MPFUN, the author's package of
C   multiprecision functions.
C
C   This translation program allows one to extend the Fortran-77 language
C   with the data types MULTIP INTEGER, MULTIP REAL and MULTIP COMPLEX.
C   These data types can be used for integer, floating point or complex
C   numbers of an arbitrarily high but pre-specified level of precision.
C   Variables in the input program may be declared to have one of these
C   multiprecision types in the output program by placing directives
C   (special comments) in the input file.  In this way, the input file
C   remains an ANSI Fortran-77 compatible program and can be run at any
C   time using ordinary arithmetic on any Fortran system for comparison
C   with the multiprecision equivalent.
C
C   This translator program should run on any Fortran-77 system that supports
C   recursive subroutine references.  On some systems, including Sun and IBM
C   workstations, two non-standard IMPLICIT AUTOMATIC statements must be
C   uncommented in this file to permit recursion.  These and other instances
C   of machine-dependent code are marked below with C>.
C
C   Instructions for compiling and testing this program are included in the
C   readme file that accompanies this code.
C
C******************************************************************************
C
C   This is the start of the main program of the translator.  In each
C   subprogram below, C+ delimits common data specifications.  Specifications
C   following the second C+ are for local variables.  C* comments delimit
C   debug printout code.  C> comments indicate machine-dependent code.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*80 LIN, ARG(MAR)
      CHARACTER*16 LINQ, NAMQ, NUMX, UCASE
      DIMENSION ITAR(MAR), LAR(MAR)
C
C   Start of input program file -- initialize and read first line.
C
      IMM = 1
      ITE = 1
      MPP = 0
      MSS = 0
      MXP = 0
      IEOF = 0
C*
C   Uncomment one of the next two lines -- the first for normal use, the
C   second for debug.
C
      OPEN (11, STATUS = 'SCRATCH')
C      open (11, file = 'scratch')
C*
      OPEN (12, STATUS = 'SCRATCH')
      REWIND 11
      REWIND 12
      READ (5, '(A)', END = 150) LIN
      LC = 1
      L1 = 1
      L2 = LNBLK (LIN)
      LS = L2
      NVAR = MINT
      NAMQ = 'TANH'
      IQTANH = ITAB (0, 0, NAMQ)
C
C   Start processing a new subprogram.
C
 100  ISTP = 0
      NVAR = MINT
      IEX = 0
      IFL = 0
      KDO = 0
      LSM = 0
      MPA = 0
      MPF = 0
      MPT = 0
      IEND = 0
C
C   Initialize the implicit type table and the special constant usage table.
C
      DO 110 I = 1, 26
        IMPL(I) = IMPS(I)
 110  CONTINUE
C
      DO 120 I = 1, 5
        KCON(I) = 0
 120  CONTINUE
C
C   Start reading a new statement.
C
 130  LN = 0
      LCT = LC
      LINE = ' '
C
 140  CONTINUE
      LL = L2 - L1 + 1
      LINE(LN+1:LN+LL) = LIN(L1:L2)
      LN = LN + LL
      READ (5, '(A)', END = 150) LIN
      LC = LC + 1
      L1 = 1
      L2 = LNBLK (LIN)
      IF (LIN(1:1) .EQ. 'C' .OR. LIN(1:1) .EQ. 'c' .OR. L2 .EQ. 0)      
     $  GOTO 160
      IF (L2 .GT. 72) THEN
        WRITE (11, 1) LC
 1      FORMAT ('CMP*'/'CMP* Characters past column 72 in line',I6,     
     $    ' are ignored.'/'CMP*')
        L2 = 72
      ENDIF
      IF (LIN(6:6) .NE. ' ') THEN
        K1 = 72 - LS
        IF (K1 .GE. 1) LINE(LN+1:LN+K1) = ' '
        LN = LN + K1
        L1 = 7
        LS = L2
        GOTO 140
      ENDIF
      LS = L2
      GOTO 160
C
C   The end of file has been encountered.
C
 150  IEOF = 1
C
C   A complete multiline statement has been read.  Check if it is a comment.
C
 160  CONTINUE
C
C   Optionally print out current statement.
C*
C      write (6, *) '%'//line(1:ln)//'%'
C*
      LQ = MIN (16, LN)
      LINQ = UCASE (LINE(1:LQ))
      IF (LN .EQ. 0 .OR. LINQ(1:1) .EQ. 'C') THEN
C
C   Check if this comment is a MP directive.
C
        CALL OUTLIN (0, LN, LINE)
        IF (LN .GE. 4 .AND. LINQ(1:4) .EQ. 'CMP+') CALL MPDEC (LN)
        GOTO 240
      ENDIF
      K1 = NBLK (7, LN, LINE)
      LQ = MIN (K1 + 15, LN)
      LINQ = UCASE (LINE(K1:LQ))
C
C   Check if this is an end statement.
C
      IF (LINQ(1:5) .EQ. 'END  ') THEN
C
C   Parse the argument list of this subprogram, compare with the subprogram
C   table, and add it if it is not there.
C
        LSV = LCT
        LCT = LCS
        CALL ARLIST (11, LSAR, SARG, NAR, ITAR, LAR, ARG)
        CALL CHKARG (11, FNAM, NAR, ITAR, LAR, ARG)
        LCT = LSV
        IEND = 1
        CALL OUTLIN (1, LN, LINE)
        GOTO 240
      ENDIF
C
C   If the executable portion of this subprogram has already been encountered,
C   and if this subprogram contains no MP variables, there is no need to check
C   the line any further.
C
      IF (IEX .NE. 0 .AND. MPT .EQ. 0 .AND. MPA .EQ. 0) THEN
        CALL OUTLIN (1, LN, LINE)
        GOTO 240
      ENDIF
C
C   Check if this line is a program, subroutine, function or block data
C   statement.
C
      IF (IPFSB (K1, LN) .NE. 0) THEN
        LCS = LCT
        GOTO 240
      ENDIF
C
C   At the beginning of a subprogram, one of the above should have been noted.
C
      IF (ISTP .EQ. 0) THEN
        CALL ERRMES (1, 0)
        WRITE (6, 2)
 2      FORMAT ('PROGRAM, SUBROUTINE, FUNCTION, or BLOCK DATA',         
     $    ' statement is missing.')
        CALL ABRT
      ENDIF
C
C   Check if it is an implicit statement.
C
      IF (LINQ(1:8) .EQ. 'IMPLICIT') THEN
        IF (IEX .NE. 0) GOTO 250
        K1 = NBLK (K1 + 8, LN, LINE)
        CALL IMPLIC (K1, LN)
        CALL OUTLIN (1, LN, LINE)
        GOTO 240
      ENDIF
C
C   Check if it is a type statement.
C
 170  IT = NTYPE (K1, LN)
      IF (IT .NE. 0) THEN
        IF (IEX .NE. 0) GOTO 250
        CALL TYPE (IT, K1, LN)
        GOTO 240
      ENDIF
C
C   Check if it is a parameter statement.
C
      IF (LINQ(1:9) .EQ. 'PARAMETER') THEN
        IF (IEX .NE. 0) GOTO 250
        I1 = ISCAN (K1 + 9, LN, LINE)
        IF (I1 .GE. 1) THEN
          K1 = K1 + 9
          CALL OUTLIN (2, LN, LINE)
          CALL PARAM (K1, LN)
        ELSE
          CALL OUTLIN (1, LN, LINE)
        ENDIF
        GOTO 240
      ENDIF
C
C   Check if it is a dimension statement.
C
      IF (LINQ(1:9) .EQ. 'DIMENSION') THEN
        IF (IEX .NE. 0) GOTO 250
        CALL DIMEN (K1 + 9, LN)
        GOTO 240
      ENDIF
C
C   Check if it is a common statement.
C
      IF (LINQ(1:6) .EQ. 'COMMON') THEN
        IF (IEX .NE. 0) GOTO 250
        K1 = K1 + 6
        I1 = INDX (K1, LN, '/', LINE)
        IF (I1 .NE. 0) THEN
          I2 = INDX (I1 + 1, LN, '/', LINE)
          IF (I2 .EQ. 0) CALL ERRMES (2, 1)
          K1 = I2 + 1
        ENDIF
        CALL DIMEN (K1, LN)
        GOTO 240
      ENDIF
C
C   Check if it is an equivalence statement.
C
      IF (LINQ(1:11) .EQ. 'EQUIVALENCE') THEN
        IF (IEX .NE. 0) GOTO 250
        CALL OUTLIN (2, LN, LINE)
C
C   Append '1,' to subscripted MP variables.
C
        K2 = LN
        CALL FIXSUB (K1 + 11, K2, LN)
        CALL OUTLIN (1, LN, LINE)
        WRITE (11, 3)
 3      FORMAT ('CMP<')
        GOTO 240
      ENDIF
C
C   Check if it is an external, intrinsic or save statement.
C
      IF (LINQ(1:8) .EQ. 'EXTERNAL' .OR. LINQ(1:9) .EQ. 'INTRINSIC')    
     $  THEN
        IF (IEX .NE. 0) GOTO 250
        K1 = K1 + 9
        CALL TYPE (0, K1, LN)
        GOTO 240
      ENDIF
      IF (LINQ(1:5) .EQ. 'SAVE ') THEN
        IF (IEX .NE. 0) GOTO 250
        CALL OUTLIN (1, LN, LINE)
        GOTO 240
      ENDIF
C
C   Check if this is the start of the executable part of the subprogram.
C
      IF (IEX .EQ. 0) THEN
        IEX = 1
C
C   Check if the subprogram name is valid and set its type.  The type of a
C   program or subroutine name is set to 0 (undefined), whereas the type of
C   a function name is set according to previous implicit or type statements.
C
        IX = ITAB (1, 0, FNAM)
        IF (IX .LE. IQTANH) THEN
          CALL ERRMES (3, 0)
          WRITE (6, 4)
 4        FORMAT ('This name may not be used as a subroutine or',       
     $      ' function name.')
          CALL ABRT
        ELSEIF (IX .GT. MINT .AND. ISTP .EQ. 3 .AND. KTYP(IX) .GE. 8    
     $      .AND. KDIM(IX) .NE. -2) THEN
          CALL ERRMES (4, 0)
          WRITE (6, 5)
 5        FORMAT ('MP function names must be declared with an explicit' 
     $      ' MP type directive'/'immediately preceding the function',  
     $      ' statement.')
          CALL ABRT
        ENDIF
        KDEC(IX) = 1
        IF (ISTP .NE. 3) KTYP(IX) = 0
C
C   Parse the argument list of this subprogram, compare with the subprogram
C   table, and add it if it is not there.
C
        CALL ARLIST (11, LSAR, SARG, NAR, ITAR, LAR, ARG)
        CALL CHKARG (11, FNAM, NAR, ITAR, LAR, ARG)
C
C   If any MP variables have been defined, or if this is the main program,
C   Insert a marker in the temporary file to mark the location of the MP
C   declarations for INIMP.
C
        IF (MPT .NE. 0 .OR. MPA .NE. 0 .OR. ISTP .EQ. 1) THEN
          WRITE (11, 6)
 6        FORMAT ('CMP>>>')
        ENDIF
C
C   If no MP variables have been identified, there is no need to further
C   analyze the statement.
C
        IF (MPT .EQ. 0 .AND. MPA .EQ. 0) THEN
          CALL OUTLIN (1, LN, LINE)
          GOTO 240
        ENDIF
      ENDIF
C
C   Check if this is a data statement.
C
      IF (LINQ(1:5) .EQ. 'DATA ') THEN
        I1 = ISCAN (K1 + 5, LN, LINE)
        IF (I1 .NE. 0) GOTO 260
        CALL OUTLIN (1, LN, LINE)
        GOTO 240
      ENDIF
C
C   Check if this is an endif statement.  If so, previous if blocks
C   generated as translations of elseif statements must be closed.
C
      IF (LINQ(1:5) .EQ. 'ENDIF' .OR. LINQ(1:6) .EQ. 'END IF') THEN
        IF (IFL .GT. 0) THEN
C
          DO 180 I = 1, IFL
            WRITE (11, 7)
 7          FORMAT (6X,'ENDIF')
 180      CONTINUE
C
          IFL = 0
        ENDIF
        CALL OUTLIN (1, LN, LINE)
        GOTO 240
      ENDIF
C
C   Nothing needs to be processed for these non-executable statements.
C
      IF (LINQ(1:6) .EQ. 'ELSE  ' .OR. LINQ(1:6) .EQ. 'ENDDO ' .OR.     
     $  LINQ(1:7) .EQ. 'END DO ' .OR. LINQ(1:8) .EQ. 'CONTINUE' .OR.    
     $  LINQ(1:6) .EQ. 'FORMAT') THEN
        CALL OUTLIN (1, LN, LINE)
        GOTO 240
      ENDIF
C
C   Nothing needs to be processed for these executable statements.
C
      IF (LINQ(1:5) .EQ. 'CLOSE' .OR. LINQ(1:5) .EQ. 'GOTO ' .OR.       
     $  LINQ(1:4) .EQ. 'OPEN' .OR. LINQ(1:5) .EQ. 'STOP ' .OR.          
     $  LINQ(1:9) .EQ. 'BACKSPACE' .OR. LINQ(1:7) .EQ. 'ENDFILE' .OR.   
     $  LINQ(1:7) .EQ. 'INQUIRE' .OR. LINQ(1:6) .EQ. 'RETURN' .OR.      
     $  LINQ(1:6) .EQ. 'REWIND') THEN
      CALL OUTLIN (1, LN, LINE)
      GOTO 240
      ENDIF
C
C   Check if the statement is a DO statement.  If so, place the DO terminal
C   number in the IDON table.
C
      IF (LINQ(1:3) .EQ. 'DO ') THEN
        K1 = NBLK (K1 + 3, LN, LINE)
        I1 = ISCAN (K1, LN, LINE)
        IF (I1 .NE. 0) GOTO 260
        IF (INDEX (DIG, LINE(K1:K1)) .NE. 0) THEN
          K2 = INDEX (LINE(K1:LN), ' ')
          IF (K2 .EQ. 0) CALL ERRMES (5, 1)
          K2 = K1 - 1 + K2
          NUMX = LINE(K1:K2)
          READ (NUMX, '(BN,I16)', ERR = 270) K
          KDO = KDO + 1
          IF (KDO .GT. NDO) THEN
            CALL ERRMES (6, 0)
            WRITE (6, 8)
 8          FORMAT ('Too many DO statements in this subprogram.')
            CALL ABRT
          ENDIF
          IDON(KDO) = K
        ENDIF
        CALL OUTLIN (1, LN, LINE)
        GOTO 240
      ENDIF
C
C   Check if statement has any MP variables.  If not, there is no need for
C   any further analysis.
C
      I1 = ISCAN (K1, LN, LINE)
      IF (I1 .EQ. 0) THEN
        CALL OUTLIN (1, LN, LINE)
        GOTO 240
      ENDIF
C
C   Output original statement as a comment.
C
      CALL OUTLIN (2, LN, LINE)
C
C   Check if a line number is presesnt.  If so, output as a continue statement.
C
      READ (LINE(1:5), '(BN,I5)', ERR = 270) K
      IF (K .LT. 0) GOTO 270
      IF (K .NE. 0) THEN
C
        DO 190 I = 1, KDO
          IF (K .EQ. IDON(I)) THEN
            CALL ERRMES (7, 0)
            WRITE (6, 9)
 9          FORMAT ('MP variables may not appear in the terminal line', 
     $        ' of a DO loop.')
            CALL ABRT
          ENDIF
 190    CONTINUE
C
        WRITE (11, 10) K
 10     FORMAT (I5,' CONTINUE')
      ENDIF
C
C   Check if this is a logical if or elseif statement with MP variables.
C
      IF (LINQ(1:2) .EQ. 'IF') THEN
        K3 = NBLK (K1 + 2, LN, LINE)
        IF (LINE(K3:K3) .NE. '(') GOTO 200
        CALL IFST (IFL, 1, K3, LN)
        GOTO 210
      ELSEIF (LINQ(1:6) .EQ. 'ELSEIF') THEN
        K3 = NBLK (K1 + 6, LN, LINE)
        CALL IFST (IFL, 2, K3, LN)
        GOTO 210
      ELSEIF (LINQ(1:7) .EQ. 'ELSE IF') THEN
        K3 = NBLK (K1 + 7, LN, LINE)
        CALL IFST (IFL, 2, K3, LN)
        GOTO 210
      ENDIF
C
C   Fix subscripts of MP variables and change names of special constants.
C
 200  K2 = LN
      CALL FIXSUB (K1, K2, LN)
C
C   Process other kinds of MP executable statements.
C
      CALL EXEC (K1, LN)
C
C   Insert a comment to mark the end of the translation of the MP executable
C   statement.
C
 210  WRITE (11, 3)
C
C   Check if the itmp table is properly zeroed.  If not, one of the routines
C   dealing with MP statements erred.
C
      DO 230 J = 1, NTYP
        DO 220 I = 1, 9
          IF (ITMP(I,J) .NE. 0) THEN
            CALL ERRMES (8, 0)
            WRITE (6, 11) CTM(J), I
 11         FORMAT ('Translator error: active temporary: MP',A1,I1/     
     $        'Please contact the author.')
            CALL ABRT
          ENDIF
 220    CONTINUE
 230  CONTINUE
C
C   If this is an end statement, copy the scratch file to the output file,
C   inserting MP declarations at the marker.
C
 240  IF (IEND .EQ. 1) THEN
        CALL COPY
        IF (IEOF .EQ. 1) STOP
        GOTO 100
      ELSE
        IF (IEOF .EQ. 1) THEN
          IF (ISTP .EQ. 0) STOP
          CALL ERRMES (9, 0)
          WRITE (6, 12)
 12       FORMAT ('The last line of the file was not an END statement.')
          CALL ABRT
        ENDIF
        GOTO 130
      ENDIF
C
 250  CALL ERRMES (10, 0)
      WRITE (6, 13)
 13   FORMAT ('A declarative statement may not appear after an',        
     $  ' executable statement.')
      CALL ABRT
C
 260  CALL ERRMES (11, 0)
      WRITE (6, 14)
 14   FORMAT ('MP variables may not appear in this statement.')
      CALL ABRT
C
 270  CALL ERRMES (12, 0)
      WRITE (6, 15)
 15   FORMAT ('Syntax error in line number.')
      CALL ABRT
C
      STOP
      END
C
      BLOCK DATA
C
C   This sets all data in common.  Here is a brief description of these
C   variables and arrays, in alphabetical order.
C
C   ALPL   Lower case alphabet.
C   ALPU   Upper case alphabet.
C   CTM    Table of one-character type abbreviations.
C   CTP    Table of two-character type abbreviations.
C   DEL    Standard Fortran delimiters.
C   DIG    The ten digits.
C   EPS    The mantissa and exponent of the current epsilon, in character form.
C   FNAM   The function name, in function subprograms.
C   IDON   DO line number table.
C   IEX    1 if the executable portion of the subprogram has been encountered.
C   IMM    Mixed mode option (0:FAST, 1:SAFE).
C   IMPL   Implicit type definition table.
C   IMPS   Default implicit type definitions.
C   ISTP   Type of subprogram (1:PROGRAM, 2:SUBROUTINE, 3:FUNCTION, 4: BLOCK D)
C   ITE    Type error option (0:OFF, 1:ON).
C   ITMP   MP temporary usage table.
C   KCON   Special constant usage table.
C   KDEC   MP variable declaration table.
C   KDIM   Dimension and misc. information for names in VAR:
C          -3   Special constant or parameter.
C          -2   MP function name (within its defining subprogram).
C          -1   Function name.
C           0   Scalar variable.
C           1   Dimensioned variable.
C           2-10   Indicates number of dimensions (not yet implemented).
C   KDO    Number of entries in DO number table.
C   KEYW   Table of Fortran keywords.
C   KOP    Operator precedence table.
C   KSTP   Table of types of subroutine argument lists.
C   KTYP   Types of variables in VAR:
C          -1   Dependent on argument (for intrinsic function names)
C           0   Undefined
C           1   Integer
C           2   Real
C           3   Double Precision
C           4   Complex
C           5   Double Complex
C           6   Character
C           7   Logical
C           8   MP Integer
C           9   MP Real
C          10   MP Complex
C   LCT    Current line count.
C   LEP    Lengths of epsilon strings.
C   LINE   Current extended working line read from file.
C   LOP    Lengths of operators in LOPR and UOPR.
C   LOPR   Lower case operators.
C   LSAR   Length of subroutine argument list.
C   LSM    Longest numeric string in current subprogram.
C   LVAR   Lengths of the names in VAR.
C   MPA    Number of MP parameters in current subprogram.
C   MPLC   Table of implicit/explicit status of names in VAR.
C   MPP    Current output precision level.
C   MPT    Set to 1 if an MP variable has been encountered in subprogram.
C   MSS    Scratch space.
C   MXP    Maximum precision level in words.
C   NARS   Table of number of arguments in argument list array KSTP.
C   NSUB   Number of subprograms encountered.
C   NVAR   Number of variables defined in current subprogram (including
C          standard and MP intrinsic names).
C   SARG   Argument list of current subprogram.
C   SNAM   Table of subroutine names.
C   UOPR   Upper case operators.
C   VAR    Table of variable names for current subprogram.
C
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      PARAMETER (NP1 = MVAR - MINT, NPC = 5, NPI = 36, NPF = 6, NPS = 2)
      DATA IDON /NDO * 0/
      DATA IMPL /26 * 0/
      DATA IMPS /8 * 2, 6 * 1, 12 * 2/
      DATA KDEC /MINT * 1, NP1 * 0/
      DATA KDIM /10 * -3, 44 * -1, NP1 * 0/
      DATA KTYP /1, 4 * 3, 1, 4 * 9, 3 * -1, 4, 4 * -1, 6, 4, 3 * -1, 3,
     $  5, -1, 5, 5, -1, 4 * 1, 6 * -1, 2, 6 * -1, 8, 10, 4 * 9, 2 * 9, 
     $  NP1 * 0/
      DATA LVAR /10 * 5, 3, 4, 4, 5, 5, 4, 4, 5, 4, 5, 5, 3, 4, 4, 6, 3,
     $  5, 5, 3, 5, 5, 3, 3, 3, 5, 3, 3, 3, 4, 4, 4, 3, 4, 4, 3, 4, 5,  
     $  6, 6, 6, 5, 6, 6, 6, NP1 * 0/
      DATA KOP /1, 6, 6, 8, 7, 7, 5, 5, 5, 5, 5, 5, 2, 3, 4/
      DATA LOP /1, 1, 1, 2, 1, 1, 4, 4, 4, 4, 4, 4, 4, 5, 5/
      DATA LEP /2 * 0/
      DATA MPLC /NPC * 0, NPC * 1, NPI * 1, NPF * 0, NPS * 1, NP1 * 1/
      DATA ALPL /'abcdefghijklmnopqrstuvwxyz'/
      DATA ALPU /'ABCDEFGHIJKLMNOPQRSTUVWXYZ'/
      DATA DEL /' ,().=+-*/:'/
      DATA DIG /'0123456789'/
      DATA CTM /'I', 'R', 'D', 'C', 'X', 'A', 'L', 'J', 'M', 'Z'/
      DATA CTP /'IN', 'SP', 'DP', 'CO', 'DC', 'CH', 'LO', 'MPI', 'MPR', 
     $  'MPC'/
      DATA EPS /2 * ' '/
      DATA KEYW/                                                        
     $  'BACKSPACE', 'BLOCK', 'CALL', 'CHARACTER', 'CLOSE', 'COMMON',   
     $  'COMPLEX', 'CONTINUE', 'DATA', 'DIMENSION', 'DO', 'DOUBLE',     
     $  'ELSE', 'ELSEIF', 'END', 'ENDFILE', 'ENDIF', 'ENTRY',           
     $  'EQUIVALENCE', 'EXTERNAL', 'FORMAT', 'FUNCTION', 'GO', 'GOTO',  
     $  'IF', 'IMPLICIT', 'INQUIRE', 'INTEGER', 'INTRINSIC', 'LOGICAL', 
     $  'OPEN', 'PARAMETER', 'PRECISION', 'PRINT', 'PROGRAM', 'READ',   
     $  'REAL', 'RETURN', 'REWIND', 'SAVE', 'STOP', 'SUBROUTINE',       
     $  'THEN', 'TO', 'WRITE'/
      DATA LOPR /'=', '+', '-', '**', '*', '/', '.eq.', '.ne.', '.gt.', 
     $  '.lt.', '.ge.', '.le.', '.or.', '.and.', '.not.'/
      DATA SFUN/                                                        
     $  'ALOG', 'ALOG10', 'AMAX0', 'AMAX1', 'AMIN0', 'AMIN1', 'AMOD',   
     $  'CABS', 'CCOS', 'CEXP', 'CLOG', 'CSIN', 'CSQRT', 'DABS',        
     $  'DACOS', 'DASIN', 'DATAN', 'DATAN2', 'DCOS', 'DCOSH', 'DCOSH',  
     $  'DDIM', 'DEXP', 'DIM', 'DINT', 'DLOG', 'DLOG10', 'DMAX1',       
     $  'DMIN1', 'DMOD', 'DNINT', 'DPROD', 'DSIGN', 'DSIN', 'DSINH',    
     $  'DSQRT', 'DTAN', 'DTANH', 'FLOAT', 'IABS', 'IDIM', 'IDINT',     
     $  'IDNINT','IFIX', 'ISIGN', 'MAX0', 'MAX1', 'MIN0', 'MIN1',       
     $  'SNGL'/
      DATA VAR /                                                        
     $  'MPNWP', 'DPEPS', 'DPL02', 'DPL10', 'DPPIC', 'MPNWQ', 'MPEPS',  
     $  'MPL02', 'MPL10', 'MPPIC',                                      
     $  'ABS', 'ACOS', 'AINT', 'AIMAG', 'ANINT', 'ASIN', 'ATAN',        
     $  'ATAN2', 'CHAR', 'CMPLX', 'CONJG', 'COS', 'COSH', 'DBLE',       
     $  'DCMPLX', 'DIM', 'DIMAG', 'DREAL', 'EXP', 'ICHAR', 'INDEX',     
     $  'INT', 'LEN', 'LOG', 'LOG10', 'MAX', 'MIN', 'MOD', 'NINT',      
     $  'REAL', 'SIGN', 'SIN', 'SINH', 'SQRT', 'TAN', 'TANH',           
     $  'MPINT', 'DPCMPL', 'DPIMAG', 'DPREAL', 'DPNRT', 'DPRAND',       
     $  'DPCSSN', 'DPCSSH', NP1 * ' '/
      DATA UOPR /'=', '+', '-', '**', '*', '/', '.EQ.', '.NE.', '.GT.', 
     $  '.LT.', '.GE.', '.LE.', '.OR.', '.AND.', '.NOT.'/
      END
C
      SUBROUTINE ABRT
C>
C   This terminates execution.  For debug purposes it may be preferable to
C   replace the standard STOP with a call to a system routine that produces
C   a traceback.
C
C   TRACBK is a traceback routine for SGI workstations.  The C code for this
C   routine is available from the author.
C
C      CALL TRACBK
C
      STOP
      END
C
      SUBROUTINE ARLIST (LU, LA, LINA, NAR, ITAR, LAR, ARG)
C
C   This processes an argument list in LINA, which has length LA.  Any
C   expressions in any argument are first processed with subroutine EXPRES.
C   The resulting argument list (NAR elements) is placed in ARG, with types
C   in ITAR and lengths in ARG.  LU is the logical unit number of output code.
C
C>  Uncomment this line on Sun and IBM worksations.
C
      IMPLICIT AUTOMATIC (A-Z)
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINA, LINB
      CHARACTER*80 ARG(MAR), ARGX
      DIMENSION ITAR(MAR), LAR(MAR)
C*
C      write (lu, *) 'enter arlist'
C      write (lu, *) '%'//lina(1:la)//'%'
C*
      K1 = 1
      NAR = 0
      IEND = 0
      IF (LA .EQ. 0 .OR. LINA(1:LA) .EQ. ' ') GOTO 120
C
 100  IF (K1 .GT. LA) CALL ERRMES (13, 1)
      K1 = NBLK (K1, LA, LINA)
      IF (K1 .EQ. 0) CALL ERRMES (14, 1)
      K2 = MATCH (1, K1, LA, LINA)
      IF (K2 .EQ. 0) THEN
        K2 = LA + 1
        IEND = 1
      ENDIF
      K2 = K2 - 1
      LB = K2 - K1 + 1
      LINB(1:LB) = LINA(K1:K2)
C
C   Evaluate the expression with EXPRES after setting the mixed mode option
C   to FAST, except if this is a parameter statement or an intrinsic function
C   reference.
C
      IMS = IMM
      IF (LU .NE. 12 .AND. IMM .NE. 2) IMM = 0
      CALL EXPRES (LU, LB, LINB, ITPX, LX, ARGX)
      IMM = IMS
C
C   Add this argument to the list.
C
 110  NAR = NAR + 1
      IF (NAR .GT. MAR) THEN
        CALL ERRMES (15, 0)
        WRITE (6, 1)
 1      FORMAT ('List has too many arguments.')
        CALL ABRT
      ENDIF
      ITAR(NAR) = ITPX
      LAR(NAR) = LX
      ARG(NAR)(1:LX) = ARGX(1:LX)
      K1 = K2 + 2
      IF (IEND .EQ. 0) GOTO 100
C
 120  CONTINUE
C*
C      write (lu, *) 'exit arlist  args:'
C      do 111 i = 1, nar
C        write (lu, *) '%'//arg(i)(1:lar(i))//'%'
C 111    continue
C*
      RETURN
      END
C
      SUBROUTINE ASST (K1, LN)
C
C   This processes MP assignment statements.  K1 and LN are the indices of the
C   first and last non-blank characters in the statement.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINA
      CHARACTER*80 ARGX
C
C   Evaluate the entire statement as an expression.
C
      LA = LN - K1 + 1
      LINA(1:LA) = LINE(K1:LN)
      CALL EXPRES (11, LA, LINA, ITPX, LX, ARGX)
C
C   Check if the final result of evaluation of the expression is non-empty.
C   If so, then the last operation was not an equal operation, and thus the
C   statement is not a valid assignment statement.
C
      IF (LX .NE. 0) THEN
        CALL ERRMES (16, 0)
        WRITE (6, 1)
 1      FORMAT ('This is not a valid MP assignment statement.')
        CALL ABRT
      ENDIF
C
      RETURN
      END
C
      SUBROUTINE CALLST (K1, LN)
C
C   This processes MP call statements.  K1 and LN are the indices of the
C   first (after 'call') and last non-blank characters in the statement.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINA, LINB
      CHARACTER*80 ARG(MAR)
      CHARACTER*16 NAMQ, NAMX
      DIMENSION ITAR(MAR), LAR(MAR)
      CHARACTER*4 TMP1
C
C   Identify subroutine name.
C
      I1 = INDX (K1, LN, '(', LINE)
      L1 = MIN (I1 - K1, 16)
      NAMX = LINE(K1:K1+L1-1)
      IX = ITAB (0, 0, NAMX)
      IF (IX .EQ. 0) THEN
        CALL ERRMES (17, 0)
        WRITE (6, 1) NAMX
 1      FORMAT ('This Fortran keyword may not CALLed: ',A)
        CALL ABRT
      ENDIF
      IF (IX .GT. MINT) KTYP(IX) = 0
      LINA(1:11) = '      CALL '
      ISS = 0
C
C   Determine if this is one of the special DP subroutine names.
C
      NAMQ = 'DPCSSN'
      IQ1 = ITAB (0, 0, NAMQ)
      NAMQ = 'DPCSSH'
      IQ2 = ITAB (0, 0, NAMQ)
      IF (IX .EQ. IQ1) THEN
        KCON(5) = 1
        LINA(12:17) = 'MPCSSN'
        LA = 17
        ISS = 1
      ELSEIF (IX .EQ. IQ2) THEN
        KCON(3) = 1
        LINA(12:17) = 'MPCSSH'
        LA = 17
        ISS = 2
      ELSE
        LINA(12:L1+11) = NAMX(1:L1)
        LA = L1 + 11
      ENDIF
C
      LINA(LA+1:LA+1) = '('
      LA = LA + 1
C
C   Identify and process list of arguments.
C
      I2 = MATCH (0, I1 + 1, LN, LINE)
      IF (I2 .NE. LN) CALL ERRMES (18, 1)
      LB = LN - I1 - 1
      IF (LB .EQ. 0) THEN
        LA = LA + 2
      ELSE
        LINB(1:LB) = LINE(I1+1:LN-1)
      ENDIF
      CALL ARLIST (11, LB, LINB, NAR, ITAR, LAR, ARG)
C
C   Check the argument list with the subprogram table.
C
      IF (ISS .EQ. 0) CALL CHKARG (11, NAMX, NAR, ITAR, LAR, ARG)
C
C   Change the argument list if this is a special DP subroutine.
C
      IF (ISS .EQ. 1) THEN
        IF (NAR .NE. 3) GOTO 120
        NAR = 4
        ITAR(4) = ITAR(3)
        LAR(4) = LAR(3)
        ARG(4) = ARG(3)
        ITAR(3) = ITAR(2)
        LAR(3) = LAR(2)
        ARG(3) = ARG(2)
        ITAR(2) = 9
        LAR(2) = 5
        ARG(2) = 'MPPIC'
      ELSEIF (ISS .EQ. 2) THEN
        IF (NAR .NE. 3) GOTO 120
        NAR = 4
        ITAR(4) = ITAR(3)
        LAR(4) = LAR(3)
        ARG(4) = ARG(3)
        ITAR(3) = ITAR(2)
        LAR(3) = LAR(2)
        ARG(3) = ARG(2)
        ITAR(2) = 9
        LAR(2) = 5
        ARG(2) = 'MPL02'
      ENDIF
C
C   Append the argument list.
C
      DO 100 J = 1, NAR
        L1 = LAR(J)
        LINA(LA+1:LA+L1) = ARG(J)(1:L1)
        LINA(LA+L1+1:LA+LA+2) = ', '
        LA = LA + L1 + 2
 100  CONTINUE
C
      LINA(LA-1:LA-1) = ')'
      LA = LA - 1
      CALL OUTLIN (1, LA, LINA)
C
C   Release any temporaries among the arguments.
C
      DO 110 I = 1, NAR
        LI = LAR(I)
        IF (LI .EQ. 4) THEN
          IF (ARG(I)(1:2) .EQ. 'MP') THEN
            TMP1 = ARG(I)(1:4)
            CALL RLTMP (TMP1)
          ENDIF
        ENDIF
 110  CONTINUE
C
      GOTO 130
C
 120  CALL ERRMES (19, 0)
      WRITE (6, 2) NAMX
 2    FORMAT ('Improper number of arguments for this special',          
     $  ' subroutine: ',A)
      CALL ABRT
C
 130  RETURN
      END
C
      SUBROUTINE CHKARG (LU, NAM, NAR, ITAR, LAR, ARG)
C
C   This routine checks to see if a subroutine or function name is in the
C   subprogram table.  If it is, the calling sequence is compared with that
C   in the table.  If not, it is added to the table.  LU is the unit number
C   for output code.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*80 ARG(MAR)
      CHARACTER*16 NAM, NAMX
      DIMENSION ITAR(MAR), LAR(MAR)
C
      IX = ITAB (0, 0, NAM)
      IF (IX .EQ. 0) CALL ERRMES (20, 1)
      NAMX = VAR(IX)
      LX = LVAR(IX)
C
C   Check if the function name is in the subprogram table.
C
      DO 100 I = 1, NSUB
        IF (SNAM(I) .EQ. NAMX) GOTO 120
 100  CONTINUE
C
C   Insert this function and its calling sequence in the subprogram table.
C
      NSUB = NSUB + 1
      IF (NSUB .GT. MSUB) THEN
        CALL ERRMES (21, 0)
        WRITE (6, 1)
 1      FORMAT ('Too many program units in this file.')
        CALL ABRT
      ENDIF
      SNAM(NSUB) = NAMX
      NARS(NSUB) = NAR
      KSTP(0,NSUB) = KTYP(IX)
C
      DO 110 I = 1, NAR
        KSTP(I,NSUB) = ITAR(I)
 110  CONTINUE
C
      GOTO 150
C
C   The function name is in the subprogram table.  Check if the types of the
C   result and the arguments are the same as in the table.
C
 120  KS = I
      I = -1
      IF (NAR .NE. NARS(KS)) GOTO 140
      I = 0
      IF (KTYP(IX) .NE. KSTP(0,KS) .AND. KTYP(IX) * KSTP(0,KS) .NE. 0)  
     $  GOTO 140
C
      DO 130 I = 1, NAR
        IF (ITAR(I) .NE. KSTP(I,KS) .AND. ITAR(I) * KSTP(I,KS) .NE. 0)  
     $    GOTO 140
 130  CONTINUE
C
      GOTO 150
C
C   A warning message or a fatal error is generated, depending on the type
C   error flag ITE.
C
 140  IF (ITE .EQ. 0) THEN
        WRITE (LU, 2) NAM(1:LX), I
 2      FORMAT ('CMP*'/'CMP*  The result type or argument list of this',
     $    ' function or subroutine is'/'CMP*  incompatible with a',     
     $    ' previous definition or reference: ',A,I4/'CMP*')
      ELSE
        CALL ERRMES (22, 0)
        WRITE (6, 3) NAM(1:LX), I
 3      FORMAT ('The result type or argument list of this function or', 
     $    ' subroutine is'/'incompatible with a previous definition or',
     $    ' reference: ',A,I4)
        CALL ABRT
      ENDIF
C
 150  RETURN
      END
C
      SUBROUTINE COPY
C
C   This reads the generated code for one subprogram and copies it to the
C   output file, inserting MP declaration code at the marker if required.
C
      CHARACTER*80 LIN
C
      ENDFILE 11
      REWIND 11
      ENDFILE 12
      REWIND 12
C
 100  READ (11, '(A)', END = 110) LIN
      LN = LNBLK (LIN)
      IF (LN .EQ. 6 .AND. LIN(1:6) .EQ. 'CMP>>>') THEN
        CALL INIMP
      ELSE
        WRITE (6, '(A)') LIN(1:LN)
      ENDIF
      GOTO 100
C
 110  REWIND 11
      REWIND 12
      RETURN
      END
C
      SUBROUTINE DIMEN (K1, LN)
C
C   This processes dimension and common statements by delimiting variable
C   names, inserting in table if required and correcting dimensions of MP
C   variables.  K1 and LN are the indices of the first (after 'dimension' or
C   'common') and last non-blank characters in the statement.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINA
      CHARACTER*16 NAM
      CHARACTER*1 CJ
      CHARACTER*8 DIM1, DIM2, DIMY
C
C   Place the MP dimension into the character variable DIMX.
C
      WRITE (DIMY, '(I8)') MXP + 4
      I1 = NBLK (1, 8, DIMY)
      LD1 = 9 - I1
      DIM1 = DIMY(I1:8)
      WRITE (DIMY, '(I8)') 2 * MXP + 8
      I1 = NBLK (1, 8, DIMY)
      LD2 = 9 - I1
      DIM2 = DIMY(I1:8)
      J1 = K1
C
C   Output statement as a comment.
C
      CALL OUTLIN (2, LN, LINE)
C
C   Extract the next character from the line.
C
 100  IF (J1 .GT. LN) GOTO 130
      J1 = NBLK (J1, LN, LINE)
      CJ = LINE(J1:J1)
C
C   Check if it the start of a name.
C
      IF (MAX (INDEX (ALPL, CJ), INDEX (ALPU, CJ)) .NE. 0) THEN
C
        DO 110 J = J1, LN
          CJ = LINE(J:J)
          IF (MAX (INDEX (ALPL, CJ), INDEX (ALPU, CJ)) .NE. 0) GOTO 110
          IF (INDEX (DIG, CJ) .NE. 0 .OR. CJ .EQ. '_') GOTO 110
          IF (INDEX (DEL, CJ) .NE. 0) GOTO 120
          CALL ERRMES (23, 1)
 110    CONTINUE
C
        J = LN + 1
 120    J2 = J - 1
        NAM = LINE(J1:J2)
        IX = ITAB (1, 0, NAM)
        IF (IX .EQ. 0) THEN
          CALL ERRMES (24, 0)
          WRITE (6, 1) NAM
 1        FORMAT ('This Fortran keyword may not appear in a dimension', 
     $      ' or common statement: '/A)
          CALL ABRT
        ENDIF
        KTP = KTYP(IX)
        IF (KTP .LT. 8) KDEC(IX) = 1
        IF (J2 .GE. LN) GOTO 130
        K3 = NBLK (J2 + 1, LN, LINE)
        CJ = LINE(K3:K3)
C
C   Check if this variable has a dimension declaration.
C
        IF (CJ .EQ. '(') THEN
          KDIM(IX) = 1
C
C   If this is a MP variable, correct the dimension.
C
          IF (KTP .GE. 8) THEN
            LINA(1:K3) = LINE(1:K3)
            IF (KTP .LT. 10) THEN
              LINA(K3+1:K3+LD1) = DIM1(1:LD1)
              LDX = LD1
            ELSE
              LINA(K3+1:K3+LD2) = DIM2(1:LD2)
              LDX = LD2
            ENDIF
            LINA(K3+LDX+1:K3+LDX+1) = ','
            LINA(K3+LDX+2:LN+LDX+1) = LINE(K3+1:LN)
            LN = LN + LDX + 1
            LINE(1:LN) = LINA(1:LN)
          ENDIF
          J2 = MATCH (0, K3 + 1, LN, LINE)
          IF (J2 .EQ. 0) CALL ERRMES (25, 1)
          I1 = ISCAN (K3, J2, LINE)
          IF (I1 .NE. 0) THEN
            CALL ERRMES (26, 0)
            WRITE (6, 2) NAM
 2          FORMAT ('The MP dimension on this variable is not',         
     $        ' allowed: ',A)
            CALL ABRT
          ENDIF
        ENDIF
        J1 = J2 + 1
        GOTO 100
C
C   The only other character that should appear here is a comma.
C
      ELSEIF (CJ .EQ. ',') THEN
        J1 = J1 + 1
        GOTO 100
      ELSE
        CALL ERRMES (27, 1)
      ENDIF
C
 130  CALL OUTLIN (1, LN, LINE)
      WRITE (11, 3)
 3    FORMAT ('CMP<')
C
      RETURN
      END
C
      SUBROUTINE ERRMES (IA, IB)
C
C   This outputs a syntax error message with the line number.  If IB is
C   nonzero, ABRT is also called.  IA is the message code, which currently
C   is in the range 1 - 96.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      IF (IB .EQ. 0) THEN
        WRITE (6, 1) LCT, IA
 1      FORMAT ('*** Error in statement starting at line',I6,3X,'Code', 
     $    I6)
      ELSE
        WRITE (6, 2) LCT, IA
 2      FORMAT ('*** Syntax error in statement starting at line',I6,    
     $    3X,'Code',I6)
        CALL ABRT
      ENDIF
C
      RETURN
      END
C
      SUBROUTINE EXEC (K1, LN)
C
C   This handles MP executable statements.  K1 and LN are the indices of the
C   first and last non-blank characters in the statement.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*16 LINQ, UCASE
C
C   Check if this is a call statement.
C
      J1 = K1
      LQ = MIN (J1 + 15, LN)
      LINQ = UCASE (LINE(J1:LQ))
      IF (LINQ(1:5) .EQ. 'CALL ') THEN
        J1 = NBLK (J1 + 5, LN, LINE)
        I1 = INDX (J1, LN, '(', LINE)
        IF (I1 .NE. 0) THEN
          CALL CALLST (J1, LN)
          GOTO 110
        ENDIF
      ENDIF
C
C   Check if this is a read or write statement.
C
      IRW = 0
      IF (LINQ(1:4) .EQ. 'READ') THEN
        IRW = 1
        K3 = J1 + 4
      ELSEIF (LINQ(1:5) .EQ. 'WRITE') THEN
        IRW = 2
        K3 = J1 + 5
      ENDIF
      IF (IRW .NE. 0) THEN
        K3 = NBLK (K3, LN, LINE)
        IF (LINE(K3:K3) .NE. '(') GOTO 100
        J1 = K3
        J2 = MATCH (0, J1 + 1, LN, LINE)
        IF (J2 .EQ. 0) CALL ERRMES (28, 1)
        CALL RDWR (IRW, J1, J2, LN)
        GOTO 110
      ENDIF
C
C   Check if this is an assignment statement.
C
 100  CALL ASST (J1, LN)
C
 110  RETURN
      END
C
      SUBROUTINE EXPRES (LU, LA, LINA, ITPX, LX, ARGX)
C
C   This processes the arithmetic and/or logical expression in LINA, of length
C   = LA.  The result, after evaluation, is placed in ARGX, with type = ITPX,
C   and length = LX.  If the last result was =, then the ARGX is set to blanks,
C   and ITPX and LX are set to zero.  LU is the unit number for output code.
C>
C   Uncomment this line on Sun and IBM workstations.
C
      IMPLICIT AUTOMATIC (A-Z)
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINA, LINB, LINC
      CHARACTER*80 AR(2), ARG(MAR), ARG1, ARGX, ARGY
      CHARACTER*16 NAM
      DIMENSION ITAR(MAR), LAR(MAR), ITA(2), LNA(2), ID1(2), ID2(2),    
     $  IOP(2)
      CHARACTER*1 CJ
C*
C      write (lu, *) 'enter expres'
C*
C   Search for an executable operation (one that is not dependent on the
C   results of operations with higher precedence) in the statement.  The two
C   arguments of the operation, their types and lengths, are identified.
C
      LB = LA
      LINB(1:LB) = LINA(1:LA)
C
 100  I1 = 1
C*
C      write (lu, *) 'expres lb =', lb
C      write (lu, *) '%'//linb(1:lb)//'%'
C*
 110  DO 200 II = 1, 2
        I1 = NBLK (I1, LB, LINB)
        IF (I1 .EQ. 0) CALL ERRMES (29, 1)
        CJ = LINB(I1:I1)
        IX = 0
C*
C        write (lu, *) 'cj =', cj
C*
C   Check if this is the start of a numeric constant.
C
        IS1 = INDEX (DIG, CJ)
        IF ((CJ .EQ. '-' .OR. CJ .EQ. '.') .AND. I1 .LT. LB) THEN
          I2 = NBLK (I1 + 1, LB, LINB)
          IS2 = INDEX (DIG, LINB(I2:I2))
        ELSE
          IS2 = 0
        ENDIF
C
        IF (IS1 .NE. 0 .OR. IS2 .NE. 0) THEN
          IT = NUMCON (I1, I2, LB, LINB)
C
C   If the mixed mode safe flag is on, then the numeric constant is always
C   of type MP.
C
          IF (IMM .GE. 1) THEN
            IF (IT .EQ. 1) THEN
              IT = 8
            ELSEIF (IT .EQ. 2 .or. IT .EQ. 3) THEN
              IT = 9
            ENDIF
          ENDIF
          IF (IT .LE. 3) THEN
            ID1(II) = I1
            ID2(II) = I2
            ITA(II) = IT
            LNA(II) = I2 - I1 + 1
            AR(II) = LINB(I1:I2)
          ELSE
            ID1(1) = I1
            ID2(2) = I2
            ITPY = IT
            CALL GENCON (LU, I1, I2, LINB, ITPY, LY, ARGY)
            I1 = NBLK (I2 + 1, LB, LINB)
            GOTO 220
          ENDIF
C
C   Check if string is a variable name.
C
        ELSEIF (MAX (INDEX (ALPL, CJ), INDEX (ALPU, CJ)) .NE. 0) THEN
C
          DO 120 J = I1, LB
            CJ = LINB(J:J)
            IF (MAX (INDEX (ALPL, CJ), INDEX (ALPU, CJ)) .NE. 0) THEN
              GOTO 120
            ELSEIF (INDEX (DIG, CJ) .NE. 0 .OR. CJ .EQ. '_') THEN
              GOTO 120
            ELSE
              GOTO 130
            ENDIF
 120      CONTINUE
C
          J = LB + 1
C
C   Variable or function name has been identified.
C
 130      I2 = J - 1
          L1 = I2 - I1 + 1
          NAM = LINB(I1:I2)
          IX = ITAB (1, 0, NAM)
          IF (IX .EQ. 0) THEN
            CALL ERRMES (30, 0)
            WRITE (6, 1) NAM
 1          FORMAT ('This Fortran keyword may not appear in an',        
     $        ' expression: ',A)
            CALL ABRT
          ELSEIF (KTYP(IX) .EQ. 0 .AND. KDEC(IX) .EQ. 0) THEN
            CALL ERRMES (31, 0)
            WRITE (6, 2) NAM
 2          FORMAT ('This variable has not been typed: ',A)
            CALL ABRT
          ENDIF
          ID1(II) = I1
          ID2(II) = I2
          ITA(II) = KTYP(IX)
          LNA(II) = L1
          AR(II)(1:L1) = LINB(I1:I2)
C
C   Check if string is a logical constant.
C
        ELSEIF (CJ .EQ. '.' .AND. I1 .LT. LB .AND.                      
     $      (LINB(I1+1:I1+1) .EQ. 'T' .OR. LINB(I1+1:I1+1) .EQ. 't' .OR.
     $      LINB(I1+1:I1+1) .EQ. 'F' .OR. LINB(I1+1:I1+1) .EQ. 'f'))    
     $      THEN
          IF (LINB(I1:I1+2) .EQ. '.T.' .OR. LINB(I1:I1+2) .EQ. '.t.'.OR.
     $      LINB(I1:I1+2) .EQ. '.F.' .OR. LINB(I1:I1+2) .EQ. '.t.')     
     $      THEN
            I2 = I1 + 2
          ELSEIF (LINB(I1:I1+5) .EQ. '.TRUE.' .OR.                      
     $        LINB(I1:I1+5) .EQ. '.true.') THEN
            I2 = I1 + 5
          ELSEIF (LINB(I1:I1+6) .EQ. '.FALSE.' .OR.                     
     $        LINB(I1:I1+6) .EQ. '.false.') THEN
            I2 = I1 + 6
          ELSE
            CALL ERRMES (32, 1)
          ENDIF
          ID1(II) = I1
          ID2(II) = I2
          ITA(II) = 7
          LNA(II) = I2 - I1 + 1
          AR(II) = LINB(I1:I2)
C
C   Check if argument is a character constant delimited by ".
C
        ELSEIF (CJ .EQ. '"') THEN
          J1 = I1
 140      I2 = INDX (J1 + 1, LB, '"', LINB)
          IF (I2 .EQ. 0) CALL ERRMES (33, 1)
          IF (I2 .LT. LB .AND. LINB(I2+1:I2+1) .EQ. '"') THEN
            J1 = I2 + 1
            GOTO 140
          ENDIF
          ID1(II) = I1
          ID2(II) = I2
          ITA(II) = 6
          LNA(II) = I2 - I1 + 1
          AR(II) = LINB(I1:I2)
C
C   Check if argument is a character constant delimited by '.
C
        ELSEIF (CJ .EQ. "'") THEN
          J1 = I1
 150      I2 = INDX (J1 + 1, LB, "'", LINB)
          IF (I2 .EQ. 0) CALL ERRMES (34, 1)
          IF (I2 .LT. LB .AND. LINB(I2+1:I2+1) .EQ. "'") THEN
            J1 = I2 + 1
            GOTO 150
          ENDIF
          ID1(II) = I1
          ID2(II) = I2
          ITA(II) = 6
          LNA(II) = I2 - I1 + 1
          AR(II) = LINB(I1:I2)
C
C   Check if argument is a unary minus sign (i.e. argument 1 is null).
C
        ELSEIF (CJ .EQ. '-') THEN
          IF (II .EQ. 2) GOTO 110
          ID1(1) = I1
          ID2(1) = I1
          ITA(1) = 0
          LNA(1) = 1
          AR(1) = ' '
          IOP(1) = 3
          I1 = I1 + 1
          GOTO 200
C
C   Check if argument is a unary minus sign (i.e. argument 1 is null).
C
        ELSEIF (CJ .EQ. '.' .AND. I1 + 4 .LE. LB) THEN
          IF (LINB(I1:I1+4) .EQ. '.NOT.' .or. LINB(I1:I1+4) .EQ.        
     $      '.not.') THEN
            IF (II .EQ. 2) GOTO 110
            ID1(1) = I1
            ID2(1) = I1
            ITA(1) = 0
            LNA(1) = 1
            AR(1) = ' '
            IOP(1) = 15
            I1 = I1 + 5
            GOTO 200
          ENDIF
C
C   Check if the next character is a left parenthesis.  If so, evaluate the
C   expression in parentheses.
C
        ELSEIF (CJ .EQ. '(') THEN
          I2 = MATCH (0, I1 + 1, LB, LINB)
          IF (I2 .EQ. 0) CALL ERRMES (35, 1)
          LC = I2 - I1 - 1
          LINC(1:LC) = LINB(I1+1:I2-1)
          ID1(1) = I1
          ID2(2) = I2
          IF (LB .EQ. 0) CALL ERRMES (36, 1)
          CALL EXPRES (LU, LC, LINC, ITPY, LY, ARGY)
          GOTO 220
        ELSE
          CALL ERRMES (37, 0)
          WRITE (6, 3) CJ
 3        FORMAT ('Illegal character: ',A)
          CALL ABRT
        ENDIF
C
C   Check if the end of the variable or constant is the end of the statement.
C
 160    CONTINUE
        IF (I2 .EQ. LB .OR. LINB(I2+1:LB) .EQ. ' ') THEN
C
C   If this occurs on the first pass, we are done.
C
          IF (II .EQ. 1) THEN
            ITPX = ITA(1)
            LX = LNA(1)
            ARGX(1:LX) = AR(1)(1:LX)
            GOTO 230
C
C   If this occurs on the second pass, proceed to evaluate.
C
          ELSE
            GOTO 210
          ENDIF
        ENDIF
C
C   Check if the next character after the variable name is a left parenthesis.
C
        I3 = NBLK (I2 + 1, LB, LINB)
        CJ = LINB(I3:I3)
        IF (CJ .EQ. '(') THEN
          IF (IX .EQ. 0) CALL ERRMES (38, 1)
          I2 = MATCH (0, I3 + 1, LB, LINB)
          IF (I2 .EQ. 0) CALL ERRMES (39, 1)
          K1 = INDEX (LINB(I2:LB), '=')
 170      CONTINUE
C
C   Check if the subscripted variable is really a function reference.
C
          IF (KDIM(IX) .EQ. -1) THEN
            IF (K1 .NE. 0) THEN
              CALL ERRMES (40, 0)
              WRITE (6, 4) NAM(1:L1)
 4            FORMAT ('A function name may not appear on the LHS of',   
     $          ' an assignment statement: ',A)
              CALL ABRT
            ENDIF
C
C   Generate a function call.  If it is an intrinsic reference and the mixed
C   mode SAFE option is in effect, set IMM = 2 as a flag to inform ARLIST not
C   to revert to mixed mode FAST while evaluating the argument list.
C
            ID1(1) = I1
            ID2(2) = I2
            LC = I2 - I3 - 1
            LINC(1:LC) = LINB(I3+1:I2-1)
            IMS = IMM
            IF (IX .LE. MINT .AND. IMM .GE. 1) IMM = 2
            CALL ARLIST (LU, LC, LINC, NAR, ITAR, LAR, ARG)
            IMM = IMS
            IT1 = ITA(II)
            LN1 = LNA(II)
            ARG1 = AR(II)
            CALL GENFUN (LU, IT1, LN1, ARG1, NAR, ITAR, LAR, ARG, ITPY, 
     $        LY, ARGY)
            I1 = NBLK (I2 + 1, LB, LINB)
            GOTO 220
C
C   Check if the subscripted variable has a dimension.  If not, this is
C   assumed to be an external function reference.
C
          ELSEIF (KDIM(IX) .EQ. 0) THEN
            IF (K1 .EQ. 0) THEN
C
C   If this function name was earlier listed in an external statement and was
C   not explicitly typed, it must now be implicitly typed.
C
              IF (KDEC(IX) .EQ. 1 .AND. MPLC(IX) .EQ. 1) THEN
                CJ = NAM(1:1)
                IJ = MAX (INDEX (ALPU, CJ), INDEX (ALPL, CJ))
                KTYP(IX) = IMPL(IJ)
                ITA(II) = KTYP(IX)
              ENDIF
              KDEC(IX) = 1
              KDIM(IX) = -1
              KT = KTYP(IX)
              WRITE (LU, 5) NAM(1:L1), CTP(KT)
 5            FORMAT ('CMP*'/'CMP*  Undimensioned variable assumed to', 
     $          ' be an external function.'/'CMP*  Name: ',A,4X,        
     $          'Type: ',A/'CMP*')
              GOTO 170
            ELSE
              CALL ERRMES (41, 0)
              WRITE (6, 6)
 6            FORMAT ('MP variables may not be used in statement',      
     $          ' function definitions.'/'Define an external function', 
     $          ' for this purpose.')
              CALL ABRT
            ENDIF
C
C   Otherwise it must be an ordinary array with subscript.  The combination of
C   the variable name and the subscript will now be treated as a variable.
C   IX is set to 0 as a flag indicating that this has been done.
C
          ELSE
            I4 = ISCAN (I3, I2, LINB)
            IF (I4 .NE. 0) THEN
              CALL ERRMES (42, 0)
              WRITE (6, 7) NAM
 7            FORMAT ('The MP subscript on this variable is not',       
     $          ' allowed: ',A)
              CALL ABRT
            ENDIF
            L1 = I2 - I1 + 1
            ID2(II) = I2
            LNA(II) = L1
            AR(II)(1:L1) = LINB(I1:I2)
            IX = 0
            GOTO 160
          ENDIF
        ELSE
C
C   The variable does not have a subscript.  Check if it has a dimension.
C
          IF (IX .GT. 0) THEN
            IF (KDIM(IX) .GT. 0) THEN
              CALL ERRMES (43, 0)
              WRITE (6, 8) NAM(1:L1)
 8            FORMAT ('This dimensioned variable is used without a',    
     $          ' subscript: ',A)
              CALL ABRT
            ENDIF
          ENDIF
        ENDIF
        I1 = I3
C
C   Identify the operator.
C
        DO 180 I = 1, NOP
          L1 = LOP(I) - 1
          IF (LINB(I1:I1+L1) .EQ. LOPR(I) .OR. LINB(I1:I1+L1) .EQ.      
     $      UOPR(I)) GOTO 190
 180    CONTINUE
C
        CALL ERRMES (44, 0)
        WRITE (6, 9) LINB(I1:I1)
 9      FORMAT ('Illegal operator: ',A)
        CALL ABRT
C
 190    IOP(II) = I
        I1 = I1 + LOP(I)
 200  CONTINUE
C
C   Compare the precedence levels of the two operators.
C
      IF (KOP(IOP(1)) .LT. KOP(IOP(2))) THEN
        I1 = ID1(2)
        GOTO 110
      ENDIF
C
C   An operation can be performed.
C
 210  CALL GEN (LU, ITA, LNA, AR, IOP(1), ITPY, LY, ARGY)
C
C   Replace the two operands and the operator with the result in LINB.
C
 220  IF (LY .NE. 0) THEN
        I1 = ID1(1)
        I2 = ID2(2)
        L1 = I2 - I1 + 1
        LD = LY - L1
        IF (I1 .GT. 0) LINC(1:I1-1) = LINB(1:I1-1)
        LINC(I1:I1+LY-1) = ARGY(1:LY)
        IF (LB .GT. I2) LINC(I1+LY:LB+LD) = LINB(I2+1:LB)
        LB = LB + LD
        LINB(1:LB) = LINC(1:LB)
        GOTO 100
      ELSE
        ITPX = ITPY
        LX = LY
        ARGX = ARGY
      ENDIF
C
C   Finished at last.
C
 230  CONTINUE
C*
C      write (lu, *) 'exit express, argx = %'//argx(1:lx)//'%'
C*
      RETURN
      END
C
      SUBROUTINE FIXSUB (K1, K2, LN)
C
C   This routine prepends '1,' to subscripts MP variables in LINE between K1
C   and K2.  LN is the length of the full line.  It also changes the names of
C   the special constants when found.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINX
      CHARACTER*16 NAM, NAMQ
      CHARACTER*1 CJ
C
      J1 = K1
      NAMQ = 'MPNWP'
      IQNWP = ITAB (0, 0, NAMQ)
      NAMQ = 'DPPIC'
      IQPIC = ITAB (0, 0, NAMQ)
      IQD = IQPIC - IQNWP + 1
C
C   Extract the next character from the line.
C
 100  IF (J1 .GT. K2) GOTO 130
      J1 = NBLK (J1, K2, LINE)
      CJ = LINE(J1:J1)
C
C   Check if it is the start of a numeric constant.
C
      IS1 = INDEX (DIG, CJ)
      IF ((CJ .EQ. '-' .OR. CJ .EQ. '.') .AND. J1 .LT. K2) THEN
        J2 = NBLK (J1 + 1, K2, LINE)
        IS2 = INDEX (DIG, LINE(J2:J2))
      ELSE
        IS2 = 0
      ENDIF
C
      IF (IS1 .NE. 0 .OR. IS2 .NE. 0) THEN
        ITP = NUMCON (J1, J2, LN, LINE)
        J1 = J2 + 1
        GOTO 100
C
C   Check if it the start of a name.
C
      ELSEIF (MAX (INDEX (ALPL, CJ), INDEX (ALPU, CJ)) .NE. 0) THEN
C
        DO 110 J = J1, K2
          CJ = LINE(J:J)
          IF (INDEX (DEL, CJ) .NE. 0) GOTO 120
 110    CONTINUE
C
        J = K2 + 1
C
C   The variable has been identified.
C
 120    I2 = J - 1
        NAM = LINE(J1:I2)
        IX = ITAB (0, 0, NAM)
        IF (IX .EQ. 0) THEN
          J1 = I2 + 1
          GOTO 100
        ENDIF
        ITP = KTYP(IX)
C
C   Check if the variable is the function value.  If so, change its name.
C
        IF (KDIM(IX) .EQ. -2) THEN
          LX = LN - I2 + 5
          LINX(1:5) = 'MPFVX'
          LINX(6:LX) = LINE(I2+1:LN)
          LD = J1 - I2 + 4
          K2 = K2 + LD
          LN = LN + LD
          LINE(J1:LN) = LINX(1:LX)
          J1 = J1 + 5
          GOTO 100
C
C   Check if the variable is a special constant.  If so, change its name.
C
        ELSEIF (IX .GE. IQNWP .AND. IX .LE. IQPIC) THEN
          LINE(J1:I2) = VAR(IX+IQD)
          KCON(IX) = 1
          J1 = I2 + 1
          GOTO 100
C
C   Check if the variable is MP.
C
        ELSEIF (ITP .GE. 8) THEN
C
C   Check if this MP variable has a subscript.
C
          I1 = NBLK (I2 + 1, K2, LINE)
          IF (I1 .EQ. 0) GOTO 130
          IF (LINE(I1:I1) .EQ. '(') THEN
            IF (KDIM(IX) .GT. 0) THEN
              LX = LN - I1 + 2
              LINX(1:2) = '1,'
              LINX(3:LX) = LINE(I1+1:LN)
              LINE(I1+1:LN+2) = LINX(1:LX)
              K2 = K2 + 2
              LN = LN + 2
              J1 = INDX (I1 + 1, K2, ')', LINE)
              IF (J1 .EQ. 0) CALL ERRMES (45, 1)
            ENDIF
          ELSE
            J1 = I2 + 1
            GOTO 100
          ENDIF
        ENDIF
        J1 = I2 + 1
        GOTO 100
C
C   Check if it is the start of a logical constant.
C
      ELSEIF (CJ .EQ. '.') THEN
        I1 = INDX (J1 + 1, K2, '.', LINE)
        IF (I1 .EQ. 0) CALL ERRMES (46, 1)
        J1 = I1 + 1
        GOTO 100
C
C   Check if it is the start of a character constant.
C
      ELSEIF (CJ .EQ. '"') THEN
        I1 = INDX (J1 + 1, K2, '"', LINE)
        IF (I1 .EQ. 0) CALL ERRMES (47, 1)
        J1 = I1 + 1
        GOTO 100
      ELSEIF (CJ .EQ. "'") THEN
        I1 = INDX (J1 + 1, K2, "'", LINE)
        IF (I1 .EQ. 0) CALL ERRMES (48, 1)
        J1 = I1 + 1
        GOTO 100
      ENDIF
C
C   Check if it is one of the miscellaneous symbols.
C
      I1 = INDEX (DEL, CJ)
      IF (I1 .EQ. 0) CALL ERRMES (49, 1)
      J1 = J1 + 1
      GOTO 100
C
 130  RETURN
      END
C
      SUBROUTINE GEN (LU, ITA, LNA, AR, IOP, ITPX, LX, ARGX)
C
C   This generates code for a single operation.  The two input argument names
C   are in AR, with types in ITA and lengths in LNA.  The operation code is in
C   IOP.  The result (ordinarily a temporary variable name, but empty in case
C   of assignments) is placed in ARGX, with type ITPX and length LX.  LU is
C   the unit number for output code.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINA
      CHARACTER*80 AR(2), ARG1, ARG2, ARG3, ARGX
      CHARACTER*4 TMP1, TMP2, TMP3, TMP4, TMP5, TMP6, TMP7, TMP8, GETMP
      DIMENSION ITA(2), LNA(2)
C
      ITP1 = ITA(1)
      L1 = LNA(1)
      ARG1(1:L1) = AR(1)(1:L1)
      ITP2 = ITA(2)
      L2 = LNA(2)
      ARG2(1:L2) = AR(2)(1:L2)
C*
C      write (lu, *) 'enter gen ', itp1, itp2, ' ', uopr(iop)
C      write (lu, *) 'args: ', arg1(1:l1), '  ', arg2(1:l2)
C*
C   Check for character entities with non-character entities.
C
      IF (ITP1 .EQ. 6 .AND. ITP2 .NE. 6 .OR. ITP1 .NE. 6 .AND.          
     $  ITP2 .EQ. 6) GOTO 110
C
C   Check for logical entities with non-logical entities.
C
      IF (ITP1 .EQ. 7 .AND. ITP2 .NE. 7 .OR. ITP1 .NE. 0 .AND.          
     $  ITP1 .NE. 7 .AND. ITP2 .EQ. 7) GOTO 110
C
C   Check for assignments.
C
      IF (IOP .EQ. 1) THEN
        ITPX = 0
        ARGX(1:4) = ' '
        LX = 0
        CALL GENASN (LU, ITP1, L1, ARG1, ITP2, L2, ARG2)
        GOTO 100
      ENDIF
C
C   Handle all other operations here.  The result variable will be a
C   temporary variable and the result type is the max of the two type numbers,
C   except for comparisons and a couple of other mixed mode cases.
C
      IF (IOP .LE. 6) THEN
        IF (ITP1 .EQ. 8 .AND. (ITP2 .EQ. 2 .OR. ITP2 .EQ. 3) .OR.       
     $    ITP2 .EQ. 8 .AND. (ITP1 .EQ. 2 .OR. ITP1 .EQ. 3)) THEN
          ITPX = 9
        ELSEIF (ITP1 .GE. 8 .AND. (ITP2 .EQ. 4 .OR. ITP2 .EQ. 5) .OR.   
     $      ITP2 .GE. 8 .AND. (ITP1 .EQ. 4 .OR. ITP1 .EQ. 5)) THEN
          ITPX = 10
        ELSE
          ITPX = MAX (ITP1, ITP2)
        ENDIF
C
C   If the mixed mode safe option is in effect, change IN, SP, DP, CO and DC
C   results to the appropriate MP type.
C
        IF (IMM .GE. 1) THEN
          IF (ITPX .EQ. 1) THEN
            ITPX = 8
            ITP3 = 8
          ELSEIF (ITPX .EQ. 2 .OR. ITPX .EQ. 3) THEN
            ITPX = 9
            ITP3 = 9
          ELSEIF (ITPX .EQ. 4 .OR. ITPX .EQ. 5) THEN
            ITPX = 10
            ITP3 = 10
          ENDIF
        ENDIF
      ELSE
        ITPX = 7
      ENDIF
C
C   Check if the operation is really the definition of a MPI or MPR constant.
C
      IF (IOP .EQ. 2 .AND. L2 .EQ. 1 .AND. ARG2(1:1) .EQ. '0') THEN
        IF (INDEX (DIG, ARG1(1:1)) .NE. 0 .OR. ARG1(1:1) .EQ. '.' .OR.  
     $    ARG1(1:1) .EQ. '-' .OR. ARG1(1:1) .EQ. '+') THEN
          LINA(1:L1) = ARG1(1:L1)
          IF (ITP1 .EQ. 1 .OR. ITP1 .EQ. 8) THEN
            ITPX = 8
          ELSEIF (ITP1 .EQ. 2 .OR. ITP1 .EQ. 3 .OR. ITP1 .EQ. 9) THEN
            ITPX = 9
          ENDIF
          CALL GENCON (LU, 1, L1, LINA, ITPX, LX, ARGX)
        ELSE
          ITPX = ITP1
          LX = L1
          ARGX(1:LX) = ARG1(1:L1)
        ENDIF
        GOTO 120
      ENDIF
      LX = 4
      ARGX(1:4) = GETMP (ITPX)
C
C   Generate code for non-MP operands.
C
      IF (ITP1 .LT. 8 .AND. ITP2 .LT. 8) THEN
        IF (IMM .EQ. 0 .OR. ITP1 .EQ. 7 .AND. ITP2 .EQ. 7 .OR.          
     $    IOP .GE. 7 .OR. IOP .EQ. 15) THEN
C
C   Either mixed mode fast option is in effect, or else the operation is a
C   comparison or logical operation.  Generate a simple one line non-MP
C   statement.
C
          WRITE (LU, 1) ARGX(1:4), ARG1(1:L1), UOPR(IOP)(1:LOP(IOP)),   
     $      ARG2(1:L2)
 1        FORMAT (6X,A,' = ',A,' ',A,' ',A)
          GOTO 100
        ELSE
C
C   The mixed mode safe option is in effect.  Convert the left operand to
C   the appropriate MP type so that it will be evaluated using the MP routines.
C
          TMP1 = GETMP (ITP3)
          ARG3(1:4) = TMP1
          L3 = 4
          CALL GENASN (LU, ITP3, L3, ARG3, ITP1, L1, ARG1)
C
C   If ARG1 is a temporary, release it.  Then set ARG1 = ARG3 and generate
C   code for this operation using the appropriate GENXXX routine.
C
          IF (L1 .EQ. 4) THEN
            IF (ARG1(1:2) .EQ. 'MP') THEN
              TMP1 = ARG1(1:4)
              CALL RLTMP (TMP1)
            ENDIF
          ENDIF
          ITP1 = ITP3
          L1 = L3
          ARG1(1:L1) = ARG3(1:L3)
        ENDIF
      ENDIF
C
C   Check if operation is a plus.
C
      IF (IOP .EQ. 2) THEN
        CALL GENADD (LU, ITP1, L1, ARG1, ITP2, L2, ARG2, ARGX)
C
C   Check if the operation is minus.
C
      ELSEIF (IOP .EQ. 3) THEN
        CALL GENSUB (LU, ITP1, L1, ARG1, ITP2, L2, ARG2, ARGX)
C
C   Check if the operation is exponentiation.
C
      ELSEIF (IOP .EQ. 4) THEN
        CALL GENEXP (LU, ITP1, L1, ARG1, ITP2, L2, ARG2, ARGX)
C
C   Check if the operation is multiplication.
C
      ELSEIF (IOP .EQ. 5) THEN
        CALL GENMUL (LU, ITP1, L1, ARG1, ITP2, L2, ARG2, ARGX)
C
C   Check if the operation is division.
C
      ELSEIF (IOP .EQ. 6) THEN
        CALL GENDIV (LU, ITP1, L1, ARG1, ITP2, L2, ARG2, ARGX)
C
C   Check if the operation is comparison.
C
      ELSEIF (IOP .GE. 7 .AND. IOP .LE. 12) THEN
        CALL GENCPR (LU, ITP1, L1, ARG1, ITP2, L2, ARG2, IOP, ARGX)
        GOTO 100
      ELSE
        GOTO 110
      ENDIF
C
C   If the result is of type MPI, truncate the result.
C
      IF (ITPX .EQ. 8) THEN
        TMP1 = GETMP (9)
        WRITE (LU, 2) ARGX(1:4), ARGX(1:4), TMP1
 2      FORMAT (6X,'CALL MPINFR (',A,', ',A,', ',A,')')
        CALL RLTMP (TMP1)
      ENDIF
C
C   Release any temporaries among the arguments.
C
 100  IF (L1 .EQ. 4) THEN
        IF (ARG1(1:2) .EQ. 'MP') THEN
          TMP1 = ARG1(1:4)
          CALL RLTMP (TMP1)
        ENDIF
      ENDIF
      IF (L2 .EQ. 4) THEN
        IF (ARG2(1:2) .EQ. 'MP') THEN
          TMP1 = ARG2(1:4)
          CALL RLTMP (TMP1)
        ENDIF
      ENDIF
      GOTO 120
C
 110  CALL ERRMES (50, 0)
      WRITE (6, 3) UOPR(IOP)(1:LOP(IOP))
 3    FORMAT ('Operation ',A,' is not defined with these operands.')
      CALL ABRT
C
 120  CONTINUE
C*
C      write (lu, *) 'exit gen itpx, lx, argx =', itpx, lx, ' ',
C     $  argx(1:lx)
C*
      RETURN
      END
C
      SUBROUTINE GENADD (LU, ITP1, L1, ARG1, ITP2, L2, ARG2, ARGX)
C
C   This generates code for an add operation.  The operands are in ARG1 and
C   ARG2, with types ITP1 and ITP2, and with lengths L1 and L2.  The result
C   name is in ARGX (also input).  LU is the unit number for output code.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*80 ARG1, ARG2, ARGX
      CHARACTER*4 TMP1, TMP2, TMP3, TMP4, TMP5, TMP6, TMP7, TMP8, GETMP
C
      IF (ITP1 .LT. 8) THEN
        IF (ITP1 .EQ. 3) THEN
C
C   (Arg1 is DP) and (Arg2 is MP).
C
          TMP1 = GETMP (9)
          IF (ITP2 .EQ. 10) THEN
            WRITE (LU, 1) ARG2(1:L2), ARGX(1:4)
 1          FORMAT (6X,'CALL MPCEQ (MPNW4, ',A,', ',A,')')
          ENDIF
          WRITE (LU, 2) ARG1(1:L1), TMP1
 2        FORMAT (6X,'CALL MPDMC (',A,', 0, ',A,')')
          WRITE (LU, 3) TMP1, ARG2(1:L2), ARGX(1:4)
 3        FORMAT (6X,'CALL MPADD (',A,', ',A,', ',A,')')
          CALL RLTMP (TMP1)
        ELSEIF (ITP1 .EQ. 1 .OR. ITP1 .EQ. 2) THEN
C
C   (Arg1 is IN or SP) and (Arg2 is MP).
C
          TMP1 = GETMP (3)
          TMP2 = GETMP (9)
          IF (ITP2 .EQ. 10) THEN
            WRITE (LU, 1) ARG2(1:L2), ARGX(1:4)
          ENDIF
          WRITE (LU, 4) TMP1, ARG1(1:L1)
 4        FORMAT (6X,A,' = ',A)
          WRITE (LU, 2) TMP1, TMP2
          WRITE (LU, 3) TMP2, ARG2(1:L2), ARGX(1:4)
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
        ELSE
C
C   (Arg1 is CO or DC) and (Arg2 is MP).
C
          TMP1 = GETMP (3)
          TMP2 = GETMP (3)
          TMP3 = GETMP (9)
          TMP4 = GETMP (9)
          TMP5 = GETMP (10)
          IF (ITP1 .EQ. 5) THEN
            WRITE (LU, 5) TMP1, ARG1(1:L1)
 5          FORMAT (6X,A,' = DREAL (',A,')')
            WRITE (LU, 6) TMP2, ARG1(1:L1)
 6          FORMAT (6X,A,' = DIMAG (',A,')')
          ELSE
            WRITE (LU, 7) TMP1, ARG1(1:L1)
 7          FORMAT (6X,A,' = REAL (',A,')')
            WRITE (LU, 8) TMP2, ARG1(1:L1)
 8          FORMAT (6X,A,' = AIMAG (',A,')')
          ENDIF
          WRITE (LU, 2) TMP1, TMP3
          WRITE (LU, 2) TMP2, TMP4
          WRITE (LU, 9) TMP3, TMP4, TMP5
 9        FORMAT (6X,'CALL MPMMPC (',A,', ',A,', MPNW4, ',A,')')
          IF (ITP2 .EQ. 10) THEN
            WRITE (LU, 10) TMP5, ARG2(1:L2), ARGX(1:4)
 10         FORMAT (6X,'CALL MPCADD (MPNW4, ',A,', ',A,', ',A,')')
          ELSE
            TMP6 = GETMP (10)
            WRITE (LU, 11) TMP3
 11         FORMAT (6X,'CALL MPDMC (0.D0, 0, ',A,')')
            WRITE (LU, 9) ARG2(1:L2), TMP3, TMP6
            WRITE (LU, 10) TMP5, TMP6, ARGX(1:4)
            CALL RLTMP (TMP6)
          ENDIF
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
          CALL RLTMP (TMP3)
          CALL RLTMP (TMP4)
          CALL RLTMP (TMP5)
        ENDIF
      ELSEIF (ITP2 .LT. 8) THEN
        IF (ITP2 .EQ. 3) THEN
C
C   (Arg1 is MP) and (Arg2 is DP).
C
          TMP1 = GETMP (9)
          IF (ITP1 .EQ. 10) THEN
            WRITE (LU, 1) ARG1(1:L1), ARGX(1:4)
          ENDIF
          WRITE (LU, 2) ARG2(1:L2), TMP1
          WRITE (LU, 3) ARG1(1:L1), TMP1, ARGX(1:4)
          CALL RLTMP (TMP1)
        ELSEIF (ITP2 .EQ. 1 .OR. ITP2 .EQ. 2) THEN
C
C   (Arg1 is MP) and (Arg2 is IN or SP).
C
          TMP1 = GETMP (3)
          TMP2 = GETMP (9)
          IF (ITP1 .EQ. 10) THEN
            WRITE (LU, 1) ARG1(1:L1), ARGX(1:4)
          ENDIF
          WRITE (LU, 4) TMP1, ARG2(1:L2)
          WRITE (LU, 2) TMP1, TMP2
          WRITE (LU, 3) ARG1(1:L1), TMP2, ARGX(1:4)
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
        ELSE
C
C   (Arg1 is MP) and (Arg2 is CO or DC).
C
          TMP1 = GETMP (3)
          TMP2 = GETMP (3)
          TMP3 = GETMP (9)
          TMP4 = GETMP (9)
          TMP5 = GETMP (10)
          IF (ITP2 .EQ. 5) THEN
            WRITE (LU, 5) TMP1, ARG2(1:L2)
            WRITE (LU, 6) TMP2, ARG2(1:L2)
          ELSE
            WRITE (LU, 7) TMP1, ARG2(1:L2)
            WRITE (LU, 8) TMP2, ARG2(1:L2)
          ENDIF
          WRITE (LU, 2) TMP1, TMP3
          WRITE (LU, 2) TMP2, TMP4
          WRITE (LU, 9) TMP3, TMP4, TMP5
          IF (ITP1 .EQ. 10) THEN
            WRITE (LU, 10) ARG1(1:L1), TMP5, ARGX(1:4)
          ELSE
            TMP6 = GETMP (10)
            WRITE (LU, 11) TMP3
            WRITE (LU, 9) ARG1(1:L1), TMP3, TMP6
            WRITE (LU, 10) TMP6, TMP5, ARGX(1:4)
            CALL RLTMP (TMP6)
          ENDIF
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
          CALL RLTMP (TMP3)
          CALL RLTMP (TMP4)
          CALL RLTMP (TMP5)
        ENDIF
      ELSEIF (ITP1 .NE. 10 .AND. ITP2 .NE. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is MPI or MPR).
C
        WRITE (LU, 3) ARG1(1:L1), ARG2(1:L2), ARGX(1:4)
      ELSEIF (ITP1 .NE. 10 .AND. ITP2 .EQ. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is MPC).
C
        TMP1 = GETMP (3)
        TMP2 = GETMP (10)
        WRITE (LU, 11) TMP1
        WRITE (LU, 9) ARG1(1:L1), TMP1, TMP2
        WRITE (LU, 10) TMP2, ARG2(1:L2), ARGX(1:4)
        CALL RLTMP (TMP1)
        CALL RLTMP (TMP2)
      ELSEIF (ITP1 .EQ. 10 .AND. ITP2 .NE. 10) THEN
C
C   (Arg1 is MPC) and (Arg1 is MPI or MPR).
C
        TMP1 = GETMP (3)
        TMP2 = GETMP (10)
        WRITE (LU, 11) TMP1
        WRITE (LU, 9) ARG2(1:L2), TMP1, TMP2
        WRITE (LU, 10) ARG1(1:L1), TMP2, ARGX(1:4)
        CALL RLTMP (TMP1)
        CALL RLTMP (TMP2)
      ELSE
C
C   (Arg1 is MPC) and (Arg2 is MPC).
C
        WRITE (LU, 10) ARG1(1:L1), ARG2(1:L2), ARGX(1:4)
      ENDIF
C
      RETURN
      END
C
      SUBROUTINE GENASN (LU, ITP1, L1, ARG1, ITP2, L2, ARG2)
C
C   This generates code for an assign operation.  The operands are in ARG1 and
C   ARG2, with types ITP1 and ITP2, and with lengths L1 and L2.  There is no
C   "result" temporary with assignments.  LU is the unit number for output
C   code.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*80 ARG1, ARG2, ARGX
      CHARACTER*16 NAM, NAMQ
      CHARACTER*4 TMP1, TMP2, TMP3, TMP4, TMP5, TMP6, TMP7, TMP8, GETMP
C
      NAMQ = 'MPNWP'
      IQNWP = ITAB (0, 0, NAMQ)
      NAMQ = 'DPPIC'
      IQPIC = ITAB (0, 0, NAMQ)
      IQD = IQPIC - IQNWP + 1
      ISC = IQNWP + 2 * IQD - 1
C
C   Check if Arg1 is an appropriate name for the result of an assignment
C   (i.e. it must not be special constant or a function name or parameter).
C
      K = INDEX (ARG1, '(')
      IF (K .EQ. 0) K = 100
      L = MIN (K - 1, L1, 16)
      NAM = ARG1(1:L)
      IX = ITAB (0, 0, NAM)
      IF (IX .GT. IQPIC .AND. IX .LE. ISC .AND. LU .EQ. 12) THEN
        IX = IX - IQD
        WRITE (LU, 1) VAR(IX)(1:LVAR(IX))
 1      FORMAT ('CMP*'/'CMP*  The parameter definition of this special',
     $    ' constant is ignored: ',A/'CMP*')
        GOTO 100
      ELSEIF (KTYP(IX) .LE. 8 .AND. LU .EQ. 12) THEN
        CALL ERRMES (107, 0)
        WRITE (6, 2) NAM(1:L)
 2      FORMAT ('Non-MP parameters may not be defined in a MP',         
     $    ' parameter statement: ',A)
        CALL ABRT
      ELSEIF (IX .LE. MINT .OR. KDIM(IX) .LT. 0) THEN
        CALL ERRMES (51, 0)
        WRITE (6, 3) NAM(1:L)
 3      FORMAT ('This name may not appear on the LHS of an assignment', 
     $    ' statement: ',A)
        CALL ABRT
      ELSEIF (LU .EQ. 12) THEN
C
C   If this is a MP parameter definition, set KDIM(IX) to -3 so that it can't
C   be stored into again.
C
        KDIM(IX) = -3
      ENDIF
C
      IF (ITP1 .LT. 8) THEN
        IF (ITP2 .LT. 8) THEN
C
C   (LHS is non-MP) and (RHS is non-MP).
C
          WRITE (LU, 4) ARG1(1:L1), ARG2(1:L2)
 4        FORMAT (6X,A,' = ',A)
        ELSEIF (ITP1 .LE. 3 .OR. ITP2 .NE. 10) THEN
C
C   (LHS is IN, SP or DP) or (RHS is MPI or MPR).  At least one is not complex.
C
          TMP1 = GETMP (3)
          TMP2 = GETMP (1)
          WRITE (LU, 5) ARG2(1:L2), TMP1, TMP2
 5        FORMAT (6X,'CALL MPMDC (',A,', ',A,', ',A,')')
          WRITE (LU, 6) ARG1(1:L1), TMP1, TMP2
 6        FORMAT (6X,A,' = ',A,' * 2.D0 ** ',A)
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
        ELSE
C
C   (LHS is CO or DC) and (RHS is MPC).
C
          TMP1 = GETMP (9)
          TMP2 = GETMP (9)
          TMP3 = GETMP (3)
          TMP4 = GETMP (3)
          TMP5 = GETMP (1)
          WRITE (LU, 7) ARG2(1:L2), TMP1, TMP2
 7        FORMAT (6X,'CALL MPMPCM (MPNW4, ',A,', ',A,', ',A,')')
          WRITE (LU, 5) TMP1, TMP3, TMP5
          WRITE (LU, 6) TMP3, TMP3, TMP5
          WRITE (LU, 5) TMP2, TMP4, TMP5
          WRITE (LU, 6) TMP4, TMP4, TMP5
          WRITE (LU, 8) ARG1(1:L1), TMP3, TMP4
 8        FORMAT (6X,A,' = DCMPLX (',A,', ',A,')')
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
          CALL RLTMP (TMP3)
          CALL RLTMP (TMP4)
          CALL RLTMP (TMP5)
        ENDIF
      ELSEIF (ITP2 .LT. 8) THEN
        IF (ITP2 .EQ. 3) THEN
C
C   (LHS is MP) and (RHS is DP).
C
          WRITE (LU, 9) ARG2(1:L2), ARG1(1:L1)
 9        FORMAT (6X,'CALL MPDMC (',A,', 0, ',A,')')
        ELSEIF (ITP1 .NE. 10 .OR. ITP2 .EQ. 1 .OR. ITP2 .EQ. 2) THEN
C
C   (LHS is MP) or (RHS is IN or SP).  At least one is not complex.
C
          TMP1 = GETMP (3)
          WRITE (LU, 4) TMP1, ARG2(1:L2)
          WRITE (LU, 9) TMP1, ARG1(1:L1)
          CALL RLTMP (TMP1)
        ELSE
C
C   (LHS is MPC) and (RHS is CO or DC).
C
          TMP1 = GETMP (3)
          TMP2 = GETMP (3)
          TMP3 = GETMP (9)
          TMP4 = GETMP (9)
          IF (ITP2 .EQ. 5) THEN
            WRITE (LU, 10) TMP1, ARG2(1:L2)
 10         FORMAT (6X,A,' = DREAL (',A,')')
            WRITE (LU, 11) TMP2, ARG2(1:L2)
 11         FORMAT (6X,A,' = DIMAG (',A,')')
          ELSE
            WRITE (LU, 12) TMP1, ARG2(1:L2)
 12         FORMAT (6X,A,' = REAL (',A,')')
            WRITE (LU, 13) TMP2, ARG2(1:L2)
 13         FORMAT (6X,A,' = AIMAG (',A,')')
          ENDIF
          WRITE (LU, 9) TMP1, TMP3
          WRITE (LU, 9) TMP2, TMP4
          WRITE (LU, 14) TMP3, TMP4, ARG1(1:L1)
 14       FORMAT (6X,'CALL MPMMPC (',A,', ',A,', MPNW4, ',A,')')
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
          CALL RLTMP (TMP3)
          CALL RLTMP (TMP4)
        ENDIF
      ELSEIF (ITP1 .NE. 10 .OR. ITP2 .NE. 10) THEN
C
C   (LHS is MPI or MPR) or (RHS is MPI or MPR).  At least one is not MPC.
C
        WRITE (LU, 15) ARG2(1:L2), ARG1(1:L1)
 15     FORMAT (6X,'CALL MPEQ (',A,', ',A,')')
      ELSE
C
C   (LHS is MPC) and (RHS is MPC).
C
        WRITE (LU, 16) ARG2(1:L2), ARG1(1:L1)
 16     FORMAT (6X,'CALL MPCEQ (MPNW4, ',A,', ',A,')')
      ENDIF
      IF (ITP1 .EQ. 8 .AND. ITP2 .NE. 1 .AND. ITP2 .NE. 8) THEN
C
C   Truncate the result if (LHS is MPI) and (RHS is not IN or MPI).
C
        TMP1 = GETMP (9)
        WRITE (LU, 17) ARG1(1:L1), ARG1(1:L1), TMP1
 17     FORMAT (6X,'CALL MPINFR (',A,', ',A,', ',A,')')
        CALL RLTMP (TMP1)
      ELSEIF (ITP1 .EQ. 10 .AND. ITP2 .NE. 4 .AND. ITP2 .NE. 5 .AND.    
     $    ITP2 .NE. 10) THEN
C
C   Zero the imaginary part if (LHS is MPC) and (RHS is not CO or DC or MPC).
C
        TMP1 = GETMP (9)
        WRITE (LU, 18) TMP1
 18     FORMAT (6X,'CALL MPDMC (0.D0, 0, ',A,')')
        WRITE (LU, 14) ARG1(1:L1), TMP1, ARG1(1:L1)
        CALL RLTMP (TMP1)
      ENDIF
C
 100  RETURN
      END
C
      SUBROUTINE GENCON (LU, I1, I2, LINA, ITPY, LY, ARGY)
C
C   This generates the code for a MPR constant.  I1 and I2 are the indices
C   of LINA delimiting the constant.  The output MP temporary is placed in
C   ARGY, with length LY.  LU is the unit number for output code.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINA, LINB
      CHARACTER*80 ARGY
      CHARACTER*4 NUM, TMP1, TMP2, TMP3, GETMP
C
      TMP1 = GETMP (11)
      TMP2 = GETMP (6)
      TMP3 = GETMP (1)
      LY = 4
      ARGY(1:4) = GETMP (ITPY)
      LA = I2 - I1 + 1
      L2 = MAX (INDEX (LINA(I1:I2), 'D'), INDEX (LINA(I1:I2), 'd'),     
     $  INDEX (LINA(I1:I2), 'E'), INDEX (LINA(I1:I2), 'e')) - 1
      IF (L2 .GE. 0) THEN
        L1 = LA - L2 - 1
      ELSE
        L1 = 0
        L2 = LA
      ENDIF
      LINB(1:6) = ' '
      LINB(7:10) = TMP1
      LINB(11:17) = " = '10^"
      IF (L1 .GT. 0) THEN
        LINB(18:L1+17) = LINA(I1+L2+1:I2)
      ELSE
        L1 = 1
        LINB(18:18) = '0'
      ENDIF
      LINB(L1+18:L1+20) = ' x '
      LINB(L1+21:L1+L2+20) = LINA(I1:I1+L2-1)
      LINB(L1+L2+21:L1+L2+21) = "'"
      L = L1 + L2 + 21
      IF (LU .EQ. 11) THEN
        CALL OUTLIN (1, L, LINB)
      ELSE
        CALL OUTLIN (3, L, LINB)
      ENDIF
      LS = L1 + L2 + 6
      LSM = MAX (LSM, LS)
      WRITE (NUM, '(I3)') LS
      WRITE (LU, 1) TMP1, NUM(1:3), TMP2, TMP3, TMP3, NUM(1:3)
 1    FORMAT (6X,'READ (',A,", '(",A,"A1)' ) (",A,'(',A,'), ',A,        
     $  ' = 1, ',A,')')
      WRITE (LU, 2) TMP2, LS, ARGY(1:4)
 2    FORMAT (6X,'CALL MPINPC (',A,', ',I3,', ',A,')')
      CALL RLTMP (TMP3)
C
      RETURN
      END
C
      SUBROUTINE GENCPR (LU, ITP1, L1, ARG1, ITP2, L2, ARG2, IOP, ARGX)
C
C   This generates code for a compare operation.  The operands are in ARG1 and
C   ARG2, with types ITP1 and ITP2, and with lengths L1 and L2.  The result
C   name is in ARGX (also input).  LU is the unit number for output code.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*80 ARG1, ARG2, ARGX
      CHARACTER*8 ANDOR
      CHARACTER*4 TMP1, TMP2, TMP3, TMP4, TMP5, TMP6, TMP7, TMP8, GETMP
C
C   If one of the operands has a complex type, only .EQ. and .NE. are allowed.
C
      IF (ITP1 .EQ. 4 .OR. ITP1 .EQ. 5 .OR. ITP1 .EQ. 10 .OR.           
     $  ITP2 .EQ. 4 .OR. ITP2 .EQ. 5 .OR. ITP2 .EQ. 10) THEN
        IF (IOP .GE. 9) GOTO 100
        IF (IOP .EQ. 7) THEN
          ANDOR = '.AND.'
        ELSE
          ANDOR = '.OR.'
        ENDIF
      ENDIF
C
      IF (ITP1 .LT. 8) THEN
        IF (ITP1 .EQ. 3) THEN
          IF (ITP2 .NE. 10) THEN
C
C   (Arg1 is DP) and (Arg2 is MPI or MPR).
C
            TMP1 = GETMP (9)
            TMP2 = GETMP (1)
            WRITE (LU, 1) ARG1(1:L1), TMP1
 1          FORMAT (6X,'CALL MPDMC (',A,', 0, ',A,')')
            WRITE (LU, 2) TMP1, ARG2(1:L2), TMP2
 2          FORMAT (6X,'CALL MPCPR (',A,', ',A,', ',A,')')
            WRITE (LU, 3) ARGX(1:4), TMP2, UOPR(IOP)(1:LOP(IOP))
 3          FORMAT (6X,A,' = ',A,' ',A,' 0')
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
          ELSE
C
C   (Arg1 is DP) and (Arg2 is MPC).
C
            TMP1 = GETMP (9)
            TMP2 = GETMP (9)
            TMP3 = GETMP (9)
            TMP4 = GETMP (9)
            TMP5 = GETMP (1)
            TMP6 = GETMP (1)
            WRITE (LU, 1) ARG1(1:L1), TMP1
            WRITE (LU, 4) TMP2
 4          FORMAT (6X,'CALL MPDMC (0.D0, 0, ',A,')')
            WRITE (LU, 5) ARG2(1:L2), TMP3, TMP4
 5          FORMAT (6X,'CALL MPMPCM (MPNW4, ',A,', ',A,', ',A,')')
            WRITE (LU, 2) TMP1, TMP3, TMP5
            WRITE (LU, 2) TMP2, TMP4, TMP6
            WRITE (LU, 6) ARGX(1:4), TMP5, UOPR(IOP)(1:LOP(IOP)),       
     $       ANDOR(1:5), TMP6, UOPR(IOP)(1:LOP(IOP))
 6          FORMAT (6X,A,' = ',A,' ',A,' 0 ',A,' ',A,' ',A,' 0')
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
            CALL RLTMP (TMP4)
            CALL RLTMP (TMP5)
            CALL RLTMP (TMP6)
          ENDIF
        ELSEIF (ITP1 .EQ. 1 .OR. ITP1 .EQ. 2) THEN
          IF (ITP2 .NE. 10) THEN
C
C   (Arg1 is IN or SP) and (Arg2 is MPI or MPR).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (9)
            TMP3 = GETMP (1)
            WRITE (LU, 7) TMP1, ARG1(1:L1)
 7          FORMAT (6X,A,' = ',A)
            WRITE (LU, 1) TMP1, TMP2
            WRITE (LU, 2) TMP2, ARG2(1:L2), TMP3
            WRITE (LU, 3) ARGX(1:4), TMP3, UOPR(IOP)(1:LOP(IOP))
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
          ELSE
C
C   (Arg1 is IN or SP) and (Arg2 is MPC).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (9)
            TMP3 = GETMP (9)
            TMP4 = GETMP (9)
            TMP5 = GETMP (9)
            TMP6 = GETMP (1)
            TMP7 = GETMP (1)
            WRITE (LU, 7) TMP1, ARG1(1:L1)
            WRITE (LU, 1) TMP1, TMP2
            WRITE (LU, 4) TMP3
            WRITE (LU, 5) ARG2(1:L2), TMP4, TMP5
            WRITE (LU, 2) TMP2, TMP4, TMP6
            WRITE (LU, 2) TMP3, TMP5, TMP7
            WRITE (LU, 6) ARGX(1:4), TMP6, UOPR(IOP)(1:LOP(IOP)),       
     $       ANDOR(1:5), TMP7, UOPR(IOP)(1:LOP(IOP))
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
            CALL RLTMP (TMP4)
            CALL RLTMP (TMP5)
            CALL RLTMP (TMP6)
            CALL RLTMP (TMP7)
          ENDIF
        ELSEIF (ITP1 .EQ. 4 .OR. ITP1 .EQ. 5) THEN
          IF (ITP2 .NE. 10) THEN
C
C   (Arg1 is CO or DC) and (Arg2 is MPI or MPR).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (3)
            TMP3 = GETMP (9)
            TMP4 = GETMP (1)
            IF (ITP1 .EQ. 5) THEN
              WRITE (LU, 8) TMP1, ARG1(1:L1)
 8            FORMAT (6X,A,' = DREAL (',A,')')
              WRITE (LU, 9) TMP2, ARG1(1:L1)
 9            FORMAT (6X,A,' = DIMAG (',A,')')
            ELSE
              WRITE (LU, 10) TMP1, ARG1(1:L1)
 10           FORMAT (6X,A,' = REAL (',A,')')
              WRITE (LU, 11) TMP2, ARG1(1:L1)
 11           FORMAT (6X,A,' = AIMAG (',A,')')
            ENDIF
            WRITE (LU, 1) TMP1, TMP3
            WRITE (LU, 2) TMP3, ARG2(1:L2), TMP4
            WRITE (LU, 6) ARGX(1:4), TMP4, UOPR(IOP)(1:LOP(IOP)),       
     $       ANDOR(1:5), TMP2, UOPR(IOP)(1:LOP(IOP))
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
            CALL RLTMP (TMP4)
          ELSE
C
C   (Arg1 is CO or DC) and (Arg2 is MPC).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (3)
            TMP3 = GETMP (9)
            TMP4 = GETMP (9)
            TMP5 = GETMP (9)
            TMP6 = GETMP (9)
            TMP7 = GETMP (1)
            TMP8 = GETMP (1)
            IF (ITP1 .EQ. 5) THEN
              WRITE (LU, 8) TMP1, ARG1(1:L1)
              WRITE (LU, 9) TMP2, ARG1(1:L1)
            ELSE
              WRITE (LU, 10) TMP1, ARG1(1:L1)
              WRITE (LU, 11) TMP2, ARG1(1:L1)
            ENDIF
            WRITE (LU, 1) TMP1, TMP3
            WRITE (LU, 1) TMP2, TMP4
            WRITE (LU, 5) ARG2(1:L2), TMP5, TMP6
            WRITE (LU, 2) TMP3, TMP5, TMP7
            WRITE (LU, 2) TMP4, TMP6, TMP8
            WRITE (LU, 6) ARGX(1:4), TMP7, UOPR(IOP)(1:LOP(IOP)),       
     $       ANDOR(1:5), TMP8, UOPR(IOP)(1:LOP(IOP))
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
            CALL RLTMP (TMP4)
            CALL RLTMP (TMP5)
            CALL RLTMP (TMP6)
            CALL RLTMP (TMP7)
            CALL RLTMP (TMP8)
          ENDIF
        ENDIF
      ELSEIF (ITP2 .LT. 8) THEN
        IF (ITP2 .EQ. 3) THEN
          IF (ITP1 .NE. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is DP).
C
            TMP1 = GETMP (9)
            TMP2 = GETMP (1)
            WRITE (LU, 1) ARG2(1:L2), TMP1
            WRITE (LU, 2) ARG1(1:L1), TMP1, TMP2
            WRITE (LU, 3) ARGX(1:4), TMP2, UOPR(IOP)(1:LOP(IOP))
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
          ELSE
C
C   (Arg1 is MPC) and (Arg2 is DP).
C
            TMP1 = GETMP (9)
            TMP2 = GETMP (9)
            TMP3 = GETMP (9)
            TMP4 = GETMP (9)
            TMP5 = GETMP (1)
            TMP6 = GETMP (1)
            WRITE (LU, 1) ARG2(1:L2), TMP1
            WRITE (LU, 4) TMP2
            WRITE (LU, 5) ARG1(1:L1), TMP3, TMP4
            WRITE (LU, 2) TMP3, TMP1, TMP5
            WRITE (LU, 2) TMP4, TMP2, TMP6
            WRITE (LU, 6) ARGX(1:4), TMP5, UOPR(IOP)(1:LOP(IOP)),       
     $       ANDOR(1:5), TMP6, UOPR(IOP)(1:LOP(IOP))
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
            CALL RLTMP (TMP4)
            CALL RLTMP (TMP5)
            CALL RLTMP (TMP6)
          ENDIF
        ELSEIF (ITP2 .EQ. 1 .OR. ITP2 .EQ. 2) THEN
          IF (ITP1 .NE. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is IN or SP).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (9)
            TMP3 = GETMP (1)
            WRITE (LU, 7) TMP1, ARG2(1:L2)
            WRITE (LU, 1) TMP1, TMP2
            WRITE (LU, 2) ARG1(1:L1), TMP2, TMP3
            WRITE (LU, 3) ARGX(1:4), TMP3, UOPR(IOP)(1:LOP(IOP))
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
          ELSE
C
C   (Arg1 is MPC) and (Arg2 is IN or SP).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (9)
            TMP3 = GETMP (9)
            TMP4 = GETMP (9)
            TMP5 = GETMP (9)
            TMP6 = GETMP (1)
            TMP7 = GETMP (1)
            WRITE (LU, 7) TMP1, ARG2(1:L2)
            WRITE (LU, 1) TMP1, TMP2
            WRITE (LU, 4) TMP3
            WRITE (LU, 5) ARG1(1:L1), TMP4, TMP5
            WRITE (LU, 2) TMP4, TMP2, TMP6
            WRITE (LU, 2) TMP5, TMP3, TMP7
            WRITE (LU, 6) ARGX(1:4), TMP6, UOPR(IOP)(1:LOP(IOP)),       
     $       ANDOR(1:5), TMP7, UOPR(IOP)(1:LOP(IOP))
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
            CALL RLTMP (TMP4)
            CALL RLTMP (TMP5)
            CALL RLTMP (TMP6)
            CALL RLTMP (TMP7)
          ENDIF
        ELSEIF (ITP2 .EQ. 4 .OR. ITP2 .EQ. 5) THEN
          IF (ITP1 .NE. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is CO or DC).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (3)
            TMP3 = GETMP (9)
            TMP4 = GETMP (1)
            IF (ITP2 .EQ. 5) THEN
              WRITE (LU, 8) TMP1, ARG2(1:L2)
              WRITE (LU, 9) TMP2, ARG2(1:L2)
            ELSE
              WRITE (LU, 10) TMP1, ARG2(1:L2)
              WRITE (LU, 11) TMP2, ARG2(1:L2)
            ENDIF
            WRITE (LU, 1) TMP1, TMP3
            WRITE (LU, 2) ARG1(1:L1), TMP3, TMP4
            WRITE (LU, 6) ARGX(1:4), TMP4, UOPR(IOP)(1:LOP(IOP)),       
     $       ANDOR(1:5), TMP2, UOPR(IOP)(1:LOP(IOP))
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
            CALL RLTMP (TMP4)
          ELSE
C
C   (Arg1 is MPC) and (Arg2 is CO or DC).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (3)
            TMP3 = GETMP (9)
            TMP4 = GETMP (9)
            TMP5 = GETMP (9)
            TMP6 = GETMP (9)
            TMP7 = GETMP (1)
            TMP8 = GETMP (1)
            IF (ITP2 .EQ. 5) THEN
              WRITE (LU, 8) TMP1, ARG2(1:L2)
              WRITE (LU, 9) TMP2, ARG2(1:L2)
            ELSE
              WRITE (LU, 10) TMP1, ARG2(1:L2)
              WRITE (LU, 11) TMP2, ARG2(1:L2)
            ENDIF
            WRITE (LU, 1) TMP1, TMP3
            WRITE (LU, 1) TMP2, TMP4
            WRITE (LU, 5) ARG1(1:L1), TMP5, TMP6
            WRITE (LU, 2) TMP5, TMP3, TMP7
            WRITE (LU, 2) TMP6, TMP4, TMP8
            WRITE (LU, 6) ARGX(1:4), TMP7, UOPR(IOP)(1:LOP(IOP)),       
     $       ANDOR(1:5), TMP8, UOPR(IOP)(1:LOP(IOP))
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
            CALL RLTMP (TMP4)
            CALL RLTMP (TMP5)
            CALL RLTMP (TMP6)
            CALL RLTMP (TMP7)
            CALL RLTMP (TMP8)
          ENDIF
        ENDIF
      ELSEIF (ITP1 .NE. 10 .AND. ITP2 .NE. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is MPI or MPR).
C
        TMP1 = GETMP (1)
        WRITE (LU, 2) ARG1(1:L1), ARG2(1:L2), TMP1
        WRITE (LU, 3) ARGX(1:4), TMP1, UOPR(IOP)(1:LOP(IOP))
        CALL RLTMP (TMP1)
      ELSEIF (ITP1 .EQ. 10 .AND. ITP2 .NE. 10) THEN
C
C   (Arg1 is MPC) and (Arg2 is MPI or MPR).
C
        TMP1 = GETMP (9)
        TMP2 = GETMP (9)
        TMP3 = GETMP (9)
        TMP4 = GETMP (1)
        TMP5 = GETMP (1)
        WRITE (LU, 5) ARG1(1:L1), TMP1, TMP2
        WRITE (LU, 4) TMP3
        WRITE (LU, 2) TMP1, ARG2(1:L2), TMP4
        WRITE (LU, 2) TMP2, TMP3, TMP5
        WRITE (LU, 6) ARGX(1:4), TMP4, UOPR(IOP)(1:LOP(IOP)),           
     $   ANDOR(1:5), TMP5, UOPR(IOP)(1:LOP(IOP))
        CALL RLTMP (TMP1)
        CALL RLTMP (TMP2)
        CALL RLTMP (TMP3)
        CALL RLTMP (TMP4)
        CALL RLTMP (TMP5)
      ELSEIF (ITP1 .NE. 10 .AND. ITP2 .EQ. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is MPC).
C
        TMP1 = GETMP (9)
        TMP2 = GETMP (9)
        TMP3 = GETMP (9)
        TMP4 = GETMP (1)
        TMP5 = GETMP (1)
        WRITE (LU, 5) ARG2(1:L2), TMP1, TMP2
        WRITE (LU, 4) TMP3
        WRITE (LU, 2) ARG1(1:L1), TMP1, TMP4
        WRITE (LU, 2) TMP3, TMP2, TMP5
        WRITE (LU, 6) ARGX(1:4), TMP4, UOPR(IOP)(1:LOP(IOP)),           
     $   ANDOR(1:5), TMP5, UOPR(IOP)(1:LOP(IOP))
        CALL RLTMP (TMP1)
        CALL RLTMP (TMP2)
        CALL RLTMP (TMP3)
        CALL RLTMP (TMP4)
        CALL RLTMP (TMP5)
      ELSE
C
C   (Arg1 is MPC) AND (Arg2 is MPC).
C
        TMP1 = GETMP (9)
        TMP2 = GETMP (9)
        TMP3 = GETMP (9)
        TMP4 = GETMP (9)
        TMP5 = GETMP (1)
        TMP6 = GETMP (1)
        WRITE (LU, 5) ARG1(1:L1), TMP1, TMP2
        WRITE (LU, 5) ARG2(1:L2), TMP3, TMP4
        WRITE (LU, 2) TMP1, TMP3, TMP5
        WRITE (LU, 2) TMP2, TMP4, TMP6
        WRITE (LU, 6) ARGX(1:4), TMP5, UOPR(IOP)(1:LOP(IOP)),           
     $   ANDOR(1:5), TMP6, UOPR(IOP)(1:LOP(IOP))
        CALL RLTMP (TMP1)
        CALL RLTMP (TMP2)
        CALL RLTMP (TMP3)
        CALL RLTMP (TMP4)
        CALL RLTMP (TMP5)
        CALL RLTMP (TMP6)
      ENDIF
      GOTO 110
C
C
 100  CALL ERRMES (52, 0)
      WRITE (6, 12) UOPR(IOP)(1:LOP(IOP))
 12   FORMAT ('Operation ',A,' is not defined with these operands.')
      CALL ABRT
C
 110  RETURN
      END
C
      SUBROUTINE GENDIV (LU, ITP1, L1, ARG1, ITP2, L2, ARG2, ARGX)
C
C   This generates code for a divide operation.  The operands are in ARG1 and
C   ARG2, with types ITP1 and ITP2, and with lengths L1 and L2.  The result
C   name is in ARGX (also input).  LU is the unit number for output code.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*80 ARG1, ARG2, ARGX
      CHARACTER*4 TMP1, TMP2, TMP3, TMP4, TMP5, TMP6, TMP7, TMP8, GETMP
C
      IF (ITP1 .LT. 8) THEN
        IF (ITP1 .EQ. 3) THEN
          IF (ITP2 .NE. 10) THEN
C
C   (Arg1 is DP) and (Arg2 is MPI or MPR).
C
            TMP1 = GETMP (9)
            WRITE (LU, 1) ARG1(1:L1), TMP1
 1          FORMAT (6X,'CALL MPDMC (',A,', 0, ',A,')')
            WRITE (LU, 2) TMP1, ARG2(1:L2), ARGX(1:4)
 2          FORMAT (6X,'CALL MPDIV (',A,', ',A,', ',A,')')
            CALL RLTMP (TMP1)
          ELSE
C
C   (Arg1 is DP) and (Arg2 is MPC).
C
            TMP1 = GETMP (9)
            TMP2 = GETMP (9)
            TMP3 = GETMP (10)
            WRITE (LU, 1) ARG1(1:L1), TMP1
            WRITE (LU, 3) TMP2
 3          FORMAT (6X,'CALL MPDMC (0.D0, 0, ',A,')')
            WRITE (LU, 4) TMP1, TMP2, TMP3
 4          FORMAT (6X,'CALL MPMMPC (',A,', ',A,', MPNW4, ',A,')')
            WRITE (LU, 5) TMP3, ARG2(1:L2), ARGX(1:4)
 5          FORMAT (6X,'CALL MPCDIV (MPNW4, ',A,', ',A,', ',A,')')
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
          ENDIF
        ELSEIF (ITP1 .EQ. 1 .OR. ITP1 .EQ. 2) THEN
          IF (ITP2 .NE. 10) THEN
C
C   (Arg1 is IN or SP) and (Arg2 is MPI or MPR).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (9)
            WRITE (LU, 6) TMP1, ARG1(1:L1)
 6          FORMAT (6X,A,' = ',A)
            WRITE (LU, 1) TMP1, TMP2
            WRITE (LU, 2) TMP2, ARG2(1:L2), ARGX(1:4)
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
          ELSE
C
C   (Arg1 is IN or SP) and (Arg2 is MPC).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (9)
            TMP3 = GETMP (9)
            TMP4 = GETMP (10)
            WRITE (LU, 6) TMP1, ARG1(1:L1)
            WRITE (LU, 1) TMP1, TMP2
            WRITE (LU, 3) TMP3
            WRITE (LU, 4) TMP2, TMP3, TMP4
            WRITE (LU, 5) TMP4, ARG2(1:L2), ARGX(1:4)
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
            CALL RLTMP (TMP4)
          ENDIF
        ELSEIF (ITP1 .EQ. 4 .OR. ITP1 .EQ. 5) THEN
          IF (ITP2 .NE. 10) THEN
C
C   (Arg1 is CO or DP) and (Arg2 is MPI or MPR).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (3)
            TMP3 = GETMP (9)
            TMP4 = GETMP (9)
            IF (ITP1 .EQ. 5) THEN
              WRITE (LU, 7) TMP1, ARG1(1:L1)
 7            FORMAT (6X,A,' = DREAL (',A,')')
              WRITE (LU, 8) TMP2, ARG1(1:L1)
 8            FORMAT (6X,A,' = DIMAG (',A,')')
            ELSE
              WRITE (LU, 9) TMP1, ARG1(1:L1)
 9            FORMAT (6X,A,' = REAL (',A,')')
              WRITE (LU, 10) TMP2, ARG1(1:L1)
 10           FORMAT (6X,A,' = AIMAG (',A,')')
            ENDIF
            WRITE (LU, 1) TMP1, TMP3
            WRITE (LU, 1) TMP2, TMP4
            WRITE (LU, 2) TMP3, ARG2(1:L2), ARGX(1:4)
            WRITE (LU, 11) TMP4, ARG2(1:L2), ARGX(1:4)
 11         FORMAT (6X,'CALL MPDIV (',A,', ',A,', ',A,'(MPNWQ+5))')
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
            CALL RLTMP (TMP4)
          ELSE
C
C   (Arg1 is CO or DP) and (Arg2 is MPC).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (3)
            TMP3 = GETMP (9)
            TMP4 = GETMP (9)
            TMP5 = GETMP (10)
            IF (ITP1 .EQ. 5) THEN
              WRITE (LU, 7) TMP1, ARG1(1:L1)
              WRITE (LU, 8) TMP2, ARG1(1:L1)
            ELSE
              WRITE (LU, 9) TMP1, ARG1(1:L1)
              WRITE (LU, 10) TMP2, ARG1(1:L1)
            ENDIF
            WRITE (LU, 1) TMP1, TMP3
            WRITE (LU, 1) TMP2, TMP4
            WRITE (LU, 4) TMP3, TMP4, TMP5
            WRITE (LU, 12) TMP5, ARG2(1:L2), ARGX(1:4)
 12         FORMAT (6X,'CALL MPCDIV (MPNW4, ',A,', ',A,', ',A,')')
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
            CALL RLTMP (TMP4)
            CALL RLTMP (TMP5)
          ENDIF
        ENDIF
      ELSEIF (ITP2 .LT. 8) THEN
        IF (ITP2 .EQ. 3) THEN
          IF (ITP1 .NE. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is DP).
C
            WRITE (LU, 13) ARG1(1:L1), ARG2(1:L2), ARGX(1:4)
 13         FORMAT (6X,'CALL MPDIVD (',A,', ',A,', 0, ',A,')')
          ELSE
C
C   (Arg1 is MPC) and (Arg2 is DP).
C
            TMP1 = GETMP (9)
            TMP2 = GETMP (9)
            WRITE (LU, 14) ARG1(1:L1), TMP1, TMP2
 14         FORMAT (6X,'CALL MPMPCM (MPNW4, ',A,', ',A,', ',A,')')
            WRITE (LU, 13) TMP1, ARG2(1:L2), ARGX(1:4)
            WRITE (LU, 15) TMP2, ARG2(1:L2), ARGX(1:4)
 15         FORMAT (6X,'CALL MPDIVD (',A,', ',A,', 0, ',A,              
     $        '(MPNWQ+5))')
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
          ENDIF
        ELSEIF (ITP2 .EQ. 1 .OR. ITP2 .EQ. 2) THEN
          IF (ITP1 .NE. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is IN or SP).
C
            TMP1 = GETMP (3)
            WRITE (LU, 6) TMP1, ARG2(1:L2)
            WRITE (LU, 13) ARG1(1:L1), TMP1, ARGX(1:4)
            CALL RLTMP (TMP1)
          ELSE
C
C   (Arg1 is MPC) and (Arg2 is IN or SP).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (9)
            TMP3 = GETMP (9)
            WRITE (LU, 6) TMP1, ARG2(1:L2)
            WRITE (LU, 14) ARG1(1:L1), TMP2, TMP3
            WRITE (LU, 13) TMP2, TMP1, ARGX(1:4)
            WRITE (LU, 15) TMP3, TMP1, ARGX(1:4)
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
          ENDIF
        ELSEIF (ITP2 .EQ. 4 .OR. ITP2 .EQ. 5) THEN
          IF (ITP1 .NE. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is CO or DC).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (3)
            TMP3 = GETMP (9)
            TMP4 = GETMP (9)
            TMP5 = GETMP (10)
            TMP6 = GETMP (10)
            IF (ITP2 .EQ. 5) THEN
              WRITE (LU, 7) TMP1, ARG2(1:L2)
              WRITE (LU, 8) TMP2, ARG2(1:L2)
            ELSE
              WRITE (LU, 9) TMP1, ARG2(1:L2)
              WRITE (LU, 10) TMP2, ARG2(1:L2)
            ENDIF
            WRITE (LU, 1) TMP1, TMP3
            WRITE (LU, 1) TMP2, TMP4
            WRITE (LU, 4) TMP3, TMP4, TMP5
            WRITE (LU, 3) TMP3
            WRITE (LU, 4) ARG1(1:L1), TMP3, TMP6
            WRITE (LU, 12) TMP6, TMP5, ARGX(1:4)
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
            CALL RLTMP (TMP4)
            CALL RLTMP (TMP5)
            CALL RLTMP (TMP6)
          ELSE
C
C   (Arg1 is MPC) and (Arg2 is CO or DC).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (3)
            TMP3 = GETMP (9)
            TMP4 = GETMP (9)
            TMP5 = GETMP (10)
            IF (ITP2 .EQ. 5) THEN
              WRITE (LU, 7) TMP1, ARG2(1:L2)
              WRITE (LU, 8) TMP2, ARG2(1:L2)
            ELSE
              WRITE (LU, 9) TMP1, ARG2(1:L2)
              WRITE (LU, 10) TMP2, ARG2(1:L2)
            ENDIF
            WRITE (LU, 1) TMP1, TMP3
            WRITE (LU, 1) TMP2, TMP4
            WRITE (LU, 4) TMP3, TMP4, TMP5
            WRITE (LU, 12) ARG1(1:L1), TMP5, ARGX(1:4)
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
            CALL RLTMP (TMP4)
            CALL RLTMP (TMP5)
          ENDIF
        ENDIF
      ELSEIF (ITP1 .NE. 10 .AND. ITP2 .NE. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is MPI or MPR).
C
        WRITE (LU, 2) ARG1(1:L1), ARG2(1:L2), ARGX(1:4)
      ELSEIF (ITP1 .EQ. 10 .AND. ITP2 .NE. 10) THEN
C
C   (Arg1 is MPC) and (Arg2 is MPI or MPR).
C
        TMP1 = GETMP (9)
        TMP2 = GETMP (9)
        WRITE (LU, 14) ARG1(1:L1), TMP1, TMP2
        WRITE (LU, 2) TMP1, ARG2(1:L2), ARGX(1:4)
        WRITE (LU, 11) TMP2, ARG2(1:L2), ARGX(1:4)
        CALL RLTMP (TMP1)
        CALL RLTMP (TMP2)
      ELSEIF (ITP1 .NE. 10 .AND. ITP2 .EQ. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is MPC).
C
        TMP1 = GETMP (9)
        TMP2 = GETMP (10)
        WRITE (LU, 3) TMP1
        WRITE (LU, 4) ARG1(1:L1), TMP1, TMP2
        WRITE (LU, 12) TMP2, ARG2(1:L2), ARGX(1:4)
        CALL RLTMP (TMP1)
        CALL RLTMP (TMP2)
      ELSEIF (ITP1 .EQ. 10 .AND. ITP2 .EQ. 10) THEN
C
C   (Arg1 is MPC) and (Arg2 is MPC).
C
        WRITE (LU, 12) ARG1(1:L1), ARG2(1:L2), ARGX(1:4)
      ENDIF
C
      RETURN
      END
C
      SUBROUTINE GENEXP (LU, ITP1, L1, ARG1, ITP2, L2, ARG2, ARGX)
C
C   This generates code for an exponentiation.  The operands are in ARG1 and
C   ARG2, with types ITP1 and ITP2, and with lengths L1 and L2.  The result
C   name is in ARGX (also input).  LU is the unit number for output code.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*80 ARG1, ARG2, ARGX
      CHARACTER*4 TMP1, TMP2, TMP3, TMP4, TMP5, TMP6, TMP7, TMP8, GETMP
C
      IOP = 4
      IF (ITP1 .LT. 8) THEN
        IF (ITP1 .EQ. 3 .AND. ITP2 .EQ. 8) THEN
C
C   (Arg1 is DP) and (Arg2 is MPI).
C
          KCON(3) = 1
          TMP1 = GETMP (9)
          TMP2 = GETMP (8)
          TMP3 = GETMP (3)
          TMP4 = GETMP (1)
          TMP5 = GETMP (9)
          TMP6 = GETMP (9)
          WRITE (LU, 1) ARG1(1:L1), TMP1
 1        FORMAT (6X,'CALL MPDMC (',A,', 0, ',A,')')
          WRITE (LU, 2) ARG2(1:L2), TMP2
 2        FORMAT (6X,'CALL MPEQ (',A,', ',A,')')
          WRITE (LU, 3) TMP2, TMP2
 3        FORMAT (6X,'IF (',A,'(2) .EQ. 0. .OR. ',A,'(2) .EQ. 1) THEN')
          WRITE (LU, 4) TMP2, TMP3, TMP4
 4        FORMAT (6X,'CALL MPMDC (',A,', ',A,', ',A,')')
          WRITE (LU, 5) TMP4, TMP3, TMP4
 5        FORMAT (6X,A,' = ',A,' * 2.D0 ** ',A,' + 0.25D0')
          WRITE (LU, 6) TMP1, TMP4, ARGX(1:4)
 6        FORMAT (6X,'CALL MPNPWR (',A,', ',A,', ',A,')')
          WRITE (LU, 7)
 7        FORMAT (6X,'ELSE')
          WRITE (LU, 8) TMP1, TMP5
 8        FORMAT (6X,'CALL MPLOG (',A,', MPL02, ',A,')')
          WRITE (LU, 9) ARG2(1:L2), TMP5, TMP6
 9        FORMAT (6X,'CALL MPMUL (',A,', ',A,', ',A,')')
          WRITE (LU, 10) TMP6, ARGX(1:4)
 10       FORMAT (6X,'CALL MPEXP (',A,', MPL02, ',A,')')
          WRITE (LU, 11)
 11       FORMAT (6X,'ENDIF')
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
          CALL RLTMP (TMP3)
          CALL RLTMP (TMP4)
          CALL RLTMP (TMP5)
          CALL RLTMP (TMP6)
        ELSEIF (ITP1 .EQ. 3 .AND. ITP2 .EQ. 9) THEN
C
C   (Arg1 is DP) and (Arg2 is MPR).
C
          KCON(3) = 1
          TMP1 = GETMP (9)
          TMP2 = GETMP (9)
          TMP3 = GETMP (9)
          WRITE (LU, 1) ARG1(1:L1), TMP1
          WRITE (LU, 8) TMP1, TMP2
          WRITE (LU, 9) ARG2(1:L2), TMP2, TMP3
          WRITE (LU, 10) TMP3, ARGX(1:4)
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
          CALL RLTMP (TMP3)
        ELSEIF ((ITP1 .EQ. 1 .OR. ITP1 .EQ. 2) .AND. ITP2 .EQ. 8) THEN
C
C   (Arg1 is IN or SP) and (Arg2 is MPI).
C
          KCON(3) = 1
          TMP1 = GETMP (9)
          TMP2 = GETMP (8)
          TMP3 = GETMP (3)
          TMP4 = GETMP (1)
          TMP5 = GETMP (9)
          TMP6 = GETMP (9)
          WRITE (LU, 12) TMP3, ARG1(1:L1)
 12       FORMAT (6X,A,' = ',A)
          WRITE (LU, 1) TMP3, TMP1
          WRITE (LU, 2) ARG2(1:L2), TMP2
          WRITE (LU, 3) TMP2, TMP2
          WRITE (LU, 4) TMP2, TMP3, TMP4
          WRITE (LU, 5) TMP4, TMP3, TMP4
          WRITE (LU, 6) TMP1, TMP4, ARGX(1:4)
          WRITE (LU, 7)
          WRITE (LU, 8) TMP1, TMP5
          WRITE (LU, 9) ARG2(1:L2), TMP5, TMP6
          WRITE (LU, 10) TMP6, ARGX(1:4)
          WRITE (LU, 11)
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
          CALL RLTMP (TMP3)
          CALL RLTMP (TMP4)
          CALL RLTMP (TMP5)
          CALL RLTMP (TMP6)
        ELSEIF ((ITP1 .EQ. 1 .OR. ITP1 .EQ. 2) .AND. ITP2 .EQ. 9) THEN
C
C   (Arg1 is IN or SP) and (Arg2 is MPI or MPR).
C
          KCON(3) = 1
          TMP1 = GETMP (3)
          TMP2 = GETMP (9)
          TMP3 = GETMP (9)
          TMP4 = GETMP (9)
          WRITE (LU, 13) TMP1, ARG1(1:L1)
 13       FORMAT (6X,A,' = ',A)
          WRITE (LU, 1) TMP1, TMP2
          WRITE (LU, 8) TMP2, TMP3
          WRITE (LU, 9) ARG2(1:L2), TMP3, TMP4
          WRITE (LU, 10) TMP4, ARGX(1:4)
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
          CALL RLTMP (TMP3)
          CALL RLTMP (TMP4)
        ELSE
          GOTO 100
        ENDIF
      ELSEIF (ITP2 .LT. 8) THEN
        IF (ITP1 .NE. 10 .AND. ITP2 .EQ. 1) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is IN).
C
          WRITE (LU, 14) ARG1(1:L1), ARG2(1:L2), ARGX(1:4)
 14       FORMAT (6X,'CALL MPNPWR (',A,', ',A,', ',A,')')
        ELSEIF (ITP1 .EQ. 10 .AND. ITP2 .EQ. 1) THEN
C
C   (Arg1 is MPC) and (Arg2 is IN).  This is the only permissible
C   exponentiation with a MPC operand.
C
          WRITE (LU, 15) ARG1(1:L1), ARG2(1:L2), ARGX(1:4)
 15       FORMAT (6X, 'CALL MPCPWR (MPNW4, ',A,', ',A,', ',A,')')
        ELSEIF (ITP1 .NE. 10 .AND. (ITP2 .EQ. 2 .OR. ITP2 .EQ. 3))      
     $      THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is SP or DP).
C
          KCON(3) = 1
          TMP1 = GETMP (3)
          TMP2 = GETMP (9)
          TMP3 = GETMP (9)
          TMP4 = GETMP (9)
          WRITE (LU, 13) TMP1(1:4), ARG2(1:L2)
          WRITE (LU, 1) TMP1(1:4), TMP2
          WRITE (LU, 8) ARG1(1:L1), TMP3
          WRITE (LU, 9) TMP2, TMP3, TMP4
          WRITE (LU, 10) TMP4, ARGX(1:4)
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
          CALL RLTMP (TMP3)
          CALL RLTMP (TMP4)
        ELSE
          GOTO 100
        ENDIF
      ELSEIF (ITP1 .NE. 10 .AND. ITP2 .EQ. 8) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is MPI).
C
        KCON(3) = 1
        TMP1 = GETMP (8)
        TMP2 = GETMP (3)
        TMP3 = GETMP (1)
        TMP4 = GETMP (9)
        TMP5 = GETMP (9)
        WRITE (LU, 2) ARG2(1:L2), TMP1
        WRITE (LU, 3) TMP1, TMP1
        WRITE (LU, 4) TMP1, TMP2, TMP3
        WRITE (LU, 5) TMP3, TMP2, TMP3
        WRITE (LU, 6) ARG1(1:L1), TMP3, ARGX(1:4)
        WRITE (LU, 7)
        WRITE (LU, 8) ARG1(1:L1), TMP4
        WRITE (LU, 9) ARG2(1:L2), TMP4, TMP5
        WRITE (LU, 10) TMP5, ARGX(1:4)
        WRITE (LU, 11)
        CALL RLTMP (TMP1)
        CALL RLTMP (TMP2)
        CALL RLTMP (TMP3)
        CALL RLTMP (TMP4)
        CALL RLTMP (TMP5)
      ELSEIF (ITP1 .NE. 10 .AND. ITP2 .EQ. 9) THEN
C
C  (Arg1 is MPI or MPR) and (Arg2 is MPI or MPR).
C
        KCON(3) = 1
        TMP1 = GETMP (9)
        TMP2 = GETMP (9)
        WRITE (LU, 8) ARG1(1:L1), TMP1
        WRITE (LU, 9) ARG2(1:L2), TMP1, TMP2
        WRITE (LU, 10) TMP2, ARGX(1:4)
        CALL RLTMP (TMP1)
        CALL RLTMP (TMP2)
      ENDIF
      GOTO 110
C
 100  CALL ERRMES (53, 0)
      WRITE (6, 16) UOPR(IOP)(1:LOP(IOP))
 16   FORMAT ('Operation ',A,' is not defined with these operands.')
      CALL ABRT
C
 110  RETURN
      END
C
      SUBROUTINE GENMUL (LU, ITP1, L1, ARG1, ITP2, L2, ARG2, ARGX)
C
C   This generates code for a multiply operation.  The operands are in ARG1 and
C   ARG2, with types ITP1 and ITP2, and with lengths L1 and L2.  The result
C   name is in ARGX (also input).  LU is the unit number for output code.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*80 ARG1, ARG2, ARGX
      CHARACTER*4 TMP1, TMP2, TMP3, TMP4, TMP5, TMP6, TMP7, TMP8, GETMP
C
      IF (ITP1 .LT. 8) THEN
        IF (ITP1 .EQ. 3) THEN
          IF (ITP2 .NE. 10) THEN
C
C   (Arg1 is DP) and (Arg2 is MPI or MPR).
C
            WRITE (LU, 1) ARG2(1:L2), ARG1(1:L1), ARGX(1:4)
 1          FORMAT (6X,'CALL MPMULD (',A,', ',A,', 0, ',A,')')
          ELSE
C
C   (Arg1 is DP) and (Arg2 is MPC).
C
            TMP1 = GETMP (9)
            TMP2 = GETMP (9)
            WRITE (LU, 2) ARG2(1:L2), TMP1, TMP2
 2          FORMAT (6X,'CALL MPMPCM (MPNW4, ',A,', ',A,', ',A,')')
            WRITE (LU, 1) TMP1, ARG1(1:L1), ARGX(1:4)
            WRITE (LU, 3) TMP2, ARG1(1:L1), ARGX(1:4)
 3          FORMAT (6X,'CALL MPMULD (',A,', ',A,', 0, ',A,              
     $        '(MPNWQ+5))')
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
          ENDIF
        ELSEIF (ITP1 .EQ. 1 .OR. ITP1 .EQ. 2) THEN
          IF (ITP2 .NE. 10) THEN
C
C   (Arg1 is IN or SP) and (Arg2 is MPI or MPR).
C
            TMP1 = GETMP (3)
            WRITE (LU, 4) TMP1, ARG1(1:L1)
 4          FORMAT (6X,A,' = ',A)
            WRITE (LU, 1) ARG2(1:L2), TMP1, ARGX(1:4)
            CALL RLTMP (TMP1)
          ELSE
C
C   (Arg1 is IN or SP) and (Arg2 is MPC).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (9)
            TMP3 = GETMP (9)
            WRITE (LU, 4) TMP1, ARG1(1:L1)
            WRITE (LU, 2) ARG2(1:L2), TMP2, TMP3
            WRITE (LU, 1) TMP2, TMP1, ARGX(1:4)
            WRITE (LU, 3) TMP3, TMP1, ARGX(1:4)
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
          ENDIF
        ELSEIF (ITP1 .EQ. 4 .OR. ITP1 .EQ. 5) THEN
          IF (ITP2 .NE. 10) THEN
C
C   (Arg1 is CO or DP) and (Arg2 is MPI or MPR).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (3)
            IF (ITP1 .EQ. 5) THEN
              WRITE (LU, 5) TMP1, ARG1(1:L1)
 5            FORMAT (6X,A,' = DREAL (',A,')')
              WRITE (LU, 6) TMP2, ARG1(1:L1)
 6            FORMAT (6X,A,' = DIMAG (',A,')')
            ELSE
              WRITE (LU, 7) TMP1, ARG1(1:L1)
 7            FORMAT (6X,A,' = REAL (',A,')')
              WRITE (LU, 8) TMP2, ARG1(1:L1)
 8            FORMAT (6X,A,' = AIMAG (',A,')')
            ENDIF
            WRITE (LU, 1) ARG2(1:L2), TMP1, ARGX(1:4)
            WRITE (LU, 3) ARG2(1:L2), TMP2, ARGX(1:4)
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
          ELSE
C
C   (Arg1 is CO or DP) and (Arg2 is MPC).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (3)
            TMP3 = GETMP (9)
            TMP4 = GETMP (9)
            TMP5 = GETMP (10)
            IF (ITP1 .EQ. 5) THEN
              WRITE (LU, 5) TMP1, ARG1(1:L1)
              WRITE (LU, 6) TMP2, ARG1(1:L1)
            ELSE
              WRITE (LU, 7) TMP1, ARG1(1:L1)
              WRITE (LU, 8) TMP2, ARG1(1:L1)
            ENDIF
            WRITE (LU, 9) TMP1, TMP3
 9          FORMAT (6X,'CALL MPDMC (',A,', 0, ',A,')')
            WRITE (LU, 9) TMP2, TMP4
            WRITE (LU, 10) TMP3, TMP4, TMP5
 10         FORMAT (6X,'CALL MPMMPC (',A,', ',A,', MPNW4, ',A,')')
            WRITE (LU, 11) TMP5, ARG2(1:L2), ARGX(1:4)
 11         FORMAT (6X,'CALL MPCMUL (MPNW4, ',A,', ',A,', ',A,')')
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
            CALL RLTMP (TMP4)
            CALL RLTMP (TMP5)
          ENDIF
        ENDIF
      ELSEIF (ITP2 .LT. 8) THEN
        IF (ITP2 .EQ. 3) THEN
          IF (ITP1 .NE. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is DP).
C
            WRITE (LU, 1) ARG1(1:L1), ARG2(1:L2), ARGX(1:4)
          ELSE
C
C   (Arg1 is MPC) and (Arg2 is DP).
C
            TMP1 = GETMP (9)
            TMP2 = GETMP (9)
            WRITE (LU, 2) ARG1(1:L1), TMP1, TMP2
            WRITE (LU, 1) TMP1, ARG2(1:L2), ARGX(1:4)
            WRITE (LU, 3) TMP2, ARG2(1:L2), ARGX(1:4)
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
          ENDIF
        ELSEIF (ITP2 .EQ. 1 .OR. ITP2 .EQ. 2) THEN
          IF (ITP1 .NE. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is IN or SP).
C
            TMP1 = GETMP (3)
            WRITE (LU, 4) TMP1, ARG2(1:L2)
            WRITE (LU, 1) ARG1(1:L1), TMP1, ARGX(1:4)
            CALL RLTMP (TMP1)
          ELSE
C
C   (Arg1 is MPC) and (Arg2 is IN or SP).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (9)
            TMP3 = GETMP (9)
            WRITE (LU, 4) TMP1, ARG2(1:L2)
            WRITE (LU, 2) ARG1(1:L1), TMP2, TMP3
            WRITE (LU, 1) TMP2, TMP1, ARGX(1:4)
            WRITE (LU, 3) TMP3, TMP1, ARGX(1:4)
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
          ENDIF
        ELSEIF (ITP2 .EQ. 4 .OR. ITP2 .EQ. 5) THEN
          IF (ITP1 .NE. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is CO or DC).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (3)
            IF (ITP2 .EQ. 5) THEN
              WRITE (LU, 5) TMP1, ARG2(1:L2)
              WRITE (LU, 6) TMP2, ARG2(1:L2)
            ELSE
              WRITE (LU, 7) TMP1, ARG2(1:L2)
              WRITE (LU, 8) TMP2, ARG2(1:L2)
            ENDIF
            WRITE (LU, 1) ARG1(1:L1), TMP1, ARGX(1:4)
            WRITE (LU, 3) ARG1(1:L1), TMP2, ARGX(1:4)
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
          ELSE
C
C   (Arg1 is MPC) and (Arg2 is CO or DC).
C
            TMP1 = GETMP (3)
            TMP2 = GETMP (3)
            TMP3 = GETMP (9)
            TMP4 = GETMP (9)
            TMP5 = GETMP (10)
            IF (ITP2 .EQ. 5) THEN
              WRITE (LU, 5) TMP1, ARG2(1:L2)
              WRITE (LU, 6) TMP2, ARG2(1:L2)
            ELSE
              WRITE (LU, 7) TMP1, ARG2(1:L2)
              WRITE (LU, 8) TMP2, ARG2(1:L2)
            ENDIF
            WRITE (LU, 9) TMP1, TMP3
            WRITE (LU, 9) TMP2, TMP4
            WRITE (LU, 10) TMP3, TMP4, TMP5
            WRITE (LU, 11) ARG1(1:L1), TMP5, ARGX(1:4)
            CALL RLTMP (TMP1)
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
            CALL RLTMP (TMP4)
            CALL RLTMP (TMP5)
          ENDIF
        ENDIF
      ELSEIF (ITP1 .NE. 10 .AND. ITP2 .NE. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is MPI or MPR).
C
        WRITE (LU, 12) ARG1(1:L1), ARG2(1:L2), ARGX(1:4)
 12     FORMAT (6X,'CALL MPMUL (',A,', ',A,', ',A,')')
      ELSEIF (ITP1 .EQ. 10 .AND. ITP2 .NE. 10) THEN
C
C   (Arg1 is MPC) and (Arg2 is MPI or MPR).
C
        TMP1 = GETMP (9)
        TMP2 = GETMP (9)
        WRITE (LU, 2) ARG1(1:L1), TMP1, TMP2
        WRITE (LU, 12) TMP1, ARG2(1:L2), ARGX(1:4)
        WRITE (LU, 13) TMP2, ARG2(1:L2), ARGX(1:4)
 13     FORMAT (6X,'CALL MPMUL (',A,', ',A,', ',A,'(MPNWQ+5))')
        CALL RLTMP (TMP1)
        CALL RLTMP (TMP2)
      ELSEIF (ITP1 .NE. 10 .AND. ITP2 .EQ. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is MPC).
C
        TMP1 = GETMP (9)
        TMP2 = GETMP (9)
        WRITE (LU, 2) ARG2(1:L2), TMP1, TMP2
        WRITE (LU, 12) ARG1(1:L1), TMP1, ARGX(1:4)
        WRITE (LU, 13) ARG1(1:L1), TMP2, ARGX(1:4)
        CALL RLTMP (TMP1)
        CALL RLTMP (TMP2)
      ELSEIF (ITP1 .EQ. 10 .AND. ITP2 .EQ. 10) THEN
C
C   (Arg1 is MPC) and (Arg2 is MPC).
C
        WRITE (LU, 11) ARG1(1:L1), ARG2(1:L2), ARGX(1:4)
      ENDIF
C
      RETURN
      END
C
      SUBROUTINE GENSUB (LU, ITP1, L1, ARG1, ITP2, L2, ARG2, ARGX)
C
C   This generates code for a subtract operation.  The operands are in ARG1 and
C   ARG2, with types ITP1 and ITP2, and with lengths L1 and L2.  The result
C   name is in ARGX (also input).  LU is the unit number for output code.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*80 ARG1, ARG2, ARGX
      CHARACTER*4 TMP1, TMP2, TMP3, TMP4, TMP5, TMP6, TMP7, TMP8, GETMP
C
      IF (ITP1 .EQ. 0) THEN
C
C   Handle the negation of a MP entity.
C
        IF (ITP2 .NE. 10) THEN
C
C   Arg2 is MPI or MPR.
C
          WRITE (LU, 1) ARG2(1:L2), ARGX(1:4)
 1        FORMAT (6X,'CALL MPEQ (',A,', ',A,')')
          WRITE (LU, 2) ARGX(1:4), ARGX(1:4)
 2        FORMAT (6X,A,'(1) = - ',A,'(1)')
        ELSE
C
C   Arg2 is MPC.
C
          WRITE (LU, 3) ARG2(1:L2), ARGX(1:4)
 3        FORMAT (6X,'CALL MPCEQ (MPNW4, ',A,', ',A,')')
          WRITE (LU, 2) ARGX(1:4), ARGX(1:4)
          WRITE (LU, 4) ARGX(1:4), ARGX(1:4)
 4        FORMAT (6X,A,'(MPNWQ+5) = - ',A,'(MPNWQ+5)')
        ENDIF
      ELSEIF (ITP1 .LT. 8) THEN
        IF (ITP1 .EQ. 3) THEN
C
C   (Arg1 is DP) and (Arg2 is MP).
C
          TMP1 = GETMP (9)
          IF (ITP2 .EQ. 10) THEN
            WRITE (LU, 5) ARG2(1:L2), ARGX(1:4)
 5          FORMAT (6X,'CALL MPCEQ (MPNW4, ',A,', ',A,')')
          ENDIF
          WRITE (LU, 6) ARG1(1:L1), TMP1
 6        FORMAT (6X,'CALL MPDMC (',A,', 0, ',A,')')
          WRITE (LU, 7) TMP1, ARG2(1:L2), ARGX(1:4)
 7        FORMAT (6X,'CALL MPSUB (',A,', ',A,', ',A,')')
          CALL RLTMP (TMP1)
        ELSEIF (ITP1 .EQ. 1 .OR. ITP1 .EQ. 2) THEN
C
C   (Arg1 is IN or SP) and (Arg2 is MP).
C
          TMP1 = GETMP (3)
          TMP2 = GETMP (9)
          IF (ITP2 .EQ. 10) THEN
            WRITE (LU, 5) ARG2(1:L2), ARGX(1:4)
          ENDIF
          WRITE (LU, 8) TMP1, ARG1(1:L1)
 8        FORMAT (6X,A,' = ',A)
          WRITE (LU, 6) TMP1, TMP2
          WRITE (LU, 9) TMP2, ARG2(1:L2), ARGX(1:4)
 9        FORMAT (6X,'CALL MPSUB (',A,', ',A,', ',A,')')
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
        ELSE
C
C   (Arg1 is CO or DC) and (Arg2 is MP).
C
          TMP1 = GETMP (3)
          TMP2 = GETMP (3)
          TMP3 = GETMP (9)
          TMP4 = GETMP (9)
          TMP5 = GETMP (10)
          IF (ITP1 .EQ. 5) THEN
            WRITE (LU, 10) TMP1, ARG1(1:L1)
 10         FORMAT (6X,A,' = DREAL (',A,')')
            WRITE (LU, 11) TMP2, ARG1(1:L1)
 11         FORMAT (6X,A,' = DIMAG (',A,')')
          ELSE
            WRITE (LU, 12) TMP1, ARG1(1:L1)
 12         FORMAT (6X,A,' = REAL (',A,')')
            WRITE (LU, 13) TMP2, ARG1(1:L1)
 13         FORMAT (6X,A,' = AIMAG (',A,')')
          ENDIF
          WRITE (LU, 6) TMP1, TMP3
          WRITE (LU, 6) TMP2, TMP4
          WRITE (LU, 14) TMP3, TMP4, TMP5
 14       FORMAT (6X,'CALL MPMMPC (',A,', ',A,', MPNW4, ',A,')')
          IF (ITP2 .EQ. 10) THEN
            WRITE (LU, 15) TMP5, ARG2(1:L2), ARGX(1:4)
 15         FORMAT (6X,'CALL MPCSUB (MPNW4, ',A,', ',A,', ',A,')')
          ELSE
            TMP6 = GETMP (10)
            WRITE (LU, 16) TMP3
 16         FORMAT (6X,'CALL MPDMC (0.D0, 0, ',A,')')
            WRITE (LU, 14) ARG2(1:L2), TMP3, TMP6
            WRITE (LU, 15) TMP5, TMP6, ARGX(1:4)
            CALL RLTMP (TMP6)
          ENDIF
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
          CALL RLTMP (TMP3)
          CALL RLTMP (TMP4)
          CALL RLTMP (TMP5)
        ENDIF
      ELSEIF (ITP2 .LT. 8) THEN
        IF (ITP2 .EQ. 3) THEN
C
C   (Arg1 is MP) and (Arg2 is DP).
C
          TMP1 = GETMP (9)
          IF (ITP1 .EQ. 10) THEN
            WRITE (LU, 5) ARG1(1:L1), ARGX(1:4)
          ENDIF
          WRITE (LU, 6) ARG2(1:L2), TMP1
          WRITE (LU, 7) ARG1(1:L1), TMP1, ARGX(1:4)
          CALL RLTMP (TMP1)
        ELSEIF (ITP2 .EQ. 1 .OR. ITP2 .EQ. 2) THEN
C
C   (Arg1 is MP) and (Arg2 is IN or SP).
C
          TMP1 = GETMP (3)
          TMP2 = GETMP (9)
          IF (ITP1 .EQ. 10) THEN
            WRITE (LU, 5) ARG1(1:L1), ARGX(1:4)
          ENDIF
          WRITE (LU, 8) TMP1, ARG2(1:L2)
          WRITE (LU, 6) TMP1, TMP2
          WRITE (LU, 7) ARG1(1:L1), TMP2, ARGX(1:4)
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
        ELSE
C
C   (Arg1 is MP) and (Arg2 is CO or DC).
C
          TMP1 = GETMP (3)
          TMP2 = GETMP (3)
          TMP3 = GETMP (9)
          TMP4 = GETMP (9)
          TMP5 = GETMP (10)
          IF (ITP2 .EQ. 5) THEN
            WRITE (LU, 10) TMP1, ARG2(1:L2)
            WRITE (LU, 11) TMP2, ARG2(1:L2)
          ELSE
            WRITE (LU, 12) TMP1, ARG2(1:L2)
            WRITE (LU, 13) TMP2, ARG2(1:L2)
          ENDIF
          WRITE (LU, 6) TMP1, TMP3
          WRITE (LU, 6) TMP2, TMP4
          WRITE (LU, 14) TMP3, TMP4, TMP5
          IF (ITP1 .EQ. 10) THEN
            WRITE (LU, 15) ARG1(1:L1), TMP5, ARGX(1:4)
          ELSE
            TMP6 = GETMP (10)
            WRITE (LU, 16) TMP3
            WRITE (LU, 14) ARG1(1:L1), TMP3, TMP6
            WRITE (LU, 15) TMP6, TMP5, ARGX(1:4)
            CALL RLTMP (TMP6)
          ENDIF
          CALL RLTMP (TMP1)
          CALL RLTMP (TMP2)
          CALL RLTMP (TMP3)
          CALL RLTMP (TMP4)
          CALL RLTMP (TMP5)
        ENDIF
      ELSEIF (ITP1 .NE. 10 .AND. ITP2 .NE. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is MPI or MPR).
C
        WRITE (LU, 9) ARG1(1:L1), ARG2(1:L2), ARGX(1:4)
      ELSEIF (ITP1 .NE. 10 .AND. ITP2 .EQ. 10) THEN
C
C   (Arg1 is MPI or MPR) and (Arg2 is MPC).
C
        TMP1 = GETMP (3)
        TMP2 = GETMP (10)
        WRITE (LU, 16) TMP1
        WRITE (LU, 14) ARG1(1:L1), TMP1, TMP2
        WRITE (LU, 15) TMP2, ARG2(1:L2), ARGX(1:4)
        CALL RLTMP (TMP1)
        CALL RLTMP (TMP2)
      ELSEIF (ITP1 .EQ. 10 .AND. ITP2 .NE. 10) THEN
C
C   (Arg1 is MPC) and (Arg1 is MPI or MPR).
C
        TMP1 = GETMP (3)
        TMP2 = GETMP (10)
        WRITE (LU, 16) TMP1
        WRITE (LU, 14) ARG2(1:L2), TMP1, TMP2
        WRITE (LU, 15) ARG1(1:L1), TMP2, ARGX(1:4)
        CALL RLTMP (TMP1)
        CALL RLTMP (TMP2)
      ELSE
C
C   (Arg1 is MPC) and (Arg2 is MPC).
C
        WRITE (LU, 15) ARG1(1:L1), ARG2(1:L2), ARGX(1:4)
      ENDIF
C
      RETURN
      END
C
      SUBROUTINE GENFUN (LU, ITP1, L1, ARG1, NAR, ITAR, LAR, ARG, ITPX, 
     $  LX, ARGX)
C
C   This generates code for a function reference.  The function name is ARG1,
C   with type ITP1 and length L1.  NAR is the number of arguments.  The
C   argument list is input in ARG, with types in ITAR and lengths in LAR.
C   The result is placed in ARGX, with type in ITPX and length in LX.  LU is
C   the unit number for output code.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINY
      CHARACTER*80 ARG1, ARG(MAR), ARGI, ARGJ, ARGX
      CHARACTER*16 NAM, NAMQ, UCASE
      DIMENSION ITAR(MAR), LAR(MAR)
      CHARACTER*4 GETMP, TMP1
C*
C      write (lu, *) 'enter genfun nar, arg1 =', nar,
C     $  '  %'//arg1(1:l1)//'%'
C      write (lu, '(2i4,2x,a)') (i, itar(i), arg(i)(1:lar(i)),
C     $  i = 1, nar)
C*
      NAMQ = 'DPNRT'
      IQNRT = ITAB (0, 0, NAMQ)
      NAMQ = 'MPINT'
      IQINT = ITAB (0, 0, NAMQ)
      NAMQ = 'ABS'
      IQABS = ITAB (0, 0, NAMQ)
      NAMQ = 'NINT'
      IQNINT = ITAB (0, 0, NAMQ)
C
C   Check if this function is one of the obsolescent type-specific intrinsics.
C
      NAMQ = UCASE (ARG1(1:L1))
C
      DO 100 I = 1, NSF
        IF (SFUN(I) .EQ. NAMQ) THEN
          CALL ERRMES (54, 0)
          WRITE (6, 1) ARG1(1:L1)
 1        FORMAT ('This type-specific Fortran intrinsic function is',   
     $      ' not allowed: ',A/'Replace with the equivalent Fortran',   
     $      ' generic function.')
          CALL ABRT
        ENDIF
 100  CONTINUE
C
C   Find the function name in the symbol table.
C
      LL = MIN (L1, 16)
      NAM = ARG1(1:LL)
      IX = ITAB (0, 0, NAM)
      IF (IX .EQ. 0) CALL ERRMES (55, 1)
      IF (IX .GT. MINT) CALL CHKARG (LU, NAM, NAR, ITAR, LAR, ARG)
C
C   Check if this is a reference to an intrisic function.
C
      IF (IX .LE. MINT) THEN
C
C   Check if the mixed mode safe option is in effect.
C
        IF (IMM .GE. 1) THEN
C
C   Convert any non-MP arguments to MP.
C
          DO 110 I = 1, NAR
            ITPI = ITAR(I)
            LI = LAR(I)
            ARGI(1:LI) = ARG(I)(1:LI)
            IF (IX .EQ. IQNRT .AND. I .EQ. 2) GOTO 110
            IF (ITPI .EQ. 1) THEN
              ITPJ = 8
            ELSEIF (ITPI .EQ. 2 .OR. ITPI .EQ. 3) THEN
              ITPJ = 9
            ELSEIF (ITPI .EQ. 4 .OR. ITPI .EQ. 5) THEN
              ITPJ = 10
            ELSE
              GOTO 110
            ENDIF
            LJ = 4
            ARGJ(1:4) = GETMP (ITPJ)
            CALL GENASN (LU, ITPJ, LJ, ARGJ, ITPI, LI, ARGI)
            IF (LI .EQ. 4) THEN
              IF (ARGI(1:2) .EQ. 'MP') THEN
                TMP1 = ARGI(1:4)
                CALL RLTMP (TMP1)
              ENDIF
            ENDIF
C
C   Substitute the new argument (a MP temporary) for the old.
C
            ITAR(I) = ITPJ
            LAR(I) = 4
            ARG(I)(1:4) = ARGJ(1:4)
 110      CONTINUE
C
        ENDIF
        ITM = 0
C
C   Determine if any arguments are of a MP type.
C
        DO 120 I = 1, NAR
          IF (ITAR(I) .GE. 8) ITM = 1
 120    CONTINUE
C
C   Call INTRIN for intrinsic calls with MP arguments.  Other intrinsic
C   references will be handled in this routine.
C
        IF (ITM .NE. 0 .OR. IX .GE. IQINT) THEN
          CALL INTRIN (LU, ITP1, L1, ARG1, NAR, ITAR, LAR, ARG, ITPX,   
     $      LX, ARGX)
          GOTO 150
        ELSEIF (ITP1 .EQ. -1) THEN
C
C   Except for ABS with a complex or double complex argument, and NINT, the
C   result type of a Fortran-77 intrinsic functions with an argument-dependent
C   type is the type of the first argument.
C
          IF (IX .EQ. IQABS) THEN
            IF (ITAR(1) .EQ. 4) THEN
              ITPX = 2
            ELSEIF (ITAR(1) .EQ. 5) THEN
              ITPX = 3
            ELSE
              ITPX = ITAR(1)
            ENDIF
          ELSEIF (IX .EQ. IQNINT) THEN
            ITPX = 1
          ELSE
            ITPX = ITAR(1)
          ENDIF
        ELSE
          ITPX = ITP1
        ENDIF
      ELSE
C
C   For all other cases, set the result to be a temporary of type ITP1.
C
        ITPX = ITP1
      ENDIF
C
      IF (LU .EQ. 12 .AND. IX .GT. MINT) THEN
        CALL ERRMES (56, 0)
        WRITE (6, 2)
 2      FORMAT ('Only intrinsic functions may appear in a parameter',   
     $    ' statement.')
        CALL ABRT
      ENDIF
C
      LX = 4
      ARGX = GETMP (ITPX)
C
C   Check if this is an ordinary function reference of type MP.  If so,
C   generate a call statement.
C
      IF (ITP1 .GE. 8) THEN
        LINY(1:11) = '      CALL '
        LINY(12:L1+11) = ARG1(1:L1)
        LINY(L1+12:L1+13) = ' ('
        LY = L1 + 13
C
C   Append the argument list.
C
        DO 130 I = 1, NAR
          LI = LAR(I)
          LINY(LY+1:LY+LI) = ARG(I)(1:LI)
          LINY(LY+LI+1:LY+LI+2) = ', '
          LY = LY + LI + 2
 130    CONTINUE
C
C   Set the last argument of the call statement to be the result temporary
C   name.
C
        LINY(LY+1:LY+4) = ARGX(1:4)
        LINY(LY+5:LY+5) = ')'
        LY = LY + 5
C
C   Check if it is a non-MP function reference.  If so, generate an assignment
C   statement.
C
      ELSE
        LINY(1:6) = ' '
        LINY(7:10) = ARGX(1:4)
        LINY(11:13) = ' = '
        LINY(14:L1+13) = ARG1(1:L1)
        LINY(L1+14:L1+15) = ' ('
        LY = L1 + 15
C
C   Append the argument list.
C
        DO 140 I = 1, NAR
          LI = LAR(I)
          LINY(LY+1:LY+LI) = ARG(I)(1:LI)
          LINY(LY+LI+1:LY+LI+2) = ', '
          LY = LY + LI + 2
 140    CONTINUE
C
        IF (NAR .EQ. 0) THEN
          LY = LY + 1
        ELSE
          LY = LY - 1
        ENDIF
        LINY(LY:LY) = ')'
C
      ENDIF
C
      IF (LU .EQ. 11) THEN
        CALL OUTLIN (1, LY, LINY)
      ELSE
        CALL OUTLIN (3, LY, LINY)
      ENDIF
C
C   Release any temporaries among the arguments.
C
 150  DO 160 I = 1, NAR
        LI = LAR(I)
        IF (LI .EQ. 4) THEN
          IF (ARG(I)(1:2) .EQ. 'MP') THEN
            TMP1 = ARG(I)(1:4)
            CALL RLTMP (TMP1)
          ENDIF
        ENDIF
 160  CONTINUE
C*
C      write (lu, *) 'exit genfun  itpx, argx =', itpx,
C     $  ' %'//argx(1:lx)//'%'
C*
      RETURN
      END
C
      FUNCTION GETMP (ITP)
C
C   This returns a temporary variable name that is of type ITP.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*4 GETMP
      CHARACTER*16 NAM
C
      IF (ITP .LE. 0 .OR. ITP .GT. 11) THEN
        CALL ERRMES (57, 0)
        WRITE (6, 1)
 1      FORMAT ('Improper type input to GETMP.')
        CALL ABRT
      ENDIF
C
C   The two character types don't need to be put in the table, since they are
C   never retained after definition.
C
      IF (ITP .EQ. 6) THEN
        GETMP = 'MPA1'
        GOTO 120
      ELSEIF (ITP .EQ. 11) THEN
        GETMP = 'MPA2'
        GOTO 120
      ENDIF
C
      DO 100 I = 1, 9
        IF (ITMP(I,ITP) .EQ. 0) GOTO 110
 100  CONTINUE
C
      CALL ERRMES (58, 0)
      WRITE (6, 2)
 2    FORMAT ('Statement is too complicated.')
      CALL ABRT
C
 110  ITMP(I,ITP) = 1
      WRITE (GETMP, 3) CTM(ITP), I
 3    FORMAT ('MP',A1,I1)
C
 120  NAM = GETMP
      IX = ITAB (2, ITP, NAM)
C
      RETURN
      END
C
      SUBROUTINE IFST (IFL, IFS, K1, LN)
C
C   This handles MP IF and ELSEIF statements.  K1 is the index of the left
C   parenthesis, and LN is the last non-blank character in the statement.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINA
      CHARACTER*80 ARGX
      CHARACTER*16 LINQ, UCASE
      CHARACTER*4 TMP1
C
C   If this is an elseif statement, increase the IFL counter and output an
C   else statement.
C
      IF (IFS .EQ. 2) THEN
        IFL = IFL + 1
        WRITE (11, 1)
 1      FORMAT (6X,'ELSE')
      ENDIF
C
C   Locate the matching right parenthesis of the left parenthesis.
C
      J1 = K1
      J2 = MATCH (0, J1 + 1, LN, LINE)
C
C   Determine whether the expression inside the parentheses and the expression
C   following the parentheses have MP variables.
C
      I1 = ISCAN (J1, J2, LINE)
      IF (I1 .NE. 0) CALL FIXSUB (J1, J2, LN)
      J3 = NBLK (J2 + 1, LN, LINE)
      LQ = MIN (J3 + 15, LN)
      LINQ = UCASE (LINE(J3:LQ))
      IF (J3 + 3 .EQ. LN .AND. LINQ(1:4) .EQ. 'THEN') THEN
        I2 = 0
        ITH = 1
      ELSE
        I2 = ISCAN (J3, LN, LINE)
        J4 = LN
        IF (I2 .NE. 0) CALL FIXSUB (J3, J4, LN)
        ITH = 0
      ENDIF
C
C   If the expression inside parentheses has no MP variables, there is no
C   need to process it.
C
      IF (I1 .EQ. 0) THEN
        LA = J2
        LINA(1:LA) = LINE(1:LA)
        CALL OUTLIN (1, LA, LINA)
        WRITE (11, 2)
 2      FORMAT (5X,'$  THEN')
      ELSE
C
C   Process the expression inside parentheses.
C
        LA = J2 - J1 - 1
        LINA(1:LA) = LINE(J1+1:J2-1)
        CALL EXPRES (11, LA, LINA, ITPX, LX, ARGX)
C
C   Check if the result of the expression in parentheses is of type logical.
C
        IF (ITPX .NE. 7) THEN
          CALL ERRMES (59, 0)
          WRITE (6, 3)
 3        FORMAT ('Result of expression in parentheses is not of type', 
     $      ' logical.')
          CALL ABRT
        ENDIF
C
C   Output IF statement with the resulting logical variable.
C
        WRITE (11, 4) ARGX(1:LX)
 4      FORMAT (6X,'IF (',A,') THEN')
C
C   Release final logical temporary variable.
C
        TMP1 = ARGX(1:LX)
        CALL RLTMP (TMP1)
      ENDIF
C
C   If the expression following the parentheses is merely THEN, then we are
C   done.
C
      IF (ITH .EQ. 1) GOTO 100
C
C   If the expression following the parentheses has no MP variables, then
C   it does not need to be processed.
C
      IF (I2 .EQ. 0) THEN
        LA = LN - J2 + 6
        LINA(1:6) = ' '
        LINA(7:LA) = LINE(J2+1:LN)
        CALL OUTLIN (1, LA, LINA)
      ELSE
C
C   Process the executable MP expression after the parentheses.
C
        J1 = J2 + 1
        IF (J1 .GT. LN) CALL ERRMES (60, 1)
        J1 = NBLK (J1, LN, LINE)
        CALL EXEC (J1, LN)
      ENDIF
C
      WRITE (11, 5)
 5    FORMAT (6X,'ENDIF')
C
 100  RETURN
      END
C
      SUBROUTINE IMPLIC (K1, LN)
C
C   This handles IMPLICIT statements.  K1 and LN are the indices of the
C   first (after 'IMPLICIT') and last non-blank characters in the statement.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*16 LINQ, UCASE
      CHARACTER*1 CJ
C
C   Check if this is an implicit 'none' statement.
C
      J1 = K1
      IF (J1 + 3 .EQ. LN) THEN
        LINQ = UCASE (LINE(J1:LN))
        IF (LINQ(1:4) .EQ. 'NONE') THEN
C
          DO 100 I = 1, 26
            IF (IMPL(I) .LE. 7) IMPL(I) = 0
 100      CONTINUE
C
          GOTO 130
        ENDIF
      ENDIF
      IT = NTYPE (J1, LN)
      J1 = INDX (J1, LN, '(', LINE)
      IF (IT .EQ. 0 .OR. J1 .EQ. 0) CALL ERRMES (61, 1)
C
C   Process a normal implicit statement.
C
 110  J1 = NBLK (J1 + 1, LN, LINE)
      CJ = LINE(J1:J1)
      IF (CJ .EQ. '(') GOTO 110
      IF (CJ .EQ. ',') GOTO 110
      IF (CJ .EQ. ')') GOTO 130
      I1 = MAX (INDEX (ALPL, CJ), INDEX (ALPU, CJ))
      IF (J1 .EQ. 0 .OR. I1 .EQ. 0) CALL ERRMES (62, 1)
      IF (IMPL(I1) .LE. 7) IMPL(I1) = IT
      J2 = NBLK (J1 + 1, LN, LINE)
      CJ = LINE(J2:J2)
      IF (CJ .EQ. '-') THEN
        J2 = NBLK (J2 + 1, LN, LINE)
        CJ = LINE(J2:J2)
        I2 = MAX (INDEX (ALPL, CJ), INDEX (ALPU, CJ))
        IF (I2 .EQ. 0) CALL ERRMES (63, 1)
C
        DO 120 I = I1, I2
          IF (IMPL(I) .LE. 7) IMPL(I) = IT
 120    CONTINUE
C
        J1 = J2
      ENDIF
      GOTO 110
C
 130  RETURN
      END
C
      FUNCTION INDX (K1, K2, CX, LINA)
C
C   Finds the index of the first instance of character CX in LINA between
C   positions K1 and K2.
C
      CHARACTER*1600 LINA
      CHARACTER*1 CX
C
      DO 100 I = K1, K2
        IF (LINA(I:I) .EQ. CX) GOTO 110
 100  CONTINUE
C
      I = 0
 110  INDX = I
C
      RETURN
      END
C
      SUBROUTINE INIMP
C
C   This generates declaration and initialization code for MP routines.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*80 LIN
      CHARACTER*16 NAM
      CHARACTER*8 NUM0, NUM1, NUM2, NUM3, NUM4, NUMX
C
C   Optionally print out the symbol table for this subprogram.
C*
C      write (6, '(4i4,2x,a)') (i, ktyp(i), kdim(i), lvar(i),
C     $  '%'//var(i)(1:lvar(i))//'%', i = mint + 1, nvar)
C*
C   Place the character and MP dimensions into character variables.
C
      WRITE (NUMX, '(I8)') MXP + 4
      I1 = NBLK (1, 8, NUMX)
      N1 = 9 - I1
      NUM0 = NUMX(I1:8)
      WRITE (NUMX, '(I8)') MXP + 5
      I1 = NBLK (1, 8, NUMX)
      N1 = 9 - I1
      NUM1 = NUMX(I1:8)
      WRITE (NUMX, '(I8)') 2 * MXP + 8
      I1 = NBLK (1, 8, NUMX)
      N2 = 9 - I1
      NUM2 = NUMX(I1:8)
      WRITE (NUMX, '(I8)') INT (7.225 * MXP) + 100
      I1 = NBLK (1, 8, NUMX)
      N3 = 9 - I1
      NUM3 = NUMX(I1:8)
      WRITE (NUMX, '(I8)') LSM + 4
      I1 = NBLK (1, 8, NUMX)
      N4 = 9 - I1
      NUM4 = NUMX(I1:8)
C
C   If this is the main program, make sure that at least one MPR temporary
C   is defined.
C
      IF (ISTP .EQ. 1) THEN
        NAM = 'MPM1'
        IX = ITAB (2, 9, NAM)
      ENDIF
C
C   Insert declarations variables that do not yet have valid declarations.
C   These include, for example, the temporaries generated by GETMP.
C
      DO 100 I = MINT + 1, NVAR
        KDM = KDIM(I)
        KTP = KTYP(I)
        LV = LVAR(I)
        NAM = VAR(I)
        IF (KDEC(I) .EQ. 0) THEN
          IF (KTP .EQ. 1) THEN
            WRITE (6, 1) NAM(1:LV)
 1          FORMAT (6X,'INTEGER ',A)
          ELSEIF (KTP .EQ. 2) THEN
            WRITE (6, 2) NAM(1:LV)
 2          FORMAT (6X,'REAL ',A)
          ELSEIF (KTP .EQ. 3) THEN
            WRITE (6, 3) NAM(1:LV)
 3          FORMAT (6X,'DOUBLE PRECISION ',A)
          ELSEIF (KTP .EQ. 4) THEN
            WRITE (6, 4) NAM(1:LV)
 4          FORMAT (6X,'COMPLEX ',A)
          ELSEIF (KTP .EQ. 5) THEN
            WRITE (6, 5) NAM(1:LV)
 5          FORMAT (6X,'DOUBLE COMPLEX ',A)
          ELSEIF (KTP .EQ. 6) THEN
            WRITE (6, 6) NAM(1:LV), NUM3(1:N3)
 6          FORMAT (6X,'CHARACTER*1 ',A,'(',A,')')
          ELSEIF (KTP .EQ. 7) THEN
            WRITE (6, 7) NAM(1:LV)
 7          FORMAT (6X, 'LOGICAL ',A)
C
C   For MP dimensioned variables, only declare the type as real.
C
          ELSEIF (KDM .GT. 0) THEN
            WRITE (6, 8) NAM(1:LV)
 8          FORMAT (6X,'REAL ',A)
C
C   For MP scalar variables, declare as real with the appropriate dimension.
C
          ELSEIF (KTP .EQ. 8 .OR. KTP .EQ. 9) THEN
            WRITE (6, 9) NAM(1:LV), NUM0(1:N1)
 9          FORMAT (6X,'REAL ',A,'(',A,')')
          ELSEIF (KTP .EQ. 10) THEN
            WRITE (6, 9) NAM(1:LV), NUM2(1:N2)
          ELSEIF (KTP .EQ. 11) THEN
            WRITE (6, 10) NUM4(1:N4), NAM(1:LV)
 10         FORMAT (6X,'CHARACTER*',A,' ',A)
          ENDIF
        ENDIF
C
C   If the variable is a parameter, save it.
C
        IF (KDM .EQ. -3) WRITE (6, 11) NAM(1:LV)
 11     FORMAT (6X,'SAVE ',A)
 100  CONTINUE
C
C   Insert declarations for MPNWQ, MPNW4, MPL02, MPL10 and MPPIC if any is
C   required in this subprogram.
C
      IF (ISTP .EQ. 1 .OR. KCON(1) .NE. 0 .OR. KCON(3) .NE. 0 .OR.      
     $  KCON(4) .NE. 0 .OR. KCON(5) .NE. 0)                             
     $  WRITE (6, 12) (NUM1(1:N1), I = 1, 3)
 12   FORMAT (6X,'INTEGER MPNWQ, MPNW4'/6X,'REAL MPL02, MPL10, MPPIC'/  
     $  6X,'COMMON /MPTCON/ MPNWQ, MPNW4, MPL02(',A,'), MPL10(',A,      
     $  '),'/5X,'$  MPPIC(',A,')')
C
C   Insert declaration for MPEPS if it is required.
C
      IF (KCON(2) .NE. 0) WRITE (6, 13)
 13   FORMAT (6X,'REAL MPEPS(8)')
C
C   If the scratch space directive is in effect, insert allocation.
C
      IF (ISTP .EQ. 1 .AND. MSS .NE. 0) WRITE (6, 14) MSS
 14   FORMAT (6X,'REAL MPSS'/6X,'COMMON /MPCOM3/ MPSS(',I8,')')
C
C   If the precision level is very high, insert allocation for DP scratch.
C
      IF (MXP .GE. 1016) WRITE (6, 15) MXP + 8
 15   FORMAT (6X,'DOUBLE PRECISION MPDS'/                               
     $  6X,'COMMON /MPCOM4/ MPDS(',I8,')')
C
C   If any MP parameters have been defined, define a flag.
C
      IF (MPA .GT. 0) WRITE (6, 16)
 16   FORMAT (6X,'INTEGER MPPAR'/6X,'SAVE MPPAR'/6X,'DATA MPPAR /0/')
C
C   This is the end of the declaration section.
C
      WRITE (6, 17)
 17   FORMAT ('C')
C
C   Check if this is the main program.
C
      IF (ISTP .EQ. 1) THEN
C
C   Insert calls to set the precision level and scratch space.
C
        WRITE (6, 18) MXP + 1
 18     FORMAT (6X,'CALL MPSETP (''NW'', ',I6,')')
        IF (MSS .NE. 0) WRITE (6, 19) MSS
 19     FORMAT (6X,'CALL MPSETP (''IMS'', ',I8,')')
C
C   Insert code to compute MPNWQ, MPNW4, MPL02, MPL10 and MPPIC.
C
        WRITE (6, 20) MXP, MXP + 4
 20     FORMAT (                                                        
     $    6X,'MPNWQ = ',I6/                                             
     $    6X,'MPNW4 = ',I6/                                             
     $    6X,'CALL MPDMC (2.D0, 0, MPM1)'/                              
     $    6X,'CALL MPLOG (MPM1, MPL02, MPL02)'/                         
     $    6X,'CALL MPDMC (10.D0, 0, MPM1)'/                             
     $    6X,'CALL MPLOG (MPM1, MPL02, MPL10)'/                         
     $    6X,'CALL MPPI (MPPIC)')
        WRITE (6, 18) MXP
      ENDIF
C
C   Set value for MPEPS if required.
C
      IF (KCON(2) .NE. 0) WRITE (6, 21) EPS(1)(1:LEP(1)),               
     $    EPS(2)(1:LEP(2))
 21   FORMAT (6X,'CALL MPDMC (',A,', ',A,', MPEPS)')
C
C   If MP parameters have been defined, insert code here.
C
      IF (MPA .GT. 0) THEN
        WRITE (6, 22)
 22     FORMAT (6X,'IF (MPPAR .EQ. 0) THEN')
C
 110    READ (12, '(A)', END = 120) LIN
        L1 = LNBLK (LIN)
        WRITE (6, '(A)') LIN(1:L1)
        GOTO 110
C
 120    WRITE (6, 23)
 23     FORMAT (6X,'MPPAR = 1'/6X,'ENDIF')
      ENDIF
C
      WRITE (6, 24)
 24   FORMAT ('CMP<')
C
      RETURN
      END
C
      SUBROUTINE INTRIN (LU, ITP1, L1, ARG1, NAR, ITAR, LAR, ARG, ITPX, 
     $  LX, ARGX)
C
C   This generates code for MP intrinsic functions.  The function name is ARG1,
C   with type ITP1 and length L1.  NAR is the number of arguments.  The
C   argument list is input in ARG, with types in ITAR and lengths in LAR.
C   The result is placed in ARGX, with type in ITPX and length in LX.
C   LU is the unit number for output code.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*80 ARG1, ARG(MAR), ARGX, ARGZ
      CHARACTER*16 NAM
      DIMENSION ITAR(MAR), LAR(MAR)
      CHARACTER*4 GETMP, TMP1, TMP2, TMP3, TMP4, TMP5
C*
C      write (lu, *) 'enter intrin, nar, arg1 =', nar,
C     $  ' %'//arg1(1:l1)//'%'
C      write (lu, '(2i4,2x,a)') (i, itar(i), arg(i)(1:lar(i)), i = 1, nar)
C*
      LL = MIN (L1, 16)
      NAM = ARG1(1:LL)
      IX = ITAB (0, 0, NAM)
      KTP = KTYP(IX)
C
C   For functions with an argument-dependent result type, the type of the
C   result is the type of the first argument, except for ABS with a MPC
C   argument and NINT.
C
      IF (KTP .NE. -1) THEN
        ITPX = KTP
      ELSEIF (IX .EQ. 11 .AND. ITAR(1) .EQ. 10) THEN
        ITPX = 9
      ELSEIF (IX .EQ. 39) THEN
        ITPX = 8
      ELSE
        ITPX = ITAR(1)
      ENDIF
      LX = 4
      ARGX = GETMP (ITPX)
C
      GOTO (                                                            
     $  100, 110, 120, 440, 130, 140, 150, 160, 450, 170,               
     $  180, 190, 200, 210, 220, 440, 450, 450, 230, 450,               
     $  450, 240, 450, 250, 260, 270, 280, 290, 300, 310,               
     $  320, 330, 340, 350, 360, 370, 380, 390, 400, 410,               
     $   420, 430, 460, 460) IX - 10
C
C   It is ABS.
C
 100  IF (NAR .NE. 1) GOTO 450
      LA1 = LAR(1)
      IF (ITAR(1) .NE. 10) THEN
        WRITE (LU, 1) ARG(1)(1:LA1), ARGX(1:4)
 1      FORMAT (6X,'CALL MPEQ (',A,', ',A,')')
        WRITE (LU, 2) ARGX(1:4), ARGX(1:4)
 2      FORMAT (6X,A,'(1) = ABS (',A,'(1))')
      ELSE
        TMP1 = GETMP (9)
        TMP2 = GETMP (9)
        TMP3 = GETMP (9)
        TMP4 = GETMP (9)
        WRITE (LU, 3) ARG(1)(1:LA1), TMP1, TMP2
 3      FORMAT (6X,'CALL MPMPCM (MPNW4, ',A,', ',A,', ',A,')')
        WRITE (LU, 4) TMP1, TMP1, TMP3
 4      FORMAT (6X,'CALL MPMUL (',A,', ',A,', ',A,')')
        WRITE (LU, 4) TMP2, TMP2, TMP4
        WRITE (LU, 5) TMP3, TMP4, TMP1
 5      FORMAT (6X,'CALL MPADD (',A,', ',A,', ',A,')')
        WRITE (LU, 6) TMP1, ARGX(1:4)
 6      FORMAT (6X,'CALL MPSQRT (',A,', ',A,')')
        CALL RLTMP (TMP1)
        CALL RLTMP (TMP2)
        CALL RLTMP (TMP3)
        CALL RLTMP (TMP4)
      ENDIF
      GOTO 470
C
C   It is ACOS.
C
 110  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      KCON(5) = 1
      LA1 = LAR(1)
      TMP1 = GETMP (9)
      TMP2 = GETMP (9)
      TMP3 = GETMP (9)
      WRITE (LU, 7) TMP1
 7    FORMAT (6X,'CALL MPDMC (1.D0, 0, ',A,')')
      WRITE (LU, 4) ARG(1)(1:LA1), ARG(1)(1:LA1), TMP2
      WRITE (LU, 8) TMP1, TMP2, TMP3
 8    FORMAT (6X,'CALL MPSUB (',A,', ',A,', ',A,')')
      WRITE (LU, 6) TMP3, TMP1
      WRITE (LU, 9) ARG(1)(1:LA1), TMP1, ARGX(1:4)
 9    FORMAT (6X,'CALL MPANG (',A,', ',A,', MPPIC, ',A,')')
      CALL RLTMP (TMP1)
      CALL RLTMP (TMP2)
      CALL RLTMP (TMP3)
      GOTO 470
C
C   It is AINT.
C
 120  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      LA1 = LAR(1)
      TMP1 = GETMP (9)
      WRITE (LU, 10) ARG(1)(1:LA1), ARGX(1:4), TMP1
 10   FORMAT (6X,'CALL MPINFR (',A,', ',A,', ',A,')')
      CALL RLTMP (TMP1)
      GOTO 470
C
C   It is ANINT.
C
 130  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      LA1 = LAR(1)
      WRITE (LU, 11) ARG(1)(1:LA1), ARGX(1:4)
 11   FORMAT (6X,'CALL MPNINT (',A,', ',A,')')
      GOTO 470
C
C   It is ASIN.
C
 140  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      KCON(5) = 1
      LA1 = LAR(1)
      TMP1 = GETMP (9)
      TMP2 = GETMP (9)
      TMP3 = GETMP (9)
      WRITE (LU, 7) TMP1
      WRITE (LU, 4) ARG(1)(1:LA1), ARG(1)(1:LA1), TMP2
      WRITE (LU, 8) TMP1, TMP2, TMP3
      WRITE (LU, 6) TMP3, TMP1
      WRITE (LU, 9) TMP1, ARG(1)(1:LA1), ARGX(1:4)
      CALL RLTMP (TMP1)
      CALL RLTMP (TMP2)
      CALL RLTMP (TMP3)
      GOTO 470
C
C   It is ATAN.
C
 150  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      KCON(5) = 1
      LA1 = LAR(1)
      TMP1 = GETMP (9)
      WRITE (LU, 7) TMP1
      WRITE (LU, 9) TMP1, ARG(1)(1:LA1), ARGX(1:4)
      CALL RLTMP (TMP1)
      GOTO 470
C
C   It is ATAN2.
C
 160  IF (NAR .NE. 2 .OR. ITAR(1) .NE. 9 .OR. ITAR(2) .NE. 9) GOTO 450
      KCON(5) = 1
      LA1 = LAR(1)
      LA2 = LAR(2)
      WRITE (LU, 9) ARG(2)(1:LA2), ARG(1)(1:LA1), ARGX(1:4)
      GOTO 470
C
C   It is CMPLX (convert MPC to CO).
C
 170  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 10) GOTO 450
      WRITE (LU, 12)
 12   FORMAT ('CMP*'/'CMP*  Note: The result of CMPLX with an MP',      
     $  ' argument has type CO.'/'CMP*  If an MPC result is',           
     $  ' required, use DPCMPL or an assignment statement.'/'CMP*')
      LA1 = LAR(1)
      TMP1 = GETMP (9)
      TMP2 = GETMP (9)
      TMP3 = GETMP (3)
      TMP4 = GETMP (3)
      TMP5 = GETMP (1)
      WRITE (LU, 3) ARG(1)(1:LA1), TMP1, TMP2
      WRITE (LU, 13) TMP1, TMP3, TMP5
 13   FORMAT (6X,'CALL MPMDC (',A,', ',A,', ',A,')')
      WRITE (LU, 14) TMP3, TMP3, TMP5
 14   FORMAT (6X,A,' = ',A,' * 2.D0 ** ',A)
      WRITE (LU, 13) TMP2, TMP4, TMP5
      WRITE (LU, 14) TMP4, TMP4, TMP5
      WRITE (LU, 15) ARGX(1:4), TMP3, TMP4
 15   FORMAT (6X,A,' = DCMPLX (',A,', ',A,')')
      CALL RLTMP (TMP1)
      CALL RLTMP (TMP2)
      CALL RLTMP (TMP3)
      CALL RLTMP (TMP4)
      CALL RLTMP (TMP5)
      GOTO 470
C
C   It is CONJG.
C
 180  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 10) GOTO 450
      LA1 = LAR(1)
      WRITE (LU, 16) ARG(1)(1:LA1), ARGX(1:4)
 16   FORMAT (6X,'CALL MPCEQ (MPNW4, ',A,', ',A,')')
      WRITE (LU, 17) ARGX(1:4), ARGX(1:4)
 17   FORMAT (6X,A,'(MPNWQ+5) = - ',A,'(MPNWQ+5)')
      GOTO 470
C
C   It is COS.
C
 190  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      KCON(5) = 1
      LA1 = LAR(1)
      TMP1 = GETMP (9)
      WRITE (LU, 18) ARG(1)(1:LA1), ARGX(1:4), TMP1
 18   FORMAT (6X,'CALL MPCSSN (',A,', MPPIC, ',A,', ',A,')')
      CALL RLTMP (TMP1)
      GOTO 470
C
C   It is COSH.
C
 200  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      KCON(3) = 1
      LA1 = LAR(1)
      TMP1 = GETMP (9)
      WRITE (LU, 19) ARG(1)(1:LA1), ARGX(1:4), TMP1
 19   FORMAT (6X,'CALL MPCSSH (',A,', MPL02, ',A,', ',A,')')
      CALL RLTMP (TMP1)
      GOTO 470
C
C   It is DBLE (convert MPI or MPR to DP).
C
 210  IF (NAR .NE. 1) GOTO 450
      WRITE (LU, 20)
 20   FORMAT ('CMP*'/'CMP*  Note: The result of DBLE with an MP',       
     $  ' argument has type DP.'/'CMP*  If an MPR result is',           
     $  ' required, use DPREAL or an assignment statement.'/'CMP*')
      LA1 = LAR(1)
      TMP1 = GETMP (3)
      TMP2 = GETMP (1)
      WRITE (LU, 13) ARG(1)(1:LA1), TMP1, TMP2
      WRITE (LU, 14) ARGX(1:4), TMP1, TMP2
      CALL RLTMP (TMP1)
      CALL RLTMP (TMP2)
      GOTO 470
C
C   It is DCMPLX (convert MPC to DC).
C
 220  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 10) GOTO 450
      WRITE (LU, 21)
 21   FORMAT ('CMP*'/'CMP*  Note: The result of DCMPLX with an MP',     
     $  ' argument has type DC.'/'CMP*  If an MPC result is',           
     $  ' required, use DPCMPL or an assignment statement.'/'CMP*')
      LA1 = LAR(1)
      TMP1 = GETMP (9)
      TMP2 = GETMP (9)
      TMP3 = GETMP (3)
      TMP4 = GETMP (3)
      TMP5 = GETMP (1)
      WRITE (LU, 3) ARG(1)(1:LA1), TMP1, TMP2
      WRITE (LU, 13) TMP1, TMP3, TMP5
      WRITE (LU, 14) TMP3, TMP3, TMP5
      WRITE (LU, 13) TMP2, TMP4, TMP5
      WRITE (LU, 14) TMP4, TMP4, TMP5
      WRITE (LU, 15) ARGX(1:4), TMP3, TMP4
      CALL RLTMP (TMP1)
      CALL RLTMP (TMP2)
      CALL RLTMP (TMP3)
      CALL RLTMP (TMP4)
      CALL RLTMP (TMP5)
      GOTO 470
C
C   It is EXP.
C
 230  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      KCON(3) = 1
      LA1 = LAR(1)
      WRITE (LU, 22) ARG(1)(1:LA1), ARGX(1:4)
 22   FORMAT (6X,'CALL MPEXP (',A,', MPL02, ',A,')')
      GOTO 470
C
C   It is INT (convert MPI or MPR to IN).
C
 240  IF (NAR .NE. 1) GOTO 450
      WRITE (LU, 23)
 23   FORMAT ('CMP*'/'CMP*  Note: The result of INT with an MP',        
     $  ' argument has type IN.'/'CMP*  If an MPI result is',           
     $  ' required, use MPINT or an assignment statement.'/'CMP*')
      LA1 = LAR(1)
      TMP1 = GETMP (3)
      TMP2 = GETMP (1)
      WRITE (LU, 13) ARG(1)(1:LA1), TMP1, TMP2
      WRITE (LU, 14) ARGX(1:4), TMP1, TMP2
      CALL RLTMP (TMP1)
      CALL RLTMP (TMP2)
      GOTO 470
C
C   It is LOG.
C
 250  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      KCON(3) = 1
      LA1 = LAR(1)
      WRITE (LU, 24) ARG(1)(1:LA1), ARGX(1:4)
 24   FORMAT (6X,'CALL MPLOG (',A,', MPL02, ',A,')')
      GOTO 470
C
C   It is LOG10.
C
 260  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      KCON(4) = 1
      LA1 = LAR(1)
      TMP1 = GETMP (9)
      WRITE (LU, 24) ARG(1)(1:LA1), TMP1
      WRITE (LU, 25) TMP1, 'MPL10', ARGX(1:4)
 25   FORMAT (6X,'CALL MPDIV (',A,', ',A,', ',A,')')
      CALL RLTMP (TMP1)
      GOTO 470
C
C   It is MAX.
C
 270  IF (NAR .NE. 2 .OR. ITAR(1) .NE. ITAR(2) .OR. ITAR(1) .EQ. 10)    
     $  GOTO 450
      LA1 = LAR(1)
      LA2 = LAR(2)
      TMP1 = GETMP (1)
      WRITE (LU, 26) ARG(1)(1:LA1), ARG(2)(1:LA2), TMP1
 26   FORMAT (6X,'CALL MPCPR (',A,', ',A,', ',A,')')
      WRITE (LU, 27) TMP1, ARG(1)(1:LA1), ARGX(1:4), ARG(2)(1:LA2),     
     $  ARGX(1:4)
 27   FORMAT (6X,'IF (',A,' .GE. 0) THEN'/                              
     $  8X,'CALL MPEQ (',A,', ',A,')'/6X,'ELSE'/                        
     $  8X,'CALL MPEQ (',A,', ',A,')'/6X,'ENDIF')
      CALL RLTMP (TMP1)
      GOTO 470
C
C   It is MIN.
C
 280  IF (NAR .NE. 2 .OR. ITAR(1) .NE. ITAR(2) .OR. ITAR(1) .EQ. 10)    
     $  GOTO 450
      LA1 = LAR(1)
      LA2 = LAR(2)
      TMP1 = GETMP (1)
      WRITE (LU, 26) ARG(1)(1:LA1), ARG(2)(1:LA2), TMP1
      WRITE (LU, 27) TMP1, ARG(2)(1:LA2), ARGX(1:4), ARG(1)(1:LA1),     
     $  ARGX(1:4)
      CALL RLTMP (TMP1)
      GOTO 470
C
C   It is MOD.
C
 290  IF (NAR .NE. 2 .OR. ITAR(1) .NE. ITAR(2) .OR. ITAR(1) .EQ. 10)    
     $  GOTO 450
      LA1 = LAR(1)
      LA2 = LAR(2)
      TMP1 = GETMP (9)
      TMP2 = GETMP (8)
      TMP3 = GETMP (9)
      WRITE (LU, 25) ARG(1)(1:LA1), ARG(2)(1:LA2), TMP1
      WRITE (LU, 10) TMP1, TMP2, TMP3
      WRITE (LU, 4) ARG(2)(1:LA2), TMP2, TMP1
      WRITE (LU, 8) ARG(1)(1:LA1), TMP1, ARGX(1:4)
      CALL RLTMP (TMP1)
      CALL RLTMP (TMP2)
      CALL RLTMP (TMP3)
      GOTO 470
C
C   It is NINT.
C
 300  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      LA1 = LAR(1)
      WRITE (LU, 11) ARG(1)(1:LA1), ARGX(1:4)
      GOTO 470
C
C   It is REAL (convert MPI or MPR to SP).
C
 310  IF (NAR .NE. 1) GOTO 450
      WRITE (LU, 28)
 28   FORMAT ('CMP*'/'CMP*  Note: The result of REAL with an MP',       
     $  ' argument has type SP.'/'CMP*  If an MPR result is',           
     $  ' required, use DPREAL or an assignment statement.'/'CMP*')
      LA1 = LAR(1)
      TMP1 = GETMP (3)
      TMP2 = GETMP (1)
      WRITE (LU, 13) ARG(1)(1:LA1), TMP1, TMP2
      WRITE (LU, 14) ARGX(1:4), TMP1, TMP2
      CALL RLTMP (TMP1)
      CALL RLTMP (TMP2)
      GOTO 470
C
C   It is SIGN.
C
 320  IF (NAR .NE. 2 .OR. ITAR(1) .NE. ITAR(2) .OR. ITAR(1) .EQ. 10)    
     $  GOTO 450
      LA1 = LAR(1)
      LA2 = LAR(2)
      TMP1 = GETMP (9)
      WRITE (LU, 1) ARG(1)(1:LA1), ARGX(1:4)
      WRITE (LU, 1) ARG(2)(1:LA2), TMP1
      WRITE (LU, 29) TMP1, ARGX(1:4), ARGX(1:4), ARGX(1:4), ARGX(1:4)
 29   FORMAT (6X,'IF (',A,'(1) .GE. 0.) THEN'/                          
     $  6X,A,'(1) = ABS (',A,'(1))'/6X,'ELSE'/                          
     $  6X,A,'(1) = - ABS (',A,'(1))'/6X,'ENDIF')
      CALL RLTMP (TMP1)
      GOTO 470
C
C   It is SIN.
C
 330  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      KCON(5) = 1
      LA1 = LAR(1)
      TMP1 = GETMP (9)
      WRITE (LU, 18) ARG(1)(1:LA1), TMP1, ARGX(1:4)
      CALL RLTMP (TMP1)
      GOTO 470
C
C   It is SINH.
C
 340  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      KCON(3) = 1
      LA1 = LAR(1)
      TMP1 = GETMP (9)
      WRITE (LU, 19) ARG(1)(1:LA1), TMP1, ARGX(1:4)
      CALL RLTMP (TMP1)
      GOTO 470
C
C   It is SQRT.
C
 350  IF (NAR .NE. 1 .OR. ITAR(1) .EQ. 8) GOTO 450
      LA1 = LAR(1)
      IF (ITAR(1) .EQ. 9) THEN
        WRITE (LU, 6) ARG(1)(1:LA1), ARGX(1:4)
      ELSEIF (ITAR(1) .EQ. 10) THEN
        WRITE (LU, 30) ARG(1)(1:LA1), ARGX(1:4)
 30     FORMAT (6X,'CALL MPCSQR (MPNW4, ',A,', ',A,')')
      ENDIF
      GOTO 470
C
C   It is TAN.
C
 360  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      KCON(5) = 1
      LA1 = LAR(1)
      TMP1 = GETMP (9)
      TMP2 = GETMP (9)
      WRITE (LU, 18) ARG(1)(1:LA1), TMP1, TMP2
      WRITE (LU, 25) TMP2, TMP1, ARGX(1:4)
      CALL RLTMP (TMP1)
      CALL RLTMP (TMP2)
      GOTO 470
C
C   It is TANH.
C
 370  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      KCON(3) = 1
      LA1 = LAR(1)
      TMP1 = GETMP (9)
      TMP2 = GETMP (9)
      WRITE (LU, 19) ARG(1)(1:LA1), TMP1, TMP2
      WRITE (LU, 25) TMP2, TMP1, ARGX(1:4)
      CALL RLTMP (TMP1)
      CALL RLTMP (TMP2)
      GOTO 470
C
C   It is MPINT.
C
 380  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 9) GOTO 450
      LA1 = LAR(1)
      TMP1 = GETMP (9)
      WRITE (LU, 10) ARG(1)(1:LA1), ARGX(1:4), TMP1
      CALL RLTMP (TMP1)
      GOTO 470
C
C   It is DPCMPL.
C
 390  IF (NAR .NE. 2 .OR. ITAR(1) .NE. 9 .OR. ITAR(2) .NE. 9) GOTO 450
      LA1 = LAR(1)
      LA2 = LAR(2)
      WRITE (LU, 31) ARG(1)(1:LA1), ARG(2)(1:LA2), ARGX(1:4)
 31   FORMAT (6X,'CALL MPMMPC (',A,', ',A,', MPNW4, ',A,')')
      GOTO 470
C
C   It is DPIMAG.
C
 400  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 10) GOTO 450
      LA1 = LAR(1)
      TMP1 = GETMP (9)
      WRITE (LU, 3) ARG(1)(1:LA1), TMP1, ARGX(1:4)
      CALL RLTMP (TMP1)
      GOTO 470
C
C   It is DPREAL.
C
 410  IF (NAR .NE. 1 .OR. ITAR(1) .NE. 10) GOTO 450
      LA1 = LAR(1)
      WRITE (LU, 1) ARG(1)(1:LA1), ARGX(1:4)
      GOTO 470
C
C   It is DPNRT.
C
 420  IF (NAR .NE. 2 .OR. ITAR(1) .NE. 9 .OR. ITAR(2) .NE. 1) GOTO 450
      LA1 = LAR(1)
      LA2 = LAR(2)
      WRITE (LU, 32) ARG(1)(1:LA1), ARG(2)(1:LA2), ARGX(1:4)
 32   FORMAT (6X,'CALL MPNRT (',A,', ',A,', ',A,')')
      GOTO 470
C
C   It is DPRAND.
C
 430  IF (NAR .NE. 0) GOTO 450
      WRITE (LU, 33) ARGX(1:4)
 33   FORMAT (6X,'CALL MPRAND (',A,')')
      GOTO 470
C
 440  CALL ERRMES (64, 0)
      WRITE (6, 34) ARG1(1:L1)
 34   FORMAT ('This intrinsic function is not defined with this MP',    
     $  ' argument type: ',A)
      CALL ABRT
C
 450  CALL ERRMES (65, 0)
      WRITE (6, 35) ARG1(1:L1)
 35   FORMAT ('This intrinsic function has an improper argument list: ',
     $  A)
      CALL ABRT
C
 460  CALL ERRMES (66, 0)
      WRITE (6, 36) ARG1(1:L1)
 36   FORMAT ('This subroutine name may not be used as a function: ',A)
      CALL ABRT
C
 470  CONTINUE
C*
C      write (lu, *) 'exit intrin  itpx, argx =', itpx,
C     $  ' %'//argx(1:lx)//'%'
C*
      RETURN
      END
C
      FUNCTION IPFSB (K1, LN)
C
C   This checks to see if the statement is a program, subroutine, function
C   or block data statement. K1 and LN are the indices of the first and last
C   non-blank characters in the statement.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINA
      CHARACTER*16 LINQ, NAM, UCASE
C
      J1 = K1
      LQ = MIN (J1 + 15, LN)
      LINQ = UCASE (LINE(J1:LQ))
      IF (LINQ(1:7) .EQ. 'PROGRAM') THEN
        IF (IEX .NE. 0) GOTO 100
        ISTP = 1
        IPFSB = 1
        J1 = NBLK (J1 + 7, LN, LINE)
        IF (J1 .EQ. 0) GOTO 110
        FNAM = LINE(J1:LN)
        LSAR = 0
        CALL OUTLIN (1, LN, LINE)
      ELSEIF (LINQ(1:10) .EQ. 'SUBROUTINE') THEN
        IF (IEX .NE. 0) GOTO 100
        ISTP = 2
        IPFSB = 1
        J1 = NBLK (J1 + 10, LN, LINE)
        IF (J1 .EQ. 0) GOTO 110
        J3 = INDX (J1, LN, '(', LINE)
        IF (J3 .NE. 0) THEN
          J2 = J3 - 1
        ELSE
          J2 = LN
        ENDIF
        FNAM = LINE(J1:J2)
        IF (J3 .NE. 0) THEN
          LSAR = LN - J3 - 1
          SARG(1:LSAR) = LINE(J3+1:LN-1)
        ELSE
          LSAR = 0
          SARG(1:1) = ' '
        ENDIF
        CALL OUTLIN (1, LN, LINE)
      ELSEIF (LINQ(1:8) .EQ. 'FUNCTION') THEN
        IF (IEX .NE. 0) GOTO 100
        ISTP = 3
        JS = J1
        J1 = NBLK (J1 + 8, LN, LINE)
        J3 = INDX (J1, LN, '(', LINE)
        IF (J1 .EQ. 0 .OR. J3 .EQ. 0) GOTO 110
        J2 = J3 - 1
        FNAM = LINE(J1:J2)
        IX = ITAB (0, 0, FNAM)
        LSAR = LN - J3 - 1
        SARG(1:LSAR) = LINE(J3+1:LN-1)
C
C   If the function name is MP, then change to subroutine statement.
C
        IF (IX .GT. MINT .AND. KTYP(IX) .GE. 8) THEN
          KDIM(IX) = -2
          CALL OUTLIN (2, LN, LINE)
          LINA(1:JS-1) = LINE(1:JS-1)
          LINA(JS:JS+9) = 'SUBROUTINE'
          LINA(JS+10:LN+1) = LINE(JS+8:LN-1)
          IF (LSAR .EQ. 0 .OR. SARG(1:LSAR) .EQ. ' ') THEN
            LINA(LN+2:LN+7) = 'MPFVX)'
            LA = LN + 7
          ELSE
            LINA(LN+2:LN+9) = ', MPFVX)'
            LA = LN + 9
          ENDIF
          NAM = 'MPFVX'
          IX = ITAB (2, KTYP(IX), NAM)
          CALL OUTLIN (1, LA, LINA)
          WRITE (11, 1)
 1        FORMAT ('CMP<')
        ELSE
          CALL OUTLIN (1, LN, LINE)
        ENDIF
        IPFSB = 1
      ELSEIF (LINQ(1:10) .EQ. 'BLOCK DATA') THEN
        IF (IEX .NE. 0) GOTO 100
        ISTP = 4
        IPFSB = 1
        CALL OUTLIN (1, LN, LINE)
      ELSE
        IPFSB = 0
      ENDIF
      GOTO 130
C
 100  CALL ERRMES (67, 0)
      WRITE (6, 2)
 2    FORMAT ('A declarative statement may not appear after an',        
     $  ' executable statement.')
      CALL ABRT
C
 110  CALL ERRMES (68, 0)
      WRITE (6, 3)
 3    FORMAT ('The subprogram name or argument list is missing.')
      CALL ABRT
C
 120  CALL ERRMES (69, 0)
      WRITE (6, 4)
 4    FORMAT ('Too many program units in this file.')
      CALL ABRT
C
 130  RETURN
      END
C
      FUNCTION ISCAN (K1, LN, LINA)
C
C   This scans LINE between positions K1 and LN for variable names and
C   enters new ones into the table.  The result is 1 if a MP constant or
C   special constant name is found, 2 if a MP variable is found, and 0
C   otherwise.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINA
      CHARACTER*16 NAM
      CHARACTER*1 CJ
C
      J1 = K1
      ISCAN = 0
C
C   Extract the next character from the line.
C
 100  IF (J1 .GT. LN) GOTO 130
      J1 = NBLK (J1, LN, LINA)
      CJ = LINA(J1:J1)
C
C   Check if it is the start of a numeric constant.
C
      IS1 = INDEX (DIG, CJ)
      IF ((CJ .EQ. '-' .OR. CJ .EQ. '.') .AND. J1 .LT. LN) THEN
        J2 = NBLK (J1 + 1, LN, LINA)
        IS2 = INDEX (DIG, LINA(J2:J2))
      ELSE
        IS2 = 0
      ENDIF
C
      IF (IS1 .NE. 0 .OR. IS2 .NE. 0) THEN
        ITP = NUMCON (J1, J2, LN, LINA)
        J1 = J2 + 1
        GOTO 100
C
C   Check if it the start of a name.
C
      ELSEIF (MAX (INDEX (ALPL, CJ), INDEX (ALPU, CJ)) .NE. 0) THEN
C
        DO 110 J = J1, LN
          CJ = LINA(J:J)
          IF (INDEX (DEL, CJ) .NE. 0) GOTO 120
 110    CONTINUE
C
        J = LN + 1
 120    I2 = J - 1
        NAM = LINA(J1:I2)
        IX = ITAB (1, 0, NAM)
        IF (IX .EQ. 0) THEN
          J1 = I2 + 1
          GOTO 100
        ENDIF
        ITP = KTYP(IX)
        IF (ITP .LT. 8) KDEC(IX) = 1
C
C   Check if the variable is a special constant.
C
        IF (IX .GE. 1 .AND. IX .LE. 5) THEN
          ISCAN = MAX (ISCAN, 1)
C
C   Check if the variable is MP.
C
        ELSEIF (ITP .GE. 8) THEN
          ISCAN = 2
        ENDIF
        J1 = I2 + 1
        GOTO 100
C
C   Check if it is the start of a logical constant.
C
      ELSEIF (CJ .EQ. '.') THEN
        I1 = INDX (J1 + 1, LN, '.', LINA)
        IF (I1 .EQ. 0) CALL ERRMES (70, 1)
        J1 = I1 + 1
        GOTO 100
C
C   Check if it is the start of a character constant.
C
      ELSEIF (CJ .EQ. '"') THEN
        I1 = INDX (J1 + 1, LN, '"', LINA)
        IF (I1 .EQ. 0) CALL ERRMES (71, 1)
        J1 = I1 + 1
        GOTO 100
      ELSEIF (CJ .EQ. "'") THEN
        I1 = INDX (J1 + 1, LN, "'", LINA)
        IF (I1 .EQ. 0) CALL ERRMES (72, 1)
        J1 = I1 + 1
        GOTO 100
      ENDIF
C
C   Check if it is one of the miscellaneous symbols.
C
      I1 = INDEX (DEL, CJ)
      IF (I1 .EQ. 0) THEN
        CALL ERRMES (73, 0)
        WRITE (6, 1) CJ
 1      FORMAT ('Illegal character: ',A)
        CALL ABRT
      ENDIF
      J1 = J1 + 1
      GOTO 100
C
 130  CONTINUE
      RETURN
      END
C
      FUNCTION ITAB (IC, ITP, NAM)
C
C   This routine searches for NAM in the variable table and returns its index
C   if found.  If it is not found, the action depends on the value of IC as
C   follows:
C     IC
C     0   Name is not entered in table and zero is returned.
C     1   New names are entered in table.  Four letter names starting with DP
C         or MP are NOT allowed.  The entry in KTYP is set to ITP if ITP is
C         nonzero, otherwise to the implicit type.
C     2   New names are entered in table.  Four letter names starting with DP
C         or MP are allowed.  The entry in KTYP is set to ITP if ITP is
C         nonzero, otherwise to the implicit type.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*16 NAM, NAMX, UCASE
      CHARACTER*1 CJ
C
C   Set NAMX = NAM with upper case alphabetics and check table.
C
      NAMX = UCASE (NAM)
C
      DO 100 I = 1, NVAR
        IF (VAR(I) .EQ. NAMX) THEN
          ITAB = I
          GOTO 120
        ENDIF
 100  CONTINUE
C
C   NAMX is not in table.
C
      IF (IC .EQ. 0) THEN
        ITAB = 0
        GOTO 120
      ENDIF
C
C   Check if NAMX is a common Fortran keyword -- if so, don't enter in table.
C
      DO 110 I = 1, NKY
        IF (NAMX .EQ. KEYW(I)) THEN
          ITAB = 0
          GOTO 120
        ENDIF
 110  CONTINUE
C
C   Check if it is a four-letter name starting with DP or MP.
C
      L1 = LNBLK (NAMX)
      IF (IC .EQ. 1 .AND. L1 .EQ. 4) THEN
        IF (NAMX(1:2) .EQ. 'DP' .OR. NAMX(1:2) .EQ. 'MP') THEN
          CALL ERRMES (74, 0)
          WRITE (6, 1)
 1        FORMAT ('Four-letter names starting with DP or MP are not',   
     $      ' allowed.')
          CALL ABRT
        ENDIF
      ENDIF
C
C   Add NAMX to table.
C
      NVAR = NVAR + 1
      IF (NVAR .GT. MVAR) THEN
        CALL ERRMES (75, 0)
        WRITE (6, 2)
 2      FORMAT ('Subprogram has too many variables.')
        CALL ABRT
      ENDIF
C
      VAR(NVAR) = NAMX
      KDEC(NVAR) = 0
      KDIM(NVAR) = 0
      LVAR(NVAR) = L1
C
C   If ITP is greater than zero, set the type of the new variable to ITP.
C   If ITP is zero, set the type to the implicit type.  The type is stored
C   in the array KTYP.  Whether explicit or implicit is stored in MPLC.
C
      IF (ITP .GT. 0) THEN
        KTYP(NVAR) = ITP
        MPLC(NVAR) = 0
      ELSE
        CJ = NAMX(1:1)
        I1 = INDEX (ALPU, CJ)
        IF (I1 .EQ. 0) CALL ERRMES (76, 1)
        KTYP(NVAR) = IMPL(I1)
        MPLC(NVAR) = 1
      ENDIF
      ITAB = NVAR
C
 120  RETURN
      END
C
      FUNCTION LNBLK (LIN)
      CHARACTER*(*) LIN
C
C   This finds the index of the last non-blank character in LIN.
C
      LN = LEN (LIN)
C
      DO 100 I = LN, 1, -1
        IF (LIN(I:I) .NE. ' ') GOTO 110
 100  CONTINUE
C
      I = 0
 110  LNBLK = I
C
      RETURN
      END
C
      FUNCTION MATCH (IC, K1, LA, LINA)
C
C   This finds the location (up to LA) in LINA of the right parenthesis that
C   matches the left parenthesis at location K1 - 1.  If IC is nonzero, a comma
C   will also will be accepted as a terminator.  Parentheses or commas in
C   character constants are ignored.
C
      CHARACTER*1600 LINA
      CHARACTER*1 CJ
C
      J1 = K1
      IP = 0
      MATCH = 0
C
 100  IF (J1 .GT. LA) GOTO 110
      J1 = NBLK (J1, LA, LINA)
      IF (J1 .EQ. 0) GOTO 110
      CJ = LINA(J1:J1)
      IF (CJ .EQ. ')' .OR. (CJ .EQ. ',' .AND. IC .NE. 0 .AND.           
     $  IP .EQ. 0)) THEN
        IF (IP .EQ. 0) THEN
          MATCH = J1
          GOTO 110
        ELSE
          IP = IP - 1
          J1 = J1 + 1
          GOTO 100
        ENDIF
      ELSEIF (CJ .EQ. '(') THEN
        IP = IP + 1
        J1 = J1 + 1
        GOTO 100
      ELSEIF (CJ .EQ. '"') THEN
        J1 = INDX (J1 + 1, LA, '"', LINA)
        IF (J1 .EQ. 0) CALL ERRMES (77, 1)
        J1 = J1 + 1
        GOTO 100
      ELSEIF (CJ .EQ. "'") THEN
        J1 = INDX (J1 + 1, LA, "'", LINA)
        IF (J1 .EQ. 0) CALL ERRMES (78, 1)
        J1 = J1 + 1
        GOTO 100
      ELSE
        J1 = J1 + 1
        GOTO 100
      ENDIF
C
 110  CONTINUE
C
      RETURN
      END
C
      SUBROUTINE MPDEC (LN)
C
C   This checks to see if a comment is a MP directive.  LN is the index of
C   the last non-blank character.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
C>   Set CON to be the Log_10 BDX, where BDX is the radix in MPFUN.
C    For IEEE and other systems for which BDX = 2^24, CON = 7.224719896D0.
C    For Cray systems and others for which BDX = 2^22, CON = 6.622659905D0.
C
      DOUBLE PRECISION CON, CL2, RLT, T1
      PARAMETER (CON = 7.224719896D0, RLT = 3.3219280948873621D0)
      CHARACTER*16 LINQ, NUMX, NUMY, UCASE
C
      J1 = NBLK (5, LN, LINE)
C
      LQ = MIN (J1 + 15, LN)
      LINQ = UCASE (LINE(J1:LQ))
      IF (LINQ(1:15) .EQ. 'PRECISION LEVEL') THEN
        IF (MXP .NE. 0 .OR. ISTP .NE. 0) GOTO 110
        IF (IEX .NE. 0) GOTO 140
        NUMX = LINE(J1+15:LN)
        READ (NUMX, '(BN,I16)', ERR = 120) K
        IF (K .LE. 0) GOTO 120
        MXP = (K + 7) / CON
        MPP = CON * MXP
        T1 = RLT * (7 - K)
        N1 = T1 - 1.D0
        T1 = 2.D0 ** (T1 - N1)
        WRITE (NUMY, '(F14.12,"D0")') T1
        I1 = NBLK (1, 16, NUMY)
        LEP(1) = 17 - I1
        EPS(1) = NUMY(I1:16)
        WRITE (NUMY, '(I10)') N1
        I1 = NBLK (1, 10, NUMY)
        LEP(2) = 11 - I1
        EPS(2) = NUMY(I1:10)
      ELSEIF (LINQ(1:13) .EQ. 'SCRATCH SPACE') THEN
        IF (MXP .EQ. 0) GOTO 100
        IF (MSS .NE. 0 .OR. ISTP .NE. 0) GOTO 110
        IF (IEX .NE. 0) GOTO 140
        NUMX = LINE(J1+13:LN)
        READ (NUMX , '(BN,I16)', ERR = 120) K
        IF (K .LE. 0) GOTO 120
        MSS = K
      ELSEIF (LINQ(1:8) .EQ. 'IMPLICIT') THEN
        IF (MXP .EQ. 0) GOTO 100
        IF (ISTP .EQ. 0) GOTO 110
        IF (IEX .NE. 0) GOTO 140
        K1 = NBLK (J1 + 8, LN, LINE)
        CALL IMPLIC (K1, LN)
        MPT = 1
      ELSEIF (LINQ(1:10) .EQ. 'TYPE ERROR') THEN
        IF (MXP .EQ. 0) GOTO 100
        IF (IEX .NE. 0) GOTO 140
        J1 = NBLK (J1 + 10, LN, LINE)
        LQ = MIN (J1 + 15, LN)
        LINQ = UCASE (LINE(J1:LQ))
        IF (LINQ(1:2) .EQ. 'ON') THEN
          ITE = 1
        ELSEIF (LINQ(1:3) .EQ. 'OFF') THEN
          ITE = 0
        ELSE
          CALL ERRMES (79, 0)
        ENDIF
      ELSEIF (LINQ(1:10) .EQ. 'MIXED MODE') THEN
        IF (MXP .EQ. 0) GOTO 100
        IF (IEX .NE. 0) GOTO 140
        J1 = NBLK (J1 + 10, LN, LINE)
        LQ = MIN (J1 + 15, LN)
        LINQ = UCASE (LINE(J1:LQ))
        IF (LINQ(1:4) .EQ. 'SAFE') THEN
          IMM = 1
        ELSEIF (LINQ(1:4) .EQ. 'FAST') THEN
          IMM = 0
        ELSE
          CALL ERRMES (80, 0)
        ENDIF
      ELSEIF (LINQ(1:16) .EQ. 'OUTPUT PRECISION') THEN
        IF (MXP .EQ. 0) GOTO 100
        IF (IEX .NE. 0) GOTO 140
        NUMX = LINE(J1+16:LN)
        READ (NUMX, '(BN,I16)', ERR = 120) K
        IF (K .LE. 0) GOTO 120
        MPP = K
      ELSEIF (LINQ(1:7) .EQ. 'EPSILON') THEN
        IF (MXP .EQ. 0) GOTO 100
        IF (IEX .NE. 0) GOTO 140
        K1 = NBLK (J1 + 7, LN, LINE)
        I1 = MAX (INDX (K1, LN, 'E', LINE), INDX (K1, LN, 'e', LINE))
        IF (I1 .EQ. 0) CALL ERRMES (81, 1)
        NUMX = LINE(K1:I1-1)
        READ (NUMX, '(F16.0)') T1
        NUMX = LINE(I1+1:LN)
        READ (NUMX, '(BN,I16)') N1
        IF (T1 .LE. 0 .OR. N1 .GE. 0) CALL ERRMES (82, 1)
        T1 = RLT * (N1 + LOG10 (T1))
        N1 = T1 - 1.D0
        T1 = 2.D0 ** (T1 - N1)
        WRITE (NUMY, '(F14.12,"D0")') T1
        I1 = NBLK (1, 16, NUMY)
        LEP(1) = 17 - I1
        EPS(1) = NUMY(I1:16)
        WRITE (NUMY, '(I10)') N1
        I1 = NBLK (1, 10, NUMY)
        LEP(2) = 11 - I1
        EPS(2) = NUMY(I1:10)
      ELSE
C
C   Check for MP type declarative.
C
        IT = NTYPE (J1, LN)
        IF (IT .GE. 8) THEN
          IF (MXP .EQ. 0) GOTO 100
          IF (IEX .NE. 0) GOTO 140
          CALL TYPE (IT, J1, LN)
          MPT = 1
          IF (IT .EQ. 10) KCON(1) = 1
        ELSE
          GOTO 130
        ENDIF
      ENDIF
      GOTO 150
C
 100  CALL ERRMES (83, 0)
      WRITE (6, 1)
 1    FORMAT ('Precision level has not yet been declared.')
      CALL ABRT
C
 110  CALL ERRMES (84, 0)
      WRITE (6, 2)
 2    FORMAT ('Improper placement of MP directive.')
      CALL ABRT
C
 120  CALL ERRMES (85, 0)
      WRITE (6, 3)
 3    FORMAT ('Improper integer constant.')
      CALL ABRT
C
 130  CALL ERRMES (86, 0)
      WRITE (6, 4)
 4    FORMAT ('Unrecognized CMP+ directive.')
      CALL ABRT
C
 140  CALL ERRMES (87, 0)
      WRITE (6, 5)
 5    FORMAT ('A declarative statement may not appear after an',        
     $  ' executable statement.')
      CALL ABRT
C
 150  RETURN
      END
C
      FUNCTION NBLK (K1, K2, LIN)
C
C   This finds the index of the first non-blank character in LIN between
C   positions K1 and K2.  LIN may be of any character type.
C
      CHARACTER*(*) LIN
C
      DO 100 I = K1, K2
        IF (LIN(I:I) .NE. ' ') GOTO 110
 100  CONTINUE
C
      I = 0
 110  NBLK = I
C
      RETURN
      END
C
      FUNCTION NTYPE (K1, K2)
C
C   Identifies type declarations in type statements or implicit statements
C   and repositions pointer one past end of declarative.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*16 LINQ, UCASE
C
      LQ = MIN (K1 + 15, K2)
      LINQ = UCASE (LINE(K1:LQ))
      IF (LINQ(1:7) .EQ. 'INTEGER') THEN
        NTYPE = 1
        K1 = NBLK (K1 + 7, K2, LINE)
        IF (LINE(K1:K1) .EQ. '*') K1 = 1 + NBLK (K1 + 1, K2, LINE)
      ELSEIF (LINQ(1:5) .EQ. 'REAL ' .OR. LINQ(1:5) .EQ. 'REAL*') THEN
        NTYPE = 2
        K1 = NBLK (K1 + 4, K2, LINE)
        IF (LINE(K1:K1) .EQ. '*') THEN
          K1 = NBLK (K1 + 1, K2, LINE)
          IF (LINE(K1:K1) .EQ. '8') NTYPE = 3
          K1 = K1 + 1
        ENDIF
      ELSEIF (LINQ(1:6) .EQ. 'DOUBLE') THEN
        NTYPE = 3
        K1 = NBLK (K1 + 6, K2, LINE)
        LQ = MIN (K1 + 15, K2)
        LINQ = UCASE (LINE(K1:LQ))
        IF (LINQ(1:9) .EQ. 'PRECISION') THEN
          K1 = K1 + 9
        ELSEIF (LINQ(1:7) .EQ. 'COMPLEX') THEN
          NTYPE = 5
          K1 = K1 + 7
        ENDIF
      ELSEIF (LINQ(1:7) .EQ. 'COMPLEX') THEN
        NTYPE = 4
        K1 = NBLK (K1 + 7, K2, LINE)
        IF (LINE(K1:K1) .EQ. '*') THEN
          K1 = NBLK (K1 + 1, K2, LINE)
          IF (LINE(K1:K1+1) .EQ. '16') THEN
            NTYPE = 5
            K1 = K1 + 2
          ELSE
            K1 = K1 + 1
          ENDIF
        ENDIF
      ELSEIF (LINQ(1:9) .EQ. 'CHARACTER') THEN
        NTYPE = 6
        K1 = NBLK (K1 + 9, K2, LINE)
        IF (LINE(K1:K1) .EQ. '*') THEN
          K1 = NBLK (K1 + 1, K2, LINE)
          J3 = INDX (K1, K2, ' ', LINE)
          IF (J3 .NE. 0) THEN
            K1 = J3
          ELSE
            K1 = K2 + 1
          ENDIF
        ENDIF
      ELSEIF (LINQ(1:7) .EQ. 'LOGICAL') THEN
        NTYPE = 7
        K1 = NBLK (K1 + 7, K2, LINE)
        IF (LINE(K1:K1) .EQ. '*') K1 = 1 + NBLK (K1 + 1, K2, LINE)
      ELSEIF (LINQ(1:14) .EQ. 'MULTIP INTEGER') THEN
        NTYPE = 8
        K1 = NBLK (K1 + 14, K2, LINE)
      ELSEIF (LINQ(1:11) .EQ. 'MULTIP REAL') THEN
        NTYPE = 9
        K1 = NBLK (K1 + 11, K2, LINE)
      ELSEIF (LINQ(1:14) .EQ. 'MULTIP COMPLEX') THEN
        NTYPE = 10
        K1 = NBLK (K1 + 14, K2, LINE)
      ELSE
        NTYPE = 0
      ENDIF
C
      RETURN
      END
C
      FUNCTION NUMCON (K1, K2, LA, LINA)
C
C   This parses numeric constants, returning the type of the constant.
C   K1 is the index of the start of the constant, and K2 is the index of
C   the end (an output value).  LA is the index of the last non-blank
C   character in LINA.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINA
      CHARACTER*1 CJ
C
C   IB =  1 if the previous character was a blank, 0 otherwise.
C   ID =  1 if a digit has occurred, 0 otherwise.  Reset to 0 after D or E.
C   IP =  1 if a period has occurred, 0 otherwise.
C   IS =  1 if a sign has occurred, 0 otherwise.  Reset to 0 after D or E.
C   IT =  The type number of the constant (1, 2, 3 or 9).
C   IX =  1 if a D or E has occurred, 0 otherwise.
C
      IB = 0
      ID = 0
      IP = 0
      IS = 0
      IT = 1
      IX = 0
C
      DO 100 J = K1, LA
        CJ = LINA(J:J)
        IF (INDEX (DIG, CJ) .NE. 0) THEN
          ID = 1
          GOTO 100
        ELSEIF (CJ .EQ. '.') THEN
          IF (IP .NE. 0 .OR. IX .NE. 0 .OR. J .EQ. IB + 1) GOTO 110
          IP = 1
          IT = 2
          GOTO 100
        ELSEIF (CJ .EQ. 'D' .OR. CJ .EQ. 'd') THEN
          IF (IX .EQ. 1) CALL ERRMES (88, 1)
          ID = 0
          IS = 0
          IT = 3
          IX = 1
          GOTO 100
        ELSEIF (CJ .EQ. 'E' .OR. CJ .EQ. 'e') THEN
          IF (IX .EQ. 1) CALL ERRMES (89, 1)
          ID = 0
          IS = 0
          IT = 2
          IX = 1
          GOTO 100
        ELSEIF (CJ .EQ. '+' .OR. CJ .EQ. '-') THEN
          IF (ID .NE. 0 .OR. IS .NE. 0) GOTO 110
          IF (IP .NE. 0 .AND. IX .EQ. 0) CALL ERRMES (90, 1)
          IS = 1
          GOTO 100
        ELSEIF (CJ .EQ. ' ') THEN
          IB = J
          GOTO 100
        ELSE
          GOTO 110
        ENDIF
 100  CONTINUE
C
      J = LA + 1
C
C   Numeric constant has been parsed.  Trim any trailing blanks.
C
 110  K2 = J - 1
 120  IF (K2 .GE. K1 .AND. LINA(K2:K2) .EQ. ' ') THEN
        K2 = K2 - 1
        GOTO 120
      ENDIF
      NUMCON = IT
C
      RETURN
      END
C
      SUBROUTINE OUTLIN (IC, LA, LINA)
C
C   This outputs Fortran statements.  If IC is 0, LINA is a comment line up
C   to 80 characters long.  If IC is 1, LINA is a possibly multiline Fortran
C   statement.  If IC is 2, LINA is a possibly multiline Fortran statement that
C   will be output with 'CMP>' at the start of each line.  If IC is 3, LINA is
C   a possibly multiline Fortran statement that is to be output on unit 8
C   instead of 7.
C
      CHARACTER*1600 LINA
C
      IF (IC .EQ. 0) THEN
        I1 = MAX (LA, 1)
        WRITE (11, 1) LINA(1:I1)
 1      FORMAT (A)
      ELSEIF (IC .EQ. 1) THEN
        I1 = MIN (LA, 72)
        WRITE (11, 1) LINA(1:I1)
C
        DO 100 I = 73, LA, 66
          I1 = MIN (I + 65, LA)
          WRITE (11, 2) LINA(I:I1)
 2        FORMAT (5X,'$',A)
 100    CONTINUE
C
      ELSEIF (IC .EQ. 2) THEN
        I1 = MIN (LA, 72)
        WRITE (11, 3) LINA(7:I1)
 3      FORMAT ('CMP>    ',A)
C
        DO 110 I = 73, LA, 66
          I1 = MIN (I + 65, LA)
          WRITE (11, 4) LINA(I:I1)
 4        FORMAT ('CMP>   $',A)
 110    CONTINUE
C
      ELSEIF (IC .EQ. 3) THEN
        I1 = MIN (LA, 72)
        WRITE (12, 1) LINA(1:I1)
C
        DO 120 I = 73, LA, 66
          I1 = MIN (I + 65, LA)
          WRITE (12, 2) LINA(I:I1)
 120    CONTINUE
C
      ENDIF
C
      RETURN
      END
C
      SUBROUTINE PARAM (K1, LN)
C
C   This processes parameter statements.  K1 is the index of the first
C   character after 'PARAMETER'.  LN is the index of the last non-blank
C   character.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINA
      CHARACTER*80 ARG(MAR), ARGX
      DIMENSION ITAR(MAR), LAR(MAR)
C
C   Use FIXSUB to change names of special constants.
C
      K2 = LN
      CALL FIXSUB (K1, K2, LN)
C
C   Use ARLIST to process each separate expression.
C
      J1 = NBLK (K1, LN, LINE)
      IF (LINE(J1:J1) .NE. '(' .OR. LINE(LN:LN) .NE. ')')               
     $  CALL ERRMES (91, 1)
      LA = LN - J1 - 1
      LINA(1:LA) = LINE(J1+1:LN-1)
      CALL ARLIST (12, LA, LINA, NAR, ITAR, LAR, ARG)
      IF (NAR .EQ. 0) CALL ERRMES (92, 1)
      MPA = MPA + NAR
C
C   Make sure that all expressions were assignments.
C
      DO 100 I = 1, NAR
        IF (LAR(I) .NE. 0) CALL ERRMES (93, 1)
 100  CONTINUE
C
      RETURN
      END
C
      SUBROUTINE RDWR (IRW, K1, K2, LN)
C
C   This processes read and write statements, depending on whether IRW is
C   1 or 2.  K1 and K2 and the indices of the parentheses.  LN is the index
C   of the last non-blank character.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINA
      CHARACTER*80 ARG(MAR), ARGK
      CHARACTER*16 UNIT, NAM
      CHARACTER*8 NUMX, NUMY
      DIMENSION ITAR(MAR), LAR(MAR)
      CHARACTER*4 TMP1, TMP2, TMP3, GETMP
C
C   Place the output precision parameter into a character variable.
C
      WRITE (NUMY, '(I8)') MPP
      I1 = NBLK (1, 8, NUMY)
      LX = 9 - I1
      NUMX = NUMY(I1:8)
C
C   Determine unit number.
C
      I1 = INDX (K1, LN, ',', LINE)
      IF (I1 .NE. 0 .AND. I1 .LT. K2) THEN
        I2 = I1
      ELSE
        I2 = K2
      ENDIF
      LUN = I2 - K1 - 1
      UNIT(1:LUN) = LINE(K1+1:I2-1)
C
C   Check if there is a * for the format.
C
      IF (I2 .LT. K2) THEN
        I2 = NBLK (I2 + 1, K2, LINE)
        IF (LINE(I2:K2-1) .NE. '*') THEN
          CALL ERRMES (94, 0)
          WRITE (6, 1)
 1        FORMAT ('This form of read/write statement is not allowed',   
     $      ' with MP variables.')
          CALL ABRT
        ENDIF
      ELSE
C
C   There is no star or format number -- this is a binary I/O statement.
C   Check to make sure there are no parentheses.
C
        I1 = INDX (K2 + 1, LN, '(', LINE)
        IF (I1 .NE. 0) THEN
          CALL ERRMES (95, 0)
          WRITE (6, 2)
 2        FORMAT ('Only entire arrays may be output with binary MP',    
     $      ' I/O.')
          CALL ABRT
        ENDIF
        CALL OUTLIN (1, LN, LINE)
        GOTO 120
      ENDIF
C
C   Form a list of the arguments.
C
      J1 = NBLK (K2 + 1, LN, LINE)
      LA = LN - J1 + 1
      LINA(1:LA) = LINE(J1:LN)
      CALL ARLIST (11, LA, LINA, NAR, ITAR, LAR, ARG)
C
C   Process the statement.
C
      DO 100 K = 1, NAR
        ARGK = ARG(K)
        LK = LAR(K)
        ITK = ITAR(K)
C
C   Set NAM to be the argument without subscripts.
C
        I1 = INDEX (ARGK(1:LK), '(')
        IF (I1 .EQ. 0) THEN
          I2 = LK
        ELSE
          I2 = I1 - 1
        ENDIF
        I2 = MIN (I2, 16)
        NAM = ARGK(1:I2)
C
C   Check if the argument is a constant or a variable.
C
        IF (MAX (INDEX (ALPL, NAM(1:1)), INDEX (ALPU, NAM(1:1))) .GT. 0)
     $    THEN
C
C   Check if the variable has subscripts and if it is dimensioned.
C
          IX = ITAB (0, 0, NAM)
          IF (IX .EQ. 0) THEN
            CALL ERRMES (96, 0)
            WRITE (6, 3) NAM
 3          FORMAT ('This Fortran keyword may not appear in a',         
     $        ' read/write statement: ',A)
            CALL ABRT
          ELSEIF (I1 .EQ. 0 .AND. KDIM(IX) .NE. 0 .AND.                 
     $        KDIM(IX) .NE. -3) THEN
            CALL ERRMES (97, 0)
            WRITE (6, 4) NAM
 4          FORMAT ('Dimensioned variables must be subscripted in this',
     $        ' form of read/write: ',A)
            CALL ABRT
          ENDIF
        ENDIF
C
C   Check if it is read or a write.
C
        IF (IRW .EQ. 1) THEN
          IF (ITK .LT. 8) THEN
C
C   Read an ordinary non-MP variable.
C
            WRITE (11, 5) UNIT(1:LUN), ARGK(1:LK)
 5          FORMAT (6X,'READ (',A,', *) ',A)
          ELSEIF (ITK .NE. 10) THEN
C
C   Read a MPI or MPR variable, possibly on multiple lines.
C
            TMP1 = GETMP (6)
            WRITE (11, 6) UNIT(1:LUN), ARGK(1:LK), TMP1
 6          FORMAT (6X,'CALL MPINP (',A,', ',A,', ',A,')')
          ELSE
C
C   Read a MPC variable.
C
            TMP1 = GETMP (6)
            TMP2 = GETMP (9)
            TMP3 = GETMP (9)
            WRITE (11, 6) UNIT(1:LUN), TMP2, TMP1
            WRITE (11, 6) UNIT(1:LUN), TMP3, TMP1
            WRITE (11, 7) TMP2, TMP3, ARGK(1:LK)
 7          FORMAT (6X,'CALL MPMMPC (',A,', ',A,', MPNW4, ',A,')')
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
          ENDIF
        ELSE
          IF (ITK .EQ. 1) THEN
C
C   Write an integer variable.
C
            WRITE (11, 8) UNIT(1:LUN), ARGK(1:LK)
 8          FORMAT (6X,'WRITE (',A,', ''(I12)'') ',A)
          ELSEIF (ITK .GE. 2 .AND. ITK .LE. 5) THEN
C
C   Write an SP, DP, CO or DC variable.
C
            WRITE (11, 9) UNIT(1:LUN), ARGK(1:LK)
 9          FORMAT (6X,'WRITE (',A,', ''(1P2D25.15)'') ',A)
          ELSEIF (ITK .EQ. 6) THEN
C
C   Write a character variable.
C
            WRITE (11, 10) UNIT(1:LUN), ARGK(1:LK)
 10         FORMAT (6X,'WRITE (',A,', ''(A)'') ',A)
          ELSEIF (ITK .EQ. 7) THEN
C
C   Write a logical variable.
C
            WRITE (11, 11) UNIT(1:LUN), ARGK(1:LK)
 11         FORMAT (6X,'WRITE (',A,', ''(L4)'') ',A)
          ELSEIF (ITK .NE. 10) THEN
C
C   Write a MPI or MPR variable.
C
            TMP1 = GETMP (6)
            WRITE (11, 12) UNIT(1:LUN), ARG(K)(1:LAR(K)), NUMX(1:LX),   
     $        TMP1
 12         FORMAT (6X,'CALL MPOUT (',A,', ',A,', ',A,', ',A,')')
          ELSE
C
C   Read a MPC variable.
C
            TMP1 = GETMP (6)
            TMP2 = GETMP (9)
            TMP3 = GETMP (9)
            WRITE (11, 13) ARGK(1:LK), TMP2, TMP3
 13         FORMAT (6X,'CALL MPMPCM (MPNW4, ',A,', ',A,', ',A,')')
            WRITE (11, 12) UNIT(1:LUN), TMP2, NUMX(1:LX), TMP1
            WRITE (11, 12) UNIT(1:LUN), TMP3, NUMX(1:LX), TMP1
            CALL RLTMP (TMP2)
            CALL RLTMP (TMP3)
          ENDIF
        ENDIF
 100  CONTINUE
C
C   Release any temporaries among the arguments.
C
      DO 110 I = 1, NAR
        LI = LAR(I)
        IF (LI .EQ. 4) THEN
          IF (ARG(I)(1:2) .EQ. 'MP') THEN
            TMP1 = ARG(I)(1:4)
            CALL RLTMP (TMP1)
          ENDIF
        ENDIF
 110  CONTINUE
C
      GOTO 120
C
 120  RETURN
      END
C
      SUBROUTINE RLTMP (TMP)
C
C   This releases temporary variable TMP for future use.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*4 TMP
      CHARACTER*1 CX
C
      READ (TMP, 1, ERR = 110) CX, K
 1    FORMAT (2X,A1,I1)
C
      DO 100 I = 1, NTYP
        IF (CTM(I) .EQ. CX) GOTO 120
 100  CONTINUE
C
 110  CALL ERRMES (98, 0)
      WRITE (6, 2) TMP
 2    FORMAT ('RLTMP could not find temporary variable in table : ',A/  
     $  'Please contact the author.')
      CALL ABRT
C
 120  ITMP(K,I) = 0
C
      RETURN
      END
C
      SUBROUTINE TYPE (ITP, K1, LN)
C
C   This processes type statements by delimiting variable names, inserting in
C   table if required (with types set to ITP) and placing any previously
C   declared MP variables in a separate statement, with dimensions corrected.
C   If ITP = 0, this is a flag that the statement being processed is an
C   external directive, and no types are set.  K1 and LN are the indices of the
C   first (after the type name) and last non-blank characters in the statement.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
      CHARACTER*1600 LINA, LINB
      CHARACTER*16 NAM
      CHARACTER*1 CJ
      CHARACTER*8 DIM1, DIM2, DIMY
C
C  Output the original line as a comment, unless (1) this is a MP directive or
C  (2) this is a non-MP type statement and no MP type directives have yet
C  appeared in this routine.
C
      IF (ITP .LT. 8 .AND. MPT .NE. 0) CALL OUTLIN (2, LN, LINE)
C
C   Form the start of LINA and LINB.
C
      KA = 12
      NA = 0
      LINA(1:KA-1) = '      REAL '
      KB = K1
      NB = 0
      LINB(1:KB-1) = LINE(1:KB-1)
      LINB(KB:KB) = ' '
      KB = KB + 1
C
C   Place the MP dimension into the character variables DIM1 and DIM2.
C
      WRITE (DIMY, '(I8)') MXP + 4
      I1 = NBLK (1, 8, DIMY)
      LD1 = 9 - I1
      DIM1 = DIMY(I1:8)
      WRITE (DIMY, '(I8)') 2 * MXP + 8
      I1 = NBLK (1, 8, DIMY)
      LD2 = 9 - I1
      DIM2 = DIMY(I1:8)
      J1 = K1
C
C   Extract the next character from the line.
C
 100  IF (J1 .GT. LN) GOTO 130
      J1 = NBLK (J1, LN, LINE)
      CJ = LINE(J1:J1)
C
C   Check if it the start of a name.
C
      IF (MAX (INDEX (ALPL, CJ), INDEX (ALPU, CJ)) .NE. 0) THEN
C
        DO 110 J = J1, LN
          CJ = LINE(J:J)
          IF (MAX (INDEX (ALPL, CJ), INDEX (ALPU, CJ)) .NE. 0) GOTO 110
          IF (INDEX (DIG, CJ) .NE. 0 .OR. CJ .EQ. '_') GOTO 110
          IF (INDEX (DEL, CJ) .NE. 0) GOTO 120
          CALL ERRMES (99, 1)
 110    CONTINUE
C
        J = LN + 1
 120    J2 = J - 1
        NAM = LINE(J1:J2)
C
C   Add entry to variable table.  With a few exceptions, it should not already
C   be in the table.  Exceptions: the function name, variables in the argument
C   list, variables in previous MP and conventional explicit type statements,
C   the special constants and the special functions.
C
        IX = ITAB (0, 0, NAM)
        IF (IX .NE. 0 .AND. (ITP .GE. 8 .OR. ITP .GT. 0. AND.           
     $    ITP .LT. 8 .AND. MPLC(IX) .EQ. 1)) THEN
          CALL ERRMES (100, 0)
          WRITE (6, 1) NAM
 1        FORMAT ('This reserved or previously defined name may not',   
     $      ' appear in a type'/'statement: ',A)
          CALL ABRT
        ENDIF
        IX = ITAB (1, ITP, NAM)
        IF (IX .EQ. 0) THEN
          CALL ERRMES (101, 0)
          WRITE (6, 2) NAM
 2        FORMAT ('This Fortran keyword may not appear in a type',      
     $      ' statement: ',A)
          CALL ABRT
        ENDIF
        IF (ITP .EQ. 0 .AND. MPLC(IX) .EQ. 1) KTYP(IX) = 0
        KTP = KTYP(IX)
C
C   If this is a CMP+ directive, there should be no dimensions.
C
        IF (ITP .GE. 8) THEN
          I1 = INDEX (LINE(J2:LN), '(')
          IF (I1 .NE. 0) THEN
            CALL ERRMES (102, 0)
            WRITE (6, 3)
 3          FORMAT ('MP type directives may not specify dimensions.')
            CALL ABRT
          ENDIF
          J1 = J2 + 1
          GOTO 100
        ENDIF
C
C   Check if this is a MP variable with a dimension.  If so, copy it to LINA.
C   If it is a MP variable without a dimension, copy it to neither line.
C
        IF (ITP .NE. 0 .AND. KTP .GE. 8) THEN
          IF (J2 .LT. LN) THEN
            J3 = NBLK (J2 + 1, LN, LINE)
            CJ = LINE(J3:J3)
            IF (CJ .EQ. '(') THEN
              KDEC(IX) = 1
              NA = NA + 1
              LINA(KA:KA+J2-J1) = LINE(J1:J2)
              KA = KA + J2 - J1 + 1
              J1 = J3
C
C   Insert MP dimension as the first dimension.
C
              LINA(KA:KA) = '('
              IF (KTP .LT. 10) THEN
                LINA(KA+1:KA+LD1) = DIM1(1:LD1)
                KA = KA + LD1 + 1
              ELSE
                LINA(KA+1:KA+LD2) = DIM2(1:LD2)
                KA = KA + LD2 + 1
              ENDIF
              KDIM(IX) = 1
              LINA(KA:KA) = ','
              KA = KA + 1
              J2 = MATCH (0, J1 + 1, LN, LINE)
              IF (J2 .EQ. 0) CALL ERRMES (103, 1)
              I1 = ISCAN (J1, J2, LINE)
              IF (I1 .NE. 0) THEN
                CALL ERRMES (104, 0)
                WRITE (6, 4) NAM
 4              FORMAT ('The MP dimension on this variable is not',     
     $            ' allowed: ',A)
                CALL ABRT
              ENDIF
              LINA(KA:KA+J2-J1-1) = LINE(J1+1:J2)
              KA = KA + J2 - J1
              LINA(KA:KA+1) = ', '
              KA = KA + 2
            ENDIF
          ENDIF
C
C   Otherwise this is an ordinary variable -- copy to LINB.
C
        ELSE
          KDEC(IX) = 1
          NB = NB + 1
          LINB(KB:KB+J2-J1) = LINE(J1:J2)
          KB = KB + J2 - J1 + 1
          IF (J1 .LT. LN) THEN
            J3 = NBLK (J2 + 1, LN, LINE)
            CJ = LINE(J3:J3)
            IF (CJ .EQ. '(') THEN
              KDIM(IX) = 1
              LINB(KB:KB) = '('
              KB = KB + 1
              J1 = J3
              J2 = MATCH (0, J1 + 1, LN, LINE)
              I1 = ISCAN (J1, J2, LINE)
              IF (I1 .NE. 0) THEN
                CALL ERRMES (105, 0)
                WRITE (6, 4) NAM
                CALL ABRT
              ENDIF
              LINB(KB:KB+J2-J1-1) = LINE(J1+1:J2)
              KB = KB + J2 - J1
            ENDIF
          ENDIF
          LINB(KB:KB+1) = ', '
          KB = KB + 2
        ENDIF
        J1 = J2 + 1
        GOTO 100
C
C   The only other character that should appear here is a comma.
C
      ELSEIF (CJ .EQ. ',') THEN
        J1 = J1 + 1
        GOTO 100
      ELSE
        CALL ERRMES (106, 1)
      ENDIF
C
C   Output LINA and LINB, provided there is something to output.
C
 130  IF (NA .GT. 0) CALL OUTLIN (1, KA - 3, LINA)
      IF (NB .GT. 0) CALL OUTLIN (1, KB - 3, LINB)
      IF (ITP .LT. 8 .AND. MPT .NE. 0) WRITE (11, 5)
 5    FORMAT ('CMP<')
C
      RETURN
      END
C
      FUNCTION UCASE (NAM)
C
C   This routine returns the character string NAM with upper case alphabetics.
C+
      PARAMETER (MAR = 40, MVAR = 400, MINT = 54, MSUB = 100, NDO = 50, 
     $  NKY = 45, NOP = 15, NSF = 50, NTYP = 10)
      CHARACTER*1600 LINE, SARG
      CHARACTER*26 ALPL, ALPU
      CHARACTER*16 EPS, FNAM, KEYW, SFUN, SNAM, VAR
      CHARACTER*12 DEL
      CHARACTER*10 DIG
      CHARACTER*8 CTP, LOPR, UOPR
      CHARACTER*1 CTM
      COMMON /CMP1/ IEX, IMM, ISTP, ITE, KDO, LCT, LSAR, LSM, MPA, MPP, 
     $  MPT, MSS, MXP, NSUB, NVAR
      COMMON /CMP2/ IDON(NDO), IMPL(26), IMPS(26), ITMP(9,NTYP),        
     $  KTYP(MVAR), KCON(5), KDEC(MVAR), KDIM(MVAR), KSTP(0:MAR,MSUB),  
     $  LVAR(MVAR), KOP(NOP), LOP(NOP), LEP(2), MPLC(MVAR), NARS(MSUB)
      COMMON /CMP3/ ALPL, ALPU, DEL, DIG, FNAM, LINE, SARG
      COMMON /CMP4/ CTM(NTYP), CTP(NTYP), EPS(2), KEYW(NKY), LOPR(NOP), 
     $  SFUN(NSF), SNAM(MSUB), UOPR(NOP), VAR(MVAR)
C+
C
      CHARACTER*16 NAMX, UCASE
      CHARACTER*(*) NAM
      CHARACTER*1 CJ
C
      NAMX = ' '
      LQ = LEN (NAM)
C
      DO 100 J = 1, LQ
        CJ = NAM(J:J)
        I1 = INDEX (ALPL, CJ)
        IF (I1 .GT. 0) THEN
          NAMX(J:J) = ALPU(I1:I1)
        ELSE
          NAMX(J:J) = CJ
        ENDIF
 100  CONTINUE
C
      UCASE = NAMX
      RETURN
      END
