{$M 20000, 0, 650000}
PROGRAM euler(input, output);

USES pfloat, crt;

CONST maxbernoulli=20;

VAR B            : ARRAY [0..maxbernoulli] OF mfloat;
    factorial    : ARRAY [0..2*maxbernoulli] OF mfloat;
    R1,R2,delta  : mfloat;
    maxerr,gamma : mfloat;
    remainder    : mfloat;
    h1,h2,R,S,fac: mfloat;
    m,k,i,n,lu   : integer;

BEGIN
  clrscr;
  writeln;
  writeln;
  writeln('          Calculation of Euler''s or Mascheroni''s constant â');
  writeln('          =================================================');
  writeln;
  writeln('                              Ú  n   1            ¿');
  writeln('                    â =  lim  ³  ä  ÄÄÄ   - ln(n) ³');
  writeln('                       n -> ì À i=1  i            Ù');
  writeln;
  m := 256;  { increase, if necessary }
  ldtomf(gamma,0);
  FOR i := 1 TO m-1 DO
  BEGIN
    ldtomf(h1,1);
    divi(h1,i);
    addm(gamma,h1);
  END;
  ldtomf(h1,m);
  logm(h1);
  subm(gamma,h1);
  ldtomf(h1,1);
  divi(h1,2*m);
  addm(gamma,h1);
  ldtomf(maxerr,1E4000);
  ldtomf(B[0],1);
  ldtomf(factorial[0],1);
  ldtomf(factorial[1],1);
  FOR k := 1 TO maxbernoulli - 1 DO
  BEGIN
    equm(factorial[2*k],factorial[2*k-1]);
    multi(factorial[2*k],2*k);
    equm(factorial[2*k+1],factorial[2*k]);
    multi(factorial[2*k+1],2*k+1);
    ldtomf(B[k],0.5);
    FOR i := 0 TO k - 1 DO
    BEGIN
      equm(h1,factorial[2*k]);
      multm(h1,B[i]);
      equm(h2,factorial[2*i]);
      multm(h2,factorial[2*k+1-2*i]);
      divm(h1,h2);
      subm(B[k],h1);
    END;
    { R1 / remainder > 1 ... upper bound }
    equm(R1,B[k]);
    ldtomf(h1,m-1);
    powi(h1,-2*k);
    multm(R1,h1);
    divi(R1,2*k);
    { R2 / remainder < 1 ... lower bound }
    equm(R2,B[k]);
    ldtomf(h1,m+1);
    powi(h1,-2*k);
    multm(R2,h1);
    divi(R2,2*k);
    equm(delta,R1);
    subm(delta,R2);
    fabsm(delta);
    divi(delta,2);
    IF gtm(maxerr,delta) THEN
    BEGIN
      equm(maxerr,delta);
      equm(remainder,R1);
      addm(remainder,R2);
      divi(remainder,2);
      IF (k > 1) THEN
      BEGIN
        ldtomf(h1,m);
        powi(h1,-2*(k-1));
        multm(h1,B[k-1]);
        divi(h1,2*(k-1));
	addm(gamma,h1);
      END;
    END
    ELSE break;
  END;
  addm(gamma,remainder);
  writeln;
  writeln('gamma = ');
  writeln(mftostr(gamma,100,'.77F'));
  writeln;
  write('Error of the calculation due to termination of the series: ');
  writeln(mftostr(maxerr,100,'.5E'));
  { Second method for the calculation: }
  { Method of Brent                    }
  n := 62;    { this value is optimal for 77 digit arithmetic }
  { estimate of R }
  ldtomf(R,1);
  ldtomf(fac,1);
  FOR k := 1 TO n - 2 DO
  BEGIN
    multi(fac,k);
    divi(fac,-n);
    addm(R,fac);
  END;
  ldtomf(h1,-n);
  expm(h1);
  divi(h1,n);
  multm(R,h1);
  { estimate of S }
  lu := trunc(4.3191*n);
  ldtomf(fac,-1);
  ldtomf(S,0);
  FOR k := 1 TO lu DO
  BEGIN
    multi(fac,-n);
    divi(fac,k);
    equm(h1,fac);
    divi(h1,k);
    addm(S,h1);
  END;
  ldtomf(gamma,n);
  logm(gamma);
  negm(gamma);
  addm(gamma,S);
  subm(gamma,R);
  writeln;
  writeln;
  writeln('Second Method (less accurate):');
  writeln;
  writeln('gamma = ');
  writeln(mftostr(gamma,100,'.77F'));
END.