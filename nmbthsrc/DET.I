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

begin
for i := 1 to n do              {reduce elements (mod m)}
  for j := 1 to n do
    begin
      t := A[i, j]/m;
      B[i, j] := A[i, j] - t*m
    end;
d := 1;
for c := 1 to n - 1 do          {Reduce to upper triangular form,
                                 working on column c.}
begin
  repeat
    i0 := c - 1;
    repeat
      i0 := i0 + 1
    until (B[i0, c] <> 0) or (i0 > n);
    if i0 > n
    then
      begin
        detModM := 0;
        exit
      end
    else
      begin
        i1 := i0;
        repeat
          i1 := i1 + 1
        until (B[i1, c] <> 0) or (i1 > n);
        if i1 <= n
        then
          begin
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
              for j := c to n do B[i0, j] := B[i0, j] - q*B[i1, j];
              for j := c + 1 to n do
                begin
                  t := B[i0, j]/m;
                  B[i0, j] := B[i0, j] - t*m;
                end;
              ti := i0;          {swap indices}
              i0 := i1;
              i1 := ti;
            until B[i1, c] = 0;  {the Euclidean algorithm is complete}
          end
        else                     {only one non-zero element}
          begin
            if i0 > c
            then
              begin
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
end;
for j := 1 to n do
  begin
    d := d*B[j, j];
    t := d/m;
    d := d - t*m
  end;
if d < 0 then d := d + m;           {force d into interval [0, d)}
DetModM := d
end;                                {of function DetModM}

