UNIT pfloat;
{ *** Procedures for calculation with mfloat numbers *** }

INTERFACE

{$F+}

{----------------------------------------------------------------------------}
{ mfloat types }
{----------------------------------------------------------------------------}

CONST MfloatWords = 17;
TYPE  mfloat = ARRAY[0..MfloatWords-1] OF integer;

{----------------------------------------------------------------------------}
{ mfloat basic functions }
{----------------------------------------------------------------------------}

PROCEDURE SetMantissawords(number : integer);
FUNCTION  GetMantissawords : integer;
PROCEDURE ResetError;
FUNCTION  GetError : boolean;
PROCEDURE mfloatError(Nr : integer);

PROCEDURE equm(   VAR a, b : mfloat);           { *** a <-- b            *** }
PROCEDURE addm(   VAR a, b : mfloat);           { *** a <-- a + b        *** }
PROCEDURE subm(   VAR a, b : mfloat);           { *** a <-- a - b        *** }
PROCEDURE multm(  VAR a, b : mfloat);           { *** a <-- a * b        *** }
PROCEDURE divm(   VAR a, b : mfloat);           { *** a <-- a / b        *** }
PROCEDURE multi(  VAR a : mfloat; b : integer); { *** a <-- a * b        *** }
PROCEDURE divi(   VAR a : mfloat; b : integer); { *** a <-- a / b        *** }
PROCEDURE inversm(VAR a : mfloat);              { *** a <-- 1 / a        *** }
PROCEDURE negm(   VAR a : mfloat);              { *** a <-  - a          *** }
FUNCTION  eqZero( VAR a : mfloat) : boolean;    { *** eqZero <-- a = 0   *** }
FUNCTION  gtZero( VAR a : mfloat) : boolean;    { *** gtZero <-- a > 0   *** }
FUNCTION  geZero( VAR a : mfloat) : boolean;    { *** geZero <-- a >= 0  *** }
FUNCTION  gtm(    VAR a, b : mfloat) : boolean; { *** gtm <-- a > b      *** }
FUNCTION  eqm(    VAR a, b : mfloat) : boolean; { *** eqm <-- a = b      *** }
PROCEDURE GetZerom(VAR a : mfloat);             { *** a <- 0             *** }
PROCEDURE GetOnem(VAR a : mfloat);              { *** a <- 1             *** }
PROCEDURE GetPim( VAR a : mfloat);              { *** a <- pi            *** }
PROCEDURE GetLn2m(VAR a : mfloat);              { *** a <- ln(2)         *** }
PROCEDURE GetLn10m(VAR a : mfloat);             { *** a <- ln(10)        *** }
FUNCTION  strtomf(VAR a : mfloat;               { *** a <-- string       *** }
                      b : string)
                        : integer;
FUNCTION  mftoa(  VAR a : mfloat;               { *** string <-- a       *** }
                      len : integer)            { !!! compare with C         }
                          : string;
FUNCTION  mftostr(VAR a : mfloat;               { *** string <-- a       *** }
                      len : integer;            { !!! compare with C         }
                      format : string)
                          : string;
FUNCTION  MfToD(  VAR a : mfloat) : double;     { *** MfToD <- a         *** }
FUNCTION  MfToLd( VAR a : mfloat) : extended;   { *** MfToLd <- a        *** }
PROCEDURE DToMf(  VAR a : mfloat; b : double);  { *** a <- b             *** }
PROCEDURE LdToMf( VAR a : mfloat; b : extended);{ *** a <- b             *** }
PROCEDURE IntToMf(VAR a : mfloat; b : integer); { *** a <- b             *** }

{----------------------------------------------------------------------------}
{ standard functions (Borland C: MATH.H) }
{----------------------------------------------------------------------------}

PROCEDURE acosm(  VAR a : mfloat);              { *** a <- arccos(a)     *** }
PROCEDURE asinm(  VAR a : mfloat);              { *** a <- arcsin(a)     *** }
PROCEDURE atanm(  VAR a : mfloat);              { *** a <- arctan(a)     *** }
PROCEDURE atan2m( VAR a, b : mfloat);           { *** a <- atan2(a, b)   *** }
{         atof                                        see strtomf            }
PROCEDURE ceilm(  VAR a : mfloat);              { *** a <-- ceil(a)      *** }
PROCEDURE cosm(   VAR a : mfloat);              { *** a <- cos(a)        *** }
PROCEDURE coshm(  VAR a : mfloat);              { *** a <- cosh(a)       *** }
PROCEDURE expm(   VAR a : mfloat);              { *** a <- exp(a)        *** }
PROCEDURE fabsm(  VAR a : mfloat);              { *** a <-- fabs(a)      *** }
PROCEDURE floorm( VAR a : mfloat);              { *** a <-- floor(a)     *** }
PROCEDURE fmodm(  VAR a, b : mfloat);           { *** a <- fmod(a,b)     *** }
PROCEDURE frexpm( VAR a : mfloat;
                  VAR b : integer);             { *** a <- frexp(a,b)    *** }
PROCEDURE hypotm( VAR a, b : mfloat);           { *** a <- hypot(a,b)    *** }
PROCEDURE ldexpm( VAR a : mfloat; b : integer); { *** a <- ldexp(a,b)    *** }
PROCEDURE logm(   VAR a : mfloat);              { *** a <- ln(a)         *** }
PROCEDURE log10m( VAR a : mfloat);              { *** a <- log10(a)      *** }
PROCEDURE modfm(  VAR a, b : mfloat);           { *** a, b <- modf(a)    *** }
PROCEDURE powm(   VAR a, b : mfloat);           { *** a <- a^b           *** }
PROCEDURE powi(   VAR a : mfloat; b : integer); { *** a <- a^b           *** }
PROCEDURE pow10m( VAR a : mfloat; b : integer); { *** a <- 10^b          *** }
PROCEDURE sinm(   VAR a : mfloat);              { *** a <- sin(a)        *** }
PROCEDURE sinhm(  VAR a : mfloat);              { *** a <- sinh(a)       *** }
PROCEDURE sqrtm(  VAR a : mfloat);              { *** a <- sqrt(a)       *** }
PROCEDURE tanm(   VAR a : mfloat);              { *** a <- tan(a)        *** }
PROCEDURE tanhm(  VAR a : mfloat);              { *** a <- tanh(a)       *** }

{----------------------------------------------------------------------------}
{ extended standard functions }
{----------------------------------------------------------------------------}

PROCEDURE acoshm( VAR a : mfloat);              { *** a <- arcosh(a)     *** }
PROCEDURE acotm(  VAR a : mfloat);              { *** a <- arccot(a)     *** }
PROCEDURE acothm( VAR a : mfloat);              { *** a <- arcoth(a)     *** }
PROCEDURE asinhm( VAR a : mfloat);              { *** a <- arsinh(a)     *** }
PROCEDURE atanhm( VAR a : mfloat);              { *** a <- artanh(a)     *** }
PROCEDURE cossinm(VAR a,b : mfloat);      { *** a <- cos(a), b <- sin(a) *** }
PROCEDURE cotm(   VAR a : mfloat);              { *** a <- cot(a)        *** }
PROCEDURE cothm(  VAR a : mfloat);              { *** a <- coth(a)       *** }
PROCEDURE exp10m( VAR a : mfloat);              { *** a <- 10 ** a       *** }
PROCEDURE sqrm(   VAR a : mfloat);              { *** a <- sqr(a)        *** }
PROCEDURE truncm( VAR a : mfloat);              { *** a <-- trunc(a)     *** }

{----------------------------------------------------------------------------}

CONST useInterruptAtError : boolean = true;

IMPLEMENTATION

{$L mfloata.obj}
{$L mfloatb.obj}

{----------------------------------------------------------------------------}
{ initialized static variables }
{----------------------------------------------------------------------------}

CONST mantissawords      : integer = 1;
      calculationerror   : boolean = false;
      use386instructions : boolean = false;

{----------------------------------------------------------------------------}
{ externals }
{----------------------------------------------------------------------------}

{ mfloat basic functions }
PROCEDURE MfloatTest80386;                      EXTERNAL;
PROCEDURE SetMantissawords(number : integer);   EXTERNAL;
FUNCTION  GetMantissawords : integer;           EXTERNAL;
PROCEDURE equm(   VAR a, b : mfloat);           EXTERNAL;
PROCEDURE addm(   VAR a, b : mfloat);           EXTERNAL;
PROCEDURE subm(   VAR a, b : mfloat);           EXTERNAL;
PROCEDURE multm(  VAR a, b : mfloat);           EXTERNAL;
PROCEDURE divm(   VAR a, b : mfloat);           EXTERNAL;
PROCEDURE multi(  VAR a : mfloat; b : integer); EXTERNAL;
PROCEDURE divi(   VAR a : mfloat; b : integer); EXTERNAL;
PROCEDURE inversm(VAR a : mfloat);              EXTERNAL;
PROCEDURE negm(   VAR a : mfloat);              EXTERNAL;
FUNCTION  eqZero( VAR a : mfloat) : boolean;    EXTERNAL;
FUNCTION  gtZero( VAR a : mfloat) : boolean;    EXTERNAL;
FUNCTION  geZero( VAR a : mfloat) : boolean;    EXTERNAL;
FUNCTION  gtm(    VAR a, b : mfloat) : boolean; EXTERNAL;
FUNCTION  eqm(    VAR a, b : mfloat) : boolean; EXTERNAL;
PROCEDURE GetZerom(VAR a : mfloat);             EXTERNAL;
PROCEDURE GetOnem(VAR a : mfloat);              EXTERNAL;
PROCEDURE GetPim( VAR a : mfloat);              EXTERNAL;
PROCEDURE GetLn2m(VAR a : mfloat);              EXTERNAL;
PROCEDURE GetLn10m(VAR a : mfloat);             EXTERNAL;
PROCEDURE DToMf(  VAR a : mfloat; b : double);  EXTERNAL;
PROCEDURE LdToMf( VAR a : mfloat; b : extended);EXTERNAL;

{ standard functions }
PROCEDURE acosm(  VAR a : mfloat);              EXTERNAL;
PROCEDURE asinm(  VAR a : mfloat);              EXTERNAL;
PROCEDURE atanm(  VAR a : mfloat);              EXTERNAL;
PROCEDURE atan2m( VAR a, b : mfloat);           EXTERNAL;
PROCEDURE ceilm(  VAR a : mfloat);              EXTERNAL;
PROCEDURE cosm(   VAR a : mfloat);              EXTERNAL;
PROCEDURE coshm(  VAR a : mfloat);              EXTERNAL;
PROCEDURE expm(   VAR a : mfloat);              EXTERNAL;
PROCEDURE fabsm(  VAR a : mfloat);              EXTERNAL;
PROCEDURE floorm( VAR a : mfloat);              EXTERNAL;
PROCEDURE fmodm(  VAR a, b : mfloat);           EXTERNAL;
PROCEDURE frexpm( VAR a : mfloat;
                  VAR b : integer);             EXTERNAL;
PROCEDURE hypotm( VAR a, b : mfloat);           EXTERNAL;
PROCEDURE ldexpm( VAR a : mfloat; b : integer); EXTERNAL;
PROCEDURE logm(   VAR a : mfloat);              EXTERNAL;
PROCEDURE log10m( VAR a : mfloat);              EXTERNAL;
PROCEDURE modfm(  VAR a, b : mfloat);           EXTERNAL;
PROCEDURE powm(   VAR a, b : mfloat);           EXTERNAL;
PROCEDURE powi(   VAR a : mfloat; b : integer); EXTERNAL;
PROCEDURE pow10m( VAR a : mfloat; b : integer); EXTERNAL;
PROCEDURE sinm(   VAR a : mfloat);              EXTERNAL;
PROCEDURE sinhm(  VAR a : mfloat);              EXTERNAL;
PROCEDURE sqrtm(  VAR a : mfloat);              EXTERNAL;
PROCEDURE tanm(   VAR a : mfloat);              EXTERNAL;
PROCEDURE tanhm(  VAR a : mfloat);              EXTERNAL;
{ extended standard functions }
PROCEDURE acoshm( VAR a : mfloat);              EXTERNAL;
PROCEDURE acotm(  VAR a : mfloat);              EXTERNAL;
PROCEDURE acothm( VAR a : mfloat);              EXTERNAL;
PROCEDURE asinhm( VAR a : mfloat);              EXTERNAL;
PROCEDURE atanhm( VAR a : mfloat);              EXTERNAL;
PROCEDURE cossinm(VAR a,b : mfloat);            EXTERNAL;
PROCEDURE cotm(   VAR a : mfloat);              EXTERNAL;
PROCEDURE cothm(  VAR a : mfloat);              EXTERNAL;
PROCEDURE exp10m( VAR a : mfloat);              EXTERNAL;
PROCEDURE sqrm(   VAR a : mfloat);              EXTERNAL;
PROCEDURE truncm( VAR a : mfloat);              EXTERNAL;
{ internal functions }
PROCEDURE SetMantissawords_(number : integer);  EXTERNAL;
PROCEDURE mftostr_(VAR str;
                  VAR a : mfloat;
                  VAR bound : mfloat;
                  VAR len : integer;
                  VAR format);                  EXTERNAL;
FUNCTION strtomf_(VAR a : mfloat; VAR bound : mfloat; VAR b;
                  len : integer) : integer;     EXTERNAL;
PROCEDURE MfToD_( VAR a : double; VAR b : mfloat);  EXTERNAL;
PROCEDURE MfToLd_(VAR a : extended; VAR b : mfloat);EXTERNAL;
PROCEDURE IntToMf(VAR a : mfloat; b : integer);     EXTERNAL;

{----------------------------------------------------------------------------}

PROCEDURE mfloatError(Nr : integer);

BEGIN
  calculationerror := true;
  IF useInterruptAtError THEN
  BEGIN
    writeln;
    writeln('MFLOAT-error:');
    CASE Nr OF
      1  : writeln('divide by zero');
      2  : writeln('floating point overflow');
      3  : writeln('sqrt DOMAIN error');
      4  : writeln('log DOMAIN error');
      5  : writeln('log SINGULARITY error');
      6  : writeln('log10 DOMAIN error');
      7  : writeln('log10 SINGULARITY error');
      8  : writeln('tan SINGULARITY error');
      9  : writeln('cot SINGULARITY error');
      10 : writeln('asin DOMAIN error');
      11 : writeln('acos DOMAIN error');
      12 : writeln('atan SINGULARITY error');
      13 : writeln('acot SINGULARITY error');
      14 : writeln('atan2 DOMAIN error');
      15 : writeln('tanh SINGULARITY error');
      16 : writeln('coth SINGULARITY error');
      17 : writeln('acosh DOMAIN error');
      18 : writeln('atanh DOMAIN error');
      19 : writeln('atanh SINGULARITY error');
      20 : writeln('acoth DOMAIN error');
      21 : writeln('acoth SINGULARITY error');
      22 : writeln('pow DOMAIN error');
      23 : writeln('pow SINGULARITY error');
      24 : writeln('error in sine series');
      25 : writeln('error in sine hyperbolic series');
      26 : writeln('error in arctan series');
      27 : writeln('error in Artanh series');
      28 : writeln('error at conversion of number-formats');
      ELSE writeln('unknown');
    END;
    halt(1);
  END;
END;

{----------------------------------------------------------------------------}

PROCEDURE mfloatStackOverflow; FAR;

BEGIN
  writeln;
  writeln('MFLOAT-error:');
  writeln('stack overflow');
  halt(1);
END;

{----------------------------------------------------------------------------}

PROCEDURE SetMantissawords(number : integer);

BEGIN
  IF number > MfloatWords - 1 THEN
    number := MfloatWords-1;
  SetMantissawords_(number);
END;

{----------------------------------------------------------------------------}

PROCEDURE ResetError;

BEGIN
  calculationerror := false;
END;

{----------------------------------------------------------------------------}

FUNCTION GetError : boolean;

BEGIN
  GetError := calculationerror;
END;

{----------------------------------------------------------------------------}

FUNCTION  strtomf(VAR a : mfloat; b : string) : integer;

VAR bnd : mfloat;  { Here only dummy. }

begin
  strtomf := strtomf_(a,bnd,b[1],ord(b[0]));
end;

{----------------------------------------------------------------------------}

FUNCTION  mftoa(VAR a : mfloat;               { *** string <-- a *** }
                len   : integer) : string;    { !!! compare with C }

CONST format : string[8] = '.32767F'+#0;
VAR tmp   : string;
    bound : mfloat;

BEGIN
  bound[0] := 0;       { *** No error assumed *** }
  IF len > 255 THEN len := 255;
  mftostr_(tmp[1],a,bound,len,format[1]);
  tmp[0] := chr(len);
  mftoa := tmp;
END;

{----------------------------------------------------------------------------}

FUNCTION  mftostr(VAR a : mfloat; len : integer; format : string) : string;

VAR tmp   : string;
    bound : mfloat;

begin
  bound[0] := 0;       { *** No error assumed *** }
  IF len > 255 THEN
    len := 255;
  IF length(format) = 255 THEN
    format[255] := #0
  ELSE
    format[length(format)+1] := #0;
  mftostr_(tmp[1],a,bound,len,format[1]);
  tmp[0] := chr(len);
  mftostr := tmp;
END;

{----------------------------------------------------------------------------}

FUNCTION  MfToD(VAR a : mfloat) : double;

VAR tmp : double;

BEGIN
  MfToD_(tmp,a);
  MfToD := tmp;
END;

{----------------------------------------------------------------------------}

FUNCTION  MfToLd(VAR a : mfloat) : extended;   { *** MfToLd <- a *** }

VAR tmp : extended;

BEGIN
  MfToLd_(tmp,a);
  MfToLd := tmp;
END;

{----------------------------------------------------------------------------}

BEGIN
  Setmantissawords(MfloatWords-1); { *** use maximum precision *** }
  MfloatTest80386;
END.
