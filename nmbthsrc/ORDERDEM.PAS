program OrderDem;     {Demonstrate the procedure for
                       determining the order of a (mod m)}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
a, c, m, r      : comp;
x               : extended;
code, i         : integer;
St              : string[20];
InputOK         : boolean;

procedure Prompt;

var
x0, y0                    {coordinates of cursor location}
         : integer;
Ch       : Char;

begin
  x0 := WhereX;
  y0 := WhereY;
  if y0 = 25
  then
    begin
      WriteLn;
      y0 := 24
    end;
  GoToXY(1, 25);
  ClrEoL;
  Write('                         Press any key to continue ... ');
  Ch := ReadKey;
  if Ch = #0
  then Ch := ReadKey;
  GoToXY(1, 25);
  ClrEoL;
  GoToXY(x0, y0)
end;                               {of procedure Prompt}

function orddem(a, m, c : comp) : comp;
                       {The order of a (mod m).  If (a, m) > 1 it returns 0.
                        c is a multiple of the order, such as phi(m) or
                        Carmichael(m)}
var
i, j,                               {indices}
k                                   {The number of prime factors of c}
                : integer;
f,                                  {initially c, eventually ord}
r,
v
                : comp;
p                                   {An array containing the
                                     prime factors of c}
                : primes;
mul                                 {An array giving the multiplicity
                                     of the prime divisors of c}
                : multiplicity;

begin
  if condition(a, m) = 1
  then
    begin
      orddem := 1;
      exit
    end;
  WriteLn;
  Write('Since ', a:1:0);
  GoToXY(WhereX, WhereY - 1);
  Write(c:1:0);
  GoToXY(WhereX, WhereY + 1);
  WriteLn(' ð 1 (mod ', m:1:0, '),');
  Write(c:1:0, ' is a multiple of the order of ', a:1:0, ' (mod ');
  WriteLn(m:1:0, ').');
  WriteLn('We factor ', c:1:0, ':');
  WriteLn;
  canonic(c, k, p, mul, False);
  Write(c:1:0, ' = ');
  for i := 1 to k do
    begin
      Write(p[i]:1:0);
      if mul[i] > 1
      then
        begin
          GoToXY(WhereX, WhereY - 1);
          Write(mul[i]:1);
          GoToXY(WhereX, WhereY+1)
        end;
      if i < k
      then Write('ù')
      else WriteLn;
    end;
  WriteLn;
  Prompt;
  f := c;
  for i := 1 to k do
    begin
      WriteLn('Testing for powers of ', p[i]:1:0, ' in order(', a:1:0, '):');
      v := 1;
      j := mul[i];
      while (j > 0) and (v = 1) do
        begin
          r := f/p[i];
          v := power(a, r, m);
          if v = 1
          then
            begin
              j := j - 1;
              WriteLn;
              Write('     Since ', a:1:0);
              GoToXY(WhereX, WhereY - 1);
              Write(f:1:0, '/', p[i]:1:0);
              GoToXY(WhereX, WhereY + 1);
              WriteLn(' ð 1 (mod ', m:1:0, '),');
              f := r;
              Write('     ', f:1:0);
              WriteLn(' is a multiple of order(', a:1:0, ').');
              Prompt;
              if j = 0
              then
                begin
                  Write('     Since ', p[i]:1:0, ' does not divide ');
                  WriteLn(f:1:0, ',');
                  Write('     we have eliminated all powers of ');
                  WriteLn(p[i]:1:0, '.');
                  Prompt
                end;
            end
          else
            begin
              WriteLn;
              Write('     Since ', a:1:0);
              GoToXY(WhereX, WhereY - 1);
              Write(f:1:0, '/', p[i]:1:0);
              GoToXY(WhereX, WhereY + 1);
              WriteLn(' ð ', v:1:0, ' (mod ', m:1:0, '),');
              Write('     it follows that the power of ', p[i]:1:0);
              WriteLn(' dividing ', f:1:0);
              Write('     (namely ', j:1, ') is the same as the power of ');
              WriteLn(p[i]:1:0);
              WriteLn('     dividing order(', a:1:0, ').');
              Prompt
            end;
        end;
    end;
 orddem := f
end;                                {of the function orddem}

begin                               {main body of OrderDem}
  if (ParamCount = 2) or (ParamCount = 3)
  then
    begin
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0) and (Abs(x) <= MaxAllow)
      then a := x
      else Halt;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0) and (x > 0) and (x <= MaxAllow)
      then m := x
      else Halt;
      if gcd(a, m) > 1
      then
        begin
          WriteLn('The order of ', a:1:0, ' (mod ', m:1:0, ') is undefined');
          WriteLn('because (', a:1:0, ', ', m:1:0, ') > 1.');
          Halt
        end;
      if ParamCount = 3
      then
        begin
          Val(ParamStr(3), x, code);
          if (frac(x) = 0) and (code = 0) and (x > 0) and (x <= MaxAllow)
          then c := x
          else Halt;
          r := power(a, c, m);
          if r <> 1
          then
            begin
              WriteLn;
              Write(a:1:0);
              GoToXY(WhereX, WhereY - 1);
              Write(c:1:0);
              GoToXY(WhereX, WhereY + 1);
              WriteLn(' ð ', r:1:0, ' (mod ', m:1:0, ').');
              Write('Hence the number ', c:1:0);
              WriteLn(' provided is not a multiple of');
              WriteLn('the order of ', a:1:0, ' (mod ', m:1:0, ').');
              WriteLn('Consequently we factor ', m:1:0, ',');
              Write('and use this factorization to determine í(');
              WriteLn(m:1:0, ').');
              c := phi(m);
              WriteLn('í(', m:1:0, ') = ', c:1:0, '.');
              Prompt
            end
        end
      else
        begin
          WriteLn('We begin by factoring ', m:1:0, ', and use this');
          WriteLn('factorization to calculate í(', m:1:0, ').');
          c := phi(m);
          WriteLn('í(', m:1:0, ') = ', c:1:0, '.');
          Prompt
        end;
    end
  else
    begin
      WriteLn('Will calculate the order of a (mod m) where ');
      a := GetInput(WhereX, WhereY, 'a = ', '', -MaxAllow, MaxAllow);
      m := GetInput(WhereX, WhereY, 'm = ', '', 1, MaxAllow);
      if gcd(a, m) > 1
      then
        begin
          WriteLn('The order of ', a:1:0, ' (mod ', m:1:0, ') is undefined');
          WriteLn('because (', a:1:0, ', ', m:1:0, ') > 1.');
          Halt
        end;
      Write('c =                     (a multiple of the order of a');
      WriteLn(' (mod m),');
      WriteLn('such as phi(m), if known.  Otherwise type <Return>, ');
      WriteLn('and the program will factor m and then set c := í(m).');
      GoToXY(5, WhereY - 3);
      Write('                   ');
      GoToXY(5, WhereY);
      ReadLn(St);
      ClrEoL;
      WriteLn;
      ClrEoL;
      if St = ''
      then
        begin
          c := phi(m);
          WriteLn('í(', m:1:0, ') = ', c:1:0, '.');
          Prompt
        end
      else
        begin
          Val(St, x, code);
          if (frac(x) = 0) and (code = 0) and (x > 0) and (x <= MaxAllow)
          then
            begin
              c := x;
              r := power(a, c, m);
              if r <> 1
              then
                begin
                  WriteLn;
                  Write(a:1:0);
                  GoToXY(WhereX, WhereY - 1);
                  Write(c:1:0);
                  GoToXY(WhereX, WhereY + 1);
                  WriteLn(' ð ', r:1:0, ' (mod ', m:1:0, ').');
                  Write('Hence the number ', c:1:0);
                  WriteLn(' provided is not a multiple of');
                  WriteLn('the order of ', a:1:0, ' (mod ', m:1:0, ').');
                  WriteLn('Consequently we factor ', m:1:0, ',');
                  Write('and use this factorization to determine í(');
                  WriteLn(m:1:0, ').');
                  c := phi(m);
                  WriteLn('í(', m:1:0, ') = ', c:1:0, '.');
                  Prompt
                end
            end
          else Halt;
        end;
    end;
  r := orddem(a, m, c);
  WriteLn;
  WriteLn('The order of ', a:1:0, ' (mod ', m:1:0, ') is ', r:1:0, '.')
end.

