
program ProveP;             {Prove that the argument is prime}
{$N+,E+}
uses CRT, nothy;
{$I getinput.i }

var
p,                         {The putative prime}
n,                         {p - 1}
r,                         {The portion of n which remains to be factored}
s,                         {The factored portion of n}
p1,                        {a prime factor of r}
s0,                        {Sqrt(p)}
d,                         {A trial divisor of n}
m,                         {multiplicity to which d divides n}
c0, c1,                    {coefficients of a quadratic}
r1, r2,                    {roots of a quadratic}
t                          {a temporary variable}
          : comp;
x         : extended;
i,                         {an index}
code      : integer;
Ch        : char;

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

procedure test(d : comp);

var
a,                         {A base of an exponential}
k,                         {An exponent}
b,                         {A power}
g                          {A gcd}
          : comp;
x         : extended;

begin
  x := r/d;
  if frac(x) = 0
  then
    begin
      a := 10000;
      k := n/d;
      repeat
        a := a + 1;
        b := power(a, k, p);
        g := gcd(b - 1, p);
        if (g > 1) and (g < p)
        then
          begin
            WriteLn;
            WriteLn(p:1:0, ' is COMPOSITE');
            WriteLn;
            WriteLn('because it has the proper divisor ', g:1:0, '.');
            Halt
          end;
        b := power(b, d, p);
        if b > 1
        then
          begin
            WriteLn;
            WriteLn(p:1:0, ' is COMPOSITE');
            WriteLn;
            Write('because ', a:1:0, '^', n:1:0, ' ð ' , b:1:0);
            WriteLn(' (mod ', p:1:0, ').');
            Halt
          end;
        if (b = 0) and (a < p)
        then
          begin
            g := gcd(a, p);
            WriteLn;
            WriteLn(p:1:0, ' is COMPOSITE');
            WriteLn;
            WriteLn('because it has the proper divisor ', g:1:0, '.');
            Halt
          end;
      until ((g = 1) and (b = 1)) or (a > 10200);
      if (g = 1) and (b = 1)
      then
        begin
          m := 0;
          repeat
            r := x;
            x := r/d;
            s := s*d;
            m := m + 1
          until frac(x) > 0;
          Write('Since q = ', d:1:0, ' is prime, q');
          if m > 1
          then Write('^', m:1:0);
          WriteLn('º(p - 1),');
          Write(a:1:0, '^(p-1) ð 1 (mod p), and (');
          WriteLn(a:1:0, '^(p-1)/q - 1, p) = 1,');
          Write('it follows that all prime factors of p');
          Write(' are ð 1 (mod ', d:1:0);
          if m > 1
          then Write('^', m:1:0);
          WriteLn(').');
          if WhereY > 20
          then Prompt
        end
      else
        begin
          WriteLn('Unable to find a such that a^', k:1:0, ' is not ð 1');
          WriteLn('(mod ', p:1:0, ').  Hence ', p:1:0, ' is probably ');
          WriteLn('composite.  Try some spsp tests.');
          Halt
        end
    end
end;                                {of procedure test}

begin                               {main body}
  if ParamCount = 1
  then
    begin
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0) and (x > 1) and (x <= MaxAllow)
      then p := x
      else Halt;
      WriteLn('p = ', p:1:0)
    end
  else
    begin
      WriteLn('Will prove that a given number p is prime.');
      p := GetInput(WhereX, WhereY, 'Enter putative prime, p = ', '',
                    2, MaxAllow)
    end;
  n := p - 1;
  if p = 2
  then
    begin
      WriteLn('p = 2 is PRIME.');
      exit
    end;
  if (p>2) and (frac(p/2) = 0)
  then
    begin
      WriteLn('p = ', p:1:0, ' is COMPOSITE');
      WriteLn('because it has the proper divisor 2.');
      Exit
    end;
  s := 1;
  r := n;
  s0 := round(0.5 + Sqrt(p));
  while s0*s0 < p do s0 := s0 + 1;
  test(2);
  test(3);
  test(5);
  d := 1;
  repeat
    d := d + 6;
    test(d);
    d := d + 4;
    test(d);
    d := d + 2;
    test(d);
    d := d + 4;
    test(d);
    d := d + 2;
    test(d);
    d := d + 4;
    test(d);
    d := d + 6;
    test(d);
    d := d + 2;
    test(d);
    if KeyPressed
    then
      begin
        WriteLn('Attempt to factor ', n:1:0, ' interrupted. ');
        WriteLn('Partial result: ', n:1:0, ' = ', s:1:0, 'ù', r:1:0);
        WriteLn('where ', s:1:0, ' (the factored portion) is composed ');
        WriteLn('entirely of primes ó ', d:1:0);
        WriteLn('while ', r:1:0, ' is composed');
        WriteLn('entirely of primes > ', d:1:0);
        WriteLn('Choose an alternative:');
        WriteLn('     C  --  Continue');
        WriteLn('     S  --  Supply a prime divisor of ', r:1:0);
        WriteLn('     Q  --  Quit');
        repeat
          Ch := ReadKey
        until UpCase(Ch) in ['C', 'S', 'Q'];
        if UpCase(Ch) = 'Q' then Halt;
        for i := 1 to 10 do
          begin
            GoToXY(1, WhereY - 1);
            ClrEoL
          end;
        if UpCase(Ch) = 'S'
        then
          begin
            WriteLn('Enter a prime factor of ', r:1:0, '.');
            WriteLn('This number MUST be prime.');
            Write('p1 = ');
            ReadLn(p1);
            test(p1)
          end;
      end
  until (d*s > 5*s0) or (d*d > r);
                                 {if d*s > 5*s0 then the amount of time
                                  required to test p by divisors ð 1 (mod s)
                                  is no greater than has already been spent
                                  trying to factor p - 1.}
  if (d*d > r) and (r > 1)       {if r is prime}
  then test(r);
  Write('Altogether, all prime factors of p ');
  WriteLn('are ð 1 (mod ', s:1:0, ').');
  if s*s > p
  then
    begin
      WriteLn;
      WriteLn('Conclusion: ', p:1:0, ' is PRIME');
      WriteLn;
      WriteLn('because ', s:1:0, ' > Sqrt(', p:1:0, ').');
      Halt
    end;
  if s*s*s > p
  then
    begin
      c1 := n/s;
      c0 := c1/s;
      c1 := c1 - c0*s;
      if c1 <= 0
      then
        begin
          c1 := c1 + s;
          c0 := c0 - 1
        end;               {p = 1 + c1*s + c0*sý}
      t := c1*c1 - 4*c0;
      if t > 0
      then
        begin
          d := round(Sqrt(t));
          while (d+1)*(d+1) <= t do d := d + 1;
          while (d-1)*(d-1) >= t do d := d - 1
        end
      else d := 0;
      if d*d = t
      then
        begin
          r1 := (c1 - d)/2;
          r2 := (c1 + d)/2;
          WriteLn;
          WriteLn('Conclusion: ', p:1:0, ' is COMPOSITE');
          WriteLn;
          WriteLn('because ', p:1:0, ' = ', r1:1:0, 'ù', r2:1:0, '.');
          Halt
        end
      else
        begin
          WriteLn;
          Write('Conclusion: ');
          if WhereY > 12 then Prompt;
          WriteLn(p:1:0, ' is PRIME');
          WriteLn;
          WriteLn('because ', s:1:0, ' > ', p:1:0, '^(1/3).');
          WriteLn('If ', p:1:0, ' were composite then we would have');
          WriteLn(p:1:0, ' = (', s:1:0, 'r1 + 1)(', s:1:0, 'r2 + 1).');
          WriteLn('Writing ', p:1:0, ' in base ', s:1:0, ', we find that');
          Write(p:1:0, ' = 1 + ', c1:1:0, 'ù', s:1:0, ' + ', c0:1:0);
          WriteLn('ù(', s:1:0, ')ý.');
          WriteLn('Hence r1 and r2 would be roots of the quadratic equation');
          WriteLn('xý - ', c1:1:0, 'x + ', c0:1:0, ' = 0.');
          WriteLn('But this equation has discriminant ', t:1:0);
          WriteLn('which is not a perfect square.');
          Halt
        end;
    end;
  d := 1;
  Write('Will now test p for divisibility');
  WriteLn(' by numbers of the form ', s:1:0, 'm + 1.');
  repeat
    d := d + s;
    x := p/d;
    if frac(x) = 0
    then
      begin
        WriteLn;
        WriteLn('Conclusion: ', p:1:0, ' is COMPOSITE');
        WriteLn;
        WriteLn('because it has the proper divisor ', d:1:0, '.');
        Halt
      end
  until d > s0;
  WriteLn;
  WriteLn('Conclusion: ', p:1:0, ' is PRIME');
  WriteLn;
  WriteLn('because it has no proper divisor');
  WriteLn('d ó ', s0:1:0, ' of the form d = ', s:1:0, 'm + 1.')
end.