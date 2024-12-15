program IndTab;  {Display indices with respect to a given
                  primitive root (mod p)}
{$N+,E+}
uses CRT, Printer, nothy;

const
  MaxMod = 9999;

var
p,                     {modulus}
g,                     {least positive primitive root}
a,                     {g^a is new primitive root}
b,                     {newind is b*oldind.  That is, ab ð 1 (mod p-1).}
h, i, j, k,            {indices}
j0, r
         :  longint;
a1, m1   :  comp;
x        :  extended;  {raw input}
ind, exp
         :  array[0..10000] of integer;
OriginalMode         {Save original screen mode for restoration}
         : word;
Ch       : char;     {Answer to prompt}
St       : string[20];
funckey,             {was a function key pressed?}
inputok,             {is the input acceptable?}
powers,              {are we displaying powers or indices?}
NewMod               {is this a new prime modulus?}
         : boolean;

procedure Display(j : longint);          {Display array,
                                          starting with 10j}

var
St   :  string[20];
l,                     {length of a string}
i0                     {limit of index i}
     :  integer;

begin
  Str(p:0, St);
  l := Length(St);
  Str(exp[a]:0, St);
  l := l + Length(St);
  if powers
  then i := 44
  else i := 30;
  i := (i - l) div 2;   {amount to indent by, to center title}
  GoToXY(i, WhereY);
  if powers
  then WriteLn('POWERS OF THE PRIMITIVE ROOT ',
                exp[a]:0, ' (MOD ', p:1, ')')
  else WriteLn('INDICES WITH RESPECT TO THE PRIMITIVE ROOT ',
                exp[a]:0, ' (MOD ', p:1, ')');
  WriteLn;
  Write('    j       10j  10j+1  10j+2  10j+3  10j+4  10j+5');
  WriteLn('  10j+6  10j+7  10j+8  10j+9');
  k := 10*j;
  h := round(condition(k*a, p-1));
  i0 := 19;
  if k + 200 > p
  then i0 := (p - 1 - k) div 10;
  for i := 0 to i0 do
    begin
      Write((j+i):6);
      TextColor(14);
      Write('  ');
      for r := 0 to 9 do
        begin
          if powers
          then
            begin
              if k < p
              then Write(exp[h]:7)
              else Write('       ');
              h := h + a;
              if h >= p - 1 then h := h - p + 1;
            end
          else
            if (k > 0) and (k < p)
            then
              begin
                h := round(condition(b*ind[k], p-1));
                Write(h:7)
              end
            else Write('       ');
          k := k + 1
        end;
      TextColor(15);
      WriteLn
    end;
  TextBackground(7);
  GoToXY(1, 25);
  ClrEoL;
  TextColor(4);
  if p > 200
  then
    begin
      Write('    PgUp    PgDn    J');
      TextColor(0);
      Write('ump');
      TextColor(4)
    end
  else Write('            ');
  if p > 80
  then
    if powers
    then
      begin
        Write('    I');
        TextColor(0);
        Write('ndices')
      end
    else
      begin
        Write('    E');
        TextColor(0);
        Write('xponentials')
      end;
  TextColor(4);
  Write('    M');
  TextColor(0);
  Write('odulus');
  TextColor(4);
  Write('    B');
  TextColor(0);
  Write('ase');
  TextColor(4);
  Write('    P');
  TextColor(0);
  Write('rint');
  TextColor(4);
  Write('    Esc');
  GoToXY(1, 25);             {hide the cursor}
  TextColor(7);
  Write(' ');
  GoToXY(1, 25);
  TextColor(0)
end;                         {of procedure DisplayExp}


procedure PrinTab;          {Print array}

var
St   :  string[20];
l,                     {length of a string}
i0                     {limit of index i}
     :  integer;

begin
  Str(p:0, St);
  l := Length(St);
  Str(exp[a]:0, St);
  l := l + Length(St);
  if powers
  then i0 := 44
  else i0 := 30;
  i0 := (i0 - l) div 2;   {amount to indent by, to center title}
  WriteLn(lst);
  WriteLn(lst);
  for i := 1 to i0 do Write(lst, ' ');
  if powers
  then WriteLn(lst, 'POWERS OF THE PRIMITIVE ROOT ',
                exp[a]:0, ' (MOD ', p:1, ')')
  else WriteLn(lst, 'INDICES WITH RESPECT TO THE PRIMITIVE ROOT ',
                exp[a]:0, ' (MOD ', p:1, ')');
  WriteLn(lst);
  Write(lst, '    j       10j  10j+1  10j+2  10j+3  10j+4  10j+5');
  WriteLn(lst, '  10j+6  10j+7  10j+8  10j+9');
  k := 10*j;
  h := round(condition(k*a, p-1));
  i0 := (p-1) div 10;
  for i := 0 to i0 do
    begin
      Write(lst, (j+i):6);
      Write(lst, '  ');
      for r := 0 to 9 do
        begin
          if powers
          then
            begin
              if k < p
              then Write(lst, exp[h]:7)
              else Write(lst, '       ');
              h := h + a;
              if h >= p - 1 then h := h - p + 1;
            end
          else
            if (k > 0) and (k < p)
            then
              begin
                h := round(condition(b*ind[k], p-1));
                Write(lst, h:7)
              end
            else Write(lst, '       ');
          k := k + 1
        end;
      WriteLn(lst)
    end;
    WriteLn(lst);
    WriteLn(lst);
    WriteLn(lst)
end;                        {of procedure PrinTab}

function GetInput(XCoord, YCoord : integer; prompt, comm : string;
                                         min, max : comp) : comp;

var
x                                    {raw input}
         : extended;
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
  Ch := 'F';
  funckey := false;
  TextColor(14);
  TextBackground(1);
  ClrScr;
  repeat
    WriteLn('Display indices with respect to a primitive root (mod p)');
    p := round(GetInput(WhereX, WhereY, '          Enter p = ',
    ' (must have 2 < p < 9999)',
     1, MaxMod));
    WriteLn('Locating least positive primitive root ... ');
    g := round(primroot(p, 1));
    if g = 0
    then
      begin
        WriteLn(p:1, ' is composite.  Try again ... ');
        Delay(2000)
      end;
  until g > 0;
  NewMod := True;
  Powers := False;
  repeat
    if (not funckey) and ((Ch = 'm') or (Ch = 'M'))
      then
        begin
          repeat
            p := round(GetInput(1, 24, 'Enter new prime modulus p = ',
            '         (must have 0 < p < 9999)',
             1, MaxMod));
            WriteLn('Locating least positive primitive root ... ');
            g := round(primroot(p, 1));
            if g = 0
            then
              begin
                WriteLn(p:1, ' is composite.  Try again.');
                WriteLn;
                Delay(2000)
              end;
          until g > 0;
          NewMod := True
        end;
    if NewMod
    then
      begin
        WriteLn('Calculating the arrays ... ');
        a := 1;
        b := 1;
        r := 1;
        for i := 0 to p - 1 do
          begin
            exp[i] := r;
            ind[r] := i;
            r := round(condition(r*g, p))
          end;
        j := 0;
        NewMod := False
      end;
      if (funckey) and (Ch = #73)          {PgUp}
      then
        begin
          j := j - 20;
          if j < 0 then j := 0
        end;
      if (funckey) and (Ch = #81)          {PgDn}
      then
        begin
          j := j + 20;
          if 10*j + 190 >= p
          then j := (p - 1) div 10 - 19;
          if j < 0 then j := 0;
        end;
      if (not funckey) and (UpCase(Ch) = 'J')
      then
        begin
          j0 := (p - 1) div 10 - 19;
          Str(j0:1, St);
          St := '        (0 ó j ó ' + St + ')';
          j := round(GetInput(WhereX, WhereY, 'Jump to row j = ',
                              St, 0, j0))
        end;
      if (not funckey) and (UpCase(Ch) = 'I')
      then powers := False;
      if (not funckey) and (UpCase(Ch) = 'E')
      then powers := True;
      if (not funckey) and (Upcase(Ch) = 'B')
      then
        begin
          repeat
            g := round(GetInput(WhereX, WhereY,
                       'Choose a new primitive root g = ', '', -9999, 9999));
            g := round(condition(g, p));
            a := ind[g];
            lincon(a, 1, p-1, a1, m1);
            if m1 = 0
            then
              begin
                Write(g:1, ' is not a primitive root (mod ', p:1, ').');
                WriteLn('  Try again.');
                Delay(2000)
              end;
          until m1 > 0;
          b := round(a1)
        end;
      TextColor(15);
      TextBackground(1);
      ClrScr;
      if p > 200
      then Display(j);
      if (p > 80) and (p < 200)
      then Display(0);
      if p < 80
      then
        begin
          powers := False;
          Display(0);
          powers := True;
          TextColor(15);
          TextBackground(1);
          GoToXY(1, 13);
          Display(0);
          powers := false
        end;
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
        if (p > 200) and funckey and ((Ch = #73) or (Ch = #81))
        then inputok := true;
        if (not funckey)
          and (Ch in [#27, 'b', 'B', 'm', 'M', 'p', 'P'])
        then inputok := true;
        if (not funckey) and (p > 200) and (Upcase(Ch) = 'J')
        then inputok := true;
        if (not funckey) and (p > 80) and powers and (Upcase(Ch) = 'I')
        then inputok := true;
        if (not funckey) and (p > 80)
           and (not powers) and (Upcase(Ch) = 'E')
        then inputok := true;
      until inputok;
      if UpCase(Ch) = 'P'
      then
        if p < 80
        then
          begin
            GoToXY(1, 25);
            ClrEoL;
            Write('   Sending tables to the printer ... ');
            powers := false;
            printab;
            powers := true;
            printab;
            Write('done.');
            Delay(2000)
          end
        else
          begin
            GoToXY(1, 25);
            ClrEoL;
            Write('   Sending table to the printer ... ');
            PrinTab;
            Write('done.');
            Delay(2000)
          end;
  Until (not funckey) and (ch = #27);
TextMode(OriginalMode)
end.

