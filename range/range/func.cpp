#define main_program
#include <fstream.h>
#include "range.h"
     
void main()
{
svar f_entry, x0_entry, xi_entry, xf_entry, variables = "x";
rvar x, x0, xi, xf;
int xd1, xd2, fd1, fd2;

cout << "                   Program to evaluate a function f(x)\n";
cout << "                at equally spaced values of the argument x\n\n";

if (cin.seekg(0,ios::beg).tellg() != cin.seekg(0,ios::end).tellg()) {
     svar::input_from_file = TRUE;}
else svar::input_from_file = FALSE;
cin.seekg(0,ios::beg); cin.clear();
list_tokens(x0);
entry("Enter the function f(x)   ", zero, f_entry, variables);

entry("Enter initial x argument   ", x0, x0_entry);

do {
     entry("Enter x increment   ", xi, xi_entry);
     if (xi == zero) cout << "The increment can not be zero\n";
     else break;}
while(1);
do {
     entry("Enter final x argument   ", xf, xf_entry); 
     if (xi > zero && xf < x0 || xi < zero && xf > x0) {
          cout << "Final x argument is not consistent with increment\n";}
     else break;}
while(1);

int_entry("\
Enter x print format: 1 for fixed decimal point\n\
                      2 for floating decimal point   ", xd1, 1, 2);
                      
int_entry("Enter number of decimal places desired for x   ", xd2, 0, 30,
"", "30 decimals is the maximum\n");

int_entry("\
Enter f(x) print format: 1 for fixed decimal point\n\
                         2 for floating decimal point   ", fd1, 1, 2);
                         
int_entry("Enter number of decimal places desired for f(x)   ", fd2, 0, 30,
"", "30 decimals is the maximum\n");

if (svar::input_from_file == FALSE && svar::input_logs_allowed == TRUE) {
     ofstream input_log("func.log");
     input_log << f_entry   << "\n" << x0_entry  << "\n";
     input_log << xi_entry  << "\n" << xf_entry  << "\n";
     input_log << ltos(xd1) << "\n" << ltos(xd2) << "\n";
     input_log << ltos(fd1) << "\n" << ltos(fd2) << "\n";
     input_log.close();}

int fieldx, fieldf, xpos, fpos, code;
fieldx = rtos(one, xd1, xd2).len(); xpos = fieldx/2 - 1;
if (xd1 == 1) {fieldx += 5; xpos +=5;} else fieldx += 2;
fieldf = rtos(one, fd1, fd2).len(); fpos = fieldf/2 - 1;
if (fd1 == 1) {fieldf += 5; fpos +=5;} else fieldf += 2;
cout.setf(ios::right, ios::adjustfield);
cout << "\n\n"; cout.width(xpos); cout << ""; cout << "x";
cout.width(fieldx - xpos + 1 + fpos); cout << ""; cout << "f(x)\n";

rvar step = zero, f, num_steps = abs((xf - x0)/xi);
set_precision(fd2 + 10);
do {
     do {
          x = x0 + step * xi;
          evaluate(f, f_entry, &x);
          test(x, xd1, xd2); test(f, fd1, fd2);
          if (test_failure) {
               add_precision();
               evaluate(x0, x0_entry);
               evaluate(xi, xi_entry);}
          else break;}
     while(1);
     cout.width(fieldx); 
     if (xd1 == 1) {
          cout << rtos(x, xd1, xd2);}
     else {
          cout.setf(ios::left, ios::adjustfield);
          cout << rtos(x, xd1, xd2);
          cout.setf(ios::right, ios::adjustfield);}
     cout << "  ";
     cout.width(fieldf); 
     if (fd1 == 1) {
          cout << rtos(f, fd1, fd2);}
     else {
          cout.setf(ios::left, ios::adjustfield);
          cout << rtos(f, fd1, fd2);
          cout.setf(ios::right, ios::adjustfield);}
     cout << "\n"; cout.flush();
     step = step + one;}
while (step <= num_steps);
}     
     
     
     

