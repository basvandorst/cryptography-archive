%!PS-Adobe-2.0 EPSF-1.2
%%DocumentFonts: Helvetica Times-Roman Helvetica-Bold
%%Pages: 1
%%BoundingBox: 82 123 548 589
%%EndComments

50 dict begin

/arrowHeight 8 def
/arrowWidth 4 def
/none null def
/numGraphicParameters 17 def
/stringLimit 65535 def

/Begin {
save
numGraphicParameters dict begin
} def

/End {
end
restore
} def

/SetB {
dup type /nulltype eq {
pop
false /brushRightArrow idef
false /brushLeftArrow idef
true /brushNone idef
} {
/brushDashOffset idef
/brushDashArray idef
0 ne /brushRightArrow idef
0 ne /brushLeftArrow idef
/brushWidth idef
false /brushNone idef
} ifelse
} def

/SetCFg {
/fgblue idef
/fggreen idef
/fgred idef
} def

/SetCBg {
/bgblue idef
/bggreen idef
/bgred idef
} def

/SetF {
/printSize idef
/printFont idef
} def

/SetP {
dup type /nulltype eq {
pop true /patternNone idef
} {
dup -1 eq {
/patternGrayLevel idef
/patternString idef
} {
/patternGrayLevel idef
} ifelse
false /patternNone idef
} ifelse
} def

/BSpl {
0 begin
storexyn
newpath
n 1 gt {
0 0 0 0 0 0 1 1 true subspline
n 2 gt {
0 0 0 0 1 1 2 2 false subspline
1 1 n 3 sub {
/i exch def
i 1 sub dup i dup i 1 add dup i 2 add dup false subspline
} for
n 3 sub dup n 2 sub dup n 1 sub dup 2 copy false subspline
} if
n 2 sub dup n 1 sub dup 2 copy 2 copy false subspline
patternNone not brushLeftArrow not brushRightArrow not and and { ifill } if
brushNone not { istroke } if
0 0 1 1 leftarrow
n 2 sub dup n 1 sub dup rightarrow
} if
end
} dup 0 4 dict put def

/Circ {
newpath
0 360 arc
patternNone not { ifill } if
brushNone not { istroke } if
} def

/CBSpl {
0 begin
dup 2 gt {
storexyn
newpath
n 1 sub dup 0 0 1 1 2 2 true subspline
1 1 n 3 sub {
/i exch def
i 1 sub dup i dup i 1 add dup i 2 add dup false subspline
} for
n 3 sub dup n 2 sub dup n 1 sub dup 0 0 false subspline
n 2 sub dup n 1 sub dup 0 0 1 1 false subspline
patternNone not { ifill } if
brushNone not { istroke } if
} {
Poly
} ifelse
end
} dup 0 4 dict put def

/Elli {
0 begin
newpath
4 2 roll
translate
scale
0 0 1 0 360 arc
patternNone not { ifill } if
brushNone not { istroke } if
end
} dup 0 1 dict put def

/Line {
0 begin
2 storexyn
newpath
x 0 get y 0 get moveto
x 1 get y 1 get lineto
brushNone not { istroke } if
0 0 1 1 leftarrow
0 0 1 1 rightarrow
end
} dup 0 4 dict put def

/MLine {
0 begin
storexyn
newpath
n 1 gt {
x 0 get y 0 get moveto
1 1 n 1 sub {
/i exch def
x i get y i get lineto
} for
patternNone not brushLeftArrow not brushRightArrow not and and { ifill } if
brushNone not { istroke } if
0 0 1 1 leftarrow
n 2 sub dup n 1 sub dup rightarrow
} if
end
} dup 0 4 dict put def

/Poly {
3 1 roll
newpath
moveto
-1 add
{ lineto } repeat
closepath
patternNone not { ifill } if
brushNone not { istroke } if
} def

/Rect {
0 begin
/t exch def
/r exch def
/b exch def
/l exch def
newpath
l b moveto
l t lineto
r t lineto
r b lineto
closepath
patternNone not { ifill } if
brushNone not { istroke } if
end
} dup 0 4 dict put def

/Text {
ishow
} def

/idef {
dup where { pop pop pop } { exch def } ifelse
} def

/ifill {
0 begin
gsave
patternGrayLevel -1 ne {
fgred bgred fgred sub patternGrayLevel mul add
fggreen bggreen fggreen sub patternGrayLevel mul add
fgblue bgblue fgblue sub patternGrayLevel mul add setrgbcolor
eofill
} {
eoclip
originalCTM setmatrix
pathbbox /t exch def /r exch def /b exch def /l exch def
/w r l sub ceiling cvi def
/h t b sub ceiling cvi def
/imageByteWidth w 8 div ceiling cvi def
/imageHeight h def
bgred bggreen bgblue setrgbcolor
eofill
fgred fggreen fgblue setrgbcolor
w 0 gt h 0 gt and {
l b translate w h scale
w h true [w 0 0 h neg 0 h] { patternproc } imagemask
} if
} ifelse
grestore
end
} dup 0 8 dict put def

/istroke {
gsave
brushDashOffset -1 eq {
[] 0 setdash
1 setgray
} {
brushDashArray brushDashOffset setdash
fgred fggreen fgblue setrgbcolor
} ifelse
brushWidth setlinewidth
originalCTM setmatrix
stroke
grestore
} def

/ishow {
0 begin
gsave
fgred fggreen fgblue setrgbcolor
/fontDict printFont findfont printSize scalefont dup setfont def
/descender fontDict begin 0 [FontBBox] 1 get FontMatrix end
transform exch pop def
/vertoffset 0 descender sub printSize sub printFont /Courier ne
printFont /Courier-Bold ne and { 1 add } if def {
0 vertoffset moveto show
/vertoffset vertoffset printSize sub def
} forall
grestore
end
} dup 0 3 dict put def

/patternproc {
0 begin
/patternByteLength patternString length def
/patternHeight patternByteLength 8 mul sqrt cvi def
/patternWidth patternHeight def
/patternByteWidth patternWidth 8 idiv def
/imageByteMaxLength imageByteWidth imageHeight mul
stringLimit patternByteWidth sub min def
/imageMaxHeight imageByteMaxLength imageByteWidth idiv patternHeight idiv
patternHeight mul patternHeight max def
/imageHeight imageHeight imageMaxHeight sub store
/imageString imageByteWidth imageMaxHeight mul patternByteWidth add string def
0 1 imageMaxHeight 1 sub {
/y exch def
/patternRow y patternByteWidth mul patternByteLength mod def
/patternRowString patternString patternRow patternByteWidth getinterval def
/imageRow y imageByteWidth mul def
0 patternByteWidth imageByteWidth 1 sub {
/x exch def
imageString imageRow x add patternRowString putinterval
} for
} for
imageString
end
} dup 0 12 dict put def

/min {
dup 3 2 roll dup 4 3 roll lt { exch } if pop
} def

/max {
dup 3 2 roll dup 4 3 roll gt { exch } if pop
} def

/arrowhead {
0 begin
transform originalCTM itransform
/taily exch def
/tailx exch def
transform originalCTM itransform
/tipy exch def
/tipx exch def
/dy tipy taily sub def
/dx tipx tailx sub def
/angle dx 0 ne dy 0 ne or { dy dx atan } { 90 } ifelse def
gsave
originalCTM setmatrix
tipx tipy translate
angle rotate
newpath
0 0 moveto
arrowHeight neg arrowWidth 2 div lineto
arrowHeight neg arrowWidth 2 div neg lineto
closepath
patternNone not {
originalCTM setmatrix
/padtip arrowHeight 2 exp 0.25 arrowWidth 2 exp mul add sqrt brushWidth mul
arrowWidth div def
/padtail brushWidth 2 div def
tipx tipy translate
angle rotate
padtip 0 translate
arrowHeight padtip add padtail add arrowHeight div dup scale
arrowheadpath
ifill
} if
brushNone not {
originalCTM setmatrix
tipx tipy translate
angle rotate
arrowheadpath
istroke
} if
grestore
end
} dup 0 9 dict put def

/arrowheadpath {
newpath
0 0 moveto
arrowHeight neg arrowWidth 2 div lineto
arrowHeight neg arrowWidth 2 div neg lineto
closepath
} def

/leftarrow {
0 begin
y exch get /taily exch def
x exch get /tailx exch def
y exch get /tipy exch def
x exch get /tipx exch def
brushLeftArrow { tipx tipy tailx taily arrowhead } if
end
} dup 0 4 dict put def

/rightarrow {
0 begin
y exch get /tipy exch def
x exch get /tipx exch def
y exch get /taily exch def
x exch get /tailx exch def
brushRightArrow { tipx tipy tailx taily arrowhead } if
end
} dup 0 4 dict put def

/midpoint {
0 begin
/y1 exch def
/x1 exch def
/y0 exch def
/x0 exch def
x0 x1 add 2 div
y0 y1 add 2 div
end
} dup 0 4 dict put def

/thirdpoint {
0 begin
/y1 exch def
/x1 exch def
/y0 exch def
/x0 exch def
x0 2 mul x1 add 3 div
y0 2 mul y1 add 3 div
end
} dup 0 4 dict put def

/subspline {
0 begin
/movetoNeeded exch def
y exch get /y3 exch def
x exch get /x3 exch def
y exch get /y2 exch def
x exch get /x2 exch def
y exch get /y1 exch def
x exch get /x1 exch def
y exch get /y0 exch def
x exch get /x0 exch def
x1 y1 x2 y2 thirdpoint
/p1y exch def
/p1x exch def
x2 y2 x1 y1 thirdpoint
/p2y exch def
/p2x exch def
x1 y1 x0 y0 thirdpoint
p1x p1y midpoint
/p0y exch def
/p0x exch def
x2 y2 x3 y3 thirdpoint
p2x p2y midpoint
/p3y exch def
/p3x exch def
movetoNeeded { p0x p0y moveto } if
p1x p1y p2x p2y p3x p3y curveto
end
} dup 0 17 dict put def

/storexyn {
/n exch def
/y n array def
/x n array def
n 1 sub -1 0 {
/i exch def
y i 3 2 roll put
x i 3 2 roll put
} for
} def

%%EndProlog

%I Idraw 7 Grid 4 

%%Page: 1 1

Begin
%I b u
%I cfg u
%I cbg u
%I f u
%I p u
%I t
[ 0.8 0 0 0.8 0 0 ] concat
/originalCTM matrix currentmatrix def

Begin %I Pict
%I b u
%I cfg u
%I cbg u
%I f u
%I p u
%I t
[ 0.716418 0 0 0.716418 530.546 119.281 ] concat

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.627307 0 0 0.627307 80.2143 182.107 ] concat
%I
[
(LEGEND:)
] Text
End

Begin %I Rect
%I b 13107
1 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
none SetP %I p n
%I t
[ 0.654202 0 0 0.60069 -400.392 -142.735 ] concat
%I
720 443 925 558 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.627307 0 0 0.627307 84.0428 162.964 ] concat
%I
[
([X]K)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.627307 0 0 0.627307 84.0428 147.649 ] concat
%I
[
({X}K)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.627307 0 0 0.627307 114.671 147.649 ] concat
%I
[
(X signed by K)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.627307 0 0 0.627307 114.671 162.964 ] concat
%I
[
(X encrypted by K)
] Text
End

End %I eop

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-times-medium-r-*-120-*
/Times-Roman 12 SetF
%I t
[ 1 0 0 1 277 169 ] concat
%I
[
(Figure 10: Alternative software key escrow system)
] Text
End

Begin %I Rect
%I b 65535
2 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
none SetP %I p n
%I t
[ 0.270831 0 0 0.221034 161.019 464.16 ] concat
%I
377 195 606 295 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.593194 0 0 0.593194 165.791 438.619 ] concat
%I
[
(        Sending)
(Software Program)
] Text
End

Begin %I Rect
%I b 65535
2 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
none SetP %I p n
%I t
[ 0.270831 0 0 0.513393 310.053 523.826 ] concat
%I
377 195 606 295 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.593194 0 0 0.593194 419.439 670.453 ] concat
%I
[
(Key Escrow)
(   Agent 1)
] Text
End

Begin %I Rect
%I b 65535
2 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
none SetP %I p n
%I t
[ 0.348492 0 0 0.30264 303.664 433.77 ] concat
%I
377 195 606 295 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.593194 0 0 0.593194 440.137 515.638 ] concat
%I
[
(Law Enforcement)
(     Decryptor)
] Text
End

Begin %I Rect
%I b 13107
2 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
none SetP %I p n
%I t
[ 0.437823 0 0 0.931858 -12.621 168.536 ] concat
%I
377 195 606 295 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.593194 0 0 0.593194 266.549 525.432 ] concat
%I
[
(    Software)
(Vendor / User)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 110.039 393.566 ] concat
%I
[
(M)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 317.323 393.373 ] concat
%I
[
(LEAF = [KS1]KEA1pub, [KS2]KEA2pub)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 334.229 404.371 ] concat
%I
[
(encrypted message C = [M]KS)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 107.278 379.24 ] concat
%I
[
(KS)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 183.055 392.159 ] concat
%I
[
(KEA1pub)
(KEA2pub)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0 -0.593194 0.593194 0 508.585 576.928 ] concat
%I
[
(KS2)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 466.417 698.485 ] concat
%I
[
(KEA2pub)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 401.266 713.093 ] concat
%I
[
(KEA1pub)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.593194 0 0 0.593194 484.859 670.453 ] concat
%I
[
(Key Escrow)
(   Agent 2)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 424.736 646.157 ] concat
%I
[
(KEA1pub)
(KEA1priv)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 490.155 647.051 ] concat
%I
[
(KEA2pub)
(KEA2priv)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0 -0.593194 0.593194 0 466.646 576.928 ] concat
%I
[
(KS1)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0 -0.593194 0.593194 0 489.535 598.92 ] concat
%I
[
([KS2]KEA2pub)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0 -0.593194 0.593194 0 447.596 598.92 ] concat
%I
[
([KS1]KEA1pub)
] Text
End

Begin %I Line
%I b 65535
1 0 1 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.552135 0 0 0.552135 62.8872 291.78 ] concat
%I
109 177 148 177 Line
End

Begin %I Line
%I b 65535
1 0 1 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.552135 0 0 0.552135 62.8872 296.778 ] concat
%I
109 147 148 147 Line
End

Begin %I Line
%I b 65535
1 0 1 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.577062 0 0 0.644157 92.5694 278.508 ] concat
%I
288 167 378 167 Line
End

Begin %I Rect
%I b 65535
2 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
none SetP %I p n
%I t
[ 0.270831 0 0 0.513393 375.472 523.826 ] concat
%I
377 195 606 295 Rect
End

Begin %I Line
%I b 65535
1 0 1 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.602329 0 0 0.803516 141.527 196.461 ] concat
%I
512 416 512 521 Line
End

Begin %I Line
%I b 65535
1 0 1 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.602329 0 0 0.803516 185.726 196.461 ] concat
%I
522 521 522 416 Line
End

Begin %I Line
%I b 65535
1 0 1 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.602329 0 0 0.803516 183.467 196.461 ] concat
%I
512 416 512 521 Line
End

Begin %I Line
%I b 65535
1 0 1 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.602329 0 0 0.803516 143.787 196.59 ] concat
%I
522 521 522 416 Line
End

Begin %I Line
%I b 65535
1 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.552135 0 0 0.265054 75.5687 358.025 ] concat
%I
557 192 557 351 Line
End

Begin %I Line
%I b 65535
1 0 1 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.634803 0 0 0.591979 137.008 330.162 ] concat
%I
600 299 645 299 Line
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 551.062 510.37 ] concat
%I
[
(M)
] Text
End

Begin %I Line
%I b 65535
1 0 1 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.462511 0 0 0.552135 339.989 293.424 ] concat
%I
288 167 378 167 Line
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.593194 0 0 0.593194 542.573 428.109 ] concat
%I
[
(       Receiving)
(Software Program)
] Text
End

Begin %I Rect
%I b 13107
2 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
none SetP %I p n
%I t
[ 0.505632 0 0 1.03047 330.833 128.531 ] concat
%I
377 195 606 295 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 477.82 355.226 ] concat
%I
[
(KS)
] Text
End

Begin %I Line
%I b 65535
1 0 1 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.552135 0 0 0.552135 432.428 292.764 ] concat
%I
104 116 148 147 Line
End

Begin %I Line
%I b 65535
1 0 1 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.634803 0 0 0.591979 260.476 204.601 ] concat
%I
600 299 645 299 Line
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 674.529 384.809 ] concat
%I
[
(M)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 350.838 381.849 ] concat
%I
[
(EVS = [ KS1, KS2 ]KS)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 561.598 402.417 ] concat
%I
[
(KEA1pub)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 561.598 392.918 ] concat
%I
[
(KEA2pub)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 550.412 360.178 ] concat
%I
[
([ KS1]KEA1pub,)
([KS2]KEA2pub)
(     =? LEAF)
] Text
End

Begin %I Line
%I b 13107
1 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.780863 0 0 0.724074 93.2356 204.371 ] concat
%I
137 374 391 374 Line
End

Begin %I Line
%I b 13107
1 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.724074 0 0 0.666148 115.441 311.968 ] concat
%I
391 245 640 245 Line
End

Begin %I Line
%I b 13107
1 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.724074 0 0 0.6945 115.441 302.981 ] concat
%I
640 245 640 619 Line
End

Begin %I Line
%I b 13107
1 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.738164 0 0 0.724074 106.425 284.674 ] concat
%I
640 619 127 619 Line
End

Begin %I Line
%I b 13107
1 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.724074 0 0 1.03294 108.2 93.4825 ] concat
%I
127 619 127 374 Line
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.593194 0 0 0.593194 521.951 726.598 ] concat
%I
[
(  Protected)
(Environment)
] Text
End

Begin %I Line
%I b 65535
1 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.988324 0 0 0.907642 -140.602 174.364 ] concat
%I
530 305 619 305 Line
End

Begin %I Line
%I b 65535
1 0 1 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.907642 0 0 0.749429 -90.6111 222.185 ] concat
%I
619 305 619 359 Line
End

Begin %I Line
%I b 65535
1 1 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.676023 0 0 0.676023 16.4816 281.131 ] concat
%I
392 372 392 621 Line
End

Begin %I Line
%I b 65535
1 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
1 SetP
%I t
[ 0.676023 0 0 0.676023 16.4816 281.131 ] concat
%I
392 621 631 621 Line
End

Begin %I Line
%I b 65535
1 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
1 SetP
%I t
[ 0.676023 0 0 0.477193 16.4816 404.605 ] concat
%I
631 621 631 571 Line
End

Begin %I Line
%I b 65535
1 1 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.676023 0 0 0.676023 16.4816 281.131 ] concat
%I
426 372 426 601 Line
End

Begin %I Line
%I b 65535
1 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
1 SetP
%I t
[ 0.676023 0 0 0.676023 16.4816 281.131 ] concat
%I
426 601 725 601 Line
End

Begin %I Line
%I b 65535
1 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
1 SetP
%I t
[ 0.676023 0 0 0.346256 16.4816 479.321 ] concat
%I
725 601 725 571 Line
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 342.534 371.709 ] concat
%I
[
(where KS = KS1 XOR KS2)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.593194 0 0 0.593194 538.912 377.428 ] concat
%I
[
(KS1 XOR KS2 =? KS)
] Text
End

End %I eop

showpage

%%Trailer

end
