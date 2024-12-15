#include "range.h"
#include "complex.h"
#include "vector.h"
#include "cvector.h"
#include "ivector.h"
#include "poly.h"

void find_approx(vector& p, cvector& roots, int& no_roots)
{
// Root approximations are found to the real monic polynomial whose
// coefficients are held in vector p.  As each approximation is found,
// it is put in cvector roots, component(no_roots), and the integer
// no_roots incremented.

int i, deg = p.no_comps();
if (deg == 1) {roots(no_roots++) = complex(-p(0)); return;}

// If deg exceeds 2,  a radius in the complex plane bounding all roots
// is computed and stored in z_radius.

vector q(deg); rvar a, z_radius, x, poly, poly1;
if (deg > 2) {
     z_radius = tenth;
     for (i=0; i<deg; i++) q(i) = abs(p(i));
     do {
          poly = one;
          for (i=deg-1; i>=0; i--) poly = poly * z_radius - q(i);
          if (poly > zero) break; else z_radius = z_radius * ten;}
     while (1);}    

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
               if (poly1 == zero) {restart = TRUE; break;};
               x1 = x - poly / poly1;
               if (abs(x1) > z_radius) {restart = TRUE; break;}
               if (x1 == x) break; else x = mid(x1);}
          while (1);}
     while (restart);
     roots(no_roots++) = complex(x);
     x1 = mid(p(--deg) + x);
     for (i = deg-1; i>=0; i--) { // divide out factor x - root
          poly = p(i); p(i) = x1; x1 = mid(poly + x1 * x);}}

// The rest of the roots are found by Bairstow's method.

rvar a1, b, b1, u, u1, delta_u, v, v1, delta_v, t, poly_size, last_size;
rvar limit1 = z_radius * rvar(4), limit2 = z_radius * z_radius * rvar(2);
complex last = rvar("1.1"), multiplier = last * exp(complex(zero, one));
int trouble_count;

restart = FALSE;
while (deg > 0) {
     if (deg == 2) { u = p(1); v = p(0);}
     else do {
          if (restart) {
               last = last * multiplier; restart = FALSE;
               u = mid(last.real * rvar(2));
               v = mid(abs_square(last));}
          else { u = zero; v = zero;}
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
                    if (trouble_count++ > 3) {restart = TRUE; break;}
                    delta_u = div2(delta_u); delta_v = div2(delta_v);
                    u = mid(u - delta_u); v = mid(v - delta_v); continue;}
               b1 = one; a1 = q(deg-1) - u;
               for (i=deg-2; i>2; i--) {
                    t = q(i) - a1 * u - b1 * v; b1 = a1; a1 = t;}
               b1 = q(2) - b1 * v; t = b1 * (b1 - a1 * u) + a1 * a1 * v;
               if (t == zero) {restart = TRUE; break;}
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
          roots(no_roots++) = complex(mid(a + b));
          roots(no_roots++) = complex(mid(a - b));}
     else {
          b = mid(sqrt(-b)); a = mid(a);
          roots(no_roots++) = complex(a,  b);
          roots(no_roots++) = complex(a, -b);}
     deg -= 2;
     for (i=0; i<deg; i++) p(i) = mid(q(i+2));}
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

int compute_error(vector& q, cvector& roots, ivector& multiplicity)
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
          upper(0) = complex(q(j)) + (upper(0) * z);}
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
          p(j) = No_roots * abs(upper(j));}
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

void find_roots(vector& q, cvector& roots, ivector& multiplicity, int need_err)
{
int no_roots = 0, n = q.no_comps(), n1 = n+1;
roots = cvector(n); multiplicity = ivector(n); 

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
     for (i=0; i<=deg1; i++) p1(j1, i) = p1(j2, i); // copy polynomial s(x)
     j2 = j1 + 1; deg2 = deg1 - 1;
     for (i=1; i<=deg1; i++) p1(j2, i-1) = p1(j1, i) * rvar(i); // s'(x)
     do {     // Euclidean algorithm
          while (deg1 >= deg2) {
               dif = deg1 - deg2; temp = p1(j1, deg1--) / p1(j2, deg2);
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
          for (k=0; k<dif; k++) p1(j1, k) = p1(j1, k+deg2);
          p1(j1, dif) = one;
          deg(j1) = dif;}}

for (i=0; i<no_poly; i++) { // find roots of problem polynomials
     k = deg(i);
     if (k == 0) continue;
     p = vector(k); t = i + 1;
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








