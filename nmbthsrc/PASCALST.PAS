program PascalsT;  {Display Pascal's triangle (mod m)}
uses CRT;

const
  MaxMod = 999;
  ArraySize = 9999;
  w = 4;               {width of field for each entry in display}
  i0 = 17;             {number of columns in display}

var
m,                     {modulus}
i, j, k, n,            {indices}
t, u                   {temporary variables}
     :  integer;
BinCoef                {BinCoef[k] = (n choose k) (mod m)}
     :  array[0..ArraySize] of integer;
OriginalMode         {Save original screen mode for restoration}
         : word;
Ch       : char;     {Answer to prompt}
funckey,             {was a function key pressed?}
inputok              {is the input acceptable?}
         : boolean;

procedure Display(n, k : integer);   {(n choose k) (mod m) is in upper
                                      left corner}

var
ind                        {amount to indent by}
          :  integer;
Line      :  array[-20..20] of integer;

begin
  TextColor(14);
  TextBackground(1);
  ClrScr;
  ind := (53 - w) div 2;   {amount to indent by, to center title}
  GoToXY(ind, 1);
  WriteLn('BINOMIAL COEFFICIENTS (MOD ', m:1, ')');
  WriteLn;
  TextColor(15);
  Write('    k');
  for i := k to (k + i0) do Write(i:w);
  WriteLn;
  WriteLn('  n  ');
  for i := -20 to 20 do
    begin
      j := k + i;
      if (j >= 0) and (j <= n)
      then line[i] := BinCoef[j]
      else line[i] := 0
    end;
  j := 0;
  while (j + n <= 9999) and (j < 20) do
    begin
      TextColor(15);
      Write((n+j):4);
      TextColor(14);
      Write(' ');
      for i := 0 to i0 do
        if (k + i) <= n + j
        then
          if line[i] = 0
          then
            begin
              TextColor(15);
              Write(0:w);
              TextColor(14)
            end
          else Write(line[i]:w)
        else for ind := 1 to w do Write(' ');
      WriteLn;
      t := line[j-20];
      j := j + 1;
      for i := j - 20 to 20 do
        begin
          u := t + line[i];
          if u >= m then u := u - m;
          t := line[i];
          line[i] := u
        end;
    end;
  TextBackground(7);
  GoToXY(1, 25);
  ClrEoL;
  TextColor(4);
  if n > k
  then Write('      ', #24)
  else Write('       ');
  if n < 9980
  then Write('        ', #25)
  else Write('         ');
  if k > 0
  then Write('        ', #27)
  else Write('         ');
  if k < n + 2
  then Write('        ', #26)
  else Write('         ');
  if (n > 0) or (k > 0)
  then
    begin
      Write('         T');
      TextColor(0);
      Write('op         ');
      TextColor(4)
    end
  else
    Write('                     ');
  Write('M');
  TextColor(0);
  Write('odulus         ');
  TextColor(4);
  Write('Esc');
  GoToXY(1, 25);             {hide the cursor}
  TextColor(7);
  Write(' ');
  GoToXY(1, 25);
  TextColor(0)
end;                         {of procedure Display}

function GetInput(XCoord, YCoord : integer; prompt, comm : string;
                                         min, max : integer) : integer;

var
x                                    {raw input}
         : integer;
x0                                   {x coordinate of cursor when ready
                                      to read input}
         : integer;
InputOK  : Boolean;

begin                                {body of function GetInput}
  GoToXY(XCoord, YCoord);
  ClrEoL;
  Write(prompt);
  x0 := WhereX;
  if comm <> ''
  then
    begin
      WriteLn;
      ClrEoL;
      Write(comm);
      GoToXY(x0, WhereY - 1)
    end;
  InputOK := False;
  repeat
    ReadLn(x);
    if (frac(x) = 0) and (x <= max) and (x >= min)
    then InputOK := True
    else
      begin
        GoToXY(x0, WhereY - 1);
        ClrEoL
      end;
  until InputOK;
  ClrEoL;
  GetInput := x
end;                             {of function GetInput}

begin  {main body}
  OriginalMode := LastMode;
  funckey := false;
  Ch := 'F';
  TextColor(14);
  TextBackground(1);
  ClrScr;
  WriteLn('Display binomial coefficients (n choose k) (mod m).');
  m := GetInput(WhereX, WhereY, '  Enter modulus m = ',
       '(Must have 0 < m ó 999)', 1, MaxMod);
  n := 0;
  k := 0;
  BinCoef[0] := 1;
  repeat
    if (not funckey) and ((Ch = 'm') or (Ch = 'M'))
    then
      begin
        m := GetInput(1, 24, 'Enter new modulus m = ',
        '  (must have 0 < m ó 999)',
         1, MaxMod);
        n := 0;
        k := 0;
      end;
    if (funckey) and (Ch = #72)          {UpArrow}
    then
      begin
        u := n - k;
        if u > 20
        then u := 20;
        for j := 1 to u do
          begin
            n := n - 1;
            for i := 1 to n do
              begin
                t := BinCoef[i] - BinCoef[i-1];
                if t < 0 then t := t + m;
                BinCoef[i] := t
              end
          end
      end;
    if (funckey) and (Ch = #80)          {DownArrow}
    then
      begin
        j := 0;
        while (n < 9980) and (j < 20) do
        begin
          j := j + 1;
          t := 1;
          for i := 1 to n do
            begin
              u := t + BinCoef[i];
              if u >= m
              then u := u - m;
              t := BinCoef[i];
              BinCoef[i] := u
            end;
          n := n + 1;
          BinCoef[n] := 1
        end;
      end;
    if (funckey) and (Ch = #77)
    then
      begin
        k := k + i0 + 1;
        if k > n + 2 then k := n + 2;
      end;
    if (funckey) and (Ch = #75)
    then
      begin
        k := k - i0 - 1;
        if k < 0 then k := 0;
      end;
    if (not funckey) and (UpCase(Ch) = 'T')
    then
      begin
        k := 0;
        n := 0
      end;
    Display(n, k);
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
      if (funckey) and (Ch = #72) and (n > k)
      then InputOk := True;
      if (funckey) and (Ch = #80) and (n < 9980)
      then InputOk := True;
      if (funckey) and (Ch = #77) and (k < n + 2)
      then InputOk := True;
      if (funckey) and (Ch = #75) and (k > 0)
      then InputOk := true;
      if (not funckey)
        and (Ch in [#27, 'm', 'M'])
      then inputok := true;
      if (not funckey) and (UpCase(Ch) = 'T') and ((n > 0) or (k > 0))
      then InputOk := True
    until inputok;
  until (not funckey) and (ch = #27);
  TextMode(OriginalMode)
end.

