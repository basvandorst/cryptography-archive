Return-Path: <kbriggs@mundoe.maths.mu.OZ.AU>
Received: from munnari.oz.au by clvax1.cl.msu.edu with SMTP ; 
          Mon, 29 Apr 91 00:02:37 EDT
Received: from mundoe.maths.mu.OZ.AU by munnari.oz.au with SMTP (5.64+1.3.1+0.50)
	id AA10332; Mon, 29 Apr 1991 14:02:01 +1000 (from kbriggs@mundoe.maths.mu.OZ.AU)
Received: by mundoe.maths.mu.OZ.AU (4.0)
	id AA08882; Mon, 29 Apr 91 14:01:55 EST
Date: Mon, 29 Apr 91 14:01:55 EST
From: kbriggs@mundoe.maths.mu.OZ.AU (Keith Briggs rba8 7088)
Message-Id: <9104290401.8882@mundoe.maths.mu.OZ.AU>
To: riordanmr@clvax1.cl.msu.edu
Subject: RE: MP

unit arith;  {  Turbo Pascal 5 by K M Briggs.  Last revised 1 June 1989.  }

{$A+,B-,D+,E-,F-,I+,L+,N+,O-,R-,S+,V-} {$M 16384,0,655360}

{-----------------------------------------------------------------------------

  ARITH.PAS - Multiple precision integer and rational arithmetic.

  Uses algorithms from Knuth, Art of Computer Programming vol.2, sec.4.3.1.

  Version with base 2^15 = 32768 = maxint div 2.

  Type mp is an array of maxprec non-negative integers, 1 element being the
  most significant.  `Digits' of x to base are x[1],x[2],...,x[abs[x[0]].
  Type rational is record n,d:mp end; representing numerator and denominator.

  Zero element is sign(x)*(number of digits) ( <=maxprec ).

  N.B. Range checking must be off, because integer overflow is exploited
       to detect carries.
       Inputs to rational procedures must be in lowest terms.
       ( use ratcancel to ensure this.)

-----------------------------------------------------------------------------}

interface

uses  crt;                              { Defines wherex, wherey }

const maxprec               =8;        { May be >=1 }
      base      :longint    =32768;     { Must not be changed !! }
      mperror   :integer    =0;         { 0 means no error }

type  mp        =array[0..maxprec] of integer;
      rational  =record n,d:mp end;

var   zerorat,onerat,tworat,undefrat:rational;{0,1,2 and undefined (1/0) rats }
      zeromp, onemp, twomp          :mp;      {0,1 and 2 multiple precision   }

function intmax(x,y:integer):integer;               { Integer max,min,sign... }
function intmin(x,y:integer):integer;
function intsign(x:integer) :integer;

procedure mpset(var u,v:mp);                                      { Sets u:=v }
procedure mperrormsg(n:integer);                     { Shows mp error message }
function  mpcompabs(var u,v:mp):integer;{ 1 if abs(u)>abs(v), 0 if =, -1 else }
function  mpcomp(var u,v:mp):integer;         { 1 if u>v, 0 if equal, -1 else }
procedure mp_to_string(x:mp; var out:string);         { Converts mp to string }
procedure mpshow(lab:string; x:mp);                     { Writes mp to screen }
procedure mpshowln(lab:string; x:mp);            { Ditto with carriage return }
procedure mpadd0(var u:mp; v:mp; var w:mp); { Adds absolute values of u and v }
procedure mpadd(var u:mp; v:mp; var w:mp);       { Adds arbitrary mp integers }
procedure mpaddint(var u:mp; i:integer; var w:mp);       { Adds mp to integer }
procedure mpsub0(u,v:mp; var w:mp);{ Finds abs(u)-abs(v). Difference must be>0}
procedure mpsub(u,v:mp; var w:mp);          { Subtracts arbitrary mp integers }
procedure mpmul(u,v:mp; var w:mp);         { Multiplies arbitrary mp integers }
procedure mpmulint(u:mp; v:integer; var w:mp);     { Multiplies mp by integer }
procedure mpdivint(var u:mp; v:integer; var q:mp; var r:integer);
procedure mpdiv(u,v:mp; var q,r:mp);         { Divides arbitrary mp integers. }
procedure mpsqrt(a:mp; var x:mp);        { Newton's method for trunc(sqrt(a)).}
procedure string_to_mp(s:string; var u:mp);    { Convert decimal string to mp }
procedure mpeuclid(a,b:mp; var d:mp);                { Euclid's GCD algorithm }
procedure pollard(n,a,x1:mp);                 { Pollard's factoring algorithm }
procedure mprandom(d:integer; var r:mp);    { Gets a d digit random mp number }

procedure ratshow(s:string; var x:rational);               { Writes a ratonal }
procedure ratshowln(s:string; var x:rational);   { Ditto with carriage return }
procedure ratform(var r:rational; x,y:integer);      { Form a rational as x/y }
procedure ratcancel(var n:rational);      { Reduces a rational to lowest terms}
procedure ratadd(r,s:rational; var t:rational);  { Four rational operations...}
procedure ratsub(r,s:rational; var t:rational);
procedure ratmul(r,s:rational; var t:rational);
procedure ratdiv(r,s:rational; var t:rational);
procedure ratmulint(r:rational; s:integer; var t:rational);   { rat times int }
procedure ratdivint(r:rational; s:integer; var t:rational);
function  ratequal(var x,y:rational):integer;           { 1 if x<>y, 0 if x=y }
procedure ratrandom(d:integer; var r:rational);      { Gets a random rational }

{-----------------------------------------------------------------------------}

implementation

{---- utilities -------------------------------------------------------------}

function intmax(x,y:integer):integer;
begin if x>y then intmax:=x else intmax:=y end;

function intmin(x,y:integer):integer;
begin if x>y then intmin:=y else intmin:=x end;

function intsign(x:integer):integer;
begin if x=0 then intsign:=0 else if x>0 then intsign:=1 else intsign:=-1 end;

procedure mpset(var u,v:mp);           { Sets u:=v }
var i:integer;
begin for i:=0 to abs(v[0]) do u[i]:=v[i] end;{mpset}

procedure mperrormsg(n:integer);                      { Show mp error message }
begin
  write(^G^J^M); mperror:=n;
  case n of
   1: writeln('Length error in mp addition.');
   2: writeln('Division by zero in mpdivi.');
   3: writeln('Division by zero in mpdiv.');
   4: writeln('abs(u)>abs(v) in mpsub0.');
   5: writeln('Number too long to display.');
   6: writeln('Negative argument in square root.');
   7: writeln('Overflow in multiplication.');
  end
end;{mperrormsg}

{---- mp comparison  ---------------------------------------------------------}

function mpcompabs(var u,v:mp):integer; { 1 if abs(u)>abs(v), 0 if =, -1 else }
var u0,v0,c,j:integer;
label b;
begin
  u0:=abs(u[0]); v0:=abs(v[0]);
  if (u0=0) and (v0=0) then begin mpcompabs:=0; exit end;
  if u0=v0 then begin { equal length }
    j:=1;
    b: if u[j]<v[j] then begin mpcompabs:=-1; exit end;
    if u[j]=v[j] then begin
      if j<u0 then begin inc(j); goto b end else begin mpcompabs:=0; exit end
    end else c:=1; end
  else { unequal length } c:=intsign(u0-v0); mpcompabs:=c
end;{mpcompabs}

function mpcomp(var u,v:mp):integer;   { 1 if u>v, 0 if equal, -1 else }
{ Knuth p 581 ex 11.}
var absu0,u0,v0,c,j:integer;
label b;
begin
  u0:=u[0]; v0:=v[0]; absu0:=abs(u0);
  if (u0=0) and (v0=0) then begin mpcomp:= 0; exit end;
  if (u0>0) and (v0<0) then begin mpcomp:= 1; exit end;
  if (u0<0) and (v0>0) then begin mpcomp:=-1; exit end;
  { u,v now same sign }
  if u0=v0 then begin { equal length }
    j:=1;
    b: if u[j]<v[j] then begin mpcomp:=-1; exit end;
    if u[j]=v[j] then begin
      if j<absu0 then begin inc(j);    goto b   end
                 else begin mpcomp:=0; exit     end
    end else c:=1; end
  else { unequal length } if u0>0 { both +, or both - } then c:=intsign(u0-v0);
  mpcomp:=c
end;{mpcomp}

{---- mp output  -----------------------------------------------------------}

procedure mp_to_string(x:mp; var out:string);
var   i,n,r:integer; t:longint; strr:string[1];
begin
  n:=intmin(abs(x[0]),maxprec);
  if n=0 then
    out:='0'
  else begin
    out:='';
    while n>0 do begin { repeatedly divide x by 10 }
      r:=0;
      for i:=1 to n do begin
        t:=r*base+x[i]; x[i]:=t div 10; r:=t mod 10 end;
      if x[1]=0 then begin dec(n); for i:=1 to n do x[i]:=x[i+1] end;
      str(r:1,strr); out:=strr+out;
      if length(out)>255 then begin mperrormsg(5); exit end;
    end;{while}
    if x[0]<0 then out:='-'+out;
  end
end;{mp_to_string}

procedure mpshow(lab:string; x:mp);                    { write mp to screen }
const lines:integer=0;
var   i,n,r:integer; t:longint; out:string; strr:string[1];
begin
  inc(lines); if wherey=24 then begin
    write(^J^M'--- more ---'); i:=ord(readkey); clrscr end;
  write(lab); mp_to_string(x,out); write(out);
end;{mpshow}

procedure mpshowln(lab:string; x:mp);
begin mpshow(lab,x); writeln end;{mpshowln}

{---- mp addition  -----------------------------------------------------------}

procedure mpadd0(var u:mp; v:mp; var w:mp); { Add absolute values of u and v }
                                            { w may be same as v }
var absu0,absv0,carry,j,k,su,sv,total:integer;
begin
  su:=intsign(u[0]); sv:=intsign(v[0]);
  if su=0 then begin mpset(w,v); exit end;
  if sv=0 then begin mpset(w,u); exit end;
  absu0:=abs(u[0]); absv0:=abs(v[0]); carry:=0; k:=absu0-absv0;
  if intmax(absv0,absu0)>=maxprec then begin { sum too long }
    mperrormsg(1); w:=zeromp; exit end;
  if k>=0 then begin { u not shorter than v }
    for j:=absu0 downto k+1 do begin
      total:=u[j]+v[j-k]+carry;
      if total<0 then carry:=1 else carry:=0;
      w[j]:=total and $7FFF
    end;
    for j:=k downto 1 do begin
      total:=u[j]+carry;
      if total<0 then carry:=1 else carry:=0;
      w[j]:=total and $7FFF
    end;
    if carry=0 then w[0]:=absu0
    else begin
      for j:=absu0 downto 1 do w[j+1]:=w[j]; w[1]:=carry; w[0]:=absu0+1 end;
  end {k>=0}
  else begin  { u  shorter than v }
    k:=-k;
    for j:=absv0 downto k+1 do begin
      total:=v[j]+u[j-k]+carry;
      if total<0 then carry:=1 else carry:=0;
      w[j]:=total and $7FFF
    end;
    for j:=k downto 1 do begin
      total:=v[j]+carry;
      if total<0 then carry:=1 else carry:=0;
      w[j]:=total and $7FFF
    end;
    if carry=0 then w[0]:=absv0
    else begin
      for j:=absv0 downto 1 do w[j+1]:=w[j]; w[1]:=carry; w[0]:=absv0+1 end;
  end
end;{mpadd0}

procedure mpadd(var u:mp; v:mp; var w:mp);   { Add arbitrary mp integers }
                                             { w may be same as v }
var absu0,absv0,carry,j,k,su,sv,s,total:integer;
begin
  su:=intsign(u[0]); sv:=intsign(v[0]); s:=su*sv;
  if s=0 then begin { one argument zero }
    if su=0 then mpset(w,v) else mpset(w,u); exit
  end;
  absu0:=abs(u[0]); absv0:=abs(v[0]); carry:=0; k:=absu0-absv0;
  if s>0 then begin { both argument same sign }
    if intmax(absv0,absu0)>=maxprec then begin { sum too long }
      mperrormsg(1); w:=zeromp; exit end;
    if k>=0 then begin { u not shorter than v }
      for j:=absu0 downto k+1 do begin
        total:=u[j]+v[j-k]+carry;
        if total<0 then carry:=1 else carry:=0;
        w[j]:=total and $7FFF
      end;
      for j:=k downto 1 do begin
        total:=u[j]+carry;
        if total<0 then carry:=1 else carry:=0;
        w[j]:=total and $7FFF
      end;
      if carry=0 then w[0]:=absu0
      else begin
        for j:=absu0 downto 1 do w[j+1]:=w[j]; w[1]:=carry; w[0]:=absu0+1 end;
    end {k>=0}
    else begin  { u  shorter than v }
      k:=-k;
      for j:=absv0 downto k+1 do begin
        total:=v[j]+u[j-k]+carry;
        if total<0 then carry:=1 else carry:=0;
        w[j]:=total and $7FFF
      end;
      for j:=k downto 1 do begin
        total:=v[j]+carry;
        if total<0 then carry:=1 else carry:=0;
        w[j]:=total and $7FFF
      end;
      if carry=0 then w[0]:=absv0
      else begin
        for j:=absv0 downto 1 do w[j+1]:=w[j]; w[1]:=carry; w[0]:=absv0+1 end;
    end;
    if su<0 then w[0]:=-w[0]; { both arguments negative, so sum negative }
  end
  else begin { arguments differ in intsign }
    if mpcompabs(u,v)>=0 then begin { abs(u)>=abs(v) }
                           mpsub0(u,v,w); if u[0]<0 then w[0]:=-w[0] end
                        else begin { abs(u)< abs(v) }
                           mpsub0(v,u,w); if v[0]<0 then w[0]:=-w[0] end
  end;
end;{mpadd}

procedure mpaddint(var u:mp; i:integer; var w:mp);
{ Add integer to absolute value of u. w may be same as u }
var absu0,carry,j,total:integer;
begin
  absu0:=abs(u[0]); carry:=0; if (absu0=0) and (i<>0) then inc(absu0);
  for j:=absu0 downto 1 do begin
    total:=u[j]+i+carry; i:=0;
    if total<0 then carry:=1 else carry:=0; w[j]:=total and $7FFF
  end;
  if carry=0 then w[0]:=absu0 else begin
    for j:=absu0 downto 1 do w[j+1]:=w[j]; w[1]:=carry; w[0]:=absu0+1 end;
end;{mpaddi}

{---- mp subtraction  -------------------------------------------------------}

procedure mpsub0(u,v:mp; var w:mp);{ Find abs(u)-abs(v). Difference must be>0}
{ NB no error checking - answer is garbage if conditions not satisfied.}

var u0,v0,borrow,i,j,k,total :integer;
begin
  u0:=abs(u[0]); v0:=abs(v[0]); k:=u0-v0; borrow:=0;
  if k>=0 then begin { u longer, or equal length.}
    for j:=u0 downto k+1 do begin
      total:=u[j]-v[j-k]+borrow;
      if total<0 then begin w[j]:=(total+base) and $7FFF; borrow:=-1 end
                 else begin w[j]:=total and $7FFF;        borrow:= 0 end;
    end;{j}
    for j:=k downto 1 do begin
      total:=u[j]+borrow;
      if total<0 then begin w[j]:=(total+base) and $7FFF; borrow:=-1 end
                 else begin w[j]:=total and $7FFF;        borrow:= 0 end;
    end;{j}
    w[0]:=u0
  end else begin { v longer }
    k:=-k; writeln('k=',k);
    for j:=v0 downto k+1 do begin
      total:=u[j-k]-v[j]+borrow;
      if total<0 then begin w[j]:=(total+base) and $7FFF; borrow:=-1 end
                 else begin w[j]:=total and $7FFF;        borrow:= 0 end;
    end;{j}
    for j:=k downto 1 do begin
      total:=-v[j]+borrow;
      if total<0 then begin w[j]:=(total+base) and $7FFF; borrow:=-1 end
                 else begin w[j]:=total and $7FFF;        borrow:= 0 end;
    end;{j}
    w[0]:=v0
  end;
  k:=1; j:=w[0]; while (k<=j) and (w[k]=0) do inc(k);
  if k>j then j:=0 else if k>1 then begin
     j:=j-k+1; for i:=1 to j do w[i]:=w[i+k-1] end;
  w[0]:=j;
  if borrow<>0 then mperrormsg(4)
end;{mpsub0}

procedure mpsub(u,v:mp; var w:mp);        { Subtract arbitrary mp integers }
{ No length checking needed here since this routine calls mpadd0 and mpsub0
  which do the checking.}
var absu0,absv0,c,s,su,sv,u0,v0,borrow,i,j,k,total :integer;
begin
  su:=intsign(u[0]); sv:=intsign(v[0]); s:=su*sv;
  if s=0 then begin { one argument zero }
    if su=0 then begin mpset(w,v); w[0]:=-w[0] end else mpset(w,u); exit
  end;
  u0:=u[0]; v0:=v[0]; absu0:=abs(u0); absv0:=abs(v0); c:=mpcompabs(u,v);
  if s>0 then begin  { u and v same intsign }
    if c>=0 then begin { abs(u)>=abs(v) }
                   mpsub0(u,v,w); if u0<0 then w[0]:=-w[0] end
            else begin { abs(u)< abs(v) }
                   mpsub0(v,u,w); if u0>0 then w[0]:=-w[0] end
  end else if u0>0 then { u>0, v<0 } mpadd0(u,v,w)
                   else { u<0, v>0 } begin mpadd0(u,v,w); w[0]:=-w[0] end;
end;{mpsub}

{---- mp multiplication  ----------------------------------------------------}

procedure mpmul(u,v:mp; var w:mp);           { Multiply arbitrary mp integers }
var i,j,k,l,m,n,s,w0:integer; t,vj:longint;
    tr:record l,h:integer end absolute t;
begin
  s:=intsign(u[0])*intsign(v[0]);
  n:=abs(u[0]); m:=abs(v[0]); w0:=m+n;
  if (n=0) or (m=0) then begin w[0]:=0; exit end;
  if w0>maxprec then begin mperrormsg(7); w:=zeromp; exit end;
  for i:=m+1 to w0 do w[i]:=0;
  for j:=m downto 1 do begin
    k:=0; vj:=v[j];
    for i:=n downto 1 do begin
      l:=i+j; t:=u[i]; t:=t*vj+w[l]+k;       { longint (ie. double) product }
      w[l]:=tr.l and $7FFF;                      { =t mod base }
      k:=(tr.h shl 1) + (tr.l and $8000) shr 15; { =t div base }
    end;{i}
    w[j]:=k
  end;{j}
  k:=1; while (w[k]=0) and (k<=w0) do inc(k); { find first nonzero element }
  if k=w0+1 then s:=0 else if k>1 then begin
    w0:=w0-k+1; for i:=1 to w0 do w[i]:=w[i+k-1] end;
  w[0]:=w0*s;
end;{mpmul}

procedure mpmulint(u:mp; v:integer; var w:mp);
{ Multiply mp integer by single integer. }
var absv,i,k,n:integer; t:longint;
    tr:record l,h:integer end absolute t;
begin
  absv:=abs(v); k:=0; n:=abs(u[0]);
  for i:=n downto 1 do begin
    t:=u[i]; t:=t*absv+k;
    w[i]:=tr.l and $7FFF;                      { =t mod base }
    k:=(tr.h shl 1) + (tr.l and $8000) shr 15; { =t div base }
  end;
  if k>0 then begin inc(n); for i:=n downto 1 do w[i+1]:=w[i]; w[1]:=k end;
  w[0]:=intsign(u[0])*intsign(v)*n
end;{mpmulint}

{---- mp division  ----------------------------------------------------------}

procedure mpdivint(var u:mp; v:integer; var q:mp; var r:integer);
{ Divide arbitrary mp integer by single integer. Knuth vol.2 p.582 }
var absv,j,n:integer; t:longint;
begin
  if v=0 then begin mperrormsg(2); q:=onemp; r:=0; exit end;
  r:=0; n:=abs(u[0]); absv:=abs(v);
  for j:=1 to n do begin { divide absolute values }
    t:=r*base+u[j]; q[j]:=t div absv; r:=t mod absv end;
  if q[1]=0 then begin dec(n); for j:=1 to n do q[j]:=q[j+1] end;
  q[0]:=n;
  { fix signs if u or v, or both, are negative }
  if (u[0]>0) and (v<0) then q[0]:=-q[0];
  if (u[0]<0) then begin
    if r>0 then begin mpaddint(q,1,q); r:=-r+absv end;
    q[0]:=-intsign(v)*q[0]
  end
end;{mpdivint}

procedure mpdiv(u,v:mp; var q,r:mp);     { Divide arbitrary mp integers. }
                                         { Knuth vol.2, p.257. Algorithm D.}

var   aa,a,carry,d,i,j,k,m,n,rh,qh,u0,v1,v2,x,z:integer;
      w:word; ss,tt:longint; savev:mp;
      ttr:record l,h:integer end absolute tt;

label 2,D4;

begin
  if v[0]=0 then begin mperrormsg(3); q:=onemp; r:=zeromp; exit end;
  if mpcompabs(u,v)<0 then begin  { abs(u)<abs(v) so q=0 or + or - 1 }
    if u[0]>=0 then begin  { numerator>=0 }
                      q[0]:=0; mpset(r,u) end
               else begin  { numerator<0 }
                      q[0]:=-intsign(v[0]); q[1]:=1; mpset(r,v);
                      r[0]:=abs(r[0]); mpadd(u,r,r)
                    end;
    exit
  end;
  n:=abs(v[0]); u0:=u[0]; m:=abs(u0)-n;
  if n=1 then begin
    mpdivint(u,intsign(v[0])*v[1],q,r[1]);
    if r[1]=0 then r[0]:=0 else r[0]:=1
  end
  else begin { n>1.  Divide absolute values, fix signs at end.}
    mpset(savev,v);
    d:=base div (v[1]+1); { 1<=d<=base div 2 }

    { multiply u and v by d, using u[0] for any carry }
    if d=1 then
      u[0]:=0
    else begin { d>1 }
      k:=0;
      for i:=m+n downto 1 do begin
        tt:=u[i]; tt:=tt*d+k;
        u[i]:=ttr.l and $7FFF;                       { =tt mod base }
        k:=(ttr.h shl 1) + (ttr.l and $8000) shr 15; { =tt div base }
      end;
      u[0]:=k; k:=0;
      for i:=n   downto 1 do begin
        tt:=v[i]; tt:=tt*d+k;
        v[i]:=ttr.l and $7FFF;
        k:=(ttr.h shl 1) + (ttr.l and $8000) shr 15;
      end;
    end;
    v1:=v[1]; v2:=v[2];

    for j:=0 to m do begin
      {D3}
      tt:=u[j];
      tt:=tt*base+u[j+1];  { double product and sum }
      a:=tt div v1; x:=tt mod v1;  { a<=2*(base-1), I think. }
      if a>=0 { no quotient overflow } then begin qh:=a; rh:=x; goto 2 end;
      x:=base-1; a:=u[j+1];
      repeat { This is messy but efficient }
        qh:=x; a:=a+v1;
        if a<0 then { overflow } goto D4;
        rh:=a; a:=qh;
        2: tt:=a; tt:=tt*v2;
        x:=ttr.l and $7FFF;
        a:=(ttr.h shl 1) + (ttr.l and $8000) shr 15;
        if a<rh then goto D4;
        if a=rh then if x<=u[j+2] then goto D4;
        {3H} x:=qh; dec(x); a:=rh
      until false;

      D4:   { trial quotient digit qh now determined }
      x:=0; { borrow }
      for i:=n downto 1 do begin
        k:=i+j; tt:=v[i]; tt:=qh*tt; tt:=u[k]-tt+x;
        if tt<0 then begin
          tt:=-tt; ss:=ttr.l and $7FFF;
          if ss>0 then begin
            u[k]:=base-ss; tt:=base+tt;
            x:=-((ttr.h shl 1) + (ttr.l and $8000) shr 15);
          end else begin
            u[k]:=0; x:=-((ttr.h shl 1) + (ttr.l and $8000) shr 15) end
        end
        else begin u[k]:=tt; x:=0 end;
      end;{i}
      tt:=u[j]+x;
      if tt<0 then begin
        tt:=-tt; ss:=ttr.l and $7FFF;
        if ss>0 then begin
          u[j]:=base-ss; tt:=base+tt;
          x:=-((ttr.h shl 1) + (ttr.l and $8000) shr 15)
        end else begin
          u[j]:=0; x:=-((ttr.h shl 1) + (ttr.l and $8000) shr 15) end
      end
      else begin u[j]:=tt; x:=0 end;

      {D5}
      q[j]:=qh;
      if x<0 then begin { D6 - presence of carry indicates qh was one too big }
        dec(q[j]); x:=0;
        for i:=n downto 1 do begin
          a:=v[i]+u[j+i]+x;
          u[j+i]:=a and $7FFF; if a<0 then x:=1 else x:=0
        end;{i}
        u[j]:=0 { ignore carry }
      end;

      {D7} { here u[j]=0 }
    end;{j}

    { D8 }
    ss:=0;
    for j:=1 to n do begin
      tt:=ss*base+u[m+j]; r[j]:=tt div d; ss:=tt mod d end;
    r[0]:=n;
    k:=1; while (k<=n) and (r[k]=0) do inc(k);
    if k>n then n:=0 else if k>1 then begin
      n:=n-k+1; for i:=1 to n do r[i]:=r[i+k-1] end;
    r[0]:=n;
    if q[0]<>0 then begin inc(m); for i:=m downto 1 do q[i]:=q[i-1] end;
    q[0]:=m;

    { We now have divided absolute values and should have
      abs(u)=q*abs(v)+r, 0<=r<v.   Fix signs if u or v, or both are negative.
      We want u=q*v+r always, with 0<=r<v. }

    if (u0>0) and (savev[0]<0) then
      q[0]:=-q[0];
    if (u0<0) then begin
       j:=-intsign(savev[0]);
       if r[0]>0 then begin
         mpaddint(q,1,q); { OK to use mpaddint since q>0 at this point }
         r[0]:=-r[0];  savev[0]:=abs(savev[0]); mpadd(savev,r,r)
       end;
       q[0]:=j*q[0]
    end
  end; {n>1}

end;{mpdiv}

procedure mpsqrt(a:mp; var x:mp);        { Newton's method for trunc(sqrt(a)).}
var y,z:mp;
begin
  if a[0]<0 then begin mperrormsg(6); x:=zeromp; exit end;
  x:=a; if mpcompabs(x,twomp)<0 then exit; y:=onemp;
  while mpcompabs(y,x)<=0 do begin
    mpadd(y,y,y); mpdiv(x,twomp,x,z)
  end;
  repeat
    mpadd(y,x,y); mpdiv(y,twomp,x,z); mpdiv(a,x,y,z)
  until mpcompabs(x,y)<=0
end;{mpsqrt}

(*
procedure mpsqrt(a:mp; var root,rem:mp);    { Finds root,rem st. root^2+rem=a }
{ Bisection method - SLOW !!! }
var d,x,u,l,z:mp; done:boolean; equal:integer;
begin
  if mpcomp(a,zeromp)<0 then begin
    mperrormsg(6); root:=onemp; rem:=zeromp; exit end;
  if mpcompabs(a,twomp)<=0 then begin root:=a; rem:=zeromp; exit end;
  l:=zeromp; u:=a;
  repeat
    mpadd(l,u,z); mpdiv(z,twomp,x,z); mpmul(x,x,z);
    mpsub(u,l,d); equal:=mpcompabs(z,a);
    done:=(mpcompabs(d,onemp)=0) or (equal=0);
    if equal>0 then u:=x else l:=x
  until done;
  root:=x;
  if equal=0 then rem:=zeromp
             else mpsub(a,z,rem);
end;{mpsqrt}
*)

{---- string to mp ----------------------------------------------------------}

procedure string_to_mp(s:string; var u:mp);  { Convert decimal string to mp }
var i:integer; ch:char;
begin
  u[0]:=0; u[1]:=0; i:=1; if s[1]='-' then inc(i);
  for i:=i to length(s) do begin
    ch:=s[i];
    if ch in ['0'..'9'] then begin
      mpmulint(u,10,u); mpaddint(u,ord(ch)-ord('0'),u) end
      else begin writeln(^G'Bad character in string_to_mp.'); u[0]:=0; exit end
  end;
  if s[1]='-' then u[0]:=-u[0];
end;{string_to_mp}

{---- mp gcd --------------------------------------------------------------}

procedure mpeuclid(a,b:mp; var d:mp); { Euclid's GCD algorithm }
var q,r:mp; a1,b1,i,rr:integer;
begin
  a[0]:=abs(a[0]); b[0]:=abs(b[0]);
  if (b[0]=1) and (a[0]=1) then begin { handle single case }
    a1:=a[1]; b1:=b[1];
    while b1<>0 do begin rr:=a1 mod b1; a1:=b1; b1:=rr end;
    d[0]:=1; d[1]:=a1
  end
  else begin  { mp case }
    while b[0]>0 do begin mpdiv(a,b,q,r); mpset(a,b); mpset(b,r) end;
    mpset(d,a);
  end;
end; {mpeuclid}

procedure pollard(n,a,x1:mp);
{ Find factors of n by Pollard's method }
var i:integer; x,y,q,w,p,z:mp;
begin
  write('Pollard''s method - finding factors of '); mpshowln('',n);
  mpset(x,x1); mpset(y,x1); q[0]:=1; q[1]:=1;
  for i:=1 to 10000 do begin
    mpmul(x,x,z); mpsub(z,a,z);  mpdiv(z,n,w,x);
    mpmul(y,y,z); mpsub(z,a,z);  mpdiv(z,n,w,y);
    mpmul(y,y,z); mpsub(z,a,z);  mpdiv(z,n,w,y);
    mpsub(y,x,z); mpmul(q,z,q);  mpdiv(q,n,w,q);
    if i mod 20=0 then begin
      mpeuclid(q,n,p);
      if ((p[0]=1) and (p[1]>1)) or (p[0]>1) then begin
        mpdiv(n,p,z,w);
        if (w[0]=0) then begin
          write('For i=',i:5); mpshowln(',  factor=',p);
          mpset(n,z); if (n[0]=1) and (n[1]=1) then exit
        end;
      end;
    end;
  end;
  writeln('No factor found in 10000 cycles.');
end;{pollard}

procedure mprandom(d:integer; var r:mp);   { get a d digit random mp number }
var i:integer;
begin
  r[0]:=d; if random<0.5 then r[0]:=-r[0];
  for i:=1 to d do r[i]:=random(base); while r[1]=0 do r[1]:=random(base);
end;{mprandom}

{---- Rational arithmetic procedures ----------------------------------------

    See Knuth, Art of Computer Programming vol. 2, section 4.5.1
    and
    B. Buchberger ed. Computer Algebra (2nd ed. Springer 1983) p.200.

------------------------------------------------------------------------------}

procedure ratshow(s:string; var x:rational);
const infinity=#236;
begin
  if x.d[0]=0 then write(s,infinity) else begin
    mpshow(s,x.n); { Don't show /1 }
    if (x.n[0]<>0) and not((x.d[0]=1) and (x.d[1]=1)) then mpshow('/',x.d) end;
end;{ratshow}

procedure ratshowln(s:string; var x:rational);
begin ratshow(s,x); writeln end;{ratshowln}

procedure ratform(var r:rational; x,y:integer);      { Form a rational as x/y }
begin  { y must be <> 0 }
  r.n[0]:=intsign(x)*intsign(y); r.d[0]:=1; r.n[1]:=abs(x); r.d[1]:=abs(y);
  ratcancel(r)
end;{ratform}

procedure ratcancel(var n:rational); {reduce a rational to lowest terms}
var g,r:mp;
begin
  if n.d[0]=0 then begin n:=undefrat; exit end;
  if n.n[0]<>0 then begin
    mpeuclid(n.n,n.d,g);
    if (g[0]<>1) or (g[1]<>1) then begin
      mpdiv(n.n,g,n.n,r); mpdiv(n.d,g,n.d,r) end;
  end
end;{ratcancel}

procedure ratadd(r,s:rational; var t:rational);
var d,rdd,temp1,temp2:mp;
begin
  if (r.d[0]=0) or (s.d[0]=0) then begin s:=undefrat; exit end;
  if r.n[0]=0 then begin t:=s; exit end;
  if s.n[0]=0 then begin t:=r; exit end;
  mpeuclid(r.d,s.d,d);
  if (d[0]=1) and (d[1]=1) then begin { d=1 }
    mpmul(r.n,s.d,temp1);   mpmul(r.d,s.n,temp2);
    mpadd(temp1,temp2,t.n); mpmul(r.d,s.d,t.d)
  end
  else begin { d<>1 }
    mpdiv(r.d,d,rdd,temp1); mpdiv(s.d,d,s.d,temp1);
    mpmul(r.n,s.d,temp1);   mpmul(s.n,rdd,temp2);
    mpadd(temp1,temp2,t.n);
    if t.n[0]=0 then begin t:=zerorat; exit end;
    mpmul(r.d,s.d,t.d);     mpeuclid(t.n,d,d);
    if (d[0]=1) and (d[1]=1) then exit;
    mpdiv(t.n,d,t.n,temp1); mpdiv(t.d,d,t.d,temp1);
  end
end;{ratadd}

procedure ratsub(r,s:rational; var t:rational);
var d,rdd,temp1,temp2:mp;
begin
  if (r.d[0]=0) or (s.d[0]=0) then begin s:=undefrat; exit end;
  { Negate s and use addition algorithm }
  s.n[0]:=-s.n[0];
  if r.n[0]=0 then begin t:=s; exit end;
  if s.n[0]=0 then begin t:=r; exit end;
  mpeuclid(r.d,s.d,d);
  if (d[0]=1) and (d[1]=1) then begin { d=1 }
    mpmul(r.n,s.d,temp1);   mpmul(r.d,s.n,temp2);
    mpadd(temp1,temp2,t.n); mpmul(r.d,s.d,t.d)
  end
  else begin { d<>1 }
    mpdiv(r.d,d,rdd,temp1); mpdiv(s.d,d,s.d,temp1);
    mpmul(r.n,s.d,temp1);   mpmul(s.n,rdd,temp2);
    mpadd(temp1,temp2,t.n);
    if t.n[0]=0 then begin t:=zerorat; exit end;
    mpmul(r.d,s.d,t.d);     mpeuclid(t.n,d,d);
    if (d[0]=1) and (d[1]=1) then exit;
    mpdiv(t.n,d,t.n,temp1); mpdiv(t.d,d,t.d,temp1);
  end
end;{ratsub}

procedure ratmul(r,s:rational; var t:rational);
var d1,d2,rem:mp;
begin
  if (r.d[0]=0) or (s.d[0]=0) then begin t:=undefrat; exit end;
  if (r.n[0]=0) or (s.n[0]=0) then begin t:=zerorat;  exit end;
  mpeuclid(r.n,s.d,d1); mpeuclid(r.d,s.n,d2);
  if (d1[0]<>1) or (d1[1]<>1) then begin { d1<>1 }
    mpdiv(r.n,d1,r.n,rem); mpdiv(s.d,d1,s.d,rem) end;
  if (d2[0]<>1) or (d2[1]<>1) then begin { d2<>1 }
    mpdiv(s.n,d2,s.n,rem); mpdiv(r.d,d2,r.d,rem) end;
  mpmul(r.n,s.n,t.n); mpmul(r.d,s.d,t.d)
end;{ratmul}

procedure ratdiv(r,s:rational; var t:rational);
var sign:integer; d1,d2,rem:mp;
begin
  if (r.d[0]=0) or (s.d[0]=0) then begin t:=undefrat; exit end;
  if (r.n[0]=0) then begin t:=zerorat; exit end;
  mpeuclid(r.n,s.n,d1); mpeuclid(r.d,s.d,d2);
  sign:=intsign(s.n[0]);
  if (d1[0]<>1) or (d1[1]<>1) then begin { d1<>1 }
    mpdiv(r.n,d1,r.n,rem); mpdiv(s.n,d1,s.n,rem) end;
  if (d2[0]<>1) or (d2[1]<>1) then begin { d2<>1 }
    mpdiv(s.d,d2,s.d,rem); mpdiv(r.d,d2,r.d,rem) end;
  mpmul(r.n,s.d,t.n);  mpmul(r.d,s.n,t.d);
  t.n[0]:=t.n[0]*sign; t.d[0]:=abs(t.d[0])
end;{ratdiv}

procedure ratmulint(r:rational; s:integer; var t:rational);
var g,smp:mp;
begin
  if (r.d[0]=0) then begin t:=undefrat; exit end;
  smp[0]:=intsign(s); smp[1]:=abs(s); mpeuclid(smp,r.d,g);
  if (g[0]=1) and (g[1]=1) then begin mpmulint(r.n,s,t.n); t.d:=r.d end
   else begin mpdiv(smp,g,t.n,smp); mpmul(r.n,t.n,t.n); mpdiv(r.d,g,t.d,g) end
end;{ratmulint}

procedure ratdivint(r:rational; s:integer; var t:rational);
var sign:integer; g,smp:mp;
begin
  if (r.d[0]=0) or (s=0) then begin t:=undefrat; exit end;
  {mpmulint(r.d,abs(s),t.d); t.n:=r.n; t.n[0]:=intsign(s)*t.n[0]; ratcancel(t)}
  sign:=intsign(s); s:=abs(s); smp[0]:=1; smp[1]:=s; mpeuclid(smp,r.n,g);
  if (g[0]=1) and (g[1]=1) then begin mpmulint(r.d,s,t.d); t.n:=r.n end
  else begin mpdiv(smp,g,t.d,smp); mpmul(r.d,t.d,t.d); mpdiv(r.n,g,t.n,g) end;
  if sign<0 then t.n[0]:=-t.n[0]
end;{ratdivint}

function ratequal(var x,y:rational):integer;  { 1 if x<>y, 0 if x=y }
var n,d:integer;
begin
  n:=mpcomp(x.n,y.n); d:=mpcomp(x.d,y.d);
  if (n=0) and (d=0) then ratequal:=0 else ratequal:=1;
end;{ratequal}

procedure ratrandom(d:integer; var r:rational); { get a random rational }
var i:integer;
begin
  r.n[0]:=d; if random<0.5 then r.n[0]:=-r.n[0];
  for i:=1 to d do  r.n[i]:=random(base);
  while r.n[1]=0 do r.n[1]:=random(base);
  r.d[0]:=intmax(1,d);
  for i:=1 to r.d[0] do r.d[i]:=random(base);
  while r.d[1]=0     do r.d[1]:=random(base);
  ratcancel(r)
end;{mprandom}

procedure arithinit;
var i:integer;
begin
  for i:=0 to maxprec do begin zeromp[i]:=0; onemp[i]:=0; twomp[i]:=0 end;
  onemp[0]:=1;         onemp[1]:=1;
  twomp[0]:=1;         twomp[1]:=2;
  undefrat.n:=onemp;   undefrat.d:=zeromp;
  zerorat.n :=zeromp;  zerorat.d :=onemp;
  onerat.n  :=onemp;   onerat.d  :=onemp;
  tworat.n  :=twomp;   tworat.d  :=onemp;
  mperror:=0
end;{arithinit}

BEGIN { initialization }
  arithinit;
END.


