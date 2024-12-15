#define main_program
#include "range.h"

void main()
{
int n; rvar E; svar input_line;
cout << "\n                 Calculator program\n";
int_entry("Enter the number of decimal places desired    ", n, 0, 150,
"","150 places is max\n");
rvar::evaluate_exit_on_error = FALSE;
cout << "(To quit program type the letter q and then hit <RETURN>  ) \n\n";
list_tokens(E);
do {
     set_precision(n + 7);
     entry("Enter expression    ", E, input_line);
     while (1) {
          test(E, 1, n);
          if (test_failure) {
               add_precision(2); 
               evaluate(E, input_line);
               if (rvar::evaluate_error) break;}
          else break;}
     if (rvar::evaluate_error == FALSE) cout << rtos(E, 1, n) << "\n";}
while (1);
}
