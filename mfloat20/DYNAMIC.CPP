extern unsigned _stklen = 40000U;

#include <conio.h>
#include <iomanip.h>
#include <alloc.h>
#include <dos.h>
#include "mf_cpp.h"                      // use the mfloat-library

int main(void) {

  #define maxdim 2000
  typedef mfloat* vector[maxdim];
  vector a;               //this is an array of pointers to mfloat variables
  int i;

  clrscr();
  cout << "\n\n";
  cout << "                       Dynamic data structures\n";
  cout << "                       =======================\n\n\n";
  cout << "The heap memory is free!\n";
  cout << "Number of free bytes:" << (unsigned long)coreleft() << "\n\n";
  // initialize
  for (i=0; i < maxdim; i++) {
    a[i]=new mfloat;             // if there is no memory --> pointer = 0
    if (!a[i]) {
      cout << "No memory available!\n";
      exit(1);
    }
  }
  cout << "Now the memory is allocated to the variables.\n";
  cout << "Number of free bytes:" << (unsigned long)coreleft() << "\n";
  cout << "Please wait!\n\n";
  delay(2000);
  // calculate (it has no sence here)
  for (i=0; i < maxdim; i++) {
    *a[i]=i;
    *a[i]=sqrt(*a[i]);
  }
  // output of the result
  for (i=0; i < maxdim; i++) {
    cout << "result " << i << "  = " << *a[i] << "\n";
  }
  // gain back memory
  for (i=0; i < maxdim; i++) {
    if (a[i]) delete a[i];     // checks, if the pointer isn't zero
  }
  cout << "\n\n";
  cout << "Now the heap memory is free again!\n";
  cout << "Number of free bytes:" << (unsigned long)coreleft() << "\n\n";
  return 0;
}
