program ClaNoTab;   {Construct a table of class numbers of positive
                     definite binary quadratic forms}
uses crt, printer;

const
LastD = 10000;
Limit = 2500;                      {MUST have Limit = LastD/4.}
PrinterLimit = 600;                {maximum number of lines in
                                    printed table}
var
a, b, c,                           {coefficients of the quadratic form}
d,                                 {the discriminant}
g, g1,                             {gcd's}
r,                                 {b = 2r or 2r + 1}
m                                  {d = -4m or -4m + 1}
             : longint;
h1odd, h2odd,
h1even, h2even
             : array[0..Limit] of integer;
i, i0, j                           {indices}
             : integer;
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

function gcd(a, b : longint) : longint;
var
t     : longint;

begin
  a := Abs(a);
  b := Abs(b);
  while b > 0 do
    begin
      t := b;
      b := a mod b;
      a := t
    end;
  gcd := a
end;                         {of function gcd}

begin                        {main body}
OriginalMode := LastMode;
TextBackground(1);
TextColor(14);
ClrScr;
WriteLn;
Write('           Will construct a table of the ');
WriteLn('class numbers of');
WriteLn('     positive definite binary quadratic forms.');
WriteLn;
WriteLn('     H(d) denotes the total number of equivalence classes');
WriteLn('     of forms axý + bxy + cyý of discriminant d = bý - 4ac.');
WriteLn;
WriteLn('     h(d) denotes the number of primitive classes, for which ');
WriteLn('     gcd(a, b, c) = 1.');
WriteLn;
WriteLn('     Values are being calculated for -', LastD:1, ' < d < 0.');
Write('     This may take several minutes, so please be patient ... ');
FillChar(h1odd, 2*Limit + 2, 0);
FillChar(h2odd, 2*Limit + 2, 0);
FillChar(h1even, 2*Limit + 2, 0);
FillChar(h2even, 2*Limit + 2, 0);
b := 1;     {starting with odd class numbers, b = 1}
a := 1;
r := 0;
m := 1;     {m = ac - rý - r}
while m <= Limit do
  begin                  {increasing values of b}
    while m <= Limit do
      begin              {increasing values of a}
        c := a;
        g1 := gcd(a, b);
        while m <= Limit do
          begin          {increasing values of c}
            g := gcd(g1, c);
            if (a < c) and (b < a)
            then h2odd[m] := h2odd[m] + 2
            else h2odd[m] := h2odd[m] + 1;
            if g = 1
            then
              if (a < c) and (b < a)
              then h1odd[m] := h1odd[m] + 2
              else h1odd[m] := h1odd[m] + 1;
            c := c + 1;
            m := m + a
          end;
        a := a + 1;
        m := a*a - r*(r+1);
      end;
    r := r + 1;
    b := b + 2;
    a := b;
    m := a*a - r*(r + 1);
  end;
a := 1;       {now b = 0, m = ac}
m := 1;
  while m <= Limit do        {increasing values of a}
    begin
      while m <= Limit do
        begin                {increasing values of c}
          g := gcd(a, c);
          h2even[m] := h2even[m] + 1;
          if g = 1 then h1even[m] := h1even[m] + 1;
          m := m + a;
          c := c + 1
        end;
      a := a + 1;
      m := a*a;
      c := a
    end;
r := 1;                     {now b even, b <> 0}
a := 2;
m := 3;
b := 2;
while m <= Limit do         {increasing values of b, b = 2r}
  begin
    while m <= Limit do     {increasing values of a}
      begin
        c := a;
        g1 := gcd(a, b);
        while m <= Limit do    {increasing values of c}
          begin
            g := gcd(g1, c);
            if (a < c) and (b < a)
            then h2even[m] := h2even[m] + 2
            else h2even[m] := h2even[m] + 1;
            if g = 1
            then
              if (a < c) and (b < a)
              then h1even[m] := h1even[m] + 2
              else h1even[m] := h1even[m] + 1;
            c := c + 1;
            m := m + a
          end;
        a := a + 1;
        m := a*a - r*r
      end;
    r := r + 1;
    b := b + 2;
    a := b;
    m := a*a - r*r
  end;                   {arrays complete}
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
    end;
  if (funckey) and (Ch = #81)     {PgDn}
  then
    begin
      j := j + 20;
      if j > Limit - 19 then j := limit - 19;
      if j < 0 then j := 0
    end;
  if (not funckey) and (Upcase(Ch) = 'J')
  then
    begin
      Str(LastD:1, St);
      St := '                             (-' + St + ' ó d < 0)';
      j := round(GetInput(1, 24, 'Jump to discriminants near d = -',
                 St, 1, LastD)) div 4 - 10;
      if j < 1
      then j := 1;
      if j > Limit - 19
      then j := Limit - 19;
    end;
  TextColor(14);
  TextBackground(1);
  ClrScr;
  Write('          CLASS NUMBERS OF POSITIVE DEFINITE');
  WriteLn(' BINARY QUADRATIC FORMS');
  WriteLn;
  Write('            d      h(d)     H(d)');
  WriteLn('                d     h(d)     H(d)');
  for i := j to j + 19 do
    begin
      Write(-(4*i-1):14, h1odd[i]:8, h2odd[i]:9, '          ');
      WriteLn(-4*i:9, h1even[i]:7, h2even[i]:9)
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
      Write(lst, '          CLASS NUMBERS OF POSITIVE DEFINITE');
      WriteLn(lst, ' BINARY QUADRATIC FORMS');
      WriteLn(lst);
      Write(lst, '            d      h(d)     H(d)');
      WriteLn(lst, '                d     h(d)     H(d)');
      i0 := Limit;
      if i0 > PrinterLimit
      then i0 := PrinterLimit;
      for i := 1 to i0 do
        begin
          Write(lst, -(4*i-1):14, h1odd[i]:8, h2odd[i]:9, '          ');
          WriteLn(lst, -4*i:9, h1even[i]:7, h2even[i]:9)
        end;
      WriteLn(lst)
    end;
until (not funckey) and (Ch = #27);
TextMode(OriginalMode)
end.