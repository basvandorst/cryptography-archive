#include <iostream.h>
 
main()
{
int size_i = sizeof(int), size_l = sizeof(long);
cout << "sizeof(int)  = " << size_i << "\n";
cout << "sizeof(long) = " << size_l << "\n\n";
if (size_i < 2 || size_l < 4) {
     cout << "Range arithmetic module will not work with this compiler\n";}
else if (size_i < 3 || size_l < 6) {
     cout << "#define four_digits should be left unchanged in range.h file\n";}
else if (size_i < 4 || size_l < 8) {
     cout << "#define four_digits should be changed to #define six_digits\n\
in range.h file for greater efficiency.\n";}
else {
     cout << "#define four_digits should be changed to #define eight_digits\n\
in range.h file for greater efficiency.";}
return 0;
}
