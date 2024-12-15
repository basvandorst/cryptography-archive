program RhoDem;               {Demonstration of Pollard rho method}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
n,                         {The number to be factored}
c,                         {constant added in recurrence}
g, h,                      {factors found}
u,                         {u(i)}
v                          {u(2i)}
      :  comp;
i,                         {indices}
code                       {error level in converting string to number}
      :  integer;
k     :  longint;
x     :  extended;
cycle                      {Use the cycle-detecting algorithm?}
      :  Boolean;
Ch    :  char;

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
  Write('         Press any key to continue ... (or Esc to terminate)');
  Ch := ReadKey;
  if Ch = #0
  then Ch := ReadKey;
  if Ch = #27
  then
    begin
      WriteLn;
      Halt
    end;
  GoToXY(1, 25);
  ClrEoL;
  GoToXY(x0, y0)
end;                               {of procedure Prompt}

begin                              {main body}
  If ParamCount = 1
  then
    begin
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0) and (x > 1) and (x <= MaxAllow)
      then n := x;
      Cycle := True;
      c := 1
    end
  else
    begin
      WriteLn('Will employ the Pollard rho method to attempt to find');
      WriteLn('a proper factor of a given number n.  This should be done');
      WriteLn('only for n which are already known to be composite.');
      Write('Use cycle-detecting algorithm? (Y or N) ');
      repeat
        Ch := ReadKey;
        if Ch = #0
        then Ch := ReadKey
      until UpCase(Ch) IN ['Y', 'N'];
      if UpCase(Ch) = 'Y'
      then Cycle := True
      else Cycle := False;
      WriteLn;
      n := GetInput(WhereX, WhereY, '     Enter n = ',
          'where 1 < n ó 1000000000000000000.', 2, MaxAllow);
      c := GetInput(WhereX, WhereY, '     Enter c = ',
          'where u(k+1) ğ u(k)ı + c (mod n)', -MaxAllow, MaxAllow)
    end;
  ClrScr;
  WriteLn('               u(k+1) ğ u(k)ı + ', c:1:0, ' (mod ', n:1:0, ')');
  WriteLn;
  if Cycle
  then
    begin
      Write('     k                u(k)               u(2k)');
      WriteLn('            gcd');
      Write('      0                   0                   0');
      k := 0;
      u := 0;
      v := 0;
      i := 0;
      repeat
        if i = 20
        then
          begin
            i := 0;
            Prompt;
            ClrScr;
            Write('               u(k+1) ğ u(k)ı + ', c:1:0);
            WriteLn(' (mod ', n:1:0, ')');
            WriteLn;
            Write('     k                u(k)               u(2k)');
            Write('            gcd');
          end;
        u := mult(u, u, n) + c;
        v := mult(v, v, n) + c;
        v := mult(v, v, n) + c;
        k := k + 1;
        i := i + 1;
        g := gcd(u-v, n);
        WriteLn;
        Write(k:7, u:20:0, v:20:0, g:15:0);
        if g = n
        then
          begin
            WriteLn;
            WriteLn;
            WriteLn('     gcd(u(', k:1, ') - u(', (2*k):1, '), n) ');
            WriteLn('   = gcd(', u:1:0, ' - ', v:1:0, ', ', n:1:0, ')');
            WriteLn('   = ', g:1:0);
            Write('We replace c = ', c:1:0, ' by ');
            c := c + 1;
            WriteLn(c:1:0, ', and start over.');
            u := 0;
            v := 0;
            i := 20;
            k := 0;
          end
      until (g > 1) and (g < n);
      WriteLn;
      WriteLn('     gcd(u(', k:1, ') - u(', (2*k):1, '), n) ');
      WriteLn('   = gcd(', u:1:0, ' - ', v:1:0, ', ', n:1:0, ')');
      WriteLn('   = ', g:1:0);
      WriteLn;
      Prompt;
      h := n/g;
      WriteLn(n:1:0, ' = ', g:1:0, 'ù', h:1:0)
    end
  else
    begin
      WriteLn('                k                u(k)');
      Write('                 0                   0');
      k := 0;
      u := 0;
      i := 0;
      repeat
        if i = 20
        then
          begin
            i := 0;
            Prompt;
            ClrScr;
            Write('               u(k+1) ğ u(k)ı + ', c:1:0);
            WriteLn(' (mod ', n:1:0, ')');
            WriteLn;
            Write('                k                u(k)');
          end;
        u := mult(u, u, n) + c;
        k := k + 1;
        i := i + 1;
        WriteLn;
        Write(k:18, u:20:0);
      until Ch = #27
    end
end.
