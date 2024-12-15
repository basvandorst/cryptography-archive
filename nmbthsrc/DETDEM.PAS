program DetDem;          {Demonstrate the calculation of the determinant
                          (mod m)}
{$N+,E+}
uses CRT;
{$I GetInput.i }

type
Matrix            =  array[1..9] of array[1..9] of comp;

var
d,
m,                              {the modulus}
t                               {a temporary variable}
                  : comp;
f,                              {field width for displaying elements}
i, j, k, l,
n,                              {the dimension of the matrix}
ti                              {temporary integral variable}
                  : integer;
A                 : Matrix;
s                 : string[30]; {for handling format of matrix elements}
OriginalMode      : word;
Rev               : Boolean;    {Reverse video?}

procedure display(A : matrix; mi0, mi1, mc : integer);

var
i, j              : integer;

begin
Rev := False;
ClrScr;
WriteLn;
for i := 1 to n do
  begin
    Write('    ');
    if (i = mi0) or (i = mi1)
    then
      begin
        TextColor(1);
        TextBackground(7);
        Rev := True
      end;
    for j := 1 to n do
      begin
        Write('  ');
        if j = mc
        then
          if Rev
          then
            TextColor(4)
          else
            begin
              TextColor(1);
              TextBackground(7)
            end;
        str(Abs(A[i, j]):1:0, s);
        if length(s) <= f
        then Write(A[i, j]:f:0)
        else
          begin
            str(A[i, j], s);
            s := copy(s, 1, f-2) + 'E' + copy(s, length(s), 1);
            Write(s:f)
          end;
        if j = mc
        then
          if Rev
          then
            TextColor(1)
          else
            begin
              TextColor(14);
              TextBackground(1)
            end;
      end;
      if Rev
      then
        begin
          TextColor(14);
          TextBackground(1);
          Rev := False
        end;
    WriteLn;
    Write('    ');
    for j := 1 to n do
      begin
        Write('  ');
        if j = mc
        then
          TextBackground(7);
        for ti := 1 to f do Write(' ');
        if j = mc
        then
          TextBackground(1)
      end;
    WriteLn
  end;
WriteLn;
WriteLn
end;

function DetModM(A : matrix; n : integer; m : comp) : comp;

var
B                    : matrix;
d,                              {The determinant -- eventually.}
t,                              {a temporary variable used for swapping}
q                               {the quotient, rounded to a nearby integer}
                     : comp;
i, i0, i1,                      {indices of rows}
j, c,                           {indices of columns}
ti                              {temporary variable,
                                 used for swapping indices, etc.}
                     : integer;
Start                : Boolean; {Starting work on column c?}

begin
display(A, 0, 0, 0);
WriteLn('Will begin by reducing the elements of A (mod ', m:1:0, ').');
Write('Press <Return> to continue ... ');
ReadLn;
for i := 1 to n do              {reduce elements (mod m)}
  for j := 1 to n do
    begin
      t := A[i, j]/m;
      B[i, j] := A[i, j] - t*m
    end;
d := 1;
Display(B, 0, 0, 0);
Write('We apply row operations to this matrix, reducing elements (mod ');
WriteLn(m:1:0, ')');
WriteLn('as we go, until the matrix is in upper-triangular form.');
Write('Press <Return> to continue ... ');
ReadLn;
for c := 1 to n - 1 do          {Reduce to upper triangular form,
                                 working on column c.}
begin
  Start := True;
  repeat
    i0 := c - 1;
    repeat
      i0 := i0 + 1
    until (B[i0, c] <> 0) or (i0 > n);
    if i0 > n
    then
      begin
        Display(B, 0, 0, c);
        Write('All the elements in rows ', c:1, ' through ', n:1);
        WriteLn(' of column ', c:1, ' are 0.');
        Write('Hence the determinant is ð 0 (mod m),');
        WriteLn('so we can cease this nonsense.');
        Write('Press <Return> to continue ... ');
        ReadLn;
        detModM := 0;
        exit
      end
    else
      begin
        if Start
        then
          begin
            Start := False;
            Display(B, 0, 0, c);
            WriteLn('Initiating work on column ', c:1, '.');
            Write('Press <Return> to continue ... ');
            ReadLn
          end;
        i1 := i0;
        repeat
          i1 := i1 + 1
        until (B[i1, c] <> 0) or (i1 > n);
        if i1 <= n
        then
          begin
            Display(B, i0, i1, c);
            Write('Will subtract integral multiples of row ', i1:1);
            WriteLn(' from row ', i0:1, ',');
            Write('and vice versa, until a(', i0:1, ', ');
            WriteLn(c:1, ') = 0 or a(', i1:1, ', ', c:1, ') = 0.');
            if Abs(B[i0, c]) < Abs(B[i1, c])
            then                       {swap indices}
              begin
                ti := i1;
                i1 := i0;
                i0 := ti
              end;
            repeat
              q := B[i0, c]/B[i1, c];  {quotient of elements,
                                        rounded to nearby integer}
              if q > 0
              then
                begin
                  Write('Subtract ', q:1:0, ' times row ');
                  WriteLn(i1:1, ' from row ', i0:1)
                end;
              if q < 0
              then
                begin
                  Write('Add ', -q:1:0, ' times row ');
                  WriteLn(i1:1, ' to row ', i0:1)
                end;
              Write('Press <Return> to continue ... ');
              ReadLn;
              for j := c to n do B[i0, j] := B[i0, j] - q*B[i1, j];
              display(B, i0, i1, c);
              Write('Next we reduce the elements in row ', i0:1);
              WriteLn(' modulo ', m:1:0, '.');
              Write('Press <Return> to continue ... ');
              ReadLn;
              for j := c + 1 to n do
                begin
                  t := B[i0, j]/m;
                  B[i0, j] := B[i0, j] - t*m;
                end;
              display(B, i0, i1, c);
              ti := i0;          {swap indices}
              i0 := i1;
              i1 := ti;
            until B[i1, c] = 0;  {the Euclidean algorithm is complete}
            WriteLn('The Euclidean algorithm has terminated, as ');
            WriteLn('a(', i1:1, ', ', c:1, ') = 0.');
            WriteLn('Press <Return> to continue.');
            ReadLn
          end
        else                     {only one non-zero element}
          begin
            if i0 > c
            then
              begin
                Display(B, c, i0, c);
                WriteLn('Next we swap rows ', c:1, ' and ', i0:1, '.');
                WriteLn('(This multiplies the determinant by -1.)');
                Write('Press <Return> to continue ... ');
                ReadLn;
                for j := c to n do
                  begin
                    t := B[c, j];
                    B[c, j] := B[i0, j];
                    B[i0, j] := t
                  end;
                d := -d;
              end;
          end;
      end;
  until i1 > n;
  Display(B, 0, 0, c);
  WriteLn('Work on column ', c:1, ' is complete.');
  Write('Press <Return> to continue ... ');
  ReadLn
end;
Display(B, 0, 0, 0);
WriteLn('The matrix is now in upper-triangular form.');
Write('We next multiply the diagonal elements, reducing the product ');
WriteLn('(mod m) as we go.');
Write('Press <Return> to continue ... ');
ReadLn;
for j := 1 to n do
  begin
    d := d*B[j, j];
    t := d/m;
    d := d - t*m
  end;
if d < 0 then d := d + m;           {force d into interval [0, m)}
DetModM := d
end;                                {of function DetModM}

begin {Main body}
OriginalMode := LastMode;
TextColor(14);
TextBackground(1);
ClrScr;
m := GetInput(WhereX, WhereY, 'Calculate det(A) (mod m) where m = ',
'   (m must be an integer, 0 < m < 1000000000)', 1, 999999999);
n := round(GetInput(WhereX, WhereY, 'A is an nxn matrix with n = ',
     ' (n must be an integer, 1 ó n ó 9)', 1, 9));
WriteLn('The elements of A are:');
t := m;
for i := 1 to n do
  for j := 1 to n do
    begin
      repeat
        Write('a(', i:1, ', ', j:1, ') = ');
        ReadLn(A[i, j]);
        if Abs(A[i, j]) >= 1E9
        then
        WriteLn('The elements must have absolute value smaller than 1E9.')
      until Abs(A[i, j]) < 1E9;
      if Abs(A[i, j]) > t then t := Abs(A[i, j])
    end;
str(t:1:0, s);
f := 2 + length(s);
if f*n > 72 then f := trunc(73/n);
d := DetModM(A, n, m);
ClrScr;
k := 1;
Write('    Ú');
for j := 1 to f*n do Write(' ');
WriteLn(' ¿');
for i := 1 to n do
  begin
    if k = n then Write('A = ³') else Write('    ³');
    for j := 1 to n do
      begin
        str(Abs(A[i, j]):1:0, s);
        if length(s) + 2 <= f
        then Write(A[i, j]:f:0)
        else
          begin
            str(A[i, j], s);
            s := copy(s, 1, f-4) + 'E' + copy(s, length(s), 1);
            Write(s:f)
          end
      end;
    WriteLn(' ³');
    k := k + 1;
    if k = n
    then
      begin
        Write('A = ³');
        for j := 1 to f*n do Write(' ');
        WriteLn(' ³')
      end;
    if (k <> n) and (k < 2*n)
    then
      begin
        Write('    ³');
        for j := 1 to f*n do Write(' ');
        WriteLn(' ³')
      end;
    k := k + 1;
  end;
Write('    À');
for j := 1 to f*n do Write(' ');
WriteLn(' Ù');
WriteLn;
WriteLn('det(A) ð ', d:1:0, ' (mod ', m:1:0, ').');
ReadLn;
TextMode(OriginalMode)
end.