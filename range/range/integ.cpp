#define main_program
#include <fstream.h>
#include "range.h"
#include "stack.h"
#include "interval.h"
#include "series.h"

const unsigned bound = 12; // max deg is 12
series s, u, var[1];
svar f_entry, a_entry, b_entry, variables;
rvar a, b, ab_width, r, r1, dimension_factor(2), delta, percent_factor(200),
     region_error, boundary_error, new_error, new_u_width, old_u_mid,
     integral = zero, integral_error = zero, completed_region = zero,
     percentage = zero;
stack region(4);
interval partial_integral;
int d, depth, cycle_failure;

void increase_precision();
void integration_cycle();
void split_region();

void main()
{
cout << "\n\
Program to find the integral of a function f(x) over an interval [a, b]\n\
      f(x) must be continuous over [a, b] and analytic over (a, b)\n\n";   

if (cin.seekg(0,ios::beg).tellg() != cin.seekg(0,ios::end).tellg()) {
     svar::input_from_file = TRUE;}
else svar::input_from_file = FALSE;
cin.seekg(0,ios::beg); cin.clear();

variables = svar("x");
set_precision(30); list_tokens(a);
entry("Enter f(x)   ", zero, f_entry, variables);
entry("Enter upper limit b   ", b, b_entry);
entry("Enter lower limit a   ", a, a_entry);
ab_width = b - a;

int_entry("\
Enter the number of decimal places desired   ", d, 0, 20, "", "Max is 20\n");

if (svar::input_from_file == FALSE && svar::input_logs_allowed == TRUE) {
     ofstream input_log("integ.log");
     input_log << f_entry << "\n" << b_entry << "\n" << a_entry << "\n";
     input_log << ltos(d) << "\n";
     input_log.close();}

series::initiate(1, 12, &u);
series::evaluate_error_display = FALSE;
series::evaluate_exit_on_error = FALSE;
set_precision(d + 10);
region_error = rvar(".2") * (ten ^ rvar(-(d+1)) ) / dimension_factor;
boundary_error = (region_error / rvar(4)) / rvar(2);
// error allotment for each boundary point
delta = ten ^ rvar(-(d+2)); // for check of function

cout << "Integral now being evaluated.  If more correct decimals found\n\
than requested, these will be shown in parentheses.\n";
cout << "Percentage of interval [a, b] analyzed = 0\r";
cout.flush();

region(0) = half;
// region(0) holds midpoint;
region(1) = half;
// region(1) holds width;
region(2) = region_error;
// region(2) holds subinterval error allotment
region(3) = region_error + rvar(2) * boundary_error;
// region(3) holds total error allotment
region.push();

do {
     region.pop();
     integration_cycle();
     if (cycle_failure) split_region();}
while (region.not_empty);

integral = dimension_factor * integral;
integral_error = dimension_factor * integral_error;
r = integral % integral_error;
for (int i=d+1; i<21; i++) {
     test(r, 1, i); if (test_failure) break;}
i--;
cout << "\nIntegral = " << rtos(r, 1, d);
if (i > d) cout <<  "  ( = " << rtos(r, 1, i) << ")\n";
else cout << "\n";
}          

void increase_precision()
{
add_precision(4);
evaluate(a, a_entry);
evaluate(b, b_entry);
ab_width = b - a;
}

void integration_cycle()
{
// Subregion is interval determined by u_mid = region(0), u_wid = region(1).
cycle_failure = FALSE;
do {
     u.const_term(region(0), region(1));
     var[0] = a + ab_width * u;
     series::set_max_deg(bound);
     evaluate(s, f_entry, var);
     if (series::evaluate_error) {
          if (region(3) == region(2)) { // not boundary subinterval
               if (var[0](0,0,0).wid >= delta) {
                    cycle_failure = TRUE; return;}}
          series::set_max_deg(0); // boundary or tiny interior subinterval
          evaluate(s, f_entry, var);
          if (series::evaluate_error) {
               if (var[0](0,0,0).wid >= delta) {
                    cycle_failure = TRUE; return;}
               cout <<"\nUnable to verify that f(x) is defined\n";
               if (region(3) == region(2)) {
                    cout << "inside the integration region at x = ";}
               else cout << "on the integration region boundary at x = ";
               r = var[0](0,0,0).mid % var[0](0,0,0).wid;
               cout << rtos(r,1,d) << "\n";
               cout << "Integration terminated\n"; exit(1);}}
     partial_integral = ab_width * integrate(s, region(1));
     r = partial_integral.wid; r1 = partial_integral.mid;
         if (r + range(r1) < region(3) ) break; // accuracy test passed
     else if (range(r) > mid(r) || range(r1) > region(2) ) {
          increase_precision(); continue;}
     else {
          cycle_failure = TRUE; return;}}
while (1);
integral += mid(r1);
integral_error += partial_integral.wid + range(r1);
rvar::exact_arithmetic = TRUE;
completed_region = completed_region + region(1);
r = trunc(percent_factor * completed_region);
if (r > percentage) {
     percentage = r;
     cout << "Percentage of interval [a, b] analyzed = ";
     cout << rtos(percentage, 0) << "\r"; cout.flush();}
rvar::exact_arithmetic = FALSE;
}

void split_region()
{
rvar::exact_arithmetic = TRUE;
old_u_mid = region(0); new_u_width = div2(region(1));
new_error = div2(region(2));
region(0) = old_u_mid + new_u_width;
region(1) = new_u_width;
region(2) = region(3) = new_error;
if (region(0) == region(1) || region(0) == one - region(1)) {
     region(3) += boundary_error;}
region.push();
region(0) = old_u_mid - new_u_width;
region(1) = new_u_width;
region(2) = region(3) = new_error;
if (region(0) == region(1) || region(0) == one - region(1)) {
     region(3) += boundary_error;}
region.push();
rvar::exact_arithmetic = FALSE;
}
