#define main_program
#include <fstream.h>
#include "range.h"
#include "rational.h"
#include "vector.h"
#include "rvector.h"

void main()
{
cout <<"\n         Program to solve n linear equations in n unknowns\n";
cout <<"               for rational solutions in the form p/q\n\n";
cout <<"      All equation coefficients and constants must be rational\n";
cout <<"                   The matrix equation is AX = B\n\n";

if (cin.seekg(0,ios::beg).tellg() != cin.seekg(0,ios::end).tellg()) {
     svar::input_from_file = TRUE;}
else svar::input_from_file = FALSE;
cin.seekg(0,ios::beg); cin.clear();

int i, j, n;
int_entry("Enter the number of equations (1 to 20)   ", n, 1, 20);

rvector B(n), C; rmatrix A(n,n);
svector B1; smatrix A1;
rational temp;
list_tokens(temp);
entry("enter a", A, A1); entry("enter b", B, B1);

if (svar::input_from_file == FALSE && svar::input_logs_allowed == TRUE) {
     ofstream input_log("r_equat.log");
     input_log << ltos(n) << "\n";
     for (i=0; i<n; i++) for (j=0; j<n; j++) input_log << A1(i,j) << "\n";
     for (i=0; i<n; i++)                     input_log << B1(i)   << "\n";
     input_log.close();}

C = B / A;

B1 = rtos(C);
cout << "Solution of equations\n";
for (i=0; i<n; i++) {
     cout << "x("; cout.width(2); cout << i+1 << ") = " << B1(i) << "\n";}
}
