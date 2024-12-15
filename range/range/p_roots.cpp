#define main_program
#include <fstream.h>
#include "range.h"
#include "complex.h"
#include "vector.h"
#include "cvector.h"
#include "ivector.h"

//     This program finds the roots of a polynomial with real coefficients.

void combine(cvector&, ivector&, int);
int compute_error(vector&, cvector&, ivector&);
void display_roots(cvector&, ivector&, int, int);
void find_approx(vector&, cvector&);
void find_roots(vector&, cvector&, ivector&);

int num_roots, find_roots_failure; rvar delta;

void main()
{
cout<<"\nProgram to find the roots of a polynomial with real coefficients\n\n";

if (cin.seekg(0,ios::beg).tellg() != cin.seekg(0,ios::end).tellg()) {
     svar::input_from_file = TRUE;}
else svar::input_from_file = FALSE;
cin.seekg(0,ios::beg); cin.clear();

set_precision(30);
int i, n, n1;
int_entry("Enter the degree of the polynomial (1 to 20)    ", n, 1, 20, "",
"20 is the maximum\n");
n1 = n+1; vector p(n1); svector pe(n1);
cout << "\n"; list_tokens(zero);
svar s;
for (i=n; i>=0; i--) {
     s = svar("Enter coefficient of x^") + ltos(i) + svar("   ");
     if (i < 10) s+= svar(" ");
     entry(!s, p(i), pe(i)); 
     if (i == n && p(i) == zero) {
          cout << "Leading cefficient can not be zero\n";
          i++;}}
int d1, d2;
int_entry("\
Enter the solution print format: 1 for fixed decimal point\n\
                                 2 for floating decimal point    ", d1, 1, 2);
int_entry("\
Enter the number of decimal places desired   ", d2, 0, 20, "",
"20 decimal places is the maximum\n");

if (svar::input_from_file == FALSE && svar::input_logs_allowed == TRUE) {
     ofstream input_log("p_roots.log");
     input_log << ltos(n) << "\n";
     for (i=n; i>=0; i--) input_log << pe(i) << "\n";
     input_log << ltos(d1) << "\n" << ltos(d2) << "\n";
     input_log.close();}

delta = ten ^ rvar(-(d2+2));
set_precision(d2 + 15);
vector q(n); cvector roots; ivector multiplicity;
while (1) {
     for (i=0; i<n; i++) q(i) = p(i) / p(n);
     find_roots(q, roots, multiplicity);
     if (find_roots_failure) add_precision(8);
     else {
          test(roots, d1, d2);
          if (test_failure) {
               cout << "Roots insufficiently accurate;";
               cout << " computation repeated at higher precision\n";
               int m=0;
               for (i=0; i<num_roots; i++) {
                    if (multiplicity(i) > m) m = multiplicity(i);}
               test_failure = (test_failure+1) * m;
               add_precision();}
          else break;}
     evaluate(p, pe);}
display_roots(roots, multiplicity, d1, d2);
}

void combine(cvector& roots, ivector& multiplicity, int i)
{
// Remove all roots equal to roots(i), or, if none are equal,
// then remove the closest one to roots(i).
int equal_count = 0, j, k; complex z = roots(i);
for (j=0; j<num_roots; j++) {
     if (i == j) continue;
     if (roots(j) == z) {
          ++equal_count;
          multiplicity(i) += multiplicity(j); --num_roots;
          for (k=j; k<num_roots; k++) {
               roots(k) = roots(k+1);
               multiplicity(k) = multiplicity(k+1);}}}
if (equal_count == 0) {// try again and combine closest root
     j = -1; rvar temp, minimum;
     for (k=0; k<num_roots; k++) {
          if (k == i) continue;
          temp = abs_square(z - roots(k));
          if (j < 0 || temp < minimum) {minimum = temp; j = k;}}
     if (j == -1) error("unexpected difficulty combining roots");
     multiplicity(i) += multiplicity(j);
     --num_roots;
     for (k=j; k<num_roots; k++) {
          roots(k) = roots(k+1);
          multiplicity(k) = multiplicity(k+1);}}
roots.no_comps_reduced_to(num_roots);
multiplicity.no_comps_reduced_to(num_roots);
}

int compute_error(vector& q, cvector& roots, ivector& multiplicity)
{
// Computation of error bound and arrangement of roots in order;
// if no problem return -1;
// otherwise return i to indicate roots(i) must be combined with others.

int i, i1, j, k, l, m, m1, m2, n = q.no_comps(), n1 = n-1, repeat, t;
rvar r, r1, Num_roots(num_roots);
complex z, z1, z2;
vector p(n), bound(n); cvector upper(n), lower(n);

for (i=0; i<num_roots; i++) {
     z = roots(i); m = multiplicity(i); m1 = m-1;
     for (j=1; j<m; j++) {
          upper(j) = c_zero; lower(j) = c_zero;}
     upper(0) = c_one; lower(0) = c_one;
     for (j=n1; j>=0; j--) { // polynomial evaluation
          for (k=m1; k>0; k--) upper(k) = upper(k) * z + upper(k-1);
          upper(0) = complex(q(j)) + (upper(0) * z);}
     for (j=0; j<num_roots; j++) { // denominator evaluation
          if (j == i) continue;
          z1 = z - roots(j);
          m2 = multiplicity(j);
          for (k=0; k<m2; k++) {
               for (l=m1; l>0; l--) {
                    lower(l) = lower(l) * z1 + lower(l-1);}
               lower(0) = lower(0) * z1;}}
     z1 = lower(0);
     if (z1 == c_zero) return i; // need to combine roots(i) with others
     // start division of upper by lower; result in upper
     for (j=0; j<m; j++) {
          z2 = upper(j);
          for (k=0; k<j; k++) z2 = z2 - upper(k) * lower(j-k);
          upper(j) = z2 / z1;}
     for (j=0; j<m; j++) {
          p(j) = Num_roots * (abs(upper(j).real) + abs(upper(j).imag));}
     if (m == 1) bound(i) = p(0);
     else {          
          for (j=0; j<m1; j++) p(j) = p(j) ^ (one / rvar(m-j));
          r = zero;
          for (j=0; j<m; j++) r = max(p(j), r);
          r += r;
          bound(i) = r;}}

for (i=0; i<num_roots; i++) {
     z = roots(i);
     for (j=i+1; j<num_roots; j++) {
          z1 = z - roots(j);
          r = bound(i) + bound(j);
          if (r < abs(z1.real) || r < abs(z1.imag) || 
               r * r < abs_square(z1) ) continue;
          else return i;}}
for (i=0; i<num_roots; i++) {
                    roots(i) = roots(i) % complex(bound(i), bound(i));}

// Sort adjacent roots by real part, or by imag part if real parts equal
do {
     repeat = FALSE;
     for (i=1; i<num_roots; i++) {
          i1 = i - 1;
          if (roots(i1).real < roots(i).real ||
          roots(i1).real == roots(i).real &&
          roots(i1).imag < roots(i).imag) {
               swap(roots(i), roots(i1));
               t = multiplicity(i1); multiplicity(i1) = multiplicity(i);
               multiplicity(i) = t;
               repeat = TRUE;}}}
while (repeat);
return -1;
}

void display_roots(cvector& roots, ivector& multiplicity, int d1, int d2)
{
int i, j, n = roots.no_comps();
int lengthr = 0, lengthi = 0, fieldr = 14, fieldi = 14;
for (i=0; i<n; i++) {
     j = rtos(roots(i).real,d1,d2).len(); if (j > lengthr) lengthr = j;
     j = rtos(roots(i).imag,d1,d2).len(); if (j > lengthi) lengthi = j;}

if (lengthr > fieldr) fieldr = lengthr;
if (lengthi > fieldi) fieldi = lengthi;

int total_length = 2 + 3 + fieldr + 2 + fieldi + 2;
cout.setf(ios::right, ios::adjustfield);
cout << "\n\n";
cout.width(total_length/2 - 8); cout << "";
cout << "Polynomial Roots\n";

int frontsp_r, backsp_r, frontsp_i, backsp_i;
frontsp_r = (fieldr - 9)/2; backsp_r = fieldr - frontsp_r - 9;
frontsp_i = (fieldi - 14)/2; backsp_i = fieldi - frontsp_i - 14;
cout.width(fieldr + fieldi + 11); cout << "";
cout << "Apparent\n";
cout << "No.  "; cout.width(frontsp_r); cout << "";
cout << "Real Part"; cout.width(backsp_r + 2 + frontsp_i); cout << "";
cout << "Imaginary Part"; cout.width(backsp_i); cout << "";
cout << "  multiplicity\n";

if (lengthr < fieldr) {
     frontsp_r = (fieldr - lengthr)/2;
     backsp_r = fieldr - frontsp_r - lengthr;}
else {frontsp_r = 0; backsp_r = 0;}
if (lengthi < fieldi) {
     frontsp_i = (fieldi - lengthi)/2;
     backsp_i = fieldi - frontsp_i - lengthi;}
else {frontsp_i = 0; backsp_i = 0;}

for (i=0; i<n; i++) {
     cout.width(2); cout << i+1 << "   ";
     cout.width(frontsp_r); cout << "";
     cout.width(lengthr);
     if (d1 != 1) cout.setf(ios::left, ios::adjustfield);
     cout << rtos(roots(i).real, d1, d2);
     cout.setf(ios::right, ios::adjustfield);
     cout.width(backsp_r + 2 + frontsp_i); cout << "";
     cout.width(lengthi);
     if (d1 != 1) cout.setf(ios::left, ios::adjustfield);
     cout << rtos(roots(i).imag, d1, d2);
     cout.setf(ios::right, ios::adjustfield);
     cout.width(backsp_i + 7); cout << "";
     cout.width(2); cout << multiplicity(i) << "\n";}
}

void find_approx(vector& p, cvector& roots)
{
// Root approximations are found to the monic polynomial whose
// coefficients are held in vector p. As each approximation is found, 
// it is put in cvector roots, component(num_roots), and the integer
// num_roots incremented.

int i, deg = p.no_comps();
if (deg == 1) { 
     roots(num_roots++) = complex(-p(0));
     cout<< "No. of root approximations found = " << num_roots << "\r";
     cout.flush(); return;}

// If deg exceeds 2,  a radius in the complex plane bounding all roots
// is computed and stored in z_radius.

vector q(deg); rvar a, z_radius, x, poly, poly1;
if (deg > 2) {
     for (i=0; i<deg; i++) {
          q(i) = abs(p(i));
          if (q(i) > z_radius) z_radius = q(i);}
     z_radius = z_radius + one;
     while (1) {
          x = z_radius; poly1 = one; poly = x - q(deg-1);
          for (i=deg-2; i>=0; i--) {
               poly1 = poly1 * x + poly; poly = poly * x - q(i);}
          x = mid(x - poly / poly1); a = abs(z_radius - x); z_radius = x;
          if (a < one) break;}}

// If the degree of p is odd then a first root is found by Newton's method.
int restart = FALSE, first_cycle;
if ( (deg % 2) == 1) {
     rvar left_limit, right_limit, x1;
     left_limit = -z_radius; right_limit = z_radius;
     do {
          x = div2(left_limit + right_limit);
          first_cycle = TRUE; restart = FALSE;
          do {
               poly1 = one; poly = x + p(deg-1);
               for (i=deg-2; i>=0; i--) {
                    poly1 = poly1 * x + poly; poly = poly * x + p(i);}
               if (first_cycle) {
                    if (poly < zero) left_limit = x; 
                    else if (poly > zero) right_limit = x; else break;
                    first_cycle = FALSE;}
               if (poly1 == zero) {
                    restart = TRUE; break;};
               x1 = x - poly / poly1;
               if (abs(x1) > z_radius) {
                    restart = TRUE; break;}
               if (x1 == x) break; else x = mid(x1);}
          while (1);}
     while (restart);
     roots(num_roots++) = complex(x);
     cout<< "No. of root approximations found = " << num_roots << "\r";
     cout.flush();
     x1 = p(--deg) + x;
     for (i = deg-1; i>=0; i--) { // divide out factor x - root
          poly = p(i); p(i) = mid(x1); x1 = poly + x1 * x;}}

// The rest of the roots are found by Bairstow's method.

rvar a1, b, b1, u, u1, delta_u, v, v1, delta_v, t, poly_size, last_size;
rvar limit1 = z_radius * rvar(4), limit2 = z_radius * z_radius * rvar(2);
complex last = rvar("1.1"), multiplier = last * exp(complex(zero, one));
int trouble_count;

restart = FALSE;
while (deg > 0) {
     if (deg == 2) {
          u = p(1); v = p(0);}
     else do {
          if (restart) {
               add_precision(4);
               last = last * multiplier; restart = FALSE;
               u = mid(last.real * rvar(2));
               v = mid(abs_square(last));}
          else {
               u = zero; v = zero;}
          first_cycle = TRUE;
          do {
               b = one; a = p(deg-1) - u; q(deg-1) = a;
               for (i=deg-2; i>0; i--) {
                    t = p(i) - a * u - b * v; q(i) = t; b = a; a = t;}
               b = p(0) - b * v; poly_size = abs(a) + abs(b);
               if (first_cycle) {
                    last_size = poly_size;
                    first_cycle = FALSE; trouble_count = 0;}
               else if (poly_size < last_size) {
                    last_size = poly_size; trouble_count = 0;}
               else {
                    if (a == zero && b == zero) break;
                    if (trouble_count++ > 3) {
                         restart = TRUE; break;}
                    delta_u = div2(delta_u); delta_v = div2(delta_v);
                    u = mid(u - delta_u); v = mid(v - delta_v); continue;}
               b1 = one; a1 = q(deg-1) - u;
               for (i=deg-2; i>2; i--) {
                    t = q(i) - a1 * u - b1 * v; b1 = a1; a1 = t;}
               b1 = q(2) - b1 * v; t = b1 * (b1 - a1 * u) + a1 * a1 * v;
               if (t == zero) {
                    restart = TRUE; break;}
               delta_u = (a * b1 - a1 * b) / t;
               delta_v = (a1 * (a * v - b * u) + b1 * b) / t;
               while(abs(delta_u) > limit1 || abs(delta_v) > limit2) {
                     delta_u = delta_u / ten; delta_v = delta_v / ten;}
               u1 = u + delta_u; v1 = v + delta_v;
               if (u1 == u && v1 == v) break;
               u = mid(u1); v = mid(v1);}
          while (1);}
     while (restart);
     a = -div2(u); b = a * a - v;
     if (b >= zero) {
          b = sqrt(b);
          roots(num_roots++) = complex(mid(a + b));
          roots(num_roots++) = complex(mid(a - b));}
     else {
          b = mid(sqrt(-b)); a = mid(a);
          roots(num_roots++) = complex(a,  b);
          roots(num_roots++) = complex(a, -b);}
     cout<< "No. of root approximations found = " << num_roots << "\r";
     cout.flush();
     deg -= 2;
     for (i=0; i<deg; i++) p(i) = mid(q(i+2));}
}

void find_roots(vector& q, cvector& roots, ivector& multiplicity)
{
int n = q.no_comps(), n1 = n+1;
roots = cvector(n); multiplicity = ivector(n); 

find_roots_failure = FALSE;
num_roots = 0;
// This part of program applies the Euclidean algorithm to split polynomial
// into a set of problem polynomials each with only simple roots. If the
// original polynomial has only simple roots, then it does not split.
// The number of problem polynomials obtained is held in no_poly.

int deg1 = n, deg2, dif, i, i1, j, j1, j2, k, k1, no_poly=0, t;
ivector deg(n); matrix p1(n, n1); vector p;
rvar temp;

for (i=0; i<n; i++) p1(0,i) = q(i); // copy q as first poly
p1(0,n) = one; deg(0) = n;

while (1) {
     deg(no_poly++) = deg1; if (deg1 <= 1) break; j1 = no_poly; j2 = j1-1;
     for (i=0; i<=deg1; i++) p1(j1, i) = p1(j2, i); // copy p(x)
     j2 = j1 + 1; deg2 = deg1 - 1;
     for (i=1; i<=deg1; i++) p1(j2, i-1) = p1(j1, i) * rvar(i); // p'(x)
     do {     // Euclidean algorithm
          while (deg1 >= deg2) {
               dif = deg1 - deg2;
               if (p1(j2, deg2) == zero) {
                    find_roots_failure = TRUE; return;}
               temp = p1(j1, deg1--) / p1(j2, deg2);
               for (i=deg2-1; i>=0; i--) {
                    i1 = i+dif; p1(j1, i1) = p1(j1, i1) - temp * p1(j2, i);}}
          while (deg1 >= 0 && p1(j1, deg1) == zero) deg1--;
          t = deg1; deg1 = deg2; deg2 = t; t = j1; j1 = j2; j2 = t;}
     while (deg2 > 0);
     if (deg2 < 0 && deg1 > 0) {
          if (no_poly != j1) 
               for (i=0; i<=deg1; i++) p1(no_poly, i) = p1(j1, i);}
     else break;}

for (i=1; i<no_poly; i++) { // make polynomials monic
     k = deg(i); temp = p1(i, k);
     if (temp == zero) {
          find_roots_failure = TRUE; return;}
     for (j=0; j<k; j++) p1(i, j) = p1(i, j) / temp;
     p1(i, k) = one;}

for (i=1; i<=2; i++) { // construct problem polynomials by division
     for (j=1; j<no_poly; j++) {     
          j1 = j-1; deg1 = deg(j1); deg2 = deg(j);
          for (k=deg1; k>deg2; k--) {
               temp = p1(j1, k); dif = k - deg2;
               for (k1=deg2-1; k1>=0; k1--) {
                    t = k1+dif; p1(j1, t) = p1(j1, t) - temp * p1(j, k1);}}
          dif = deg1 - deg2;
          for (k=0; k<dif; k++) {
               p1(j1, k) = p1(j1, k+deg2);}
          p1(j1, dif) = one;
          deg(j1) = dif;}}

for (i=0; i<no_poly; i++) { // check accuracy of problem polynomials
     k = deg(i);
     for (j=0; j<k; j++) {
          if (range(p1(i,j)) > delta) {
               find_roots_failure = TRUE; return;}}}

cout << "No. of root approximations found = 0\r"; cout.flush();
     
for (i=0; i<no_poly; i++) { // find roots of problem polynomials
     k = deg(i);
     if (k == 0) continue;
     p = vector(k); t = i + 1;
     for (j=0; j<k; j++) {
          p(j) = mid(p1(i,j));
          multiplicity(num_roots + j) = t;}
     find_approx(p, roots);}
roots.no_comps_reduced_to(num_roots);
multiplicity.no_comps_reduced_to(num_roots);
cout << "\nNow testing root approximations for accuracy\n"; cout.flush();
while (1) {
     i = compute_error(q, roots, multiplicity); 
     if (i < 0) break; else combine(roots, multiplicity, i);}
}




