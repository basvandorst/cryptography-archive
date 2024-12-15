{$M 20000, 0, 650000}
PROGRAM polar(input, output);
{ *** This program demonstrates the conversion of cartesian coordinates *** }
{ *** to polar coordinates with mfloat numbers                          *** }

USES pfloat, crt;

VAR x, y, r, phi, xr, yr : mfloat;
    ch                   : string;

BEGIN
  Setmantissawords(16);
  ClrScr;
  writeln('          Conversion cartesian coordinates - polar coordinates');
  writeln('          ====================================================');
  writeln;
  writeln;
  writeln('Cartesian coordinates:');
  REPEAT
    write('x = ');
    readln(ch);
  UNTIL strtomf(x, ch) = 0;
  REPEAT
    write('y = ');
    readln(ch);
  UNTIL strtomf(y, ch) = 0;
  writeln;
  writeln('x   = ', mftoa(x,50));
  writeln('y   = ', mftoa(y,50));
  writeln;
  writeln('Conversion to polar coordinates:');
  equm(r,x);
  hypotm(r,y);
  equm(phi,y);
  atan2m(phi,x);
  writeln;
  writeln('r   = ', mftoa(r,50));
  writeln('phi = ', mftoa(phi,50));
  writeln;
  writeln;
  writeln('Conversion back to cartesian coordinates:');
  equm(xr, phi);
  cossinm(xr, yr);
  multm(xr, r);
  multm(yr, r);
  writeln;
  writeln('x   = ', mftoa(xr,50));
  writeln('y   = ', mftoa(yr,50));
END.