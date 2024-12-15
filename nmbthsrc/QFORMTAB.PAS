program QFormTab;                {Construct a table of reduced binary
                                  quadratic forms of specified discriminant}
uses crt, printer;

const
Limit = 5000;                      {Maximum number of forms that
                                    can be stored}
MaxD = 1000000000;                 {Must have ³d³ < MaxD}
PrinterLimit = 600;                {maximum number of lines in
                                    printed table}
var
a0, b0, c0,                        {coefficients to be tested}
d,                                 {the discriminant}
g1,                                {gcd}
h,                                 {the number of primitive classes}
h1,                                {total number of reduced forms}
ALim,                              {upper limit of values of a}
BMax,                              {upper limit of values of b}
BMin,                              {lower limit of values of b}
t                                  {temporary variable}
             : longint;
a, b, g      : array[1..Limit] of integer;
c            : array[1..Limit] of longint;
i, j, k,                           {indices}
i0,                                {upper limit on index i}
l,                                 {length of string}
ind                                {amount to indent by}
             : integer;
OriginalMode : word;
Ch           : char;
St, St1      : string[80];
FuncKey,
InputOk      : Boolean;

function GetDisc(XCoord, YCoord : integer; prompt : string;
                  min, max : longint) : longint;
var
x,                          {raw input}
t                           {temporary variable}
             : longint;
x0                          {x coordinate of cursor when ready
                             to read input}
             : integer;
InputOk      : Boolean;
St1, St2     : string[80];

begin                    {body of function GetDisc}
  Str(MaxD:1, St1);
  St1 := '        (Must have 0 < ³d³ ó ' + St1 + ',';
  St2 := '         d ð 0 or 1 (mod 4), d  not a square.)';
  GoToXY(XCoord, YCoord);
  ClrEoL;
  Write(Prompt);
  x0 := WhereX;
  WriteLn;
  ClrEoL;
  WriteLn(St1);
  ClrEoL;
  Write(St2);
  GoToXY(x0, WhereY - 2);
  repeat
    ReadLn(x);
    InputOk := True;
    if frac(x) > 0 then InputOk := False;
    if x < min then InputOk := False;
    if x > max then InputOk := False;
    if x = 0
    then InputOk := False;
    if x > 0
    then
      begin
        t := round(Sqrt(x));
        if t*t = x
        then InputOk := False
      end;
    t := x mod 4;
    if t < 0
    then t := t + 4;
    if (t = 2) or (t = 3) then InputOk := False;
    if not InputOk
    then
      begin
        GoToXY(x0, WhereY - 1);
        ClrEoL
      end;
  until InputOk;
  ClrEoL;
  WriteLn;
  ClrEoL;
  GetDisc := x
end;             {of function GetDisc}

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
WriteLn('                                   QFORMTAB');
WriteLn;
Write('           Will construct a table of the reduced binary ');
WriteLn('quadratic forms');
Write('     f(x, y) = axý + bxy + cyý of specified discriminant ');
WriteLn('d = bý - 4ac,');
Write('     where a form is considered to be reduced if ');
WriteLn('³a³ < b ó ³a³ < ³c³ or');
Write('     if 0 ó b ó ³a³ = ³c³.  If d < 0 then all forms are ');
WriteLn('definite.  The');
Write('     form f is positive definite if and only if a > 0.');
WriteLn('  Since -f is negative');
Write('     definite if and only if f ');
WriteLn('is positive definite, we restrict our');
Write('     attention to positive definite ');
WriteLn('forms when d < 0.  Any positive definite');
Write('     form is equivalent to a unique ');
WriteLn('reduced form; thus the reduced forms');
Write('     are inequivalent, and the number ');
WriteLn('of equivalence classes is exactly the');
Write('     number of reduced forms.  However, ');
WriteLn('for d > 0 our reduction process is');
Write('     incomplete, ');
WriteLn('and reduced forms may be equivalent.');
WriteLn;
Write('          The CONTENT of a form is the greatest ');
WriteLn('common divisor of its');
Write('     coefficients.  A form is PRIMITIVE if it has ');
WriteLn('content 1.  The number');
Write('     of equivalence classes of forms of ');
WriteLn('discriminant d is denoted by H(d).');
Write('     The number of equivalence classes of ');
WriteLn('primitive forms is denoted');
Write('     by h(d).  These ''class numbers'' are ');
WriteLn('determined only when d < 0.');
WriteLn;
Write('          The running time is proportional to ³d³, so start with ');
WriteLn('a small d.');
d := GetDisc(WhereX, WhereY, '     Enter discriminant, d = ', -MaxD, MaxD);
Write('                           Calculating . . . ');
Ch := 'F';
funckey := false;
repeat
  if (funckey) and (Ch = #73) and (j > 1)     {PgUp}
  then
    begin
      j := j - 40;
      if j < 1
      then j := 1;
    end;
  if (funckey) and (Ch = #81) and (j + 39 < h1)     {PgDn}
  then
    begin
      j := j + 40;
      if j + 39 > h1
      then j := h1 - 39;
    end;
  if (not FuncKey) and (UpCase(Ch) = 'D')
  then
    begin
      d := GetDisc(1, 23, ' Enter new discriminant, d = ', -MaxD, MaxD);
      Write('                             Calculating . . . ')
    end;



  if (not FuncKey) and (UpCase(Ch) IN ['F', 'D'])
  then
    begin
      j := 1;
      h := 0;
      h1 := 0;
      if d < 0
      then
        begin
          ALim := round(Sqrt(-d/3));
          for a0 := 1 to ALim do
            begin
              t := 4*a0*a0 + d;
              if t > 0
              then
                begin
                  BMin := round(Sqrt(t));
                  if BMin*BMin < t
                  then BMin := BMin + 1
                end
              else BMin := 0;
              for b0 := BMin to a0 do
                begin
                  t := b0*b0 - d;
                  if (t mod (4*a0)) = 0
                  then
                    begin
                      c0 := t div (4*a0);
                      h1 := h1 + 1;
                      g1 := gcd(a0, b0);
                      g1 := gcd(g1, c0);
                      if g1 = 1 then h := h + 1;
                      if h1 <= Limit
                      then
                        begin
                          a[h1] := a0;
                          b[h1] := b0;
                          c[h1] := c0;
                          g[h1] := g1
                        end;
                      if (b0 > 0) and (b0 < a0) and (c0 > a0)
                      then
                        begin
                          h1 := h1 + 1;
                          if g1 = 1 then h := h + 1;
                          if h1 <= Limit
                          then
                            begin
                              a[h1] := a0;
                              b[h1] := -b0;
                              c[h1] := c0;
                              g[h1] := g1
                            end;
                        end;
                    end;
                end;
            end;
        end
      else
        begin
          ALim := round(Sqrt(d)/2);
          if ALim*ALim > d/4
          then ALim := ALim - 1;
          for a0 := 1 to ALim do
            begin
              t := d - 4*a0*a0;
              if t < a0*a0
              then
                begin
                  BMax := round(Sqrt(t));
                  if BMax*BMax > t
                  then BMax := BMax -1;
                end
              else BMax := a0;
              for b0 := 0 to BMax do
                begin
                  t := d - b0*b0;
                  if (t mod (4*a0)) = 0
                  then
                    begin
                      c0 := -(t div (4*a0));
                      h1 := h1 + 1;
                      g1 := gcd(a0, b0);
                      g1 := gcd(g1, c0);
                      if g1 = 1 then h := h + 2;
                      if h1 <= Limit
                      then
                        begin
                          a[h1] := a0;
                          b[h1] := b0;
                          c[h1] := c0;
                          g[h1] := g1
                        end;
                      h1 := h1 + 1;
                      if h1 <= Limit
                      then
                        begin
                          a[h1] := -a0;
                          b[h1] := b0;
                          c[h1] := -c0;
                          g[h1] := g1
                        end;
                      if (b0 > 0) and (b0 < a0) and (-c0 <> a0)
                      then
                        begin
                          h1 := h1 + 1;
                          if g1 = 1 then h := h + 2;
                          if h1 <= Limit
                          then
                            begin
                              a[h1] := a0;
                              b[h1] := -b0;
                              c[h1] := c0;
                              g[h1] := g1
                            end;
                          h1 := h1 + 1;
                          if h1 <= Limit
                          then
                            begin
                              a[h1] := -a0;
                              b[h1] := -b0;
                              c[h1] := -c0;
                              g[h1] := g1
                            end;
                        end
                    end
                end
            end
        end
    end;
  TextColor(14);
  TextBackground(1);
  ClrScr;
  Str(d:1, St);
  l := length(St);
  ind := (15 - l) div 2;
  for i := 1 to ind do Write(' ');
  if d > 0
  then for i := 1 to 3 do Write(' ');
  Write('REDUCED ');
  if d < 0
  then Write('POSITIVE DEFINITE ')
  else Write('INDEFINITE ');
  WriteLn('BINARY QUADRATIC FORMS OF DISCRIMINANT ', d:1);
  if d < 0
  then
    begin
      l := 2*l;
      Str(h:1, St);
      l := l + length(St);
      Str(h1:1, St);
      l := l + length(St);
      ind := (56 - l) div 2;
      for i := 1 to ind do Write(' ');
      WriteLn('h(', d:1, ') = ', h:1, '          ', 'H(', d:1, ') = ', h1:1);
      WriteLn
    end
  else
    begin
      St := 'There are ';
      Str(h1:1, St1);
      St := St + St1 + ' reduced forms,';
      if h = h1
      then
        begin
          if h = 2
          then St := St + ' both'
          else St := St + ' all';
          St := St + ' of them primitive.'
        end
      else
        begin
          St := St + ' of which ';
          Str(h:1, St1);
          St := St + St1 + ' are primitive.'
        end;
      ind := (78 - length(St)) div 2;
      for i := 1 to ind do Write(' ');
      WriteLn(St);
      Write('            ');
      WriteLn('These reduced forms are not necessarily inequivalent.')
    end;
  Write('   a         b          c    content');
  if h1 > 1
  then WriteLn('       a         b          c    content')
  else WriteLn;
  k := j;
  for i := 0 to 19 do
    begin
      if k <= h1 then Write(a[k]:5, b[k]:10, c[k]:11, g[k]:7);
      k := k + 1;
      if k <= h1 then WriteLn(a[k]:12, b[k]:10, c[k]:11, g[k]:7);
      k := k + 1
    end;
  TextBackground(7);
  GoToXY(1, 25);
  ClrEoL;
  TextColor(4);
  if j > 1
  then Write('       PgUp')
  else Write('           ');
  if j + 39 < h1
  then Write('            PgDn')
  else Write('                ');
  Write('            d            P');
  TextColor(0);
  Write('rint');
  TextColor(4);
  Write('            Esc');
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
    InputOk := False;
    if FuncKey and (Ch = #73) and (j > 1)
    then InputOk := True;
    if FuncKey and (Ch = #81) and (j + 39 < h1)
    then InputOk := True;
    if (not FuncKey) and (Ch IN [#27, 'd', 'D', 'p', 'P'])
    then InputOk := True
  until InputOk;
  if (not FuncKey) and (UpCase(Ch) = 'P')      {Print}
  then
    begin
      ClrEoL;
      Write('               Printing . . . ');
      Str(d:1, St);
      l := length(St);
      ind := (15 - l) div 2;
      for i := 1 to ind do Write(lst, ' ');
      if d > 0
      then for i := 1 to 3 do Write(lst, ' ');
      Write(lst, 'REDUCED ');
      if d < 0
      then Write(lst, 'POSITIVE DEFINITE ')
      else Write(lst, 'INDEFINITE ');
      WriteLn(lst, 'BINARY QUADRATIC FORMS OF DISCRIMINANT ', d:1);
      if d < 0
      then
        begin
          l := 2*l;
          Str(h:1, St);
          l := l + length(St);
          Str(h1:1, St);
          l := l + length(St);
          ind := (56 - l) div 2;
          for i := 1 to ind do Write(lst, ' ');
          Write(lst, 'h(', d:1, ') = ', h:1);
          WriteLn(lst, '          ', 'H(', d:1, ') = ', h1:1);
          WriteLn(lst);
        end
      else
        begin
          St := 'There are ';
          Str(h1:1, St1);
          St := St + St1 + ' reduced forms,';
          if h = h1
          then
            begin
              if h = 2
              then St := St + ' both'
              else St := St + ' all';
              St := St + ' of them primitive.'
            end
          else
            begin
              St := St + ' of which ';
              Str(h:1, St1);
              St := St + St1 + ' are primitive.'
            end;
          ind := (78 - length(St)) div 2;
          for i := 1 to ind do Write(lst, ' ');
          WriteLn(lst, St);
          Write(lst, '            ');
          Write(lst, 'These reduced forms ');
          WriteLn(lst, 'are not necessarily inequivalent.')
        end;
      Write(lst, '   a         b          c    content');
      if h1 > 1
      then WriteLn(lst, '       a         b          c    content')
      else WriteLn(lst);
      i0 := (h1 + 1) div 2;
      if i0 > PrinterLimit
      then i0 := PrinterLimit;
      k := 0;
      for i := 1 to i0 do
        begin
          k := k + 1;
          if k <= h1 then Write(lst, a[k]:5, b[k]:10, c[k]:11, g[k]:7);
          k := k + 1;
          if k <= h1 then WriteLn(lst, a[k]:12, b[k]:10, c[k]:11, g[k]:7)
        end;
      WriteLn(lst);
      Write('done.')
    end;
until (not FuncKey) and (Ch = #27);
TextMode(OriginalMode)
end.