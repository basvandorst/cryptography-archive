extern unsigned _stklen = 40000U;

#include <conio.h>
#include <alloc.h>
#include <dos.h>
#include "mfloat.h"                      /* use the mfloat-library */

int main(void) {

  #define maxdim 2000
  typedef mfloat* vector[maxdim];
  vector a;      /* this is an array of pointers to mfloat variables */
  int i;
  char ch[100];

  mfloatTest80386();
  clrscr();
  printf("\n\n");
  printf("                       Dynamic data structures\n");
  printf("                       =======================\n\n\n");
  printf("The heap memory is free!\n");
  printf("Number of free bytes: %li \n\n",(unsigned long)coreleft());
  /* initialize */
  for (i=0; i < maxdim; i++) {
    a[i]=(mfloat*)malloc(sizeof(mfloat));
    /* if there is no memory --> pointer = 0 */
    if (!a[i]) {
      printf("No memory available!\n");
      exit(1);
    }
  }
  printf("Now the memory is allocated to the variables. \n");
  printf("Number of free bytes: %li \n\n",(unsigned long)coreleft());
  printf("Please wait!\n\n");
  delay(2000);
  /* calculate (it has no sence here) */
  for (i=0; i < maxdim; i++) {
    ldtomf(*a[i],i);
    sqrtm(*a[i]);
  }
  /* output of the result */
  for (i=0; i < maxdim; i++)
    printf("result %i = %s \n",i,mftostr(ch,*a[i],100,".6F"));
  /* gain back memory */
  for (i=0; i < maxdim; i++) {
    if (a[i]) free(a[i]);     /* checks, if the pointer isn't zero */
  }
  printf("\n\n");
  printf("Now the heap memory is free again!\n");
  printf("Number of free bytes: %li \n\n",(unsigned long)coreleft());
  return 0;
}
