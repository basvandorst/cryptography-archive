program Reduce;  {Reduce a binary quadratic form axý + bxy + cyý,
                  and determine related information}
{$N+,E+}
uses nothy, CRT, printer;
{$I getinput.i }

const
TableSize = 400;

type
Matrix   = array[1..2] of array[1..2] of comp;
RMatrix  = array[1..2] of array[1..2] of extended;
Line     = string[80];

var
a0, b0, c0,                      {coefficients of the given form}
a1, b1, c1,                      {coefficients of subsequent forms}
z,                               {temporary variable used in swapping, etc.}
m0                               {multiple of 2a to add to b}
                 :     comp;
ra, rb, rc,                      {coeffs as floating point real numbers}
rm0,                             {m0 as a real number}
x                                {raw input}
                 :     extended;
M,                               {M takes g to f, regardles of the value
                                  of Mftog}
N                                {M or M^(-1), whichever is to be displayed}
                :     Matrix;
RM                               {M with real entries}
                 :     RMatrix;
Ch                               {answers to prompts}
                 :     char;
OriginalMode     :     word;

ind,                             {amount to indent by}
l, l0,                           {lengths of strings}
l1, l2, l3,
code,                            {error level in Val}
x0, y0                           {cursor locations}
                 :     integer;
f, g             :     Line;
Stra,                            {coefficient a0 as a string}
Strx,                            {variable x as a string}
Stry,                            {variable y as a string}
StrDisc                          {discriminant as a string}
                 :     string[25];
St,                              {strings used to display or print lines}
St1,
St2
                 :     Line;
h,                               {a residue class mod TableSize, marking
                                  the old bottom entry}
i,                               {a residue class mod TableSize, marking
                                  the new bottom entry}
j,                               {Top ó j ó Bottom, marks the first line
                                  to be displayed}
k,                               {an index}
Top,                             {top (first) member of the table}
Bottom                           {bottom (last) member of the table}
                 :     longint;
a, b, c,                         {coefficients of forms}
M11, M12, M21, M22,              {elements of matrices}
m1                               {power of T}
                 :     array[0..TableSize-1] of comp;
Op                               {operation performed}
                 :     array[0..TableSize-1] of string[1];

FuncKey,                         {was a function key pressed?}
InputOk,                         {was the key pressed on the menu?}
SizeOk,                          {no overflow}
ParamsSet,                       {were the parameters set
                                  on the command line?}
Mftog,                           {matrix displayed maps f to g}
pos                              {is the discriminant positive?}
                 :     boolean;

procedure S;

begin
  z := a1;
  a1 := c1;
  c1 := z;
  b1 := -b1;
  z := M[2, 1];
  M[2, 1] := M[1, 1];
  M[1, 1] := -z;
  z := M[2, 2];
  M[2, 2] := M[1, 2];
  M[1, 2] := -z
end;                                 {of S}

procedure S1;                        {transformation S recorded in arrays}
  begin
    h := Bottom mod TableSize;
    Bottom := Bottom + 1;
    i := Bottom mod TableSize;
    if Bottom = Top + TableSize
    then Top := Top + 1;
    if j + 5 < Bottom
    then j := Bottom - 5;
    a[i] := c[h];
    c[i] := a[h];
    b[i] := -b[h];
    M11[i] := -M21[h];
    M21[i] := M11[h];
    M12[i] := -M22[h];
    M22[i] := M12[h];
    Op[i] := 'S';
    m1[i] := 1
  end;                               {of S1}

procedure T;

begin
  SizeOk := True;
  ra := a1;
  rb := b1;
  rc := c1;
  rm0 := m0;
  rc :=(ra*rm0 + rb)*rm0 + rc;
  rb := rb + 2*rm0*ra;
  for i := 1 to 2 do
    for j := 1 to 2 do RM[i, j] := M[i, j];
  RM[1, 1] := RM[1, 1] - rm0*RM[2, 1];
  RM[1, 2] := RM[1, 2] - rm0*RM[2, 2];
  if Abs(rc) > MaxAllow then SizeOk := False;
  if Abs(rb) > MaxAllow then SizeOk := False;
  for j := 1 to 2 do if Abs(RM[1, j]) > MaxAllow then SizeOk := False;
  if SizeOk
  then
    begin
      b1 := rb;
      c1 := rc;
      for j := 1 to 2 do M[1, j] := RM[1, j]
    end
end;                                 {of T}

procedure T1;                        {transform T^m0 performed on arrays}
  begin
    h := Bottom mod TableSize;
    SizeOk := True;
    ra := a[h];
    rb := b[h];
    rc := c[h];
    rc := (ra*m0 + rb)*m0 + rc;
    rb := rb + 2*m0*ra;
    RM[1,1] := M11[h];
    RM[1,2] := M12[h];
    RM[2,1] := M21[h];
    RM[2,2] := M22[h];
    RM[1, 1] := RM[1, 1] - m0*RM[2, 1];
    RM[1, 2] := RM[1, 2] - m0*RM[2, 2];
    if Abs(rc) > MaxAllow then SizeOk := False;
    if Abs(rb) > MaxAllow then SizeOk := False;
    for k := 1 to 2 do
      if Abs(RM[1, k]) > MaxAllow
      then SizeOk := False;
    if SizeOk
    then
    begin
      Bottom := Bottom + 1;
      i := Bottom mod TableSize;
      if Bottom = Top + TableSize
      then Top := Top + 1;
      if j + 5 < Bottom
      then j := Bottom - 5;
      a[i] := a[h];
      b[i] := rb;
      c[i] := rc;
      M11[i] := RM[1,1];
      M12[i] := RM[1,2];
      M21[i] := RM[2,1];
      M22[i] := RM[2,2];
      Op[i] := 'T';
      m1[i] := m0;
    end
  end;                               {of T1}

function WriteForm(a1, b1, c1 : comp) : Line;
var
St, St1 : Line;

begin
  St := '';
  if a1 < 0
  then St := ' - ';
  if Abs(a1) > 1
  then
    begin
      Str(Abs(a1):1:0, St1);
      St := St + St1
    end;
  if a1 <> 0
  then St := St + 'xý';
  if (b1 > 0) and (a1 <> 0)
  then St := St + ' + ';
  if b1 < 0
  then St := St + ' - ';
  if Abs(b1) > 1
  then
    begin
      Str(Abs(b1):1:0, St1);
      St := St + St1
    end;
  if b1 <> 0 then St := St + 'xy';
  if (c1 > 0) and ((a1 <> 0) or (b1 <> 0))
  then St := St + ' + '
  else St := St + ' - ';
  if Abs(c1) > 1
  then
    begin
      Str(Abs(c1):1:0, St1);
      St := St + St1
    end;
  if c1 <> 0
  then St := St + 'yý';
  WriteForm := St
end;                                 {of WriteForm}

procedure Display;                   {Display a screenful of data}

var
h, i, i1, k, OffSet  : longint;
Advance              : Boolean;

begin
  TextColor(14);
  TextBackground(1);
  GoToXY(35, 1);
  WriteLn('REDUCE');
  f := WriteForm(a0, b0, c0);
  l := length(f);
  ind := (70 - l) div 2;              {indent by ind to center}
  GoToXY(ind, WhereY);
  WriteLn('f(x, y) = ', f);
  l := length(StrDisc);
  ind := (76 - l) div 2;
  GoToXY(ind, 3);
  WriteLn('d', StrDisc);
  Write('         a                 b                 ');
  Write('c           ');
  if Mftog
  then Write('M:f', #26, 'g')
  else Write('M:g', #26, 'f');
  Write('         Operation');
  i := j - 1;
  GoToXY(1, 5);
  ClrEoL;
  OffSet := 0;
  repeat
    Advance := True;
    i := i + 1;
    k := i mod TableSize;
    if Mftog
    then
      begin
        N[1,1] := M22[k];
        N[1,2] := -M12[k];
        N[2,1] := -M21[k];
        N[2,2] := M11[k]
      end
    else
      begin
        N[1,1] := M11[k];
        N[1,2] := M12[k];
        N[2,1] := M21[k];
        N[2,2] := M22[k]
      end;
    Str(N[1,1]:1:0, St);
    l1 := length(St);
    Str(N[2,1]:1:0, St);
    l := length(St);
    if l > l1
    then l1 := l;            {l1 is now the width needed for the 1st column}
    Str(N[1,2]:1:0, St);
    l2 := length(St);
    Str(N[2,2]:1:0, St);
    l := length(St);
    if l > l2
    then l2 := l;            {l2 is now the width needed for the 2nd column}
    if m1[k] <> 1
    then
      begin
        Str(m1[k]:1:0, St);
        l := length(St) + 1
      end
    else l := 1;
    Str(a[k]:17:0, St2);
    St2 := St2 + ' ';
    Str(b[k]:17:0, St);
    St2 := St2 + St + ' ';
    Str(c[k]:17:0, St);
    St2 := St2 + St;
    l0 := length(St2);
    St1 := '';
    if l0 + l1 + l2 > 74       {coefficients and matrix
                                do not fit on one line}
    then
      begin
        While length(St2) < 80 do St2 := St2 + ' ';
        ClrEoL;
        WriteLn;
        Write(St2);
        Advance := False;
        OffSet := 0;
        St2 := ''
      end
    else for i1 := 1 to (length(St2) - OffSet) do St1 := St1 + ' ';
    St1 := St1 + '  Ú';
    Str(N[1,1]:l1:0, St);
    St1 := St1 + St + ' ';
    Str(N[1,2]:l2:0, St);
    St1 := St1 + St + '¿';
    St2 := St2 + '  À';
    Str(N[2,1]:l1:0, St);
    St2 := St2 + St + ' ';
    Str(N[2,2]:l2:0, St);
    St2 := St2 + St + 'Ù';
    if Advance
    then h := (79 - length(St2) - l) div 2
    else h := 5;
    if ((Op[k] = 'S') or (Op[k] = 'T'))
        and (h < 0)                       {operation does not fit}
    then
      begin
        While length(St1) + OffSet< 80 do St1 := St1 + ' ';
        While length(St2) < 80 do St2 := St2 + ' ';
        Write(St1);
        Write(St2);
        St1 := ' ';
        St2 := Op[k];
        Advance := False
      end
    else
      begin
        for i1 := 1 to h + 1 do St1 := St1 + ' ';
        for i1 := 1 to h do St2 := St2 + ' ';
        St2 := St2 + Op[k]
      end;
    if m1[k] <> 1
    then
      begin
        Str(m1[k]:1:0, St);
        St1 := St1 + St
       end;
    While length(St1) + OffSet < 80 do St1 := St1 + ' ';
    Write(St1);
    if Advance
    then
      begin
        While length(St2) < 80 do St2 := St2 + ' ';
        Write(St2);
        ClrEoL;
        WriteLn;
        ClrEoL;
        OffSet := 0
      end
    else
      begin
        Write(St2);
        ClrEoL;
        OffSet := length(St2);
        ClrEoL
      end;
  until (i = j + 5) or (i = Bottom mod TableSize);
end;                                 {of procedure Display}

procedure Print;                     {Print table}

var
i, i1, k             : longint;
Advance              : Boolean;

begin
  For i := 1 to 35 do Write(lst, ' ');
  WriteLn(lst, 'REDUCE');
  f := WriteForm(a0, b0, c0);
  l := length(f);
  ind := (70 - l) div 2;              {indent by ind to center}
  for i := 1 to ind do Write(lst, ' ');
  WriteLn(lst, 'f(x, y) = ', f);
  l := length(StrDisc);
  ind := (76 - l) div 2;
  for i := 1 to ind do Write(lst, ' ');
  WriteLn(lst, 'd', StrDisc);
  Write(lst, '         a                 b                 ');
  Write(lst, 'c           ');
  if Mftog
  then Write(lst, 'M:f', #26, 'g')
  else Write(lst, 'M:g', #26, 'f');
  Write(lst, '         Operation');
  WriteLn(lst);
  for i := Top to Bottom do
    begin
      WriteLn(lst);
      k := i mod TableSize;
      if Mftog
      then
        begin
          N[1,1] := M22[k];
          N[1,2] := -M12[k];
          N[2,1] := -M21[k];
          N[2,2] := M11[k]
        end
      else
        begin
          N[1,1] := M11[k];
          N[1,2] := M12[k];
          N[2,1] := M21[k];
          N[2,2] := M22[k]
        end;
      Str(N[1,1]:1:0, St);
      l1 := length(St);
      Str(N[2,1]:1:0, St);
      l := length(St);
      if l > l1
      then l1 := l;          {l1 is now the width needed for the 1st column}
      Str(N[1,2]:1:0, St);
      l2 := length(St);
      Str(N[2,2]:1:0, St);
      l := length(St);
      if l > l2
      then l2 := l;          {l2 is now the width needed for the 2nd column}
      if m1[k] <> 1
      then
        begin
          Str(m1[k]:1:0, St);
          l := length(St) + 1
        end
      else l := 1;
      Str(a[k]:17:0, St2);
      St2 := St2 + ' ';
      Str(b[k]:17:0, St);
      St2 := St2 + St + ' ';
      Str(c[k]:17:0, St);
      St2 := St2 + St;
      l0 := length(St2);
      St1 := '';
      Advance := True;
      if l0 + l1 + l2 > 74       {coefficients and matrix
                                  do not fit on one line}
      then
        begin
          While length(St2) < 80 do St2 := St2 + ' ';
          WriteLn(lst);
          Write(lst, St2);
          Advance := False;
          St2 := ''
        end
      else for i1 := 1 to length(St2) do St1 := St1 + ' ';
      St1 := St1 + '  Ú';
      Str(N[1,1]:l1:0, St);
      St1 := St1 + St + ' ';
      Str(N[1,2]:l2:0, St);
      St1 := St1 + St + '¿';
      St2 := St2 + '  À';
      Str(N[2,1]:l1:0, St);
      St2 := St2 + St + ' ';
      Str(N[2,2]:l2:0, St);
      St2 := St2 + St + 'Ù';
      if Advance
      then h := (79 - length(St2) - l) div 2
      else h := 5;
      if ((Op[k] = 'S') or (Op[k] = 'T'))
          and (h < 0)                       {operation does not fit}
      then
        begin
          While length(St1) < 80 do St1 := St1 + ' ';
          While length(St2) < 80 do St2 := St2 + ' ';
          Write(lst, St1);
          Write(lst, St2);
          St1 := ' ';
          St2 := Op[k];
          Advance := False
        end
      else
        begin
          for i1 := 1 to h + 1 do St1 := St1 + ' ';
          for i1 := 1 to h do St2 := St2 + ' ';
          St2 := St2 + Op[k]
        end;
      if m1[k] <> 1
      then
        begin
          Str(m1[k]:1:0, St);
          St1 := St1 + St
         end;
      While length(St1) < 80 do St1 := St1 + ' ';
      Write(lst, St1);
      While length(St2) < 80 do St2 := St2 + ' ';
      Write(lst, St2);
      WriteLn(lst)
    end
end;                                 {of procedure Print}

procedure Disc(a1, b1, c1 : comp);   {Evaluate d and set StrDisc, pos
                                      accordingly.  The calculation of
                                      d is somewhat tricky because ³d³
                                      may be as large as 5ù10^36}
var
i                                    {an index}
                 :     integer;
ra, rb, rc,                          {coefficients as extended reals}
rd                                   {the discriminant as a real number}
                 :     extended;
d0, d1,                              {tentative values of d}
t                                    {a temporary variable in evaluations}
                 :     comp;
Moduli, Discs    :     array[1..2] of comp;

begin
  Moduli[1] := 1000000000000000000.0;
  Moduli[2] := 9999999967.0;
  ra := a1;
  rb := b1;
  rc := c1;
  rd := rb*rb - (4.0)*ra*rc;
  if Abs(rd) > MaxAllow
  then
    begin
      if rd > 0
      then
        begin
          StrDisc := ' > 10^18';
          pos := True
        end
      else
        begin
          StrDisc := ' < -10^18';
          pos := False
        end
    end
  else
    begin
      for i := 1 to 2 do
        begin
          t := mult(a1, c1, Moduli[i]);
          t := mult(4, t, Moduli[i]);
          t := mult(b1, b1, Moduli[i]) - t;
          t := condition(t, Moduli[i]);            {t ð d (mod Moduli[i])}
          Discs[i] := t
        end;
      d1 := condition(Discs[1], Moduli[1]);
      d0 := d1 - Moduli[1];           {if |d| ó 10^18 then d = d0 or d = d1}
      if Discs[2] = condition(d1, Moduli[2])
      then
        begin
          Str(d1:1:0, StrDisc);
          pos := True
        end;
      if Discs[2] = condition(d0, Moduli[2])
      then
        begin
          Str(d0:1:0, StrDisc);
          pos := False
        end;
      StrDisc := ' = ' + StrDisc
    end
end;                                 {of procedure Disc}


begin                                {main}
  ParamsSet := False;
  if ParamCount = 3
  then
    begin
      ParamsSet := True;
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0) and (Abs(x) <= MaxAllow)
      then a0 := x
      else ParamsSet := False;
      if a0 = 0
      then ParamsSet := False;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0) and (Abs(x) <= MaxAllow)
      then b0 := x
      else ParamsSet := False;
      Val(ParamStr(3), x, code);
      if (frac(x) = 0) and (code = 0) and (Abs(x) <= MaxAllow)
      then c0 := x
      else ParamsSet := False;
      if c0 = 0
      then ParamsSet := False
    end;
  if ParamsSet
  then
    begin
      M[1, 1] := 1;
      M[1, 2] := 0;
      M[2, 1] := 0;
      M[2, 2] := 1;
      a1 := a0;
      b1 := b0;
      c1 := c0;
      repeat
        if (Abs(c1) < Abs(a1))
            or ((Abs(c1) = Abs(a1)) and (Abs(a1) <= b1) and (b1 < 0))
        then S;
        WriteLn(a1:22:0, b1:22:0, c1:22:0, '  S');
        if ((b1 > Abs(a1)) or (b1 <= -Abs(a1))) and (a1 <> 0)
        then
          begin
            m0 := int((1-b1/a1)/2);
            if b1 + 2*a1*m0 > abs(a1)
            then
              begin
                if a1 > 0
                then m0 := m0 - 1
                else m0 := m0 + 1
              end;
            if b1 + 2*a1*m0 <= -Abs(a1)
            then
              begin
                if a1 > 0
                then m0 := m0 + 1
                else m0 := m0 - 1
              end;
            T;
            if not SizeOk
            then
              begin
                WriteLn('Reduction aborted because of overflow.');
                Halt
              end
          end;
        WriteLn(a1:22:0, b1:22:0, c1:22:0, '  T', m0:1:0);
        repeat
        until KeyPressed
      until (Abs(c1) > Abs(a1))
             or ((Abs(c1) = Abs(a1)) and (b1 >= 0) and (b1 <= Abs(a1)))
             or (a1 = 0);
      WriteLn('     The form');
      f := WriteForm(a0, b0, c0);
      l := length(f);
      ind := (70 - l) div 2;              {indent by ind to center}
      GoToXY(ind, WhereY);
      WriteLn('f(x, y) = ', f);
      Write('is equivalent to the ');
      if a1 <> 0 then Write('reduced ');
      WriteLn('form');
      g := WriteForm(a1, b1, c1);
      l := length(g);
      ind := (70 - l) div 2;
      GoToXY(ind, WhereY);
      WriteLn('g(x, y) = ', g, '.');
      if a1 <> 0
      then
        begin
          Disc(a1, b1, c1);
          WriteLn('These forms have discriminant d ', StrDisc, '.');
          Write('     The leading coefficient of f');
          WriteLn(' can be properly represented by g:');
          Str(a0:1:0, Stra);
          l1 := length(Stra);
          Str(M[1, 1]:1:0, Strx);
          l2 := length(Strx);
          Str(M[2, 1]:1:0, Stry);
          l3 := length(Stry);
          ind := (71 - l1 - l2 - l3) div 2;
          GoToXY(ind, WhereY);
          WriteLn(Stra, ' = g(', Strx, ', ', Stry, ').')
        end
      else
        begin
          Write('This form is not reduced, but reduction');
          WriteLn(' is inaapropriate here becuase the');
          Write('discriminant of ');
          Write('these forms is a perfect square: ');
          WriteLn('d = ', Abs(b1):1:0, 'ý.')
        end;
    end
  else
    begin
      OriginalMode := LastMode;
      TextBackground(1);
      TextColor(14);
      ClrScr;
      WriteLn('   Will reduce, or otherwise manipulate, a given binary ');
      Write('quadratic form axý + bxy + cyý.  Enter the initial');
      WriteLn(' coefficients:');
      a0 := GetInput(WhereX, WhereY, '         a = ',
            'where ³a³ ó 1000000000000000000.', -MaxAllow, MaxAllow);
      b0 := GetInput(WhereX, WhereY, '         b = ',
            'where ³b³ ó 1000000000000000000.', -MaxAllow, MaxAllow);
      c0 := GetInput(WhereX, WhereY, '         c = ',
            'where ³c³ ó 1000000000000000000.', -MaxAllow, MaxAllow);
      Mftog := True;
      Ch := 'O';
      Top := 0;
      Bottom := 0;
      a[0] := a0;
      b[0] := b0;
      c[0] := c0;
      M11[0] := 1;
      M12[0] := 0;
      M21[0] := 0;
      M22[0] := 1;
      Op[0]  := '';
      m1[0] := 1;
      j := 0;
      Disc(a0, b0, c0);
      ClrScr;
      SizeOk := True;
      Ch := 'O';
      Display;
      repeat
        if (FuncKey) and (Ch = #73)                   {PgUp}
        then
          begin
            j := j - 5;
            if j < Top
            then j := Top;
            Display;
            GoToXY(1, 24);
            ClrEoL
          end;
        if (FuncKey) and (Ch = #81)                   {PgDn}
        then
          begin
            j := j + 5;
            if j + 5 > Bottom
            then j := Bottom - 5;
            Display;
            GoToXY(1, 24);
            ClrEoL
          end;
        if (not FuncKey) and (UpCase(Ch) = 'A')       {enter new a}
        then
          begin
            a0 := GetInput(1, 24, 'Enter new coefficient a = ',
            '             where ³a³ ó 1000000000000000000.',
            -MaxAllow, MaxAllow);
            Top := 0;
            Bottom := 0;
            a[0] := a0;
            b[0] := b0;
            c[0] := c0;
            M11[0] := 1;
            M12[0] := 0;
            M21[0] := 0;
            M22[0] := 1;
            Op[0] := '';
            m1[0] := 1;
            j := 0;
            Disc(a0, b0, c0);
            ClrScr;
            SizeOk := True;
            Display;
            GoToXY(1, 24);
            ClrEoL
          end;
        if (not FuncKey) and (UpCase(Ch) = 'B')       {enter new b}
        then
          begin
            b0 := GetInput(1, 24, 'Enter new coefficient b = ',
            '             where ³b³ ó 1000000000000000000.',
            -MaxAllow, MaxAllow);
            Top := 0;
            Bottom := 0;
            a[0] := a0;
            b[0] := b0;
            c[0] := c0;
            M11[0] := 1;
            M12[0] := 0;
            M21[0] := 0;
            M22[0] := 1;
            Op[0] := '';
            m1[0] := 1;
            j := 0;
            Disc(a0, b0, c0);
            ClrScr;
            SizeOk := True;
            Display;
            GoToXY(1, 24);
            ClrEoL
          end;
        if (not FuncKey) and (UpCase(Ch) = 'C')       {enter new c}
        then
          begin
            c0 := GetInput(1, 24, 'Enter new coefficient c = ',
            '             where ³c³ ó 1000000000000000000.',
            -MaxAllow, MaxAllow);
            Top := 0;
            Bottom := 0;
            a[0] := a0;
            b[0] := b0;
            c[0] := c0;
            M11[0] := 1;
            M12[0] := 0;
            M21[0] := 0;
            M22[0] := 1;
            Op[0] := '';
            m1[0] := 1;
            j := 0;
            Disc(a0, b0, c0);
            ClrScr;
            SizeOk := True;
            Display;
            GoToXY(1, 24);
            ClrEoL
          end;
        if (not FuncKey) and (UpCase(Ch) = 'R')       {Reduce}
        then
          begin
            SizeOk := True;
            i := Bottom mod TableSize;
            repeat
              if (Abs(c[i]) < Abs(a[i]))
                or ((Abs(c[i]) = Abs(a[i]))
                and (Abs(a[i]) <= b[i]) and (b[i] < 0))
              then
                begin
                  S1;
                  Display
                end;
              if ((b[i] > Abs(a[i]))
                or (b[i] <= -Abs(a[i]))) and (a[i] <> 0)
              then
                begin
                  m0 := int((1-b[i]/a[i])/2);
                  if b[i] + 2*a[i]*m0 > abs(a[i])
                  then
                    begin
                      if a[i] > 0
                      then m0 := m0 - 1
                      else m0 := m0 + 1
                    end;
                  if b[i] + 2*a[i]*m0 <= -Abs(a[i])
                  then
                    begin
                      if a[i] > 0
                      then m0 := m0 + 1
                      else m0 := m0 - 1
                    end;
                  T1;
                  if SizeOk
                  then Display;
                end;
            until (Abs(c[i]) > Abs(a[i]))
             or ((Abs(c[i]) = Abs(a[i]))
             and (b[i] >= 0) and (b[i] <= Abs(a[i])))
             or (a[i] = 0) or (not SizeOk);
            GoToXY(1, 24);
            TextColor(4);
            TextBackground(7);
            ClrEoL;
            if (not SizeOk)
            then
              begin
                Write('        Reduction abandoned because continuation');
                Write(' would lead to overflow.')
              end
            else
              begin
                if (pos) and (a[i] <> 0)
                then
                  begin
                    Write('The bottom form is reduced, but');
                    Write(' it may be equivalent to other reduced forms.')
                  end;
                if a[i] = 0
                then
                  begin
                    Write('Reduction is inappropriate because d is a ');
                    Write('perfect square: d = ', Abs(b[i]):1:0, 'ý.')
                  end;
                if not pos
                then
                  begin
                    Write('                 The bottom form is ');
                    TextColor(20);
                    Write('reduced');
                    TextColor(4);
                    Write('.')
                  end
              end
          end;
        if (not FuncKey) and (UpCase(Ch) = 'S')       {transform S}
        then
          begin
            SizeOk := True;
            S1;
            Display;
            GoToXY(1, 24);
            ClrEoL
          end;
        if (not FuncKey) and (Upcase(Ch) = 'T')       {transform T}
        then
          begin
            m0 := 1;
            T1;
            if SizeOk
            then
              begin
                Display;
                GoToXY(1, 24);
                ClrEoL
              end
            else
              begin
                GoToXY(1, 24);
                TextColor(4);
                TextBackground(7);
                ClrEoL;
                Write('                Cannot perform T because');
                Write(' it would cause overflow.')
              end
          end;
        if (not FuncKey) and (UpCase(Ch) = 'I')
        then
          begin
            m0 := -1;
            T1;
            if SizeOk
            then
              begin
                Display;
                GoToXY(1, 24);
                ClrEoL
              end
            else
              begin
                GoToXY(1, 24);
                TextColor(4);
                TextBackground(7);
                ClrEoL;
                Write('              Cannot perform T^(-1) because');
                Write(' it would cause overflow.')
              end
          end;
        if (not FuncKey) and (UpCase(Ch) = 'M')
        then
          begin
            Mftog := not Mftog;
            Display;
            GoToXY(1, 24);
            ClrEoL
          end;
        if (not FuncKey) and (UpCase(Ch) = 'P')
        then
          begin
            GoToXY(1, 24);
            TextColor(0);
            TextBackground(7);
            ClrEoL;
            Write('                    Sending the table to the ');
            Write('printer . . . ');
            Print;
            Write('done.')
          end;
        GoToXY(1, 25);
        TextBackground(7);
        ClrEoL;
        TextColor(4);
        if j > Top
        then Write(' PgUp')
        else Write('     ');
        if j + 5 < Bottom
        then Write('   PgDn')
        else Write('       ');
        Write('    a   b   c    R');
        TextColor(0);
        Write('educe   ');
        TextColor(4);
        Write('S   T   I');
        TextColor(0);
        Write('nverse T    ');
        TextColor(4);
        Write('M');
        TextColor(0);
        if Mftog
        then Write(':g', #26, 'f    ')
        else Write(':f', #26, 'g    ');
        TextColor(4);
        Write('P');
        TextColor(0);
        Write('rint   ');
        TextColor(4);
        Write('Esc');
        GoToXY(1, 25);                     {hide the cursor}
        TextColor(7);
        Write(' ');
        GoToXY(1, 25);
        TextColor(14);
        TextBackground(1);
        repeat
          Ch := ReadKey;
          if Ch <> #0
          then FuncKey := False
          else
            begin
              FuncKey := True;
              Ch := ReadKey
            end;
          InputOk := false;
          if (FuncKey) and (Ch = #73) and (j > Top)       {PgUp}
          then InputOk := True;
          if (FuncKey) and (Ch = #81) and (j + 5 < Bottom){PgDn}
          then InputOK := True;
          if (not FuncKey) and (Ch = #27)
          then InputOk := True;
          if (not FuncKey) and (Upcase(Ch) IN
              ['A', 'B', 'C', 'P', 'R', 'S', 'T', 'I', 'M'])
          then InputOk := True
        until InputOk;
      until (not FuncKey) and (Ch = #27);
      TextMode(OriginalMode)
    end
end.
