program factab;   {Produce a table of the least prime factors of
                   odd integers}
uses crt;

const
lines = 20;   {Number of rows in table}
length = 100; {Number of entries in table is 5 times the number of lines}

var
smallp       : array[1..15803] of Boolean; {Small primes used for sifting.
                                         Smallp[i] = true if 2i + 1 is prime}
primes       : array[1..3400] of integer;
table        : array[1..length] of integer;
h, i, j, k, l : integer;
OriginalMode : word;
Ch           : char;
funckey,
inputok      : Boolean;
m, n, r      : longint;

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

begin                        {main body}
OriginalMode := LastMode;
TextBackground(1);
TextColor(14);
ClrScr;
WriteLn('          Constructing a table of small primes,');
Write('     for use as trial divisors ... ');
FillChar(Smallp, 15803, 1);
for i := 1 to 86 do
  if smallp[i] = true                 {2i + 1 is prime}
  then
    begin
       k := 2*i + 1;
       j := 2*i*(i+1);                {Start sifting at 2j + 1 = (2i + 1)^2}
       while j <= 15803 do
         begin
           smallp[j] := false;        {2j + 1 is divisible by 2i + 1}
           j := j + k
         end
    end;                           {Array smallp, giving all primes < 31621
                                    has been constructed.  The last prime is
                                    31607, corresponding to i = 15803.  The
                                    array was sifted by all primes < 179,
                                    the last one being 173, corresponding to
                                    i = 86.}
h := 0;
for i := 1 to 15803 do
if smallp[i]
then
  begin
    h := h + 1;
    primes[h] := i
  end;
WriteLn('Done.');
WriteLn;
Write('           Will construct a table of the ');
WriteLn('least prime factor for');
WriteLn('     odd integers between 10N and 10N + ',lines:1, '0.');
n := GetInput(WhereX, WhereY, 'Enter N = ', ' (0 ó N ó 100000000)',
              0, 100000000);
Ch := 'F';
funckey := false;
Repeat
  if (not funckey) and (UpCase(Ch) = 'N')
  then
    n := GetInput(1, 24, 'Enter new value of N = ',
                  '              (0 ó N ó 100000000)', 0, 100000000);
  if (funckey) and (Ch = #73)     {PgUp}
  then
    begin
      n := n - 20;
      if n < 0 then n := 0;
    end;
  if (funckey) and (Ch = #81)     {PgDn}
  then
    begin
      n := n + 20;
      if n > 100000000 then n := 100000000;
    end;
  r := 5*n;
  TextColor(14);
  TextBackground(1);
  ClrScr;
  WriteLn('                      THE LEAST PRIME FACTOR OF ODD NUMBERS');
  WriteLn;
  Write('             n        10n+1     10n+3     10n+5     10n+7');
  WriteLn('     10n+9');
  for j := 1 to length do table[j] := 0;
                                 {Table[j] corresponds to the number
                                  10N + 2j - 1.}
  if n = 0 then table[1] := -1;  {The odd number 1 has no prime factor}
  m := 10*(n + lines);           {The maximum number in the table}
  l := round((Sqrt(m)-1)/2);     {Will sift by primes up to 2l + 1}
  h := 1;
  while (primes[h] <= l) and (h <= 3400) do
      begin
        i := primes[h];
        h := h + 1;
        k := 2*i + 1;            {k is the prime we are sifting by}
        m := trunc(r/k + 0.5);   {(2m+1)k is the least odd multiple
                                  of k larger than 10n}
        j := round(m*k + i + 1 - r);
                                 {This is the corresponding
                                  index in the table}
        while j <= length do
          begin
            if (table[j] = 0)
              and (r + j <> i + 1)      {k <> 10N + 2j - 1}
            then table[j] := k;
            j := j + k
          end
      end;
  for i := 0 to lines - 1 do
    begin
      Write((n+i):17);
      k := 5*i + 1;
      for j := 1 to 5 do
        begin
          if table[k] < 0 then Write('          ');
          if table[k] = 0 then
            begin
              TextColor(15);
              Write('     prime');
              TextColor(14)
            end;
          if table[k] > 0 then Write(table[k]:10);
          k := k + 1;
        end;
      WriteLn;
    end;
  WriteLn;
  TextBackground(7);
  GoToXY(1, 25);
  ClrEoL;
  TextColor(4);
  Write('      PgUp                 PgDn                  N');
  Write('                   Esc');
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
    if (not funckey) and (Ch IN [#27, 'n', 'N'])
    then inputok := true
  until inputok;
until (not funckey) and (Ch = #27);
TextMode(OriginalMode)
end.