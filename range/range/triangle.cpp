#define main_program
#include "range.h"

rvar triangle_input(char*, svar&);
svar u = "u"; rvar rvar_180 = 180;

void main()
{
int i, j, l, n, units, code, error_precision, solutions, maxlength;
rvar alpha, beta, gamma, a, b, c, beta2, gamma2, c2, degrees, minutes;
rvar ra, rb, rc, r1, r2, r3, ra2, rb2, rc2, r12, r22, r32;
svar entry_line, sa, sb, sc, s1, s2, s3, sdegrees, sminutes;
char response;
char *p;

cout << "\nThis program computes unknown angles and sides of triangles.\n";
cout << "\nOn output angles are displayed both in degrees in decimal form and in degrees";
cout << "\nand nearest minute.\n";
cout << "\nOn input angles may be specified in degrees and minutes by entering degrees +";
cout << "\nminutes/60; e.g. 23 degrees and 47 minutes would be entered as 23 + 47/60.\n";

while (1) {
int_entry("\nEnter number of decimal places desired in output: ", n, 0, 100,\
"Enter a number between 0 and 100\n", "Enter a number between 0 and 100\n");
set_precision(20+n);
cout << "\n                                    Angle 3\n";
cout <<   "                                       *\n";
cout <<   "                                     *   *\n";
cout <<   "                           Side B  *       *  Side A\n";
cout <<   "                                 *           *\n";
cout <<   "                               *               *\n";
cout <<   "                    Angle 1  * * * * * * * * * * *  Angle 2\n";
cout <<   "                                     Side C\n\n";
units = 0;
code = 0;
error_precision = 0;
solutions = 1;
do
   r1 = triangle_input("Enter Angle 1 in degrees (or u if unknown): ",
   entry_line);
while (entry_line != u && (r1 <= zero || r1 >= rvar_180));
if (entry_line != u) {
   s1 = entry_line;
   code = code + 1;
   units++;}
do
   r2 = triangle_input("Enter Angle 2 in degrees (or u if unknown): ",
   entry_line);
while (entry_line != u && (r2 <= zero || r2 >= rvar_180));
if (entry_line != u) {
   s2 = entry_line;
   code = code + 2;
   units++;}
if (units < 2) {
   do
      r3 = triangle_input("Enter Angle 3 in degrees (or u if unknown): ",
      entry_line);
   while (entry_line != u && (r3 <= zero || r3 >= rvar_180));
   if (entry_line != u) {
      s3 = entry_line;
      code = code + 4;
      units++;}}

if (code == 3) {
   r3 = rvar_180 - r1 - r2;
   if (r3 <= zero) {
      cout << "\nInconsistent input.\n";
      solutions = 0;}}
if (code == 5) {
   r2 = rvar_180 - r1 - r3;
   if (r2 <= zero) {
      cout << "\nInconsistent input.\n";
      solutions = 0;}}
if (code == 6) {
   r1 = rvar_180 - r2 - r3;
   if (r1 <= zero) {
      cout << "\nInconsistent input.\n";
      solutions = 0;}}

if (solutions) {
   do
      ra = triangle_input("Enter Side A (or u if unknown): ", entry_line);
   while (entry_line != u && ra <= zero);
   if (entry_line != u) {
      sa = entry_line;
      code = code + 8;
      units++;}
   if (units < 3) {
      do
         rb = triangle_input("Enter Side B (or u if unknown): ", entry_line);
      while (entry_line != u && rb <= zero);
      if (entry_line != u) {
         sb = entry_line;
         code = code + 16;
         units++;}}
   if (units < 3) {
      do
         rc = triangle_input("Enter Side C (or u if unknown): ", entry_line);
      while (entry_line != u && rc <= zero);
      if (entry_line != u) {
         sc = entry_line;
         code = code + 32;
         units++;}}
   if (units < 3) {
      cout << "\nInsufficient information supplied.\n";
      solutions = 0;}}

if (solutions) {
while (1) {

switch (code) {

case 11: case 13: case 14: case 42: case 50:
   a = rb; b = rc; c = ra;
   alpha = r2; beta = r3;
   break;

case 19: case 21: case 22: case 28: case 44:
   a = rc; b = ra; c = rb;
   alpha = r3; beta = r1;
   break;

case 25: case 35: case 37: case 38: case 49: case 56:
   a = ra; b = rb; c = rc;
   alpha = r1; beta = r2;
   break;

case 26:
   a = rb; b = ra; c = rc;
   alpha = r2; beta = r1;
   break;

case 41:
   a = ra; b = rc; c = rb;
   alpha = r1; beta = r3; 
   break;

case 52:
   a = rc; b = rb; c = ra;
   alpha = r3; beta = r2;} 

alpha = alpha * pi / rvar_180;
beta = beta*pi / rvar_180;

switch (code) {

case 11: case 13: case 14: case 19: case 21: case 22: case 35: case 37: case 38:
   gamma = pi - alpha - beta;
   if (sin(gamma) == zero) {error_precision = 1; break;}
   a = c*sin(alpha)/sin(gamma);
   b = c*sin(beta)/sin(gamma);
   break;

case 28: case 42: case 49:
   a = sqrt(b*b + c*c - two*b*c*cos(alpha));
   if (a == zero) {error_precision = 1; break;}
   beta = acos((a*a + c*c - b*b)/(two*a*c));
   gamma = pi - alpha - beta;
   break;

case 25: case 26: case 41: case 44: case 50: case 52:
   if (alpha >= div2(pi)) {
      if (a < b) {
         solutions = 0;
         cout << "\nInconsistent input.\n";
         break;}
      c = b*cos(alpha) + sqrt(a*a - b*b*sin(alpha)*sin(alpha));}
   else {
      if (a < b*sin(alpha)) {
         solutions = 0;
         cout << "\nInconsistent input.\n";
         break;}
      if (a == b*sin(alpha)) 
         c = b*cos(alpha) % sqrt(a*a - b*b*sin(alpha)*sin(alpha));
      if (a > b*sin(alpha)) {
         c = b*cos(alpha) + sqrt(a*a - b*b*sin(alpha)*sin(alpha));
         if (a < b) {
            solutions = 2;
            c2 = b*cos(alpha) - sqrt(a*a - b*b*sin(alpha)*sin(alpha));}}}
   gamma = acos((a*a + b*b - c*c)/(two*a*b));
   beta = pi - alpha - gamma;
   if (solutions == 2) {
      gamma2 = acos((a*a + b*b - c2*c2)/(two*a*b));
      beta2 = pi - alpha - gamma2;}
   break;

case 56:
   if ((a + b < c) || (a + c < b) || (b + c < a)) {
      solutions = 0;
      cout << "\nInconsistent input.\n";
      break;}
   alpha = acos((b*b + c*c - a*a)/(two*b*c));
   beta = acos((a*a + c*c - b*b)/(two*a*c));
   gamma = pi - alpha - beta;}

if (solutions == 0) break;

alpha = alpha*rvar_180/pi;
beta = beta*rvar_180/pi;
gamma = gamma*rvar_180/pi;
if (solutions == 2) {
   beta2 = beta2*rvar_180/pi;
   gamma2 = gamma2*rvar_180/pi;}

if (error_precision == 0) {
switch (code) {

case 11: case 13: case 14: case 42: case 50:
   rb = a; rc = b; ra = c;
   r2 = alpha; r3 = beta; r1 = gamma;
   if (solutions == 2) {
      rb2 = a; rc2 = b; ra2 = c2;
      r22 = alpha; r32 = beta2; r12 = gamma2;}
   break;

case 19: case 21: case 22: case 28: case 44:
   rc = a; ra = b; rb = c;
   r3 = alpha; r1 = beta; r2 = gamma;
   if (solutions == 2) {
      rc2 = a; ra2 = b; rb2 = c2;
      r32 = alpha; r12 = beta2; r22 = gamma2;}
   break;

case 25: case 35: case 37: case 38: case 49: case 56:
   ra = a; rb = b; rc = c;
   r1 = alpha; r2 = beta; r3 = gamma;
   if (solutions == 2) {
      ra2 = a; rb2 = b; rc2 = c2;
      r12 = alpha; r22 = beta2; r32 = gamma2;}
   break;

case 26:
   rb = a; ra = b; rc = c;
   r2 = alpha; r1 = beta; r3 = gamma;
   if (solutions == 2) {
      rb2 = a; ra2 = b; rc2 = c2;
      r22 = alpha; r12 = beta2; r32 = gamma2;}
   break;

case 41:
   ra = a; rc = b; rb = c;
   r1 = alpha; r3 = beta; r2 = gamma;
   if (solutions == 2) {
      ra2 = a; rc2 = b; rb2 = c2;
      r12 = alpha; r32 = beta2; r22 = gamma2;}
   break;

case 52:
   rc = a; rb = b; ra = c;
   r3 = alpha; r2 = beta; r1 = gamma;
   if (solutions == 2) {
      rc2 = a; rb2 = b; ra2 = c2;
      r32 = alpha; r22 = beta2; r12 = gamma2;}}

if (n < 2) l = 2;
else       l = n;
test(a,1,l);
test(b,1,l);
test(c,1,l);
test(alpha,1,l);
test(beta,1,l);
test(gamma,1,l);
if (solutions == 2) {
   test(c2,1,l);
   test(beta2,1,l);
   test(gamma2,1,l);}
if (test_failure) add_precision();
else              break;}      

if (error_precision) {
   error_precision = 0;
   add_precision(10);}

if (s1 != empty) evaluate(r1, s1);
if (s2 != empty) evaluate(r2, s2);
if (s3 != empty) evaluate(r3, s3);
if (sa != empty) evaluate(ra, sa);
if (sb != empty) evaluate(rb, sb);
if (sc != empty) evaluate(rc, sc);}}

if (solutions) {
if (solutions == 1) cout << "\nSolution:\n\n";
else                    cout << "\nSolution 1:\n\n";
for (j=0; j<2; j++) {
   sa = rtos(ra,1,n); maxlength = sa.len(); 
   sb = rtos(rb,1,n); l = sb.len(); if (l > maxlength) maxlength = l;
   sc = rtos(rc,1,n); l = sc.len(); if (l > maxlength) maxlength = l;
   s1 = rtos(r1,1,n); l = s1.len(); if (l > maxlength) maxlength = l;
   s2 = rtos(r2,1,n); l = s2.len(); if (l > maxlength) maxlength = l;
   s3 = rtos(r3,1,n); l = s3.len(); if (l > maxlength) maxlength = l;
   l = maxlength - sa.len(); cout << "Side A:  ";
   for (i=0; i<l; i++) cout << " "; cout << sa << "\n";
   l = maxlength - sb.len(); cout << "Side B:  ";
   for (i=0; i<l; i++) cout << " "; cout << sb << "\n";
   l = maxlength - sc.len(); cout << "Side C:  ";
   for (i=0; i<l; i++) cout << " "; cout << sc << "\n";
   l = maxlength - s1.len(); cout << "Angle 1: ";
   for (i=0; i<l; i++) cout << " "; cout << s1 << "\n";
   l = maxlength - s2.len(); cout << "Angle 2: ";
   for (i=0; i<l; i++) cout << " "; cout << s2 << "\n";
   l = maxlength - s3.len(); cout << "Angle 3: ";
   for (i=0; i<l; i++) cout << " "; cout << s3 << "\n";
   degrees = trunc(r1);
   sdegrees = rtos(degrees,3);
   minutes = (r1 - degrees)*rvar(60);
   sminutes = rtos(minutes, 1, 0);
   cout << "Angle 1: ";
   if (degrees < rvar(100)) cout << " ";
   if (degrees < rvar(10))  cout << " ";
   cout << sdegrees << " deg  ";
   p = !sminutes;
   if (sminutes.len() < 4) {cout << " " << *p << " min\n";}
   else               {cout << *p++; cout << *p << " min\n";}
   degrees = trunc(r2);
   sdegrees = rtos(degrees,3);
   minutes = (r2 - degrees)*rvar(60);
   sminutes = rtos(minutes, 1, 0);
   cout << "Angle 2: ";
   if (degrees < rvar(100)) cout << " ";
   if (degrees < rvar(10))  cout << " ";
   cout << sdegrees << " deg  ";
   p = !sminutes;
   if (sminutes.len() < 4) {cout << " " << *p << " min\n";}
   else               {cout << *p++; cout << *p << " min\n";}
   degrees = trunc(r3);
   sdegrees = rtos(degrees,3);
   minutes = (r3 - degrees)*rvar(60);
   sminutes = rtos(minutes, 1, 0);
   cout << "Angle 3: ";
   if (degrees < rvar(100)) cout << " ";
   if (degrees < rvar(10))  cout << " ";
   cout << sdegrees << " deg  ";
   p = !sminutes;
   if (sminutes.len() < 4) {cout << " " << *p << " min\n";}
   else               {cout << *p++; cout << *p << " min\n";}

   if (solutions == 1) break;
   if (j == 0) cout << "\nSolution 2:\n\n";
   ra = ra2; rb = rb2; rc = rc2;
   r1 = r12; r2 = r22; r3 = r32;}}

do {
   svar_entry("\nDo you wish to continue the program (y/n)? ", entry_line);
   char* p = !entry_line;
   while(*p == ' ') p++;
   response = *p;
   if (response == 'N') response = 'n';
   if (response == 'Y') response = 'y';}
while (response != 'y' && response != 'n');
if (response == 'n') break;}
}


rvar triangle_input( char* message, svar& entry_line)
{ // this function is a modified version of entry function
rvar cc;
while (1) { 
   svar_entry(message, entry_line);
   if (entry_line == u) return zero;
   compile(entry_line);
   if (errorstatus == no_error) {
      evaluate(cc, entry_line); 
      if (rvar::evaluate_error == FALSE) return cc;}}
}

