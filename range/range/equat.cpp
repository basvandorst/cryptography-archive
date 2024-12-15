#define main_program
#include <fstream.h>
#include "range.h"
#include "vector.h"

void main()
{
cout << "          Program to solve a set of linear equations \n";
cout << "                    The matrix equation is AX = B\n\n";

if (cin.seekg(0,ios::beg).tellg() != cin.seekg(0,ios::end).tellg()) {
     svar::input_from_file = TRUE;}
else svar::input_from_file = FALSE;
cin.seekg(0,ios::beg); cin.clear();

int d1, d2, d2a, i, j, n;
int_entry("Enter the number of equations (1 to 20)   ", n, 1, 20);

vector B(n), C; matrix A(n,n);
svector B1; smatrix A1;
rvar temp;

// various message svars created below
svar mess1("\
Coefficient matrix may be singular.\n\
Computation halted after determining that abs(det(A)) < 10^-n\n\
where n = ");
svar mess2(", the number of decimal places requested.\n");
svar mess3("\
Enter a higher n integer to control computation,\n\
or type q and hit <RETURN> to quit ");

int_entry("\
Enter the solution print format: 1 for fixed decimal point\n\
                                 2 for floating decimal point    ", d1, 1, 2);
int_entry("Enter the number of decimal places desired   ", d2, 0, 50, "","\
50 decimal places is the maximum\n");
d2a = d2;
set_precision(d2 + n + 5);
matrix::failure_report_wanted = TRUE;

list_tokens(temp);
entry("enter a", A, A1); entry("enter b", B, B1);

if (svar::input_from_file == FALSE && svar::input_logs_allowed == TRUE) {
     ofstream input_log("equat.log");
     input_log << ltos(n) << "\n" << ltos(d1) << "\n" << ltos(d2) << "\n";
     for (i=0; i<n; i++) for (j=0; j<n; j++) input_log << A1(i,j) << "\n";
     for (i=0; i<n; i++)                     input_log << B1(i)   << "\n";
     input_log.close();}

do {
     C = B / A;
     if (matrix::failure) { // C cannot be computed
          temp = det(A);
          if (temp != zero) test_failure = 8; // add precision and try again
          else {
               if (is_zero(temp)) error("coefficient matrix A is singular");
               test(temp, 2, d2a);
               if (!test_failure) {
                    cout << mess1 << d2a;
                    if (d2a > d2) exit(1); // must be after second try
                    else cout << mess2;    // first encounter with trouble
                    int_entry(!mess3, d2a, d2a+1, 100, "","100 is n max\n");
                    test(temp, 2, d2a);}}} // find precision increase needed
     else {
          test(C, d1, d2);
          if (!test_failure) break;}
     add_precision();
     evaluate(B, B1); evaluate(A, A1);}
while(1);

cout << "\n         Solution of equations\n";
B1 = rtos(C, d1, d2);
for (i=0; i<n; i++) {
     cout << "x("; cout.width(2); cout << i+1 << ") = " << B1(i) << "\n";}
}
     
