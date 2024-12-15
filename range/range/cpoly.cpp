#include "range.h"
#include "complex.h"
#include "vector.h"
#include "cvector.h"
#include "ivector.h"
#include "cpoly.h"

void find_approx(cvector& p, cvector& roots, int& no_roots)
{
// Root approximations are found to the complex monic polynomial whose
// coefficients are held in cvector p.  As each approximation is found,
// it is put in cvector roots, component(no_roots), and the integer
// no_roots incremented.

int i, deg = p.no_comps(), deg1 = deg-1, deg2 = deg1-1;
if (deg == 1) {roots(no_roots++) = -p(0); return;}

// A radius in the complex plane bounding all roots
// is computed and stored in z_radius.

vector q(deg); rvar z_radius, x, poly;
for (i=0; i<deg; i++) q(i) = abs(p(i).real) + abs(p(i).imag);
z_radius = tenth;
do {
     poly = one;
     for (i=deg-1; i>=0; i--) poly = poly * z_radius - q(i);
     if (poly > zero) break; else z_radius = z_radius * ten;}
while (1);

// The roots are found by Newton's method.

complex last(rvar("1.1")), multiplier = last * exp(complex(zero, one)),
     z, z1, zpoly, zpoly1, delta_z;
rvar last_size, poly_size;
int first_cycle, restart = FALSE, trouble_count;

while (deg > 0) {
     if (restart) {
          last = last * multiplier; restart = FALSE;
          z = mid(last);}
     else { z = - p(deg1) / rvar(deg);}
     first_cycle = TRUE;
     while (1) {
          zpoly1 = c_one; zpoly = z + p(deg1);
          for (i=deg2; i>=0; i--) {
               zpoly1 = zpoly1 * z + zpoly; zpoly = zpoly * z + p(i);}
          if (zpoly == c_zero) break;
          poly_size = abs(zpoly.real) + abs(zpoly.imag);
          if (first_cycle) {
               last_size = poly_size;
               first_cycle = FALSE; trouble_count = 0;}
          else if (poly_size < last_size) {
               last_size = poly_size; trouble_count = 0;}
          else {
               if (trouble_count++ > 3) {restart = TRUE; break;}
               delta_z.real = div2(delta_z.real);
               delta_z.imag = div2(delta_z.imag);
               z = mid(z + delta_z); continue;}
          if (zpoly1 == c_zero) {restart = TRUE; break;}
          delta_z = zpoly / zpoly1;
          while (abs(delta_z.real) + abs(delta_z.imag) > z_radius) {
               delta_z = delta_z / ten;}
          z1 = z - delta_z;
          if (z1 == z) break; z = mid(z1);}
     if (restart) continue;
     roots(no_roots++) = z;
     z1 = mid(p(--deg) + z); deg1 = deg2--;
     for (i=deg1; i>=0; i--) { // divide out factor z - root
          zpoly = p(i); p(i) = z1; z1 = mid(zpoly + z1 * z);}}
}

void combine(cvector& roots, ivector& multiplicity, int i)
{
// Remove all roots equal to roots(i), or, if none are equal,
// then remove the closest one to roots(i).
int equal_count = 0, j, k, n = roots.no_comps(); complex z = roots(i);
for (j=0; j<n; j++) {
     if (i == j) continue;
     if (roots(j) == z) {
          ++equal_count;
          multiplicity(i) += multiplicity(j); --n;
          for (k=j; k<n; k++) {
               roots(k) = roots(k+1);
               multiplicity(k) = multiplicity(k+1);}}}
if (equal_count == 0) {// try again and combine closest root
     j = -1; rvar temp, minimum;
     for (k=0; k<n; k++) {
          if (k == i) continue;
          temp = abs_square(z - roots(k));
          if (j < 0 || temp < minimum) {minimum = temp; j = k;}}
     if (j == -1) error("unexpected difficulty combining roots");
     multiplicity(i) += multiplicity(j);
     --n;
     for (k=j; k<n; k++) {
          roots(k) = roots(k+1);
          multiplicity(k) = multiplicity(k+1);}}
roots.no_comps_reduced_to(n); multiplicity.no_comps_reduced_to(n);
}

int compute_error(cvector& q, cvector& roots, ivector& multiplicity)
{
// Computation of error bound and arrangement of roots in order;
// if no problem return -1;
// otherwise return i to indicate roots(i) must be combined with others.

int i, i1, j, k, l, m, m1, m2, n = q.no_comps(), n1 = n-1, 
     no_roots = roots.no_comps(), repeat, t;
rvar r, r1, No_roots(no_roots);
complex z, z1, z2;
vector p(n), bound(n); cvector upper(n), lower(n);

for (i=0; i<no_roots; i++) {
     z = roots(i); m = multiplicity(i); m1 = m-1;
     for (j=1; j<m; j++) {upper(j) = c_zero; lower(j) = c_zero;}
     upper(0) = c_one; lower(0) = c_one;
     for (j=n1; j>=0; j--) { // polynomial evaluation
          for (k=m1; k>0; k--) upper(k) = upper(k) * z + upper(k-1);
          upper(0) = q(j) + (upper(0) * z);}
     for (j=0; j<no_roots; j++) { // denominator evaluation
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
          p(j) = No_roots * (abs(upper(j).real) + abs(upper(j).imag));}
     if (m == 1) bound(i) = p(0);
     else {          
          for (j=0; j<m1; j++) p(j) = p(j) ^ (one / rvar(m-j));
          r = zero;
          for (j=0; j<m; j++) r = max(p(j), r);
          bound(i) = r;}}

for (i=0; i<no_roots; i++) {
     z = roots(i); r = bound(i);
     for (j=i+1; j<no_roots; j++) {
          if ((r + bound(j)) < abs(z - roots(j)) ) continue;
          else return i;}}
for (i=0; i<no_roots; i++) {
     roots(i) = roots(i) % complex(bound(i), bound(i));}

// Sort adjacent roots by real part, or by imag part if real parts equal
do {
     repeat = FALSE;
     for (i=1; i<no_roots; i++) {
          i1 = i - 1;
          if (roots(i1).real < roots(i).real ||
          roots(i1).real == roots(i).real &&
          roots(i1).imag < roots(i).imag) {
               z = roots(i1); roots(i1) = roots(i); roots(i) =  z;
               t = multiplicity(i1); multiplicity(i1) = multiplicity(i);
               multiplicity(i) = t;
               repeat = TRUE;}}}
while (repeat);
return -1;
}

void find_roots(cvector& q, cvector& roots, ivector& multiplicity, int need_err)
{
int no_roots = 0, n = q.no_comps(), n1 = n+1;
roots = cvector(n); multiplicity = ivector(n); 

// This part of program applies the Euclidean algorithm to split polynomial
// into a set of problem polynomials each with only simple roots. If the
// original polynomial has only simple roots, then it does not split.
// The number of problem polynomials obtained is held in no_poly.

int deg1 = n, deg2, dif, i, i1, j, j1, j2, k, k1, no_poly=0, t;
ivector deg(n); cmatrix p1(n, n1); cvector p;
complex temp;

for (i=0; i<n; i++) p1(0,i) = q(i); // copy q as first poly
p1(0,n) = c_one; deg(0) = n;

while (1) {
     deg(no_poly++) = deg1; if (deg1 <= 1) break; j1 = no_poly; j2 = j1-1;
     for (i=0; i<=deg1; i++) p1(j1, i) = p1(j2, i); // copy polynomial s(x)
     j2 = j1 + 1; deg2 = deg1 - 1;
     for (i=1; i<=deg1; i++) p1(j2, i-1) = rvar(i) * p1(j1, i); // s'(x)
     do {     // Euclidean algorithm
          while (deg1 >= deg2) {
               dif = deg1 - deg2; temp = p1(j1, deg1--) / p1(j2, deg2);
               for (i=deg2-1; i>=0; i--) {
                    i1 = i+dif; p1(j1, i1) = p1(j1, i1) - temp * p1(j2, i);}}
          while (deg1 >= 0 && p1(j1, deg1) == c_zero) deg1--;
          t = deg1; deg1 = deg2; deg2 = t; t = j1; j1 = j2; j2 = t;}
     while (deg2 > 0);
     if (deg2 < 0 && deg1 > 0) {
          if (no_poly != j1) 
               for (i=0; i<=deg1; i++) p1(no_poly, i) = p1(j1, i);}
     else break;}

for (i=1; i<no_poly; i++) { // make polynomials monic
     k = deg(i); temp = p1(i, k);
     for (j=0; j<k; j++) p1(i, j) = p1(i, j) / temp;
     p1(i, k) = c_one;}

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
          p1(j1, dif) = c_one;
          deg(j1) = dif;}}

for (i=0; i<no_poly; i++) { // find roots of problem polynomials
     k = deg(i); 
     if (k == 0) continue;
     p = cvector(k); t = i + 1;
     for (j=0; j<k; j++) {
          p(j) = mid(p1(i,j));
          multiplicity(no_roots + j) = t;}
     find_approx(p, roots, no_roots);}
roots.no_comps_reduced_to(no_roots);
multiplicity.no_comps_reduced_to(no_roots);
if (need_err) {
      while (1) {
           i = compute_error(q, roots, multiplicity); 
           if (i < 0) break; else combine(roots, multiplicity, i);}}
}








