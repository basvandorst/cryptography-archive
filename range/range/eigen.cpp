#define main_program
#include <fstream.h>
#include "range.h"
#include "complex.h"
#include "vector.h"
#include "cvector.h"
#include "ivector.h"
#include "poly.h"

/*     This program finds eigenvalues and, if desired, eigenvector components
     of a real square matrix of size n by n.  First the matrix is transformed
     so that it consists of one or more companion matrices in diagonal
     position. Then each companion matrix is transformed so that the
     eigenvalues are along the diagonal, with each set of k multiple
     eigenvalues having k-1 ones appearing just above them.
     If an accuracy check is passed, then eigenvalues and eigenvectors are 
     displayed, otherwise precision is increased and the process repeated.
     When eigenvalue components are desired, the transforming matrix Q
     is kept track of, and its final components give the eigenvector
     components.
*/
void diagonal_test();
void diagonalize_companion();
void find_companion();
void print_Q_col(int);
void print_solution();
     
enum eigen_type{group_member, group_end, list_end, eigenvector,
     near_eigenvector};

int  d, distinct_count, eigenvectors_wanted, n, n1, num_blocks,
     test_trouble, transform_trouble;
rvar delta, delta1, delta2, delta_min;
matrix A; cmatrix A1, B, C, I, Q;
smatrix Ae;
cvector eigenvalues;
ivector companion_bounds, eigen_mult, v_type; imatrix distinct, kind;

void main()
{
cout << "\n" <<
"Program to find the eigenvalues and eigenvectors of a real square matrix A\n\n";

if (cin.seekg(0,ios::beg).tellg() != cin.seekg(0,ios::end).tellg()) {
     svar::input_from_file = TRUE;}
else svar::input_from_file = FALSE;
cin.seekg(0,ios::beg); cin.clear();

set_precision(30);
int_entry("Enter the number of rows of A  ", n, 1, 20, "", "20 rows is max\n");
n1 = n+1;
A = matrix(n,n); Q = cmatrix(n, n);
int i, j;
I = cmatrix(n,n); for (i=0; i<n; i++) I(i,i).real = one;

int_entry("\
Enter 0 for eigenvalues only\n\
  and 1 for both eigenvalues and eigenvector components   ", 
eigenvectors_wanted, 0, 1);
  
int_entry("\
Enter the number of decimal places desired for displayed results   ", d,
0, 20, "", "20 decimal places is the maximum\n");
set_precision(d + n*2 + 5);
delta =  (ten ^ (rvar(-(d+1))) ) * rvar(".2"); // error goal
delta1 =  div2(delta); // error radius for eigenvalues
delta2 = ten ^ rvar(-(d+1)); // test value for near eigenvectors
delta_min = delta1 / rvar(2*n);
// If two calculated eigenvalues are found to be closer than delta_min,
// they are counted as equal.
list_tokens(zero);
entry("enter A", A, Ae);

if (svar::input_from_file == FALSE && svar::input_logs_allowed == TRUE) {
     ofstream input_log("eigen.log");
     input_log << ltos(n) << "\n" << ltos(eigenvectors_wanted) << "\n";
     input_log << ltos(d) << "\n";
     for (i=0; i<n; i++) for (j=0; j<n; j++) input_log << Ae(i,j) << "\n";
     input_log.close();}

cout << "Now computing eigenvalue ";
if (eigenvectors_wanted) cout << "and eigenvector ";
cout << "approximations\n"; cout.flush();
while (1) {
     if (eigenvectors_wanted) Q = copy(I); 
     find_companion();
     diagonalize_companion();
     if (transform_trouble) add_precision(12);
     else {
          diagonal_test();
          if (test_trouble) add_precision(12);
          else {
               if (test_failure) {
                    if (test_failure < 12) test_failure = 12;
                    add_precision();}
               else break;}}
     cout << "Precision increase to " << current_precision() << "\n";
     cout.flush();
     evaluate(A, Ae);}
print_solution(); 
}

void diagonal_test()
{
int h, i, j, k;
rvar radius1;
complex center, center1;
vector radius(distinct_count);
test_trouble = FALSE;
// Set up the radius vector; the radius of an eigenvalue set is an upper
// bound on the distance from the lead eigenvalue to any other of the set.
for (i=0; i<distinct_count; i++) {
     k = distinct(i,0); center = A1(k,k); radius1 = zero;
     for (j=0; kind(i,j) != list_end; j++) {
          k = distinct(i,j+1); radius1 += abs(center - A1(k,k));}
     radius(i) = radius1;}

// Check for overlapping eigenvalues; sets may have to be combined.
int k1, repeat; rvar dist;
do {
     repeat = FALSE;
     for (i=0; i<distinct_count; i++) {
          k = distinct(i,0); center = A1(k,k);
          for (j=i+1; j<distinct_count; j++) {
               k = distinct(j,0); center1 = A1(k,k);
               dist = abs(center - center1) - radius(i) - radius(j);
               if (dist > delta_min) continue;
               repeat = TRUE;
               v_type(i) = near_eigenvector;
               // add j list to i list
               radius(i) += radius(j) + abs(center - center1);
               eigen_mult(i) += eigen_mult(j);
               k = 0;
               while (kind(i,k) != list_end) k++;
               kind(i,k) = group_end; k1 = 0;
               do {
                    distinct(i,++k) = distinct(j,k1);
                    kind(i,k) = kind(j,k1);}
               while (kind(j,k1++) != list_end);
               // move lists after j back
               for (k=j+1; k<distinct_count; k++) {
                    k1 = k-1;
                    radius(k1) = radius(k); eigen_mult(k1) = eigen_mult(k);
                    v_type(k1) = v_type(k);
                    for (h=0; h<n; h++) {
                         distinct(k1,h) = distinct(k,h);
                         kind(k1,h) = kind(k,h);}}
               j--; distinct_count--;}}}
while (repeat);

// determination of max_upper and max_lower
rvar max_upper, max_lower, upper, lower; // initial values are zero
for (i=0; i<n; i++) {
     upper = zero; lower = zero;
     for (j=0; j<i; j++) lower += abs(A1(i,j));
     for (j=i+1; j<n; j++) upper += abs(A1(i,j));
     max_upper = max(upper, max_upper);
     max_lower = max(lower, max_lower);}

// determination of min distance between eigenvalue sets and max_radius
rvar min_dist, max_radius;
if (distinct_count > 1) {
     k = distinct(0, 0); k1 = distinct(1, 0);
     min_dist = abs(A1(k, k)) + abs(A1(k1, k1));
     for (i=0; i<distinct_count; i++) {
          max_radius = max(max_radius, radius(i));
          k = distinct(i, 0); center = A1(k, k);
          for (j=i+1; j<distinct_count; j++) {
               k1 = distinct(j, 0);
               dist = abs(center - A1(k1, k1)) - radius(i) - radius(j);
               min_dist = min(min_dist, dist);}}}
else max_radius = radius(0);
               
// Delta is error in eigenvalues and eigenvectors to be achieved.
// The range of computed eigenvalues and eigenvectors is assumed
// no larger than delta1 = delta / 2,
// and the error expressions are required to be less than delta1.
// If max_radius is too large, precision must be increased
// and computation repeated.
rvar test_delta = delta1 - max_radius, eigen_error = delta1;
if (test_delta <= zero) {
     do {
          max_radius /= ten;}
     while (++test_failure < 50 && max_radius >= delta1);
     return;} // very large eigenvalues may require big precision increase
// Determination of final value for min_dist.
if (distinct_count > 1) {
     dist = min_dist - test_delta - test_delta;
     if (dist > zero) min_dist = dist;
     else {
          min_dist = div2(min_dist);
          test_delta = div2(min_dist);
          eigen_error = test_delta + max_radius;}}

// determination of max eigenvalue multiplicity
int max_eigen_mult = 0; 
for (i=0; i<distinct_count; i++) {
     if (eigen_mult(i) > max_eigen_mult) max_eigen_mult = eigen_mult(i);}

// determination of test_failure for eigenvalues
rvar q, r, temp;
if (distinct_count == 1) q = one;
else {
     if (max_upper == zero) { // max_upper may have wide interval
          if (max_upper < one) q = min(one, min_dist);
          else {
               test_trouble = TRUE; return;}}
     else {
          temp = min_dist / max_upper;
          q = min(one, temp);}}
temp = rvar(2) * max_upper * q; 
if (temp == zero) { // may have wide interval
     if (temp < test_delta) r = one;
     else {
          test_trouble = TRUE; return;}}
else {
     temp = test_delta / temp;
     r = min(one, temp);}
temp = (max_lower * rvar(2)) /
     ( test_delta * (r ^ rvar(max_eigen_mult)) * ( q ^ rvar(n-1) ) );
while (temp >= one && ++test_failure < 50) temp = temp / ten;
if (test_failure) return; 

eigenvalues = cvector(distinct_count);
center1 = complex(eigen_error, eigen_error);
for (i=0; i<distinct_count; i++) {
     k = distinct(i,0);
     eigenvalues(i) = A1(k,k) % center1;
     test(eigenvalues(i),1,d);}
if (test_failure) return;

if (eigenvectors_wanted) {
     int I, J, first_near_eigenvector = TRUE, l1, l2, s;
     cmatrix D; complex divisor, z;
     rvar norm, diag_min, Q_norm, term;
     for (i=0; i<distinct_count; i++) {
          if (v_type(i) == eigenvector) {
               I = distinct(i,0); k = 0;
               while (kind(i,k) == group_member) k++;
               J = distinct(i,k); // I > J because eigenvalue rows go hi to lo
               D = copy(A1);
               for (j=0; j<n; j++) {
                    if (j == I) continue;
                    if (j < I && j >= J) D(j,j) = complex(eigen_error);
                    else D(j,j) -= eigenvalues(i);}
               for (j=0; j<distinct_count; j++) { // clear 1's above diagonal
                    if (j == i) continue;
                    k = 0;
                    while (kind(j,k) != list_end) {
                         if (kind(j,k) == group_end) {k++; continue;}
                         l1 = distinct(j,k); l2 = distinct(j,++k);
                         divisor = D(l1,l1);
                         if (divisor == c_zero) {
                              test_trouble = TRUE; return;}
                         z = D(l2,l1) / divisor;
                         for (s=0; s<n; s++) {
                              if (s == l1)  D(l2,s) = c_zero;
                              else D(l2,s) -= z * D(l1,s);}}}
          // Find ||D'||, where D' is D with row I and col J deleted
          // also find min abs value of D' diag elements 
               norm = zero; diag_min = ten;
               for (j=0; j<n; j++) {
                    if (j < J || j > I) k1 = j;
                    else if (j == J) continue;
                    else k1 = j-1;
                    diag_min = min(abs(D(k1,j)), diag_min);
                    temp = zero;
                    for (k=0; k<n; k++) {
                         if (k == I || k == k1) continue;
                         temp += abs(D(k,j));}
                    norm = max(temp, norm);}
               if (norm >= diag_min) {
                    test_trouble = TRUE; return;}
               // find sum of abs values of col J (skip I component)
               temp = zero;
               for (j=0; j<n; j++) {
                    if (j == I) continue;
                    temp += abs(D(j,J));}
               temp /= (diag_min - norm);
               if (temp >= half) {
                    test_trouble = TRUE; return;}
               temp += temp; // final eigenvector component error
               center1 = complex(temp, temp);
               for (j=0; j<n; j++) {
                    Q(j,J) = Q(j,J) % center1;
                    test(Q(j,J), 1, d);}}
          else { // near eigenvector check
               if (first_near_eigenvector) { // find Q_norm
                    Q_norm = zero; 
                    for (j=0; j<n; j++) {
                         temp = zero;
                         for (k=0; k<n; k++) temp += abs(Q(k,j));
                         Q_norm = max(temp, Q_norm);}
                    first_near_eigenvector = FALSE;}
               k = 0;
               do {
                    while (kind(i,k) == group_member) k++;
                    J = distinct(i,k);
                    temp = zero;
                    for (s=1; s<n; s++) {
                         if (s == J) term = eigen_error;
                         else term = abs(A1(s,J));
                         temp += term;}
                    temp *= Q_norm;
                    if (temp >= delta2) {
                         test_trouble = TRUE; return;}
                    for (j=0; j<n; j++) test(Q(j,J), 1, d);}
               while (kind(i,k++) != list_end);}}}
}
               
void diagonalize_companion()
{
int deg, deg1, h, h_hi, h_lo, h_max, i, j, j1, j_row, k, m, num_roots;
complex multiplier, term, z; rvar sum, temp, temp1;
vector p, q(n); cvector roots; ivector multiplicity;
distinct = imatrix(n, n); kind = imatrix(n, n); eigen_mult = ivector(n);
v_type = ivector(n);
B = cmatrix(n,n);
distinct_count = 0; transform_trouble = FALSE;
for (h=0; h<num_blocks; h++) {
     h_lo = companion_bounds(h); h_hi = companion_bounds(h+1);
     h_max = h_hi - 1; j_row = h_lo - 1;
     deg = h_hi - h_lo; p = vector(deg); deg1 = deg - 1;
     for (i=h_lo; i<h_hi; i++) p(i-h_lo) = -A(i,h_max);
     find_roots(p, roots, multiplicity, 0);
     num_roots = roots.no_comps();
     for (i=0; i<num_roots; i++) {
          m = multiplicity(i); eigen_mult(distinct_count) = m; j_row += m;
          v_type(distinct_count) = eigenvector; z = roots(i);
          if (m > 1) for (k=1; k<deg1; k++) q(k) = one; 
          multiplier = c_one;
          for (k=0; k<deg1; k++) {
               B(j_row, h_lo+k) = multiplier; multiplier *= z;}
          B(j_row, h_lo+deg1) = multiplier;
          distinct(distinct_count, 0) = j_row--;
          kind(distinct_count, 0) = group_member;
          for (j=1; j<m; j++) {
               multiplier = c_one; temp = one; term = c_one;
               for (k=j; k<deg1; k++) {
                    temp1 = temp + q(k); q(k) = temp; temp = temp1;
                    B(j_row, h_lo+k) = term;
                    multiplier *= z; term = temp * multiplier;}
               B(j_row, h_lo+deg1) = term;
               distinct(distinct_count, j) = j_row--;
               kind(distinct_count, j) = group_member;}
          kind(distinct_count++, j-1) = list_end; j_row += m;}}
A1 = cmatrix(n,n); // Further work with A may be in the complex field
for (i=0; i<n; i++) for (j=0; j<n; j++) A1(i,j) = complex(A(i,j));
A1 = B * A1;
cmatrix::failure_report_wanted = TRUE;
C = inverse(B);
if (cmatrix::failure) {
     transform_trouble = TRUE; return;}
A1 = A1 * C;
if (eigenvectors_wanted) {
     Q = Q * C;
     // Make all columns of Q which could become eigenvectors or
     // near-eigenvectors have length 1 without disturbing A1 significantly.
     // All the columns of a group are divided by the eigenvector length.
     // This leaves the 1's above the diagonal undisturbed.
     for (h=0; h<distinct_count; h++) {
          j1 = distinct(h,0);
          k = 0; while (kind(h,k) == group_member) k++;
          j = distinct(h,k);
          sum = zero;
          for (i=0; i<n; i++) sum += abs_square(Q(i,j));
          sum = sqrt(sum);
          if (sum == zero) {
               transform_trouble = TRUE; return;}
          z = complex(sum);
          for (i=0; i<n; i++) {
               for (k=j; k<=j1; k++) {
                    Q(i,k) /= z;
                    if (i >= j && i <= j1) continue;
                    A1(i,k) /= z;
                    A1(k,i) *= z;}}}}
}

void find_companion()
{
int block_start = 0, block_end = 0, h, i, j, k, be1, s, t;
rvar divisor, multiplier, sum, temp;
companion_bounds = ivector(n1);
companion_bounds(0) = 0;     num_blocks = 0;
while (1) { // create companion block
     while (1) { // attempt to add column to block
          be1 = block_end + 1;
          if (be1 == n) {block_end++; break;}
          for (i=be1; i<n; i++) {
               if (A(i, block_end) != zero) break;}
          if (i < n) { // non-zero pivot element found
               if (i > be1) {// transform to move pivot element
                    for (k=0; k<n; k++) swap(A(be1,k), A(i,k));// row exchange
                    for (k=0; k<n; k++) swap(A(k,be1), A(k,i));// col exchange
                    if (eigenvectors_wanted) {
                         for (k=0; k<n; k++) swap(Q(k,be1), Q(k,i));}}
               // transform to get another 0 - 1 col
               for (h=0; h<n; h++) {// post-multiply A
                    sum = zero;
                    for (k=0; k<n; k++) {
                         sum += A(h,k) * A(k,block_end);}
                    A(h,be1) = sum;}
               if (eigenvectors_wanted) {
                    for (h=0; h<n; h++) {// post-multiply Q
                         sum = zero;
                         for (k=0; k<n; k++) {
                              sum += Q(h,k).real * A(k,block_end);}
                         Q(h,be1).real = sum;}}
               divisor = A(be1, block_end); A(be1, block_end) = one;
               for (k=0; k<n; k++) {// pre-multiply A
                    if (k != be1) { 
                         multiplier = A(k, block_end) / divisor;
                         A(k, block_end) = zero;
                         for (j=0; j<n; j++) {
                              if (j != block_end) {
                                   A(k,j) -= multiplier * A(be1,j);}}}}
               for (k=0; k<n; k++) {
                    if (k != block_end) {
                         A(be1,k) /= divisor;}}
               block_end++;}
          else { // no non-zero pivot; try to make matrix 0 right of block
               for (k=block_end; k>block_start; k--) {
                    s = k-1;
                    for (j=0; j<n; j++) {// pre-multiply A
                         sum = zero;
                         for (t=be1; t<n; t++) sum += A(k,t) * A(t,j);
                         A(s,j) += sum;}
                    if (eigenvectors_wanted) {
                         for (j=be1; j<n; j++) {// post-multiply Q
                              multiplier = A(k,j);
                              for (t=0; t<n; t++) {
                                   Q(t,j) -= multiplier * Q(t,s);}}}
                    for (j=be1; j<n; j++) {// post-multiply A
                         multiplier = A(k,j); A(k,j) = zero;
                         for (t=0; t<n; t++)
                              if (t != k) {
                                   A(t,j) -= multiplier * A(t,s);}}}
               for (k=be1; k<n; k++) {//any nonzero elements in block_start row?
                    if (A(block_start,k) != zero) break;}
               if (k < n) { // move element to block_start col
                    for (i=0; i<n; i++) {// col cycle A
                         temp = A(i,k);
                         for (j=k; j>block_start; j--) A(i,j) = A(i,j-1);
                         A(i,block_start) = temp;}
                    if (eigenvectors_wanted) {
                         for (i=0; i<n; i++) {// col cycle Q
                              temp = Q(i,k).real;
                              for (j=k; j>block_start; j--) {
                                   Q(i,j).real = Q(i,j-1).real;}
                              Q(i,block_start).real = temp;}}
                    for (i=0; i<n; i++) {// row cycle A
                         temp = A(k,i);
                         for (j=k; j>block_start; j--) A(j,i) = A(j-1,i);
                         A(block_start,i) = temp;}
                    block_end = block_start;}
               else { // companion block constructed
                    block_end++; break;}}} 
     companion_bounds(++num_blocks) = block_end;
     if (block_end == n) break; else block_start = block_end;}
}

void print_Q_col(int col)
{
int backspace, fieldwidth = 14, frontspace, i, printwidth;
svar zero_field = rtos(zero, 1, d);
printwidth = zero_field.len() + 1; // 1 space for possible negative sign
if (printwidth > fieldwidth) fieldwidth = printwidth;
frontspace = (fieldwidth - 9) / 2; backspace = fieldwidth - 9 - frontspace;
cout << "Component";
cout.width(3+frontspace); cout << ""; cout << "Real Part";
cout.width(backspace+3+frontspace); cout << ""; cout << "Imag Part\n";
frontspace = (fieldwidth - printwidth) / 2;
backspace = fieldwidth - frontspace - printwidth;
cout.setf(ios::right, ios::adjustfield);
for (i=0; i<n; i++) {
     cout << "    "; cout.width(2); cout << i+1 << "   ";
     cout.width(3+frontspace); cout << "";
     cout.width(printwidth); cout << rtos(Q(i,col).real, 1, d);
     cout.width(backspace+3+frontspace); cout << "";
     cout.width(printwidth); cout << rtos(Q(i,col).imag, 1, d);
     cout << "\n";}
}

void print_solution()
{
// Sort eigenvalues by real part or, if real parts equal, by imaginary part
int i, j, k, repeat, t; complex z; 
do {
     repeat = FALSE;
     for (i=1; i<distinct_count; i++) {
          j = i - 1;
          if (eigenvalues(j).real < eigenvalues(i).real ||
          eigenvalues(j).real == eigenvalues(i).real &&
          eigenvalues(j).imag < eigenvalues(i).imag) {
               swap(eigenvalues(i), eigenvalues(j));
               t = eigen_mult(j); eigen_mult(j) = eigen_mult(i);
               eigen_mult(i) = t;
               for (k=0; k<n; k++) {
                    t = distinct(i,k); distinct(i,k) = distinct(j,k);
                    distinct(j,k) = t;
                    t = kind(i,k); kind(i,k) = kind(j,k); kind(j,k) = t;}
               repeat = TRUE;}}}
while (repeat);

int lengthr = 0, lengthi = 0, fieldr = 14, fieldi = 14;
for (i=0; i<distinct_count; i++) {
     j = rtos(eigenvalues(i).real,1,d).len(); if (j > lengthr) lengthr = j;
     j = rtos(eigenvalues(i).imag,1,d).len(); if (j > lengthi) lengthi = j;}

if (lengthr > fieldr) fieldr = lengthr;
if (lengthi > fieldi) fieldi = lengthi;

int total_length = 2 + 3 + fieldr + 2 + fieldi + 2;
cout << "\n"; cout.width(total_length/2 - 5); cout << "";
cout << "Eigenvalues\n";

int frontsp_r, backsp_r, frontsp_i, backsp_i;
frontsp_r = (fieldr - 9)/2; backsp_r = fieldr - frontsp_r - 9;
frontsp_i = (fieldi - 14)/2; backsp_i = fieldi - frontsp_i - 14;
cout.width(fieldr + fieldi + 11); cout << "";
cout << "Apparent\n";
cout << "No.  "; cout.width(frontsp_r); cout << ""; cout << "Real Part";
cout.width(backsp_r + 2 + frontsp_i); cout << ""; cout << "Imaginary Part";
cout.width(backsp_i); cout << ""; cout << "  multiplicity\n";

if (lengthr < fieldr) {
     frontsp_r = (fieldr - lengthr)/2;
     backsp_r = fieldr - frontsp_r - lengthr;}
else {frontsp_r = 0; backsp_r = 0;}
if (lengthi < fieldi) {
     frontsp_i = (fieldi - lengthi)/2;
     backsp_i = fieldi - frontsp_i - lengthi;}
else {frontsp_i = 0; backsp_i = 0;}

for (i=0; i<distinct_count; i++) {
     cout.width(2); cout << i+1 << "   ";
     cout.width(frontsp_r); cout << "";
     cout.width(lengthr); cout << rtos(eigenvalues(i).real, 1, d);
     cout.width(backsp_r + 2 + frontsp_i); cout << "";
     cout.width(lengthi); cout << rtos(eigenvalues(i).imag, 1, d);
     cout.width(backsp_i + 7); cout << "";
     cout.width(2); cout << eigen_mult(i) << "\n";}

if (eigenvectors_wanted) {
     int first_near_vector = TRUE, num_vectors;
     cout << "\n          Eigenvectors\n";
     cout << "All vectors displayed are of length 1\n";
     for (i=0; i<distinct_count; i++) {
          k = 0; num_vectors = 1;
          while (kind(i,k) != list_end) {
               if (kind(i,k++) == group_end) num_vectors++;}
          cout << "\nEigenvalue No. " << i+1;
          if (eigen_mult(i) == 1) cout << " eigenvector";
          else {
               if (num_vectors == 1) cout << ": 1 eigenvector";
               else {
                    cout << ": " << num_vectors << " near-eigenvectors found";
                    if (first_near_vector) {
                         cout << "\n Note: A near-eigenvector is a vector X such that all components";
                         cout << "\n of (AX - eigenvalue * X) are less in absolute value than 10^-(d+1)"; 
                         cout << "\n where d is the number of decimal places requested, " << d << " on this run.";
                         first_near_vector = FALSE;}}}
          cout << "\n";
          k = 0;
          do {
               while (kind(i,k) == group_member) k++;
               print_Q_col(distinct(i,k));}
          while(kind(i,k++) != list_end);}}
                    
}


