{$M 20000, 0, 650000}
PROGRAM bessel(input, output);
{ *** This test program shows the advantages of mfloat numbers. *** }
{ *** The bessel function J0(x) is calculated by the series.    *** }

USES pfloat, crt;

{-------------------------------------------------------------------------}

FUNCTION J0(x : extended) : extended;

CONST epsi = 1e-20;

VAR sum, sqrx, prod, mepsi : mfloat;
    i                      : integer;

BEGIN
  ldtomf(mepsi, epsi);
  ldtomf(sqrx, x);
  ldexpm(sqrx, -1);
  sqrm(sqrx);
  Getonem(sum);
  Getonem(prod);
  i := 0;
  REPEAT
    i := i + 1;
    multm(prod, sqrx);
    divi(prod, i);
    divi(prod, i);
    IF odd(i) THEN
      subm(sum, prod)
    ELSE
      addm(sum, prod);
  UNTIL gtm(mepsi, prod);
  J0 := mftold(sum);
END;

{-------------------------------------------------------------------------}

FUNCTION J0x(x : extended) : extended;

CONST epsi = 1e-20;

VAR sum, sqrx, prod : extended;
    i               : integer;

BEGIN
  sqrx := sqr(x / 2);
  sum := 1;
  prod := 1;
  i := 0;
  REPEAT
    i := i + 1;
    prod := prod * sqrx / i / i;
    IF odd(i) THEN
      sum := sum - prod
    ELSE
      sum := sum + prod
  UNTIL epsi > prod;
  J0x := sum;
END;

{-------------------------------------------------------------------------}

VAR accuracy : integer;
    x        : extended;

BEGIN
  ClrScr;
  writeln('              Calculation of the bessel function J0(x)');
  writeln('              ========================================');
  writeln;
  x := 100;
  writeln('mantissa words        result     (x = ', x:5:1, ')');
  writeln;
  FOR accuracy := 1 TO 15 DO
  BEGIN
    Setmantissawords(accuracy);
    writeln(accuracy:3, '                J0(',x:5:1,') = ', J0(x));
  END;
  writeln('IEEE arithmetic    J0(',x:5:1,') = ', J0x(x));
  writeln;
  writeln('The accuracy of 12 mantissa words is needed to get a good result!');
END.