{$M 20000, 0, 650000}
PROGRAM Pi_AGM(input,output);
{  This test program shows the calculation of pi using Arithmetic-Geometric }
{  Mean.                                                                    }

USES pfloat, crt;

VAR y,a,z    : mfloat;
    h1,h2,h3 : mfloat;
    n,m      : integer;

BEGIN
  clrscr;
  writeln;
  writeln;
  writeln('              Calculation of Pi using Geometric-Arithmetic Mean');
  writeln('              =================================================');
  writeln;
  writeln;
  writeln;
  writeln('Idea from');
  writeln('Fausto A. A. Barbuto          Email: BJ06@C53000.PETROBRAS.ANRJ.BR');
  writeln('and');
  writeln('Jay R. Hill, PhD.             Email: HillJr@Jupiter.SAIC.Com');
  writeln;
  writeln;
  ldtomf(y,0.5);
  sqrtm(y);
  ldtomf(a,0.5);
  m := 1;
  FOR n := 0 TO 8 DO
  BEGIN
    m := 2*m;
    ldtomf(h1,1);
    ldtomf(h2,1);
    equm(h3,y);
    sqrm(h3);
    subm(h2,h3);
    sqrtm(h2);
    addm(h1,h2);
    divm(y,h1);
    sqrm(y);
    equm(h1,y);
    ldtomf(h2,1);
    addm(h1,h2);
    sqrm(h1);
    multm(a,h1);
    equm(h3,y);
    multi(h3,m);
    subm(a,h3);
    equm(h1,a);
    inversm(h1);
    writeln('Pi(',n,') = ',mftostr(h1,100,'.69F'));
  END;
  writeln;
  writeln('Pi = ');
  writeln(mftostr(h1,100,'.77F'));
END.