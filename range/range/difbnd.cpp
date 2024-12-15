#define main_program
#include <fstream.h>
#include "range.h"
#include "vector.h"
#include "interval.h"
#include "i_vector.h"
#include "series1.h"

const int N = 20; // N is max number of linear dif. equats
int d, R, num_times_same_h_used, print_num, ident[N], b_ref,
     printed_values_wanted = FALSE;
rvar a, b, x0, xi, xf, next_x_print_point, r, r1, r2,
     current_x_expansion_point, next_x_expansion_point,
     h, h_cutoff, last_h, expansion_factor("1.5"), local_error_rate;
svar a_entry, b_entry, x0_entry, xi_entry, xf_entry, cutoff_message;
vector u1, Ua, Ub, Ub0, C;
vector local_error, global_error, error1, error2, error3, trial_width;
Ivector VI; Imatrix MI, PI;
matrix M1, M2, A, B, U;
svector symbol, value, bc_entry, f_entry; 
series u[N], f[N], x, s, s1;
interval I;

void boundary_solution();
void compute_error();
void determine_x_step_h();
void extract_linear_conditions();
void initialize_series_and_arrays();
void insert_initial_conditions();
void insert_new_x_and_u_values();
int  linear_bnd_cond(svar&);
int  linear_in_u(svar&);
void print_x_and_u(rvar&, vector&);
void print_data_points();
vector solution_at_b();

void main()
{
int i, j; rvar temp; svar variables, var1, var2; char *p;
cout << "\n\
Program to solve a linear two point boundary value problem for the functions\n\
u1(x), u2(x), . . . , uR(x) satisfying a system of R linear first order\n\
differential equations.\n\n\
Two supplied values, a and b, determine the x boundary points, and\n\
the boundary conditions are R linear functions of u1(a), . . . , uR(a), and\n\
u1(b), . . . , uR(b), which are simultaneously zero.\n\
A solution for u1, . . . , uR is displayed at designated x arguments.\n\n";

if (cin.seekg(0,ios::beg).tellg() != cin.seekg(0,ios::end).tellg()) {
     svar::input_from_file = TRUE;}
else svar::input_from_file = FALSE;
cin.seekg(0,ios::beg); cin.clear();

set_precision(30);
var1 = svar("Max R is ") + ltos(N) + svar("\n");
int_entry("Enter the number of equations R    ",
R, 2, N, "Min R is 2\n", !var1);
symbol = svector(R+1); value = svector(R+1);
bc_entry = svector(R); f_entry = svector(R);
if (R > 9) symbol(R) = svar("  x"); else symbol(R) = svar(" x");
for (i=0; i<R; i++) {
     if (R > 9 && i < 9) symbol(i) = svar(" u"); else symbol(i) = svar("u");
     symbol(i) += ltos(i+1);}
var1 = svar("u"); variables = svar("u1");
for (i=1; i<R; i++) {
     variables += svar(",") + var1 + ltos(i+1);}
variables += svar(",x");
list_tokens(temp);
cout << "\n\
The R linear differential equations are defined by entering functions of the\n\
form g1(x) * u1 + . . . + gR(x) * uR + h(x) ";
cout << "defining the various u derivatives.\n";
var1 = svar("Enter u");
for (i=0; i<R; i++) {
     var2 = var1 + ltos(i+1) + svar("\'");
     if (R > 9 && i < 9) var2 += svar("   "); else var2 += svar("  ");
     while (1) {
          entry(!var2, temp, f_entry(i), variables);
          if (linear_in_u(f_entry(i))) break;}}
cout << "\n";
while(1) {
     entry("Enter a   ", a, a_entry);
     entry("Enter b   ", b, b_entry);
     if (a < b) break;
     else cout << "a must be less than b; re-enter a and b.\n";}
var1 = svar("u1a");
var2 = svar("u1b");
for (i=1; i<R; i++) {
     var1 += svar(",u") + ltos(i+1) + svar("a");
     var2 += svar(",u") + ltos(i+1) + svar("b");}
variables = var1 + svar(",") + var2;
cout << "\n\
Writing u1a and u1b for u1(a) and u1(b), etc.,\n\
each boundary condition is entered as a linear function of \n\
u1a, u2a, . . . , uRa, and u1b, u2b, . . . , uRb, \n\
that is required to equal zero.  Example: u1a + 2*u2b - 3\n";
var1 = svar("Enter boundary condition ");
for (i=0; i<R; i++) {
     var2 = var1 + ltos(i+1);
     if (R > 9 && i < 9) var2 += svar("   "); else var2 += svar("  ");
     while (1) {
          entry(!var2, temp, bc_entry(i), variables);
          if (linear_bnd_cond(bc_entry(i))) break;}}

cout << "\n\
A solution u1(x), u2(x), . . . , uR(x) is displayed at the x values\n\
x0, x0 + xi, x0 + 2*xi, x0 + 3*xi, . . . ,  until a point xf is passed.\n\
The starting value x0 and the ending value xf are required,\n\
and also the increment xi if xf is greater than x0.\n";

while (1) {
     entry("Enter the starting value x0   ", x0, x0_entry);
     if (x0 >= a) break;
     else cout << "starting value x0 can not be less than point a\n";}
while (1) {
     entry("Enter the ending value   xf   ", xf, xf_entry);
     if (xf >= x0) break;
     else cout << "final value xf can not be less than x0\n";}
if (x0 < xf) {
     while (1) {
          entry("Enter the x increment    xi   ", xi, xi_entry);
          if (xi > zero) break;
          else cout << "increment xi must be positive\n";}}
cout << "\n";
int_entry("\
Enter the number of decimal places desired for the displayed values  ", 
      d, 0, 20, "", "20 is max.\n");

if (svar::input_from_file == FALSE && svar::input_logs_allowed == TRUE) {
     ofstream input_log("difbnd.log");
     input_log << ltos(R) << "\n";
     for (i=0; i<R; i++) input_log << f_entry(i) << "\n";
     input_log << a_entry << "\n" << b_entry << "\n";
     for (i=0; i<R; i++) input_log << bc_entry(i) << "\n";
     input_log << x0_entry << "\n" << xf_entry << "\n";
     if (xi_entry != empty) input_log << xi_entry << "\n";
     input_log << ltos(d) << "\n";
     input_log.close();}
cout << "\n"; set_precision(d + 15 + R/2);
initialize_series_and_arrays();
extract_linear_conditions();
// above function checks to determine any ui(a) values fixed by boundary conds.
// b_ref equals the number of these ui(a) values.
if (b_ref < R) {
     for (i=0; i<b_ref; i++) Ua(i) = C(i);
     for (i=b_ref; i<R; i++) Ua(i) = zero;
     cout << "Now computing " << R-b_ref+1 << " different solutions ";
     cout << "to the differential equations\n";
     cout << "with various initial conditions at x = a.\n";
     cout << "The value of these solutions at x = b solve ";
     cout << "the boundary problem.\n";
     cout << "No. of solutions completed = 0\r"; cout.flush();
     cout.flush();
     for (i=0; i<b_ref; i++) Ua(i) = C(i);
     Ub0 = solution_at_b();
     for (i=b_ref; i<R; i++) {
          cout << "No. of solutions completed = " << i-b_ref+1 <<"\r";
          cout.flush();
          Ua(i) = one; if (i > b_ref) Ua(i-1) = zero;
          Ub = solution_at_b();
          Ub = Ub - Ub0;
          for (j=0; j<R; j++) U(j,i) = Ub(j);}
     cout << "No. of solutions completed = " << R-b_ref+1 <<"\n"; cout.flush();
     C = C - B * Ub0; A = A + B * U;
     C = C / A;
     if (matrix::failure) {
          cout << "Unable to determine solution from the boundary conditions\n";
          cout << "It is possible the boundary problem is indeterminate\n";
          cout << "End of program\n";
          exit(1);}}
for (i=0; i<R; i++) Ua(i) = C(i);
cout << "\nSolution of the boundary value problem\n\n"; cout.flush();
boundary_solution();
}

void boundary_solution()
{
next_x_print_point = x0;
print_num = 0;
r = xf - a; if (r == zero) r = one;
local_error_rate = rvar(".2") * (ten ^ rvar(-(d+1))) / r;
h_cutoff = xi / rvar(1000);
cutoff_message = svar("\
Integration step width has dropped to less than one thousandth of xi.\n");
insert_initial_conditions();
while (1) { 
     determine_x_step_h();
     print_data_points(); // exits when xf passed
     insert_new_x_and_u_values();
     compute_error();}
}

void compute_error()
{
int i, j, k;
for (i=0; i<R; i++) {
     for (j=0; j<R; j++) {
          I = I_zero;
          for (k=0; k<R; k++) I = I + M1(k,j) * MI(i,k);
          I.wid += range(I.mid);
          I.mid =  mid(I.mid);
          PI(i,j) = I;
          M2(i,j) = I.mid;}}
M1 = M2; M2 = inverse(M1); // failure of inverse sets matrix::failure TRUE
if (! matrix::failure) {
     for (i=0; i<R; i++) { // method 1: keep M1
          for (j=0; j<R; j++) {
               I = I_zero; 
               for (k=0; k<R; k++) I = I + M2(i,k) * PI(k,j);
               MI(i,j) = I;}}
     for (i=0; i<R; i++) {
          r = zero;
          for (k=0; k<R; k++) {
               r += (abs(MI(i,k).mid) + MI(i,k).wid) * error1(k);}
          error2(i) = r;}
     for (i=0; i<R; i++) {
          r = zero;
          for (k=0; k<R; k++) r += abs(M2(i,k)) * local_error(k);
          error3(i) = r + error2(i);} // error3 will replace error1
     r = zero;
     for (i=0; i<R; i++) for (j=0; j<R; j++) r = max(r, abs(M2(i,j)));
     if (r >= ten) { // compare two methods of computing errors
          r1 = zero; r2 = zero;
          for (i=0; i<R; i++) { // method 2: replace M1 by I
               r = zero;
               for (k=0; k<R; k++) {
                    r += (abs(PI(i,k).mid) + PI(i,k).wid) * error1(k);}
               error2(i) = r + local_error(i);
               r2 = max(r2, error2(i));}
          for (i=0; i<R; i++) {// method 1 check
               for (k=0; k<R; k++) r1 = max(r1, error3(i));}
          if (r1 < two * r2) { // method 1
               for (i=0; i<R; i++) error1(i) = error3(i);}
          else {  // method 2
               for (i=0; i<R; i++) for (j=0; j<R; j++) M1(i,j) = zero;
               for (i=0; i<R; i++) {
                    error1(i) = error2(i);
                    M1(i,i) = one;}}}
     else { // method 1
          for (i=0; i<R; i++) error1(i) = error3(i);}}
else {
     for (i=0; i<R; i++) { // method 2: replace M1 by I
          r = zero;
          for (k=0; k<R; k++) {
               r += (abs(PI(i,k).mid) + PI(i,k).wid) * error1(k);
               M1(i,k) = zero;}
          error2(i) = r + local_error(i);}
     for (i=0; i<R; i++) {
          error1(i) = error2(i);
          M1(i,i) = one;}}

for (i=0; i<R; i++) { // purify error1
     r = error1(i);
     r = mid(r) + range(r);
     error1(i) = r;}
for (i=0; i<R; i++) {
     r = zero;
     for (k=0; k<R; k++) r += abs(M1(i,k)) * error1(k);
     global_error(i) = r;}
}

void determine_x_step_h()
{
if (++num_times_same_h_used >= 5) {
     num_times_same_h_used = 0;
     if (last_h < half) last_h = two * last_h;}
h = last_h;
int i, j, k, k1, width_sufficient;
while (1) {
     x(0).wid = h;
     for (i=0; i<R; i++) trial_width(i) = two * global_error(i);
     for (j=0; j<=R; j++) {
          width_sufficient = TRUE;
          for (i=0; i<R; i++) u[i](0).wid = trial_width(i);
          series_computation(0);
          if (series::error) break;
          for (i=0; i<R; i++) { 
               I = f[i](0);
               r = h * (abs(I.mid) + I.wid) + global_error(i);
               if (r >= trial_width(i)) {
                    r *= expansion_factor;
                    trial_width(i) = mid(r) + range(r);
                    width_sufficient = FALSE;}}
          if (width_sufficient) {
               for (i=0; i<R; i++) u[i](1) = f[i](0);
               for (k=1; k<max_x_deg; k++) {
                    series_computation(k);
                    k1 = k + 1; r = integ[k1]; 
                    for (i=0; i<R; i++) u[i](k1) = f[i](k) / r;}
               r = h ^ integ[max_x_deg1];
               for (i=0; i<R; i++) {
                    if (r * u[i](max_x_deg).wid >= local_error_rate) break;}
               if (i < R) break;
               // repeat the series expansion to get u partial terms
               // for computing error
               x(0).wid = zero;
               for (i=0; i<R; i++) u[i](0).wid = global_error(i);
               for (k=0; k<max_x_deg1; k++) {
                    series_computation1(k);
                    k1 = k + 1; r = integ[k1];
                    for (i=0; i<R; i++) {
                         s = u[i]; s1 = f[i];
                         for (j=-1; j<R; j++) s(k1,j) = s1(k,j) / r;}}
               r = x(0).mid;
               current_x_expansion_point = r;
               next_x_expansion_point = r + h;
               return;}}
     h = div2(h);
     if (h < h_cutoff) {
          cout << cutoff_message;
          cout << "End of computation attempt.\n";
          exit(1);}
     last_h = h;
     num_times_same_h_used = 0;}
}

void extract_linear_conditions()
{
// Converts the entered linear boundary conditions into a matrix array in D.
// Each row of D has the constants for a different boundary condition
// Col i=0 to R-1 has the constant multipliers for the various uia variables
// Col i=R to 2R-1 has the constant multipliers for the various uib variables
// Col i=2R has any simple constant
matrix D(R,2*R+1);
rvar r[evaluation_stack_size];
vector v[evaluation_stack_size];
int type[evaluation_stack_size], i, i1, j, k, last_col = R+R, RR = R+R+1;
char c, *p, *q, *q1;
for (i=0; i<R; i++) {
     p = bc_entry(i).tokens(); q = !bc_entry(i);
     j = -1;
     do {
          c = *p;
          switch (c) {
               case end_: if (j != 0 ) error("extract1");
               if (type[j] == const_) D(i,last_col) = r[j];
               else for (k=0; k<RR; k++) D(i,k) = v[j](k);
               break;

               case const_: q1 = q + ((*(p+1) << 7) + *(p+2));
               r[++j] = rvar(q1); type[j] = const_; break;
          
               case pi_const_: r[++j] = pi; type[j] = const_; break;

               case var_: v[++j] = vector(RR);
               k = *(p+2);
               v[j](k) = one;
               type[j] = var_; break;

               case unary_minus_:
               if (type[j] == const_) r[j] = - r[i];
               else for (k=0; k<RR; k++) v[j](k) = - v[j](k);
               break;

               case abs_ : r[j] = abs(r[j]);  break;
               case acos_: r[j] = acos(r[j]); break;
               case asin_: r[j] = asin(r[j]); break;
               case atan_: r[j] = atan(r[j]); break;
               case cos_ : r[j] = cos(r[j]);  break;
               case sin_ : r[j] = sin(r[j]);  break;
               case tan_ : r[j] = tan(r[j]);  break;
               case cosh_: r[j] = cosh(r[j]); break;
               case sinh_: r[j] = sinh(r[j]); break;
               case tanh_: r[j] = tanh(r[j]); break;
               case exp_ : r[j] = exp(r[j]);  break;
               case log_ : r[j] = log(r[j]);  break;
               case sqrt_: r[j] = sqrt(r[j]); break;

               case '+':
               if (type[j-1] == const_) {
                    if (type[j] == const_) r[j-1] += r[j];
                    else {
                         v[j](last_col) += r[j-1];
                         v[j-1] = v[j]; type[j-1] = var_;}}
               else if (type[j] == const_) v[j-1](last_col) += r[j];
               else for (k=0; k<RR; k++) v[j-1](k) += v[j](k);
               j--; break;

               case '-':
               if (type[j-1] == const_) {
                    if (type[j] == const_) r[j-1] -= r[j];
                    else {
                         v[j](last_col) -= r[j-1];
                         v[j-1] = v[j]; type[j-1] = var_;}}
               else if (type[j] == const_) v[j-1](last_col) -= r[j];
               else for (k=0; k<RR; k++) v[j-1](k) -= v[j](k);
               j--; break;

               case '*':
               if (type[j-1] == const_) {
                    if (type[j] == const_) r[j-1] *= r[j];
                    else {
                         for (k=0; k<RR; k++) v[j](k) *= r[j-1];
                         v[j-1] = v[j]; type[j-1] = var_;}}
               else for (k=0; k<RR; k++) v[j-1](k) *= r[j];
               j--; break;

               case '/':
               if (type[j-1] == const_) r[j-1] /= r[j];
               else for (k=0; k<RR; k++) v[j-1](k) /= r[j];
               j--; break;

               case '^': r[j-1] = r[j-1] ^ r[j];
               j--; break;

               case power_odd_p_odd_q: r[j-1] = expon_o(r[j-1], r[j]);
               j--; break;
     
               case power_even_p_odd_q: r[j-1] = expon_e(r[j-1], r[j]);
               j--; break;

               default: error("extract2");}
          p += 3;}
     while (c != end_);}
// Extract the information in array D to matrices A and B, and to vector C
// A has uia coefficients, B has uib coefficients, and C has constants
A = matrix(R,R); B = matrix(R,R); C = vector(R);
for (i=0; i<R; i++) {
     C(i) = -D(i,last_col);
     for (j=0; j<R; j++) {
          A(i,j) = D(i,j); B(i,j) = D(i,j+R);}}
// Next deterimine the rows of B which are all zero
// and rearrange A, B, C so these rows are at top of arrays
b_ref = 0;
for (i=0; i<R; i++) {
     for (j=0; j<R; j++) if (!is_zero(B(i,j))) break;
     if (j == R) {
          if (i != b_ref) {
               for (k=0; k<R; k++) {
                    swap (A(b_ref,k), A(i,k));
                    swap (B(b_ref,k), B(i,k));}
               swap (C(b_ref), C(i));}
          b_ref++;}}
// Bring rows of A with no uib reference to reduced echelon form,
// and switch columns if necessary, so that the leading 1's form
// an identity submatrix to the left of these rows.
// Keep track of column switches with ident array.
for (k=0; k<R; k++) ident[k] = k;
rvar divisor, multiplier;
for (i=0; i<b_ref; i++) {
     j = i;
     while (A(i,j) == zero && ++j < R) {;}
     if (j == R) {
          cout << "Difficulty with the following boundary condition(s) \n";
          for (k=0; k<b_ref; k++) cout << k+1 << ": " << bc_entry(k) << "\n";
          cout << "Rank of matrix of ui(a) coefficients apparently less than ";
          cout << b_ref << "\n"; exit(1);}
     if (j > i) {
          for (k=0; k<R; k++) {
               swap(A(k,i), A(k,j)); swap(B(k,i), B(k,j));}
          k = ident[i]; ident[i] = ident[j]; ident[j] = k;}
     divisor = A(i,i); A(i,i) = one; C(i) /= divisor;
     for (j=i+1; j<R; j++) A(i,j) /= divisor;
     for (j=0; j<b_ref; j++) {
          if (j == i) continue;
          multiplier = A(j,i); C(j) -= multiplier * C(i);
          for (k=0; k<R; k++) A(j,k) -= multiplier * A(i,k);
          A(j,i) = zero;}}
for (i=b_ref-1; i>=0; i--) {
     for (j=b_ref; j<R; j++) if (!is_zero(A(i,j))) break;
     if (j != R) {
     // need to eliminate row i from group, but still want an I submatrix
          if (i == b_ref-1) b_ref--;
          else { // make a row and col exchange of i, i+1 and jump to row i+1
               i1 = i+1;
               swap(C(i),C(i1));
               for (k=0; k<R; k++) swap(A(i,k), A(i1,k));
               for (k=0; k<R; k++) {
                    swap(A(k,i), A(k,i1)); swap(B(k,i), B(k,i1));}
               k = ident[i]; ident[i] = ident[i1]; ident[i1] = k;
               i += 2;}}}
// ident[k] from k=b_ref to k=R-1 identifies the ui(a)'s which are
// determined by solving a set of initial value problems.
// ident[k] from k=0 to k=b_ref-1 identifies the ui(a)'s which are
// fixed in advance.
}
 
void initialize_series_and_arrays()
{
int i;
prepare_series(f_entry, u, f, &x);
compute_series_constants();
error1 = vector(R); error2 = vector(R); error3 = vector(R);
trial_width = vector(R); C = vector(R); Ua = vector(R); u1 = vector(R);
global_error = vector(R); local_error = vector(R); 
VI = Ivector(R); MI = Imatrix(R,R); PI = Imatrix(R,R);
M1 = matrix(R,R); M2 = matrix(R,R); A = matrix(R,R); B = matrix(R,R);
U = matrix(R,R);
matrix::failure_report_wanted = TRUE;
}

void insert_initial_conditions()
{
int i, j;
for (i=0; i<R; i++) {
     I.mid = Ua(i); I.wid = zero;
     u[ident[i]](0) = I;}
x(0).mid = a; x(0).wid = zero;
series_computation(0);
if (series::error) {
     cout << "Error: function defining u" << series::function_count + 1;
     cout << "\' is not analytic at x = a\n";
     exit(1);}
r = rvar("1e-50");
for (i=0; i<R; i++) {
     error1(i) = r; // nonzero values needed
     global_error(i) = r;
     for (j=0; j<R; j++) M1(i,j) = zero;
     M1(i,i) = one;}
last_h = half;
num_times_same_h_used = 0;
}

void insert_new_x_and_u_values()
{
int i, j, k;
r1 = h ^ integ[max_x_deg];
for (i=0; i<R; i++) {
     s = u[i];
     r = s(max_x_deg).mid * h + s(max_x_deg1).mid;
     for (j=0; j<R; j++) VI(j) = s(max_x_deg1,j);
     for (k=max_x_deg2; k>=0; k--) {
          r = h * r + s(k).mid;
          for (j=0; j<R; j++) VI(j) = h * VI(j) + s(k,j);}
     local_error(i) = range(r) + s(max_x_deg).wid * r1;
     s(0).mid = mid(r);
     for (j=0; j<R; j++) MI(i,j) = VI(j);}
x(0).mid = next_x_expansion_point;
}

int linear_bnd_cond(svar& bc_entry)
{
// Checks that bc_entry function is a sum of constants and 
// constants times u variables.
// Uses const_ to represent a constant and var_ to represent a u variable.
// Returns TRUE or FALSE according as check is or is not passed.
// If want_display == TRUE, displays point of non-linearity if check not passed
char *p, c;
int type[evaluation_stack_size], i, j = -1, j1, k, non_linearity_found = FALSE;
p = bc_entry.tokens();
while (1) {
     c = *p; k = (*(p+1) << 7) + *(p+2);
     switch (c) {
          case end_: if (j != 0) error("program3");
          return TRUE;

          case const_: case pi_const_:
          type[++j] = const_; break;

          case var_: type[++j] = c; break;

          case unary_minus_: break;

          case abs_:  case acos_: case asin_: case atan_: case cos_:
          case cosh_: case exp_:  case log_:  case sin_:  case sinh_:
          case sqrt_: case tan_:  case tanh_:
          if (type[j] == var_) non_linearity_found = TRUE;
          break;

          case '+': case '-': j1 = j--;
          if (type[j1] == var_ || type[j] == var_) type[j] = var_;
          break;

          case '*': j1 = j--; 
          if (type[j] == var_ && type[j1] == var_) non_linearity_found = TRUE;
          if (type[j1] == var_ || type[j] == var_) type[j] = var_;
          break;

          case '/': j1 = j--;
          if (type[j1] == var_) non_linearity_found = TRUE;
          break;

          case '^': case power_even_p_odd_q: case power_odd_p_odd_q:
          j1 = j--;
          if (type[j1] == var_ || type[j] == var_) non_linearity_found = TRUE;
          break;
               
          default: error("program4");}
     if (non_linearity_found) {
          cout << "Entry not in required linear form\n";
          show_error_pos(bc_entry, k);
          return FALSE;}
     p += 3;}
}

int linear_in_u(svar& f_entry)
{
// Checks that f_entry function is a sum of x functions times u variables.
// Uses const_ to represent a constant or an x function and var_ to represent
// a u variable.
// Returns TRUE or FALSE according as check is or is not passed.
// If want_display == TRUE, displays point of non-linearity if check not passed
char *p, c;
int type[evaluation_stack_size], i, j = -1, j1, k, non_linearity_found = FALSE;
p = f_entry.tokens();
while (1) {
     c = *p; k = (*(p+1) << 7) + *(p+2);
     switch (c) {
          case end_: if (j != 0) error("program1");
          return TRUE;

          case const_: case pi_const_:
          type[++j] = const_; break;

          case var_: if (*(p+2) == R) c = const_;
          type[++j] = c; break;

          case unary_minus_: break;

          case abs_: case acos_: case asin_: case atan_: case cos_:
          case cosh_: case exp_: case log_: case sin_: case sinh_:
          case sqrt_: case tan_: case tanh_:
          if (type[j] == var_) non_linearity_found = TRUE;
          break;

          case '+': case '-': j1 = j--;
          if (type[j1] == var_ || type[j] == var_) type[j] = var_;
          break;

          case '*': j1 = j--; 
          if (type[j] == var_ && type[j1] == var_) non_linearity_found = TRUE;
          if (type[j1] == var_ || type[j] == var_) type[j] = var_;
          break;

          case '/': j1 = j--;
          if (type[j1] == var_) non_linearity_found = TRUE;
          break;

          case '^': case power_even_p_odd_q: case power_odd_p_odd_q:
          j1 = j--;
          if (type[j1] == var_ || type[j] == var_) non_linearity_found = TRUE;
          break;
               
          default: error("program2");}
     if (non_linearity_found) {
          cout << "Entry not in required linear form\n";
          show_error_pos(f_entry, k);
          return FALSE;}
     p += 3;}
}

void print_data_points()
{
int i, k;
while (next_x_expansion_point > next_x_print_point) {
     r = next_x_print_point - current_x_expansion_point;
     for (i=0; i<R; i++) {
          s = u[i];
          I = s(max_x_deg);
          for (k=max_x_deg1; k>=0; k--) I = r * I + s(k);
          u1(i) = I.mid % I.wid;
          test(u1(i), 1, d);}
     if (test_failure > d) {
          cout << "Error now affecting digits before the decimal point.\n";
          cout << "End of computation.\n";
          exit(1);}
     test_failure = 0;
     print_x_and_u(next_x_print_point, u1);
     next_x_print_point = x0 + rvar(++print_num) * xi;
     if (next_x_print_point > xf || xi == zero) exit(1);}
}

void print_x_and_u(rvar& x_value, vector& u_value)
{
int i, max, t;
value(R) = rtos(x_value, 1, d);
max = value(R).len();
for (i=0; i<R; i++) {
     value(i) = rtos(u_value(i), 1, d);
     t = value(i).len();
     if (t > max) max = t;}
cout << symbol(R) << " = ";
cout.width(max); cout << value(R) << "\n";
for (i=0; i<R; i++) {
     cout << symbol(i) << " = ";
     cout.width(max); cout << value(i) << "\n";}
cout << "\n"; cout.flush();
}

vector solution_at_b()
{
int i, j, k;
local_error_rate = rvar(".2") * (ten ^ rvar(-(d+4))) / (b - a);
h_cutoff = (b - a) / rvar(10000);
cutoff_message = svar("\
Integration step width has dropped to less than one ten thousandth of b-a.\n");
insert_initial_conditions();
while (1) {
     determine_x_step_h();
     if (next_x_expansion_point > b) break;
     insert_new_x_and_u_values();
     compute_error();}
vector U_at_b(R);
h = b - current_x_expansion_point;
r1 = h ^ integ[max_x_deg];
for (i=0; i<R; i++) {
     s = u[ident[i]];
     r = s(max_x_deg).mid * h + s(max_x_deg1).mid;
     for (j=0; j<R; j++) VI(j) = s(max_x_deg1,j);
     for (k=max_x_deg2; k>=0; k--) {
          r = h * r + s(k).mid;
          for (j=0; j<R; j++) VI(j) = h * VI(j) + s(k,j);}
     for (j=0; j<R; j++) VI(j).mid = abs(VI(j).mid) + VI(j).wid;
     r2 = s(max_x_deg).wid * r1;
     for (j=0; j<R; j++) r2 += global_error(j) * VI(j).mid;
     U_at_b(i) = r % r2;}
return U_at_b;
}


