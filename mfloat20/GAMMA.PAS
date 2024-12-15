{$M 20000, 0, 650000}
PROGRAM gammafunction(input, output);

USES pfloat, crt;

CONST maxbernoulli = 22;
      mingamma = 220;

{------------------------------------------------------------------------}

PROCEDURE realinput(VAR x : mfloat; name : string);

VAR ch : string;

BEGIN
  REPEAT
    write(name);
    readln(ch);
    IF strtomf(x,ch) = 0 THEN break;
    writeln('Error in input!');
    writeln;
  UNTIL false;
END;

{-----------------------------------------------------------------------}

VAR B            : ARRAY [0..maxbernoulli] OF mfloat;
    factorial    : ARRAY [0..2*maxbernoulli] OF mfloat;
    error,maxerr : mfloat;
    z,gamma,fak  : mfloat;
    h1,h2        : mfloat;
    k,i          : integer;

BEGIN
  clrscr;
  writeln;
  writeln;
  writeln('          Calculation of the Gamma Function â(z)');
  writeln('          ======================================');
  writeln;
  writeln('                           ì');
  writeln('                           ô  z-1  -t');
  writeln('                    â(z) = ³ t    e   dt');
  writeln('                           õ');
  writeln('                           0');
  writeln;
  realinput(z,'z = ');
  ldtomf(fak,1);
  ldtomf(h2,mingamma);
  ldtomf(h1,1);
  WHILE gtm(h2,z) DO
  BEGIN
    multm(fak,z);
    addm(z,h1);
  END;
  inversm(fak);
  equm(gamma,z);
  ldtomf(h1,0.5);
  subm(gamma,h1);
  equm(h2,z);
  logm(h2);
  multm(gamma,h2);
  subm(gamma,z);
  getpim(h2);
  multi(h2,2);
  logm(h2);
  divi(h2,2);
  addm(gamma,h2);
  ldtomf(maxerr,1E300);
  ldtomf(B[0],1);
  ldtomf(factorial[0],1);
  ldtomf(factorial[1],1);
  FOR k := 1 TO  maxbernoulli - 1 DO
  BEGIN
    equm(factorial[2*k],factorial[2*k-1]);
    multi(factorial[2*k],2*k);
    equm(factorial[2*k+1],factorial[2*k]);
    multi(factorial[2*k+1],2*k+1);
    ldtomf(B[k],0.5);
    FOR i := 0 TO  k - 1 DO
    BEGIN
      equm(h1,factorial[2*i]);
      multm(h1,factorial[2*k+1-2*i]);
      equm(h2,factorial[2*k]);
      multm(h2,B[i]);
      divm(h2,h1);
      subm(B[k],h2);
    END;
    equm(error,B[k]);
    fabsm(error);
    divi(error,2*k);
    divi(error,2*k-1);
    equm(h1,z);
    fabsm(h1);
    powi(h1,1-2*k);
    multm(error,h1);
    IF gtm(maxerr,error) THEN
    BEGIN
      equm(maxerr,error);
      IF k > 1 THEN
      BEGIN
        equm(h1,B[k-1]);
        divi(h1,2*k-2);
	divi(h1,2*k-3);
        equm(h2,z);
	powi(h2,3-2*k);
        multm(h1,h2);
	addm(gamma,h1);
      END;
    END
    ELSE break;
  END;
  expm(gamma);
  multm(gamma,fak);
  writeln('gamma = ');
  writeln(mftostr(gamma,100,'.70E'));
  writeln;
  write('Relative Error of the calculation due to termination of the series: ');
  writeln(mftostr(maxerr,100,'.5E'));
END.
