program JacobDem;                   {Demonstration of calculation
                                     of the Jacobi symbol}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
P, Q,
t, u           :  comp;
x              :  extended;
code,
k,
J                          {The Jacobi symbol}
               :  integer;
minus          :  Boolean;

procedure Prompt;

var
x0, y0                     {coordinates of cursor location}
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

procedure DisplayCalc;

var
P0, P1, Q1            : comp;

begin
  P1 := P;
  Q1 := Q;
  x := Q1/2;
  if frac(x) = 0
  then
    begin
      WriteLn('Cannot compute Jacobi symbol (P/Q) when Q is even.');
      WriteLn('Here P = ', P:1:0,', and Q = ', Q:1:0,'.');
      Halt
    end;
  if Q < 1
  then
    begin
      WriteLn('The Jacobi symbol (P/Q) is undefined when Q < 1.');
      WriteLn('Here P = ', P:1:0, ', and Q = ', Q:1:0, '.');
      Halt
    end;
  if P = 0
  then
    begin
      J := 0;
      Exit
    end;
  J := 1;
  if P1 < 0
  then
    begin
      P1 := -P1;
      t := Q/4;
      if Q < 4*t                  {if Q ð 3 (mod 4)}
      then
        begin
          J := -J;
          Write('= - ')
        end
      else Write('= ');
      Write('(', P1:1:0, '/', Q1:1:0, ')');
      GoToXY(40, WhereY);
      Write('(since ', Q1:1:0, ' ð ');
      if J < 0
      then Write('3')
      else Write('1');
      WriteLn(' (mod 4)');
      Prompt;
      WriteLn
    end;
  while P1 > 1 do
    begin
      P0 := P1;
      P1 := condition(P1, Q1);
      if P0 <> P1
      then
        begin
          Write('= ');
          if J < 0
          then Write('- ');
          Write('(', P1:1:0, '/', Q1:1:0, ')');
          GoToXY(40, WhereY);
          Write('(since ', P0:1:0, ' ð ', P1:1:0);
          WriteLn(' (mod ', Q1:1:0, '))');
          Prompt;
          WriteLn
        end;
      if P1 = 0
      then
        begin
          J := 0;
          WriteLn('= 0');
          Prompt;
          WriteLn;
          exit
        end;
      k := 0;
      P0 := P1;
      x := P1/2;
      while frac(x) = 0 do          {while P1 is even}
        begin
          P1 := x;
          x := P1/2;
          k := k + 1
        end;                        {k is now the power of 2 in P1}
      if k mod 2 = 1                {if k is odd}
      then
        begin
          Write('= ');
          if J < 0
          then Write('- ');
          WriteLn('(2/', Q1:1:0, ')(', P1:1:0, '/', Q1:1:0, ')');
          if k > 1
          then WriteLn;
          GoToXY(40, WhereY);
          Write('(since ', P0:1:0, ' = 2');
          if k > 1
          then
            begin
              GoToXY(WhereX, WhereY - 1);
              Write(k:1);
              GoToXY(WhereX, WhereY+1)
            end;
          if P1 > 1
          then Write('ù', P1:1:0);
          WriteLn(')');
          Prompt;
          WriteLn;
          t := Q1/8;
          t := Q1 - 8*t;
          if t < 0 then t := t + 8;
          if (t = 3) or (t = 5)    {if Q1 ð ñ3 (mod 8)}
          then J := -J;
          Write('= ');
          if J < 0
          then Write('- ');
          Write('(', P1:1:0, '/', Q1:1:0, ')');
          GoToXY(40, WhereY);
          WriteLn('(since ', Q1:1:0, ' ð ', t:1:0, ' (mod 8))');
          Prompt;
          WriteLn
        end;
      if (k mod 2 = 0) and (k > 0)
      then
        begin
          Write('= ');
          if J < 0
          then Write('- ');
          Write('(', P1:1:0, '/', Q1:1:0, ')');
          GoToXY(40, WhereY);
          Write('(since ', P0:1:0, ' = 2');
          if k > 1
          then
            begin
              GoToXY(WhereX, WhereY - 1);
              Write(k:1);
              GoToXY(WhereX, WhereY + 1)
            end;
          if P1 > 1
          then Write('ù', P1:1:0);
          WriteLn(')');
          Prompt;
          WriteLn
        end;
      if P1 > 1                      {exchange P1 and Q1}
      then
        begin
          t := Q1;
          Q1 := P1;
          P1 := t;
          t := P1/4;
          t := P1 - 4*t;
          if t < 0 then t := t + 4;
          u := Q1/4;
          u := Q1 - 4*u;
          if u < 0 then u := u + 4;
          if (t = 3) and (u = 3)      {if P1 ð Q1 ð 3 (mod 4)}
          then J := -J;
          Write('= ');
          if J < 0
          then Write('- ');
          Write('(', P1:1:0, '/', Q1:1:0, ')');
          GoToXY(40, WhereY);
          Write('(since ');
          if t = 1
          then WriteLn(P1:1:0, ' ð 1 (mod 4))');
          if (t = 3) and (u = 1)
          then WriteLn(Q1:1:0, ' ð 1 (mod 4))');
          if (t = 3) and (u = 3)
          then WriteLn(P1:1:0, ' ð ', Q1:1:0, ' ð 3 (mod 4))');
          Prompt;
          WriteLn
        end;
    end;
  WriteLn('= ', J:1);
  Prompt;
  WriteLn
end;                              {of procedure DisplayCalc}

begin                             {main body}
  if (ParamCount = 2)
  then
    begin
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0)
         and (x >= -MaxAllow) and (x <= MaxAllow)
      then P := x
      else Halt;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0)
          and (x > 0) and (x <= MaxAllow)
      then Q := x
      else Halt;
    end
  else
    begin
      Write('Will demonstrate the computation');
      WriteLn(' of the Jacobi symbol (P/Q).');
      P := GetInput(WhereX, WhereY, 'Enter P = ', '', -MaxAllow, MaxAllow);
      Q := GetInput(WhereX, WhereY, 'Enter Q = ', '', 1, MaxAllow)
    end;
  ClrScr;
  WriteLn('  (', P:1:0, '/', Q:1:0, ')');
  WriteLn;
  DisplayCalc;
  WriteLn('(', P:1:0, '/', Q:1:0, ') = ', J:1)
end.

