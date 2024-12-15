From msuinfo!agate!howland.reston.ans.net!usc!nic-nac.CSU.net!charnel.ecst.csuchico.edu!psgrain!nntp.cs.ubc.ca!alberta!adec23!ersys!alpha3!russell Wed Jul 13 16:42:50 1994
Path: msuinfo!agate!howland.reston.ans.net!usc!nic-nac.CSU.net!charnel.ecst.csuchico.edu!psgrain!nntp.cs.ubc.ca!alberta!adec23!ersys!alpha3!russell
From: russell@alpha3.ersys.edmonton.ab.ca (Russell Schulz)
Newsgroups: alt.sources,rec.arts.book,rec.arts.sf.written
Subject: isbn - give the check digit
Message-ID: <940707.201542.7i3.rusnews.w164w@alpha3.ersys.edmonton.ab.ca>
Date: Thu, 7 Jul 1994 20:15:42 -0600
Reply-To: russell@alpha3.ersys.edmonton.ab.ca (Russell Schulz)
Organization: Private System, Edmonton, AB, Canada
X-Newsreader: rusnews v1.15+
Lines: 37
Xref: msuinfo alt.sources:10569 rec.arts.sf.written:71740

Archive-name: isbn.pas
Submitted-by: russell@alpha3.ersys.edmonton.ab.ca (Russell Schulz)

the latest Del Ray Internet Newsletter gave a ton of ISBNs, but
missing the leading digit.  it was embarassing phoning a book store
knowing all but the first digit, so I copied the simple algorithm
out of my coding theory book and checked:  it's a zero.

       9
      ---
x   = >   i x   (mod 11)
 10   ---    i
      i=1

program isbn;

var
  s: string;
  i: integer;
  j: integer;

begin
  write('what are the first 9 digits of the isbn: ');
  readln(s);
  if length(s)<>9 then
    halt(1);
  j := 0;
  for i := 1 to 9 do
    j := j+i*(ord(s[i])-ord('0'));
  j := j mod 11;
  if j=10 then
    writeln('last digit is X')
  else
    writeln('last digit is ',j);
end.
-- 
Russell Schulz  russell@alpha3.ersys.edmonton.ab.ca  ersys!rschulz  Shad 86c

