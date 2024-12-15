program ArFcnTab;    {Table of arithmetic functions}
{$N+,E+}
uses crt, printer;

const
limit        = 1000000000;
PrinterLimit = 500;                {maximum number of lines in
                                    printed table}
var
smallp       : array[1..15813] of Boolean; {Small primes used for sifting.
                                         Smallp[i] = true if 2i + 1 is prime}
primes                                     {primes used as trial divisors}
             : array[1..3402] of integer;

mu, divisor, LitOmega, BigOmega, phi,
factors                                    {record of progress made in
                                            factoring j+i}
             : array[0..printerlimit] of longint;
sigma        : array[0..printerlimit] of extended;
h,                                         {j - 1 (mod p)}
i,                                         {index of terms from j to j + 19}
l                                          {length of table}
             : integer;
j,                                         {first number in displayed table}
k,                                         {j ó k < j + 20, p|k}
p,                                         {a prime divisor of k}
q,                                         {a power of p dividing k}
m,                                         {multiplicity of p in k}
sig,                                       {1 + p + p^2 + ..., for computing
                                            sigma(k)}
r,                                         {r = k/p}
r1,                                        {r1 = r/p}
plim                                       {limit of primes used as
                                            trial divisors}
             : longint;
OriginalMode : word;
Ch           : char;
St           : string[40];
funckey,
inputok      : Boolean;

function GetInput(XCoord, YCoord : integer; prompt, comm : string;
                  min, max : longint) : longint;
var
x            : longint;     {raw input}
x0                          {x coordinate of cursor when ready
                             to read input}
             : integer;
InputOk      : Boolean;

begin                    {body of function GetInput}
  GoToXY(XCoord, YCoord);
  ClrEoL;
  Write(Prompt);
  x0 := WhereX;
  if comm <> ''
  then
    begin
      WriteLn;
      ClrEoL;
      Write(comm);
      GoToXY(x0, WhereY - 1)
    end;
  InputOk := False;
  repeat
    ReadLn(x);
    if (frac(x) = 0) and (x >= min) and (x <= max)
    then InputOk := True
    else
      begin
        GoToXY(x0, WhereY - 1);
        ClrEoL
      end;
  until InputOk;
  ClrEoL;
  GetInput := x
end;             {of function GetInput}

procedure CalcTab(j : longint; l : integer);  {factor the numbers in
                                               [j, j+l), and compute the
                                               values of their arithmetic
                                               functions}
begin                        {main body of CalcTab}
  for i := 0 to l - 1 do
    begin
      sigma[i] := 1;
      mu[i] := 1;
      divisor[i] := 1;
      LitOmega[i] := 0;
      BigOmega[i] := 0;
      phi[i] := 1;
      factors[i] := 1
    end;
  plim := 1 + round(Sqrt(j+l));
  i := 1;
  while primes[i] < plim do
    begin
      p := primes[i];
      h := (j - 1) mod p;
      k := j + p - 1 - h;
      while k < j + l do
        begin
          LitOmega[k-j] := LitOmega[k-j] + 1;
          phi[k-j] := phi[k-j]*(p - 1);
          factors[k-j] := factors[k-j]*p;
          r := k div p;
          r1 := r div p;
          m := 1;
          q := p;
          sig := 1 + p;
          while r1*p = r do
            begin
              r := r1;
              r1 := r div p;
              m := m + 1;
              q := q*p;
              phi[k-j] := phi[k-j]*p;
              factors[k-j] := factors[k-j]*p;
              sig := sig + q
            end;
          sigma[k-j] := sigma[k-j]*sig;
          if m = 1
          then mu[k-j] := -mu[k-j]
          else mu[k-j] := 0;
          divisor[k-j] := divisor[k-j]*(m + 1);
          BigOmega[k-j] := BigOmega[k-j] + m;
          k := k + p
        end;
      i := i + 1;
    end;
  for i := 0 to l - 1 do
  if factors[i] < j + i
  then
    begin
      p := (j+i) div factors[i];
      sigma[i] := sigma[i]*(p + 1);
      mu[i] := -mu[i];
      divisor[i] := divisor[i]*2;
      BigOmega[i] := BigOmega[i] + 1;
      LitOmega[i] := LitOmega[i] + 1;
      phi[i] := phi[i]*(p - 1)
    end;
  end;                              {of procedure CalcTab}

begin                        {main body}
OriginalMode := LastMode;
TextBackground(1);
TextColor(14);
ClrScr;
WriteLn;
WriteLn('Constructing a list of the first 3402 primes,');
Write('to be used as trial divisors ... ');
FillChar(Smallp, 15813, 1);
for i := 1 to 86 do
  if smallp[i] = true                 {2i + 1 is prime}
  then
    begin
       k := 2*i + 1;
       j := 2*i*(i+1);                {Start sifting at 2j + 1 = (2i + 1)^2}
       while j <= 15813 do
         begin
           smallp[j] := false;        {2j + 1 is divisible by 2i + 1}
           j := j + k
         end
    end;                            {Array smallp, giving all primes < 31643
                                     has been constructed.  The last prime is
                                     31627, corresponding to i = 15813.  The
                                     array was sifted by all primes < 179,
                                     the last one being 173, corresponding to
                                     i = 86.}
primes[1] := 2;                     {Using the array smallp, a list of the}
h := 1;                             {first 3402 primes is constructed,}
for i := 1 to 15813 do              {in the array primes}
if smallp[i]
then
  begin
    h := h + 1;
    primes[h] := 2*i + 1;
  end;
Ch := 'F';
j := 1;
funckey := false;
Repeat
  if (funckey) and (Ch = #73)     {PgUp}
  then
    begin
      j := j - 20;
      if j < 1
      then j := 1;
      GoToXY(1, 25);
      ClrEoL;
      Write('                       Calculating new values ... ')
    end;
  if (funckey) and (Ch = #81)     {PgDn}
  then
    begin
      j := j + 20;
      if j > Limit - 19 then j := limit - 19;
      if j < 0 then j := 0;
      GoToXY(1, 25);
      ClrEoL;
      Write('                       Calculating new values ... ')
    end;
  if (not funckey) and (Upcase(Ch) = 'J')
  then
    begin
      j := round(GetInput(1, 24, 'Jump to n = ',
                 '  (1 ó n < 1000000000)', 1, limit));
      if j > Limit - 19
      then j := Limit - 19;
      GoToXY(1, 25);
      ClrEoL;
      Write('                       Calculating new values ... ')
    end;
  CalcTab(j, 20);
  TextColor(14);
  TextBackground(1);
  ClrScr;
  WriteLn('                             ARITHMETIC FUNCTIONS');
  WriteLn;
  Write('     n        omega(n)   OMEGA(n)    mu(n)     d(n)');
  WriteLn('      phi(n)       sigma(n)');
  for i := 0 to 19 do
    begin
      Write((i+j):10, LitOmega[i]:8, BigOmega[i]:11, mu[i]:12);
      WriteLn(divisor[i]:9, phi[i]:14, sigma[i]:15:0)
    end;
  WriteLn;
  TextBackground(7);
  GoToXY(1, 25);
  ClrEoL;
  TextColor(4);
  Write('      PgUp           PgDn           J');
  TextColor(0);
  Write('ump');
  TextColor(4);
  Write('          P');
  TextColor(0);
  Write('rint');
  TextColor(4);
  Write('           Esc');
  GoToXY(1, 25);                                    {hide the cursor}
  TextColor(7);
  Write(' ');
  GoToXY(1, 25);
  TextColor(0);
  repeat
    Ch := ReadKey;
    if Ch <> #0
    then funckey := false
    else
      begin
        funckey := true;
        Ch := ReadKey
      end;
    inputok := false;
    if funckey and ((Ch = #73) or (Ch = #81))
    then inputok := true;
    if (not funckey) and (Ch IN [#27, 'j', 'J', 'p', 'P'])
    then inputok := true
  until inputok;
  if (not funckey) and (UpCase(Ch) = 'P')      {Print}
  then
    begin
      GoToXY(1, 25);
      ClrEoL;
      Write('              Calculating values ... ');
      l := limit - j;
      if l > PrinterLimit then l := PrinterLimit;
              {l = min(PrinterLimit, limit - j)}
      CalcTab(j, l);
      Write('done.  Printing ... ');
      WriteLn(lst, '                             ARITHMETIC FUNCTIONS');
      WriteLn(lst);
      WriteLn(lst, 'for n from ', j:1, ' to ', (j + l - 1):1);
      WriteLn(lst);
      Write(lst, '     n        omega(n)   OMEGA(n)    mu(n)     d(n)');
      WriteLn(lst, '      phi(n)       sigma(n)');
      for i := 0 to l - 1 do
        begin
          Write(lst, (i+j):10, LitOmega[i]:8, BigOmega[i]:11, mu[i]:12);
          WriteLn(lst, divisor[i]:9, phi[i]:14, sigma[i]:15:0)
        end;
      WriteLn(lst);
      Write(' done.')
    end;
until (not funckey) and (Ch = #27);
TextMode(OriginalMode)
end.

