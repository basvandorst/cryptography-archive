program rat;                         {Convert decimal to rational form.
                                      This program returns the rational
                                      number a/q with least q such that
                                      |x - a/q| < (0.5)10^(-d) where x is
                                      given with d places after the
                                      decimal.}
{$N+,E+,R+}
uses crt;

const
MaxAllow = 1E18;

var
d,                            {tolerance of desired approximation}
u, v,                         {u/v is h/v but with type extended}
w,                            {remainder}
x,                            {decimal to be approximated}
theta                         {values used to determine partial quotients}
              :   extended;
St                            {input as a string}
              :   string[30];
h0, k0,                       {rational approximations}
h1, k1,
h2, k2,
q,                            {partial quotients}
c0, c1, c                     {search for least c such that
                               |(c*h1 + h0)/(c*k1 + k0) - x| ó d}
              :   comp;
x0, y0,                       {cursor coordinates}
i,                            {position of decimal point within St}
j,                            {index for loop}
code                          {error level in converting string to real}
              :   integer;
ParamSet,                     {was the input given acceptably on the
                               command line?}
Overflow                      {is overflow going to occur?}
              :   Boolean;

begin
  ParamSet := False;
  if ParamCount = 1
  then
    begin
      St := ParamStr(1);
      Val(St, x, code);
      if code = 0
      then ParamSet := True;
    end;
  if not ParamSet
  then
    begin
      WriteLn('Enter real number x to be rationally approximated,');
      Write('x = ');
      x0 := WhereX;
      y0 := WhereY;
      repeat
        GoToXY(x0, y0);
        ClrEoL;
        ReadLn(St);
        Val(St, x, code)
      until (code = 0);
      ClrEoL;
      for j := 1 to 2 do
        begin
          GoToXY(1, WhereY - 1);
          ClrEoL
        end
    end;
  i := pos('.', St);
  d := 0.5;
  if i > 0
  then for j := 1 to length(St) - i do d := d/10;
  h1 := 0;
  k1 := 1;
  h2 := 1;
  k2 := 0;
  u := 1;
  v := 1;
  theta := x;
  Overflow := False;
  while (Abs(u/v - x) > d) and (not Overflow) do
    begin
      h0 := h1;
      k0 := k1;
      h1 := h2;
      k1 := k2;
      if Abs(theta) <= MaxAllow
      then
        begin
          q := theta;
          if q > theta
          then q := q - 1;
          theta := theta - q;
          if theta > 0
          then theta := 1/theta;
          u := q*h1 + h0;
          v := q*k1 + k0;
          if (Abs(u) > MaxAllow) or (Abs(v) > MaxAllow)
          then Overflow := True
          else
            begin
              h2 := u;
              k2 := v
            end
        end
      else
        begin
          Overflow := True;
          q := MaxAllow
        end;
    end;
  c0 := 1;
  c1 := q;
  if Overflow
  then
    begin
      while c1 - c0 > 1 do
        begin
          c := (c1 + c0)/2;
          if (c*h1 + h0 <= MaxAllow) and (c*k1 + k0 <= MaxAllow)
          then c0 := c
          else c1 := c
        end;
      c1 := c0;
      c0 := 1;
      u := c1*h1 + h0;
      v := c1*k1 + k0;
      if Abs(u/v - x) <= d
      then Overflow := False;
    end;
  if not Overflow
  then
    begin
      while c1 - c0 > 1 do
        begin
          c := (c1 + c0)/2;
          u := c*h1 + h0;
          v := c*k1 + k0;
          w := x - u/v;
          if (Abs(w) > d)
          then c0 := c
          else c1 := c
        end;
      u := c1*h1 + h0;
      v := c1*k1 + k0;
      w := x - u/v;
      Write(St, ' = ', u:1:0, '/', v:1:0);
      if w = 0.0
      then WriteLn(' (exactly)');
      if w > 0
      then WriteLn(' + ', w:1:20);
      if w < 0
      then WriteLn(' - ', Abs(w):1:20);
    end
  else
    begin
      WriteLn('Failed to find a rational number a/q such that');
      WriteLn(' ³', St, ' - a/q³ < ', d:0, ',');
      Write('³a³ ó ', MaxAllow:1:0, ', and 1 ó q ó ');
      WriteLn(MaxAllow:1:0, '.');
    end
end.
