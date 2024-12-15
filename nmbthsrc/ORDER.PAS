{program order;     Determine the order of a (mod m)}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
a, c, m, r      : comp;
x               : extended;
code, i         : integer;
St              : string[20];
InputOK         : boolean;

begin                               {main body of order}
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
          if power(a, c, m) <> 1 then c := Carmichael(m);
        end
      else c := Carmichael(m);
    end
  else
    begin
      WriteLn('Will calculate the order of a (mod m) where ');
      a := GetInput(WhereX, WhereY, 'a = ', '', -MaxAllow, MaxAllow);
      m := GetInput(WhereX, WhereY, 'm = ', '', 1, MaxAllow);
      Write('c =                     (a multiple of the order of a');
      WriteLn(' (mod m),');
      WriteLn('such as phi(m), if known.  Otherwise type <Return>, ');
      WriteLn('and the program will factor m and then set c := Carmichael(m).');
      GoToXY(5, WhereY - 2);
      InputOK := False;
      repeat
        GoToXY(5, WhereY - 1);
        Write('                   ');
        GoToXY(5, WhereY);
        ReadLn(St);
        if St = ''
        then
          begin
            c := Carmichael(m);
            InputOK := True
          end
        else
          begin
            Val(St, x, code);
            if (frac(x) = 0) and (code = 0) and (x > 0) and (x <= MaxAllow)
            then
              begin
                c := x;
                if power(a, c, m) = 1 then InputOK := True
              end
          end
      until InputOK
    end;
  r := order(a, m, c);
  if (ParamCount <> 2) and (ParamCount <> 3)
  then
    begin
      GoToXY(1, WhereY + 1);
      for i := 1 to 5 do
      begin
        ClrEoL;
        GoToXY(1, WhereY - 1)
      end;
      ClrEoL
    end;
  WriteLn('The order of ', a:1:0, ' (mod ', m:1:0, ') is ', r:1:0, '.')
end.
