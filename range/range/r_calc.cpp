#define main_program
#include "range.h"
#include "rational.h"

void main()
{
int n; rational E;
svar input_line;
cout << "\n                 Rational calculator program\n";
cout << "(To quit program type the letter q and then hit <RETURN>  ) \n\n";
list_tokens(E);
do {
     entry("Enter expression    ", E, input_line);
     cout << rtos(E) << "\n";}
while (1); 
}
