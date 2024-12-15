#define main_program
#include <fstream.h>
#include "range.h"
#include "vector.h"
#include "interval.h"
#include "i_vector.h"
#include "series1.h"

const int N = 30; // N is max number of dif. equats
int d, R, num_times_same_h_used, print_num = 1;
rvar x0, xi, xf, next_x_print_point, r, r1, r2,
     current_x_expansion_point, next_x_expansion_point,
     h, h_cutoff, last_h, expansion_factor("1.5"), local_error_rate;
svar x0_entry, xi_entry, xf_entry;
vector u0, u1, local_error, global_error, error1, error2, error3, trial_width;
Ivector VI; Imatrix MI, PI;
matrix M1, M2;
svector symbol, value, f_entry, u0_entry; 
series u[N], f[N], x, s, s1;
interval I;

void compute_error();
void determine_x_step_h();
void insert_initial_conditions();
void insert_new_x_and_u_values();
void print_data_points();
void print_x_and_u(rvar&, vector&);

void main()
{
int i, k, k1; rvar temp; svar variables, var1, var2; char *p;
cout << "\
Program to solve a system of first order differential equations\n\
               u1' = f1(x, u1, u2, . . , uR)\n\
               u2' = f2(x, u1, u2, . . , uR)\n\
                .     .\n\
                .     .\n\
               uR' = fR(x, u1, u2, . . , uR)\n\
for the functions u1, u2, . . , uR over an interval [x0, x1].\n\
The initial value of u1, u2, . . , uR at x0 must be given and the\n\
functions f1, . . , fR must be analytic in a region of the argument\n\
space (x, u1, u2, . . , uR) containing the solutions.\n\n";

if (cin.seekg(0,ios::beg).tellg() != cin.seekg(0,ios::end).tellg()) {
     svar::input_from_file = TRUE;}
else svar::input_from_file = FALSE;
cin.seekg(0,ios::beg); cin.clear();

set_precision(30);
var1 = svar("Max R is ") + ltos(N) + svar("\n");
int_entry("Enter the number of equations R    ", R, 1, N, "", !var1);
symbol = svector(R+1); value = svector(R+1);
f_entry = svector(R); u0 = vector(R); u0_entry = svector(R); 
if (R < 10) symbol(R) = svar(" x"); else symbol(R) = svar("  x");
for (i=0; i<R; i++) {
     if (R > 9 && i < 9) symbol(i) = svar(" u"); else symbol(i) = svar("u");
     symbol(i) += ltos(i+1);}
var1 = svar("u"); variables = svar("u1");
for (i=1; i<R; i++) {
     variables += svar(",") + var1 + ltos(i+1);}
variables += svar(",x");
cout << "\n"; list_tokens(temp);
for (i=0; i<R; i++) {
     var1 = svar("Enter the function f") + ltos(i+1);
     if (i < 9) var1 += svar("   "); else var1 += svar("  ");
     entry(!var1, zero, f_entry(i), variables);}
var1 = svar("Enter    ") + symbol(R) + svar("0  "); // Enter x0
entry(!var1, x0, x0_entry);
for (i=0; i<R; i++) {
     var1 = svar("Enter ") + symbol(i) + svar("(x0)  ");
     entry(!var1, u0(i), u0_entry(i));}

while (1) {
     entry("Enter the x increment desired for solution printout  ",
     xi, xi_entry);
     if (xi > zero) break;
     else cout << "x increment must be positive\n";}
while (1) {
     entry("Enter the final x value   ", xf, xf_entry);
     if (xf > x0) break;
     else cout << "final value must be greater than x0\n";}

int_entry("Enter the number of decimal places desired  ", 
      d, 0, 20, "", "20 is max. ");

if (svar::input_from_file == FALSE && svar::input_logs_allowed == TRUE) {
     ofstream input_log("difsys.log");
     input_log << ltos(R) << "\n";
     for (i=0; i<R; i++) input_log << f_entry(i) << "\n";
     input_log << x0_entry << "\n";
     for (i=0; i<R; i++) input_log << u0_entry(i) << "\n";
     input_log << xi_entry << "\n";
     input_log << xf_entry << "\n";
     input_log << ltos(d) << "\n";
     input_log.close();}

set_precision(d + R/2 + 15);
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
                    trial_width(i) = mid(r);
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
          cout << "Integration step width is less than one thousandth ";
          cout << "of print step width.\nEnd of computation.\n";
          exit(1);}
     last_h = h;
     num_times_same_h_used = 0;}
}

void insert_initial_conditions()
{
int i;
prepare_series(f_entry, u, f, &x);
while (1) {
     test(x0, 1, d); x(0).mid = x0;
     for (i=0; i<R; i++) {
          test(u0(i), 1, d);
          u[i](0).mid = u0(i);}
     if (test_failure) add_precision(); else break;
     evaluate(x0, x0_entry);
     for (i=0; i<R; i++) evaluate(u0(i), u0_entry(i));
     evaluate(xi, xi_entry);
     evaluate(xf, xf_entry);}
cout << "\n";
print_x_and_u(x0, u0);
next_x_print_point = x0 + xi;
compute_series_constants();
series_computation(0);
if (series::error) {
     cout << "Error: function f" << series::function_count + 1;
     cout << " is not analytic at x0\n";
     exit(1);}
local_error_rate = rvar(".2") * (ten ^ rvar(-(d+1))) / (xf - x0);
error1 = vector(R); error2 = vector(R); error3 = vector(R);
trial_width = vector(R);
global_error = vector(R); local_error = vector(R); u1 = vector(R);
VI = Ivector(R); MI = Imatrix(R,R); PI = Imatrix(R,R);
M1 = matrix(R,R); M2 = matrix(R,R);
matrix::failure_report_wanted = TRUE;
for (i=0; i<R; i++) M1(i,i) = one;
last_h = half;
num_times_same_h_used = 0;
h_cutoff = xi / rvar(1000);
r = ten ^ rvar(-50);
for (i=0; i<R; i++) {
     error1(i) = r; global_error(i) = r;} // nonzero values needed
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
     if (next_x_print_point > xf) exit(1);}
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


