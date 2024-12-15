#define main_program
#include "range.h"
#include "interval.h"
#include "vector.h"
#include "series.h"

int dim; series var[3];
svar xe, ye, ze;

void increase_precision()
{
add_precision();
rvar temp; 
for (int i=1; i<=dim; i++) {
     switch (i) {
          case 1: evaluate(temp, xe);
          var[0].const_term(temp); break;
          case 2: evaluate(temp, ye);
          var[1].const_term(temp); break;
          case 3: evaluate(temp, ze);
          var[2].const_term(temp);}}
}

void main()
{
int d, n, output_lines;
rvar temp;
svar f_entry, variables, message;

cout << "\
\nProgram to find derivatives of a function at various entered arguments\n\n";
int_entry("\
Enter 1 for         derivatives of a function f(x)\n\
      2 for partial derivatives of a function f(x,y)\n\
      3 for partial derivatives of a function f(x,y,z)       ", dim, 1, 3);
if (dim == 1) {
     variables = svar("x");
     message = svar("Enter f(x)   ");}
else if (dim == 2) {
     variables = svar("x,y");
     message = svar("Enter f(x,y)   ");}
else {
     variables = svar("x,y,z");
     message = svar("Enter f(x,y,z)   ");}

set_precision(50);
list_tokens(temp);
entry(!message, zero, f_entry, variables);

int_entry("\
Enter the order of highest derivative desired    ", n, 1, 10,
"Min is 1\n", "Max is 10\n");
if (dim == 1)      output_lines = (n+1);
else if (dim == 2) output_lines = (n+1) * (n+2) / 2;
else               output_lines = (n+1) * (n+2) * (n+3) / 6;
svector output(output_lines);

int_entry("\
Enter the number of decimal places desired in derivative display   ", d,
0, 30, "", "Max is 30\n");
set_precision(d + 10);

cout << "(To quit program type the letter q and then hit <RETURN>  ) \n";
series s;
series::initiate(dim, n, var);
series::evaluate_exit_on_error = FALSE;
int i, k, k1, k2, k3, t;
while (1) {
     cout << "\nfunction is " << f_entry << "\n";
     for (i=1; i<=dim; i++) {
          switch (i) {
               case 1: entry("Enter x   ", temp, xe);
               var[0].const_term(temp); break;
               case 2: entry("Enter y   ", temp, ye);
               var[1].const_term(temp); break;
               case 3: entry("Enter z   ", temp, ze);
               var[2].const_term(temp);}}
     while (1) {
          evaluate(s, f_entry, var);
          if (series::evaluate_error) break;
          s = factorial_extend(s);
          test(s, 1, d);
          if (test_failure) increase_precision();
          else break;}

     if (series::evaluate_error == FALSE) switch (dim) {
          case 1: for (k=0; k<=n; k++) {
               output(k) = rtos(s(k,0,0).mid, 1, d);}
          break;
          case 2: t=0;
          for (k=0; k<=n; k++) {
               for (k1=k; k1>=0; k1--) {
                    k2 = k-k1;
                    output(t++) = rtos(s(k1,k2,0).mid, 1, d);}}
          break;
          case 3: t=0;
          for (k=0; k<=n; k++) {
               for (k1=k; k1>=0; k1--) {
                    for (k2=k-k1; k2>=0; k2--) {
                         k3 = k-k1-k2;
                         output(t++) = rtos(s(k1,k2,k3).mid, 1, d);}}}}

     int max_len; max_len = 0;
     if (series::evaluate_error == FALSE) {
          for (i=0; i<output_lines; i++) {
               k = output(i).len(); 
               if (k > max_len) max_len = k;}}

     if (series::evaluate_error == FALSE) switch (dim) {
          case 1:
          for (k=0; k<=n; k++) {
               if (k == 0) {
                    cout << "\n"; cout.width(16);
                    cout << "function = ";
                    cout.width(max_len);
                    cout << output(k) << "\n";}
               else {
                    cout << "derivative ";
                    cout.width(2); cout << k << " = ";
                    cout.width(max_len);
                    cout << output(k) << "\n";}}
          break;
          case 2: t=0;
          for (k=0; k<=n; k++) {
               if (k == 0) {
                    cout << "\n"; cout.width(20);
                    cout << "function = ";
                    cout.width(max_len);
                    cout << output(t++) << "\n";}
               else {
                    cout << "\n   derivatives of order ";
                    cout.width(2);
                    cout.setf(ios::left, ios::adjustfield);
                    cout << k << "\n";
                    cout.setf(ios::right, ios::adjustfield);
                    for (k1=k; k1>=0; k1--) {
                         k2 = k-k1;
                         if (k1 == 0) cout << "partial      ";
                         else {
                              cout << "partial ";
                              cout.width(2); cout << k1 << "Dx ";}
                         if (k2 == 0) cout << "     = ";
                         else {
                              cout.width(2); cout << k2 << "Dy = ";}
                         cout.width(max_len);
                         cout << output(t++) << "\n";}}}
          break;
          case 3: t=0;
          for (k=0; k<=n; k++) {
               if (k == 0) {
                    cout << "\n"; cout.width(25);
                    cout << "function = ";
                    cout.width(max_len);
                    cout << output(t++) << "\n";}
               else {
                    cout << "\n    derivatives of order ";
                    cout.width(2);
                    cout.setf(ios::left, ios::adjustfield);
                    cout << k << "\n";
                    cout.setf(ios::right, ios::adjustfield);
                    for (k1=k; k1>=0; k1--)
                    for (k2=k-k1; k2>=0; k2--) {
                         k3 = k-k1-k2;
                         if (k1 == 0) cout << "partial      ";
                         else {
                              cout << "partial ";
                              cout.width(2); cout << k1 << "Dx ";}
                         if (k2 == 0) cout << "     ";
                         else {
                              cout.width(2); cout << k2 << "Dy ";}
                         if (k3 == 0) cout << "     = ";
                         else {
                              cout .width(2); cout << k3 << "Dz = ";}
                         cout.width(max_len);
                         cout << output(t++) << "\n";}}}}}
}     
