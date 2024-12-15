{$M 20000, 0, 650000}
PROGRAM pi(input, output);
{ *** Here are some methods demonstrated to calculate the number pi *** }

USES pfloat, crt;

VAR a,b,c, bound : mfloat;
    i            : integer;

BEGIN
  ClrScr;
  Setmantissawords(16);
  Getonem(bound);
  ldexpm(bound, -16 * Getmantissawords - 16);
  writeln('                            Calculation of PI');
  writeln('                            =================');
  writeln;
  writeln;
  writeln('PI = 16 * arctan(1 / 5) - 4 * arctan(1 / 239) :');
  Getonem(a);
  multi(a,16);
  divi(a,5);
  equm(b,a);
  i := 0;
  REPEAT
    i := i + 1;
    divi(b, 25);
    equm(c, b);
    divi(c, 2 * i + 1);
    IF odd(i) THEN
      subm(a, c)
    ELSE
      addm(a, c);
  UNTIL gtm(bound, c);
  getonem(b);
  multi(b, 4);
  divi(b, 239);
  subm(a, b);
  i := 0;
  REPEAT
    i := i + 1;
    divi(b, 239);
    divi(b, 239);
    equm(c, b);
    divi(c, 2 * i + 1);
    IF odd(i) THEN
      addm(a, c)
    ELSE
      subm(a, c);
  UNTIL gtm(bound, c);
  writeln(mftostr(a,80, '.F77'));
  writeln;
  writeln('*******************************************************************************');
  writeln;
  writeln('PI = 48 * arctan(1 / 18)  + 32 * arctan(1 / 57) - 20 * arctan(1 / 239) :');
  getonem(a);
  multi(a, 8);
  divi(a, 3);
  equm(b, a);
  i := 0;
  REPEAT
    i := i + 1;
    divi(b, 324);
    equm(c, b);
    divi(c, 2 * i + 1);
    IF odd(i) THEN
      subm(a, c)
    ELSE
      addm(a, c);
  UNTIL gtm(bound, c);
  getonem(b);
  multi(b, 32);
  divi(b, 57);
  addm(a, b);
  i := 0;
  REPEAT
    i := i + 1;
    divi(b, 3249);
    equm(c, b);
    divi(c, 2 * i + 1);
    IF odd(i) THEN
      subm(a, c)
    ELSE
      addm(a, c);
  UNTIL gtm(bound, c);
  getonem(b);
  multi(b, 20);
  divi(b, 239);
  subm(a, b);
  i := 0;
  REPEAT
    i := i + 1;
    divi(b, 239);
    divi(b, 239);
    equm(c, b);
    divi(c, 2 * i + 1);
    IF odd(i) THEN
      addm(a, c)
    ELSE
      subm(a, c);
  UNTIL gtm(bound, c);
  writeln(mftostr(a,80, '.F77'));
  writeln;
  writeln('*******************************************************************************');
  writeln;
  writeln('PI = 6 * arctan(1 / sqrt(3)) :');
  Getonem(a);
  multi(a, 3);
  sqrtm(a);
  multi(a, 2);
  equm(b, a);
  i := 0;
  REPEAT
    i := i + 1;
    divi(b, 3);
    equm(c, b);
    divi(c, 2 * i + 1);
    IF odd(i) THEN
      subm(a, c)
    ELSE
      addm(a, c);
  UNTIL gtm(bound, c);
  writeln(mftostr(a,80, '.F77'));
  writeln;
  writeln('*******************************************************************************');
  writeln;
  writeln('Internal PI :');
  Getpim(a);
  writeln(mftostr(a,80, '.F77'));
  writeln;
END.